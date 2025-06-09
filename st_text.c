/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024 Frenkel Smeijers
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


static void STlib_drawNum(int16_t x, int16_t y, uint8_t color, int16_t num)
{
	if (0 <= num && num <= 9)
	{
		V_DrawSTCharacter(x + 2, y, color, '0' + num);
		V_DrawSTCharacter(x + 1, y, color, ' ');
		V_DrawSTCharacter(x + 0, y, color, ' ');
	}
	else if (10 <= num && num <= 99)
	{
		V_DrawSTCharacter(x + 2, y, color, '0' + (num % 10));
		V_DrawSTCharacter(x + 1, y, color, '0' + (num / 10));
		V_DrawSTCharacter(x + 0, y, color, ' ');
	}
	else
	{
		V_DrawSTCharacter(x + 2, y, color, '0' + (num % 10));
		num /= 10;
		V_DrawSTCharacter(x + 1, y, color, '0' + (num % 10));
		V_DrawSTCharacter(x + 0, y, color, '0' + (num / 10));
	}
}


static void STlib_drawFps(int16_t x, int16_t y, uint8_t color, int16_t num)
{
	if (0 <= num && num <= 9)
	{
		V_DrawSTCharacter(x + 3, y, color, '0' + num);
		V_DrawSTCharacter(x + 1, y, color, '0');
		V_DrawSTCharacter(x + 0, y, color, ' ');
	}
	else if (10 <= num && num <= 99)
	{
		V_DrawSTCharacter(x + 3, y, color, '0' + (num % 10));
		V_DrawSTCharacter(x + 1, y, color, '0' + (num / 10));
		V_DrawSTCharacter(x + 0, y, color, ' ');
	}
	else
	{
		V_DrawSTCharacter(x + 3, y, color, '0' + (num % 10));
		num /= 10;
		V_DrawSTCharacter(x + 1, y, color, '0' + (num % 10));
		V_DrawSTCharacter(x + 0, y, color, '0' + (num / 10));
	}
}


static void ST_drawWidgets(void)
{
	// ammo
	if (_g_fps_show)
		STlib_drawFps(7, VIEWWINDOWHEIGHT - 2, 4, _g_fps_framerate);
	else if (weaponinfo[_g_player.readyweapon].ammo != am_noammo)
		STlib_drawNum(8, VIEWWINDOWHEIGHT - 2, 4, _g_player.ammo[weaponinfo[_g_player.readyweapon].ammo]);
	else
		V_DrawSTString(8, VIEWWINDOWHEIGHT - 2, 4, "   ");

	for (int16_t i = 0; i < NUMAMMO; i++)
	{
		STlib_drawNum(VIEWWINDOWWIDTH - 8, VIEWWINDOWHEIGHT - 5 + i, 14, _g_player.ammo[i]);
		STlib_drawNum(VIEWWINDOWWIDTH - 4, VIEWWINDOWHEIGHT - 5 + i, 14, _g_player.maxammo[i]);
	}

	int16_t healthcolor = _g_player.health < 40 ? 12 : 4;
	STlib_drawNum(8, VIEWWINDOWHEIGHT - 4, healthcolor, _g_player.health);

	STlib_drawNum(8, VIEWWINDOWHEIGHT - 3, 4, _g_player.armorpoints);

	// keys
	if (_g_player.cards[0])
		V_DrawSTCharacter(8, VIEWWINDOWHEIGHT - 5, 9, 'B'); // Blue
	else
		V_DrawSTCharacter(8, VIEWWINDOWHEIGHT - 5, 7, '\xf9');

	if (_g_player.cards[1])
		V_DrawSTCharacter(9, VIEWWINDOWHEIGHT - 5, 14, 'Y'); // Yellow
	else
		V_DrawSTCharacter(9, VIEWWINDOWHEIGHT - 5, 7, '\xf9');

	if (_g_player.cards[2])
		V_DrawSTCharacter(10, VIEWWINDOWHEIGHT - 5, 12, 'R'); // Red
	else
		V_DrawSTCharacter(10, VIEWWINDOWHEIGHT - 5, 7, '\xf9');
}


static void ST_refreshBackground(void)
{
	V_DrawSTString(1, VIEWWINDOWHEIGHT - 5, 7, "Keys   ");
	V_DrawSTString(1, VIEWWINDOWHEIGHT - 4, 7, "Health ");
	V_DrawSTString(1, VIEWWINDOWHEIGHT - 3, 7, "Armor  ");

	if (_g_fps_show)
		V_DrawSTString(1, VIEWWINDOWHEIGHT - 2, 7, "FPS     .");
	else
		V_DrawSTString(1, VIEWWINDOWHEIGHT - 2, 7, "Ammo   ");

	V_DrawSTString(VIEWWINDOWWIDTH - 13, VIEWWINDOWHEIGHT - 5, 7, "Bull ");
	V_DrawSTString(VIEWWINDOWWIDTH - 13, VIEWWINDOWHEIGHT - 4, 7, "Shel ");
	V_DrawSTString(VIEWWINDOWWIDTH - 13, VIEWWINDOWHEIGHT - 3, 7, "Rckt ");
	V_DrawSTString(VIEWWINDOWWIDTH - 13, VIEWWINDOWHEIGHT - 2, 7, "Cell ");

	V_DrawSTCharacter(VIEWWINDOWWIDTH - 5, VIEWWINDOWHEIGHT - 5, 7, '/');
	V_DrawSTCharacter(VIEWWINDOWWIDTH - 5, VIEWWINDOWHEIGHT - 4, 7, '/');
	V_DrawSTCharacter(VIEWWINDOWWIDTH - 5, VIEWWINDOWHEIGHT - 3, 7, '/');
	V_DrawSTCharacter(VIEWWINDOWWIDTH - 5, VIEWWINDOWHEIGHT - 2, 7, '/');
}


void ST_Drawer(void)
{
  // draw status bar background to off-screen buff
  ST_refreshBackground();

  // and refresh all widgets
  ST_drawWidgets();

  V_SetSTPalette();
}


static boolean st_stopped = true;

void ST_Start(void)
{
  if (!st_stopped)
    ST_Stop();
  ST_initPalette();
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
