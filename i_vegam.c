/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023-2024 Frenkel Smeijers
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
 *      Video code for EGA 320x200 16 colors
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


#define PLANEWIDTH 40

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
	I_SetScreenMode(0x0d);
	I_ReloadPalette();
	I_UploadNewPalette(0);

	__djgpp_nearptr_enable();
	_s_screen = D_MK_FP(0xa200, (((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * SCREENWIDTH_VGA) / 8 + __djgpp_conventional_base);

	colors = D_MK_FP(0xa600, 0 + __djgpp_conventional_base);
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
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) | 1);
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
			uint8_t __far* src = (uint8_t __far*)(((uint32_t)_s_screen) - 0x02000000);
			if ((((uint32_t)src) & 0x9e000000) == 0x9e000000)
				src = (uint8_t __far*)(((uint32_t)src) + 0x06000000);

			src += (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			uint8_t __far* dest = _s_screen + (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			for (int16_t i = 0; i < VIEWWINDOWWIDTH * ST_HEIGHT; i++)
			{
				volatile uint8_t loadLatches = *src++;
				*dest++ = 0;
			}
		}
	}

	// page flip between segments A000, A200 and A400
	outp(CRTC_INDEX, CRTC_STARTHIGH);
	outp(CRTC_INDEX + 1, D_FP_SEG(_s_screen) >> 4);
	_s_screen = (uint8_t __far*)(((uint32_t)_s_screen) + 0x02000000);
	if ((((uint32_t)_s_screen) & 0xa6000000) == 0xa6000000)
		_s_screen = (uint8_t __far*)(((uint32_t)_s_screen) - 0x06000000);
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
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;

		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;

		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;

		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case 14: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case 13: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case 12: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case 11: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case 10: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  9: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  8: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  7: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  6: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  5: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  4: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  3: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
		case  2: loadLatches = colors[nearcolormap[source[frac >> COLBITS]]]; *dest = 0; dest += PLANEWIDTH; frac += fracstep;
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

	dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

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
	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	while (count--)
	{
		*dest = 0;
		dest += PLANEWIDTH;
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


void V_FillRect(byte colour)
{
	// TODO implement me
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	// TODO implement me
}


void V_DrawBackground(void)
{
	// TODO implement me
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
			uint8_t __far* dest = D_MK_FP(0xa610, 0 + __djgpp_conventional_base);
			for (int16_t i = 0; i < VIEWWINDOWWIDTH * cachedLumpHeight; i++)
			{
				volatile uint8_t loadLatches = colors[*src];
				*dest++ = 0;
				src += 6;
			}

			Z_ChangeTagToCache(lump);
			cachedLumpNum = num;
		}
	}

	if (cachedLumpNum == num)
	{
		uint8_t __far* src  = D_MK_FP(0xa610, 0 + __djgpp_conventional_base);
		uint8_t __far* dest = _s_screen + (offset / SCREENWIDTH) * PLANEWIDTH;
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
	// TODO implement me
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	// TODO implement me
}


void wipe_StartScreen(void)
{
	// TODO implement me
}


void D_Wipe(void)
{
	// TODO implement me
}
