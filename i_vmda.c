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
 *      Video code for MDA 80x25 monochrome text mode
 *
 *-----------------------------------------------------------------------------*/
 
#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "i_vtext.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define PLANEWIDTH (VIEWWINDOWWIDTH*2)


extern const int16_t CENTERY;

static uint8_t __far _s_screen[PLANEWIDTH * VIEWWINDOWHEIGHT];
static uint8_t __far* videomemory;


static boolean isMDA;


void I_ReloadPalette(void)
{
	char lumpName[8] = "COLORMAP";
	if (_g_gamma != 0)
	{
		lumpName[6] = 'P';
		lumpName[7] = '0' + _g_gamma;
	}

	W_ReadLumpByNum(W_GetNumForName(lumpName), (void __far*)fullcolormap);
}


static const uint8_t colors[14] =
{
	0x07,											// normal
	0x70, 0x70, 0x70, 0x70, 0x78, 0x78, 0x78, 0x78,	// red
	0x0e, 0x0e, 0x0e, 0x0e,							// yellow
	0x01											// green
};


static int8_t newpal;


void V_SetSTPalette(void)
{
	uint8_t attribute = colors[newpal];

	for (int16_t y = VIEWWINDOWHEIGHT - 5; y < VIEWWINDOWHEIGHT - 1; y++)
	{
		for (int16_t x = 1; x < 11; x++)
			_s_screen[y * PLANEWIDTH + x * 2 + 1] = attribute;

		for (int16_t x = VIEWWINDOWWIDTH - 13; x < VIEWWINDOWWIDTH - 1; x++)
			_s_screen[y * PLANEWIDTH + x * 2 + 1] = attribute;
	}
}


segment_t I_GetTextModeVideoMemorySegment(void)
{
	return isMDA ? 0xb000 : 0xb800;
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(isMDA ? 7 : 3);

	__djgpp_nearptr_enable();
	videomemory = D_MK_FP(I_GetTextModeVideoMemorySegment(), 0 + __djgpp_conventional_base);
}


static void I_DrawBuffer(uint8_t __far* buffer)
{
	_fmemcpy(videomemory, buffer, PLANEWIDTH * VIEWWINDOWHEIGHT);
}


void I_ShutdownGraphics(void)
{
	I_SetScreenMode(isMDA ? 7 : 3);
}


void I_SetPalette(int8_t pal)
{
	newpal = pal;
}


void I_FinishUpdate(void)
{
	I_DrawBuffer(_s_screen);
}



#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

const uint8_t* colormap;

const uint8_t __far* source;
uint8_t __far* dest;
uint8_t attribute;


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	switch (count)
	{
		case 25: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 24: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 23: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 22: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 21: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 20: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 19: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 18: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 17: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 16: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 15: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 14: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 13: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 12: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 11: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case 10: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  9: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  8: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  7: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  6: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  5: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  4: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  3: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  2: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute; dest += PLANEWIDTH - 1; frac += fracstep;
		case  1: *dest++ = colormap[source[frac >> COLBITS]]; *dest = attribute;
	}
}
#else
void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count);
#endif


static void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	source = dcvars->source;

	colormap = dcvars->colormap;

	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x * 2;

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	attribute = 0x0f;
	R_DrawColumn(dcvars);
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	attribute = 0x07;
	R_DrawColumn(dcvars);
}


#if defined C_ONLY
static void R_DrawColumnFlat2(uint8_t color, int16_t count)
{
	uint16_t __far* dst = (uint16_t __far*)dest;
	uint16_t c = 0x0700 | color;

	switch (count)
	{
		case 25: dst[(PLANEWIDTH / sizeof(uint16_t)) * 24] = c;
		case 24: dst[(PLANEWIDTH / sizeof(uint16_t)) * 23] = c;
		case 23: dst[(PLANEWIDTH / sizeof(uint16_t)) * 22] = c;
		case 22: dst[(PLANEWIDTH / sizeof(uint16_t)) * 21] = c;
		case 21: dst[(PLANEWIDTH / sizeof(uint16_t)) * 20] = c;
		case 20: dst[(PLANEWIDTH / sizeof(uint16_t)) * 19] = c;
		case 19: dst[(PLANEWIDTH / sizeof(uint16_t)) * 18] = c;
		case 18: dst[(PLANEWIDTH / sizeof(uint16_t)) * 17] = c;
		case 17: dst[(PLANEWIDTH / sizeof(uint16_t)) * 16] = c;
		case 16: dst[(PLANEWIDTH / sizeof(uint16_t)) * 15] = c;
		case 15: dst[(PLANEWIDTH / sizeof(uint16_t)) * 14] = c;
		case 14: dst[(PLANEWIDTH / sizeof(uint16_t)) * 13] = c;
		case 13: dst[(PLANEWIDTH / sizeof(uint16_t)) * 12] = c;
		case 12: dst[(PLANEWIDTH / sizeof(uint16_t)) * 11] = c;
		case 11: dst[(PLANEWIDTH / sizeof(uint16_t)) * 10] = c;
		case 10: dst[(PLANEWIDTH / sizeof(uint16_t)) *  9] = c;
		case  9: dst[(PLANEWIDTH / sizeof(uint16_t)) *  8] = c;
		case  8: dst[(PLANEWIDTH / sizeof(uint16_t)) *  7] = c;
		case  7: dst[(PLANEWIDTH / sizeof(uint16_t)) *  6] = c;
		case  6: dst[(PLANEWIDTH / sizeof(uint16_t)) *  5] = c;
		case  5: dst[(PLANEWIDTH / sizeof(uint16_t)) *  4] = c;
		case  4: dst[(PLANEWIDTH / sizeof(uint16_t)) *  3] = c;
		case  3: dst[(PLANEWIDTH / sizeof(uint16_t)) *  2] = c;
		case  2: dst[(PLANEWIDTH / sizeof(uint16_t)) *  1] = c;
		case  1: dst[(PLANEWIDTH / sizeof(uint16_t)) *  0] = c;
	}
}
#else
void R_DrawColumnFlat2(uint8_t color, int16_t count);
#endif


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x * 2;

	R_DrawColumnFlat2(color, count);
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0xb0
#define FUZZCOLOR3 0x00
#define FUZZCOLOR4 0xb0
#define FUZZTABLE 50

static const uint8_t fuzzcolors[FUZZTABLE] =
{
	FUZZCOLOR1,FUZZCOLOR2,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR2,
	FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR2,
	FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR4,FUZZCOLOR2,
	FUZZCOLOR1,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR4,
	FUZZCOLOR3,FUZZCOLOR2,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR1,
	FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR1,FUZZCOLOR3,
	FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR2,FUZZCOLOR1
};


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x * 2;

	static int16_t fuzzpos = 0;

	do
	{
		*dest++ = fuzzcolors[fuzzpos];
		*dest   = 0x07;
		dest += PLANEWIDTH-1;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


void V_ClearViewWindow(void)
{
	for (int16_t i = 0; i < PLANEWIDTH * VIEWWINDOWHEIGHT; i += 2)
		_s_screen[i] = 0;
}


void V_InitDrawLine(void)
{
	// Do nothing
}


void V_ShutdownDrawLine(void)
{
	// Do nothing
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	while (true)
	{
		_s_screen[y0 * PLANEWIDTH + x0 * 2 + 0] = color;
		_s_screen[y0 * PLANEWIDTH + x0 * 2 + 1] = 0x07;

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;
		}

		if (e2 <= dx)
		{
			err += dx;
			y0  += sy;
		}
	}
}


void V_DrawBackground(int16_t backgroundnum)
{
	const byte __far* src = W_GetLumpByNum(backgroundnum);

	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		uint8_t __far* dest = _s_screen + y * PLANEWIDTH;
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			*dest++ = src[((y * 2) & 63) * 64 + ((x * 2) & 63)];
			*dest++ = 0x07;
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	UNUSED(offset);

	const uint8_t __far* lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		_fmemcpy(_s_screen, lump, PLANEWIDTH * VIEWWINDOWHEIGHT);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, _s_screen);
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	UNUSED(x);
	UNUSED(y);
	UNUSED(patch);
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	UNUSED(x);
	UNUSED(y);
	UNUSED(patch);
}


void V_DrawCharacter(int16_t x, int16_t y, uint8_t color, char c)
{
	_s_screen[y * PLANEWIDTH + x * 2 + 0] = c;
	_s_screen[y * PLANEWIDTH + x * 2 + 1] = color;
}


void V_DrawSTCharacter(int16_t x, int16_t y, uint8_t color, char c)
{
	UNUSED(color);
	_s_screen[y * PLANEWIDTH + x * 2] = c;
}


void V_DrawCharacterForeground(int16_t x, int16_t y, uint8_t color, char c)
{
	_s_screen[y * PLANEWIDTH + x * 2 + 0] = c;
	_s_screen[y * PLANEWIDTH + x * 2 + 1] = color;
}


void V_DrawString(int16_t x, int16_t y, uint8_t color, const char* s)
{
	uint8_t __far* dst = _s_screen + y * PLANEWIDTH + x * 2;

	while (*s)
	{
		*dst++ = *s++;
		*dst++ = color;
	}
}


void V_DrawSTString(int16_t x, int16_t y, uint8_t color, const char* s)
{
	UNUSED(color);

	uint8_t __far* dst = _s_screen + y * PLANEWIDTH + x * 2;

	while (*s)
	{
		*dst++ = *s++;
		dst++;
	}
}


void V_ClearString(int16_t y, size_t len)
{
	UNUSED(y);
	UNUSED(len);
}


void I_InitScreenPage(void)
{
	// Do nothing
}


void I_InitScreenPages(void)
{
	// Do nothing
}


static uint8_t __far* frontbuffer;
static int16_t __far* wipe_y_lookup;


void wipe_StartScreen(void)
{
	frontbuffer = Z_TryMallocStatic(PLANEWIDTH * VIEWWINDOWHEIGHT);
	if (frontbuffer)
	{
		// copy back buffer to front buffer
		_fmemcpy(frontbuffer, _s_screen, PLANEWIDTH * VIEWWINDOWHEIGHT);
	}
}


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint8_t __far* backbuffer = _s_screen;

	while (ticks--)
	{
		I_DrawBuffer(frontbuffer);
		for (int16_t i = 0; i < VIEWWINDOWWIDTH; i++)
		{
			if (wipe_y_lookup[i] < 0)
			{
				wipe_y_lookup[i]++;
				done = false;
				continue;
			}

			// scroll down columns, which are still visible
			if (wipe_y_lookup[i] < VIEWWINDOWHEIGHT)
			{
				int16_t dy = 1;
				// At most dy shall be so that the column is shifted by VIEWWINDOWHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= VIEWWINDOWHEIGHT)
					dy = VIEWWINDOWHEIGHT - wipe_y_lookup[i];

				uint16_t __far* s = (uint16_t __far*)(&frontbuffer[i<<1] + ((VIEWWINDOWHEIGHT - 1 - dy) * PLANEWIDTH));
				uint16_t __far* d = (uint16_t __far*)(&frontbuffer[i<<1] + ((VIEWWINDOWHEIGHT - 1)      * PLANEWIDTH));

				// scroll down the column. Of course we need to copy from the bottom... up to
				// VIEWWINDOWHEIGHT - yLookup - dy

				for (int16_t j = VIEWWINDOWHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -VIEWWINDOWWIDTH;
					s += -VIEWWINDOWWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = (uint16_t __far*)(&backbuffer[i<<1]  + wipe_y_lookup[i] * PLANEWIDTH);
				d = (uint16_t __far*)(&frontbuffer[i<<1] + wipe_y_lookup[i] * PLANEWIDTH);

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += VIEWWINDOWWIDTH;
					s += VIEWWINDOWWIDTH;
				}

				wipe_y_lookup[i] += dy;
				done = false;
			}
		}
	}

	return done;
}


static void wipe_initMelt()
{
	wipe_y_lookup = Z_MallocStatic(VIEWWINDOWWIDTH * sizeof(int16_t));

	// setup initial column positions (y<0 => not ready to scroll yet)
	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int16_t i = 1; i < VIEWWINDOWWIDTH; i++)
	{
		int16_t r = (M_Random() % 3) - 1;

		wipe_y_lookup[i] = wipe_y_lookup[i - 1] + r;

		if (wipe_y_lookup[i] > 0)
			wipe_y_lookup[i] = 0;
		else if (wipe_y_lookup[i] == -16)
			wipe_y_lookup[i] = -15;
	}
}


void D_Wipe(void)
{
	if (!frontbuffer)
		return;

	wipe_initMelt();

	boolean done;
	int32_t wipestart = I_GetTime() - 1;

	do
	{
		int32_t nowtime;
		int16_t tics;
		do
		{
			nowtime = I_GetTime();
			tics = nowtime - wipestart;
		} while (!tics);

		wipestart = nowtime;
		done = wipe_ScreenWipe(tics);

		M_Drawer();                   // menu is drawn even on top of wipes

	} while (!done);

	Z_Free(frontbuffer);
	Z_Free(wipe_y_lookup);
}


static boolean I_IsMDA(void)
{
	union REGS regs;
	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	return regs.h.al == 7;
}


void D_DoomMain(int argc, const char * const * argv);

int main(int argc, const char * const * argv)
{
	isMDA = I_IsMDA();

	I_SetScreenMode(isMDA ? 7 : 3);

	printf(
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"DOOM8088 System Startup"
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"\n");

	D_DoomMain(argc, argv);
	return 0;
}
