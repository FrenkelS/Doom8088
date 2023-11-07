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
 *      Render floor and ceilings
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "r_defs.h"
#include "r_main.h"

#include "globdata.h"


#define FLAT_SPAN


static int16_t firstflat;


//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

#if defined FLAT_SPAN
static void R_DrawSpan(uint16_t y, uint16_t x1, uint16_t x2, uint16_t color)
{
	uint16_t __far* dest = _g_screen + ScreenYToOffset(y) + x1;

	uint16_t count = x2 - x1;

	_fmemset(dest, color, count * sizeof(uint16_t));
}


//
// R_MakeSpans
//

static void R_MakeSpans(int16_t x, uint16_t t1, uint16_t b1, uint16_t t2, uint16_t b2, uint16_t color)
{
	static byte spanstart[VIEWWINDOWHEIGHT];

	for (; t1 < t2 && t1 <= b1; t1++)
		R_DrawSpan(t1, spanstart[t1], x, color);

	for (; b1 > b2 && b1 >= t1; b1--)
		R_DrawSpan(b1, spanstart[b1], x, color);

	while (t2 < t1 && t2 <= b2)
		spanstart[t2++] = x;

	while (b2 > b1 && b2 >= t2)
		spanstart[b2--] = x;
}


#else


inline static void R_DrawSpanPixel(uint16_t __far* dest, const byte __far* source, const byte* colormap, uint32_t position)
{
    uint16_t color = colormap[source[((position >> 4) & 0x0fc0) | (position >> 26)]];

    *dest = color | (color << 8);
}


typedef struct {
  uint32_t            position;
  uint32_t            step;
  const byte          __far* source; // start of a 64*64 tile image
  const lighttable_t  *colormap;
} draw_span_vars_t;


static void R_DrawSpan(uint16_t y, uint16_t x1, uint16_t x2, const draw_span_vars_t *dsvars)
{
    uint16_t count = (x2 - x1);

    const byte __far* source = dsvars->source;
    const byte *colormap = dsvars->colormap;

    uint16_t __far* dest = _g_screen + ScreenYToOffset(y) + x1;

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


static const fixed_t yslopeTable[VIEWWINDOWHEIGHT / 2] =
{
    132104,134218,136400,138655,140985,143395,145889,148471,151146,153919,156796,159783,162886,166111,
    169467,172961,176602,180400,184365,188508,192842,197379,202135,207126,212370,217886,223696,229825,
    236299,243148,250406,258111,266305,275036,284360,294337,305040,316551,328965,342392,356962,372827,
    390168,409200,430185,453438,479349,508400,541201,578525,621378,671089,729444,798915,883011,986895,
    1118481,1290555,1525201,1864135,2396745,3355443,5592405,16777216
};

static const uint16_t distscaleTable[SCREENWIDTH] =
{
    0x6A75,
    0x676E,0x6438,0x6158,0x5E8C,0x5B90,0x58AB,0x55D9,0x5319,0x5034,0x4D63,0x4AA8,0x4800,0x456A,0x42B6,0x4018,0x3D8E,
    0x3B16,0x38B2,0x3637,0x33CF,0x317C,0x2F3C,0x2CEA,0x2AAB,0x2881,0x266B,0x2446,0x2237,0x203B,0x1E54,0x1C63,0x1AA1,
    0x18D7,0x1721,0x1567,0x13C1,0x1244,0x10C4,0x0F45,0x0DEB,0x0C92,0x0B4E,0x0A1C,0x08FD,0x07F0,0x06E8,0x05F4,0x051D,
    0x044F,0x0392,0x02E0,0x0249,0x01C3,0x014A,0x00E3,0x0094,0x0053,0x0025,0x000A,0x0001,0x0009,0x0023,0x0051,0x0091,
    0x00DF,0x0145,0x01BD,0x0242,0x02D9,0x038A,0x0445,0x0514,0x05E9,0x06DC,0x07E2,0x08EF,0x0A0D,0x0B3E,0x0C82,0x0DDA,
    0x0F31,0x10B0,0x122F,0x13AC,0x1550,0x1709,0x18BF,0x1A87,0x1C48,0x1E37,0x201E,0x2218,0x2427,0x264A,0x2860,0x2A88,
    0x2CC4,0x2F16,0x3155,0x33A7,0x360D,0x3887,0x3AEA,0x3D60,0x3FE9,0x4286,0x4538,0x47CB,0x4A72,0x4D2D,0x4FFB,0x52DF,
    0x559C,0x586E,0x5B51,0x5E4A,0x6115,0x63F4,0x6729 
};

static fixed_t yslope(uint8_t y)
{
	if (y >= 64)
		y = 127 - y;
	return yslopeTable[y];
}

static fixed_t distscale(uint8_t x)
{
	return 0x010000 | distscaleTable[x];
}


static int16_t __far* flattranslation;             // for global animation

static fixed_t planeheight;
static fixed_t basexscale, baseyscale;

static void R_MapPlane(uint16_t y, uint16_t x1, uint16_t x2, draw_span_vars_t *dsvars)
{    
    const fixed_t distance = FixedMul(planeheight, yslope(y));
    dsvars->step = ((FixedMul(distance,basexscale) << 10) & 0xffff0000) | ((FixedMul(distance,baseyscale) >> 6) & 0x0000ffff);

    fixed_t length = FixedMul (distance, distscale(x1));
    angle_t angle = (viewangle + xtoviewangle(x1))>>ANGLETOFINESHIFT;

    // killough 2/28/98: Add offsets
    uint32_t xfrac =  viewx + FixedMul(finecosine(angle), length);
    uint32_t yfrac = -viewy - FixedMul(finesine(  angle), length);

    dsvars->position = ((xfrac << 10) & 0xffff0000) | ((yfrac >> 6)  & 0x0000ffff);

    R_DrawSpan(y, x1, x2, dsvars);
}


//
// R_MakeSpans
//

static void R_MakeSpans(int16_t x, uint16_t t1, uint16_t b1, uint16_t t2, uint16_t b2, draw_span_vars_t *dsvars)
{
    static byte spanstart[VIEWWINDOWHEIGHT];

    for (; t1 < t2 && t1 <= b1; t1++)
        R_MapPlane(t1, spanstart[t1], x, dsvars);

    for (; b1 > b2 && b1 >= t1; b1--)
        R_MapPlane(b1, spanstart[b1], x, dsvars);

    while (t2 < t1 && t2 <= b2)
        spanstart[t2++] = x;

    while (b2 > b1 && b2 >= t2)
        spanstart[b2--] = x;
}
#endif


static void R_DoDrawPlane(visplane_t __far* pl)
{
    if (pl->minx <= pl->maxx)
    {
        if (pl->picnum == skyflatnum)
        {
            // sky flat
            R_DrawSky(pl);
        }
        else
        {
            // regular flat
            const int16_t stop = pl->maxx + 1;

            pl->top[pl->minx - 1] = pl->top[stop] = 0xff; // dropoff overflow

#if defined FLAT_SPAN
            for (register int16_t x = pl->minx; x <= stop; x++)
            {
                R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x], pl->picnum);
            }
#else
            draw_span_vars_t dsvars;

            dsvars.source   = W_GetLumpByNum(firstflat + flattranslation[pl->picnum]);
            dsvars.colormap = R_LoadColorMap(pl->lightlevel);

            planeheight = D_abs(pl->height - viewz);

            for (register int16_t x = pl->minx; x <= stop; x++)
            {
                R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x], &dsvars);
            }

            Z_ChangeTagToCache(dsvars.source);
#endif
        }
    }
}


//
// RDrawPlanes
// At the end of each frame.
//

void R_DrawPlanes (void)
{
    for (int8_t i = 0; i < MAXVISPLANES; i++)
    {
        visplane_t __far* pl = _g_visplanes[i];

        while(pl)
        {
            if(pl->modified)
                R_DoDrawPlane(pl);

            pl = pl->next;
        }
    }
}


//
// R_ClearPlanes
// At begining of frame.
//

void R_ClearPlanes(void)
{
    // opening / clipping determination
    for (int8_t i = 0; i < SCREENWIDTH; i++)
        floorclip[i] = VIEWWINDOWHEIGHT, ceilingclip[i] = -1;


    for (int8_t i = 0; i < MAXVISPLANES; i++)    // new code -- killough
        for (*freehead = _g_visplanes[i], _g_visplanes[i] = NULL; *freehead; )
            freehead = &(*freehead)->next;

    R_ClearOpenings();

#if !defined FLAT_SPAN
    static const fixed_t iprojection = 1092; //( (1 << FRACUNIT) / (SCREENWIDTH / 2))

    basexscale = FixedMul(viewsin,iprojection);
    baseyscale = FixedMul(viewcos,iprojection);
#endif
}


//
// R_InitFlats
//

void R_InitFlats(void)
{
	firstflat        = W_GetNumForName("F_START") + 1;

#if !defined FLAT_SPAN
	int16_t lastflat = W_GetNumForName("F_END")   - 1;
	int16_t numflats = lastflat - firstflat + 1;

	// Create translation table for global animation.

	flattranslation = Z_MallocStatic((numflats + 1) * sizeof(*flattranslation));

	for (int16_t i = 0; i < numflats; i++)
		flattranslation[i] = i;
#endif
}


//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
//

int16_t R_FlatNumForName(const char *name)
{
	int16_t i = W_GetNumForName(name);
	return i - firstflat;
}


#if defined FLAT_SPAN
void P_InitAnimatedFlat(void) {}
void P_UpdateAnimatedFlat(void) {}


#else
static int16_t  animated_flat_basepic;

void P_InitAnimatedFlat(void)
{
	animated_flat_basepic = R_FlatNumForName("NUKAGE1");
	                        R_FlatNumForName("NUKAGE3");
}


void P_UpdateAnimatedFlat(void)
{
	uint32_t t = _g_leveltime >> 3;

	int16_t pic = animated_flat_basepic + (t % 3);

	for (int16_t i = animated_flat_basepic; i < animated_flat_basepic + 3; i++)
		flattranslation[i] = pic;
}
#endif
