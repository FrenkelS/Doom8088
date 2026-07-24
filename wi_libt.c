/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024-2026 Frenkel Smeijers
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
 *  Text mode version.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "d_englsh.h"
#include "i_vtext.h"
#include "wi_lib.h"


//
// Builtin map names.
//
static const char *const mapnames[] =
{
    HUSTR_E1M1,
    HUSTR_E1M2,
    HUSTR_E1M3,
    HUSTR_E1M4,
    HUSTR_E1M5,
    HUSTR_E1M6,
    HUSTR_E1M7,
    HUSTR_E1M8,
    HUSTR_E1M9,
};


// ====================================================================
// WI_drawLF
// Purpose: Draw the "Finished" level name before showing stats
// Args:    none
// Returns: void
//
void WI_drawLF(int16_t map)
{
	int16_t x;

	x = (VIEWWINDOWWIDTH - strlen(mapnames[map])) / 2;
	V_DrawString(x, 0, D_WHITE, mapnames[map]);

	x = (VIEWWINDOWWIDTH - 8) / 2;
	V_DrawString(x, 1, D_LIGHT_RED, "FINISHED");
}


// ====================================================================
// WI_drawEL
// Purpose: Draw introductory "Entering" and level name
// Args:    none
// Returns: void
//
void WI_drawEL(int16_t map)
{
	int16_t x;

	x = (VIEWWINDOWWIDTH - 8) / 2;
	V_DrawString(x, 0, D_LIGHT_RED, "ENTERING");

	x = (VIEWWINDOWWIDTH - strlen(mapnames[map])) / 2;
	V_DrawString(x, 1, D_WHITE, mapnames[map]);
}


int16_t WI_getColonWidth(void)
{
	return 1;
}


void WI_drawColon(int16_t x, int16_t y)
{
	V_DrawCharacter(x, y, D_LIGHT_RED, ':');
}


void WI_drawSucks(int16_t x, int16_t y)
{
	V_DrawString(x - 8, y, D_LIGHT_RED, "Sucks");
}


void WI_drawPercentSign(int16_t x, int16_t y)
{
	V_DrawCharacter(x, y, D_LIGHT_RED, '%');
}


// ====================================================================
// WI_drawNum
// Purpose: Draws a number.  If digits > 0, then use that many digits
//          minimum, otherwise only use as many as necessary
// Args:    x, y   -- location
//          n      -- the number to be drawn
//          digits -- number of digits minimum or zero
// Returns: new x position after drawing (note we are going to the left)
//
int16_t WI_drawNum(int16_t x, int16_t y, int16_t n, int16_t digits)
{
	// draw the new number
	while (digits--)
	{
		x -= 1;
		V_DrawCharacter(x, y, D_LIGHT_RED, '0' + (n % 10));
		n /= 10;
	}

	return x;
}


// ====================================================================
// WI_drawStats
// Purpose: Put the solo stats on the screen
// Args:    none
// Returns: void
//
void WI_drawStats(int16_t cnt_kills, int16_t cnt_items, int16_t cnt_secret, int32_t cnt_time, int32_t cnt_total_time, int16_t cnt_par)
{
	V_DrawString((VIEWWINDOWWIDTH - 12) / 2, 4, D_LIGHT_RED, "Kills");
	WI_drawPercent((VIEWWINDOWWIDTH - 12) / 2 + 10, 4, cnt_kills);

	V_DrawString((VIEWWINDOWWIDTH - 12) / 2, 5, D_LIGHT_RED, "Items");
	WI_drawPercent((VIEWWINDOWWIDTH - 12) / 2 + 10, 5, cnt_items);

	V_DrawString((VIEWWINDOWWIDTH - 12) / 2, 6, D_LIGHT_RED, "Secret");
	WI_drawPercent((VIEWWINDOWWIDTH - 12) / 2 + 10, 6, cnt_secret);

	// Put the times on the screen
	V_DrawString(1, VIEWWINDOWHEIGHT - 3, D_LIGHT_RED, "Time");
	WI_drawTime(15, VIEWWINDOWHEIGHT - 3, cnt_time);

	V_DrawString(1, VIEWWINDOWHEIGHT - 2, D_LIGHT_RED, "Total");
	WI_drawTime(15, VIEWWINDOWHEIGHT - 2, cnt_total_time);

	V_DrawString(VIEWWINDOWWIDTH - 13, VIEWWINDOWHEIGHT - 3, D_LIGHT_RED, "Par");
	WI_drawTime(VIEWWINDOWWIDTH - 1, VIEWWINDOWHEIGHT - 3, cnt_par);
}


typedef struct
{
  int16_t   x;       // x/y coordinate pair structure
  int16_t   y;
} point_t;

#define NUMMAPS     9

static const point_t lnodes[NUMMAPS] =
{
	{185 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA, 164 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 1
	{148 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA, 143 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 2
	{ 69 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA, 122 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 3
	{209 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA, 102 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 4
	{116 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA,  89 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 5
	{166 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA,  55 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 6
	{ 71 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA,  56 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 7
	{135 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA,  29 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}, // location of level 8
	{ 71 * VIEWWINDOWWIDTH / SCREENWIDTH_VGA,  24 * VIEWWINDOWHEIGHT / SCREENHEIGHT_VGA}  // location of level 9
};


void WI_drawSplat(int16_t i)
{
	V_DrawCharacterForeground(lnodes[i].x, lnodes[i].y, D_LIGHT_RED, '\x0f');
}


void WI_drawYouAreHere(int16_t i)
{
	V_DrawCharacterForeground(lnodes[i].x, lnodes[i].y, D_LIGHT_RED, '\x02');
}
