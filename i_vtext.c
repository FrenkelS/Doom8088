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
 *      Video code for Text modes 80x25 and 40x25 16 color
 *
 *-----------------------------------------------------------------------------*/
 
#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#if VIEWWINDOWWIDTH == 40
#define SCREEN_MODE	1
#define PAGE_SIZE	0x0080
#elif VIEWWINDOWWIDTH == 80
#define SCREEN_MODE 3
#define PAGE_SIZE	0x0100
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

#define PAGE0		0xb800
#define PAGE1		(PAGE0+PAGE_SIZE)
#define PAGE2		(PAGE1+PAGE_SIZE)
#define PAGE3		(PAGE2+PAGE_SIZE)

#define PLANEWIDTH (VIEWWINDOWWIDTH*2)

extern const int16_t CENTERY;

static uint8_t __far* _s_screen;


void I_ReloadPalette(void)
{
	// TODO implement me
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	__djgpp_nearptr_enable();

	I_SetScreenMode(SCREEN_MODE);

	// disable blinking
	union REGS regs;
	regs.w.ax = 0x1003;
	regs.w.bx = 0x0000;
	int86(0x10, &regs, &regs);

	_s_screen = D_MK_FP(PAGE1, 1 + __djgpp_conventional_base);

	uint16_t __far* dst;
	dst = D_MK_FP(PAGE0, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x00b1;

	dst = D_MK_FP(PAGE1, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x00b1;

	dst = D_MK_FP(PAGE2, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x00b1;

	dst = D_MK_FP(PAGE3, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x00b1;

	outp(0x3d4, 0xc);
}


void I_SetPalette(int8_t pal)
{
	UNUSED(pal);
	// TODO implement me
}


void I_FinishUpdate(void)
{
	// page flip between segments
	// B800, B880, B900 for 40x25
	// B800, B900, BA00 for 80x25
	outp(0x3d5, (D_FP_SEG(_s_screen) >> 5) & 0x1c);
	_s_screen = D_MK_FP(D_FP_SEG(_s_screen) + PAGE_SIZE, 1 + __djgpp_conventional_base);
	if (D_FP_SEG(_s_screen) == PAGE3)
		_s_screen = D_MK_FP(PAGE0, 1 + __djgpp_conventional_base);
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

uint8_t nearcolormap[256];
static uint16_t nearcolormapoffset = 0xffff;

const uint8_t __far* source;
uint8_t __far* dest;


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	switch (count)
	{
		case 25: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 24: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 23: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 22: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 21: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 20: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 19: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 18: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 17: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 16: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 15: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 14: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 13: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 12: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 11: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 10: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  9: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  8: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  7: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  6: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  5: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  4: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  3: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  2: *dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  1: *dest = nearcolormap[source[frac >> COLBITS]];
	}
}
#else
void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count);
#endif


void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	source = dcvars->source;

	if (nearcolormapoffset != D_FP_OFF(dcvars->colormap))
	{
		_fmemcpy(nearcolormap, dcvars->colormap, 256);
		nearcolormapoffset = D_FP_OFF(dcvars->colormap);
	}

	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + (dcvars->x << 1);

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + (dcvars->x << 1);

	switch (count)
	{
		case 25: *dest = color; dest += PLANEWIDTH;
		case 24: *dest = color; dest += PLANEWIDTH;
		case 23: *dest = color; dest += PLANEWIDTH;
		case 22: *dest = color; dest += PLANEWIDTH;
		case 21: *dest = color; dest += PLANEWIDTH;
		case 20: *dest = color; dest += PLANEWIDTH;
		case 19: *dest = color; dest += PLANEWIDTH;
		case 18: *dest = color; dest += PLANEWIDTH;
		case 17: *dest = color; dest += PLANEWIDTH;
		case 16: *dest = color; dest += PLANEWIDTH;
		case 15: *dest = color; dest += PLANEWIDTH;
		case 14: *dest = color; dest += PLANEWIDTH;
		case 13: *dest = color; dest += PLANEWIDTH;
		case 12: *dest = color; dest += PLANEWIDTH;
		case 11: *dest = color; dest += PLANEWIDTH;
		case 10: *dest = color; dest += PLANEWIDTH;
		case  9: *dest = color; dest += PLANEWIDTH;
		case  8: *dest = color; dest += PLANEWIDTH;
		case  7: *dest = color; dest += PLANEWIDTH;
		case  6: *dest = color; dest += PLANEWIDTH;
		case  5: *dest = color; dest += PLANEWIDTH;
		case  4: *dest = color; dest += PLANEWIDTH;
		case  3: *dest = color; dest += PLANEWIDTH;
		case  2: *dest = color; dest += PLANEWIDTH;
		case  1: *dest = color;
	}
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0x08
#define FUZZCOLOR3 0x80
#define FUZZCOLOR4 0x88
#define FUZZTABLE 50

static const int8_t fuzzcolors[FUZZTABLE] =
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

	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + (dcvars->x << 1);

	static int16_t fuzzpos = 0;

	do
	{
		*dest = fuzzcolors[fuzzpos];
		dest += PLANEWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


void V_FillRect(byte colour)
{
	uint8_t __far* dest = _s_screen;
	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			*dest++ = colour;
			dest++;
		}
	}
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
		_s_screen[y0 * PLANEWIDTH + (x0 << 1)] = color;

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


void V_DrawBackground(void)
{
	const byte __far* src = W_GetLumpByName("FLOOR4_8");

	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		uint8_t __far* dest = _s_screen + y * PLANEWIDTH;
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			*dest++ = src[((y * 2) & 63) * 64 + ((x * 2) & 63)];
			dest++;
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	UNUSED(offset);

	static int16_t cachedLumpNum;

	if (cachedLumpNum != num)
	{
		const uint8_t __far* lump = W_TryGetLumpByNum(num);

		if (lump != NULL)
		{
			uint8_t __far* src = (uint8_t __far*)lump;
			uint8_t __far* dst = D_MK_FP(PAGE3, 1 + __djgpp_conventional_base);
			for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
			{
				for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
				{
					*dst++ = *src;
					src += (SCREENWIDTH / VIEWWINDOWWIDTH);
					dst++;
				}
				src += ((SCREENHEIGHT / VIEWWINDOWHEIGHT) - 1) * SCREENWIDTH;
			}
			Z_ChangeTagToCache(lump);

			cachedLumpNum = num;
		}
	}

	if (cachedLumpNum == num)
		_fmemcpy(_s_screen - 1, D_MK_FP(PAGE3, 0 + __djgpp_conventional_base), PLANEWIDTH * VIEWWINDOWHEIGHT);
}


void ST_Drawer(void)
{
	// TODO implement me
}


void V_DrawCharacter(int16_t x, int16_t y, char c)
{
	_s_screen[y * PLANEWIDTH + (x << 1) - 1] = c;
	_s_screen[y * PLANEWIDTH + (x << 1)    ] = 12;
}


void V_DrawString(int16_t y, char* s)
{
	int16_t x = 0;

	while (*s)
	{
		char c = toupper(*s);

		if (!(HU_FONTSTART <= c && c <= HU_FONTEND))
			c = ' ';

		_s_screen[y * PLANEWIDTH + x - 1] = c;
		_s_screen[y * PLANEWIDTH + x    ] = 12;

		x += 2;
		s++;
	}
}


void V_ClearString(int16_t y, size_t len)
{
	uint8_t __far* dst = _s_screen + y * PLANEWIDTH - 1;
	for (int16_t x = 0; x < len; x++)
	{
		*dst++ = 0xb1;
		dst++;
	}
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	UNUSED(x);
	UNUSED(y);
	UNUSED(patch);
	// TODO implement me
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	UNUSED(x);
	UNUSED(y);
	UNUSED(patch);
	// TODO implement me
}


void wipe_StartScreen(void)
{
	// Do nothing
}


static uint8_t __far* frontbuffer;
static int16_t __far* wipe_y_lookup;


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint8_t __far* backbuffer = _s_screen;

	while (ticks--)
	{
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
				int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : VIEWWINDOWHEIGHT / 25;
				// At most dy shall be so that the column is shifted by VIEWWINDOWHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= VIEWWINDOWHEIGHT)
					dy = VIEWWINDOWHEIGHT - wipe_y_lookup[i];

				uint8_t __far* s = &frontbuffer[i<<1] + ((VIEWWINDOWHEIGHT - 1 - dy) * PLANEWIDTH);
				uint8_t __far* d = &frontbuffer[i<<1] + ((VIEWWINDOWHEIGHT - 1)      * PLANEWIDTH);

				// scroll down the column. Of course we need to copy from the bottom... up to
				// VIEWWINDOWHEIGHT - yLookup - dy

				for (int16_t j = VIEWWINDOWHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -PLANEWIDTH;
					s += -PLANEWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i<<1]  + wipe_y_lookup[i] * PLANEWIDTH;
				d = &frontbuffer[i<<1] + wipe_y_lookup[i] * PLANEWIDTH;

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += PLANEWIDTH;
					s += PLANEWIDTH;
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
	frontbuffer = D_MK_FP(D_FP_SEG(_s_screen) - PAGE_SIZE, 1 + __djgpp_conventional_base);
	if (D_FP_SEG(frontbuffer) == (PAGE0 - PAGE_SIZE))
		frontbuffer = D_MK_FP(PAGE2, 1 + __djgpp_conventional_base);

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

	Z_Free(wipe_y_lookup);
}
