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


extern const int16_t CENTERY;


static uint8_t  __far* _s_screen;


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


static void I_UploadNewPalette(int8_t pal)
{
	// TODO implement me
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(0x0d);
	I_ReloadPalette();
	I_UploadNewPalette(0);

	outp(SC_INDEX, SC_MAPMASK);
	outp(SC_INDEX + 1, 15);

	__djgpp_nearptr_enable();
	_fmemset(D_MK_FP(0xa000, 0 + __djgpp_conventional_base), 0, 0xffff);

	_s_screen = D_MK_FP(0xa600, 0 + __djgpp_conventional_base);
	int16_t i = 0;
	for (int16_t y = 0; y < 16; y++)
	{
		for (int16_t x = 0; x < 16; x++)
		{
			int16_t plane = 1;
			for (int16_t j = 0; j < 4; j++)
			{
				int16_t c;
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
				_s_screen[i] = c;
				plane <<= 1;
			}
			i++;
		}
	}

	_s_screen = D_MK_FP(0xa200, 0 + __djgpp_conventional_base);
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

void I_FinishUpdate(void)
{
	// palette
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
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

uint8_t nearcolormap[256];

#define L_FP_OFF D_FP_OFF
static uint16_t nearcolormapoffset = 0xffff;

const uint8_t __far* source;
uint8_t __far* dest;


static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	int16_t l = count >> 4;
	while (l--)
	{
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = nearcolormap[source[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
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

	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	while (count--)
	{
		*dest = color;
		dest += PLANEWIDTH;
	}
}


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	// TODO implement me
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
	// TODO implement me
}


void ST_Drawer(void)
{
	// TODO implement me
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
