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
 *      Render floor and ceilings
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "r_defs.h"
#include "r_main.h"
#include "i_video.h"

#include "globdata.h"


#if defined FLAT_SPAN
static int16_t nukage;
#else
static int16_t firstflat;
static int16_t  animated_flat_basepic;
static int16_t __far* flattranslation;             // for global animation

static fixed_t planeheight;
static fixed_t basexscale, baseyscale;

#define MAXVISPLANES 32    /* must be a power of 2 */

static visplane_t __far* visplanes[MAXVISPLANES];
static visplane_t __far* freetail;
static visplane_t __far*__far* freehead;
#endif


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
byte R_GetPlaneColor(int16_t picnum, int16_t lightlevel)
{
	const uint8_t* colormap = R_LoadColorMap(lightlevel);

	if (picnum == -3)
		picnum = nukage;

	return colormap[picnum];
}

#else


static const fixed_t yslopeTable[VIEWWINDOWHEIGHT / 2] =
{
    132104,134218,136400,138655,140985,143395,145889,148471,151146,153919,156796,159783,162886,166111,
    169467,172961,176602,180400,184365,188508,192842,197379,202135,207126,212370,217886,223696,229825,
    236299,243148,250406,258111,266305,275036,284360,294337,305040,316551,328965,342392,356962,372827,
    390168,409200,430185,453438,479349,508400,541201,578525,621378,671089,729444,798915,883011,986895,
    1118481,1290555,1525201,1864135,2396745,3355443,5592405,16777216
};

static const uint16_t distscaleTable[VIEWWINDOWWIDTH] =
{
    0x6A75,
    0x6438,0x5E8C,0x58AB,0x5319,0x4D63,0x4800,0x42B6,0x3D8E,
    0x38B2,0x33CF,0x2F3C,0x2AAB,0x266B,0x2237,0x1E54,0x1AA1,
    0x1721,0x13C1,0x10C4,0x0DEB,0x0B4E,0x08FD,0x06E8,0x051D,
    0x0392,0x0249,0x014A,0x0094,0x0025,0x0001,0x0023,0x0091,
    0x0145,0x0242,0x038A,0x0514,0x06DC,0x08EF,0x0B3E,0x0DDA,
    0x10B0,0x13AC,0x1709,0x1A87,0x1E37,0x2218,0x264A,0x2A88,
    0x2F16,0x33A7,0x3887,0x3D60,0x4286,0x47CB,0x4D2D,0x52DF,
    0x586E,0x5E4A,0x63F4,
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


static void R_MapPlane(uint16_t y, uint16_t x1, uint16_t x2, draw_span_vars_t *dsvars)
{    
    const fixed_t distance = FixedMul(planeheight, yslope(y));
    dsvars->step = ((FixedMul(distance,basexscale) << 10) & 0xffff0000) | ((FixedMul(distance,baseyscale) >> 6) & 0x0000ffff);

    fixed_t length = FixedMul (distance, distscale(x1));
    int16_t angle = (viewangle + (((angle_t)xtoviewangleTable[x1]) << FRACBITS)) >> ANGLETOFINESHIFT;

    // killough 2/28/98: Add offsets
    uint32_t xfrac =  viewx + FixedMulAngle(length, finecosineapprox(angle));
    uint32_t yfrac = -viewy - FixedMulAngle(length, finesineapprox(  angle));

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

            draw_span_vars_t dsvars;

            dsvars.source   = W_GetLumpByNum(firstflat + flattranslation[pl->picnum]);
            dsvars.colormap = R_LoadColorMap(pl->lightlevel);

            planeheight = D_abs(pl->height - viewz);

            for (int16_t x = pl->minx; x <= stop; x++)
            {
                R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x], &dsvars);
            }

            Z_ChangeTagToCache(dsvars.source);
        }
    }
}


//
// RDrawPlanes
// At the end of each frame.
//

void R_DrawPlanes (void)
{
    static const fixed_t iprojection = (1L << FRACBITS) / (VIEWWINDOWWIDTH / 2);

    basexscale = FixedMul(viewsin,iprojection);
    baseyscale = FixedMul(viewcos,iprojection);

    for (int8_t i = 0; i < MAXVISPLANES; i++)
    {
        visplane_t __far* pl = visplanes[i];

        while(pl)
        {
            if(pl->modified)
                R_DoDrawPlane(pl);

            pl = pl->next;
        }
    }
}


//Planes are alloc'd with PU_LEVEL tag so are dumped at level
//end. This function resets the visplane arrays.
void R_ResetPlanes(void)
{
    memset(visplanes, 0, sizeof(visplanes));
    freetail = NULL;
    freehead = &freetail;
}


//
// R_ClearPlanes
// At begining of frame.
//

void R_ClearPlanes(void)
{
    for (int8_t i = 0; i < MAXVISPLANES; i++)
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
            freehead = &(*freehead)->next;
}


static visplane_t __far* new_visplane(uint16_t hash)
{
    visplane_t __far* check = freetail;

    if (!check)
        check = Z_CallocLevel(sizeof(visplane_t));
    else
    {
        if (!(freetail = freetail->next))
            freehead = &freetail;
    }

    check->next = visplanes[hash];
    visplanes[hash] = check;

    return check;
}


// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum,lightlevel,height) \
  ((uint16_t)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))


//
// R_FindPlane
//

visplane_t __far* R_FindPlane(fixed_t height, int16_t picnum, int16_t lightlevel)
{
    visplane_t __far* check;
    uint16_t hash;

    if (picnum == skyflatnum)
        height = lightlevel = 0;         // killough 7/19/98: most skies map together

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum,lightlevel,height);

    for (check=visplanes[hash]; check; check=check->next)  // killough
        if (height == check->height &&
                picnum == check->picnum &&
                lightlevel == check->lightlevel)
            return check;

    check = new_visplane(hash);         // killough

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = VIEWWINDOWWIDTH;
    check->maxx = -1;

    _fmemset(check->top, -1, sizeof(check->top));

    check->modified = false;

    return check;
}


/*
 * R_DupPlane
 *
 * create duplicate of existing visplane and set initial range
 */
visplane_t __far* R_DupPlane(const visplane_t __far* pl, int16_t start, int16_t stop)
{
    uint16_t hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
    visplane_t __far* new_pl = new_visplane(hash);

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->minx = start;
    new_pl->maxx = stop;

    _fmemset(new_pl->top, -1, sizeof(new_pl->top));

    new_pl->modified = false;

    return new_pl;
}


//
// R_CheckPlane
//
visplane_t __far* R_CheckPlane(visplane_t __far* pl, int16_t start, int16_t stop)
{
    int16_t intrl, intrh, unionl, unionh, x;

    if (start < pl->minx)
        intrl   = pl->minx, unionl = start;
    else
        unionl  = pl->minx,  intrl = start;

    if (stop  > pl->maxx)
        intrh   = pl->maxx, unionh = stop;
    else
        unionh  = pl->maxx, intrh  = stop;

    for (x=intrl ; x <= intrh && pl->top[x] == 0xff; x++) // dropoff overflow
        ;

    if (x > intrh) { /* Can use existing plane; extend range */
        pl->minx = unionl; pl->maxx = unionh;
        return pl;
    } else /* Cannot use existing plane; create a new one */
        return R_DupPlane(pl,start,stop);
}
#endif


//
// R_InitFlats
//

void R_InitFlats(void)
{
#if !defined FLAT_SPAN
	       firstflat = W_GetNumForName("F_START") + 1;

	int16_t lastflat = W_GetNumForName("F_END")   - 1;
	int16_t numflats = lastflat - firstflat + 1;

	// Create translation table for global animation.

	flattranslation = Z_MallocStatic((numflats + 1) * sizeof(*flattranslation));

	animated_flat_basepic = R_FlatNumForName("NUKAGE1");

	for (int16_t i = 0; i < numflats; i++)
		flattranslation[i] = i;
#endif
}


//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
//
#if !defined FLAT_SPAN
int16_t R_FlatNumForName(const char *name)
{
	int16_t i = W_GetNumForName(name);
	return i - firstflat;
}
#endif


#if !defined FLAT_NUKAGE1_COLOR
#define FLAT_NUKAGE1_COLOR 122
#endif

void P_UpdateAnimatedFlat(void)
{
#if defined FLAT_SPAN
	nukage = FLAT_NUKAGE1_COLOR + (((int16_t)_g_leveltime >> 3) % 3) * 2;
#else
	int16_t pic = animated_flat_basepic + (((int16_t)_g_leveltime >> 3) % 3);

	flattranslation[animated_flat_basepic + 0] = pic;
	flattranslation[animated_flat_basepic + 1] = pic;
	flattranslation[animated_flat_basepic + 2] = pic;
#endif
}
