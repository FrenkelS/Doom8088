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
 *      Video code for 16 color Text modes
 *      80x25, 40x25
 *      80x43, 40x43
 *      80x50, 40x50
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


#if VIEWWINDOWWIDTH == 40
#define SCREEN_MODE	1
#define CGA_HIGH_RESOLUTION 0
#elif VIEWWINDOWWIDTH == 80
#define SCREEN_MODE 3
#define CGA_HIGH_RESOLUTION 1
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

#if VIEWWINDOWHEIGHT == 25
#define VGA_SCANLINES 0
#elif VIEWWINDOWHEIGHT == 43
#define VGA_SCANLINES 1
#elif VIEWWINDOWHEIGHT == 50
#define VGA_SCANLINES 2
#else
#error unsupported VIEWWINDOWHEIGHT value
#endif


#define PLANEWIDTH (VIEWWINDOWWIDTH*2)

#define PAGE_SIZE (((PLANEWIDTH * VIEWWINDOWHEIGHT + 511) & ~511) >> 4)

#define PAGE0		0xb800
#define PAGE1		(PAGE0+PAGE_SIZE)
#define PAGE2		(PAGE1+PAGE_SIZE)
#define PAGE3		(PAGE2+PAGE_SIZE)


#define DITHER_CHARACTER 0xb1


extern const int16_t CENTERY;

static uint8_t __far* _s_screen;


void I_ReloadPalette(void)
{
	char* lumpName;

	if (_g_gamma == 0)
		lumpName = "COLORMAP";
	else
	{
		lumpName = "COLORMP0";
		lumpName[7] = '0' + _g_gamma;
	}

	W_ReadLumpByNum(W_GetNumForName(lumpName), (void __far*)fullcolormap);
}


typedef enum
{
	MDA,
	CGA,
	PCJR,
	TANDY,
	EGA,
	VGA,
	MCGA
} videocardsenum_t;


static videocardsenum_t videocard;


static videocardsenum_t I_DetectVideoCard(void)
{
#if VIEWWINDOWHEIGHT == 50
	return VGA;
#elif VIEWWINDOWHEIGHT == 43
	union REGS regs;
	regs.w.ax = 0x1200;
	regs.h.bl = 0x32;
	int86(0x10, &regs, &regs);

	return regs.h.al == 0x12 ? VGA : EGA;
#else
	// This code is based on Jason M. Knight's Paku Paku code

	union REGS regs;
	regs.w.ax = 0x1200;
	regs.h.bl = 0x32;
	int86(0x10, &regs, &regs);

	if (regs.h.al == 0x12)
	{
		regs.w.ax = 0x1a00;
		regs.h.bl = 0;
		int86(0x10, &regs, &regs);
		return (0x0a <= regs.h.bl && regs.h.bl <= 0x0c) ? MCGA : VGA;
	}

	regs.h.ah = 0x12;
	regs.h.bl = 0x10;
	int86(0x10, &regs, &regs);
	if (regs.h.bl & 3)
		return EGA;

	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	if (regs.h.al == 7)
		return MDA;

	uint8_t __far* fp;
	fp = D_MK_FP(0xffff, 0x000e);
	if (*fp == 0xfd)
		return PCJR;

	if (*fp == 0xff)
	{
		fp = D_MK_FP(0xfc00, 0);
		if (*fp == 0x21)
			return TANDY;
	}

	return CGA;
#endif
}


static const int8_t colors[14] =
{
	0,							// normal
	4, 4, 4, 4, 12, 12, 12, 12,	// red
	6, 6, 14, 14,				// yellow
	2							// green
};


static void I_UploadNewPalette(int8_t pal)
{
#if VIEWWINDOWHEIGHT == 25
	if (videocard == CGA)
	{
		outp(0x3d9, colors[pal]);
		return;
	}
#endif

	union REGS regs;
	regs.w.ax = 0x1000;
	regs.h.bl = 0x00;
	regs.h.bh = colors[pal];
	int86(0x10, &regs, &regs);
}


static void I_DisableBlinking(void)
{
#if VIEWWINDOWHEIGHT == 25
	if (videocard == CGA)
	{
		outp(0x3d8, 8 | CGA_HIGH_RESOLUTION);
		return;
	}
#endif

	union REGS regs;
	regs.w.ax = 0x1003;
	regs.w.bx = 0x0000;
	int86(0x10, &regs, &regs);
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	__djgpp_nearptr_enable();

	videocard = I_DetectVideoCard();
	if (videocard == VGA)
	{
		// change scan lines to get the 8x8 font
		union REGS regs;
		regs.w.ax = 0x1200 | VGA_SCANLINES;
		regs.h.bl = 0x30;
		int86(0x10, &regs, &regs);
	}

	I_SetScreenMode(SCREEN_MODE);

#if VIEWWINDOWHEIGHT == 43 || VIEWWINDOWHEIGHT == 50
	union REGS regs;
	regs.w.ax = 0x1112;
	regs.h.bl = 0;
	int86(0x10, &regs, &regs);
#endif

	I_DisableBlinking();

	_s_screen = D_MK_FP(PAGE1, 1 + __djgpp_conventional_base);

	uint16_t __far* dst;
	dst = D_MK_FP(PAGE0, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x0000 | DITHER_CHARACTER;

	dst = D_MK_FP(PAGE1, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x0000 | DITHER_CHARACTER;

	dst = D_MK_FP(PAGE2, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x0000 | DITHER_CHARACTER;

	dst = D_MK_FP(PAGE3, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x0000 | DITHER_CHARACTER;

	outp(0x3d4, 0xc);
}


void I_ShutdownGraphics(void)
{
	if (videocard == VGA)
	{
		// 400 scan lines
		union REGS regs;
		regs.w.ax = 0x1202;
		regs.h.bl = 0x30;
		int86(0x10, &regs, &regs);
	}

	I_SetScreenMode(3);
}


static int8_t newpal;


void I_SetPalette(int8_t pal)
{
	newpal = pal;
}


void V_SetSTPalette(void)
{
	// Do nothing
}


#define NO_PALETTE_CHANGE 100


void I_FinishUpdate(void)
{
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	// page flip between segments
	// B800, B880, B900 for 40x25
	// B800, B900, BA00 for 80x25
	// B800, B8E0, B9C0 for 40x43
	// B800, B9C0, BB80 for 80x43
	// B800, B900, BA00 for 40x50
	// B800, BA00, BC00 for 80x50
	outp(0x3d5, (D_FP_SEG(_s_screen) >> 5) & 0x3f);
	_s_screen = D_MK_FP(D_FP_SEG(_s_screen) + PAGE_SIZE, 1 + __djgpp_conventional_base);
	if (D_FP_SEG(_s_screen) == PAGE3)
		_s_screen = D_MK_FP(PAGE0, 1 + __djgpp_conventional_base);
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

const uint8_t* colormap;

const uint8_t __far* source;
uint8_t __far* dest;


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	switch (count)
	{
		case 25: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 24: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 23: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 22: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 21: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 20: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 19: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 18: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 17: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 16: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 15: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 14: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 13: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 12: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 11: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 10: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  9: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  8: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  7: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  6: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  5: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  4: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  3: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  2: *dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  1: *dest = colormap[source[frac >> COLBITS]];
	}
}
#else
void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count);
#endif


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	source = dcvars->source;

	colormap = dcvars->colormap;

	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + (dcvars->x << 1);

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


#if defined C_ONLY
static void R_DrawColumnFlat2(uint8_t color, int16_t count)
{
	switch (count)
	{
#if VIEWWINDOWHEIGHT >= 50
		case 50: dest[PLANEWIDTH * 49] = color;
		case 49: dest[PLANEWIDTH * 48] = color;
		case 48: dest[PLANEWIDTH * 47] = color;
		case 47: dest[PLANEWIDTH * 46] = color;
		case 46: dest[PLANEWIDTH * 45] = color;
		case 45: dest[PLANEWIDTH * 44] = color;
		case 44: dest[PLANEWIDTH * 43] = color;
#endif

#if VIEWWINDOWHEIGHT >= 43
		case 43: dest{PLANEWIDTH * 42] = color;
		case 42: dest{PLANEWIDTH * 41] = color;
		case 41: dest{PLANEWIDTH * 40] = color;
		case 40: dest{PLANEWIDTH * 39] = color;
		case 39: dest{PLANEWIDTH * 38] = color;
		case 38: dest{PLANEWIDTH * 37] = color;
		case 37: dest{PLANEWIDTH * 36] = color;
		case 36: dest{PLANEWIDTH * 35] = color;
		case 35: dest{PLANEWIDTH * 34] = color;
		case 34: dest{PLANEWIDTH * 33] = color;
		case 33: dest{PLANEWIDTH * 32] = color;
		case 32: dest{PLANEWIDTH * 31] = color;
		case 31: dest{PLANEWIDTH * 30] = color;
		case 30: dest{PLANEWIDTH * 29] = color;
		case 29: dest{PLANEWIDTH * 28] = color;
		case 28: dest{PLANEWIDTH * 27] = color;
		case 27: dest{PLANEWIDTH * 26] = color;
		case 26: dest{PLANEWIDTH * 25] = color;
#endif

		case 25: dest[PLANEWIDTH * 24] = color;
		case 24: dest[PLANEWIDTH * 23] = color;
		case 23: dest[PLANEWIDTH * 22] = color;
		case 22: dest[PLANEWIDTH * 21] = color;
		case 21: dest[PLANEWIDTH * 20] = color;
		case 20: dest[PLANEWIDTH * 19] = color;
		case 19: dest[PLANEWIDTH * 18] = color;
		case 18: dest[PLANEWIDTH * 17] = color;
		case 17: dest[PLANEWIDTH * 16] = color;
		case 16: dest[PLANEWIDTH * 15] = color;
		case 15: dest[PLANEWIDTH * 14] = color;
		case 14: dest[PLANEWIDTH * 13] = color;
		case 13: dest[PLANEWIDTH * 12] = color;
		case 12: dest[PLANEWIDTH * 11] = color;
		case 11: dest[PLANEWIDTH * 10] = color;
		case 10: dest[PLANEWIDTH *  9] = color;
		case  9: dest[PLANEWIDTH *  8] = color;
		case  8: dest[PLANEWIDTH *  7] = color;
		case  7: dest[PLANEWIDTH *  6] = color;
		case  6: dest[PLANEWIDTH *  5] = color;
		case  5: dest[PLANEWIDTH *  4] = color;
		case  4: dest[PLANEWIDTH *  3] = color;
		case  3: dest[PLANEWIDTH *  2] = color;
		case  2: dest[PLANEWIDTH *  1] = color;
		case  1: dest[PLANEWIDTH *  0] = color;
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

	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + (dcvars->x << 1);

	R_DrawColumnFlat2(color, count);
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0x08
#define FUZZCOLOR3 0x80
#define FUZZCOLOR4 0x88
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


static int16_t cachedLumpNum;


static void V_Blit(int16_t num)
{
	if (cachedLumpNum == num)
		_fmemcpy(_s_screen - 1, D_MK_FP(PAGE3, 0 + __djgpp_conventional_base), PLANEWIDTH * VIEWWINDOWHEIGHT);
}


void V_DrawBackground(int16_t backgroundnum)
{
	if (cachedLumpNum != backgroundnum)
	{
		const uint8_t __far* lump = W_GetLumpByNum(backgroundnum);

		for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
		{
			uint8_t __far* dest = D_MK_FP(PAGE3, y * PLANEWIDTH + 1 + __djgpp_conventional_base);
			for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
			{
				*dest++ = lump[((y * 2) & 63) * 64 + ((x * 2) & 63)];
				dest++;
			}
		}

		Z_ChangeTagToCache(lump);

		cachedLumpNum = backgroundnum;
	}

	V_Blit(backgroundnum);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	UNUSED(offset);

	if (cachedLumpNum != num)
	{
		const uint8_t __far* lump = W_TryGetLumpByNum(num);

		if (lump != NULL)
		{
			static const int16_t DXI = SCREENWIDTH / VIEWWINDOWWIDTH;
			static const fixed_t DYI = ((fixed_t)SCREENHEIGHT << FRACBITS) / VIEWWINDOWHEIGHT;
			fixed_t y = 0;
			uint8_t __far* dst = D_MK_FP(PAGE3, 1 + __djgpp_conventional_base);
			for (int16_t h = 0; h < VIEWWINDOWHEIGHT; h++)
			{
				int16_t x = 0;
				for (int16_t w = 0; w < VIEWWINDOWWIDTH; w++)
				{
					*dst = lump[(y >> FRACBITS) * SCREENWIDTH + x];
					x += DXI;
					dst += 2;
				}
				y += DYI;
			}
			Z_ChangeTagToCache(lump);

			cachedLumpNum = num;
		}
	}

	V_Blit(num);
}


void V_DrawCharacter(int16_t x, int16_t y, uint8_t color, char c)
{
	_s_screen[y * PLANEWIDTH + (x << 1) - 1] = c;
	_s_screen[y * PLANEWIDTH + (x << 1)    ] = color;
}


void V_DrawSTCharacter(int16_t x, int16_t y, uint8_t color, char c)
{
	V_DrawCharacter(x, y, color, c);
}


void V_DrawCharacterForeground(int16_t x, int16_t y, uint8_t color, char c)
{
	_s_screen[y * PLANEWIDTH + (x << 1) - 1] = c;

	uint8_t background = _s_screen[y * PLANEWIDTH + (x << 1)] & 0xf0;
	_s_screen[y * PLANEWIDTH + (x << 1)] = background | color;
}


void V_DrawString(int16_t x, int16_t y, uint8_t color, const char* s)
{
	x <<= 1;

	while (*s)
	{
		_s_screen[y * PLANEWIDTH + x - 1] = *s;
		_s_screen[y * PLANEWIDTH + x    ] = color;

		x += 2;
		s++;
	}
}


void V_DrawSTString(int16_t x, int16_t y, uint8_t color, const char* s)
{
	V_DrawString(x, y, color, s);
}


void V_ClearString(int16_t y, size_t len)
{
	uint8_t __far* dst = _s_screen + y * PLANEWIDTH - 1;
	for (int16_t x = 0; x < len; x++)
	{
		*dst++ = DITHER_CHARACTER;
		dst++;
	}
}


void I_InitScreenPage(void)
{
	uint8_t __far* dst = _s_screen - 1 + VIEWWINDOWWIDTH * 2;
	// Skip the first row and the last 5 rows
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * (VIEWWINDOWHEIGHT - 1 - 5); i++)
	{
		*dst++ = DITHER_CHARACTER;
		dst++;
	}
}


void I_InitScreenPages(void)
{
	uint8_t __far* dst;
	dst = D_MK_FP(PAGE0, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
	{
		*dst++ = DITHER_CHARACTER;
		dst++;
	}

	dst = D_MK_FP(PAGE1, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
	{
		*dst++ = DITHER_CHARACTER;
		dst++;
	}

	dst = D_MK_FP(PAGE2, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
	{
		*dst++ = DITHER_CHARACTER;
		dst++;
	}
}


void wipe_StartScreen(void)
{
	I_InitScreenPages();
}


static uint8_t __far* frontbuffer;
static int16_t __far* wipe_y_lookup;


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint8_t __far* backbuffer = _s_screen - 1;

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
				int16_t dy = ((VIEWWINDOWHEIGHT - 1) / 25) + 1; // 1 or 2
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
	frontbuffer = D_MK_FP(D_FP_SEG(_s_screen) - PAGE_SIZE, 0 + __djgpp_conventional_base);
	if (D_FP_SEG(frontbuffer) == (PAGE0 - PAGE_SIZE))
		frontbuffer = D_MK_FP(PAGE2, 0 + __djgpp_conventional_base);

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
