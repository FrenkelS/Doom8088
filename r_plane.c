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

#include "r_defs.h"
#include "r_main.h"

#include "globdata.h"


#define FLAT_SPAN

int16_t *flattranslation;             // for global animation

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
static void R_DrawSpan(uint32_t y, uint32_t x1, uint32_t x2, uint16_t color)
{
	uint16_t* dest = _g->screen + ScreenYToOffset(y) + x1;

	uint32_t count = x2 - x1;
	uint32_t l = count >> 4;

	while (l--)
	{
		*dest++ = color;
		*dest++ = color;
		*dest++ = color;
		*dest++ = color;

		*dest++ = color;
		*dest++ = color;
		*dest++ = color;
		*dest++ = color;

		*dest++ = color;
		*dest++ = color;
		*dest++ = color;
		*dest++ = color;

		*dest++ = color;
		*dest++ = color;
		*dest++ = color;
		*dest++ = color;
	}

	switch (count & 15)
	{
		case 15:	*dest++ = color;
		case 14:	*dest++ = color;
		case 13:	*dest++ = color;
		case 12:	*dest++ = color;
		case 11:	*dest++ = color;
		case 10:	*dest++ = color;
		case  9:	*dest++ = color;
		case  8:	*dest++ = color;
		case  7:	*dest++ = color;
		case  6:	*dest++ = color;
		case  5:	*dest++ = color;
		case  4:	*dest++ = color;
		case  3:	*dest++ = color;
		case  2:	*dest++ = color;
		case  1:	*dest   = color;
	}
}


//
// R_MakeSpans
//

static void R_MakeSpans(int32_t x, uint32_t t1, uint32_t b1, uint32_t t2, uint32_t b2, uint16_t color)
{
	static byte spanstart[SCREENHEIGHT];

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


inline static void R_DrawSpanPixel(uint16_t* dest, const byte* source, const byte* colormap, uint32_t position)
{
    uint16_t color = colormap[source[((position >> 4) & 0x0fc0) | (position >> 26)]];

    *dest = color | (color << 8);
}


typedef struct {
  uint32_t            position;
  uint32_t            step;
  const byte          *source; // start of a 64*64 tile image
  const lighttable_t  *colormap;
} draw_span_vars_t;


static void R_DrawSpan(uint32_t y, uint32_t x1, uint32_t x2, const draw_span_vars_t *dsvars)
{
    uint32_t count = (x2 - x1);

    const byte *source = dsvars->source;
    const byte *colormap = dsvars->colormap;

    uint16_t* dest = _g->screen + ScreenYToOffset(y) + x1;

    const uint32_t step = dsvars->step;
    uint32_t position = dsvars->position;

    uint32_t l = (count >> 4);

    while(l--)
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

    uint8_t r = (count & 15);

    switch(r)
    {
        case 15:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 14:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 13:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 12:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 11:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 10:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 9:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 8:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 7:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 6:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 5:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 4:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 3:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 2:     R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
        case 1:     R_DrawSpanPixel(dest, source, colormap, position);
    }
}


#if 0
static const fixed_t yslopeTable[SCREENHEIGHT] =
{
    132104,134218,136400,138655,140985,143395,145889,148471,151146,153919,156796,159783,162886,166111,
    169467,172961,176602,180400,184365,188508,192842,197379,202135,207126,212370,217886,223696,229825,
    236299,243148,250406,258111,266305,275036,284360,294337,305040,316551,328965,342392,356962,372827,
    390168,409200,430185,453438,479349,508400,541201,578525,621378,671089,729444,798915,883011,986895,
    1118481,1290555,1525201,1864135,2396745,3355443,5592405,16777216,16777216,5592405,3355443,2396745,
    1864135,1525201,1290555,1118481,986895,883011,798915,729444,671089,621378,578525,541201,508400,479349,
    453438,430185,409200,390168,372827,356962,342392,328965,316551,305040,294337,284360,275036,266305,
    258111,250406,243148,236299,229825,223696,217886,212370,207126,202135,197379,192842,188508,184365,
    180400,176602,172961,169467,166111,162886,159783,156796,153919,151146,148471,145889,143395,140985,
    138655,136400,134218,132104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const fixed_t distscaleTable[SCREENWIDTH] =
{
    92789,
    92014,91192,90456,89740,88976,88235,87513,86809,86068,85347,84648,83968,83306,82614,81944,81294,
    80662,80050,79415,78799,78204,77628,77034,76459,75905,75371,74822,74295,73787,73300,72803,72353,
    71895,71457,71015,70593,70212,69828,69445,69099,68754,68430,68124,67837,67568,67304,67060,66845,
    66639,66450,66272,66121,65987,65866,65763,65684,65619,65573,65546,65537,65545,65571,65617,65681,
    65759,65861,65981,66114,66265,66442,66629,66836,67049,67292,67554,67823,68109,68414,68738,69082,
    69425,69808,70191,70572,70992,71433,71871,72327,72776,73271,73758,74264,74791,75338,75872,76424,
    76996,77590,78165,78759,79373,80007,80618,81248,81897,82566,83256,83915,84594,85293,86011,86751,
    87452,88174,88913,89674,90389,91124,91945,
};

static fixed_t yslope(uint8_t y)
{
	return yslopeTable[y];
}

static fixed_t distscale(uint8_t x)
{
	return distscaleTable[x];
}
#else
static fixed_t yslope(uint8_t y)
{
	fixed_t s;
	fseek(_g->fileYSlope, y * sizeof(fixed_t), SEEK_SET);
	fread(&s, sizeof(fixed_t), 1, _g->fileYSlope);
	return s;
}

static fixed_t distscale(uint8_t x)
{
	fixed_t d;
	fseek(_g->fileDistScale, x * sizeof(fixed_t), SEEK_SET);
	fread(&d, sizeof(fixed_t), 1, _g->fileDistScale);
	return d;
}
#endif


static fixed_t planeheight;
static fixed_t basexscale, baseyscale;

static void R_MapPlane(uint32_t y, uint32_t x1, uint32_t x2, draw_span_vars_t *dsvars)
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

static void R_MakeSpans(int32_t x, uint32_t t1, uint32_t b1, uint32_t t2, uint32_t b2, draw_span_vars_t *dsvars)
{
    static byte spanstart[SCREENHEIGHT];

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


#define ANGLETOSKYSHIFT         22

static const fixed_t skytexturemid = 100 * FRACUNIT;
static const fixed_t skyiscale = (FRACUNIT * 200) / ((SCREENHEIGHT - ST_HEIGHT) + 16);

static void R_DrawSky(visplane_t *pl)
{
	draw_column_vars_t dcvars;
	dcvars.translation = NULL;

	// Normal Doom sky, only one allowed per level
	dcvars.texturemid = skytexturemid;    // Default y-offset

	// Sky is always drawn full bright, i.e. colormaps[0] is used.
	// Because of this hack, sky is not affected by INVUL inverse mapping.
	// Until Boom fixed this.

	if (!(dcvars.colormap = fixedcolormap))
		dcvars.colormap = fullcolormap;          // killough 3/20/98

	// proff 09/21/98: Changed for high-res
	dcvars.iscale = skyiscale;

	const texture_t* tex = R_GetTexture(_g->skytexture);

	// killough 10/98: Use sky scrolling offset
	for (register int32_t x = pl->minx; (dcvars.x = x) <= pl->maxx; x++)
	{
		if ((dcvars.yl = pl->top[x]) != -1 && dcvars.yl <= (dcvars.yh = pl->bottom[x])) // dropoff overflow
		{
			int32_t xc = (viewangle + xtoviewangle(x)) >> ANGLETOSKYSHIFT;

			uint32_t r = R_GetColumn(tex, xc);
			const patch_t* patch = W_GetLumpByNum(HIWORD(r));
			xc = LOWORD(r);
			const column_t* column = (const column_t *) ((const byte *)patch + patch->columnofs[xc]);

			dcvars.source = (const byte*)column + 3;
			R_DrawColumn(&dcvars);
			Z_Free(patch);
		}
	}
}


static void R_DoDrawPlane(visplane_t *pl)
{
    if (pl->minx <= pl->maxx)
    {
        if (pl->picnum == _g->skyflatnum)
        {
            // sky flat
            R_DrawSky(pl);
        }
        else
        {
            // regular flat
#if defined FLAT_SPAN
            const int32_t stop = pl->maxx + 1;

            pl->top[pl->minx - 1] = pl->top[stop] = 0xff; // dropoff overflow

            uint16_t color = LOBYTE(pl->picnum);
            color = (color << 8) | color;

            for (register int32_t x = pl->minx; x <= stop; x++)
            {
                R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x], color);
            }
#else
            draw_span_vars_t dsvars;

            dsvars.source   = W_GetLumpByNum(firstflat + flattranslation[pl->picnum]);
            dsvars.colormap = R_LoadColorMap(pl->lightlevel);

            planeheight = D_abs(pl->height - viewz);

            const int32_t stop = pl->maxx + 1;

            pl->top[pl->minx - 1] = pl->top[stop] = 0xff; // dropoff overflow

            for (register int32_t x = pl->minx; x <= stop; x++)
            {
                R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x], &dsvars);
            }

            Z_Free(dsvars.source);
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
        visplane_t *pl = _g->visplanes[i];

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

static const int16_t viewheight  = SCREENHEIGHT - ST_SCALED_HEIGHT;

void R_ClearPlanes(void)
{
    // opening / clipping determination
    for (int8_t i = 0; i < SCREENWIDTH; i++)
        floorclip[i] = viewheight, ceilingclip[i] = -1;


    for (int8_t i = 0; i < MAXVISPLANES; i++)    // new code -- killough
        for (*_g->freehead = _g->visplanes[i], _g->visplanes[i] = NULL; *_g->freehead; )
            _g->freehead = &(*_g->freehead)->next;

    _g->lastopening = _g->openings;

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
	int16_t lastflat = W_GetNumForName("F_END")   - 1;
	int16_t numflats = lastflat - firstflat + 1;

	// Create translation table for global animation.

	flattranslation = Z_MallocStatic((numflats + 1) * sizeof(*flattranslation));

	for (int16_t i = 0; i < numflats; i++)
		flattranslation[i] = i;
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
