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
 *  Copyright 2024 by
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
 *      Status bar code.
 *      Does the face/direction indicator animation.
 *      Does palette indicators as well (red pain/berserk, bright pickup)
 *      Text mode version
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "doomdef.h"
#include "d_player.h"
#include "m_random.h"
#include "i_system.h"
#include "i_vtext.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "r_main.h"
#include "am_map.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_englsh.h"

#include "globdata.h"


//
// Typedefs of widgets
//

// Number widget

typedef struct
{
  // upper right-hand corner
  //  of the number (right-justified)
  int16_t   x;
  int16_t   y;

  // max # of digits in number
  int16_t width;

  // last number value
  int16_t   oldnum;

  // pointer to current value
  int16_t*  num;

  // list of patches for 0-9
  int16_t* p;

} st_number_t;


// Multiple Icon widget
typedef struct
{
  // center-justified location of icons
  int16_t     x;
  int16_t     y;

  // last icon number
  int16_t     oldinum;

  // pointer to current icon
  int16_t*    inum;

  // list of icons
  int16_t*   p;

} st_multicon_t;


// 0-9, tall numbers
static int16_t tallnum[10];

// 0-9, short, yellow (,different!) numbers
static int16_t shortnum[10];

// ready-weapon widget
static st_number_t w_ready;

// ammo widgets
static st_number_t w_ammo[4];

// max ammo widgets
static st_number_t w_maxammo[4];

// a random number per tick
static int16_t      st_randomnumber;

static int8_t st_palette;


// Size of statusbar.
// Now sensitive for scaling.

// proff 08/18/98: Changed for high-res
#define ST_Y      (SCREENHEIGHT - ST_HEIGHT)


//
// STATUS BAR DATA
//

// Location of status bar
#define ST_X                    0


// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?
// I dunno, why don't you go and find out!!!  killough

// AMMO number pos.
#define ST_AMMOWIDTH            3
// proff 08/18/98: Changed for high-res
#define ST_AMMOX                (ST_X+32)
#define ST_AMMOY                (ST_Y+6)

// Ammunition counter.
#define ST_AMMO0WIDTH           3
// proff 08/18/98: Changed for high-res
#define ST_AMMO0X               (ST_X+220)
#define ST_AMMO0Y               (ST_Y+5)

#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO1X               (ST_X+220)
#define ST_AMMO1Y               (ST_Y+11)

#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO2X               (ST_X+220)
#define ST_AMMO2Y               (ST_Y+23)

#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO3X               (ST_X+220)
#define ST_AMMO3Y               (ST_Y+17)


// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO0X            (ST_X+238)
#define ST_MAXAMMO0Y            (ST_Y+5)

#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO1X            (ST_X+238)
#define ST_MAXAMMO1Y            (ST_Y+11)

#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO2X            (ST_X+238)
#define ST_MAXAMMO2Y            (ST_Y+23)

#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO3X            (ST_X+238)
#define ST_MAXAMMO3Y            (ST_Y+17)


//
// STATUS BAR CODE
//

static void ST_Stop(void);


static int16_t largeammo = 1994; // means "n/a"

static void ST_updateWidgets(void)
{
    int8_t         i;

    if(_g_fps_show)
        w_ready.num = &_g_fps_framerate;
    else if (weaponinfo[_g_player.readyweapon].ammo == am_noammo)
        w_ready.num = &largeammo;
    else
        w_ready.num = &_g_player.ammo[weaponinfo[_g_player.readyweapon].ammo];
}

void ST_Ticker(void)
{
  st_randomnumber = M_Random();
  ST_updateWidgets();
}


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13


void ST_doPaletteStuff(void)
{
    int8_t  palette;
    int16_t cnt = _g_player.damagecount;

    if (_g_player.powers[pw_strength])
    {
        // slowly fade the berzerk out
        int16_t bzc = 12 - (_g_player.powers[pw_strength] >> 6);
        if (bzc > cnt)
            cnt = bzc;
    }

    if (cnt)
    {
        palette = (cnt + 7) >> 3;
        if (palette >= NUMREDPALS)
            palette = NUMREDPALS - 1;

        /* cph 2006/08/06 - if in the menu, reduce the red tint - navigating to
       * load a game can be tricky if the screen is all red */
        if (_g_menuactive)
            palette >>= 1;

        palette += STARTREDPALS;
    }
    else if (_g_player.bonuscount)
    {
        palette = (_g_player.bonuscount + 7) >> 3;
        if (palette >= NUMBONUSPALS)
            palette = NUMBONUSPALS - 1;
        palette += STARTBONUSPALS;
    }
    else if (_g_player.powers[pw_ironfeet] > 4 * 32 || _g_player.powers[pw_ironfeet] & 8)
        palette = RADIATIONPAL;
    else
        palette = 0;

    if (palette != st_palette) {
        I_SetPalette(st_palette = palette);
    }
}


//
// STlib_updateMultIcon()
//
// Draw a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys. Displays each when the control
// numbers change
//
// Passed a st_multicon_t widget
// Returns nothing.
//
static void STlib_updateMultIcon(st_multicon_t* mi)
{
    if(!mi->p)
        return;

    if (*mi->inum != -1)  // killough 2/16/98: redraw only if != -1
		V_DrawNumPatchNotScaled(mi->x, mi->y, mi->p[*mi->inum]);

    mi->oldinum = *mi->inum;

}


/*
 * STlib_drawNum()
 *
 * A fairly efficient way to draw a number based on differences from the
 * old number.
 *
 * Passed a st_number_t widget
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps 10/99 - const pointer to colour trans table, made function static
 */
static void STlib_drawNumber(st_number_t* n)
{

  int16_t   numdigits = n->width;
  int16_t   num = *n->num;

  const int16_t   w = 1;
  int16_t   x = n->x;

  // CPhipps - compact some code, use num instead of *n->num
  if ((n->oldnum = num) < 0)
  {
    if (numdigits == 2 && num < -9)
      num = -9;
    else if (numdigits == 3 && num < -99)
      num = -99;

    num = -num;
  }

  // clear the area
  x = n->x - numdigits*w;

  // if non-number, do not draw it
  if (num == largeammo)
    return;

  x = n->x;

  // in the special case of 0, you draw 0
  if (!num)
    V_DrawNumPatchNotScaled(x - w, n->y, n->p[0]);

  // draw the new number
  while (num && numdigits--)
  {
    x -= w;
    V_DrawNumPatchNotScaled(x, n->y, n->p[num % 10]);
    num /= 10;
  }
}


static void STlib_drawNum(int16_t x, int16_t y, uint8_t color, int16_t num)
{
	if (0 <= num && num <= 9)
	{
		V_DrawCharacter(x + 2, y, color, '0' + num);
		V_DrawCharacter(x + 1, y, color, ' ');
		V_DrawCharacter(x + 0, y, color, ' ');
	}
	else if (10 <= num && num <= 99)
	{
		V_DrawCharacter(x + 2, y, color, '0' + (num % 10));
		V_DrawCharacter(x + 1, y, color, '0' + (num / 10));
		V_DrawCharacter(x + 0, y, color, ' ');
	}
	else
	{
		V_DrawCharacter(x + 2, y, color, '0' + (num % 10));
		num /= 10;
		V_DrawCharacter(x + 1, y, color, '0' + (num % 10));
		V_DrawCharacter(x + 0, y, color, '0' + (num / 10));
	}
}


static void ST_drawWidgets(void)
{
	// ammo
	if (_g_fps_show)
		STlib_drawNum(8, 23, 12, _g_fps_framerate);
	else if (weaponinfo[_g_player.readyweapon].ammo != am_noammo)
		STlib_drawNum(8, 23, 12, _g_player.ammo[weaponinfo[_g_player.readyweapon].ammo]);
	else
		V_DrawString(8, 23, 12, "   ");

	// Restore the ammo numbers for backpack stats I guess, etc ~Kippykip
	for (int8_t i = 0; i < 4; i++)
    {
		STlib_drawNumber(&w_ammo[i]);
		STlib_drawNumber(&w_maxammo[i]);
    }

	STlib_drawNum(8, 21, 12, _g_player.health);
	STlib_drawNum(8, 22, 12, _g_player.armorpoints);

	// keys
	if (_g_player.cards[0])
		V_DrawCharacter(8, 20, 9, '\x14');
	else
		V_DrawCharacter(8, 20, 7, '\xf9');

	if (_g_player.cards[1])
		V_DrawCharacter(9, 20, 14, '\x14');
	else
		V_DrawCharacter(9, 20, 7, '\xf9');

	if (_g_player.cards[2])
		V_DrawCharacter(10, 20, 12, '\x14');
	else
		V_DrawCharacter(10, 20, 7, '\xf9');
}


static void ST_refreshBackground(void)
{
	V_DrawString(1, 20, 7, "Keys   ");
	V_DrawString(1, 21, 7, "Health ");
	V_DrawString(1, 22, 7, "Armor  ");
	V_DrawString(1, 23, 7, "Ammo   ");

	V_DrawString(VIEWWINDOWWIDTH - 13, 20, 7, "Bull ");
	V_DrawString(VIEWWINDOWWIDTH - 13, 21, 7, "Shel ");
	V_DrawString(VIEWWINDOWWIDTH - 13, 22, 7, "Rckt ");
	V_DrawString(VIEWWINDOWWIDTH - 13, 23, 7, "Cell ");
}


void ST_Drawer(void)
{
  // draw status bar background to off-screen buff
  ST_refreshBackground();

  // and refresh all widgets
  ST_drawWidgets();
}


//
// ST_loadData
//
// CPhipps - Loads graphics needed for status bar
//
static void ST_loadData(void)
{
    int8_t  i;
    char namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
		sprintf(namebuf, "STGANUM%d", i); //Special GBA Doom II Red Numbers ~Kippykip
        tallnum[i] = W_GetNumForName(namebuf);

        sprintf(namebuf, "STYSNUM%d", i);
        shortnum[i] = W_GetNumForName(namebuf);
    }
}


static void ST_initData(void)
{
    st_palette = -1;
}


//
// STlib_initMultIcon()
//
// Initialize a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys.
//
// Passed a st_multicon_t widget, the position, the graphic patches, and a pointer
// to the numbers representing what to display
// Returns nothing.
//
static void STlib_initMultIcon(st_multicon_t* i, int16_t x, int16_t y, int16_t* il, int16_t* inum)
{
	i->x       = x;
	i->y       = y;
	i->oldinum = -1;
	i->inum    = inum;
	i->p       = il;
}


//
// STlib_initNum()
//
// Initializes an st_number_t widget
//
// Passed the widget, its position, the patches for the digits, a pointer
// to the value displayed, and the width
// Returns nothing
//
static void STlib_initNum(st_number_t* n, int16_t x, int16_t y, int16_t* pl, int16_t* num, int16_t width)
{
	n->x      = x;
	n->y      = y;
	n->oldnum = 0;
	n->width  = width;
	n->num    = num;
	n->p      = pl;
}


static void ST_createWidgets(void)
{
    // ready weapon ammo
    STlib_initNum(&w_ready, ST_AMMOX, ST_AMMOY, tallnum, &_g_player.ammo[weaponinfo[_g_player.readyweapon].ammo], ST_AMMOWIDTH);

	// ammo count (all four kinds)
	STlib_initNum(&w_ammo[0], ST_AMMO0X, ST_AMMO0Y, shortnum, &_g_player.ammo[0], ST_AMMO0WIDTH);
	STlib_initNum(&w_ammo[1], ST_AMMO1X, ST_AMMO1Y, shortnum, &_g_player.ammo[1], ST_AMMO1WIDTH);
	STlib_initNum(&w_ammo[2], ST_AMMO2X, ST_AMMO2Y, shortnum, &_g_player.ammo[2], ST_AMMO2WIDTH);
	STlib_initNum(&w_ammo[3], ST_AMMO3X, ST_AMMO3Y, shortnum, &_g_player.ammo[3], ST_AMMO3WIDTH);

	// max ammo count (all four kinds)
	STlib_initNum(&w_maxammo[0], ST_MAXAMMO0X, ST_MAXAMMO0Y, shortnum, &_g_player.maxammo[0], ST_MAXAMMO0WIDTH);
	STlib_initNum(&w_maxammo[1], ST_MAXAMMO1X, ST_MAXAMMO1Y, shortnum, &_g_player.maxammo[1], ST_MAXAMMO1WIDTH);
	STlib_initNum(&w_maxammo[2], ST_MAXAMMO2X, ST_MAXAMMO2Y, shortnum, &_g_player.maxammo[2], ST_MAXAMMO2WIDTH);
	STlib_initNum(&w_maxammo[3], ST_MAXAMMO3X, ST_MAXAMMO3Y, shortnum, &_g_player.maxammo[3], ST_MAXAMMO3WIDTH);
}

static boolean st_stopped = true;

void ST_Start(void)
{
  if (!st_stopped)
    ST_Stop();
  ST_initData();
  ST_createWidgets();
  st_stopped = false;
}

static void ST_Stop(void)
{
  if (st_stopped)
    return;
  I_SetPalette(0);
  st_stopped = true;
}

void ST_Init(void)
{
  ST_loadData();
}
