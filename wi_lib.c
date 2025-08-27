/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024-2025 Frenkel Smeijers
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
 *  Intermission screens library.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "r_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_lib.h"


// GLOBAL LOCATIONS
#define WI_TITLEY      2


// SINGLE-PLAYER STUFF
#define SP_STATSX     50
#define SP_STATSY     50

#define SP_TIMEX      8
#define SP_TIMEY      160


// "Finished!" graphics
static const char* const finished = "WIF";


// "Entering" graphic
static const char* const entering = "WIENTER";


// : graphic
static const char* const colon = "WICOLON";


// Time sucks.
static const char* const sucks = "WISUCKS";


// % graphic
static const char* const percent = "WIPCNT";


// You Are Here graphic
static const char* const yah = "WIURH0";


// splat
static const char* const splat = "WISPLAT";


// "Kills", "Items"
static const char* const kills = "WIOSTK";
static const char* const items = "WIOSTI";


// "secret"
static const char* const sp_secret = "WISCRT2";


static const char* const time1 = "WITIME";
static const char* const total = "WIMSTT";
static const char* const par = "WIPAR";


/* ====================================================================
 * WI_levelNameLump
 * Purpore: Returns the name of the graphic lump containing the name of
 *          the given level.
 * Args:    Level, and buffer (must by 9 chars) to write to
 * Returns: void
 */
static void WI_levelNameLump(int16_t map, char* buf)
{
	sprintf(buf, "WILV0%d", map);
}


// ====================================================================
// WI_drawLF
// Purpose: Draw the "Finished" level name before showing stats
// Args:    none
// Returns: void
//
void WI_drawLF(int16_t map)
{
	int16_t y = WI_TITLEY;
	char lname[9];

	// draw <LevelName>
	WI_levelNameLump(map, lname);
	const patch_t __far* patch = W_GetLumpByName(lname);
	V_DrawPatchScaled((SCREENWIDTH_VGA - patch->width)/2, y, patch);

	y += (5 * patch->height) / 4;
	Z_ChangeTagToCache(patch);

	// draw "Finished!"
	const patch_t __far* finishedpatch = W_GetLumpByName(finished);
	V_DrawPatchScaled((SCREENWIDTH_VGA - finishedpatch->width)/2, y, finishedpatch);
	Z_ChangeTagToCache(finishedpatch);
}


// ====================================================================
// WI_drawEL
// Purpose: Draw introductory "Entering" and level name
// Args:    none
// Returns: void
//
void WI_drawEL(int16_t map)
{
	int16_t y = WI_TITLEY;
	char lname[9];

	WI_levelNameLump(map, lname);
	const patch_t __far* patch = W_GetLumpByName(lname);

	// draw "Entering"
	const patch_t __far* enteringpatch = W_GetLumpByName(entering);
	V_DrawPatchScaled((SCREENWIDTH_VGA - enteringpatch->width)/2, y, enteringpatch);
	Z_ChangeTagToCache(enteringpatch);

	// draw level
	y += (5 * patch->height) / 4;
	V_DrawPatchScaled((SCREENWIDTH_VGA - patch->width)/2, y, patch);
	Z_ChangeTagToCache(patch);
}


int16_t WI_getColonWidth(void)
{
	return V_NumPatchWidth(W_GetNumForName(colon));
}


void WI_drawColon(int16_t x, int16_t y)
{
	V_DrawNamePatchScaled(x, y, colon);
}


void WI_drawSucks(int16_t x, int16_t y)
{
	const patch_t __far* patch = W_GetLumpByName(sucks);
	V_DrawPatchScaled(x - patch->width, y, patch);
	Z_ChangeTagToCache(patch);
}


void WI_drawPercentSign(int16_t x, int16_t y)
{
	V_DrawNamePatchScaled(x, y, percent);
}


// ====================================================================
// WI_drawNum
// Purpose: Draws a number.  If digits > 0, then use that many digits
//          minimum, otherwise only use as many as necessary
// Args:    x, y   -- location
//          n      -- the number to be drawn
//          digits -- number of digits minimum or zero
// Returns: new x position after drawing (note we are going to the left)

//fontwidth = num[0]->width;
#define fontwidth 11

int16_t WI_drawNum(int16_t x, int16_t y, int16_t n, int16_t digits)
{
	char      name[9];  // limited to 8 characters

	// draw the new number
	while (digits--)
	{
		x -= fontwidth;
		sprintf(name, "WINUM%d", n % 10);
		V_DrawNamePatchScaled(x, y, name);
		n /= 10;
	}

	return x;
}


// ====================================================================
// WI_drawStats
// Purpose: Put the solo stats on the screen
// Args:    none
// Returns: void

//lineHeight = (3 * num[0]->height) / 2;
#define lineHeight 18

void WI_drawStats(int16_t cnt_kills, int16_t cnt_items, int16_t cnt_secret, int32_t cnt_time, int32_t cnt_total_time, int16_t cnt_par)
{
	V_DrawNamePatchScaled(SP_STATSX, SP_STATSY, kills);
	WI_drawPercent(SCREENWIDTH_VGA - SP_STATSX, SP_STATSY, cnt_kills);

	V_DrawNamePatchScaled(SP_STATSX, SP_STATSY + lineHeight, items);
	WI_drawPercent(SCREENWIDTH_VGA - SP_STATSX, SP_STATSY + lineHeight, cnt_items);

	V_DrawNamePatchScaled(SP_STATSX, SP_STATSY + 2 * lineHeight, sp_secret);
	WI_drawPercent(SCREENWIDTH_VGA - SP_STATSX, SP_STATSY + 2 * lineHeight, cnt_secret);

	// Put the times on the screen
	V_DrawNamePatchScaled(SP_TIMEX, SP_TIMEY, time1);
	WI_drawTime(SCREENWIDTH_VGA / 2 - SP_TIMEX, SP_TIMEY, cnt_time);

	V_DrawNamePatchScaled(SP_TIMEX, (SP_TIMEY + SCREENHEIGHT_VGA) / 2, total);
	WI_drawTime(SCREENWIDTH_VGA / 2 - SP_TIMEX, (SP_TIMEY + SCREENHEIGHT_VGA) / 2, cnt_total_time);

	V_DrawNamePatchScaled(SCREENWIDTH_VGA / 2 + SP_TIMEX, SP_TIMEY, par);
	WI_drawTime(SCREENWIDTH_VGA - SP_TIMEX, SP_TIMEY, cnt_par);
}


typedef struct
{
  int16_t   x;       // x/y coordinate pair structure
  int16_t   y;
} point_t;

#define NUMMAPS     9

static const point_t lnodes[NUMMAPS] =
{
    { 185, 164 }, // location of level 1
    { 148, 143 }, // location of level 2
    {  69, 122 }, // location of level 3
    { 209, 102 }, // location of level 4
    { 116,  89 }, // location of level 5
    { 166,  55 }, // location of level 6
    {  71,  56 }, // location of level 7
    { 135,  29 }, // location of level 8
    {  71,  24 }  // location of level 9
};


void WI_drawSplat(int16_t i)
{
	V_DrawNamePatchScaled(lnodes[i].x, lnodes[i].y, splat);
}


void WI_drawYouAreHere(int16_t i)
{
	V_DrawNamePatchScaled(lnodes[i].x, lnodes[i].y, yah);
}
