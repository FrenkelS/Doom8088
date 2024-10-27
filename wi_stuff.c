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
 *  Intermission screens.
 *
 *-----------------------------------------------------------------------------
 */

#include "d_player.h"
#include "m_random.h"
#include "w_wad.h"
#include "g_game.h"
#include "r_main.h"
#include "wi_lib.h"
#include "wi_stuff.h"
#include "s_sound.h"
#include "sounds.h"

#include "globdata.h"


// used to accelerate or skip a stage
boolean   _g_acceleratestage;


// States for the intermission

typedef enum
{
  NoState = -1,
  StatCount,
  ShowNextLoc
} stateenum_t;


 // specifies current state
static stateenum_t  state;

// contains information passed into intermission
static wbstartstruct_t* wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int16_t    cnt;

// used for timing of background animation
static int16_t    bcnt;

static int32_t    cnt_time;
static int32_t    cnt_total_time;
static int16_t    cnt_par;
static int16_t    cnt_pause;


static int16_t  sp_state;

static int16_t cnt_kills;
static int16_t cnt_items;
static int16_t cnt_secret;

static boolean snl_pointeron;


static int16_t wimap0num;


//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//


//
// GENERAL DATA
//

//
// Locally used stuff.
//


// in seconds
#define SHOWNEXTLOCDELAY  4
//#define SHOWLASTLOCDELAY  SHOWNEXTLOCDELAY


//
// CODE
//

// ====================================================================
// WI_slamBackground
// Purpose: Put the full-screen background up prior to patches
// Args:    none
// Returns: void
//
static void WI_slamBackground(void)
{
	V_DrawRawFullScreen(wimap0num);
}


static int16_t WI_calculateDigits(int16_t n)
{
	if (n == 0)
	{
		// make variable-length zeros 1 digit long
		return 1;
	}
	else
	{
		// figure out # of digits in #
		int16_t digits = 0;
		int16_t temp = n;

		while (temp)
		{
			temp /= 10;
			digits++;
		}

		return digits;
	}
}


// ====================================================================
// WI_drawPercent
// Purpose: Draws a percentage, really just a call to WI_drawNum
//          after putting a percent sign out there
// Args:    x, y   -- location
//          p      -- the percentage value to be drawn, no negatives
// Returns: void
// CPhipps - static
void WI_drawPercent(int16_t x, int16_t y, int16_t p)
{
  if (p < 0)
    return;

  WI_drawPercentSign(x, y);
  WI_drawNum(x, y, p, WI_calculateDigits(p));
}


// ====================================================================
// WI_drawTime
// Purpose: Draws the level completion time or par time, or "Sucks"
//          if 1 hour or more
// Args:    x, y   -- location
//          t      -- the time value to be drawn
// Returns: void
//
// CPhipps - static
//         - largely rewritten to display hours and use slightly better algorithm

void WI_drawTime(int16_t x, int16_t y, int32_t t)
{
  if (t<0)
    return;

  if (t < 24L*60*60)
    for(;;) {
      int16_t n = t % 60;
      t /= 60;
      x = WI_drawNum(x, y, n, (t || n>9) ? 2 : 1) - WI_getColonWidth();

      // draw
      if (t)
        WI_drawColon(x, y);
      else break;
    }
  else // "sucks" (maybe should be "addicted", even I've never had a 24 hour game ;)
    WI_drawSucks(x, y);
}


// ====================================================================
// WI_End
// Purpose: Unloads data structures (inverse of WI_Start)
// Args:    none
// Returns: void
//
void WI_End(void)
{
	//Clean up coop game stats
	cnt_kills  = -1;
	cnt_secret = -1;
	cnt_items  = -1;
}


// ====================================================================
// WI_initNoState
// Purpose: Clear state, ready for end of level activity
// Args:    none
// Returns: void
//
static void WI_initNoState(void)
{
  state = NoState;
  _g_acceleratestage = false;
  cnt = 10;
}


// ====================================================================
// WI_updateNoState
// Purpose: Cycle until end of level activity is done
// Args:    none
// Returns: void
//
static void WI_updateNoState(void)
{
  if (!--cnt)
    G_WorldDone();
}

// ====================================================================
// WI_initShowNextLoc
// Purpose: Prepare to show the next level's location
// Args:    none
// Returns: void
//
static void WI_initShowNextLoc(void)
{
  if (_g_gamemap == 8) {
    G_WorldDone();
    return;
  }

  state = ShowNextLoc;
  _g_acceleratestage = false;
  
    cnt = SHOWNEXTLOCDELAY * TICRATE;
}


// ====================================================================
// WI_updateShowNextLoc
// Purpose: Prepare to show the next level's location
// Args:    none
// Returns: void
//
static void WI_updateShowNextLoc(void)
{
  if (!--cnt || _g_acceleratestage)
    WI_initNoState();
  else
    snl_pointeron = (cnt & 31) < 20;
}


// ====================================================================
// WI_drawShowNextLoc
// Purpose: Show the next level's location on animated backgrounds
// Args:    none
// Returns: void
//
static void WI_drawShowNextLoc(void)
{
    WI_slamBackground();

    int16_t last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

    // draw a splat on taken cities.
    for (int16_t i=0 ; i<=last ; i++)
        WI_drawSplat(i);

    // splat the secret level?
    if (wbs->didsecret)
        WI_drawSplat(8);

    // draw flashing ptr
    if (snl_pointeron)
        WI_drawYouAreHere(wbs->next);

    // draws which level you are entering..
    WI_drawEL(wbs->next);
}

// ====================================================================
// WI_drawNoState
// Purpose: Draw the pointer and next location
// Args:    none
// Returns: void
//
static void WI_drawNoState(void)
{
  snl_pointeron = true;
  WI_drawShowNextLoc();
}

// ====================================================================
// WI_initStats
// Purpose: Get ready for single player stats
// Args:    none
// Returns: void
// Comment: Seems like we could do all these stats in a more generic
//          set of routines that weren't duplicated for dm, coop, sp
//



static void WI_initStats(void)
{
  state = StatCount;
  _g_acceleratestage = false;
  sp_state = 1;

  cnt_kills = -1;
  cnt_secret = -1;
  cnt_items = -1;

  cnt_time = cnt_par = cnt_total_time = -1;
  cnt_pause = TICRATE;
}

// ====================================================================
// WI_updateStats
// Purpose: Calculate solo stats
// Args:    none
// Returns: void
//
static void WI_updateStats(void)
{
  if (_g_acceleratestage && sp_state != 10)
  {
    _g_acceleratestage = false;
    cnt_kills = (plrs[0].skills * 100) / wbs->maxkills;
    cnt_items = (plrs[0].sitems * 100) / wbs->maxitems;

    // killough 2/22/98: Make secrets = 100% if maxsecret = 0:
    cnt_secret = (wbs->maxsecret ?
      (plrs[0].ssecret * 100) / wbs->maxsecret : 100);

    cnt_total_time = wbs->totaltimes / TICRATE;
    cnt_time = plrs[0].stime / TICRATE;
    cnt_par = wbs->partime / TICRATE;
    S_StartSound(0, sfx_barexp);
    sp_state = 10;
  }

  if (sp_state == 2)
  {
    cnt_kills += 2;

    if (!(bcnt&3))
      S_StartSound(0, sfx_pistol);

    if (cnt_kills >= (plrs[0].skills * 100) / wbs->maxkills)
    {
      cnt_kills = (plrs[0].skills * 100) / wbs->maxkills;
      S_StartSound(0, sfx_barexp);
      sp_state++;
    }
  }
  else if (sp_state == 4)
  {
    cnt_items += 2;

    if (!(bcnt&3))
      S_StartSound(0, sfx_pistol);

    if (cnt_items >= (plrs[0].sitems * 100) / wbs->maxitems)
    {
      cnt_items = (plrs[0].sitems * 100) / wbs->maxitems;
      S_StartSound(0, sfx_barexp);
      sp_state++;
    }
  }
  else if (sp_state == 6)
  {
    cnt_secret += 2;

    if (!(bcnt&3))
      S_StartSound(0, sfx_pistol);

    // killough 2/22/98: Make secrets = 100% if maxsecret = 0:
    if (cnt_secret >= (wbs->maxsecret ? (plrs[0].ssecret * 100) / wbs->maxsecret : 100))
    {
      cnt_secret = (wbs->maxsecret ?
        (plrs[0].ssecret * 100) / wbs->maxsecret : 100);
      S_StartSound(0, sfx_barexp);
      sp_state++;
    }
  }
  else if (sp_state == 8)
  {
    if (!(bcnt&3))
      S_StartSound(0, sfx_pistol);

    cnt_time += 3;

    if (cnt_time >= plrs[0].stime / TICRATE)
      cnt_time = plrs[0].stime / TICRATE;

    cnt_total_time += 3;

    if (cnt_total_time >= wbs->totaltimes / TICRATE)
      cnt_total_time = wbs->totaltimes / TICRATE;

    cnt_par += 3;

    if (cnt_par >= wbs->partime / TICRATE)
    {
      cnt_par = wbs->partime / TICRATE;

      if ((cnt_time >= plrs[0].stime / TICRATE) && (cnt_total_time >= wbs->totaltimes / TICRATE))
      {
        S_StartSound(0, sfx_barexp);
        sp_state++;
      }
    }
  }
  else if (sp_state == 10)
  {
    if (_g_acceleratestage)
    {
      S_StartSound(0, sfx_sgcock);

      WI_initShowNextLoc();
    }
  }
  else if (sp_state & 1)
  {
    if (!--cnt_pause)
    {
      sp_state++;
      cnt_pause = TICRATE;
    }
  }
}


// ====================================================================
// WI_checkForAccelerate
// Purpose: See if the player has hit either the attack or use key
//          or mouse button.  If so we set acceleratestage to true and
//          all those display routines above jump right to the end.
// Args:    none
// Returns: void
//
void WI_checkForAccelerate(void)
{
    player_t  *player = &_g_player;

    if (player->cmd.buttons & BT_ATTACK)
    {
        if (!player->attackdown)
            _g_acceleratestage = true;
        player->attackdown = true;
    }
    else
        player->attackdown = false;

    if (player->cmd.buttons & BT_USE)
    {
        if (!player->usedown)
            _g_acceleratestage = true;
        player->usedown = true;
    }
    else
        player->usedown = false;
}

// ====================================================================
// WI_Ticker
// Purpose: Do various updates every gametic, for stats, animation,
//          checking that intermission music is running, etc.
// Args:    none
// Returns: void
//
void WI_Ticker(void)
{
  // counter for general background animation
  bcnt++;

  if (bcnt == 1)
  {
    // intermission music
    S_ChangeMusic(mus_inter, true);
  }

  WI_checkForAccelerate();

  switch (state)
  {
    case StatCount:
         WI_updateStats();
         break;

    case ShowNextLoc:
         WI_updateShowNextLoc();
         break;

    case NoState:
         WI_updateNoState();
         break;
  }
}


// ====================================================================
// WI_Drawer
// Purpose: Call the appropriate stats drawing routine depending on
//          what kind of game is being played (DM, coop, solo)
// Args:    none
// Returns: void
//
void WI_Drawer (void)
{
  switch (state)
  {
    case StatCount:
           WI_slamBackground();
           WI_drawLF(wbs->last);
           WI_drawStats(cnt_kills, cnt_items, cnt_secret, cnt_time, cnt_total_time, cnt_par);
         break;

    case ShowNextLoc:
         WI_drawShowNextLoc();
         break;

    case NoState:
         WI_drawNoState();
         break;
  }
}


// ====================================================================
// WI_initVariables
// Purpose: Initialize the intermission information structure
//          Note: wbstartstruct_t is defined in d_player.h
// Args:    wbstartstruct -- pointer to the structure with the data
// Returns: void
//
static void WI_initVariables(wbstartstruct_t* wbstartstruct)
{

  wbs = wbstartstruct;

  _g_acceleratestage = false;
  cnt = bcnt = 0;
  plrs = wbs->plyr;

  if (!wbs->maxkills)
    wbs->maxkills = 1;  // probably only useful in MAP30

  if (!wbs->maxitems)
    wbs->maxitems = 1;
}

// ====================================================================
// WI_Start
// Purpose: Call the various init routines
//          Note: wbstartstruct_t is defined in d_player.h
// Args:    wbstartstruct -- pointer to the structure with the
//          intermission data
// Returns: void
//
void WI_Start(wbstartstruct_t* wbstartstruct)
{
	WI_initVariables(wbstartstruct);
	WI_initStats();
}


void WI_Init(void)
{
	wimap0num = W_GetNumForName("WIMAP0");
}

