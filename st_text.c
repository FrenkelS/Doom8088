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


static int8_t st_palette;


//
// STATUS BAR CODE
//

static void ST_Stop(void);


void ST_Ticker(void)
{
	// Somehow removing this code results in a linker error:
	//
	// /usr/lib/x86_64-linux-gnu/gcc/ia16-elf/6.3.0/../../../../../ia16-elf/bin/ld: BFD (GNU Binutils) 2.39 internal error, aborting at ../../bfd/linker.c:2204 in _bfd_generic_link_output_symbols
	//
	// /usr/lib/x86_64-linux-gnu/gcc/ia16-elf/6.3.0/../../../../../ia16-elf/bin/ld: Please report this bug.

	static int16_t num;
	if (_g_fps_show)
		num = _g_fps_framerate;
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


static void STlib_drawFps(int16_t x, int16_t y, uint8_t color, int16_t num)
{
	if (0 <= num && num <= 9)
	{
		V_DrawCharacter(x + 3, y, color, '0' + num);
		V_DrawCharacter(x + 1, y, color, '0');
		V_DrawCharacter(x + 0, y, color, ' ');
	}
	else if (10 <= num && num <= 99)
	{
		V_DrawCharacter(x + 3, y, color, '0' + (num % 10));
		V_DrawCharacter(x + 1, y, color, '0' + (num / 10));
		V_DrawCharacter(x + 0, y, color, ' ');
	}
	else
	{
		V_DrawCharacter(x + 3, y, color, '0' + (num % 10));
		num /= 10;
		V_DrawCharacter(x + 1, y, color, '0' + (num % 10));
		V_DrawCharacter(x + 0, y, color, '0' + (num / 10));
	}
}


static void ST_drawWidgets(void)
{
	// ammo
	if (_g_fps_show)
		STlib_drawFps(7, 23, 4, _g_fps_framerate);
	else if (weaponinfo[_g_player.readyweapon].ammo != am_noammo)
		STlib_drawNum(8, 23, 4, _g_player.ammo[weaponinfo[_g_player.readyweapon].ammo]);
	else
		V_DrawString(8, 23, 4, "   ");

	for (int16_t i = 0; i < NUMAMMO; i++)
	{
		STlib_drawNum(VIEWWINDOWWIDTH - 8, 20 + i, 14, _g_player.ammo[i]);
		STlib_drawNum(VIEWWINDOWWIDTH - 4, 20 + i, 14, _g_player.maxammo[i]);
	}

	int16_t healthcolor = _g_player.health < 40 ? 12 : 4;
	STlib_drawNum(8, 21, healthcolor, _g_player.health);

	STlib_drawNum(8, 22, 4, _g_player.armorpoints);

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

	if (_g_fps_show)
		V_DrawString(1, 23, 7, "FPS     .");
	else
		V_DrawString(1, 23, 7, "Ammo   ");

	V_DrawString(VIEWWINDOWWIDTH - 13, 20, 7, "Bull ");
	V_DrawString(VIEWWINDOWWIDTH - 13, 21, 7, "Shel ");
	V_DrawString(VIEWWINDOWWIDTH - 13, 22, 7, "Cell ");
	V_DrawString(VIEWWINDOWWIDTH - 13, 23, 7, "Rckt ");

	V_DrawCharacter(VIEWWINDOWWIDTH - 5, 20, 7, '/');
	V_DrawCharacter(VIEWWINDOWWIDTH - 5, 21, 7, '/');
	V_DrawCharacter(VIEWWINDOWWIDTH - 5, 22, 7, '/');
	V_DrawCharacter(VIEWWINDOWWIDTH - 5, 23, 7, '/');
}


void ST_Drawer(void)
{
  // draw status bar background to off-screen buff
  ST_refreshBackground();

  // and refresh all widgets
  ST_drawWidgets();
}


static void ST_initData(void)
{
    st_palette = -1;
}


static boolean st_stopped = true;

void ST_Start(void)
{
  if (!st_stopped)
    ST_Stop();
  ST_initData();
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
	// Do nothing
}
