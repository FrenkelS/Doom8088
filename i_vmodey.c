/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023 Frenkel Smeijers
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
 *      Video code for VGA Mode Y
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
 
 
void I_SetScreenMode(uint16_t mode);

extern const int16_t CENTERY;

static uint8_t  __far* _s_screen;


void V_DrawBackground(void)
{
	//TODO implement me
}


static int16_t cachedLumpNum;
static int16_t cachedLumpHeight;

void V_DrawRaw(int16_t num, uint16_t offset)
{
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
					uint8_t __far* dest = D_MK_FP(0xa800, y * PLANEWIDTH + __djgpp_conventional_base);
					for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
					{
						*dest++ = lump[y * SCREENWIDTH + (x * 4) + plane];
					}
				}
			}
			outp(SC_INDEX + 1, 15);
			Z_ChangeTagToCache(lump);

			cachedLumpNum = num;
		}
	}

	if (cachedLumpNum == num)
	{
		// set write mode 1
		outp(GC_INDEX, GC_MODE);
		outp(GC_INDEX + 1, inp(GC_INDEX + 1) | 1);

		uint8_t __far *src  = D_MK_FP(0xa800, 0 + __djgpp_conventional_base);
		uint8_t __far *dest = _s_screen + (offset / SCREENWIDTH) * PLANEWIDTH;
		for (int16_t y = 0; y < cachedLumpHeight; y++)
		{
			for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
			{
				volatile uint8_t loadLatches = src[y * PLANEWIDTH + x];
				dest[y * PLANEWIDTH + x] = 0;
			}
		}

		// set write mode 0
		outp(GC_INDEX, GC_MODE);
		outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~1);
	}
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	//TODO implement me
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	//TODO implement me
}


void V_FillRect(byte colour)
{
	for (int16_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
		_fmemset(_s_screen + y * PLANEWIDTH, colour, SCREENWIDTH / 4);
}



void V_PlotPixel(int16_t x, int16_t y, uint8_t color)
{
	outp(SC_INDEX + 1, 1 << (x & 3));

	_s_screen[y * PLANEWIDTH + x / 4] = color;

	outp(SC_INDEX + 1, 15);
}


static boolean isGraphicsModeSet = false;

static int8_t newpal;


boolean I_IsGraphicsModeSet(void)
{
	return isGraphicsModeSet;
}


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


#define NO_PALETTE_CHANGE 100

void I_FinishUpdate(void)
{
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	// page flip
	outp(CRTC_INDEX, CRTC_STARTHIGH);
#if defined _M_I86
	outp(CRTC_INDEX + 1, D_FP_SEG(_s_screen) >> 4);
	_s_screen = (uint8_t __far*) (((uint32_t) _s_screen + 0x04000000) & 0xa400ffff); // flip between segments A000 and A400
#else
	outp(CRTC_INDEX + 1, (D_FP_SEG(_s_screen) >> 4) & 0xf0);
	_s_screen = (uint8_t __far*) (((uint32_t) _s_screen + 0x4000) & 0xfffa4fff);
#endif
}


void I_SetPalette(int8_t pal)
{
	newpal = pal;
}


void I_InitGraphics(void)
{	
	I_SetScreenMode(0x13);
	I_UploadNewPalette(0);
	isGraphicsModeSet = true;

	__djgpp_nearptr_enable();
	_s_screen = D_MK_FP(0xa000, ((SCREENWIDTH_VGA - SCREENWIDTH) / 2) / 4 + (((SCREENHEIGHT_VGA - SCREENHEIGHT) / 2) * SCREENWIDTH_VGA) / 4 + __djgpp_conventional_base);

	outp(SC_INDEX, SC_MEMMODE);
	outp(SC_INDEX + 1, (inp(SC_INDEX + 1) & ~8) | 4);

	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~0x13);

	outp(GC_INDEX, GC_MISCELLANEOUS);
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~2);

	outp(SC_INDEX, SC_MAPMASK);
	outp(SC_INDEX + 1, 15);

	_fmemset(D_MK_FP(0xa000, 0 + __djgpp_conventional_base), 0, 0xffff);

	outp(CRTC_INDEX, CRTC_UNDERLINE);
	outp(CRTC_INDEX + 1,inp(CRTC_INDEX + 1) & ~0x40);

	outp(CRTC_INDEX, CRTC_MODE);
	outp(CRTC_INDEX + 1, inp(CRTC_INDEX + 1) | 0x40);
}


void I_StartDisplay(void)
{
	// Do nothing
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

inline static void R_DrawColumnPixel(uint8_t __far* dest, const byte __far* source, const byte __far* colormap, uint16_t frac)
{
	*dest = colormap[source[frac>>COLBITS]];
}


void R_DrawColumn (const draw_column_vars_t *dcvars)
{
    int16_t count = (dcvars->yh - dcvars->yl) + 1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0)
        return;

    const byte __far* source   = dcvars->source;
    const byte __far* colormap = dcvars->colormap;

    uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

    const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
    uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.

    uint16_t l = count >> 4;

    while (l--)
    {
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;

        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;

        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;

        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
    }

    switch (count & 15)
    {
        case 15:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case 14:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case 13:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case 12:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case 11:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case 10:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  9:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  8:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  7:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  6:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  5:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  4:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  3:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  2:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=PLANEWIDTH; frac+=fracstep;
        case  1:    R_DrawColumnPixel(dest, source, colormap, frac);
    }
}


void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t color = texture;

	uint8_t __far* dest = _s_screen + (dcvars->yl * PLANEWIDTH) + dcvars->x;

	while (count--)
	{
		*dest = color;
		dest += PLANEWIDTH;
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
void R_DrawFuzzColumn (const draw_column_vars_t *dcvars)
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

    const byte __far* colormap = &fullcolormap[6 * 256];

    uint8_t __far* dest = _s_screen + (dc_yl * PLANEWIDTH) + dcvars->x;

    static int16_t fuzzpos = 0;

    do
    {        
		//TODO implement me
        *dest = 0;
        dest += PLANEWIDTH;

        fuzzpos++;
        if (fuzzpos >= FUZZTABLE)
            fuzzpos = 0;

    } while(--count);
}


void wipe_StartScreen(void)
{
	//TODO implement me
}


void D_Wipe(void)
{
	//TODO implement me
}
