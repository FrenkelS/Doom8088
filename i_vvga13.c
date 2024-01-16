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
 *      Video code for VGA Mode 13h 320x200 256 colors
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


extern const int16_t CENTERY;

// The screen is [SCREENWIDTH * SCREENHEIGHT];
static uint8_t __far* _s_screen;
static uint8_t __far* vgascreen;


#define PEL_WRITE_ADR   0x3c8
#define PEL_DATA        0x3c9

static void I_UploadNewPalette(int8_t pal)
{
	// This is used to replace the current 256 colour cmap with a new one
	// Used by 256 colour PseudoColor modes

	char lumpName[9] = "PLAYPAL0";

	if(_g_gamma == 0)
		lumpName[7] = 0;
	else
		lumpName[7] = '0' + _g_gamma;

	const uint8_t __far* palette_lump = W_TryGetLumpByNum(W_GetNumForName(lumpName));
	if (palette_lump != NULL)
	{
		const byte __far* palette = &palette_lump[pal * 256 * 3];
		outp(PEL_WRITE_ADR, 0);
		for (int_fast16_t i = 0; i < 256 * 3; i++)
			outp(PEL_DATA, (*palette++) >> 2);

		Z_ChangeTagToCache(palette_lump);
	}
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(0x13);
	I_UploadNewPalette(0);

	__djgpp_nearptr_enable();
	vgascreen = D_MK_FP(0xa000, ((SCREENWIDTH_VGA - SCREENWIDTH) / 2) + (((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * SCREENWIDTH_VGA) + __djgpp_conventional_base);

	_s_screen = Z_MallocStatic(SCREENWIDTH * SCREENHEIGHT);
	_fmemset(_s_screen, 0, SCREENWIDTH * SCREENHEIGHT);
}


void I_ShutdownGraphicsHardwareSpecificCode(void)
{
	// Do nothing
}


static void I_DrawBuffer(uint8_t __far* buffer)
{
	uint8_t __far* src = buffer;
	uint8_t __far* dst = vgascreen;

#if defined DISABLE_STATUS_BAR
	for (uint_fast8_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++) {
#else
	for (uint_fast8_t y = 0; y < SCREENHEIGHT; y++) {
#endif
		_fmemcpy(dst, src, SCREENWIDTH);
		dst += SCREENWIDTH_VGA;
		src += SCREENWIDTH;
	}
}


static int8_t newpal;


//
// I_SetPalette
//
void I_SetPalette(int8_t pal)
{
	newpal = pal;
}


//
// I_FinishUpdate
//

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


//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

uint8_t nearcolormap[256];
static uint16_t nearcolormapoffset = 0xffff;

const uint8_t __far* source;
uint8_t __far* dest;


inline static void R_DrawColumnPixel(uint8_t __far* dest, const byte __far* source, uint16_t frac)
{
	uint16_t color = nearcolormap[source[frac>>COLBITS]];
	color = (color | (color << 8));

	uint16_t __far* d = (uint16_t __far*) dest;
	*d++ = color;
	*d   = color;
}


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	while (count--)
	{
		R_DrawColumnPixel(dest, source, frac);
		dest += SCREENWIDTH;
		frac += fracstep;
	}
}
#else
void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count);
#endif


void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	if (nearcolormapoffset != D_FP_OFF(dcvars->colormap))
	{
		_fmemcpy(nearcolormap, dcvars->colormap, 256);
		nearcolormapoffset = D_FP_OFF(dcvars->colormap);
	}

	dest = _s_screen + (dcvars->yl * SCREENWIDTH) + (dcvars->x << 2);

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint16_t color = (texture << 8) | texture;

	uint8_t __far* dest = _s_screen + (dcvars->yl * SCREENWIDTH) + (dcvars->x << 2);
	uint16_t __far* d = (uint16_t __far*) dest;

	uint16_t l = count >> 4;

	while (l--)
	{
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;

		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;

		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;

		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
	}

	switch (count & 15)
	{
		case 15: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case 14: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case 13: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case 12: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case 11: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case 10: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  9: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  8: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  7: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  6: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  5: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  4: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  3: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  2: *d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		case  1: *d++ = color; *d = color;
	}
}


#define FUZZOFF (VIEWWINDOWWIDTH)
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

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//
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

	if (nearcolormapoffset != D_FP_OFF(&fullcolormap[6 * 256]))
	{
		_fmemcpy(nearcolormap, &fullcolormap[6 * 256], 256);
		nearcolormapoffset = D_FP_OFF(&fullcolormap[6 * 256]);
	}

	uint8_t __far* dest = _s_screen + (dc_yl * SCREENWIDTH) + (dcvars->x << 2);

	static int16_t fuzzpos = 0;

	do
	{
		R_DrawColumnPixel(dest, &dest[fuzzoffset[fuzzpos] * 4], 0);
		dest += SCREENWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


#if !defined FLAT_SPAN
inline static void R_DrawSpanPixel(uint32_t __far* dest, const byte __far* source, const byte __far* colormap, uint32_t position)
{
	uint16_t color = colormap[source[((position >> 4) & 0x0fc0) | (position >> 26)]];
	color = color | (color << 8);

	uint16_t __far* d = (uint16_t __far*) dest;
	*d++ = color;
	*d   = color;
}


void R_DrawSpan(uint16_t y, uint16_t x1, uint16_t x2, const draw_span_vars_t *dsvars)
{
	uint16_t count = (x2 - x1);

	const byte __far* source   = dsvars->source;
	const byte __far* colormap = dsvars->colormap;

	uint32_t __far* dest = (uint32_t __far*)(_s_screen + (y * SCREENWIDTH) + (x1 << 2));

	const uint32_t step = dsvars->step;
	uint32_t position = dsvars->position;

	uint16_t l = (count >> 4);

	while (l--)
	{
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
    }

	switch (count & 15)
	{
		case 15:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 14:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 13:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 12:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 11:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 10:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  9:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  8:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  7:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  6:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  5:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  4:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  3:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  2:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  1:    R_DrawSpanPixel(dest, source, colormap, position);
	}
}
#endif


//
// V_FillRect
//
void V_FillRect(byte colour)
{
	_fmemset(_s_screen, colour, SCREENWIDTH * (SCREENHEIGHT - ST_HEIGHT));
}


//
// V_DrawLine()
//
// Draw a line in the frame buffer.
// Classic Bresenham w/ whatever optimizations needed for speed
//
// Passed the frame coordinates of line, and the color to be drawn
// Returns nothing
//
void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	while (true)
	{
		_s_screen[y0 * SCREENWIDTH + x0] = color;

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


/*
 * V_DrawBackground tiles a 64x64 patch over the entire screen, providing the
 * background for the Help and Setup screens, and plot text between levels.
 * cphipps - used to have M_DrawBackground, but that was used the framebuffer
 * directly, so this is my code from the equivalent function in f_finale.c
 */
void V_DrawBackground(void)
{
	/* erase the entire screen to a tiled background */
	const byte __far* src = W_GetLumpByName("FLOOR4_8");

	for(uint8_t y = 0; y < SCREENHEIGHT; y++)
	{
		for(uint16_t x = 0; x < SCREENWIDTH; x+=64)
		{
			uint8_t __far* d = &_s_screen[y * SCREENWIDTH + x];
			const byte __far* s = &src[((y&63) * 64) + (x&63)];

			uint8_t len = 64;

			if (SCREENWIDTH - x < 64)
				len = SCREENWIDTH - x;

			_fmemcpy(d, s, len);
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t __far* lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		_fmemcpy(&_s_screen[offset], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_s_screen[offset]);
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte __far* desttop = _s_screen + (y * SCREENWIDTH) + x;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * SCREENWIDTH);

			uint16_t count = column->length;

			uint16_t l = count >> 2;
			while (l--)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
			}

			switch (count & 3)
			{
				case 3: *dest = *source++; dest += SCREENWIDTH;
				case 2: *dest = *source++; dest += SCREENWIDTH;
				case 1: *dest = *source++;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
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

		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + patch->columnofs[col >> 8]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte __far* dest = _s_screen + (dc_yl * SCREENWIDTH) + dc_x;

			int16_t frac = 0;

			const byte __far* source = (const byte __far*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				*dest = source[frac >> 8];
				dest += SCREENWIDTH;
				frac += DYI;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
}


static uint16_t __far* frontbuffer;
static  int16_t __far* wipe_y_lookup;


void wipe_StartScreen(void)
{
	frontbuffer = Z_TryMallocStatic(SCREENWIDTH * SCREENHEIGHT);
	if (frontbuffer)
	{
		// copy back buffer to front buffer
		_fmemcpy(frontbuffer, _s_screen, SCREENWIDTH * SCREENHEIGHT);
	}
}


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint16_t __far* backbuffer = (uint16_t __far*)_s_screen;

	while (ticks--)
	{
		I_DrawBuffer((uint8_t __far*)frontbuffer);
		for (int16_t i = 0; i < SCREENWIDTH / 2; i++)
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
				/* cph 2001/07/29 -
				 *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
				 *  the whole screen, so make the melt rate depend on SCREENHEIGHT
				 *  so it takes no longer in high res
				 */
				int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : SCREENHEIGHT / 25;
				// At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
					dy = SCREENHEIGHT - wipe_y_lookup[i];

				uint16_t __far* s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * (SCREENWIDTH / 2));

				uint16_t __far* d = &frontbuffer[i] + ((SCREENHEIGHT - 1) * (SCREENWIDTH / 2));

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -(SCREENWIDTH / 2);
					s += -(SCREENWIDTH / 2);
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * SCREENWIDTH / 2;
				d = &frontbuffer[i] + wipe_y_lookup[i] * SCREENWIDTH / 2;

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += (SCREENWIDTH / 2);
					s += (SCREENWIDTH / 2);
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
	wipe_y_lookup = Z_MallocStatic(SCREENWIDTH);

	// setup initial column positions (y<0 => not ready to scroll yet)
	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int8_t i = 1; i < SCREENWIDTH / 2; i++)
	{
		int8_t r = (M_Random() % 3) - 1;

		wipe_y_lookup[i] = wipe_y_lookup[i - 1] + r;

		if (wipe_y_lookup[i] > 0)
			wipe_y_lookup[i] = 0;
		else if (wipe_y_lookup[i] == -16)
			wipe_y_lookup[i] = -15;
	}
}


//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating

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
