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
 *      Video code for MDA 80x25 monochrome text mode
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


extern const int16_t CENTERY;

static uint8_t __far* _s_screen;
static uint8_t __far* videomemory;


static boolean isMDA;


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
	I_SetScreenMode(isMDA ? 7 : 3);

	__djgpp_nearptr_enable();
	videomemory = D_MK_FP(isMDA ? 0xb000 : 0xb800, 0 + __djgpp_conventional_base);

	_s_screen = Z_MallocStatic(VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
	_fmemset(_s_screen, 0, VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
}


static void I_DrawBuffer(uint8_t __far* buffer)
{
	uint8_t __far* src = buffer;
	uint8_t __far* dst = videomemory;
	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			*dst++ = *src++;
			dst++;
		}
	}
}


void I_ShutdownGraphics(void)
{
	I_SetScreenMode(isMDA ? 7 : 3);
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


#if defined C_ONLY
static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	switch (count)
	{
		case 25: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 24: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 23: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 22: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 21: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 20: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 19: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 18: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 17: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 16: *dest = nearcolormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
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

	switch (count)
	{
		case 25: *dest = color; dest += VIEWWINDOWWIDTH;
		case 24: *dest = color; dest += VIEWWINDOWWIDTH;
		case 23: *dest = color; dest += VIEWWINDOWWIDTH;
		case 22: *dest = color; dest += VIEWWINDOWWIDTH;
		case 21: *dest = color; dest += VIEWWINDOWWIDTH;
		case 20: *dest = color; dest += VIEWWINDOWWIDTH;
		case 19: *dest = color; dest += VIEWWINDOWWIDTH;
		case 18: *dest = color; dest += VIEWWINDOWWIDTH;
		case 17: *dest = color; dest += VIEWWINDOWWIDTH;
		case 16: *dest = color; dest += VIEWWINDOWWIDTH;
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
#define FUZZCOLOR2 0xb0
#define FUZZCOLOR3 0x00
#define FUZZCOLOR4 0xb0
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
	_fmemset(_s_screen, colour, VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	while (true)
	{
		_s_screen[y0 * VIEWWINDOWWIDTH + x0] = color;

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
		uint8_t __far* dest = _s_screen + y * VIEWWINDOWWIDTH;
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x++)
		{
			*dest++ = src[((y * 2) & 63) * 64 + ((x * 2) & 63)];
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
		_fmemcpy(_s_screen, lump, VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, _s_screen);
}


void V_DrawCharacter(int16_t x, int16_t y, uint8_t color, char c)
{
	UNUSED(color);
	_s_screen[y * VIEWWINDOWWIDTH + x] = c;
}


void V_DrawCharacterForeground(int16_t x, int16_t y, uint8_t color, char c)
{
	V_DrawCharacter(x, y, color, c);
}


void V_DrawString(int16_t x, int16_t y, uint8_t color, const char* s)
{
	UNUSED(color);

	uint8_t __far* dst = _s_screen + y * VIEWWINDOWWIDTH + x;

	while (*s)
	{
		*dst++ = *s++;
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


static boolean I_IsMDA(void)
{
	union REGS regs;
	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	return regs.h.al == 7;
}


void D_DoomMain(int argc, const char * const * argv);

int main(int argc, const char * const * argv)
{
	isMDA = I_IsMDA();

	I_SetScreenMode(isMDA ? 7 : 3);

	printf(
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"DOOM8088 System Startup"
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"\n");

	D_DoomMain(argc, argv);
	return 0;
}


void I_Endoom(void)
{
	int16_t lumpnum = W_GetNumForName("ENDOOM");
	W_ReadLumpByNum(lumpnum, D_MK_FP(isMDA ? 0xb000 : 0xb800, 0 + __djgpp_conventional_base));
}
