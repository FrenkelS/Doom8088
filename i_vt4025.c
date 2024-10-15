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


extern const int16_t CENTERY;

static uint8_t __far* _s_screen;
static uint8_t __far* videomemory;


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

	videomemory = D_MK_FP(0xb800, 0 + __djgpp_conventional_base);
	uint8_t __far* dst = videomemory;
	for (int16_t i = 0; i < VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT; i++)
	{
		*dst++ = 177;
		*dst++ = 0;
	}

	_s_screen = Z_MallocStatic(VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
	_fmemset(_s_screen, 0, VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
}


static void I_DrawBuffer(uint8_t __far* buffer)
{
	uint8_t __far* src = buffer;
	uint8_t __far* dst = videomemory + 1;

	for (uint_fast8_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		for (uint_fast8_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			*dst++ = *src++;			
			dst++;
		}
	}
}


void I_SetPalette(int8_t pal)
{
	UNUSED(pal);
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


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	int16_t l = count >> 4;
	while (l--)
	{
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 14: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 13: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 12: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 11: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 10: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  9: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  8: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  7: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  6: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  5: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  4: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  3: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  2: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
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

	dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	if (count <= 0)
		return;

	uint8_t __far* dest = _s_screen + (dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x;

	uint16_t l = count >> 4;

	while (l--)
	{
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;

		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;

		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;

		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
		*dest = color; dest += VIEWWINDOWWIDTH;
	}

	switch (count & 15)
	{
		case 15: *dest = color; dest += VIEWWINDOWWIDTH;
		case 14: *dest = color; dest += VIEWWINDOWWIDTH;
		case 13: *dest = color; dest += VIEWWINDOWWIDTH;
		case 12: *dest = color; dest += VIEWWINDOWWIDTH;
		case 11: *dest = color; dest += VIEWWINDOWWIDTH;
		case 10: *dest = color; dest += VIEWWINDOWWIDTH;
		case  9: *dest = color; dest += VIEWWINDOWWIDTH;
		case  8: *dest = color; dest += VIEWWINDOWWIDTH;
		case  7: *dest = color; dest += VIEWWINDOWWIDTH;
		case  6: *dest = color; dest += VIEWWINDOWWIDTH;
		case  5: *dest = color; dest += VIEWWINDOWWIDTH;
		case  4: *dest = color; dest += VIEWWINDOWWIDTH;
		case  3: *dest = color; dest += VIEWWINDOWWIDTH;
		case  2: *dest = color; dest += VIEWWINDOWWIDTH;
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
