/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023 by
 *  Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Do all the WAD I/O, get map description,
 *  set up initial state and misc. LUTs.
 *
 *-----------------------------------------------------------------------------*/

#include <math.h>
#include <stdint.h>

#include "d_player.h"
#include "g_game.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "i_system.h"
#include "v_video.h"

#include "globdata.h"


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//

int32_t      _g_numvertexes;
const vertex_t __far* _g_vertexes;

const seg_t    __far* _g_segs;

int32_t      _g_numsectors;
sector_t __far* _g_sectors;


static int32_t      numsubsectors;
subsector_t __far* _g_subsectors;



int32_t      _g_numlines;
const line_t   __far* _g_lines;
linedata_t __far* _g_linedata;


static int32_t      numsides;
side_t   __far* _g_sides;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.

int16_t       _g_bmapwidth, _g_bmapheight;  // size in mapblocks

// killough 3/1/98: remove blockmap limit internally:
const int16_t      __far* _g_blockmap;

// offsets in blockmap are from here
const int16_t      __far* _g_blockmaplump;

fixed_t   _g_bmaporgx, _g_bmaporgy;     // origin of block map

mobj_t    __far*__far* _g_blocklinks;           // for thing chains

//
// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without the special effect, this could
// be used as a PVS lookup as well.
//

const byte __far* _g_rejectmatrix;

// Maintain single and multi player starting spots.
mapthing_t _g_playerstarts[MAXPLAYERS];

mobj_t __far*      _g_thingPool;
uint32_t _g_thingPoolSize;


// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum {
  ML_LABEL,             // A separator, name, ExMx or MAPxx
  ML_THINGS,            // Monsters, items..
  ML_LINEDEFS,          // LineDefs, from editing
  ML_SIDEDEFS,          // SideDefs, from editing
  ML_VERTEXES,          // Vertices, edited and BSP splits generated
  ML_SEGS,              // LineSegs, from LineDefs split by BSP
  ML_SSECTORS,          // SubSectors, list of LineSegs
  ML_NODES,             // BSP nodes
  ML_SECTORS,           // Sectors, from editing
  ML_REJECT,            // LUT, sector-sector visibility
  ML_BLOCKMAP           // LUT, motion clipping, walls/grid element
};


//
// P_LoadVertexes
//
// killough 5/3/98: reformatted, cleaned up
//
static void P_LoadVertexes (int16_t lump)
{
  // Determine number of lumps:
  //  total lump length / vertex record length.
  _g_numvertexes = W_LumpLength(lump) / sizeof(vertex_t);

  // Allocate zone memory for buffer.
  _g_vertexes = W_GetLumpByNumAutoFree(lump);

}

//
// P_LoadSegs
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadSegs (int16_t lump)
{
    int32_t numsegs = W_LumpLength(lump) / sizeof(seg_t);
    _g_segs = (const seg_t __far*)W_GetLumpByNumAutoFree(lump);

    if (!numsegs)
      I_Error("P_LoadSegs: no segs in level");
}

//
// P_LoadSubsectors
//
// killough 5/3/98: reformatted, cleaned up

// SubSector, as generated by BSP.
typedef PACKEDATTR_PRE struct {
  uint16_t numsegs;
  uint16_t firstseg;    // Index of first one; segs are stored sequentially.
} PACKEDATTR_POST mapsubsector_t;

static void P_LoadSubsectors (int16_t lump)
{
  /* cph 2006/07/29 - make data a const mapsubsector_t *, so the loop below is simpler & gives no constness warnings */
  const mapsubsector_t __far* data;
  int32_t  i;

  numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
  _g_subsectors = Z_CallocLevel(numsubsectors * sizeof(subsector_t));
  data = (const mapsubsector_t __far*)W_GetLumpByNumAutoFree(lump);

  if ((!data) || (!numsubsectors))
    I_Error("P_LoadSubsectors: no subsectors in level");

  for (i=0; i<numsubsectors; i++)
  {
    _g_subsectors[i].numlines  = (uint16_t)SHORT(data[i].numsegs );
    _g_subsectors[i].firstline = (uint16_t)SHORT(data[i].firstseg);
  }
}

//
// P_LoadSectors
//
// killough 5/3/98: reformatted, cleaned up

// Sector definition, from editing.
typedef PACKEDATTR_PRE struct {
  int16_t floorheight;
  int16_t ceilingheight;
  char  floorpic[8];
  char  ceilingpic[8];
  int16_t lightlevel;
  int16_t special;
  int16_t tag;
} PACKEDATTR_POST mapsector_t;

static int16_t R_FlatNumForFarName(const char __far* far_name)
{
	uint64_t nameint = *(uint64_t __far*)far_name;
	char* near_name = (char*)&nameint;
	return R_FlatNumForName(near_name);
}

static void P_LoadSectors (int16_t lump)
{
  const byte __far* data;
  int32_t  i;

  _g_numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
  _g_sectors = Z_CallocLevel(_g_numsectors * sizeof(sector_t));
  data = W_GetLumpByNumAutoFree (lump); // cph - wad lump handling updated

  for (i=0; i<_g_numsectors; i++)
    {
      sector_t __far* ss = _g_sectors + i;
      const mapsector_t __far* ms = (const mapsector_t __far*) data + i;

      ss->floorheight = ((int32_t)SHORT(ms->floorheight))<<FRACBITS;
      ss->ceilingheight = ((int32_t)SHORT(ms->ceilingheight))<<FRACBITS;
      ss->floorpic   = R_FlatNumForFarName(ms->floorpic);
      ss->ceilingpic = R_FlatNumForFarName(ms->ceilingpic);

      ss->lightlevel = SHORT(ms->lightlevel);
      ss->special = SHORT(ms->special);
      ss->oldspecial = SHORT(ms->special);
      ss->tag = SHORT(ms->tag);

      ss->thinglist = NULL;
      ss->touching_thinglist = NULL;            // phares 3/14/98
    }
}


//
// P_LoadNodes
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadNodes (int16_t lump)
{
  numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
  nodes = W_GetLumpByNumAutoFree (lump); // cph - wad lump handling updated

  if ((!nodes) || (!numnodes))
  {
    // allow trivial maps
    if (numsubsectors == 1)
      printf("P_LoadNodes: trivial map (no nodes, one subsector)\n");
    else
      I_Error("P_LoadNodes: no nodes in level");
  }
}


/*
 * P_IsDoomnumAllowed()
 * Based on code taken from P_LoadThings() in src/p_setup.c  Return TRUE
 * if the thing in question is expected to be available.
 */

//#define NOMONSTERS

static boolean P_IsDoomnumAllowed(int16_t doomnum)
{
  // Do not spawn cool, new monsters
  switch(doomnum)
    {
#if defined NOMONSTERS
    case    7: // Spiderdemon
    case    9: // Shotgun guy
    case   16: // Cyberdemon
    case   58: // Spectre
    case   72: // Commander Keen
    case 3001: // Imp
    case 3002: // Demon
    case 3003: // Baron of Hell
    case 3004: // Zombieman
    case 3005: // Cacodemon
    case 3006: // Lost soul
#endif
    case 64:  // Arch-vile
    case 65:  // Heavy weapon dude
    case 66:  // Revenant
    case 67:  // Mancubus
    case 68:  // Arachnotron
    case 69:  // Hell knight
    case 71:  // Pain elemental
    case 84:  // Wolfenstein SS
    case 88:  // Romero's head
    case 89:  // Monster spawner
      return false;
    }

  return true;
}


/*
 * P_LoadThings
 *
 * killough 5/3/98: reformatted, cleaned up
 * cph 2001/07/07 - don't write into the lump cache, especially non-idepotent
 * changes like byte order reversals. Take a copy to edit.
 */

static void P_LoadThings (int16_t lump)
{
    int32_t  i, numthings = W_LumpLength (lump) / sizeof(mapthing_t);
    const mapthing_t __far* data = W_GetLumpByNumAutoFree (lump);

    if ((!data) || (!numthings))
        I_Error("P_LoadThings: no things in level");

    _g_thingPool = Z_CallocLevel(numthings * sizeof(mobj_t));
    _g_thingPoolSize = numthings;

    for(int16_t i = 0; i < numthings; i++)
    {
        _g_thingPool[i].type = MT_NOTHING;
    }

    for (i=0; i<numthings; i++)
    {
        const mapthing_t __far* mt = &data[i];

        if (!P_IsDoomnumAllowed(mt->type))
            continue;

        // Do spawn all other stuff.
        P_SpawnMapThing(mt);
    }
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//        ^^^
// ??? killough ???
// Does this mean secrets used to be linedef-based, rather than sector-based?
//
// killough 4/4/98: split into two functions, to allow sidedef overloading
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadLineDefs (int16_t lump)
{
    int32_t  i;

    _g_numlines = W_LumpLength (lump) / sizeof(line_t);
    _g_lines = W_GetLumpByNumAutoFree (lump);

    _g_linedata = Z_CallocLevel(_g_numlines * sizeof(linedata_t));

    for (i=0; i<_g_numlines; i++)
    {
        _g_linedata[i].special = _g_lines[i].const_special;
    }
}


// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef PACKEDATTR_PRE struct {
  int16_t textureoffset;
  int16_t rowoffset;
  int16_t toptexture;
  int16_t bottomtexture;
  int16_t midtexture;
  int16_t sector;  // Front sector, towards viewer.
} PACKEDATTR_POST mapsidedef_t;


//
// P_LoadSideDefs
//
// killough 4/4/98: split into two functions

static void P_LoadSideDefs (int16_t lump)
{
  numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
  _g_sides = Z_CallocLevel(numsides * sizeof(side_t));
}

// killough 4/4/98: delay using texture names until
// after linedefs are loaded, to allow overloading.
// killough 5/3/98: reformatted, cleaned up

static void P_LoadSideDefs2(int16_t lump)
{
    const byte __far* data = W_GetLumpByNumAutoFree(lump); // cph - const*, wad lump handling updated

    for (int16_t i = 0; i < numsides; i++)
    {
        register const mapsidedef_t __far* msd = (const mapsidedef_t __far*) data + i;
        register side_t __far* sd = _g_sides + i;
        register sector_t __far* sec;

        sd->textureoffset = msd->textureoffset;
        sd->rowoffset     = msd->rowoffset;

        /* cph 2006/09/30 - catch out-of-range sector numbers; use sector 0 instead */
        uint16_t sector_num = SHORT(msd->sector);
        if (sector_num >= _g_numsectors)
        {
            printf("P_LoadSideDefs2: sidedef %i has out-of-range sector num %u\n", i, sector_num);
            sector_num = 0;
        }
        sd->sector = sec = &_g_sectors[sector_num];

        sd->midtexture    = msd->midtexture;
        sd->toptexture    = msd->toptexture;
        sd->bottomtexture = msd->bottomtexture;

        R_GetTexture(sd->midtexture);
        R_GetTexture(sd->toptexture);
        R_GetTexture(sd->bottomtexture);
    }
}

//
// jff 10/6/98
// New code added to speed up calculation of internal blockmap
// Algorithm is order of nlines*(ncols+nrows) not nlines*ncols*nrows
//

#define blkshift 7               /* places to shift rel position for cell num */
#define blkmask ((1<<blkshift)-1)/* mask for rel position within cell */
#define blkmargin 0              /* size guardband around map used */
                                 // jff 10/8/98 use guardband>0
                                 // jff 10/12/98 0 ok with + 1 in rows,cols

typedef struct linelist_t        // type used to list lines in each block
{
  int32_t num;
  struct linelist_t *next;
} linelist_t;

//
// P_LoadBlockMap
//
// killough 3/1/98: substantially modified to work
// towards removing blockmap limit (a wad limitation)
//
// killough 3/30/98: Rewritten to remove blockmap limit,
// though current algorithm is brute-force and unoptimal.
//

static void P_LoadBlockMap (int16_t lump)
{
    _g_blockmaplump = W_GetLumpByNumAutoFree(lump);

    _g_bmaporgx = ((int32_t)_g_blockmaplump[0])<<FRACBITS;
    _g_bmaporgy = ((int32_t)_g_blockmaplump[1])<<FRACBITS;
    _g_bmapwidth  = _g_blockmaplump[2];
    _g_bmapheight = _g_blockmaplump[3];


    // clear out mobj chains - CPhipps - use calloc
    _g_blocklinks = Z_CallocLevel(_g_bmapwidth * _g_bmapheight * sizeof(*_g_blocklinks));

    _g_blockmap = _g_blockmaplump+4;
}

//
// P_LoadReject - load the reject table
// 

static void P_LoadReject(int16_t lump)
{
  _g_rejectmatrix = W_GetLumpByNumAutoFree(lump);
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
// killough 5/3/98: reformatted, cleaned up
// cph 18/8/99: rewritten to avoid O(numlines * numsectors) section
// It makes things more complicated, but saves seconds on big levels
// figgi 09/18/00 -- adapted for gl-nodes

// cph - convenient sub-function
static void P_AddLineToSector(const line_t __far* li, sector_t __far* sector)
{
  sector->lines[sector->linecount++] = li;
}

static void M_ClearBox (fixed_t *box)
{
    box[BOXTOP]    = box[BOXRIGHT] = INT32_MIN;
    box[BOXBOTTOM] = box[BOXLEFT]  = INT32_MAX;
}

static void M_AddToBox(fixed_t* box,fixed_t x,fixed_t y)
{
    if (x<box[BOXLEFT])
        box[BOXLEFT]  = x;
    else if (x>box[BOXRIGHT])
        box[BOXRIGHT] = x;

    if (y<box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    else if (y>box[BOXTOP])
        box[BOXTOP]    = y;
}

static void P_GroupLines (void)
{
    register const line_t __far* li;
    register sector_t __far* sector;
    int16_t i,j, total = _g_numlines;

    // figgi
    for (i=0 ; i<numsubsectors ; i++)
    {
        const seg_t __far* seg = &_g_segs[_g_subsectors[i].firstline];
        _g_subsectors[i].sector = NULL;
        for(j=0; j<_g_subsectors[i].numlines; j++)
        {
            if(seg->sidenum != NO_INDEX)
            {
                _g_subsectors[i].sector = _g_sides[seg->sidenum].sector;
                break;
            }
            seg++;
        }
        if(_g_subsectors[i].sector == NULL)
            I_Error("P_GroupLines: Subsector a part of no sector!\n");
    }

    // count number of lines in each sector
    for (i=0,li=_g_lines; i<_g_numlines; i++, li++)
    {
        LN_FRONTSECTOR(li)->linecount++;
        if (LN_BACKSECTOR(li) && LN_BACKSECTOR(li) != LN_FRONTSECTOR(li))
        {
            LN_BACKSECTOR(li)->linecount++;
            total++;
        }
    }

    {  // allocate line tables for each sector
        const line_t __far*__far*linebuffer = Z_MallocLevel(total*sizeof(line_t __far*), NULL);

        for (i=0, sector = _g_sectors; i<_g_numsectors; i++, sector++)
        {
            sector->lines = linebuffer;
            linebuffer += sector->linecount;
            sector->linecount = 0;
        }
    }

    // Enter those lines
    for (i=0,li=_g_lines; i<_g_numlines; i++, li++)
    {
        P_AddLineToSector(li, LN_FRONTSECTOR(li));
        if (LN_BACKSECTOR(li) && LN_BACKSECTOR(li) != LN_FRONTSECTOR(li))
            P_AddLineToSector(li, LN_BACKSECTOR(li));
    }

    for (i=0, sector = _g_sectors; i<_g_numsectors; i++, sector++)
    {
        fixed_t bbox[4];
        M_ClearBox(bbox);

        for(int16_t l = 0; l < sector->linecount; l++)
        {
            M_AddToBox (bbox, sector->lines[l]->v1.x, sector->lines[l]->v1.y);
            M_AddToBox (bbox, sector->lines[l]->v2.x, sector->lines[l]->v2.y);
        }

        sector->soundorg.x = bbox[BOXRIGHT]/2+bbox[BOXLEFT]/2;
        sector->soundorg.y = bbox[BOXTOP]/2+bbox[BOXBOTTOM]/2;
    }
}


//Planes are alloc'd with PU_LEVEL tag so are dumped at level
//end. This function resets the visplane arrays.
static void R_ResetPlanes()
{
    memset(_g_visplanes, 0, sizeof(_g_visplanes));
    _g_freetail = NULL;
    freehead = &_g_freetail;
}


static void P_FreeLevelData()
{
    R_ResetPlanes();

    Z_FreeTags();
}

//
// P_SetupLevel
//

void P_SetupLevel(int16_t map)
{
    int_fast8_t   i;
    char  lumpname[9];
    int16_t   lumpnum;

    _g_totallive = _g_totalkills = _g_totalitems = _g_totalsecret = 0;
    _g_wminfo.partime = 180;

    for (i=0; i<MAXPLAYERS; i++)
        _g_player.killcount = _g_player.secretcount = _g_player.itemcount = 0;

    // Initial height of PointOfView will be set by player think.
    _g_player.viewz = 1;

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start();

    P_FreeLevelData();

    P_InitThinkers();

    _g_leveltime = 0;
    _g_totallive = 0;

    // find map name
    sprintf(lumpname, "E1M%d", map);   // killough 1/24/98: simplify

    lumpnum = W_GetNumForName(lumpname);

    P_LoadVertexes  (lumpnum + ML_VERTEXES);
    P_LoadSectors   (lumpnum + ML_SECTORS);
    P_LoadSideDefs  (lumpnum + ML_SIDEDEFS);
    P_LoadLineDefs  (lumpnum + ML_LINEDEFS);
    P_LoadSideDefs2 (lumpnum + ML_SIDEDEFS);
    P_LoadBlockMap  (lumpnum + ML_BLOCKMAP);
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes     (lumpnum + ML_NODES);
    P_LoadSegs      (lumpnum + ML_SEGS);
    P_LoadReject    (lumpnum + ML_REJECT);

    P_GroupLines();

    // Note: you don't need to clear player queue slots --
    // a much simpler fix is in g_game.c -- killough 10/98

    /* cph - reset all multiplayer starts */
    memset(_g_playerstarts,0,sizeof(_g_playerstarts));

    for (i = 0; i < MAXPLAYERS; i++)
        _g_player.mo = NULL;

    P_MapStart();

    P_LoadThings(lumpnum + ML_THINGS);

    if (_g_playeringame && !_g_player.mo)
        I_Error("P_SetupLevel: missing player %d start\n", i+1);

    // set up world state
    P_SpawnSpecials();

    P_MapEnd();
}

//
// P_Init
//
void P_Init (void)
{
    P_InitSwitchList();
    P_InitPicAnims();
    R_InitSprites();
}
