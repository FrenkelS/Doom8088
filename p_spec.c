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
 *   -Loads and initializes texture and flat animation sequences
 *   -Implements utility functions for all linedef/sector special handlers
 *   -Dispatches walkover and gun line triggers
 *   -Initializes and implements special sector types
 *   -Implements donut linedef triggers
 *   -Initializes and implements BOOM linedef triggers for
 *     Scrollers/Conveyors
 *     Friction
 *     Wind/Current
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_setup.h"
#include "m_random.h"
#include "d_englsh.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_data.h"
#include "p_maputl.h"
#include "p_map.h"
#include "g_game.h"
#include "p_inter.h"
#include "p_user.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_system.h"

#include "globdata.h"


//
// Animating textures and planes
//
static int16_t  animated_texture_basepic;

//
// P_InitPicAnims
//
// Load the table of animation definitions, checking for existence of
// the start and end of each frame. If the start doesn't exist or
// the last doesn't exist, BOOM exits.
//
// Wall/Flat animation sequences, defined by name of first and last frame,
// The full animation sequence is given using all lumps between the start
// and end entry, in the order found in the WAD file.
//
void P_InitPicAnims (void)
{
	animated_texture_basepic = R_CheckTextureNumForName ("SLADRIP1");
	                           R_CheckTextureNumForName ("SLADRIP3");
}

///////////////////////////////////////////////////////////////
//
// Linedef and Sector Special Implementation Utility Functions
//
///////////////////////////////////////////////////////////////

//
// getNextSector()
//
// Return sector_t * of sector next to current across line.
//
// Note: returns NULL if not two-sided line, or both sides refer to sector
//
sector_t __far* getNextSector(const line_t __far* line, sector_t __far* sec)
{


  if (LN_FRONTSECTOR(line) == sec)
  {
    if (LN_BACKSECTOR(line)!=sec)
      return LN_BACKSECTOR(line); //jff 5/3/98 don't retn sec unless compatibility
    else                       // fixes an intra-sector line breaking functions
      return NULL;             // like floor->highest floor
  }
  return LN_FRONTSECTOR(line);
}


//
// P_FindLowestFloorSurrounding()
//
// Returns the fixed point value of the lowest floor height
// in the sector passed or its surrounding sectors.
//
fixed_t P_FindLowestFloorSurrounding(sector_t __far* sec)
{
  int16_t                 i;
  const line_t __far*             check;
  sector_t __far*           other;
  fixed_t             floor = sec->floorheight;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->floorheight < floor)
      floor = other->floorheight;
  }
  return floor;
}


//
// P_FindHighestFloorSurrounding()
//
// Passed a sector, returns the fixed point value of the largest
// floor height in the surrounding sectors, not including that passed
//
fixed_t P_FindHighestFloorSurrounding(sector_t __far* sec)
{
  int16_t i;
  const line_t __far* check;
  sector_t __far* other;
  fixed_t floor = -32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->floorheight > floor)
      floor = other->floorheight;
  }
  return floor;
}


//
// P_FindLowestCeilingSurrounding()
//
// Passed a sector, returns the fixed point value of the smallest
// ceiling height in the surrounding sectors, not including that passed
//
fixed_t P_FindLowestCeilingSurrounding(sector_t __far* sec)
{
  int16_t                 i;
  const line_t __far*             check;
  sector_t __far*           other;
  fixed_t             height = 32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->ceilingheight < height)
      height = other->ceilingheight;
  }
  return height;
}


//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
int16_t P_FindSectorFromLineTag(const line_t __far* line, int16_t start)
{
    int16_t	i;

    for (i=start+1; i<_g_numsectors; i++)
    {
        if (_g_sectors[i].tag == line->tag)
            return i;
    }

    return -1;
}


//
// P_CheckTag()
//
// Passed a line, returns true if the tag is non-zero or the line special
// allows no tag without harm. If compatibility, all linedef specials are
// allowed to have zero tag.
//
// Note: Only line specials activated by walkover, pushing, or shooting are
//       checked by this routine.
//
// jff 2/27/98 Added to check for zero tag allowed for regular special types
//
boolean P_CheckTag(const line_t __far* line)
{
  /* tag not zero, allowed, or
   * killough 11/98: compatibility option */
  if (line->tag)
    return true;

  switch(LN_SPECIAL(line))
  {
    case 1:                 // Manual door specials
    case 26:
    case 27:
    case 28:
    case 31:
    case 32:
    case 33:
    case 34:

    case 35:                // Lighting specials

    case 97:                // Thing teleporters

    case 11:                // Exits
    case 51:

    case 48:                // Scrolling walls
      return true;   // zero tag allowed

    default:
      break;
  }
  return false;       // zero tag not allowed
}


//////////////////////////////////////////////////////////////////////////
//
// Events
//
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//
/////////////////////////////////////////////////////////////////////////

//
// P_UpdateSpecials()
//
// Check level timer, frag counter,
// animate flats, scroll walls,
// change button textures
//
// Reads and modifies globals:
//  levelTimer, levelTimeCount,
//  levelFragLimit, levelFragLimitCount
//

static void P_UpdateAnimatedTexture(void)
{
	int16_t pic = animated_texture_basepic + ((_g_leveltime >> 3) % 3);

	for (int16_t i = animated_texture_basepic; i < animated_texture_basepic + 3; i++)
		texturetranslation[i] = pic;
}

void P_UpdateSpecials (void)
{
    // Animate flats and textures globally
    P_UpdateAnimatedFlat();
    P_UpdateAnimatedTexture();

    // Check buttons (retriggerable switches) and change texture on timeout
    for (int8_t i = 0; i < MAXBUTTONS; i++)
    {
        if (_g_buttonlist[i].btimer)
        {
            _g_buttonlist[i].btimer--;

            if (!_g_buttonlist[i].btimer)
            {
                switch(_g_buttonlist[i].where)
                {
                    case top:
                        _g_sides[_g_buttonlist[i].line->sidenum[0]].toptexture =
                                _g_buttonlist[i].btexture;
                        break;

                    case middle:
                        _g_sides[_g_buttonlist[i].line->sidenum[0]].midtexture =
                                _g_buttonlist[i].btexture;
                        break;

                    case bottom:
                        _g_sides[_g_buttonlist[i].line->sidenum[0]].bottomtexture =
                                _g_buttonlist[i].btexture;
                        break;
                }

                S_StartSound2(_g_buttonlist[i].soundorg, sfx_swtchn);
                memset(&_g_buttonlist[i],0,sizeof(button_t));
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Sector and Line special thinker spawning at level startup
//
//////////////////////////////////////////////////////////////////////

// killough 3/7/98: Initialize generalized scrolling
static void P_SpawnScrollers(void);

//
// P_SpawnSpecials
// After the map has been loaded,
//  scan for specials that spawn thinkers
//

void P_SpawnSpecials (void)
{
  sector_t __far*   sector;
  int16_t         i;

  //  Init special sectors.
  sector = _g_sectors;
  for (i=0 ; i<_g_numsectors ; i++, sector++)
  {
    if (!sector->special)
      continue;

    switch (sector->special)
    {
      case 1:
        // random off
        P_SpawnLightFlash (sector);
        break;

      case 2:
        // strobe fast
        P_SpawnStrobeFlash(sector,FASTDARK,false);
        break;

      case 3:
        // strobe slow
        P_SpawnStrobeFlash(sector,SLOWDARK,false);
        break;

      case 8:
        // glowing light
        P_SpawnGlowingLight(sector);
        break;
      case 9:
        // secret sector
        _g_totalsecret++;
        break;

      case 12:
        // sync strobe slow
        P_SpawnStrobeFlash (sector, SLOWDARK, true);
        break;

      case 13:
        // sync strobe fast
        P_SpawnStrobeFlash (sector, FASTDARK, true);
        break;
    }
  }

  P_RemoveAllActivePlats();

  for (i = 0;i < MAXBUTTONS;i++)
    memset(&_g_buttonlist[i],0,sizeof(button_t));

  P_SpawnScrollers(); // killough 3/7/98: Add generalized scrollers
}


//
// This function, with the help of r_bsp.c, supports generalized
// scrolling floors and walls, with optional mobj-carrying properties, e.g.
// conveyor belts, rivers, etc. A linedef with a special type affects all
// tagged sectors the same way, by creating scrolling and/or object-carrying
// properties. Multiple linedefs may be used on the same sector and are
// cumulative, although the special case of scrolling a floor and carrying
// things on it, requires only one linedef. The linedef's direction determines
// the scrolling direction, and the linedef's length determines the scrolling
// speed. This was designed so that an edge around the sector could be used to
// control the direction of the sector's scrolling, which is usually what is
// desired.
//
// Process the active scrollers.
//
// This is the main scrolling code

typedef struct {
	thinker_t thinker;				// Thinker structure for scrolling
	int16_t __far* textureoffset;	// Affected textureoffset
} scroll_t;


static void T_Scroll(scroll_t __far* s)
{
	(*s->textureoffset)++;
}


//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// affectee: the index of the affected sidedef
//

static void Add_Scroller(int16_t affectee)
{
	scroll_t __far* s = Z_CallocLevSpec(sizeof *s);
	s->thinker.function = T_Scroll;
	s->textureoffset = &_g_sides[affectee].textureoffset;
	P_AddThinker(&s->thinker);
}


// Initialize the scrollers
static void P_SpawnScrollers(void)
{
	const line_t __far* line = _g_lines;

	for (int16_t i = 0; i < _g_numlines; i++)
	{
		if (LN_SPECIAL(line) == 48)
			Add_Scroller(line->sidenum[0]);
		line++;
	}
}
