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
 *      Video code for MDA 80x25 monochrome
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

static uint8_t __far* _s_screen;
static uint8_t __far* videomemory;


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


void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(7);

	__djgpp_nearptr_enable();
	videomemory = D_MK_FP(0xb000, 0 + __djgpp_conventional_base);

	_s_screen = Z_MallocStatic(PLANEWIDTH * VIEWWINDOWHEIGHT);
	_fmemset(_s_screen, 7, PLANEWIDTH * VIEWWINDOWHEIGHT);
}


static const uint8_t LUT[256] =
{
	0x00, 0x00, 0x00, 0xb0, 0xdb, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0xb0, 0xb0, 0xb0,
	0xdb, 0xdb, 0xdb, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1,
	0xb1, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2,
	0xb2, 0xb2, 0xb2, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0x00, 0x00,
	0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2,
	0xb2, 0xb2, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0x00, 0x00,
	0xdb, 0xdb, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb1, 0xb1, 0xb1, 0xb0, 0xb0, 0xb0, 0x00, 0x00, 0x00,
	0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1, 0xb0, 0xb0, 0xb0,
	0xb2, 0xb2, 0xb1, 0xb1, 0xb1, 0xb0, 0xb0, 0xb0, 0xb2, 0xb1, 0xb1, 0xb1, 0xb1, 0xb0, 0xb0, 0xb0,
	0xdb, 0xdb, 0xdb, 0xb2, 0xb2, 0xb1, 0xb1, 0xb0, 0xdb, 0xdb, 0xdb, 0xdb, 0xb2, 0xb2, 0xb1, 0xb1,
	0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xdb, 0xdb, 0xdb, 0xb2, 0xb2, 0xb1, 0xb0, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb1, 0xb1, 0xb1, 0xb1, 0xb1,
	0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xb1, 0xb1, 0xb0, 0xb0, 0xb0, 0xb0, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb2, 0xdb, 0xb2, 0xb1, 0xb1, 0xb0, 0x00, 0xb2
};


static void I_DrawBuffer(uint8_t __far* buffer)
{
	_fmemcpy(videomemory, buffer, PLANEWIDTH * VIEWWINDOWHEIGHT);
}


void I_ShutdownGraphicsHardwareSpecificCode(void)
{
	// Do nothing
}


void I_SetPalette(int8_t pal)
{
	// Do nothing
}


void I_FinishUpdate(void)
{
	I_DrawBuffer(_s_screen);
}



#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

uint8_t nearcolormap[256];
static uint16_t nearcolormapoffset = 0xffff;

const uint8_t __far* source;
uint8_t __far* dest;


static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	switch (count)
	{
		case 25: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 24: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 23: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 22: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 21: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 20: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 19: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 18: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 17: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 16: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 15: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 14: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 13: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 12: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 11: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case 10: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  9: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  8: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  7: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  6: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  5: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  4: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  3: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  2: *dest = LUT[nearcolormap[source[frac >> COLBITS]]]; dest += PLANEWIDTH; frac += fracstep;
		case  1: *dest = LUT[nearcolormap[source[frac >> COLBITS]]];
	}
}


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


void R_DrawColumnFlat(uint8_t col, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	uint8_t color = LUT[col];

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
#define FUZZCOLOR2 0xb0
#define FUZZCOLOR3 0xb1
#define FUZZCOLOR4 0xb2
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
	uint8_t color = LUT[colour];
	uint8_t __far* dest = _s_screen;
	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			*dest++ = color;
			dest++;
		}
	}
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t col)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	uint8_t color = LUT[col];

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
			*dest++ = LUT[src[((y * 2) & 63) * 64 + ((x * 2) & 63)]];
			dest++;
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
		static const int16_t DXI = SCREENWIDTH / VIEWWINDOWWIDTH;
		static const fixed_t DYI = ((fixed_t)SCREENHEIGHT << FRACBITS) / VIEWWINDOWHEIGHT;
		fixed_t y = 0;
		uint8_t __far* dst = _s_screen;
		for (int16_t h = 0; h < VIEWWINDOWHEIGHT; h++)
		{
			int16_t x = 0;
			for (int16_t w = 0; w < VIEWWINDOWWIDTH; w++)
			{
				*dst = LUT[lump[(y >> FRACBITS) * SCREENWIDTH + x]];
				x += DXI;
				dst += 2;
			}
			y += DYI;
		}
		Z_ChangeTagToCache(lump);
	}
}


void V_DrawCharacter(int16_t x, int16_t y, uint8_t color, char c)
{
	UNUSED(color);
	_s_screen[y * PLANEWIDTH + (x << 1)] = c;
}


void V_DrawCharacterForeground(int16_t x, int16_t y, uint8_t color, char c)
{
	V_DrawCharacter(x, y, color, c);
}


void V_DrawString(int16_t x, int16_t y, uint8_t color, const char* s)
{
	UNUSED(color);

	while (*s)
	{
		V_DrawCharacter(x, y, color, *s);
		x++;
		s++;
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


void wipe_StartScreen(void)
{
	// Do nothing
}


void D_Wipe(void)
{
	// Do nothing
}
