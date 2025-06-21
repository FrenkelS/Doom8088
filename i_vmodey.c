/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023-2025 Frenkel Smeijers
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
 *      Video code for VGA Mode Y 320x200 256 colors
 *      Effective resolutions  60x128
 *                            120x128
 *                            240x128
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


#define PLANEWIDTH 80


#define PAGE_SIZE 0x0400

#define PAGE0		0xa000
#define PAGE1		(PAGE0+PAGE_SIZE)
#define PAGE2		(PAGE1+PAGE_SIZE)
#define PAGE3		(PAGE2+PAGE_SIZE)
#define PAGEMINUS1	(PAGE0-PAGE_SIZE)


#define SC_INDEX                0x3c4
#define SC_MAPMASK              2
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3d4
#define CRTC_STARTHIGH          12
#define CRTC_UNDERLINE          20
#define CRTC_MODE               23

#define GC_INDEX                0x3ce
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS        6

#define PEL_WRITE_ADR   0x3c8
#define PEL_DATA        0x3c9


extern const int16_t CENTERY;


static uint8_t  __far* _s_screen;


static int16_t palettelumpnum;


void I_ReloadPalette(void)
{
	char lumpName[9] = "PLAYPAL0";

	if (_g_gamma == 0)
		lumpName[7] = 0;
	else
		lumpName[7] = '0' + _g_gamma;

	palettelumpnum = W_GetNumForName(lumpName);
}


static const uint8_t colors[14][3] =
{
	// normal
	{0, 0, 0},

	// red
	{0x07, 0, 0},
	{0x0e, 0, 0},
	{0x15, 0, 0},
	{0x1c, 0, 0},
	{0x23, 0, 0},
	{0x2a, 0, 0},
	{0x31, 0, 0},
	{0x3b, 0, 0},

	// yellow
	{0x06, 0x05, 0x02},
	{0x0d, 0x0b, 0x04},
	{0x14, 0x11, 0x06},
	{0x1a, 0x17, 0x08},

	// green
	{0, 0x08, 0}
};


static void I_UploadNewPalette(int8_t pal)
{
	const uint8_t __far* palette_lump = W_TryGetLumpByNum(palettelumpnum);
	if (palette_lump != NULL)
	{
		const byte __far* palette = &palette_lump[pal * 256 * 3];
		outp(PEL_WRITE_ADR, 0);
		for (int_fast16_t i = 0; i < 256 * 3; i++)
			outp(PEL_DATA, (*palette++) >> 2);

		Z_ChangeTagToCache(palette_lump);
	}
	else
	{
		outp(PEL_WRITE_ADR, 0);
		outp(PEL_DATA, colors[pal][0]);
		outp(PEL_DATA, colors[pal][1]);
		outp(PEL_DATA, colors[pal][2]);
	}
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(0x13);
	I_ReloadPalette();
	I_UploadNewPalette(0);

	__djgpp_nearptr_enable();
	_s_screen = D_MK_FP(PAGE1, ((PLANEWIDTH - (SCREENWIDTH / 4)) / 2) + ((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * PLANEWIDTH + __djgpp_conventional_base);

	outp(SC_INDEX, SC_MEMMODE);
	outp(SC_INDEX + 1, (inp(SC_INDEX + 1) & ~8) | 4);

	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~0x13);

	outp(GC_INDEX, GC_MISCELLANEOUS);
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~2);

	outp(SC_INDEX, SC_MAPMASK);
#if VIEWWINDOWWIDTH == 60
	outp(SC_INDEX + 1, 15);
#endif

	_fmemset(D_MK_FP(PAGE0, 0 + __djgpp_conventional_base), 0, 0xffff);

	outp(CRTC_INDEX, CRTC_UNDERLINE);
	outp(CRTC_INDEX + 1,inp(CRTC_INDEX + 1) & ~0x40);

	outp(CRTC_INDEX, CRTC_MODE);
	outp(CRTC_INDEX + 1, inp(CRTC_INDEX + 1) | 0x40);
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
#if VIEWWINDOWWIDTH != 60
			outp(SC_INDEX + 1, 15);
#endif

			// set write mode 1
			outp(GC_INDEX, GC_MODE);
			outp(GC_INDEX + 1, inp(GC_INDEX + 1) | 1);

#if defined _M_I86
			uint8_t __far* src = D_MK_FP(D_FP_SEG(_s_screen) - PAGE_SIZE, D_FP_OFF(_s_screen));
			if (D_FP_SEG(src) == PAGEMINUS1)
				src = D_MK_FP(PAGE2, D_FP_OFF(src));
#else
			uint8_t __far* src = _s_screen - (PAGE_SIZE << 4);
			if ((((uint32_t)src) & (PAGEMINUS1 << 4)) == (PAGEMINUS1 << 4))
				src += (0x10000 - (PAGE_SIZE << 4));
#endif
			src += (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			uint8_t __far* dest = _s_screen + (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			for (int16_t y = 0; y < ST_HEIGHT; y++)
			{
				for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
				{
					volatile uint8_t loadLatches = src[y * PLANEWIDTH + x];
					dest[y * PLANEWIDTH + x] = 0;
				}
			}

			// set write mode 0
			outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~1);
		}
	}

	// page flip between segments A000, A400 and A800
	outp(CRTC_INDEX, CRTC_STARTHIGH);
#if defined _M_I86
	outp(CRTC_INDEX + 1, D_FP_SEG(_s_screen) >> 4);
	_s_screen = D_MK_FP(D_FP_SEG(_s_screen) + PAGE_SIZE, D_FP_OFF(_s_screen));
	if (D_FP_SEG(_s_screen) == PAGE3)
		_s_screen = D_MK_FP(PAGE0, D_FP_OFF(_s_screen));
#else
	outp(CRTC_INDEX + 1, (D_FP_SEG(_s_screen) >> 4) & 0xf0);
	_s_screen += (PAGE_SIZE << 4);
	if ((((uint32_t)_s_screen) & (PAGE3 << 4)) == (PAGE3 << 4))
		_s_screen -= (0x10000 - (PAGE_SIZE << 4));
#endif
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

const uint8_t* colormap;

const uint8_t __far* source;
uint8_t __far* dest;


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	int16_t l = count >> 4;
	while (l--)
	{
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
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

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	colormap = dcvars->colormap;

#if VIEWWINDOWWIDTH == 60
	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;
#elif VIEWWINDOWWIDTH == 120
	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x / 2;
	outp(SC_INDEX + 1, 3 << (2 * (dcvars->x & 1)));
#elif VIEWWINDOWWIDTH == 240
	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x / 4;
	outp(SC_INDEX + 1, 1 << (dcvars->x & 3));
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


#if defined C_ONLY
static void R_DrawColumnFlat2(uint8_t color, uint8_t dontcare, int16_t count)
{
	UNUSED(dontcare);

	int16_t l = count >> 4;

	while (l--)
	{
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;

		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;

		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;

		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
		*dest = color; dest += PLANEWIDTH;
	}
	
	switch (count & 15)
	{
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
void R_DrawColumnFlat2(uint8_t color, uint8_t dontcare, int16_t count);
#endif


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

#if VIEWWINDOWWIDTH == 60
	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;
#elif VIEWWINDOWWIDTH == 120
	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x / 2;
	outp(SC_INDEX + 1, 3 << (2 * (dcvars->x & 1)));
#elif VIEWWINDOWWIDTH == 240
	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x / 4;
	outp(SC_INDEX + 1, 1 << (dcvars->x & 3));
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

	R_DrawColumnFlat2(color, color, count);
}


#define FUZZOFF (PLANEWIDTH)
#define FUZZTABLE 50

static const int8_t fuzzoffset[FUZZTABLE] =
{
	FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t dc_yl = dcvars->yl;
	int16_t dc_yh = dcvars->yh;

	// Adjust borders. Low...
	if (dc_yl <= 0)
		dc_yl = 1;

	// .. and high.
	if (dc_yh >= VIEWWINDOWHEIGHT - 1)
		dc_yh = VIEWWINDOWHEIGHT - 2;

	int16_t count = (dc_yh - dc_yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	colormap = &fullcolormap[6 * 256];

#if VIEWWINDOWWIDTH == 60
	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;
#elif VIEWWINDOWWIDTH == 120
	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x / 2;
	outp(SC_INDEX + 1, 3 << (2 * (dcvars->x & 1)));
	outp(GC_INDEX, GC_READMAP);
	outp(GC_INDEX + 1, 2 * (dcvars->x & 1));
#elif VIEWWINDOWWIDTH == 240
	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x / 4;
	outp(SC_INDEX + 1, 1 << (dcvars->x & 3));
	outp(GC_INDEX, GC_READMAP);
	outp(GC_INDEX + 1, dcvars->x & 3);
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

	static int16_t fuzzpos = 0;

	do
	{
		*dest = colormap[dest[fuzzoffset[fuzzpos]]];
		dest += PLANEWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while (--count);
}


void V_ClearViewWindow(void)
{
#if VIEWWINDOWWIDTH != 60
		outp(SC_INDEX + 1, 15);
#endif

	for (int16_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
		_fmemset(_s_screen + y * PLANEWIDTH, 0, SCREENWIDTH / 4);
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

	outp(SC_INDEX + 1, 1 << (x0 & 3));

	while (true)
	{
		_s_screen[y0 * PLANEWIDTH + x0 / 4] = color;

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;
			outp(SC_INDEX + 1, 1 << (x0 & 3));
		}

		if (e2 <= dx)
		{
			err += dx;
			y0  += sy;
		}
	}

#if VIEWWINDOWWIDTH == 60
	outp(SC_INDEX + 1, 15);
#endif
}


static int16_t cachedLumpNum;


static void V_Blit(int16_t num, uint16_t offset, int16_t height)
{
	if (cachedLumpNum == num)
	{
#if VIEWWINDOWWIDTH != 60
		outp(SC_INDEX + 1, 15);
#endif

		// set write mode 1
		outp(GC_INDEX, GC_MODE);
		outp(GC_INDEX + 1, inp(GC_INDEX + 1) | 1);

		uint8_t __far* src  = D_MK_FP(PAGE3, 0 + __djgpp_conventional_base);
		uint8_t __far* dest = _s_screen + (offset / SCREENWIDTH) * PLANEWIDTH;
		for (int16_t y = 0; y < height; y++)
		{
			for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
			{
				volatile uint8_t loadLatches = src[y * PLANEWIDTH + x];
				dest[y * PLANEWIDTH + x] = 0;
			}
		}

		// set write mode 0
		outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~1);
	}
}


void V_DrawBackground(int16_t backgroundnum)
{
	if (cachedLumpNum != backgroundnum)
	{
		const uint8_t __far* lump = W_GetLumpByNum(backgroundnum);

		for (int16_t plane = 0; plane < 4; plane++)
		{
			outp(SC_INDEX + 1, 1 << plane);
			for (int16_t y = 0; y < SCREENHEIGHT; y++)
			{
				uint8_t __far* dest = D_MK_FP(PAGE3, y * PLANEWIDTH + __djgpp_conventional_base);
				for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
				{
					*dest++ = lump[(y & 63) * 64 + (((x * 4) + plane) & 63)];
				}
			}
		}
#if VIEWWINDOWWIDTH == 60
		outp(SC_INDEX + 1, 15);
#endif

		Z_ChangeTagToCache(lump);

		cachedLumpNum = backgroundnum;
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
			uint16_t lumpLength = W_LumpLength(num);
			cachedLumpHeight = lumpLength / SCREENWIDTH;
			for (int16_t plane = 0; plane < 4; plane++)
			{
				outp(SC_INDEX + 1, 1 << plane);
				for (int16_t y = 0; y < cachedLumpHeight; y++)
				{
					uint8_t __far* dest = D_MK_FP(PAGE3, y * PLANEWIDTH + __djgpp_conventional_base);
					for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
					{
						*dest++ = lump[y * SCREENWIDTH + (x * 4) + plane];
					}
				}
			}
#if VIEWWINDOWWIDTH == 60
			outp(SC_INDEX + 1, 15);
#endif
			Z_ChangeTagToCache(lump);

			cachedLumpNum = num;
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

	int16_t plane = x & 3;
	byte __far* desttop = _s_screen + (y * PLANEWIDTH) + x / 4;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++)
	{
		outp(SC_INDEX + 1, 1 << plane);

		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * PLANEWIDTH);

			uint16_t count = column->length;

			if (count == 7)
			{
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++;
			}
			else if (count == 3)
			{
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++;
			}
			else if (count == 5)
			{
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++;
			}
			else if (count == 6)
			{
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++;
			}
			else if (count == 2)
			{
				*dest = *source++; dest += PLANEWIDTH;
				*dest = *source++;
			}
			else
			{
				while (count--)
				{
					*dest = *source++; dest += PLANEWIDTH;
				}
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}

		if (++plane == 4)
		{
			plane = 0;
			desttop++;
		}
	}

#if VIEWWINDOWWIDTH == 60
	outp(SC_INDEX + 1, 15);
#endif
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

	for (int16_t dc_x = left; dc_x < right; dc_x++, col += DXI)
	{
		if (dc_x < 0)
			continue;
		else if (dc_x >= SCREENWIDTH)
			break;

		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col >> 8]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			outp(SC_INDEX + 1, 1 << (dc_x & 3));
			byte __far* dest = _s_screen + (dc_yl * PLANEWIDTH) + dc_x / 4;

			int16_t frac = 0;

			const byte __far* source = (const byte __far*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				*dest = source[frac >> 8];
				dest += PLANEWIDTH;
				frac += DYI;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}

#if VIEWWINDOWWIDTH == 60
	outp(SC_INDEX + 1, 15);
#endif
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

	// set write mode 1
	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) | 1);

	while (ticks--)
	{
		for (int16_t i = 0; i < SCREENWIDTH / 4; i++)
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

	// set write mode 0
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~1);

	return done;
}


static void wipe_initMelt()
{
	wipe_y_lookup = Z_MallocStatic((SCREENWIDTH / 4) * sizeof(int16_t));

	// setup initial column positions (y<0 => not ready to scroll yet)
	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int16_t i = 1; i < SCREENWIDTH / 4; i++)
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
#if defined _M_I86
	frontbuffer = D_MK_FP(D_FP_SEG(_s_screen) - PAGE_SIZE, D_FP_OFF(_s_screen));
	if (D_FP_SEG(frontbuffer) == PAGEMINUS1)
		frontbuffer = D_MK_FP(PAGE2, D_FP_OFF(frontbuffer));
#else
	frontbuffer	= _s_screen - (PAGE_SIZE << 4);
	if ((((uint32_t)frontbuffer) & (PAGEMINUS1 << 4)) == (PAGEMINUS1 << 4))
		frontbuffer += (0x10000 - (PAGE_SIZE << 4));
#endif

#if VIEWWINDOWWIDTH != 60
	outp(SC_INDEX + 1, 15);
#endif

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
