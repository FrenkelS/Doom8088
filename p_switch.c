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
 *  Switches, buttons. Two-state animation. Exits.
 *
 *-----------------------------------------------------------------------------*/

#include "d_englsh.h"
#include "d_player.h"
#include "w_wad.h"
#include "r_main.h"
#include "p_spec.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_system.h"

#include "globdata.h"


#define MAXSWITCHES		19

static int16_t switchlist[(MAXSWITCHES + 1) * 2];
static int16_t   numswitches;

button_t  _g_buttonlist[MAXBUTTONS];


// switch animation structure type

typedef struct
{
  char name1[9];
  char name2[9];
} switchlist_t; //jff 3/23/98 pack to read from memory

typedef char assertSwitchlistSize[sizeof(switchlist_t) == 18 ? 1 : -1];

static const switchlist_t alphSwitchList[MAXSWITCHES] =
{
    // Doom episode 1 switches
    {"SW1BRCOM", "SW2BRCOM"},
    {"SW1BRN1",  "SW2BRN1" },
    {"SW1BRN2",  "SW2BRN2" },
    {"SW1BRNGN", "SW2BRNGN"},
    {"SW1BROWN", "SW2BROWN"},
    {"SW1COMM",  "SW2COMM" },
    {"SW1COMP",  "SW2COMP" },
    {"SW1DIRT",  "SW2DIRT" },
    {"SW1EXIT",  "SW2EXIT" },
    {"SW1GRAY",  "SW2GRAY" },
    {"SW1GRAY1", "SW2GRAY1"},
    {"SW1METAL", "SW2METAL"},
    {"SW1PIPE",  "SW2PIPE" },
    {"SW1SLAD",  "SW2SLAD" },
    {"SW1STARG", "SW2STARG"},
    {"SW1STON1", "SW2STON1"},
    {"SW1STON2", "SW2STON2"},
    {"SW1STONE", "SW2STONE"},
    {"SW1STRTN", "SW2STRTN"}
};


//
// P_InitSwitchList
// Only called at game initialization.
//
void P_InitSwitchList(void)
{
    int16_t		index = 0;

    for (int16_t i = 0; i < MAXSWITCHES; i++)
    {
        switchlist[index++] = R_CheckTextureNumForName(alphSwitchList[i].name1);
        switchlist[index++] = R_CheckTextureNumForName(alphSwitchList[i].name2);
    }

    numswitches = index/2;
    switchlist[index] = -1;
}


void P_LoadTexture(int16_t texture)
{
	R_GetTexture(texture);

	for (int16_t i = 0; i < numswitches * 2; i++)
	{
		if (switchlist[i] == texture)
		{
			R_GetTexture(switchlist[i ^ 1]);
			return;
		}
	}
}


//
// P_StartButton()
//
// Start a button (retriggerable switch) counting down till it turns off.
//
// Passed the linedef the button is on, which texture on the sidedef contains
// the button, the texture number of the button, and the time the button is
// to remain active in gametics.
// No return.
//
static void P_StartButton(const line_t __far* line, bwhere_e w, int16_t texture, int16_t time)
{
  int16_t           i;

  // See if button is already pressed
  for (i = 0;i < MAXBUTTONS;i++)
    if (_g_buttonlist[i].btimer && _g_buttonlist[i].line == line)
      return;

  for (i = 0;i < MAXBUTTONS;i++)
    if (!_g_buttonlist[i].btimer)    // use first unused element of list
    {
      _g_buttonlist[i].line = line;
      _g_buttonlist[i].where = w;
      _g_buttonlist[i].btexture = texture;
      _g_buttonlist[i].btimer = time;
      /* use sound origin of line itself - no need to compatibility-wrap
       * as the popout code gets it wrong whatever its value */
      _g_buttonlist[i].soundorg = &LN_FRONTSECTOR(line)->soundorg;
      return;
    }

  I_Error("P_StartButton: no button slots left!");
}

//
// P_ChangeSwitchTexture()
//
// Function that changes switch wall texture on activation.
//
// Passed the line which the switch is on, and whether its retriggerable.
// If not retriggerable, this function clears the line special to insure that
//
// No return
//

// 1 second, in ticks.
#define BUTTONTIME  TICRATE

void P_ChangeSwitchTexture(line_t __far* line, boolean useAgain)
{
    /* Rearranged a bit to avoid too much code duplication */
    int16_t     i;
    int16_t   *texture, ttop, tmid, tbot;
    bwhere_e position;

    ttop = _g_sides[line->sidenum[0]].toptexture;
    tmid = _g_sides[line->sidenum[0]].midtexture;
    tbot = _g_sides[line->sidenum[0]].bottomtexture;

    /* don't zero line->special until after exit switch test */
    if (!useAgain)
        LN_SPECIAL(line) = 0;

    /* search for a texture to change */
    texture = NULL;
    position = 0;

    for (i = 0; i < numswitches*2; i++)
    {
        if (switchlist[i] == ttop)
        {
            texture = &ttop;
            position = top;
            break;
        }
        else if (switchlist[i] == tmid)
        {
            texture = &tmid;
            position = middle;
            break;
        }
        else if (switchlist[i] == tbot)
        {
            texture = &tbot;
            position = bottom;
            break;
        }
    }

    if (texture == NULL)
        return; /* no switch texture was found to change */

    *texture = switchlist[i^1];

    switch(position)
    {
        case top:
            _g_sides[line->sidenum[0]].toptexture = *texture;
            break;

        case middle:
            _g_sides[line->sidenum[0]].midtexture = *texture;
            break;

        case bottom:
            _g_sides[line->sidenum[0]].bottomtexture = *texture;
            break;
    }

    S_StartSound2(&LN_FRONTSECTOR(line)->soundorg, sfx_swtchn);

    if (useAgain)
        P_StartButton(line, position, switchlist[i], BUTTONTIME);
}


//
// EV_VerticalDoor
//
// Handle opening a door manually, no tag value
//
// Passed the line activating the door and the thing activating it
//
static void EV_VerticalDoor(line_t __far* line, mobj_t __far* thing)
{
  player_t* player;
  sector_t __far* sec;
  vldoor_t __far* door;

  //  Check for locks
  player = P_MobjIsPlayer(thing);

  switch(LN_SPECIAL(line))
  {
    case 26: // Blue Lock
    case 32:
      if ( !player )
        return;
      if (!player->cards[it_bluecard])
      {
          player->message = PD_BLUEK;         // Ty 03/27/98 - externalized
          S_StartSound(player->mo,sfx_oof);     // killough 3/20/98
          return;
      }
      break;

    case 27: // Yellow Lock
    case 34:
      if ( !player )
          return;
      if (!player->cards[it_yellowcard])
      {
          player->message = PD_YELLOWK;       // Ty 03/27/98 - externalized
          S_StartSound(player->mo,sfx_oof);     // killough 3/20/98
          return;
      }
      break;

    case 28: // Red Lock
    case 33:
      if ( !player )
          return;
      if (!player->cards[it_redcard])
      {
          player->message = PD_REDK;          // Ty 03/27/98 - externalized
          S_StartSound(player->mo,sfx_oof);     // killough 3/20/98
          return;
      }
      break;

    default:
      break;
  }

  // if the wrong side of door is pushed, give oof sound
  if (line->sidenum[1]==NO_INDEX)                     // killough
  {
    S_StartSound(player->mo,sfx_oof);           // killough 3/20/98
    return;
  }

  // get the sector on the second side of activating linedef
  sec = _g_sides[line->sidenum[1]].sector;

  door = sec->ceilingdata;

  /* If this is a repeatable line, and the door is already moving, then we can just reverse the current action. Note that in prboom 2.3.0 I erroneously removed the if-this-is-repeatable check, hence the prboom_4_compatibility clause below (foolishly assumed that already moving implies repeatable - but it could be moving due to another switch, e.g. lv19-509) */
  if (door &&
      (
       (LN_SPECIAL(line) == 1) || (LN_SPECIAL(line) == 26) || (LN_SPECIAL(line) == 27) || (LN_SPECIAL(line) == 28)
	  )
     ) {
    /* For old demos we have to emulate the old buggy behavior and
     * mess up non-T_VerticalDoor actions.
     */
    if (door->thinker.function == T_VerticalDoor)
    {
      /* cph - we are writing outval to door->direction iff it is non-zero */
      int16_t outval = 0;

      /* An already moving repeatable door which is being re-pressed, or a
       * monster is trying to open a closing door - so change direction
       * DEMOSYNC: we only read door->direction now if it really is a door.
       */
      if (door->thinker.function == T_VerticalDoor && door->direction == -1) {
        outval = 1; /* go back up */
      } else if (player) {
        outval = -1; /* go back down */
      }

      /* Write this to the thinker. In demo compatibility mode, we might be 
       *  overwriting a field of a non-vldoor_t thinker - we need to add any 
       *  other thinker types here if any demos depend on specific fields
       *  being corrupted by this.
       */
      if (outval) {
        if (door->thinker.function == T_VerticalDoor) {
          door->direction = outval;
        } else if (door->thinker.function == T_PlatRaise) {
          plat_t __far* p = (plat_t __far*)door;
          p->wait = outval;
        } else {
          printf("EV_VerticalDoor: unknown thinker.function in thinker corruption emulation\n");
        }

        return;
      }
    }
    /* Either we're in prboom >=v2.3 and it's not a door, or it's a door but
     * we're a monster and don't want to shut it; exit with no action.
     */
    return;
  }

  // emit proper sound
  S_StartSound2(&sec->soundorg,sfx_doropn); // normal or locked door sound

  // new door thinker
  door = Z_CallocLevSpec(sizeof(*door));
  P_AddThinker (&door->thinker);
  sec->ceilingdata = door; //jff 2/22/98
  door->thinker.function = T_VerticalDoor;
  door->sector = sec;
  door->direction = 1;
  door->speed = VDOORSPEED;
  door->topwait = VDOORWAIT;
  door->line = line; // jff 1/31/98 remember line that triggered us

  /* killough 10/98: use gradual lighting changes if nonzero tag given */
  door->lighttag = line->tag;

  // set the type of door from the activating linedef type
  switch(LN_SPECIAL(line))
  {
    case 1:
    case 26:
    case 27:
    case 28:
      door->type = normal;
      break;

    case 31:
    case 32:
    case 33:
    case 34:
      door->type = dopen;
      LN_SPECIAL(line) = 0;
      break;

    default:
      door->lighttag = 0;   // killough 10/98
      break;
  }

  // find the top and bottom of the movement range
  door->topheight = P_FindLowestCeilingSurrounding(sec);
  door->topheight -= 4*FRACUNIT;
}


//
// P_UseSpecialLine
//
//
// Called when a thing uses (pushes) a special line.
// Only the front sides of lines are usable.
// Dispatches to the appropriate linedef function handler.
//
// Passed the thing using the line and the line being used
// Returns true if a thinker was created
//
boolean P_UseSpecialLine(mobj_t __far* thing, line_t __far* line)
{
  // Switches that other things can activate.
  if (!P_MobjIsPlayer(thing))
  {
    // never open secret doors
    if (line->flags & ML_SECRET)
      return false;

    switch(LN_SPECIAL(line))
    {
      case 1:         // MANUAL DOOR RAISE
      case 32:        // MANUAL BLUE
      case 33:        // MANUAL RED
      case 34:        // MANUAL YELLOW
        break;

      default:
        return false;
    }
  }

  if (!P_CheckTag(line))  //jff 2/27/98 disallow zero tag on some types
    return false;

  // Dispatch to handler according to linedef type
  switch (LN_SPECIAL(line))
  {
    // Manual doors, push type with no tag
    case 1:             // Vertical Door
    case 26:            // Blue Door/Locked
    case 27:            // Yellow Door /Locked
    case 28:            // Red Door /Locked

    case 31:            // Manual door open
    case 32:            // Blue locked door open
    case 33:            // Red locked door open
    case 34:            // Yellow locked door open
      EV_VerticalDoor (line, thing);
      break;

    // Switches (non-retriggerable)
    case 7:
      // Build Stairs
      if (EV_BuildStairs(line))
        P_ChangeSwitchTexture(line,false);
      break;

    case 9:
      // Change Donut
      if (EV_DoDonut(line))
        P_ChangeSwitchTexture(line,false);
      break;

    case 11:
      /* Exit level
       * killough 10/98: prevent zombies from exiting levels
       */
      if (P_MobjIsPlayer(thing) && P_MobjIsPlayer(thing)->health <= 0)
      {
        S_StartSound(thing, sfx_noway);
        return false;
      }

      P_ChangeSwitchTexture(line,false);
      G_ExitLevel ();
      break;

    case 18:
      // Raise Floor to next highest floor
      if (EV_DoFloor(line, raiseFloorToNearest))
        P_ChangeSwitchTexture(line,false);
      break;

    case 20:
      // Raise Plat next highest floor and change texture
      if (EV_DoPlat(line,raiseToNearestAndChange))
        P_ChangeSwitchTexture(line,false);
      break;

    case 23:
      // Lower Floor to Lowest
      if (EV_DoFloor(line,lowerFloorToLowest))
        P_ChangeSwitchTexture(line,false);
      break;

    case 51:
      /* Secret EXIT
       * killough 10/98: prevent zombies from exiting levels
       */
      if (P_MobjIsPlayer(thing) && P_MobjIsPlayer(thing)->health <= 0)
      {
        S_StartSound(thing, sfx_noway);
        return false;
      }

      P_ChangeSwitchTexture(line,false);
      G_SecretExitLevel ();
      break;

    case 103:
      // Open Door
      if (EV_DoDoor(line,dopen))
        P_ChangeSwitchTexture(line,false);
      break;


    // Buttons (retriggerable switches)
    case 62:
      // PlatDownWaitUpStay
      if (EV_DoPlat(line,downWaitUpStay))
        P_ChangeSwitchTexture(line,true);
      break;

    case 63:
      // Raise Door
      if (EV_DoDoor(line,normal))
        P_ChangeSwitchTexture(line,true);
      break;

    case 70:
      // Turbo Lower Floor
      if (EV_DoFloor(line,turboLower))
        P_ChangeSwitchTexture(line,true);
      break;
  }
  return true;
}
