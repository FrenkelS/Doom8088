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
 *  Intermission screens.
 *
 *-----------------------------------------------------------------------------
 */

#include "doomstat.h"
#include "m_random.h"
#include "w_wad.h"
#include "g_game.h"
#include "r_main.h"
#include "wi_stuff.h"
#include "s_sound.h"
#include "sounds.h"

#include "globdata.h"

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//


// GLOBAL LOCATIONS
#define WI_TITLEY      2

// SINGLE-PLAYER STUFF
#define SP_STATSX     50
#define SP_STATSY     50

#define SP_TIMEX      8
// proff/nicolas 09/20/98 -- changed for hi-res
#define SP_TIMEY      160
//#define SP_TIMEY      (SCREENHEIGHT-32)


typedef struct
{
  int16_t   x;       // x/y coordinate pair structure
  int16_t   y;
} point_t;

#define NUMMAPS     9

static const point_t lnodes[NUMMAPS] =
{
    { 185, 164 }, // location of level 0 (CJ)
    { 148, 143 }, // location of level 1 (CJ)
    {  69, 122 }, // location of level 2 (CJ)
    { 209, 102 }, // location of level 3 (CJ)
    { 116,  89 }, // location of level 4 (CJ)
    { 166,  55 }, // location of level 5 (CJ)
    {  71,  56 }, // location of level 6 (CJ)
    { 135,  29 }, // location of level 7 (CJ)
    {  71,  24 }  // location of level 8 (CJ)
};




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
//  GRAPHICS
//

// You Are Here graphic
static const char* const yah[2] = { "WIURH0", "WIURH1" };

// splat
static const char* const splat = "WISPLAT";

// %, : graphics
static const char percent[] = {"WIPCNT"};
static const char colon[] = {"WICOLON"};



// minus sign
static const char wiminus[] = {"WIMINUS"};

// "Finished!" graphics
static const char finished[] = {"WIF"};

// "Entering" graphic
static const char entering[] = {"WIENTER"};

// "secret"
static const char sp_secret[] = {"WISCRT2"};

// "Kills", "Scrt", "Items", "Frags"
static const char kills[] = {"WIOSTK"};
static const char items[] = {"WIOSTI"};

// Time sucks.
static const char time1[] = {"WITIME"};
static const char par[] = {"WIPAR"};
static const char sucks[] = {"WISUCKS"};

// "Total", your face, your dead face
static const char total[] = {"WIMSTT"};


//
// CODE
//


/* cph -
 * Functions to return width & height of a patch.
 * Doesn't really belong here, but is often used in conjunction with
 * this code.
 */
int16_t V_NumPatchWidth(int16_t num)
{
	const patch_t* patch = W_GetLumpByNum(num);
	int16_t width = patch->width;
	Z_Free(patch);
	return width;
}

static int16_t V_NamePatchWidth(const char *name)
{
	const patch_t* patch = W_GetLumpByName(name);
	int16_t width = patch->width;
	Z_Free(patch);
	return width;
}

static int16_t V_NamePatchHeight(const char *name)
{
	const patch_t* patch = W_GetLumpByName(name);
	int16_t height = patch->height;
	Z_Free(patch);
	return height;
}


/* ====================================================================
 * WI_levelNameLump
 * Purpore: Returns the name of the graphic lump containing the name of
 *          the given level.
 * Args:    Level, and buffer (must by 9 chars) to write to
 * Returns: void
 */
static void WI_levelNameLump(int32_t map, char* buf)
{
  sprintf(buf, "WILV0%d", map);
}

// ====================================================================
// WI_slamBackground
// Purpose: Put the full-screen background up prior to patches
// Args:    none
// Returns: void
//
static void WI_slamBackground(void)
{
  // background
  W_ReadLumpByName("WIMAP0", _g->screen);
}


// ====================================================================
// WI_drawLF
// Purpose: Draw the "Finished" level name before showing stats
// Args:    none
// Returns: void
//
static void WI_drawLF(void)
{
  int32_t y = WI_TITLEY;
  char lname[9];

  // draw <LevelName>
  /* cph - get the graphic lump name and use it */
  WI_levelNameLump(_g->wbs->last, lname);
  // CPhipps - patch drawing updated
  V_DrawNamePatch((320 - V_NamePatchWidth(lname))/2, y, lname);

  // draw "Finished!"
  y += (5*V_NamePatchHeight(lname))/4;

  // CPhipps - patch drawing updated
  V_DrawNamePatch((320 - V_NamePatchWidth(finished))/2, y, finished);
}


// ====================================================================
// WI_drawEL
// Purpose: Draw introductory "Entering" and level name
// Args:    none
// Returns: void
//
static void WI_drawEL(void)
{
  int32_t y = WI_TITLEY;
  char lname[9];

  /* cph - get the graphic lump name */
  WI_levelNameLump(_g->wbs->next, lname);

  // draw "Entering"
  // CPhipps - patch drawing updated
  V_DrawNamePatch((320 - V_NamePatchWidth(entering))/2, y, entering);

  // draw level
  y += (5*V_NamePatchHeight(lname))/4;

  // CPhipps - patch drawing updated
  V_DrawNamePatch((320 - V_NamePatchWidth(lname))/2, y, lname);
}


/* ====================================================================
 * WI_drawOnLnode
 * Purpose: Draw patches at a location based on episode/map
 * Args:    n   -- index to map# within episode
 *          c[] -- array of names of patches to be drawn
 * Returns: void
 */
static void WI_drawOnLnode(int8_t n, const char* const c[])
{
	int8_t   i;
	boolean fits = false;

	i = 0;
	do
	{
		int16_t            left;
		int16_t            top;
		int16_t            right;
		int16_t            bottom;
		const patch_t* patch = W_GetLumpByName(c[i]);

		left = lnodes[n].x - patch->leftoffset;
		top = lnodes[n].y - patch->topoffset;
		right = left + patch->width;
		bottom = top + patch->height;
		Z_Free(patch);

		if (0 <= left && right < 320 && 0 <= top && bottom < 200)
		{
			fits = true;
		}
		else
		{
			i++;
		}
	} while (!fits && i != 2);

	if (fits && i < 2)
	{
		// CPhipps - patch drawing updated
		V_DrawNamePatch(lnodes[n].x, lnodes[n].y, c[i]);
	}
	else
	{
		// DEBUG
		printf("Could not place patch on level %ld\n", n + 1);
	}
}


// ====================================================================
// WI_drawNum
// Purpose: Draws a number.  If digits > 0, then use that many digits
//          minimum, otherwise only use as many as necessary
// Args:    x, y   -- location
//          n      -- the number to be drawn
//          digits -- number of digits minimum or zero
// Returns: new x position after drawing (note we are going to the left)
// CPhipps - static

//fontwidth = num[0]->width;
#define fontwidth 11

static int32_t WI_drawNum (int32_t x, int32_t y, int32_t n, int32_t digits)
{
	boolean   neg;
	int32_t   temp;
	char      name[9];  // limited to 8 characters

	if (digits < 0)
	{
		if (!n)
		{
			// make variable-length zeros 1 digit long
			digits = 1;
		}
		else
		{
			// figure out # of digits in #
			digits = 0;
			temp = n;

			while (temp)
			{
				temp /= 10;
				digits++;
			}
		}
	}

	neg = n < 0;
	if (neg)
		n = -n;

	// if non-number, do not draw it
	if (n == 1994)
		return 0;

	// draw the new number
	while (digits--)
	{
		x -= fontwidth;
		// CPhipps - patch drawing updated
		sprintf(name, "WINUM%d", n % 10);
		V_DrawNamePatch(x, y, name);
		n /= 10;
	}

	// draw a minus sign if necessary
	if (neg)
		// CPhipps - patch drawing updated
		V_DrawNamePatch(x-=8, y, wiminus);

	return x;
}


// ====================================================================
// WI_drawPercent
// Purpose: Draws a percentage, really just a call to WI_drawNum
//          after putting a percent sign out there
// Args:    x, y   -- location
//          p      -- the percentage value to be drawn, no negatives
// Returns: void
// CPhipps - static
static void WI_drawPercent(int32_t x, int32_t y, int32_t p)
{
  if (p < 0)
    return;

  // CPhipps - patch drawing updated
  V_DrawNamePatch(x, y, percent);
  WI_drawNum(x, y, p, -1);
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

static void WI_drawTime(int32_t x, int32_t y, int32_t t)
{
  int32_t   n;

  if (t<0)
    return;

  if (t < 100*60*60)
    for(;;) {
      n = t % 60;
      t /= 60;
      x = WI_drawNum(x, y, n, (t || n>9) ? 2 : 1) - V_NamePatchWidth(colon);

      // draw
      if (t)
  // CPhipps - patch drawing updated
        V_DrawNamePatch(x, y, colon);
      else break;
    }
  else // "sucks" (maybe should be "addicted", even I've never had a 100 hour game ;)
    V_DrawNamePatch(x - V_NamePatchWidth(sucks), y, sucks);
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
	_g->cnt_kills  = -1;
	_g->cnt_secret = -1;
	_g->cnt_items  = -1;
}


// ====================================================================
// WI_initNoState
// Purpose: Clear state, ready for end of level activity
// Args:    none
// Returns: void
//
static void WI_initNoState(void)
{
  _g->state = NoState;
  _g->acceleratestage = false;
  _g->cnt = 10;
}


// ====================================================================
// WI_drawTimeStats
// Purpose: Put the times on the screen
// Args:    time, total time, par time, in seconds
// Returns: void
//
// cph - pulled from WI_drawStats below

static void WI_drawTimeStats(int32_t cnt_time, int32_t cnt_total_time, int32_t cnt_par)
{
  V_DrawNamePatch(SP_TIMEX, SP_TIMEY, time1);
  WI_drawTime(320/2 - SP_TIMEX, SP_TIMEY, cnt_time);

  V_DrawNamePatch(SP_TIMEX, (SP_TIMEY+200)/2, total);
  WI_drawTime(320/2 - SP_TIMEX, (SP_TIMEY+200)/2, cnt_total_time);

  // Ty 04/11/98: redid logic: should skip only if with pwad but
  // without deh patch
  // killough 2/22/98: skip drawing par times on pwads
  // Ty 03/17/98: unless pars changed with deh patch

  V_DrawNamePatch(320/2 + SP_TIMEX, SP_TIMEY, par);
  WI_drawTime(320 - SP_TIMEX, SP_TIMEY, cnt_par);
}

// ====================================================================
// WI_updateNoState
// Purpose: Cycle until end of level activity is done
// Args:    none
// Returns: void
//
static void WI_updateNoState(void)
{
  if (!--_g->cnt)
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
  if (_g->gamemap == 8) {
    G_WorldDone();
    return;
  }

  _g->state = ShowNextLoc;
  _g->acceleratestage = false;
  
  // e6y: That was pretty easy - only a HEX editor and luck
  // There is no more desync on ddt-tas.zip\e4tux231.lmp
  // --------- tasdoom.idb ---------
  // .text:00031194 loc_31194:      ; CODE XREF: WI_updateStats+3A9j
  // .text:00031194                 mov     ds:state, 1
  // .text:0003119E                 mov     ds:acceleratestage, 0
  // .text:000311A8                 mov     ds:cnt, 3Ch
  // nowhere no hide
    _g->cnt = SHOWNEXTLOCDELAY * TICRATE;
}


// ====================================================================
// WI_updateShowNextLoc
// Purpose: Prepare to show the next level's location
// Args:    none
// Returns: void
//
static void WI_updateShowNextLoc(void)
{
  if (!--_g->cnt || _g->acceleratestage)
    WI_initNoState();
  else
    _g->snl_pointeron = (_g->cnt & 31) < 20;
}


// ====================================================================
// WI_drawShowNextLoc
// Purpose: Show the next level's location on animated backgrounds
// Args:    none
// Returns: void
//
static void WI_drawShowNextLoc(void)
{
    int32_t   i;
    int32_t   last;

    WI_slamBackground();

    last = (_g->wbs->last == 8) ? _g->wbs->next - 1 : _g->wbs->last;

    // draw a splat on taken cities.
    for (i=0 ; i<=last ; i++)
        WI_drawOnLnode(i, &splat);

    // splat the secret level?
    if (_g->wbs->didsecret)
        WI_drawOnLnode(8, &splat);

    // draw flashing ptr
    if (_g->snl_pointeron)
        WI_drawOnLnode(_g->wbs->next, yah);

    // draws which level you are entering..
    WI_drawEL();
}

// ====================================================================
// WI_drawNoState
// Purpose: Draw the pointer and next location
// Args:    none
// Returns: void
//
static void WI_drawNoState(void)
{
  _g->snl_pointeron = true;
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
  _g->state = StatCount;
  _g->acceleratestage = false;
  _g->sp_state = 1;

  _g->cnt_kills = -1;
  _g->cnt_secret = -1;
  _g->cnt_items = -1;

  _g->cnt_time = _g->cnt_par = _g->cnt_total_time = -1;
  _g->cnt_pause = TICRATE;
}

// ====================================================================
// WI_updateStats
// Purpose: Calculate solo stats
// Args:    none
// Returns: void
//
static void WI_updateStats(void)
{
  if (_g->acceleratestage && _g->sp_state != 10)
  {
    _g->acceleratestage = false;
    _g->cnt_kills = (_g->plrs[0].skills * 100) / _g->wbs->maxkills;
    _g->cnt_items = (_g->plrs[0].sitems * 100) / _g->wbs->maxitems;

    // killough 2/22/98: Make secrets = 100% if maxsecret = 0:
    _g->cnt_secret = (_g->wbs->maxsecret ?
      (_g->plrs[0].ssecret * 100) / _g->wbs->maxsecret : 100);

    _g->cnt_total_time = _g->wbs->totaltimes / TICRATE;
    _g->cnt_time = _g->plrs[0].stime / TICRATE;
    _g->cnt_par = _g->wbs->partime / TICRATE;
    S_StartSound(0, sfx_barexp);
    _g->sp_state = 10;
  }

  if (_g->sp_state == 2)
  {
    _g->cnt_kills += 2;

    if (!(_g->bcnt&3))
      S_StartSound(0, sfx_pistol);

    if (_g->cnt_kills >= (_g->plrs[0].skills * 100) / _g->wbs->maxkills)
    {
      _g->cnt_kills = (_g->plrs[0].skills * 100) / _g->wbs->maxkills;
      S_StartSound(0, sfx_barexp);
      _g->sp_state++;
    }
  }
  else if (_g->sp_state == 4)
  {
    _g->cnt_items += 2;

    if (!(_g->bcnt&3))
      S_StartSound(0, sfx_pistol);

    if (_g->cnt_items >= (_g->plrs[0].sitems * 100) / _g->wbs->maxitems)
    {
      _g->cnt_items = (_g->plrs[0].sitems * 100) / _g->wbs->maxitems;
      S_StartSound(0, sfx_barexp);
      _g->sp_state++;
    }
  }
  else if (_g->sp_state == 6)
  {
    _g->cnt_secret += 2;

    if (!(_g->bcnt&3))
      S_StartSound(0, sfx_pistol);

    // killough 2/22/98: Make secrets = 100% if maxsecret = 0:
    if (_g->cnt_secret >= (_g->wbs->maxsecret ? (_g->plrs[0].ssecret * 100) / _g->wbs->maxsecret : 100))
    {
      _g->cnt_secret = (_g->wbs->maxsecret ?
        (_g->plrs[0].ssecret * 100) / _g->wbs->maxsecret : 100);
      S_StartSound(0, sfx_barexp);
      _g->sp_state++;
    }
  }
  else if (_g->sp_state == 8)
  {
    if (!(_g->bcnt&3))
      S_StartSound(0, sfx_pistol);

    _g->cnt_time += 3;

    if (_g->cnt_time >= _g->plrs[0].stime / TICRATE)
      _g->cnt_time = _g->plrs[0].stime / TICRATE;

    _g->cnt_total_time += 3;

    if (_g->cnt_total_time >= _g->wbs->totaltimes / TICRATE)
      _g->cnt_total_time = _g->wbs->totaltimes / TICRATE;

    _g->cnt_par += 3;

    if (_g->cnt_par >= _g->wbs->partime / TICRATE)
    {
      _g->cnt_par = _g->wbs->partime / TICRATE;

      if ((_g->cnt_time >= _g->plrs[0].stime / TICRATE) && (_g->cnt_total_time >= _g->wbs->totaltimes / TICRATE))
      {
        S_StartSound(0, sfx_barexp);
        _g->sp_state++;
      }
    }
  }
  else if (_g->sp_state == 10)
  {
    if (_g->acceleratestage)
    {
      S_StartSound(0, sfx_sgcock);

      WI_initShowNextLoc();
    }
  }
  else if (_g->sp_state & 1)
  {
    if (!--_g->cnt_pause)
    {
      _g->sp_state++;
      _g->cnt_pause = TICRATE;
    }
  }
}

// ====================================================================
// WI_drawStats
// Purpose: Put the solo stats on the screen
// Args:    none
// Returns: void
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated


//lineHeight = (3 * num[0]->height) / 2;
#define lineHeight 18

static void WI_drawStats(void)
{
	WI_slamBackground();

	WI_drawLF();

	V_DrawNamePatch(SP_STATSX, SP_STATSY, kills);
	if (_g->cnt_kills)
		WI_drawPercent(320 - SP_STATSX, SP_STATSY, _g->cnt_kills);

	V_DrawNamePatch(SP_STATSX, SP_STATSY + lineHeight, items);
	if (_g->cnt_items)
		WI_drawPercent(320 - SP_STATSX, SP_STATSY + lineHeight, _g->cnt_items);

	V_DrawNamePatch(SP_STATSX, SP_STATSY + 2 * lineHeight, sp_secret);
	if (_g->cnt_secret)
		WI_drawPercent(320 - SP_STATSX, SP_STATSY + 2 * lineHeight, _g->cnt_secret);

	WI_drawTimeStats(_g->cnt_time, _g->cnt_total_time, _g->cnt_par);
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
  player_t  *player = &_g->player;

    if (_g->playeringame)
    {
      if (player->cmd.buttons & BT_ATTACK)
      {
        if (!player->attackdown)
          _g->acceleratestage = true;
        player->attackdown = true;
      }
      else
        player->attackdown = false;

      if (player->cmd.buttons & BT_USE)
      {
        if (!player->usedown)
          _g->acceleratestage = true;
        player->usedown = true;
      }
      else
        player->usedown = false;
    }
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
  _g->bcnt++;

  if (_g->bcnt == 1)
  {
    // intermission music
    S_ChangeMusic(mus_inter, true);
  }

  WI_checkForAccelerate();

  switch (_g->state)
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
  switch (_g->state)
  {
    case StatCount:
           WI_drawStats();
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

  _g->wbs = wbstartstruct;

  _g->acceleratestage = false;
  _g->cnt = _g->bcnt = 0;
  _g->plrs = _g->wbs->plyr;

  if (!_g->wbs->maxkills)
    _g->wbs->maxkills = 1;  // probably only useful in MAP30

  if (!_g->wbs->maxitems)
    _g->wbs->maxitems = 1;
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
