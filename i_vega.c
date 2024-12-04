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


#if VIEWWINDOWWIDTH == 40
#define SCREEN_MODE	0x0d
#elif VIEWWINDOWWIDTH == 80
#define SCREEN_MODE 0x0e
#else
#error unsupported VIEWWINDOWWIDTH value
#endif


#define PAGE_SIZE (((VIEWWINDOWWIDTH * 200 + 511) & ~511) >> 4)

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


extern const int16_t CENTERY;


static uint8_t __far* _s_screen;
static uint8_t __far* colors;


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
	I_ReloadPalette();
	I_UploadNewPalette(0);

	__djgpp_nearptr_enable();
	_s_screen = D_MK_FP(PAGE1, ((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * VIEWWINDOWWIDTH + __djgpp_conventional_base);

	colors = D_MK_FP(PAGE3, 0 + __djgpp_conventional_base);
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

			src += (SCREENHEIGHT - ST_HEIGHT) * VIEWWINDOWWIDTH;
			uint8_t __far* dest = _s_screen + (SCREENHEIGHT - ST_HEIGHT) * VIEWWINDOWWIDTH;
			for (int16_t i = 0; i < VIEWWINDOWWIDTH * ST_HEIGHT; i++)
			{
				volatile uint8_t loadLatches = *src++;
				*dest++ = 0;
			}
		}
	}

	// page flip between segments
	// A000, A200 and A400 for 320x200
	// A000, A400 and A800 for 640x200
	outp(CRTC_INDEX, CRTC_STARTHIGH);
	outp(CRTC_INDEX + 1, D_FP_SEG(_s_screen) >> 4);
	_s_screen = D_MK_FP(D_FP_SEG(_s_screen) + PAGE_SIZE, D_FP_OFF(_s_screen));
	if (D_FP_SEG(_s_screen) == PAGE3)
		_s_screen = D_MK_FP(PAGE0, D_FP_OFF(_s_screen));
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

static uint8_t nearcolormap[256];

#define L_FP_OFF D_FP_OFF
static uint16_t nearcolormapoffset = 0xffff;

static const uint8_t __far* source;
static uint8_t __far* dest;


static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	volatile uint8_t loadLatches;

	int16_t l = count >> 4;
	while (l--)
	{
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;

		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;

		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;

		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 14: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 13: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 12: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 11: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 10: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  9: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  8: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  7: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  6: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  5: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  4: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  3: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  2: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  1: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0;
	}
}


void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	if (nearcolormapoffset != L_FP_OFF(dcvars->colormap))
	{
		_fmemcpy(nearcolormap, dcvars->colormap, 256);
		nearcolormapoffset = L_FP_OFF(dcvars->colormap);
	}

	dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	volatile uint8_t loadLatches = colors[color];
	uint8_t __far* dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	while (count--)
	{
		*dest = 0;
		dest += VIEWWINDOWWIDTH;
	}
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

	uint8_t __far* dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	static int16_t fuzzpos = 0;

	do
	{
		volatile uint8_t loadLatches = colors[fuzzcolors[fuzzpos]];
		*dest = 0;
		dest += VIEWWINDOWWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while (--count);
}


void V_FillRect(byte colour)
{
	uint8_t __far* dest = _s_screen;
	volatile uint8_t loadLatches = colors[colour];
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dest++ = 0;
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	volatile uint8_t loadLatches = colors[color];

	while (true)
	{
		_s_screen[y0 * VIEWWINDOWWIDTH + x0] = 0;

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
	uint8_t __far* dest = _s_screen;

	for (int16_t y = 0; y < SCREENHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			volatile uint8_t loadLatches = colors[src[(y & 63) * 64 + ((x * 4) & 63)]];
			*dest++ = 0;
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	static int16_t cachedLumpNum;
	static int16_t cachedLumpHeight;

	if (cachedLumpNum != num)
	{
		const uint8_t __far* lump = W_TryGetLumpByNum(num);

		if (lump != NULL)
		{
			uint16_t lumpLength = W_LumpLength(num);
			cachedLumpHeight = lumpLength / SCREENWIDTH;
			uint8_t __far* src  = (uint8_t __far*)lump;
			uint8_t __far* dest = D_MK_FP(PAGE3 + (256 >> 4), 0 + __djgpp_conventional_base);
			for (int16_t i = 0; i < VIEWWINDOWWIDTH * cachedLumpHeight; i++)
			{
				volatile uint8_t loadLatches = colors[*src];
				*dest++ = 0;
				src += (SCREENWIDTH / VIEWWINDOWWIDTH);
			}

			Z_ChangeTagToCache(lump);
			cachedLumpNum = num;
		}
	}

	if (cachedLumpNum == num)
	{
		uint8_t __far* src  = D_MK_FP(PAGE3 + (256 >> 4), 0 + __djgpp_conventional_base);
		uint8_t __far* dest = _s_screen + (offset / SCREENWIDTH) * VIEWWINDOWWIDTH;
		for (int16_t i = 0; i < VIEWWINDOWWIDTH * cachedLumpHeight; i++)
		{
			volatile uint8_t loadLatches = *src++;
			*dest++ = 0;
		}
	}
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

	byte __far* desttop = _s_screen + (y * VIEWWINDOWWIDTH) + (x / (SCREENWIDTH / VIEWWINDOWWIDTH));

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col += (SCREENWIDTH / VIEWWINDOWWIDTH), desttop++)
	{
		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * VIEWWINDOWWIDTH);

			volatile uint8_t loadLatches;
			uint16_t count = column->length;

			uint16_t l = count >> 2;
			while (l--)
			{
				loadLatches = colors[*source++]; *dest = 0; dest += VIEWWINDOWWIDTH;
				loadLatches = colors[*source++]; *dest = 0; dest += VIEWWINDOWWIDTH;
				loadLatches = colors[*source++]; *dest = 0; dest += VIEWWINDOWWIDTH;
				loadLatches = colors[*source++]; *dest = 0; dest += VIEWWINDOWWIDTH;
			}

			switch (count & 3)
			{
				case 3: loadLatches = colors[*source++]; *dest = 0; dest += VIEWWINDOWWIDTH;
				case 2: loadLatches = colors[*source++]; *dest = 0; dest += VIEWWINDOWWIDTH;
				case 1: loadLatches = colors[*source++]; *dest = 0;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	static const int32_t DX  = (((int32_t)VIEWWINDOWWIDTH)<<FRACBITS) / SCREENWIDTH_VGA;
	static const int16_t DXI = ((((int32_t)SCREENWIDTH_VGA)<<FRACBITS) / VIEWWINDOWWIDTH) >> 8;
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
		else if (dc_x >= VIEWWINDOWWIDTH)
			break;

		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col >> 8]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte __far* dest = _s_screen + (dc_yl * VIEWWINDOWWIDTH) + dc_x;

			int16_t frac = 0;

			const byte __far* source = (const byte __far*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				volatile uint8_t loadLatches = colors[source[frac >> 8]];
				*dest = 0;
				dest += VIEWWINDOWWIDTH;
				frac += DYI;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
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

				uint8_t __far* s = &frontbuffer[i] + ((SCREENHEIGHT - 1 - dy) * VIEWWINDOWWIDTH);
				uint8_t __far* d = &frontbuffer[i] + ((SCREENHEIGHT - 1)      * VIEWWINDOWWIDTH);

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -VIEWWINDOWWIDTH;
					s += -VIEWWINDOWWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * VIEWWINDOWWIDTH;
				d = &frontbuffer[i] + wipe_y_lookup[i] * VIEWWINDOWWIDTH;

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
