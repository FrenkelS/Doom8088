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
 *      Startup and quit functions. Handles signals, inits the
 *      memory management, then calls D_DoomMain. Also contains
 *      I_Init which does other system-related startup stuff.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "doomdef.h"
#include "compiler.h"
#include "d_main.h"
#include "m_fixed.h"
#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"
#include "d_player.h"
#include "g_game.h"
#include "doomtype.h"
#include "i_sound.h"
#include "globdata.h"

#include <dos.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static globals_t globals;
globals_t* _g = &globals;


static void I_Init(void)
{
	if (!(nomusicparm && nosfxparm))
		I_InitSound();
}


static void tprintf(void)
{
	union REGS regs;

	char* msg = "                          DOOM8088 System Startup                           ";

	for (int_fast8_t i = 0; i < strlen(msg); )
	{
		regs.h.ah = 9;
		regs.h.al = msg[i];
		regs.w.cx = 1;
		regs.w.bx = (7 << 4) | 4;
		int86(0x10, &regs, &regs);

		regs.h.ah = 2;
		regs.h.bh = 0;
		regs.w.dx = ++i;
		int86(0x10, &regs, &regs);
	}

	printf("\n");
}

extern int myargc;
extern const char * const * myargv;

int main(int argc, const char * const * argv)
{
	myargc = argc;
	myargv = argv;

	/* cphipps - call to video specific startup code */
	I_InitScreen();

	tprintf();

	//Call this before Z_Init as maxmod uses malloc.
	I_Init();

	printf("Z_Init: Init zone memory allocation daemon.\n");
	Z_Init();                  /* 1/18/98 killough: start up memory stuff first */

	//InitGlobals
	memset(_g, 0, sizeof(globals_t));
	freehead = &_g->freetail;

	D_DoomMain ();
	return 0;
}
