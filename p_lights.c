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
 *  Action routines for lighting thinkers
 *  Spawn sector based lighting effects.
 *  Handle lighting linedef types
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "doomdef.h"
#include "m_random.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"

#include "globdata.h"

//////////////////////////////////////////////////////////
//
// Lighting action routines, called once per tick
//
//////////////////////////////////////////////////////////

//
// T_FireFlicker()
//
// Firelight flicker action routine, called once per tick
//
// Passed a fireflicker_t structure containing light levels and timing
// Returns nothing
//
static void T_FireFlicker (fireflicker_t __far* flick)
{
  int32_t amount;

  if (--flick->count)
    return;

  amount = (P_Random()&3)*16;

  if (flick->sector->lightlevel - amount < flick->minlight)
    flick->sector->lightlevel = flick->minlight;
  else
    flick->sector->lightlevel = flick->maxlight - amount;

  flick->count = 4;
}

//
// T_LightFlash()
//
// Broken light flashing action routine, called once per tick
//
// Passed a lightflash_t structure containing light levels and timing
// Returns nothing
//

typedef struct
{
  thinker_t thinker;
  sector_t __far* sector;
  int32_t count;
  int32_t maxlight;
  int32_t minlight;
  int32_t maxtime;
  int32_t mintime;

} lightflash_t;

static void T_LightFlash (lightflash_t __far* flash)
{
  if (--flash->count)
    return;

  if (flash->sector->lightlevel == flash->maxlight)
  {
    flash-> sector->lightlevel = flash->minlight;
    flash->count = (P_Random()&flash->mintime)+1;
  }
  else
  {
    flash-> sector->lightlevel = flash->maxlight;
    flash->count = (P_Random()&flash->maxtime)+1;
  }

}

//
// T_StrobeFlash()
//
// Strobe light flashing action routine, called once per tick
//
// Passed a strobe_t structure containing light levels and timing
// Returns nothing
//

typedef struct
{
  thinker_t thinker;
  sector_t __far* sector;
  int16_t count;
  int32_t minlight;
  int32_t maxlight;
  int16_t darktime;
  int16_t brighttime;

} strobe_t;

static void T_StrobeFlash (strobe_t __far*   flash)
{
  if (--flash->count)
    return;

  if (flash->sector->lightlevel == flash->minlight)
  {
    flash-> sector->lightlevel = flash->maxlight;
    flash->count = flash->brighttime;
  }
  else
  {
    flash-> sector->lightlevel = flash->minlight;
    flash->count =flash->darktime;
  }
}

//
// T_Glow()
//
// Glowing light action routine, called once per tick
//
// Passed a glow_t structure containing light levels and timing
// Returns nothing
//

static void T_Glow(glow_t __far* g)
{
  switch(g->direction)
  {
    case -1:
      // light dims
      g->sector->lightlevel -= GLOWSPEED;
      if (g->sector->lightlevel <= g->minlight)
      {
        g->sector->lightlevel += GLOWSPEED;
        g->direction = 1;
      }
      break;

    case 1:
      // light brightens
      g->sector->lightlevel += GLOWSPEED;
      if (g->sector->lightlevel >= g->maxlight)
      {
        g->sector->lightlevel -= GLOWSPEED;
        g->direction = -1;
      }
      break;
  }
}

//////////////////////////////////////////////////////////
//
// Sector lighting type spawners
//
// After the map has been loaded, each sector is scanned
// for specials that spawn thinkers
//
//////////////////////////////////////////////////////////

//
// P_SpawnFireFlicker()
//
// Spawns a fire flicker lighting thinker
//
// Passed the sector that spawned the thinker
// Returns nothing
//
void P_SpawnFireFlicker(sector_t __far* sector)
{
  fireflicker_t __far*  flick;

  // Note that we are resetting sector attributes.
  // Nothing special about it during gameplay.
  sector->special &= ~31; //jff 3/14/98 clear non-generalized sector type

  flick = Z_CallocLevSpec( sizeof(*flick));

  P_AddThinker (&flick->thinker);

  flick->thinker.function = T_FireFlicker;
  flick->sector = sector;
  flick->maxlight = sector->lightlevel;
  flick->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel)+16;
  flick->count = 4;
}

//
// P_SpawnLightFlash()
//
// Spawns a broken light flash lighting thinker
//
// Passed the sector that spawned the thinker
// Returns nothing
//
void P_SpawnLightFlash (sector_t __far* sector)
{
  lightflash_t __far* flash;

  // nothing special about it during gameplay
  sector->special &= ~31; //jff 3/14/98 clear non-generalized sector type

  flash = Z_CallocLevSpec(sizeof(*flash));

  P_AddThinker (&flash->thinker);

  flash->thinker.function = T_LightFlash;
  flash->sector = sector;
  flash->maxlight = sector->lightlevel;

  flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
  flash->maxtime = 64;
  flash->mintime = 7;
  flash->count = (P_Random()&flash->maxtime)+1;
}

//
// P_SpawnStrobeFlash
//
// Spawns a blinking light thinker
//
// Passed the sector that spawned the thinker, speed of blinking
// and whether blinking is to by syncrhonous with other sectors
//
// Returns nothing
//
void P_SpawnStrobeFlash(sector_t __far* sector, int16_t fastOrSlow, boolean inSync)
{
  strobe_t __far* flash;

  flash = Z_CallocLevSpec(sizeof(*flash));

  P_AddThinker (&flash->thinker);

  flash->sector = sector;
  flash->darktime = fastOrSlow;
  flash->brighttime = STROBEBRIGHT;
  flash->thinker.function = T_StrobeFlash;
  flash->maxlight = sector->lightlevel;
  flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

  if (flash->minlight == flash->maxlight)
    flash->minlight = 0;

  // nothing special about it during gameplay
  sector->special &= ~31; //jff 3/14/98 clear non-generalized sector type

  if (!inSync)
    flash->count = (P_Random()&7)+1;
  else
    flash->count = 1;
}

//
// P_SpawnGlowingLight()
//
// Spawns a glowing light (smooth oscillation from min to max) thinker
//
// Passed the sector that spawned the thinker
// Returns nothing
//
void P_SpawnGlowingLight(sector_t __far* sector)
{
  glow_t __far* g;

  g = Z_CallocLevSpec(sizeof(*g));

  P_AddThinker(&g->thinker);

  g->sector = sector;
  g->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
  g->maxlight = sector->lightlevel;
  g->thinker.function = T_Glow;
  g->direction = -1;

  sector->special &= ~31; //jff 3/14/98 clear non-generalized sector type
}

//////////////////////////////////////////////////////////
//
// Linedef lighting function handlers
//
//////////////////////////////////////////////////////////

//
// EV_StartLightStrobing()
//
// Start strobing lights (usually from a trigger)
//
// Passed the line that activated the strobing
//
void EV_StartLightStrobing(const line_t __far* line)
{
  int16_t   secnum;
  sector_t __far* sec;

  secnum = -1;
  // start lights strobing in all sectors tagged same as line
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];
    // if already doing a lighting function, don't start a second
    if (P_SectorActive(lighting_special,sec)) //jff 2/22/98
      continue;

    P_SpawnStrobeFlash (sec,SLOWDARK, false);
  }
}

//
// EV_TurnTagLightsOff()
//
// Turn line's tagged sector's lights to min adjacent neighbor level
//
// Passed the line that activated the lights being turned off
//
void EV_TurnTagLightsOff(const line_t __far* line)
{
  int16_t j;

  // search sectors for those with same tag as activating line

  // killough 10/98: replaced inefficient search with fast search
  for (j = -1; (j = P_FindSectorFromLineTag(line,j)) >= 0;)
    {
      sector_t __far* sector = _g_sectors + j;
      sector_t __far* tsec;
      int16_t i, min = sector->lightlevel;
      // find min neighbor light level
      for (i = 0;i < sector->linecount; i++)
  if ((tsec = getNextSector(sector->lines[i], sector)) &&
      tsec->lightlevel < min)
    min = tsec->lightlevel;
      sector->lightlevel = min;
    }
}

//
// EV_LightTurnOn()
//
// Turn sectors tagged to line lights on to specified or max neighbor level
//
// Passed the activating line, and a level to set the light to
// If level passed is 0, the maximum neighbor lighting is used
//
void EV_LightTurnOn(const line_t __far* line, int32_t bright)
{
  int16_t i;

  // search all sectors for ones with same tag as activating line

  // killough 10/98: replace inefficient search with fast search
  for (i = -1; (i = P_FindSectorFromLineTag(line,i)) >= 0;)
    {
      sector_t __far* temp;
      sector_t __far* sector = _g_sectors+i;
      int16_t j, tbright = bright; //jff 5/17/98 search for maximum PER sector

      // bright = 0 means to search for highest light level surrounding sector

      if (!bright)
  for (j = 0;j < sector->linecount; j++)
    if ((temp = getNextSector(sector->lines[j],sector)) &&
        temp->lightlevel > tbright)
      tbright = temp->lightlevel;

      sector->lightlevel = tbright;
    }
}

/* killough 10/98:
 *
 * EV_LightTurnOnPartway()
 *
 * Turn sectors tagged to line lights on to specified or max neighbor level
 *
 * Passed the activating line, and a light level fraction between 0 and 1.
 * Sets the light to min on 0, max on 1, and interpolates in-between.
 * Used for doors with gradual lighting effects.
 *
 */

void EV_LightTurnOnPartway(const line_t __far* line, fixed_t level)
{
  int16_t i;

  if (level < 0)          // clip at extremes
    level = 0;
  if (level > FRACUNIT)
    level = FRACUNIT;

  // search all sectors for ones with same tag as activating line
  for (i = -1; (i = P_FindSectorFromLineTag(line,i)) >= 0;)
    {
      sector_t __far* temp;
      sector_t __far* sector = _g_sectors+i;
      int16_t j, bright = 0, min = sector->lightlevel;

      for (j = 0; j < sector->linecount; j++)
  if ((temp = getNextSector(sector->lines[j],sector)))
    {
      if (temp->lightlevel > bright)
        bright = temp->lightlevel;
      if (temp->lightlevel < min)
        min = temp->lightlevel;
    }

      sector->lightlevel =   // Set level in-between extremes
  (level * bright + (FRACUNIT-level) * min) >> FRACBITS;
    }
}

