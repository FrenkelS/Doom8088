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
	P_InitAnimatedFlat();

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
// P_FindNextLowestFloor()
//
// Passed a sector and a floor height, returns the fixed point value
// of the largest floor height in a surrounding sector smaller than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
//
fixed_t P_FindNextLowestFloor(sector_t __far* sec, fixed_t currentheight)
{
  sector_t __far* other;
  int16_t i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight < currentheight)
    {
      fixed_t height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight > height &&
            other->floorheight < currentheight)
          height = other->floorheight;
      return height;
    }
  return currentheight;
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
// P_FindHighestCeilingSurrounding()
//
// Passed a sector, returns the fixed point value of the largest
// ceiling height in the surrounding sectors, not including that passed
//
fixed_t P_FindHighestCeilingSurrounding(sector_t __far* sec)
{
  int16_t             i;
  const line_t __far* check;
  sector_t __far*       other;
  fixed_t height = -32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->ceilingheight > height)
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
// P_CanUnlockGenDoor()
//
// Passed a generalized locked door linedef and a player, returns whether
// the player has the keys necessary to unlock that door.
//
// Note: The linedef passed MUST be a generalized locked door type
//       or results are undefined.
//
// jff 02/05/98 routine added to test for unlockability of
//  generalized locked doors
//
boolean P_CanUnlockGenDoor(const line_t __far* line, player_t* player)
{
  // does this line special distinguish between skulls and keys?
  int32_t skulliscard = (LN_SPECIAL(line) & LockedNKeys)>>LockedNKeysShift;

  // determine for each case of lock type if player's keys are adequate
  switch((LN_SPECIAL(line) & LockedKey)>>LockedKeyShift)
  {
    case AnyKey:
      if
      (
        !player->cards[it_redcard] &&
        !player->cards[it_redskull] &&
        !player->cards[it_bluecard] &&
        !player->cards[it_blueskull] &&
        !player->cards[it_yellowcard] &&
        !player->cards[it_yellowskull]
      )
      {
        player->message = PD_ANY; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case RCard:
      if
      (
        !player->cards[it_redcard] &&
        (!skulliscard || !player->cards[it_redskull])
      )
      {
        player->message = skulliscard? PD_REDK : PD_REDC; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case BCard:
      if
      (
        !player->cards[it_bluecard] &&
        (!skulliscard || !player->cards[it_blueskull])
      )
      {
        player->message = skulliscard? PD_BLUEK : PD_BLUEC; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case YCard:
      if
      (
        !player->cards[it_yellowcard] &&
        (!skulliscard || !player->cards[it_yellowskull])
      )
      {
        player->message = skulliscard? PD_YELLOWK : PD_YELLOWC; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case RSkull:
      if
      (
        !player->cards[it_redskull] &&
        (!skulliscard || !player->cards[it_redcard])
      )
      {
        player->message = skulliscard? PD_REDK : PD_REDS; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case BSkull:
      if
      (
        !player->cards[it_blueskull] &&
        (!skulliscard || !player->cards[it_bluecard])
      )
      {
        player->message = skulliscard? PD_BLUEK : PD_BLUES; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case YSkull:
      if
      (
        !player->cards[it_yellowskull] &&
        (!skulliscard || !player->cards[it_yellowcard])
      )
      {
        player->message = skulliscard? PD_YELLOWK : PD_YELLOWS; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
    case AllKeys:
      if
      (
        !skulliscard &&
        (
          !player->cards[it_redcard] ||
          !player->cards[it_redskull] ||
          !player->cards[it_bluecard] ||
          !player->cards[it_blueskull] ||
          !player->cards[it_yellowcard] ||
          !player->cards[it_yellowskull]
        )
      )
      {
        player->message = PD_ALL6; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      if
      (
        skulliscard &&
        (
          (!player->cards[it_redcard] &&
            !player->cards[it_redskull]) ||
          (!player->cards[it_bluecard] &&
            !player->cards[it_blueskull]) ||
          (!player->cards[it_yellowcard] &&
            !player->cards[it_yellowskull])
        )
      )
      {
        player->message = PD_ALL3; // Ty 03/27/98 - externalized
        S_StartSound(player->mo,sfx_oof);             // killough 3/20/98
        return false;
      }
      break;
  }
  return true;
}


//
// P_SectorActive()
//
// Passed a linedef special class (floor, ceiling, lighting) and a sector
// returns whether the sector is already busy with a linedef special of the
// same class. If old demo compatibility true, all linedef special classes
// are the same.
//
// jff 2/23/98 added to prevent old demos from
//  succeeding in starting multiple specials on one sector
//
boolean PUREFUNC P_SectorActive(special_e t, const sector_t __far* sec)
{
    switch (t)             // return whether thinker of same type is active
    {
      case floor_special:
        return sec->floordata != NULL;
      case ceiling_special:
        return sec->ceilingdata != NULL;
      case lighting_special:
        return false;
    }
  return true; // don't know which special, must be active, shouldn't be here
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
    case 117:
    case 118:

    case 139:               // Lighting specials
    case 170:
    case 79:
    case 35:
    case 138:
    case 171:
    case 81:
    case 13:
    case 192:
    case 169:
    case 80:
    case 12:
    case 194:
    case 173:
    case 157:
    case 104:
    case 193:
    case 172:
    case 156:
    case 17:

    case 195:               // Thing teleporters
    case 174:
    case 97:
    case 39:
    case 126:
    case 125:
    case 210:
    case 209:
    case 208:
    case 207:

    case 11:                // Exits
    case 52:
    case 197:
    case 51:
    case 124:
    case 198:

    case 48:                // Scrolling walls
    case 85:
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
	uint16_t t = _g_leveltime >> 3;

	int16_t pic = animated_texture_basepic + (t % 3);

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

// Parses command line parameters.
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

    if (sector->special&SECRET_MASK) //jff 3/15/98 count extended
      _g_totalsecret++;                 // secret sectors too

    switch (sector->special&31)
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

      case 4:
        // strobe fast/death slime
        P_SpawnStrobeFlash(sector,FASTDARK,false);
        sector->special |= 3<<DAMAGE_SHIFT; //jff 3/14/98 put damage bits in
        break;

      case 8:
        // glowing light
        P_SpawnGlowingLight(sector);
        break;
      case 9:
        // secret sector
        if (sector->special<32) //jff 3/14/98 bits don't count unless not
          _g_totalsecret++;        // a generalized sector type
        break;

      case 10:
        // door close in 30 seconds
        P_SpawnDoorCloseIn30 (sector);
        break;

      case 12:
        // sync strobe slow
        P_SpawnStrobeFlash (sector, SLOWDARK, true);
        break;

      case 13:
        // sync strobe fast
        P_SpawnStrobeFlash (sector, FASTDARK, true);
        break;

      case 14:
        // door raise in 5 minutes
        P_SpawnDoorRaiseIn5Mins (sector);
        break;

      case 17:
        // fire flickering
        P_SpawnFireFlicker(sector);
        break;
    }
  }

  P_RemoveAllActiveCeilings();  // jff 2/22/98 use killough's scheme

  P_RemoveAllActivePlats();     // killough

  for (i = 0;i < MAXBUTTONS;i++)
    memset(&_g_buttonlist[i],0,sizeof(button_t));

  P_SpawnScrollers(); // killough 3/7/98: Add generalized scrollers
}

// killough 2/28/98:
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
// killough 3/7/98

static void T_Scroll(scroll_t __far* s)
{
    side_t __far* side  =_g_sides + s->affectee;
    side->textureoffset++;
}

//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// type: the enumerated type of scrolling: floor, ceiling, floor carrier,
//   wall, floor carrier & scroller
//
// (dx,dy): the direction and speed of the scrolling or its acceleration
//
// control: the sector whose heights control this scroller's effect
//   remotely, or -1 if no control sector
//
// affectee: the index of the affected object (sector or sidedef)
//
// accel: non-zero if this is an accelerative effect
//

static void Add_Scroller(int16_t affectee)
{
  scroll_t __far* s = Z_CallocLevSpec(sizeof *s);
  s->thinker.function = T_Scroll;
  s->affectee = affectee;
  P_AddThinker(&s->thinker);
}

// Initialize the scrollers
static void P_SpawnScrollers(void)
{
    int16_t i;
    const line_t __far* l = _g_lines;

    for (i=0;i<_g_numlines;i++,l++)
        if (LN_SPECIAL(l) == 48)
            Add_Scroller(_g_lines[i].sidenum[0]);
}

