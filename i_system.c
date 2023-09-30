/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023 Frenkel Smeijers
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
 *      DOS implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <conio.h>
#include <dos.h>
#include <stdarg.h>
#include <time.h>

#include "doomdef.h"
#include "compiler.h"
#include "d_main.h"
#include "i_system.h"
#include "globdata.h"


static void I_SetScreenMode(uint16_t mode);
static void I_Quit(void);


//**************************************************************************************
//
// Keyboard code
//

#define KEYBOARDINT 9
#define KBDQUESIZE 32
static byte keyboardqueue[KBDQUESIZE];
static int32_t kbdtail, kbdhead;
static boolean isKeyboardIsrSet = false;

#if defined __DJGPP__ 
static _go32_dpmi_seginfo oldkeyboardisr, newkeyboardisr;
#elif defined __WATCOMC__
static void (__interrupt __far *oldkeyboardisr)(void);
#endif

static void __interrupt I_KeyboardISR(void)	
{
	// Get the scan code
	keyboardqueue[kbdhead & (KBDQUESIZE - 1)] = inp(0x60);
	kbdhead++;

	// acknowledge the interrupt
	outp(0x20, 0x20);
}

void I_InitScreen(void)
{
	I_SetScreenMode(3);

	replaceInterrupt(oldkeyboardisr, newkeyboardisr, KEYBOARDINT, I_KeyboardISR);
	isKeyboardIsrSet = true;
}


#define SC_ESCAPE		0x01
#define SC_TAB			0x0f
#define SC_ENTER		0x1c
#define SC_CTRL			0x1d
#define SC_LSHIFT		0x2a
#define SC_RSHIFT		0x36
#define SC_COMMA		0x33
#define SC_PERIOD		0x34
#define SC_SPACE		0x39
#define SC_F10			0x44
#define SC_UPARROW		0x48
#define SC_DOWNARROW	0x50
#define SC_LEFTARROW	0x4b
#define SC_RIGHTARROW	0x4d

void I_StartTic(void)
{
	//
	// process keyboard events
	//
	byte k;
	event_t ev;

	while (kbdtail < kbdhead)
	{
		k = keyboardqueue[kbdtail & (KBDQUESIZE - 1)];
		kbdtail++;

		// extended keyboard shift key bullshit
		if ((k & 0x7f) == SC_LSHIFT || (k & 0x7f) == SC_RSHIFT)
		{
			if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe0)
				continue;
			k &= 0x80;
			k |= SC_RSHIFT;
		}

		if (k == 0xe0)
			continue;               // special / pause keys
		if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe1)
			continue;                               // pause key bullshit

		if (k == 0xc5 && keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0x9d)
		{
			//ev.type  = ev_keydown;
			//ev.data1 = KEY_PAUSE;
			//D_PostEvent(&ev);
			continue;
		}

		if (k & 0x80)
			ev.type = ev_keyup;
		else
			ev.type = ev_keydown;

		k &= 0x7f;
		switch (k)
		{
			case SC_ESCAPE:
				ev.data1 = KEYD_START;
				break;
			case SC_ENTER:
			case SC_SPACE:
			case SC_RSHIFT:
				ev.data1= KEYD_A;
				break;
			case SC_UPARROW:
				ev.data1 = KEYD_UP;
				break;
			case SC_DOWNARROW:
				ev.data1 = KEYD_DOWN;
				break;
			case SC_LEFTARROW:
				ev.data1 = KEYD_LEFT;
				break;
			case SC_RIGHTARROW:
				ev.data1 = KEYD_RIGHT;
				break;
			case SC_TAB:
				ev.data1 = KEYD_SELECT;
				break;
			case SC_CTRL:
				ev.data1 = KEYD_B;
				break;
			case SC_COMMA:
				ev.data1 = KEYD_L;
				break;
			case SC_PERIOD:
				ev.data1 = KEYD_R;
				break;
			case SC_F10:
				I_Quit();
			default:
				continue;
		}
		D_PostEvent(&ev);
	}
}


//**************************************************************************************
//
// Screen code
//

static boolean isGraphicsModeSet = false;
static uint16_t __far* screen;
static uint16_t __far backBuffer[SCREENWIDTH * SCREENHEIGHT];

static int8_t newpal;

uint16_t __far* I_GetBackBuffer(void)
{
	return &backBuffer[0];
}


void I_CopyBackBufferToBuffer(uint16_t __far* buffer)
{
	uint16_t __far* src = &backBuffer[0];
	uint16_t __far* dst = buffer;
	for (int16_t i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++)
		*dst++ = *src++;
}


static void I_SetScreenMode(uint16_t mode)
{
	union REGS regs;
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
}


#define SCREENWIDTH_VGA  320
#define SCREENHEIGHT_VGA 200

void I_InitGraphics(void)
{	
	I_SetScreenMode(0x13);
	isGraphicsModeSet = true;

	screen = MK_FP(0xa000, ((SCREENWIDTH_VGA - SCREENWIDTH * 2) / 2) + (((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * SCREENWIDTH_VGA) + __djgpp_conventional_base);
}


void I_StartDisplay(void)
{
	_g_screen = &backBuffer[0];
}


void I_DrawBuffer(uint16_t __far* buffer)
{
	uint16_t __far* src = buffer;
	uint16_t __far* dst = screen;
	for (uint_fast8_t y = 0; y < SCREENHEIGHT; y++) {
		for (uint_fast8_t x = 0; x < (SCREENWIDTH * 2) / 2; x++) {
			*dst++ = *src++;
		}
		dst += ((SCREENWIDTH_VGA - (SCREENWIDTH * 2)) / 2);
	}
}


static void I_FinishUpdate_dos(void)
{
	I_DrawBuffer(&backBuffer[0]);
}


#define PEL_WRITE_ADR   0x3c8
#define PEL_DATA        0x3c9

static void I_SetPalette_dos(const byte __far* palette)
{
	int_fast16_t i;

	outp(PEL_WRITE_ADR, 0);
	for (i = 0; i < 768; i++)
		outp(PEL_DATA, (*palette++) >> 2);
}


static void I_UploadNewPalette(int8_t pal)
{
	// This is used to replace the current 256 colour cmap with a new one
	// Used by 256 colour PseudoColor modes

	char lumpName[9] = "PLAYPAL0";

	if(_g_gamma == 0)
		lumpName[7] = 0;
	else
		lumpName[7] = '0' + _g_gamma;

	const uint8_t __far* palette_lump = W_GetLumpByName(lumpName);
	I_SetPalette_dos(&palette_lump[pal*256*3]);
	Z_ChangeTagToCache(palette_lump);
}


//
// I_FinishUpdate
//

#define NO_PALETTE_CHANGE 100

void I_FinishUpdate (void)
{
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	I_FinishUpdate_dos();
}


//
// I_SetPalette
//
void I_SetPalette (int8_t pal)
{
	newpal = pal;
}


//**************************************************************************************
//
// Exit code
//

static void I_Shutdown(void)
{
	if (isGraphicsModeSet)
		I_SetScreenMode(3);

	if (isKeyboardIsrSet)
	{
		restoreInterrupt(KEYBOARDINT, oldkeyboardisr, newkeyboardisr);
	}

	Z_Shutdown();
}


static void I_Quit(void)
{
	I_Shutdown();

	W_ReadLumpByName("ENDOOM", MK_FP(0xb800, __djgpp_conventional_base));

	union REGS regs;
	regs.h.ah = 2;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int86(0x10, &regs, &regs);

	printf("\n");
	exit(0);
}


void I_Error (const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

int32_t I_GetTime(void)
{
    int32_t thistimereply;
    static int32_t lasttimereply = 0;
    static int32_t basetime = 0;
    clock_t now = clock();

    thistimereply = (now * TICRATE) / CLOCKS_PER_SEC;

    if (thistimereply < lasttimereply)
    {
        basetime -= 0xffff;
    }

    lasttimereply = thistimereply;


    /* Fix for time problem */
    if (!basetime)
    {
        basetime = thistimereply;
        thistimereply = 0;
    }
    else
    {
        thistimereply -= basetime;
    }

    return thistimereply;
}
