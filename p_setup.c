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
 *  Copyright 2023, 2024 by
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

const seg_t    __far* _g_segs;

int16_t      _g_numsectors;
sector_t __far* _g_sectors;


static int16_t      numsubsectors;
subsector_t __far* _g_subsectors;



int16_t      _g_numlines;
const line_t   __far* _g_lines;
linedata_t __far* _g_linedata;


static int16_t      numsides;
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

mobj_t __far*      _g_thingPool;
int16_t _g_thingPoolSize;


// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum {
  ML_LABEL,             // A separator, name, ExMx or MAPxx
  ML_THINGS,            // Monsters, items..
  ML_LINEDEFS,          // LineDefs, from editing
  ML_SIDEDEFS,          // SideDefs, from editing
  ML_SEGS,              // LineSegs, from LineDefs split by BSP
  ML_SSECTORS,          // SubSectors, list of LineSegs
  ML_NODES,             // BSP nodes
  ML_SECTORS,           // Sectors, from editing
  ML_REJECT,            // LUT, sector-sector visibility
  ML_BLOCKMAP           // LUT, motion clipping, walls/grid element
};


//
// P_LoadSegs
//

static void P_LoadSegs (int16_t lump)
{
    _g_segs = (const seg_t __far*)W_GetLumpByNumAutoFree(lump);
}

//
// P_LoadSubsectors
//

// SubSector, as generated by BSP.
typedef struct {
  int8_t numsegs;
} mapsubsector_t;

typedef char assertMapsubsectorSize[sizeof(mapsubsector_t) == 1 ? 1 : -1];

static void P_LoadSubsectors (int16_t lump)
{
  const mapsubsector_t __far* data;
  int16_t  i;
  uint16_t firstseg;    // Index of first one; segs are stored sequentially.

  numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
  _g_subsectors = Z_CallocLevel(numsubsectors * sizeof(subsector_t));
  data = W_GetLumpByNum(lump);

  firstseg = 0;
  for (i=0; i<numsubsectors; i++)
  {
    _g_subsectors[i].numlines  = data[i].numsegs;
    _g_subsectors[i].firstline = firstseg;
    firstseg += data[i].numsegs;
  }

  Z_Free(data);
}

//
// P_LoadSectors
//

// Sector definition, from editing.
#if defined FLAT_SPAN
typedef struct {
  int16_t floorheight;
  int16_t ceilingheight;
  int16_t floorpic;
  int16_t ceilingpic;
  uint8_t lightlevel;
  int8_t special;
  int16_t tag;
} mapsector_t;

typedef char assertMapsectorSize[sizeof(mapsector_t) == 12 ? 1 : -1];

#define R_FlatNumForFarName(p) (p)

#else
typedef struct {
  int16_t floorheight;
  int16_t ceilingheight;
  char  floorpic[8];
  char  ceilingpic[8];
  uint8_t lightlevel;
  int8_t special;
  int16_t tag;
} mapsector_t;

typedef char assertMapsectorSize[sizeof(mapsector_t) == 24 ? 1 : -1];

static int16_t R_FlatNumForFarName(const char __far* far_name)
{
	uint64_t nameint = *(uint64_t __far*)far_name;
	char* near_name = (char*)&nameint;
	return R_FlatNumForName(near_name);
}
#endif


static void P_LoadSectors (int16_t lump)
{
  const mapsector_t __far* data;
  int16_t  i;

  _g_numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
  _g_sectors = Z_CallocLevel(_g_numsectors * sizeof(sector_t));
  data = W_GetLumpByNum(lump);

  for (i=0; i<_g_numsectors; i++)
    {
      sector_t __far* ss = _g_sectors + i;
      const mapsector_t __far* ms = data + i;

      ss->floorheight = ((int32_t)SHORT(ms->floorheight))<<FRACBITS;
      ss->ceilingheight = ((int32_t)SHORT(ms->ceilingheight))<<FRACBITS;
      ss->floorpic   = R_FlatNumForFarName(ms->floorpic);
      ss->ceilingpic = R_FlatNumForFarName(ms->ceilingpic);

      ss->lightlevel = ms->lightlevel;
      ss->special    = ms->special;
      ss->oldspecial = ms->special;
      ss->tag = SHORT(ms->tag);

      ss->thinglist = NULL;
      ss->touching_thinglist = NULL;
    }

  Z_Free(data);
}


//
// P_LoadNodes
//

static void P_LoadNodes (int16_t lump)
{
  numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
  nodes = W_GetLumpByNumAutoFree (lump);
}


/*
 * P_LoadThings
 *
 */

static void P_LoadThings (int16_t lump)
{
    int16_t  numthings = W_LumpLength (lump) / sizeof(mapthing_t);
    const mapthing_t __far* data = W_GetLumpByNum(lump);

    _g_thingPool = Z_CallocLevel(numthings * sizeof(mobj_t));
    _g_thingPoolSize = numthings;

    for (int16_t i = 0; i < numthings; i++)
    {
        _g_thingPool[i].type = MT_NOTHING;
    }

    for (int16_t i=0; i<numthings; i++)
    {
        const mapthing_t __far* mt = &data[i];

        // Do spawn all other stuff.
        P_SpawnMapThing(mt);
    }

    Z_Free(data);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//        ^^^
// ??? killough ???
// Does this mean secrets used to be linedef-based, rather than sector-based?
//

typedef struct
{
	vertex16_t v1;
	vertex16_t v2;			// Vertices, from v1 to v2.

	int16_t dx, dy;			// Precalculated v2 - v1 for side checking.

	uint16_t sidenum[2];	// Visual appearance: SideDefs.
	int16_t bbox[4];		// Line bounding box.

	uint8_t flags;			// Animation related.
	int8_t const_special;
	int8_t tag;
	int8_t slopetype;		// To aid move clipping.

} packed_line_t;

typedef char assertLineSize[sizeof(packed_line_t) == 28 ? 1 : -1];

static void P_LoadLineDefs (int16_t lump)
{
	_g_numlines = W_LumpLength(lump) / sizeof(packed_line_t);
	_g_lines    = Z_MallocLevel(_g_numlines * sizeof(line_t), NULL);
	_g_linedata = Z_CallocLevel(_g_numlines * sizeof(linedata_t));

	line_t __far* _w_lines = (line_t __far*) _g_lines;

	const packed_line_t __far* lines = W_GetLumpByNum(lump);

	for (int16_t i = 0; i < _g_numlines; i++)
	{
		_w_lines[i].v1         = lines[i].v1;
		_w_lines[i].v2         = lines[i].v2;
		_w_lines[i].lineno     = i;
		_w_lines[i].dx         = lines[i].dx;
		_w_lines[i].dy         = lines[i].dy;
		_w_lines[i].sidenum[0] = lines[i].sidenum[0];
		_w_lines[i].sidenum[1] = lines[i].sidenum[1];
		_w_lines[i].bbox[0]    = lines[i].bbox[0];
		_w_lines[i].bbox[1]    = lines[i].bbox[1];
		_w_lines[i].bbox[2]    = lines[i].bbox[2];
		_w_lines[i].bbox[3]    = lines[i].bbox[3];
		_w_lines[i].flags      = lines[i].flags;
		_w_lines[i].tag        = lines[i].tag;
		_w_lines[i].slopetype  = lines[i].slopetype;

		_g_linedata[i].special = lines[i].const_special;
	}

	Z_Free(lines);
}


// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef PACKEDATTR_PRE struct {
  int16_t textureoffset;
  uint8_t rowoffset;
   int8_t toptexture;
   int8_t bottomtexture;
   int8_t midtexture;
  uint8_t sector;  // Front sector, towards viewer.
} PACKEDATTR_POST mapsidedef_t;

typedef char assertMapsidedefSize[sizeof(mapsidedef_t) == 7 ? 1 : -1];


//
// P_LoadSideDefs
//

static void P_LoadSideDefs (int16_t lump)
{
  numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
  _g_sides = Z_CallocLevel(numsides * sizeof(side_t));
}


static void P_LoadSideDefs2(int16_t lump)
{
    const mapsidedef_t __far* data = W_GetLumpByNum(lump);

    for (int16_t i = 0; i < numsides; i++)
    {
        const mapsidedef_t __far* msd = data + i;
        side_t __far* sd = _g_sides + i;

        sd->textureoffset = msd->textureoffset;
        sd->rowoffset     = msd->rowoffset;
        sd->sector        = &_g_sectors[msd->sector];
        sd->midtexture    = msd->midtexture;
        sd->toptexture    = msd->toptexture;
        sd->bottomtexture = msd->bottomtexture;

        P_LoadTexture(sd->midtexture);
        P_LoadTexture(sd->toptexture);
        P_LoadTexture(sd->bottomtexture);
    }

    Z_Free(data);
}


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
    const line_t __far* li;
    sector_t __far* sector;
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
            M_AddToBox (bbox, (fixed_t)sector->lines[l]->v1.x<<FRACBITS, (fixed_t)sector->lines[l]->v1.y<<FRACBITS);
            M_AddToBox (bbox, (fixed_t)sector->lines[l]->v2.x<<FRACBITS, (fixed_t)sector->lines[l]->v2.y<<FRACBITS);
        }

        sector->soundorg.x = bbox[BOXRIGHT]/2+bbox[BOXLEFT]/2;
        sector->soundorg.y = bbox[BOXTOP]/2+bbox[BOXBOTTOM]/2;
    }
}


static void P_FreeLevelData()
{
#if !defined FLAT_SPAN
    R_ResetPlanes();
#endif

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

    P_LoadLineDefs  (lumpnum + ML_LINEDEFS);
    P_LoadSegs      (lumpnum + ML_SEGS);
    P_LoadSideDefs  (lumpnum + ML_SIDEDEFS);
    P_LoadSectors   (lumpnum + ML_SECTORS);
    P_LoadSideDefs2 (lumpnum + ML_SIDEDEFS);
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes     (lumpnum + ML_NODES);
    P_LoadBlockMap  (lumpnum + ML_BLOCKMAP);
    P_LoadReject    (lumpnum + ML_REJECT);

    P_GroupLines();

    // Note: you don't need to clear player queue slots
    // a much simpler fix is in g_game.c

    for (i = 0; i < MAXPLAYERS; i++)
        _g_player.mo = NULL;

    P_LoadThings(lumpnum + ML_THINGS);

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
