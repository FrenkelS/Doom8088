/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2025 Frenkel Smeijers
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
 *      Video code for CGA 640x200 2 color
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


#define PLANEWIDTH        80
#define SCREENHEIGHT_CGA 200


extern const int16_t CENTERY;


static uint8_t __far* _s_screen;
static uint8_t __far* videomemory;


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
}


static const uint8_t colors[14] =
{
	15,							// normal
	12, 12, 12, 12, 4, 4, 4, 4,	// red
	14, 14, 6, 6,				// yellow
	10							// green
};


static void I_UploadNewPalette(int8_t pal)
{
	uint8_t color = colors[pal];

	if (videocard == CGA)
		outp(0x3d9, color);
	else
	{
		if (color > 7)
			color |= 0x10;

		union REGS regs;
		regs.w.ax = 0x1000;
		regs.h.bl = 1;
		regs.h.bh = color;
		int86(0x10, &regs, &regs);
	}
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	__djgpp_nearptr_enable();

	videocard = I_DetectVideoCard();

	I_SetScreenMode(6);

	videomemory = D_MK_FP(0xb800, (((SCREENHEIGHT_CGA - SCREENHEIGHT) / 2) / 2) * PLANEWIDTH + (PLANEWIDTH - VIEWWINDOWWIDTH) / 2 + __djgpp_conventional_base);

	_s_screen = Z_MallocStatic(VIEWWINDOWWIDTH * SCREENHEIGHT);
	_fmemset(_s_screen, 0, VIEWWINDOWWIDTH * SCREENHEIGHT);
}


void I_ShutdownGraphics(void)
{
	I_SetScreenMode(3);
}


static boolean drawStatusBar = true;


static void I_DrawBuffer(uint8_t __far* buffer)
{
	uint8_t __far* src = buffer;
	uint8_t __far* dst = videomemory;

	for (int16_t y = 0; y < (SCREENHEIGHT - ST_HEIGHT) / 2; y++)
	{
		_fmemcpy(dst, src, VIEWWINDOWWIDTH);
		dst += 0x2000;
		src += VIEWWINDOWWIDTH;

		_fmemcpy(dst, src, VIEWWINDOWWIDTH);
		dst -= 0x2000 - PLANEWIDTH;
		src += VIEWWINDOWWIDTH;
	}

	if (drawStatusBar)
	{
		for (int16_t y = 0; y < ST_HEIGHT / 2; y++)
		{
			_fmemcpy(dst, src, VIEWWINDOWWIDTH);
			dst += 0x2000;
			src += VIEWWINDOWWIDTH;

			_fmemcpy(dst, src, VIEWWINDOWWIDTH);
			dst -= 0x2000 - PLANEWIDTH;
			src += VIEWWINDOWWIDTH;
		}
	}
	drawStatusBar = true;
}


static int8_t newpal;


void I_SetPalette(int8_t p)
{
	newpal = p;
}


#define NO_PALETTE_CHANGE 100

void I_FinishUpdate(void)
{
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	I_DrawBuffer(_s_screen);
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
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 14: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 13: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 12: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 11: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 10: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  9: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  8: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  7: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  6: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  5: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  4: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  3: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  2: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
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

	dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

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
static uint8_t swapNibbles(uint8_t color)
{
	return (color << 4) | (color >> 4);
}


static void R_DrawColumnFlat2(uint8_t color, int16_t yl, int16_t count)
{
	uint8_t color0;
	uint8_t color1;

	if (yl & 1)
	{
		color0 = swapNibbles(color);
		color1 = color;
	}
	else
	{
		color0 = color;
		color1 = swapNibbles(color);
	}

	for (int16_t i = 0; i < count / 2; i++)
	{
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
	}

	if (count & 1)
		*dest = color0;
}
#else
void R_DrawColumnFlat2(uint8_t color, int16_t yl, int16_t count);
#endif


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	R_DrawColumnFlat2(color, dcvars->yl, count);
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0x02
#define FUZZCOLOR3 0x20
#define FUZZCOLOR4 0x22
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

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t __far* dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	static int16_t fuzzpos = 0;

	do
	{
		*dest = fuzzcolors[fuzzpos];
		dest += VIEWWINDOWWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;
	} while(--count);
}


void V_ClearViewWindow(void)
{
	_fmemset(_s_screen, 0, VIEWWINDOWWIDTH * (SCREENHEIGHT - ST_HEIGHT));
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
	UNUSED(color);

	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	uint8_t bitmask = 0b10000000 >> (x0 & 7);

	while (true)
	{
		uint8_t c = _s_screen[y0 * VIEWWINDOWWIDTH + (x0 >> 3)];
		_s_screen[y0 * VIEWWINDOWWIDTH + (x0 >> 3)] = (c & ~bitmask) | bitmask;

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;

			bitmask = 0b10000000 >> (x0 & 7);
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

	for (int16_t y = 0; y < SCREENHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x += 16)
		{
			uint8_t __far* d = &_s_screen[y * VIEWWINDOWWIDTH + x];
			const byte __far* s = &src[((y & 63) * 16)];

			size_t len = 16;

			if (VIEWWINDOWWIDTH - x < 16)
				len = VIEWWINDOWWIDTH - x;

			_fmemcpy(d, s, len);
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t __far* lump = W_TryGetLumpByNum(num);

	offset = (offset / SCREENWIDTH) * VIEWWINDOWWIDTH;

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		_fmemcpy(&_s_screen[offset], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_s_screen[offset]);
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
		ST_doRefresh();
	else
		drawStatusBar = false;
}


static const uint8_t bitmasks4[4] = {0x3f, 0xcf, 0xf3, 0xfc};


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte __far* desttop = _s_screen + (y * VIEWWINDOWWIDTH) + (x >> 2);

	int16_t width = patch->width;

	int16_t p = x & 3;
	uint8_t bitmask = bitmasks4[p];

	for (int16_t col = 0; col < width; col++)
	{
		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * VIEWWINDOWWIDTH);

			uint16_t count = column->length;

			uint8_t c;
			uint8_t color;

			if (count == 7)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 3)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 5)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 6)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else
			{
				while (count--)
				{
					c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				}
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}

		p++;
		if (p == 4)
		{
			p = 0;
			desttop++;
		}
		bitmask = bitmasks4[p];
	}
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	static const int32_t   DX  = (((int32_t)SCREENWIDTH)<<FRACBITS) / SCREENWIDTH_VGA;
	static const int16_t   DXI = ((((int32_t)SCREENWIDTH_VGA)<<FRACBITS) / SCREENWIDTH) >> 8;
	static const int32_t   DY  = ((((int32_t)SCREENHEIGHT)<<FRACBITS)+(FRACUNIT-1)) / SCREENHEIGHT_VGA;
	static const int16_t   DYI = ((((int32_t)SCREENHEIGHT_VGA)<<FRACBITS) / SCREENHEIGHT) >> 8;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	const int16_t left   = ( x * DX ) >> FRACBITS;
	const int16_t right  = ((x + patch->width)  * DX) >> FRACBITS;
	const int16_t bottom = ((y + patch->height) * DY) >> FRACBITS;

	uint16_t   col = 0;

	for (int16_t dc_x = left; dc_x < right; dc_x++, col += DXI)
	{
		if (dc_x < 0)
			continue;
		else if (dc_x >= SCREENWIDTH)
			break;

		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col >> 8]);

		int16_t p = dc_x & 3;
		uint8_t bitmask = bitmasks4[p];

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte __far* dest = _s_screen + (dc_yl * VIEWWINDOWWIDTH) + (dc_x >> 2);

			int16_t frac = 0;

			const byte __far* source = (const byte __far*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				uint8_t c = *dest;
				uint8_t color = source[frac >> 8];
				*dest = (c & bitmask) | (color >> (p * 2));
				dest += VIEWWINDOWWIDTH;
				frac += DYI;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
}


static uint8_t __far* frontbuffer;
static int16_t __far* wipe_y_lookup;


void wipe_StartScreen(void)
{
	frontbuffer = Z_TryMallocStatic(VIEWWINDOWWIDTH * SCREENHEIGHT);
	if (frontbuffer)
		_fmemcpy(frontbuffer, _s_screen, VIEWWINDOWWIDTH * SCREENHEIGHT);
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
			if (wipe_y_lookup[i] < SCREENHEIGHT)
			{
				int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : SCREENHEIGHT / 25;
				// At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
					dy = SCREENHEIGHT - wipe_y_lookup[i];

				uint8_t __far* s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * VIEWWINDOWWIDTH);

				uint8_t __far* d = &frontbuffer[i] + ((SCREENHEIGHT - 1) * VIEWWINDOWWIDTH);

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
