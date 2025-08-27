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
 *      Video code for EGA 320x200 and 640x200 16 colors
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


#if VIEWWINDOWWIDTH == 30
#define SCREEN_MODE	0x0d
#define PLANEWIDTH 40
#define SCALE_FACTOR 1
#elif VIEWWINDOWWIDTH == 60
#define SCREEN_MODE 0x0e
#define PLANEWIDTH 80
#define SCALE_FACTOR 2
#else
#error unsupported VIEWWINDOWWIDTH value
#endif


#define PAGE_SIZE (((PLANEWIDTH * SCREENHEIGHT_VGA + 511) & ~511) >> 4)

#define PAGE0	0xa000
#define PAGE1	(PAGE0+PAGE_SIZE)
#define PAGE2	(PAGE1+PAGE_SIZE)
#define PAGE3	(PAGE2+PAGE_SIZE)
#define PAGEMINUS1	(PAGE0-PAGE_SIZE)


#define SC_INDEX                0x3c4
#define SC_MAPMASK              2

#define CRTC_INDEX              0x3d4
#define CRTC_STARTHIGH          12

#define GC_INDEX                0x3ce
#define GC_MODE                 5
#define GC_BITMASK              8


extern const int16_t CENTERY;


static uint8_t __far* _s_screen;
static uint8_t __far* colors;
uint16_t colorsoffset;


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


static const int8_t palcolors[14] =
{
	0,									// normal
	4, 4, 4, 4, 0x1c, 0x1c, 0x1c, 0x1c,	// red
	6, 6, 0x1e, 0x1e,					// yellow
	2									// green
};


static void I_UploadNewPalette(int8_t pal)
{
	union REGS regs;
	regs.w.ax = 0x1000;
	regs.h.bl = 0x00;
	regs.h.bh = palcolors[pal];
	int86(0x10, &regs, &regs);
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(SCREEN_MODE);

	__djgpp_nearptr_enable();
	_s_screen = D_MK_FP(PAGE1, ((PLANEWIDTH - VIEWWINDOWWIDTH) / 2) + ((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * PLANEWIDTH + __djgpp_conventional_base);

	colorsoffset = (PAGE_SIZE * 2) << 4;
	colors = D_MK_FP(D_FP_SEG(_s_screen), colorsoffset + __djgpp_conventional_base); // PAGE3:0000
	volatile uint8_t __far* dst = colors;

	outp(SC_INDEX, SC_MAPMASK);

	for (int16_t y = 0; y < 16; y++)
	{
		for (int16_t x = 0; x < 16; x++)
		{
			int16_t plane = 1;
			for (int16_t p = 0; p < 4; p++)
			{
				uint8_t c;
				if (plane & x)
				{
					if (plane & y)
						c = 0xff;
					else
						c = 0x55;
				}
				else
				{
					if (plane & y)
						c = 0xaa;
					else
						c = 0;
				}
				outp(SC_INDEX + 1, plane);
				*dst = c;
				plane <<= 1;
			}
			dst++;
		}
	}
	outp(SC_INDEX + 1, 15);

	// set write mode 1
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, 1);
}


void I_ShutdownGraphics(void)
{
	I_SetScreenMode(3);
}


static int8_t newpal;


void I_SetPalette(int8_t pal)
{
	newpal = pal;
}


#define NO_PALETTE_CHANGE 100

static uint16_t st_needrefresh = 0;

void I_FinishUpdate(void)
{
	// palette
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	// status bar
	if (st_needrefresh)
	{
		st_needrefresh--;

		if (st_needrefresh != 2)
		{
			uint8_t __far* src = D_MK_FP(D_FP_SEG(_s_screen) - PAGE_SIZE, D_FP_OFF(_s_screen));
			if (D_FP_SEG(src) == PAGEMINUS1)
				src = D_MK_FP(PAGE2, D_FP_OFF(src));

			src += (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			uint8_t __far* dest = _s_screen + (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			for (int16_t y = 0; y < ST_HEIGHT; y++)
			{
				for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
				{
					volatile uint8_t loadLatches = *src++;
					*dest++ = 0;
				}
				src  += PLANEWIDTH - VIEWWINDOWWIDTH;
				dest += PLANEWIDTH - VIEWWINDOWWIDTH;
			}
		}
	}

	// page flip between segments
	// A000, A200 and A400 for 320x200
	// A000, A400 and A800 for 640x200
	outp(CRTC_INDEX, CRTC_STARTHIGH);
	outp(CRTC_INDEX + 1, D_FP_SEG(_s_screen) >> 4);
	_s_screen = D_MK_FP(D_FP_SEG(_s_screen) + PAGE_SIZE, D_FP_OFF(_s_screen));
	colorsoffset = D_FP_OFF(colors) - (PAGE_SIZE << 4);
	colors = D_MK_FP(D_FP_SEG(_s_screen), colorsoffset + __djgpp_conventional_base);
	if (D_FP_SEG(_s_screen) == PAGE3)
	{
		_s_screen = D_MK_FP(PAGE0, D_FP_OFF(_s_screen));
		colorsoffset = (PAGE_SIZE * 3) << 4 ;
		colors = D_MK_FP(D_FP_SEG(_s_screen), colorsoffset + __djgpp_conventional_base);
	}
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

uint8_t colormap[256];

static uint16_t colormapoffset = 0xffff;

const uint8_t __far* source;
uint8_t __far* dest;


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	const uint8_t __far* clrs = D_MK_FP(D_FP_SEG(dest), colorsoffset + __djgpp_conventional_base);
	volatile uint8_t loadLatches;
	uint8_t __far* dst = dest;

	int16_t l = count >> 4;
	while (l--)
	{
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;

		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;

		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;

		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case 14: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case 13: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case 12: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case 11: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case 10: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  9: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  8: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  7: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  6: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  5: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  4: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  3: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  2: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0; dst += PLANEWIDTH; frac += fracstep;
		case  1: loadLatches = clrs[colormap[source[frac >> COLBITS]]]; *dst = 0;
	}
}
#else
void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count);
#endif


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	if (colormapoffset != (uint16_t)dcvars->colormap)
	{
		memcpy(colormap, dcvars->colormap, 256);
		colormapoffset = (uint16_t)dcvars->colormap;
	}

	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


#if defined C_ONLY
static void R_DrawColumnFlat2(int16_t count)
{
	int16_t l = count >> 4;

	while (l--)
	{
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;

		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;

		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;

		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
		*dest = 0; dest += PLANEWIDTH;
	}

	switch (count & 15) {
		case 15: dest[PLANEWIDTH * 14] = 0;
		case 14: dest[PLANEWIDTH * 13] = 0;
		case 13: dest[PLANEWIDTH * 12] = 0;
		case 12: dest[PLANEWIDTH * 11] = 0;
		case 11: dest[PLANEWIDTH * 10] = 0;
		case 10: dest[PLANEWIDTH *  9] = 0;
		case  9: dest[PLANEWIDTH *  8] = 0;
		case  8: dest[PLANEWIDTH *  7] = 0;
		case  7: dest[PLANEWIDTH *  6] = 0;
		case  6: dest[PLANEWIDTH *  5] = 0;
		case  5: dest[PLANEWIDTH *  4] = 0;
		case  4: dest[PLANEWIDTH *  3] = 0;
		case  3: dest[PLANEWIDTH *  2] = 0;
		case  2: dest[PLANEWIDTH *  1] = 0;
		case  1: dest[PLANEWIDTH *  0] = 0;
	}
}
#else
void R_DrawColumnFlat2(int16_t count);
#endif

void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	volatile uint8_t loadLatches = colors[color];
	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	R_DrawColumnFlat2(count);
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

	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	static int16_t fuzzpos = 0;

	do
	{
		volatile uint8_t loadLatches = colors[fuzzcolors[fuzzpos]];
		*dest = 0;
		dest += PLANEWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while (--count);
}


void V_ClearViewWindow(void)
{
	outp(GC_INDEX + 1, 255);

	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
		_fmemset(&_s_screen[y * PLANEWIDTH], 0, VIEWWINDOWWIDTH);
}


void V_InitDrawLine(void)
{
	// set write mode 2
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, 2);

	outp(GC_INDEX, GC_BITMASK);
}


void V_ShutdownDrawLine(void)
{
	// set write mode 1
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, 1);
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	outp(GC_INDEX + 1, 128 >> (x0 & 7));

	while (true)
	{
		volatile uint8_t loadLatches = _s_screen[y0 * PLANEWIDTH + (x0 >> 3)];
		_s_screen[y0 * PLANEWIDTH + (x0 >> 3)] = color;

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;
			outp(GC_INDEX + 1, 128 >> (x0 & 7));
		}

		if (e2 <= dx)
		{
			err += dx;
			y0  += sy;
		}
	}
}


static int16_t cachedLumpNum;


static void V_Blit(int16_t num, uint16_t offset, int16_t height)
{
	if (cachedLumpNum == num)
	{
		uint8_t __far* src  = D_MK_FP(PAGE3 + (256 >> 4), 0 + __djgpp_conventional_base);
		uint8_t __far* dest = _s_screen + (offset / SCREENWIDTH) * PLANEWIDTH;
		for (int16_t y = 0; y < height; y++)
		{
			for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
			{
				volatile uint8_t loadLatches = *src++;
				*dest++ = 0;
			}
			src  += PLANEWIDTH - VIEWWINDOWWIDTH;
			dest += PLANEWIDTH - VIEWWINDOWWIDTH;
		}
	}
}


void V_DrawBackground(int16_t backgroundnum)
{
	if (cachedLumpNum != backgroundnum)
	{
		const byte __far* lump = W_GetLumpByNum(backgroundnum);

		volatile uint8_t __far* dest = D_MK_FP(PAGE3 + (256 >> 4), 0 + __djgpp_conventional_base);

		// set write mode 2
		outp(GC_INDEX, GC_MODE);
		outp(GC_INDEX + 1, 2);

		outp(GC_INDEX, GC_BITMASK);	

		for (int16_t y = 0; y < SCREENHEIGHT; y++)
		{
			for (int16_t x = 0; x < SCREENWIDTH; x++)
			{
#if VIEWWINDOWWIDTH == 30
				uint8_t c = lump[((y / 2) & 63) * 64 + (x & 63)];
#elif VIEWWINDOWWIDTH == 60
				uint8_t c = lump[( y      & 63) * 64 + (x & 63)];
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

				uint16_t offset = y * PLANEWIDTH + ((x * 2) >> 3);
				volatile uint8_t loadLatches;

				outp(GC_INDEX + 1, 128 >> ((x * 2 + 0) & 7));
				loadLatches = dest[offset];
				dest[offset] = c >> 4;

				outp(GC_INDEX + 1, 128 >> ((x * 2 + 1) & 7));
				loadLatches = dest[offset];
				dest[offset] = c;
			}
		}

		Z_ChangeTagToCache(lump);

		cachedLumpNum = backgroundnum;

		// set write mode 1
		outp(GC_INDEX, GC_MODE);
		outp(GC_INDEX + 1, 1);
	}

	V_Blit(backgroundnum, 0, SCREENHEIGHT);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	static int16_t cachedLumpHeight;

	if (cachedLumpNum != num)
	{
		const uint8_t __far* lump = W_TryGetLumpByNum(num);

		if (lump != NULL)
		{
			// set write mode 2
			outp(GC_INDEX, GC_MODE);
			outp(GC_INDEX + 1, 2);

			outp(GC_INDEX, GC_BITMASK);
			uint8_t bitmask = 128;

			uint16_t lumpLength = W_LumpLength(num);
			cachedLumpHeight = lumpLength / SCREENWIDTH;
			uint8_t __far* src  = (uint8_t __far*)lump;
			volatile uint8_t __far* dest = D_MK_FP(PAGE3 + (256 >> 4), 0 + __djgpp_conventional_base);
			for (int16_t y = 0; y < cachedLumpHeight; y++)
			{
				for (int16_t x = 0; x < SCREENWIDTH; x++)
				{
					volatile uint8_t loadLatches;
					uint8_t c = *src++;
					outp(GC_INDEX + 1, bitmask);
					loadLatches = *dest;

#if VIEWWINDOWWIDTH == 30

#elif VIEWWINDOWWIDTH == 60
					*dest = c >> 4;
					bitmask >>= 1;
					outp(GC_INDEX + 1, bitmask);
					loadLatches = *dest;
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

					*dest = c;
					bitmask >>= 1;
					if (!bitmask)
					{
						bitmask = 128;
						dest++;
					}
				}
				dest += PLANEWIDTH - VIEWWINDOWWIDTH;
			}

			Z_ChangeTagToCache(lump);
			cachedLumpNum = num;

			// set write mode 1
			outp(GC_INDEX, GC_MODE);
			outp(GC_INDEX + 1, 1);
		}
	}

	V_Blit(num, offset, cachedLumpHeight);
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
	{
		ST_doRefresh();
		st_needrefresh = 3; //3 screen pages
	}
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;
	x *= SCALE_FACTOR;

	byte __far* desttop = _s_screen + (y * PLANEWIDTH) + (x >> 3);

	int16_t width = patch->width;

	// set write mode 2
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, 2);

	outp(GC_INDEX, GC_BITMASK);
	uint8_t bitmask = 128 >> (x & 7);

	for (int16_t col = 0; col < width; col++)
	{
		const column_t __far* column;

#if VIEWWINDOWWIDTH == 30

#elif VIEWWINDOWWIDTH == 60
		outp(GC_INDEX + 1, bitmask);

		column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * PLANEWIDTH);

			volatile uint8_t loadLatches;
			uint16_t count = column->length;

			if (count == 7)
			{
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4;
			}
			else if (count == 3)
			{
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4;
			}
			else if (count == 5)
			{
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4;
			}
			else if (count == 6)
			{
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4;
			}
			else if (count == 2)
			{
				loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = (*source++) >> 4;
			}
			else
			{
				while (count--)
				{
					loadLatches = *dest; *dest = (*source++) >> 4; dest += PLANEWIDTH;
				}
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}

		bitmask >>= 1;
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

		outp(GC_INDEX + 1, bitmask);

		column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * PLANEWIDTH);

			volatile uint8_t loadLatches;
			uint16_t count = column->length;

			if (count == 7)
			{
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++;
			}
			else if (count == 3)
			{
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++;
			}
			else if (count == 5)
			{
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++;
			}
			else if (count == 6)
			{
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++;
			}
			else if (count == 2)
			{
				loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				loadLatches = *dest; *dest = *source++;
			}
			else
			{
				while (count--)
				{
					loadLatches = *dest; *dest = *source++; dest += PLANEWIDTH;
				}
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}

		bitmask >>= 1;
		if (!bitmask)
		{
			bitmask = 128;
			desttop++;
		}
	}

	// set write mode 1
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, 1);
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	static const int32_t DX  = (((int32_t)SCREENWIDTH)<<FRACBITS) / SCREENWIDTH_VGA;
	static const int16_t DXI = ((((int32_t)SCREENWIDTH_VGA)<<FRACBITS) / SCREENWIDTH) >> 8;
	static const int32_t DY  = ((((int32_t)SCREENHEIGHT)<<FRACBITS)+(FRACUNIT-1)) / SCREENHEIGHT_VGA;
	static const int16_t DYI = ((((int32_t)SCREENHEIGHT_VGA)<<FRACBITS) / SCREENHEIGHT) >> 8;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	const int16_t left   = ( x * DX ) >> FRACBITS;
	const int16_t right  = ((x + patch->width)  * DX) >> FRACBITS;
	const int16_t bottom = ((y + patch->height) * DY) >> FRACBITS;

	uint16_t col = 0;

	// set write mode 2
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, 2);

	outp(GC_INDEX, GC_BITMASK);

	for (int16_t dc_x = left; dc_x < right; dc_x++, col += DXI)
	{
		if (dc_x < 0)
			continue;
		else if (dc_x >= SCREENWIDTH)
			break;

		const column_t __far* column;
		column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col >> 8]);

		uint8_t bitmask = 128 >> ((dc_x * SCALE_FACTOR) & 7);
#if VIEWWINDOWWIDTH == 30

#elif VIEWWINDOWWIDTH == 60
		outp(GC_INDEX + 1, bitmask);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte __far* dest = _s_screen + (dc_yl * PLANEWIDTH) + ((dc_x * SCALE_FACTOR) >> 3);

			int16_t frac = 0;

			const byte __far* source = (const byte __far*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				volatile uint8_t loadLatches = *dest;
				*dest = source[frac >> 8] >> 4;
				dest += PLANEWIDTH;
				frac += DYI;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}

		column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col >> 8]);
		bitmask >>= 1;
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

		outp(GC_INDEX + 1, bitmask);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte __far* dest = _s_screen + (dc_yl * PLANEWIDTH) + ((dc_x * SCALE_FACTOR) >> 3);

			int16_t frac = 0;

			const byte __far* source = (const byte __far*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				volatile uint8_t loadLatches = *dest;
				*dest = source[frac >> 8];
				dest += PLANEWIDTH;
				frac += DYI;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}

	// set write mode 1
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, 1);
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
			if (wipe_y_lookup[i] < SCREENHEIGHT)
			{
				int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : SCREENHEIGHT / 25;
				// At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
					dy = SCREENHEIGHT - wipe_y_lookup[i];

				uint8_t __far* s = &frontbuffer[i] + ((SCREENHEIGHT - 1 - dy) * PLANEWIDTH);
				uint8_t __far* d = &frontbuffer[i] + ((SCREENHEIGHT - 1)      * PLANEWIDTH);

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -PLANEWIDTH;
					s += -PLANEWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * PLANEWIDTH;
				d = &frontbuffer[i] + wipe_y_lookup[i] * PLANEWIDTH;

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
	frontbuffer = D_MK_FP(D_FP_SEG(_s_screen) - PAGE_SIZE, D_FP_OFF(_s_screen));
	if (D_FP_SEG(frontbuffer) == PAGEMINUS1)
		frontbuffer = D_MK_FP(PAGE2, D_FP_OFF(frontbuffer));

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
