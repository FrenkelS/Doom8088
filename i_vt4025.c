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
 *      Video code for Text mode 40x25 16 color
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


extern const int16_t CENTERY;

static uint8_t __far* _s_screen;


void I_ReloadPalette(void)
{

}


void I_InitGraphicsHardwareSpecificCode(void)
{
	__djgpp_nearptr_enable();

	I_SetScreenMode(1);

	// disable blinking
	union REGS regs;
	regs.w.ax = 0x1003;
	regs.w.bx = 0x0000;
	int86(0x10, &regs, &regs);

	_s_screen = D_MK_FP(0xb880, 1 + __djgpp_conventional_base);

	uint16_t __far* dst;
	dst = D_MK_FP(0xb800, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x00b1;

	dst = D_MK_FP(0xb880, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x00b1;

	dst = D_MK_FP(0xb900, 0 + __djgpp_conventional_base);
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
		*dst++ = 0x00b1;


	outp(0x3d4, 0xc);
}


void I_SetPalette(int8_t pal)
{
	UNUSED(pal);
}


void I_FinishUpdate(void)
{
	// page flip between segments B800, B880, B900
	outp(0x3d5, (D_FP_SEG(_s_screen) >> 5) & 0x0f);
	_s_screen = (uint8_t __far*)(((uint32_t)_s_screen) + 0x00800000);
	if (D_FP_SEG(_s_screen) == 0xb980)
		_s_screen = D_MK_FP(0xb800, 1 + __djgpp_conventional_base);
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
	UNUSED(colour);
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	UNUSED(x0);
	UNUSED(y0);
	UNUSED(x1);
	UNUSED(y1);
	UNUSED(color);
}


void V_DrawBackground(void)
{

}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	UNUSED(num);
	UNUSED(offset);
}


void ST_Drawer(void)
{

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


void wipe_StartScreen(void)
{

}


void D_Wipe(void)
{

}
