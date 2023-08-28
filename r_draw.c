/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023 by
 *  Frenkel Smeijers
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
 *      Rendering main loop and setup functions,
 *       utility functions (BSP, geometry, trigonometry).
 *      See tables.c, too.
 *
 *-----------------------------------------------------------------------------*/

//This is to keep the codesize under control.
//This whole file needs to fit within IWRAM.
#pragma GCC optimize ("Os")


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "d_net.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "m_bbox.h"
#include "m_fixed.h"
#include "v_video.h"
#include "st_stuff.h"
#include "i_system.h"
#include "g_game.h"
#include "m_random.h"

#include "globdata.h"


#if 0
static const int8_t viewangletoxTable[4096];
static const angle_t _huge tantoangleTable[2049];
static const fixed_t _huge finetangentTable[4096];


static const angle_t xtoviewangleTable[121] =
{
    537395200,531628032,525336576,519569408,513802240,507510784,501219328,494927872,488636416,
    481820672,475004928,468189184,461373440,454557696,447217664,439877632,432537600,425197568,
    417857536,409993216,402128896,394264576,386400256,378011648,369623040,361234432,352845824,
    343932928,335020032,326107136,317194240,307757056,298844160,289406976,279969792,270008320,
    260046848,250609664,240648192,230162432,220200960,209715200,199229440,188743680,178257920,
    167772160,156762112,145752064,135266304,124256256,113246208,101711872,90701824,79691776,
    68157440,56623104,45613056,34078720,22544384,11534336,0,4283432960,4272422912,4260888576,
    4249354240,4238344192,4226809856,4215275520,4204265472,4193255424,4181721088,4170711040,
    4159700992,4149215232,4138205184,4127195136,4116709376,4106223616,4095737856,4085252096,
    4074766336,4064804864,4054319104,4044357632,4034920448,4024958976,4014997504,4005560320,
    3996123136,3987210240,3977773056,3968860160,3959947264,3951034368,3942121472,3933732864,
    3925344256,3916955648,3908567040,3900702720,3892838400,3884974080,3877109760,3869769728,
    3862429696,3855089664,3847749632,3840409600,3833593856,3826778112,3819962368,3813146624,
    3806330880,3800039424,3793747968,3787456512,3781165056,3775397888,3769630720,3763339264,3221225472,
};


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
#endif


#if 0
static int8_t viewangletox(int16_t viewangle)
{
	return viewangletoxTable[viewangle];
}

static angle_t tantoangle(int16_t tan)
{
	return tantoangleTable[tan];
}

static fixed_t finetangent(int16_t x)
{
	return finetangentTable[x];
}

static angle_t xtoviewangle(int8_t x)
{
	return xtoviewangleTable[x];
}

static fixed_t yslope(uint8_t y)
{
	return yslopeTable[y];
}

static fixed_t distscale(uint8_t x)
{
	return distscaleTable[x];
}
#else
static int8_t viewangletox(int16_t viewangle)
{
	int8_t x;
	fseek(_g->fileViewAngleToX, viewangle * sizeof(int8_t), SEEK_SET);
	fread(&x, sizeof(int8_t), 1, _g->fileViewAngleToX);
	return x;
}

static angle_t tantoangle(int16_t tan)
{
	angle_t angle;
	fseek(_g->fileTanToAngle, tan * sizeof(angle_t), SEEK_SET);
	fread(&angle, sizeof(angle_t), 1, _g->fileTanToAngle);
	return angle;
}

static fixed_t finetangent(int16_t x)
{
	fixed_t f;
	fseek(_g->fileFineTan, x * sizeof(fixed_t), SEEK_SET);
	fread(&f, sizeof(fixed_t), 1, _g->fileFineTan);
	return f;
}

static angle_t xtoviewangle(int8_t x)
{
	angle_t viewangle;
	fseek(_g->fileXToViewAngle, x * sizeof(angle_t), SEEK_SET);
	fread(&viewangle, sizeof(angle_t), 1, _g->fileXToViewAngle);
	return viewangle;
}

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


static uint32_t columnCacheEntries[128];

static int16_t floorclip[SCREENWIDTH];
static int16_t ceilingclip[SCREENWIDTH];

static vissprite_t* vissprite_ptrs[96];

static int16_t screenheightarray[SCREENWIDTH] =
{
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,

	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128,	128, 128, 128, 128, 128, 128
};

static int16_t negonearray[SCREENWIDTH] =
{
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,		-1, -1, -1, -1, -1, -1
};


//*****************************************
//Column cache stuff.
//GBA has 16kb of Video Memory for columns
//*****************************************

static byte columnCache[128*128];


//*****************************************
//Globals.
//*****************************************

int32_t numnodes;
const mapnode_t *nodes;

fixed_t  viewx, viewy, viewz;

angle_t  viewangle;

static byte solidcol[SCREENWIDTH];

static byte spanstart[SCREENHEIGHT];                // killough 2/8/98


static const seg_t     *curline;
static side_t    *sidedef;
static const line_t    *linedef;
static sector_t  *frontsector;
static sector_t  *backsector;
static drawseg_t *ds_p;

static visplane_t *floorplane, *ceilingplane;
static int32_t             rw_angle1;

static angle_t         rw_normalangle; // angle to line origin
static fixed_t         rw_distance;

static int32_t      rw_stopx;

static fixed_t  rw_scale;
static fixed_t  rw_scalestep;

static int32_t      worldtop;
static int32_t      worldbottom;

static int32_t didsolidcol; /* True if at least one column was marked solid */

// True if any of the segs textures might be visible.
static boolean  segtextured;
static boolean  markfloor;      // False if the back side is the same plane.
static boolean  markceiling;
static boolean  maskedtexture;
static int32_t      toptexture;
static int32_t      bottomtexture;
static int32_t      midtexture;

static fixed_t  rw_midtexturemid;
static fixed_t  rw_toptexturemid;
static fixed_t  rw_bottomtexturemid;

const lighttable_t *fullcolormap;
const lighttable_t *colormaps;

const lighttable_t* fixedcolormap;

int32_t extralight;                           // bumped light from gun blasts


static int16_t   *mfloorclip;   // dropoff overflow
static int16_t   *mceilingclip; // dropoff overflow
static fixed_t spryscale;
static fixed_t sprtopscreen;

static angle_t  rw_centerangle;
static fixed_t  rw_offset;
static int32_t      rw_lightlevel;

static int16_t      *maskedtexturecol; // dropoff overflow

fixed_t   *textureheight; //needed for texture pegging (and TFE fix - killough)

int16_t       *flattranslation;             // for global animation
int16_t       *texturetranslation;

static fixed_t basexscale, baseyscale;

fixed_t  viewcos, viewsin;

static fixed_t  topfrac;
static fixed_t  topstep;
static fixed_t  bottomfrac;
static fixed_t  bottomstep;

static fixed_t  pixhigh;
static fixed_t  pixlow;

static fixed_t  pixhighstep;
static fixed_t  pixlowstep;

static int32_t      worldhigh;
static int32_t      worldlow;

static lighttable_t current_colormap[256];
static const lighttable_t* current_colormap_ptr;

static fixed_t planeheight;

static size_t num_vissprite;

boolean highDetail = false;



//*****************************************
// Constants
//*****************************************

static const int32_t viewheight = SCREENHEIGHT-ST_SCALED_HEIGHT;
static const int32_t centery = (SCREENHEIGHT-ST_SCALED_HEIGHT)/2;
static const int32_t centerxfrac = ((int32_t)(SCREENWIDTH/2)) << FRACBITS;
static const int32_t centeryfrac = ((int32_t)((SCREENHEIGHT-ST_SCALED_HEIGHT)/2)) << FRACBITS;

static const fixed_t projection = ((int32_t)(SCREENWIDTH/2)) << FRACBITS;

static const fixed_t projectiony = ((SCREENHEIGHT * (SCREENWIDTH/2L) * 320) / 200) / SCREENWIDTH * FRACUNIT;

static const fixed_t pspritescale = FRACUNIT*SCREENWIDTH/320;
static const fixed_t pspriteiscale = FRACUNIT*320/SCREENWIDTH;

static const fixed_t pspriteyscale = (((int32_t)SCREENHEIGHT) << FRACBITS) / 200;
static const fixed_t pspriteyiscale = ((UINT32_MAX) / ((((int32_t)SCREENHEIGHT) << FRACBITS) / 200));


static const angle_t clipangle = 537395200; //xtoviewangle(0);

static const int32_t skytexturemid = 100*FRACUNIT;
static const fixed_t skyiscale = (FRACUNIT*200)/((SCREENHEIGHT-ST_HEIGHT)+16);


//********************************************
// On the GBA we exploit that an 8 bit write
// will mirror to the upper 8 bits too.
// it saves an OR and Shift per pixel.
//********************************************
typedef uint16_t pixel;


//********************************************
// This goes here as we want the Thumb code
// to BX to ARM as Thumb long mul is very slow.
//********************************************
#if defined __WATCOMC__
//
#else
inline
#endif
fixed_t CONSTFUNC FixedMul(fixed_t a, fixed_t b)
{
    return (fixed_t)((int64_t) a*b >> FRACBITS);
}


static uint32_t reciprocal(uint32_t val)
{
#if 1
	if (val == 0)
		return 0;

	return 4294967296 / val;
#else
	return reciprocalTable[val];
#endif
}


//Approx Reciprocal of v
inline static CONSTFUNC fixed_t FixedReciprocal(fixed_t v)
{
    uint32_t val = v < 0 ? -v : v;

    uint32_t shift = 0;

    while(val > (((int32_t)1) << FRACBITS))
    {
        val = (val >> 1u);
        shift++;
    }

    fixed_t result = (reciprocal(val) >> shift);

    return v < 0 ? -result : result;
}


//Approx fixed point divide of a/b using reciprocal. -> a * (1/b).
inline static CONSTFUNC fixed_t FixedApproxDiv(fixed_t a, fixed_t b)
{
    return FixedMul(a, FixedReciprocal(b));
}


// killough 5/3/98: reformatted

#define SLOPERANGE 2048

static CONSTFUNC int32_t SlopeDiv(uint32_t num, uint32_t den)
{
    den = den >> 8;

    if (den == 0)
        return SLOPERANGE;

    const uint32_t ans = FixedApproxDiv(num << 3, den) >> FRACBITS;

    return (ans <= SLOPERANGE) ? ans : SLOPERANGE;
}

//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
// killough 5/2/98: reformatted
//

static PUREFUNC int32_t R_PointOnSide(fixed_t x, fixed_t y, const mapnode_t *node)
{
    fixed_t dx = (fixed_t)node->dx << FRACBITS;
    fixed_t dy = (fixed_t)node->dy << FRACBITS;

    fixed_t nx = (fixed_t)node->x << FRACBITS;
    fixed_t ny = (fixed_t)node->y << FRACBITS;

    if (!dx)
        return x <= nx ? node->dy > 0 : node->dy < 0;

    if (!dy)
        return y <= ny ? node->dx < 0 : node->dx > 0;

    x -= nx;
    y -= ny;

    // Try to quickly decide by looking at sign bits.
    if ((dy ^ dx ^ x ^ y) < 0)
        return (dy ^ x) < 0;  // (left is negative)

    return FixedMul(y, node->dx) >= FixedMul(node->dy, x);
}

//
// R_PointInSubsector
//
// killough 5/2/98: reformatted, cleaned up

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    int32_t nodenum = numnodes-1;

    // special case for trivial maps (single subsector, no nodes)
    if (numnodes == 0)
        return _g->subsectors;

    while (!(nodenum & NF_SUBSECTOR))
        nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];
    return &_g->subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangleTable[] table.
//


CONSTFUNC angle_t R_PointToAngle2(fixed_t vx, fixed_t vy, fixed_t x, fixed_t y)
{
    x -= vx;
    y -= vy;

    if ( (!x) && (!y) )
        return 0;

    if (x>= 0)
    {
        // x >=0
        if (y>= 0)
        {
            // y>= 0

            if (x>y)
            {
                // octant 0
                return tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 1
                return ANG90-1-tantoangle(SlopeDiv(x,y));
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 8
                return -tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 7
                return ANG270+tantoangle(SlopeDiv(x,y));
            }
        }
    }
    else
    {
        // x<0
        x = -x;

        if (y>= 0)
        {
            // y>= 0
            if (x>y)
            {
                // octant 3
                return ANG180-1-tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 2
                return ANG90+ tantoangle(SlopeDiv(x,y));
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 4
                return ANG180+tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 5
                return ANG270-1-tantoangle(SlopeDiv(x,y));
            }
        }
    }
}

static CONSTFUNC angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
    return R_PointToAngle2(viewx, viewy, x, y);
}


// killough 5/2/98: move from r_main.c, made static, simplified

#define SLOPEBITS    11
#define DBITS      (FRACBITS-SLOPEBITS)

static CONSTFUNC fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
    fixed_t dx = D_abs(x - viewx);
    fixed_t dy = D_abs(y - viewy);

    if (dy > dx)
    {
        fixed_t t = dx;
        dx = dy;
        dy = t;
    }

    return FixedApproxDiv(dx, finesine((tantoangle(FixedApproxDiv(dy,dx) >> DBITS) + ANG90) >> ANGLETOFINESHIFT));
}


// Lighting constants.

#define LIGHTSEGSHIFT      4


// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.

#define NUMCOLORMAPS 32


static const lighttable_t* R_ColourMap(int32_t lightlevel)
{
    if (fixedcolormap)
        return fixedcolormap;
    else
    {
        if (curline)
        {
            if (curline->v1.y == curline->v2.y)
                lightlevel -= 1 << LIGHTSEGSHIFT;
            else if (curline->v1.x == curline->v2.x)
                lightlevel += 1 << LIGHTSEGSHIFT;
        }

        lightlevel += (extralight +_g->gamma) << LIGHTSEGSHIFT;

        int32_t cm = ((256-lightlevel)>>2) - 24;

        if(cm >= NUMCOLORMAPS)
            cm = NUMCOLORMAPS-1;
        else if(cm < 0)
            cm = 0;

        return fullcolormap + cm*256;
    }
}


//Load a colormap into IWRAM.
static const lighttable_t* R_LoadColorMap(int32_t lightlevel)
{
    const lighttable_t* lm = R_ColourMap(lightlevel);

    if(current_colormap_ptr != lm)
    {
        memcpy(current_colormap, lm, 256);
        current_colormap_ptr = lm;
    }

    return current_colormap;
}

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

#pragma GCC push_options
#pragma GCC optimize ("Ofast")

#define COLEXTRABITS 9
#define COLBITS (FRACBITS + COLEXTRABITS)

inline static void R_DrawColumnPixel(uint16_t* dest, const byte* source, const byte* colormap, uint32_t frac)
{
    pixel* d = (pixel*)dest;

    uint32_t color = colormap[source[frac>>COLBITS]];

    *d = (color | (color << 8));
}


// Packaged into a struct - POPE
typedef struct {
  int32_t                 x;
  int32_t                 yl;
  int32_t                 yh;
  fixed_t             iscale;
  fixed_t             texturemid;

  const byte          *source; // first pixel in a column

  const lighttable_t  *colormap;
  const byte          *translation;

  boolean             odd_pixel;

} draw_column_vars_t;


static void R_DrawColumn (const draw_column_vars_t *dcvars)
{
    int32_t count = (dcvars->yh - dcvars->yl) + 1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0)
        return;

    const byte *source = dcvars->source;
    const byte *colormap = dcvars->colormap;

    uint16_t* dest = _g->screen + ScreenYToOffset(dcvars->yl) + dcvars->x;

    const uint32_t		fracstep = (dcvars->iscale << COLEXTRABITS);
    uint32_t frac = (dcvars->texturemid + (dcvars->yl - centery)*dcvars->iscale) << COLEXTRABITS;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.

    uint32_t l = (count >> 4);

    while(l--)
    {
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;

        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;

        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;

        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
    }

    uint32_t r = (count & 15);

    switch(r)
    {
        case 15:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 14:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 13:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 12:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 11:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 10:    R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 9:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 8:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 7:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 6:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 5:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 4:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 3:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 2:     R_DrawColumnPixel(dest, source, colormap, frac); dest+=SCREENWIDTH; frac+=fracstep;
        case 1:     R_DrawColumnPixel(dest, source, colormap, frac);
    }
}

static void R_DrawColumnHiRes(const draw_column_vars_t *dcvars)
{
    int32_t count = (dcvars->yh - dcvars->yl) + 1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0)
        return;

    const byte *source = dcvars->source;
    const byte *colormap = dcvars->colormap;

    volatile uint16_t* dest = _g->screen + ScreenYToOffset(dcvars->yl) + dcvars->x;

    const uint32_t		fracstep = (dcvars->iscale << COLEXTRABITS);
    uint32_t frac = (dcvars->texturemid + (dcvars->yl - centery)*dcvars->iscale) << COLEXTRABITS;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.

    uint16_t mask;
    uint8_t shift;

    if(!dcvars->odd_pixel)
    {
        mask  = 0xff00;
        shift = 0;
    }
    else
    {
        mask  = 0xff;
        shift = 8;
    }

    while(count--)
    {
        uint32_t old = *dest;
        uint32_t color = colormap[source[frac>>COLBITS]];

        *dest = ((old & mask) | (color << shift));

        dest += SCREENWIDTH;
        frac += fracstep;
    }
}

#define FUZZOFF (SCREENWIDTH)
#define FUZZTABLE 50

static const int32_t fuzzoffset[FUZZTABLE] =
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
static void R_DrawFuzzColumn (const draw_column_vars_t *dcvars)
{
    int32_t dc_yl = dcvars->yl;
    int32_t dc_yh = dcvars->yh;

    // Adjust borders. Low...
    if (dc_yl <= 0)
        dc_yl = 1;

    // .. and high.
    if (dc_yh >= viewheight-1)
        dc_yh = viewheight - 2;

    int32_t count = (dc_yh - dc_yl) + 1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0)
        return;

    const byte* colormap = &fullcolormap[6*256];

    uint16_t* dest = _g->screen + ScreenYToOffset(dc_yl) + dcvars->x;

    uint32_t fuzzpos = _g->fuzzpos;

    do
    {        
        R_DrawColumnPixel(dest, (const byte*)&dest[fuzzoffset[fuzzpos]], colormap, 0); dest += SCREENWIDTH;  fuzzpos++;

        if(fuzzpos >= 50)
            fuzzpos = 0;

    } while(--count);

    _g->fuzzpos = fuzzpos;
}

#pragma GCC pop_options



//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//

typedef void (*R_DrawColumn_f)(const draw_column_vars_t *dcvars);

static void R_DrawMaskedColumn(R_DrawColumn_f colfunc, draw_column_vars_t *dcvars, const column_t *column)
{
    const fixed_t basetexturemid = dcvars->texturemid;

    const int16_t fclip_x = mfloorclip[dcvars->x];
    const int16_t cclip_x = mceilingclip[dcvars->x];

    while (column->topdelta != 0xff)
    {
        // calculate unclipped screen coordinates for post
        const int32_t topscreen = sprtopscreen + spryscale*column->topdelta;
        const int32_t bottomscreen = topscreen + spryscale*column->length;

        int32_t yh = (bottomscreen-1)>>FRACBITS;
        int32_t yl = (topscreen+FRACUNIT-1)>>FRACBITS;

        if (yh >= fclip_x)
            yh = fclip_x - 1;

        if (yl <= cclip_x)
            yl = cclip_x + 1;

        // killough 3/2/98, 3/27/98: Failsafe against overflow/crash:
        if (yh < viewheight && yl <= yh)
        {
            dcvars->source =  (const byte*)column + 3;

            dcvars->texturemid = basetexturemid - (((int32_t)column->topdelta)<<FRACBITS);

            dcvars->yh = yh;
            dcvars->yl = yl;

            // Drawn by either R_DrawColumn
            //  or (SHADOW) R_DrawFuzzColumn.
            colfunc (dcvars);
        }

        column = (const column_t *)((const byte *)column + column->length + 4);
    }

    dcvars->texturemid = basetexturemid;
}


static void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars)
{
	dcvars->x           = 0;
	dcvars->yl          = 0;
	dcvars->yh          = 0;
	dcvars->iscale      = 0;
	dcvars->texturemid  = 0;
	dcvars->source      = NULL;
	dcvars->colormap    = colormaps;
	dcvars->translation = NULL;
}


//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
// CPhipps - new wad lump handling, *'s to const*'s
static void R_DrawVisSprite(const vissprite_t *vis)
{
    fixed_t  frac;

    R_DrawColumn_f colfunc = R_DrawColumn;
    draw_column_vars_t dcvars;
    boolean hires = false;

    R_SetDefaultDrawColumnVars(&dcvars);

    dcvars.colormap = vis->colormap;

    // killough 4/11/98: rearrange and handle translucent sprites
    // mixed with translucent/non-translucenct 2s normals

    if (!dcvars.colormap)   // NULL colormap = shadow draw
        colfunc = R_DrawFuzzColumn;    // killough 3/14/98
    else
    {
        hires = highDetail;

        if(hires)
            colfunc = R_DrawColumnHiRes;
    }

    // proff 11/06/98: Changed for high-res
    dcvars.iscale = vis->iscale;
    dcvars.texturemid = vis->texturemid;
    frac = vis->startfrac;

    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dcvars.texturemid, spryscale);


    const patch_t *patch = W_GetLumpByNum(vis->patch_num);

    fixed_t xiscale = vis->xiscale;

    if(hires)
        xiscale >>= 1;

    dcvars.x = vis->x1;
    dcvars.odd_pixel = false;

    while(dcvars.x < SCREENWIDTH)
    {
        const column_t* column = (const column_t *) ((const byte *)patch + patch->columnofs[frac >> FRACBITS]);
        R_DrawMaskedColumn(colfunc, &dcvars, column);

        frac += xiscale;

        if(((frac >> FRACBITS) >= patch->width) || frac < 0)
            break;

        dcvars.odd_pixel = true;

        if(!hires)
            dcvars.x++;

        if(dcvars.x >= SCREENWIDTH)
            break;

        const column_t* column2 = (const column_t *) ((const byte *)patch + patch->columnofs[frac >> FRACBITS]);
        R_DrawMaskedColumn(colfunc, &dcvars, column2);

        frac += xiscale;

        if(((frac >> FRACBITS) >= patch->width) || frac < 0)
            break;

        dcvars.x++;
        dcvars.odd_pixel = false;
    }

    Z_Free(patch);
}


static uint32_t R_GetColumn(const texture_t* texture, int32_t texcolumn)
{
    const uint8_t patchcount = texture->patchcount;
    const uint16_t widthmask = texture->widthmask;

    const int32_t xc = texcolumn & widthmask;

    if (patchcount == 1)
    {
        //simple texture.
        return (((uint32_t)texture->patches[0].patch_num) << 16) + xc;
    }
    else
    {
        uint8_t i = 0;

        do
        {
            const texpatch_t* patch = &texture->patches[i];

            const int16_t x1 = patch->originx;

            if (xc < x1)
                continue;

            const int16_t x2 = x1 + V_NumPatchWidth(patch->patch_num);

            if (xc < x2)
                return (((uint32_t)patch->patch_num) << 16) + xc - x1;

        } while (++i < patchcount);
    }

    I_Error("R_GetColumn: can't find texcolumn");
    return 0;
}


#define LOWORD(dw)	(((uint16_t *)&dw)[0])
#define HIWORD(dw)	(((uint16_t *)&dw)[1])

//
// R_RenderMaskedSegRange
//

static void R_RenderMaskedSegRange(const drawseg_t *ds, int32_t x1, int32_t x2)
{
    int16_t      texnum;
    draw_column_vars_t dcvars;

    R_SetDefaultDrawColumnVars(&dcvars);

    // Calculate light table.
    // Use different light tables
    //   for horizontal / vertical / diagonal. Diagonal?

    curline = ds->curline;  // OPTIMIZE: get rid of LIGHTSEGSHIFT globally

    frontsector = SG_FRONTSECTOR(curline);
    backsector = SG_BACKSECTOR(curline);

    texnum = _g->sides[curline->sidenum].midtexture;
    texnum = texturetranslation[texnum];

    // killough 4/13/98: get correct lightlevel for 2s normal textures
    rw_lightlevel = frontsector->lightlevel;

    maskedtexturecol = ds->maskedtexturecol;

    rw_scalestep = ds->scalestep;
    spryscale = ds->scale1 + (x1 - ds->x1)*rw_scalestep;
    mfloorclip = ds->sprbottomclip;
    mceilingclip = ds->sprtopclip;

    // find positioning
    if (_g->lines[curline->linenum].flags & ML_DONTPEGBOTTOM)
    {
        dcvars.texturemid = frontsector->floorheight > backsector->floorheight
                ? frontsector->floorheight : backsector->floorheight;
        dcvars.texturemid = dcvars.texturemid + textureheight[texnum] - viewz;
    }
    else
    {
        dcvars.texturemid =frontsector->ceilingheight<backsector->ceilingheight
                ? frontsector->ceilingheight : backsector->ceilingheight;
        dcvars.texturemid = dcvars.texturemid - viewz;
    }

    dcvars.texturemid += (((int32_t)_g->sides[curline->sidenum].rowoffset) << FRACBITS);

    const texture_t* texture = R_GetTexture(texnum);

    dcvars.colormap = R_LoadColorMap(rw_lightlevel);

    // draw the columns
    for (dcvars.x = x1 ; dcvars.x <= x2 ; dcvars.x++, spryscale += rw_scalestep)
    {
        int16_t xc = maskedtexturecol[dcvars.x];

        if (xc != SHRT_MAX) // dropoff overflow
        {
            sprtopscreen = centeryfrac - FixedMul(dcvars.texturemid, spryscale);

            dcvars.iscale = FixedReciprocal((uint32_t)spryscale);

            // draw the texture
            uint32_t r = R_GetColumn(texture, xc);
            const patch_t* patch = W_GetLumpByNum(HIWORD(r));
            xc = LOWORD(r);
            const column_t* column = (const column_t *) ((const byte *)patch + patch->columnofs[xc]);

            R_DrawMaskedColumn(R_DrawColumn, &dcvars, column);
            Z_Free(patch);
            maskedtexturecol[dcvars.x] = SHRT_MAX; // dropoff overflow
        }
    }

    curline = NULL; /* cph 2001/11/18 - must clear curline now we're done with it, so R_ColourMap doesn't try using it for other things */
}


// killough 5/2/98: reformatted

static PUREFUNC int32_t R_PointOnSegSide(fixed_t x, fixed_t y, const seg_t *line)
{
    const fixed_t lx = line->v1.x;
    const fixed_t ly = line->v1.y;
    const fixed_t ldx = line->v2.x - lx;
    const fixed_t ldy = line->v2.y - ly;

    if (!ldx)
        return x <= lx ? ldy > 0 : ldy < 0;

    if (!ldy)
        return y <= ly ? ldx < 0 : ldx > 0;

    x -= lx;
    y -= ly;

    // Try to quickly decide by looking at sign bits.
    if ((ldy ^ ldx ^ x ^ y) < 0)
        return (ldy ^ x) < 0;          // (left is negative)

    return FixedMul(y, ldx>>FRACBITS) >= FixedMul(ldy>>FRACBITS, x);
}


//
// R_DrawSprite
//

static void R_DrawSprite (const vissprite_t* spr)
{
    int16_t* clipbot = floorclip;
    int16_t* cliptop = ceilingclip;

    fixed_t scale;
    fixed_t lowscale;

    for (int32_t x = spr->x1 ; x<=spr->x2 ; x++)
    {
        clipbot[x] = viewheight;
        cliptop[x] = -1;
    }


    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.

    // Modified by Lee Killough:
    // (pointer check was originally nonportable
    // and buggy, by going past LEFT end of array):

    const drawseg_t* drawsegs  =_g->drawsegs;

    for (const drawseg_t* ds = ds_p; ds-- > drawsegs; )  // new -- killough
    {
        // determine if the drawseg obscures the sprite
        if (ds->x1 > spr->x2 || ds->x2 < spr->x1 || (!ds->silhouette && !ds->maskedtexturecol))
            continue;      // does not cover sprite

        const int32_t r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
        const int32_t r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

        if (ds->scale1 > ds->scale2)
        {
            lowscale = ds->scale2;
            scale = ds->scale1;
        }
        else
        {
            lowscale = ds->scale1;
            scale = ds->scale2;
        }

        if (scale < spr->scale || (lowscale < spr->scale && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
        {
            if (ds->maskedtexturecol)       // masked mid texture?
                R_RenderMaskedSegRange(ds, r1, r2);

            continue;               // seg is behind sprite
        }

        // clip this piece of the sprite
        // killough 3/27/98: optimized and made much shorter

        if (ds->silhouette & SIL_BOTTOM && spr->gz < ds->bsilheight) //bottom sil
        {
            for (int32_t x = r1; x <= r2; x++)
            {
                if (clipbot[x] == viewheight)
                    clipbot[x] = ds->sprbottomclip[x];
            }

        }

        fixed_t gzt = spr->gz + (((int32_t)spr->patch_topoffset) << FRACBITS);

        if (ds->silhouette & SIL_TOP && gzt > ds->tsilheight)   // top sil
        {
            for (int32_t x=r1; x <= r2; x++)
            {
                if (cliptop[x] == -1)
                    cliptop[x] = ds->sprtopclip[x];
            }
        }
    }

    // all clipping has been performed, so draw the sprite
    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr);
}


//
// R_DrawPSprite
//

#define BASEYCENTER 100

static void R_DrawPSprite (pspdef_t *psp, int32_t lightlevel)
{
    int32_t           x1, x2;
    spritedef_t   *sprdef;
    spriteframe_t *sprframe;
    boolean       flip;
    vissprite_t   *vis;
    vissprite_t   avis;
    int32_t           width;
    fixed_t       topoffset;

    // decide which patch to use
    sprdef = &_g->sprites[psp->state->sprite];

    sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

    flip = (boolean) SPR_FLIPPED(sprframe, 0);

    const patch_t* patch = W_GetLumpByNum(sprframe->lump[0]+_g->firstspritelump);
    // calculate edges of the shape
    fixed_t       tx;
    tx = psp->sx-160*FRACUNIT;

    tx -= ((int32_t)patch->leftoffset)<<FRACBITS;
    x1 = (centerxfrac + FixedMul (tx, pspritescale))>>FRACBITS;

    tx += ((int32_t)patch->width)<<FRACBITS;
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    width = patch->width;
    topoffset = ((int32_t)patch->topoffset)<<FRACBITS;



    // off the side
    if (x2 < 0 || x1 > SCREENWIDTH)
        return;

    // store information in a vissprite
    vis = &avis;
    vis->mobjflags = 0;
    // killough 12/98: fix psprite positioning problem
    vis->texturemid = (((int32_t)BASEYCENTER)<<FRACBITS) /* +  FRACUNIT/2 */ -
            (psp->sy-topoffset);
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= SCREENWIDTH ? SCREENWIDTH-1 : x2;
    // proff 11/06/98: Added for high-res
    vis->scale = pspriteyscale;
    vis->iscale = pspriteyiscale;

    if (flip)
    {
        vis->xiscale = - pspriteiscale;
        vis->startfrac = (width<<FRACBITS)-1;
    }
    else
    {
        vis->xiscale = pspriteiscale;
        vis->startfrac = 0;
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale*(vis->x1-x1);

    vis->patch_num       = sprframe->lump[0] + _g->firstspritelump;
    vis->patch_topoffset = patch->topoffset;
    Z_Free(patch);

    if (_g->player.powers[pw_invisibility] > 4*32 || _g->player.powers[pw_invisibility] & 8)
        vis->colormap = NULL;                    // shadow draw
    else if (fixedcolormap)
        vis->colormap = fixedcolormap;           // fixed color
    else if (psp->state->frame & FF_FULLBRIGHT)
        vis->colormap = fullcolormap;            // full bright // killough 3/20/98
    else
        vis->colormap = R_LoadColorMap(lightlevel);  // local light

    R_DrawVisSprite(vis);
}



//
// R_DrawPlayerSprites
//

static void R_DrawPlayerSprites(void)
{

  int32_t i, lightlevel = _g->player.mo->subsector->sector->lightlevel;
  pspdef_t *psp;

  // clip to screen bounds
  mfloorclip = screenheightarray;
  mceilingclip = negonearray;

  // add all active psprites
  for (i=0, psp=_g->player.psprites; i<NUMPSPRITES; i++,psp++)
    if (psp->state)
      R_DrawPSprite (psp, lightlevel);
}


//
// R_SortVisSprites
//
// Rewritten by Lee Killough to avoid using unnecessary
// linked lists, and to use faster sorting algorithm.
//
static int compare (const void* l, const void* r)
{
    const vissprite_t* vl = *(const vissprite_t**)l;
    const vissprite_t* vr = *(const vissprite_t**)r;

    return vr->scale - vl->scale;
}

static void R_SortVisSprites (void)
{
    int32_t i = num_vissprite;

    if (i)
    {
        while (--i>=0)
            vissprite_ptrs[i] = _g->vissprites+i;

        qsort(vissprite_ptrs, num_vissprite, sizeof (vissprite_t*), compare);
    }
}

//
// R_DrawMasked
//

static void R_DrawMasked(void)
{
    int32_t i;
    drawseg_t *ds;
    drawseg_t* drawsegs = _g->drawsegs;


    R_SortVisSprites();

    // draw all vissprites back to front
    for (i = num_vissprite ;--i>=0; )
        R_DrawSprite(vissprite_ptrs[i]);         // killough

    // render any remaining masked mid textures

    // Modified by Lee Killough:
    // (pointer check was originally nonportable
    // and buggy, by going past LEFT end of array):
    for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    R_DrawPlayerSprites ();
}


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

#pragma GCC push_options
#pragma GCC optimize ("Ofast")

inline static void R_DrawSpanPixel(uint16_t* dest, const byte* source, const byte* colormap, uint32_t position)
{
    pixel* d = (pixel*)dest;

    uint32_t color = colormap[source[((position >> 4) & 0x0fc0) | (position >> 26)]];

    *d = (color | (color << 8));
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

    uint32_t r = (count & 15);

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

#pragma GCC pop_options

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
    for (; t1 < t2 && t1 <= b1; t1++)
        R_MapPlane(t1, spanstart[t1], x, dsvars);

    for (; b1 > b2 && b1 >= t1; b1--)
        R_MapPlane(b1, spanstart[b1], x, dsvars);

    while (t2 < t1 && t2 <= b2)
        spanstart[t2++] = x;

    while (b2 > b1 && b2 >= t2)
        spanstart[b2--] = x;
}



// New function, by Lee Killough

/* The sky map is 256*128*4 maps. */
#define ANGLETOSKYSHIFT         22

static void R_DoDrawPlane(visplane_t *pl)
{
    register int32_t x;
    draw_column_vars_t dcvars;

    R_SetDefaultDrawColumnVars(&dcvars);

    if (pl->minx <= pl->maxx)
    {
        if (pl->picnum == _g->skyflatnum)
        { // sky flat

            // Normal Doom sky, only one allowed per level
            dcvars.texturemid = skytexturemid;    // Default y-offset

          /* Sky is always drawn full bright, i.e. colormaps[0] is used.
           * Because of this hack, sky is not affected by INVUL inverse mapping.
           * Until Boom fixed this. Compat option added in MBF. */

            if (!(dcvars.colormap = fixedcolormap))
                dcvars.colormap = fullcolormap;          // killough 3/20/98

            // proff 09/21/98: Changed for high-res
            dcvars.iscale = skyiscale;

            const texture_t* tex = R_GetTexture(_g->skytexture);

            // killough 10/98: Use sky scrolling offset
            for (x = pl->minx; (dcvars.x = x) <= pl->maxx; x++)
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
        else
        {     // regular flat

            draw_span_vars_t dsvars;

            dsvars.source = W_GetLumpByNum(_g->firstflat + flattranslation[pl->picnum]);
            dsvars.colormap = R_LoadColorMap(pl->lightlevel);

            planeheight = D_abs(pl->height-viewz);

            const int32_t stop = pl->maxx + 1;

            pl->top[pl->minx-1] = pl->top[stop] = 0xff; // dropoff overflow

            for (x = pl->minx ; x <= stop ; x++)
            {
                R_MakeSpans(x,pl->top[x-1],pl->bottom[x-1], pl->top[x],pl->bottom[x], &dsvars);
            }

            Z_Free(dsvars.source);
        }
    }
}




//*******************************************

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
// killough 5/2/98: reformatted, cleaned up
// CPhipps - moved here from r_main.c

static fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
  int32_t     anglea = ANG90 + (visangle-viewangle);
  int32_t     angleb = ANG90 + (visangle-rw_normalangle);

  int32_t     den = FixedMul(rw_distance, finesine(anglea>>ANGLETOFINESHIFT));

// proff 11/06/98: Changed for high-res
  fixed_t num = FixedMul(projectiony, finesine(angleb>>ANGLETOFINESHIFT));

  return den > num>>16 ? (num = FixedDiv(num, den)) > 64*FRACUNIT ?
    64*FRACUNIT : num < 256 ? 256 : num : 64*FRACUNIT;
}


//
// R_NewVisSprite
//
static vissprite_t *R_NewVisSprite(void)
{
    if (num_vissprite >= MAXVISSPRITES)
    {
#ifdef RANGECHECK
        I_Error("Vissprite overflow.");
#endif
        return NULL;
    }

    return _g->vissprites + num_vissprite++;
}


//
// R_ProjectSprite
// Generates a vissprite for a thing if it might be visible.
//

#define MINZ        (FRACUNIT*4)
#define MAXZ        (FRACUNIT*1280)

static void R_ProjectSprite (mobj_t* thing, int32_t lightlevel)
{
    const fixed_t fx = thing->x;
    const fixed_t fy = thing->y;
    const fixed_t fz = thing->z;

    const fixed_t tr_x = fx - viewx;
    const fixed_t tr_y = fy - viewy;

    const fixed_t tz = FixedMul(tr_x,viewcos)-(-FixedMul(tr_y,viewsin));

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    //Too far away.
    if(tz > MAXZ)
        return;

    fixed_t tx = -(FixedMul(tr_y,viewcos)+(-FixedMul(tr_x,viewsin)));

    // too far off the side?
    if (D_abs(tx)>(tz<<2))
        return;

    // decide which patch to use for sprite relative to player
    const spritedef_t* sprdef = &_g->sprites[thing->sprite];
    const spriteframe_t* sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

    uint32_t rot = 0;

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        angle_t ang = R_PointToAngle(fx, fy);
        rot = (ang-thing->angle+(uint32_t)(ANG45/2)*9)>>29;
    }

    const boolean flip = (boolean)SPR_FLIPPED(sprframe, rot);
    const patch_t* patch = W_GetLumpByNum(sprframe->lump[rot] + _g->firstspritelump);

    /* calculate edges of the shape
     * cph 2003/08/1 - fraggle points out that this offset must be flipped
     * if the sprite is flipped; e.g. FreeDoom imp is messed up by this. */
    if (flip)
        tx -= ((int32_t)(patch->width - patch->leftoffset)) << FRACBITS;
    else
        tx -= ((int32_t)patch->leftoffset) << FRACBITS;

    const fixed_t xscale = FixedDiv(projection, tz);

    fixed_t xl = (centerxfrac + FixedMul(tx,xscale));

    // off the side?
    if(xl > (((int32_t)SCREENWIDTH) << FRACBITS))
        return;

    fixed_t xr = (centerxfrac + FixedMul(tx + (((int32_t)patch->width) << FRACBITS),xscale)) - FRACUNIT;

    // off the side?
    if(xr < 0)
        return;

    //Too small.
    if(xr <= (xl + (FRACUNIT >> 2)))
        return;


    const int32_t x1 = (xl >> FRACBITS);
    const int32_t x2 = (xr >> FRACBITS);

    // store information in a vissprite
    vissprite_t* vis = R_NewVisSprite ();

    //No more vissprites.
    if(!vis)
        return;

    vis->mobjflags       = thing->flags;
    vis->scale           = FixedDiv(projectiony, tz);
    vis->iscale          = tz >> 7;
    vis->patch_num       = sprframe->lump[rot] + _g->firstspritelump;
    vis->patch_topoffset = patch->topoffset;
    vis->gx              = fx;
    vis->gy              = fy;
    vis->gz              = fz;
    vis->texturemid      = (fz + (((int32_t)patch->topoffset) << FRACBITS)) - viewz;
    vis->x1              = x1 < 0 ? 0 : x1;
    vis->x2              = x2 >= SCREENWIDTH ? SCREENWIDTH-1 : x2;


    //const fixed_t iscale = FixedDiv (FRACUNIT, xscale);
    const fixed_t iscale = FixedReciprocal(xscale);

    if (flip)
    {
        vis->startfrac = (((int32_t)patch->width)<<FRACBITS)-1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }

    Z_Free(patch);

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale*(vis->x1-x1);

    // get light level
    if (thing->flags & MF_SHADOW)
        vis->colormap = NULL;             // shadow draw
    else if (fixedcolormap)
        vis->colormap = fixedcolormap;      // fixed map
    else if (thing->frame & FF_FULLBRIGHT)
        vis->colormap = fullcolormap;     // full bright  // killough 3/20/98
    else
    {      // diminished light
        vis->colormap = R_ColourMap(lightlevel);
    }
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
// killough 9/18/98: add lightlevel as parameter, fixing underwater lighting
static void R_AddSprites(subsector_t* subsec, int32_t lightlevel)
{
  sector_t* sec=subsec->sector;
  mobj_t *thing;

  // BSP is traversed by subsector.
  // A sector might have been split into several
  //  subsectors during BSP building.
  // Thus we check whether its already added.

  if (sec->validcount == _g->validcount)
    return;

  // Well, now it will be done.
  sec->validcount = _g->validcount;

  // Handle all things in sector.

  for (thing = sec->thinglist; thing; thing = thing->snext)
    R_ProjectSprite(thing, lightlevel);
}

//
// R_FindPlane
//
// killough 2/28/98: Add offsets


// New function, by Lee Killough

static visplane_t *new_visplane(uint32_t hash)
{
    visplane_t *check = _g->freetail;

    if (!check)
        check = Z_CallocLevel(sizeof(visplane_t));
    else
    {
        if (!(_g->freetail = _g->freetail->next))
            _g->freehead = &_g->freetail;
    }

    check->next = _g->visplanes[hash];
    _g->visplanes[hash] = check;

    return check;
}


// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum,lightlevel,height) \
  ((uint32_t)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))


static visplane_t *R_FindPlane(fixed_t height, int16_t picnum, int32_t lightlevel)
{
    visplane_t *check;
    uint32_t hash;                      // killough

    if (picnum == _g->skyflatnum)
        height = lightlevel = 0;         // killough 7/19/98: most skies map together

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum,lightlevel,height);

    for (check=_g->visplanes[hash]; check; check=check->next)  // killough
        if (height == check->height &&
                picnum == check->picnum &&
                lightlevel == check->lightlevel)
            return check;

    check = new_visplane(hash);         // killough

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = SCREENWIDTH; // Was SCREENWIDTH -- killough 11/98
    check->maxx = -1;

    memset(check->top, UINT32_MAX, sizeof(check->top));

    check->modified = false;

    return check;
}

/*
 * R_DupPlane
 *
 * cph 2003/04/18 - create duplicate of existing visplane and set initial range
 */
static visplane_t *R_DupPlane(const visplane_t *pl, int32_t start, int32_t stop)
{
    uint32_t hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
    visplane_t *new_pl = new_visplane(hash);

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->minx = start;
    new_pl->maxx = stop;

    memset(new_pl->top, UINT32_MAX, sizeof(new_pl->top));

    new_pl->modified = false;

    return new_pl;
}


//
// R_CheckPlane
//
static visplane_t *R_CheckPlane(visplane_t *pl, int32_t start, int32_t stop)
{
    int32_t intrl, intrh, unionl, unionh, x;

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


inline static void ByteCopy(byte* dest, const byte* src, uint32_t count)
{
    do
    {
        *dest++ = *src++;
    } while(--count);
}


static void R_DrawColumnInCache(const column_t* patch, byte* cache, int16_t originy, int16_t cacheheight)
{
    while (patch->topdelta != 0xff)
    {
        const byte* source = (const byte *)patch + 3;
        int16_t count = patch->length;
        int16_t position = originy + patch->topdelta;

        if (position < 0)
        {
            count += position;
            position = 0;
        }

        if (position + count > cacheheight)
            count = cacheheight - position;

        if (count > 0)
            ByteCopy(cache + position, source, count);

        patch = (const column_t *)(  (const byte *)patch + patch->length + 4);
    }
}

/*
 * Draw a column of pixels of the specified texture.
 * If the texture is simple (1 patch, full height) then just draw
 * straight from const patch_t*.
*/

#define CACHE_WAYS 4

#define CACHE_MASK (CACHE_WAYS-1)
#define CACHE_STRIDE (128 / CACHE_WAYS)
#define CACHE_KEY_MASK (CACHE_STRIDE-1)

#define CACHE_ENTRY(c, t) ((c << 16 | t))

#define CACHE_HASH(c, t) (((c >> 1) ^ t) & CACHE_KEY_MASK)

static uint32_t FindColumnCacheItem(int16_t texture, uint32_t column)
{
    uint32_t cx = CACHE_ENTRY(column, texture);

    uint32_t key = CACHE_HASH(column, texture);

    uint32_t* cc = (uint32_t*)&columnCacheEntries[key];

    uint32_t i = key;

    do
    {
        uint32_t cy = *cc;

        if((cy == cx) || (cy == 0))
            return i;

        cc+=CACHE_STRIDE;
        i+=CACHE_STRIDE;

    } while(i < 128);


    //No space. Random eviction.
    return ((M_Random() & CACHE_MASK) * CACHE_STRIDE) + key;
}


static const byte* R_ComposeColumn(const int16_t texture, const texture_t* tex, int32_t texcolumn, uint32_t iscale)
{
    //static int32_t total, misses;
    int32_t colmask;

    if(!highDetail)
    {
        colmask = 0xfffe;

        if(tex->width > 8)
        {
            if(iscale > (((int32_t)4) << FRACBITS))
                colmask = 0xfff0;
            else if(iscale > (((int32_t)3) << FRACBITS))
                colmask = 0xfff8;
            else if (iscale > (((int32_t)2) << FRACBITS))
                colmask = 0xfffc;
        }
    }
    else
        colmask = 0xffff;


    const int32_t xc = (texcolumn & colmask) & tex->widthmask;

    uint32_t cachekey = FindColumnCacheItem(texture, xc);

    byte* colcache = &columnCache[cachekey*128];
    uint32_t cacheEntry = columnCacheEntries[cachekey];

    //total++;

    if(cacheEntry != CACHE_ENTRY(xc, texture))
    {
        //misses++;
        byte tmpCache[128];


        columnCacheEntries[cachekey] = CACHE_ENTRY(xc, texture);

        uint8_t i = 0;
        uint8_t patchcount = tex->patchcount;

        do
        {
            const texpatch_t* patch = &tex->patches[i];

            const patch_t* realpatch = W_GetLumpByNum(patch->patch_num);

            const int16_t x1 = patch->originx;

            if (xc < x1)
                continue;

            const int16_t x2 = x1 + realpatch->width;

            if (xc < x2)
            {
                const column_t* patchcol = (const column_t *)((const byte *)realpatch + realpatch->columnofs[xc - x1]);

                R_DrawColumnInCache (patchcol, tmpCache, patch->originy, tex->height);
            }
            Z_Free(realpatch);
        } while(++i < patchcount);

        //Block copy will drop low 2 bits of len.
        memcpy(colcache, tmpCache, (tex->height + 3) & ~3);
    }

    return colcache;
}

static void R_DrawSegTextureColumn(int16_t texture, int32_t texcolumn, draw_column_vars_t* dcvars)
{
    const texture_t* tex = R_GetTexture(texture);

    if (!tex->overlapped)
    {
        uint32_t r = R_GetColumn(tex, texcolumn);
        const patch_t* patch = W_GetLumpByNum(HIWORD(r));
        texcolumn = LOWORD(r);
        const column_t* column = (const column_t *) ((const byte *)patch + patch->columnofs[texcolumn]);

        dcvars->source = (const byte*)column + 3;
        R_DrawColumn (dcvars);
        Z_Free(patch);
    }
    else
    {
        dcvars->source = R_ComposeColumn(texture, tex, texcolumn, dcvars->iscale);
        R_DrawColumn (dcvars);
    }
}

//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling textures.
// CALLED: CORE LOOPING ROUTINE.
//

#define HEIGHTBITS 12
#define HEIGHTUNIT (1<<HEIGHTBITS)

static void R_RenderSegLoop (int32_t rw_x)
{
    draw_column_vars_t dcvars;
    fixed_t  texturecolumn = 0;   // shut up compiler warning

    R_SetDefaultDrawColumnVars(&dcvars);

    dcvars.colormap = R_LoadColorMap(rw_lightlevel);

    for ( ; rw_x < rw_stopx ; rw_x++)
    {
        // mark floor / ceiling areas

        int32_t yh = bottomfrac>>HEIGHTBITS;
        int32_t yl = (topfrac+HEIGHTUNIT-1)>>HEIGHTBITS;

        int32_t cc_rwx = ceilingclip[rw_x];
        int32_t fc_rwx = floorclip[rw_x];

        // no space above wall?
        int32_t bottom,top = cc_rwx+1;

        if (yl < top)
            yl = top;

        if (markceiling)
        {
            bottom = yl-1;

            if (bottom >= fc_rwx)
                bottom = fc_rwx-1;

            if (top <= bottom)
            {
                ceilingplane->top[rw_x] = top;
                ceilingplane->bottom[rw_x] = bottom;
                ceilingplane->modified = true;
            }
            // SoM: this should be set here
            cc_rwx = bottom;
        }

        bottom = fc_rwx-1;
        if (yh > bottom)
            yh = bottom;

        if (markfloor)
        {

            top  = yh < cc_rwx ? cc_rwx : yh;

            if (++top <= bottom)
            {
                floorplane->top[rw_x] = top;
                floorplane->bottom[rw_x] = bottom;
                floorplane->modified = true;
            }
            // SoM: This should be set here to prevent overdraw
            fc_rwx = top;
        }

        // texturecolumn and lighting are independent of wall tiers
        if (segtextured)
        {
            // calculate texture offset
            angle_t angle =(rw_centerangle+xtoviewangle(rw_x))>>ANGLETOFINESHIFT;

            texturecolumn = rw_offset-FixedMul(finetangent(angle),rw_distance);

            texturecolumn >>= FRACBITS;

            dcvars.x = rw_x;

            dcvars.iscale = FixedReciprocal((uint32_t)rw_scale);
        }

        // draw the wall tiers
        if (midtexture)
        {

            dcvars.yl = yl;     // single sided line
            dcvars.yh = yh;
            dcvars.texturemid = rw_midtexturemid;
            //

            R_DrawSegTextureColumn(midtexture, texturecolumn, &dcvars);

            cc_rwx = viewheight;
            fc_rwx = -1;
        }
        else
        {

            // two sided line
            if (toptexture)
            {
                // top wall
                int32_t mid = pixhigh>>HEIGHTBITS;
                pixhigh += pixhighstep;

                if (mid >= fc_rwx)
                    mid = fc_rwx-1;

                if (mid >= yl)
                {
                    dcvars.yl = yl;
                    dcvars.yh = mid;
                    dcvars.texturemid = rw_toptexturemid;

                    R_DrawSegTextureColumn(toptexture, texturecolumn, &dcvars);

                    cc_rwx = mid;
                }
                else
                    cc_rwx = yl-1;
            }
            else  // no top wall
            {

                if (markceiling)
                    cc_rwx = yl-1;
            }

            if (bottomtexture)          // bottom wall
            {
                int32_t mid = (pixlow+HEIGHTUNIT-1)>>HEIGHTBITS;
                pixlow += pixlowstep;

                // no space above wall?
                if (mid <= cc_rwx)
                    mid = cc_rwx+1;

                if (mid <= yh)
                {
                    dcvars.yl = mid;
                    dcvars.yh = yh;
                    dcvars.texturemid = rw_bottomtexturemid;

                    R_DrawSegTextureColumn(bottomtexture, texturecolumn, &dcvars);

                    fc_rwx = mid;
                }
                else
                    fc_rwx = yh+1;
            }
            else        // no bottom wall
            {
                if (markfloor)
                    fc_rwx = yh+1;
            }

            // cph - if we completely blocked further sight through this column,
            // add this info to the solid columns array for r_bsp.c
            if ((markceiling || markfloor) && (fc_rwx <= cc_rwx + 1))
            {
                solidcol[rw_x] = 1;
                didsolidcol = 1;
            }

            // save texturecol for backdrawing of masked mid texture
            if (maskedtexture)
                maskedtexturecol[rw_x] = texturecolumn;
        }

        rw_scale += rw_scalestep;
        topfrac += topstep;
        bottomfrac += bottomstep;

        floorclip[rw_x] = fc_rwx;
        ceilingclip[rw_x] = cc_rwx;
    }
}

static boolean R_CheckOpenings(const int32_t start)
{
    int32_t pos = _g->lastopening - _g->openings;
    int32_t need = (rw_stopx - start)*4 + pos;

#ifdef RANGECHECK
    if(need > MAXOPENINGS)
        I_Error("Openings overflow. Need = %ld", need);
#endif

    return need <= MAXOPENINGS;
}


/* CPhipps -
 * FixedMod - returns a % b, guaranteeing 0<=a<b
 * (notice that the C standard for % does not guarantee this)
 */

inline static fixed_t CONSTFUNC FixedMod(fixed_t a, fixed_t b)
{
    if(!a)
        return 0;

    if (b & (b-1))
    {
        fixed_t r = a % b;
        return ((r<0) ? r+b : r);
    }
    else
        return (a & (b-1));
}


inline static CONSTFUNC int32_t IDiv32 (int32_t a, int32_t b)
{
    return a / b;
}


//
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//
static void R_StoreWallRange(const int8_t start, const int8_t stop)
{
    fixed_t hyp;
    angle_t offsetangle;

    // don't overflow and crash
    if (ds_p == &_g->drawsegs[MAXDRAWSEGS])
    {
#ifdef RANGECHECK
        I_Error("Drawsegs overflow.");
#endif
        return;
    }


    linedata_t* linedata = &_g->linedata[curline->linenum];

    // mark the segment as visible for auto map
    linedata->r_flags |= ML_MAPPED;

    sidedef = &_g->sides[curline->sidenum];
    linedef = &_g->lines[curline->linenum];

    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;

    offsetangle = rw_normalangle-rw_angle1;

    if (D_abs(offsetangle) > ANG90)
        offsetangle = ANG90;

    hyp = (viewx==curline->v1.x && viewy==curline->v1.y)?
                0 : R_PointToDist (curline->v1.x, curline->v1.y);

    rw_distance = FixedMul(hyp, finecosine(offsetangle>>ANGLETOFINESHIFT));

    int32_t rw_x = ds_p->x1 = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop+1;

    //Openings overflow. Nevermind.
    if(!R_CheckOpenings(start))
        return;

    // calculate scale at both ends and step
    ds_p->scale1 = rw_scale = R_ScaleFromGlobalAngle (viewangle + xtoviewangle(start));

    if (stop > start)
    {
        ds_p->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle(stop));
        ds_p->scalestep = rw_scalestep = IDiv32(ds_p->scale2-rw_scale, stop-start);
    }
    else
        ds_p->scale2 = ds_p->scale1;

    // calculate texture boundaries
    //  and decide if floor / ceiling marks are needed

    worldtop = frontsector->ceilingheight - viewz;
    worldbottom = frontsector->floorheight - viewz;

    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;

    if (!backsector)
    {
        // single sided line
        midtexture = texturetranslation[sidedef->midtexture];

        // a single sided line is terminal, so it must mark ends
        markfloor = markceiling = true;

        if (linedef->flags & ML_DONTPEGBOTTOM)
        {         // bottom of texture at bottom
            fixed_t vtop = frontsector->floorheight + textureheight[sidedef->midtexture];
            rw_midtexturemid = vtop - viewz;
        }
        else        // top of texture at top
            rw_midtexturemid = worldtop;

        rw_midtexturemid += FixedMod( (((int32_t)sidedef->rowoffset) << FRACBITS), textureheight[midtexture]);

        ds_p->silhouette = SIL_BOTH;
        ds_p->sprtopclip = screenheightarray;
        ds_p->sprbottomclip = negonearray;
        ds_p->bsilheight = INT32_MAX;
        ds_p->tsilheight = INT32_MIN;
    }
    else      // two sided line
    {
        ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
        ds_p->silhouette = 0;

        if(linedata->r_flags & RF_CLOSED)
        { /* cph - closed 2S line e.g. door */
            // cph - killough's (outdated) comment follows - this deals with both
            // "automap fixes", his and mine
            // killough 1/17/98: this test is required if the fix
            // for the automap bug (r_bsp.c) is used, or else some
            // sprites will be displayed behind closed doors. That
            // fix prevents lines behind closed doors with dropoffs
            // from being displayed on the automap.

            ds_p->silhouette = SIL_BOTH;
            ds_p->sprbottomclip = negonearray;
            ds_p->bsilheight = INT32_MAX;
            ds_p->sprtopclip = screenheightarray;
            ds_p->tsilheight = INT32_MIN;

        }
        else
        { /* not solid - old code */

            if (frontsector->floorheight > backsector->floorheight)
            {
                ds_p->silhouette = SIL_BOTTOM;
                ds_p->bsilheight = frontsector->floorheight;
            }
            else
                if (backsector->floorheight > viewz)
                {
                    ds_p->silhouette = SIL_BOTTOM;
                    ds_p->bsilheight = INT32_MAX;
                }

            if (frontsector->ceilingheight < backsector->ceilingheight)
            {
                ds_p->silhouette |= SIL_TOP;
                ds_p->tsilheight = frontsector->ceilingheight;
            }
            else
                if (backsector->ceilingheight < viewz)
                {
                    ds_p->silhouette |= SIL_TOP;
                    ds_p->tsilheight = INT32_MIN;
                }
        }

        worldhigh = backsector->ceilingheight - viewz;
        worldlow = backsector->floorheight - viewz;

        // hack to allow height changes in outdoor areas
        if (frontsector->ceilingpic == _g->skyflatnum && backsector->ceilingpic == _g->skyflatnum)
            worldtop = worldhigh;

        markfloor = worldlow != worldbottom
                || backsector->floorpic != frontsector->floorpic
                || backsector->lightlevel != frontsector->lightlevel
                ;

        markceiling = worldhigh != worldtop
                || backsector->ceilingpic != frontsector->ceilingpic
                || backsector->lightlevel != frontsector->lightlevel
                ;

        if (backsector->ceilingheight <= frontsector->floorheight || backsector->floorheight >= frontsector->ceilingheight)
            markceiling = markfloor = true;   // closed door

        if (worldhigh < worldtop)   // top texture
        {
            toptexture = texturetranslation[sidedef->toptexture];
            rw_toptexturemid = linedef->flags & ML_DONTPEGTOP ? worldtop :
                                                                        backsector->ceilingheight+textureheight[sidedef->toptexture]-viewz;
            rw_toptexturemid += FixedMod( (((int32_t)sidedef->rowoffset) << FRACBITS), textureheight[toptexture]);
        }

        if (worldlow > worldbottom) // bottom texture
        {
            bottomtexture = texturetranslation[sidedef->bottomtexture];
            rw_bottomtexturemid = linedef->flags & ML_DONTPEGBOTTOM ? worldtop : worldlow;

            rw_bottomtexturemid += FixedMod( (((int32_t)sidedef->rowoffset) << FRACBITS), textureheight[bottomtexture]);
        }

        // allocate space for masked texture tables
        if (sidedef->midtexture)    // masked midtexture
        {
            maskedtexture = true;
            ds_p->maskedtexturecol = maskedtexturecol = _g->lastopening - rw_x;
            _g->lastopening += rw_stopx - rw_x;
        }
    }

    // calculate rw_offset (only needed for textured lines)
    segtextured = ((midtexture | toptexture | bottomtexture | maskedtexture) > 0);

    if (segtextured)
    {
        rw_offset = FixedMul (hyp, -finesine(offsetangle >>ANGLETOFINESHIFT));

        rw_offset += (((int32_t)sidedef->textureoffset) << FRACBITS) + curline->offset;

        rw_centerangle = ANG90 + viewangle - rw_normalangle;

        rw_lightlevel = frontsector->lightlevel;
    }

    // if a floor / ceiling plane is on the wrong side of the view
    // plane, it is definitely invisible and doesn't need to be marked.
    if (frontsector->floorheight >= viewz)       // above view plane
        markfloor = false;
    if (frontsector->ceilingheight <= viewz &&
            frontsector->ceilingpic != _g->skyflatnum)   // below view plane
        markceiling = false;

    // calculate incremental stepping values for texture edges
    worldtop >>= 4;
    worldbottom >>= 4;

    topstep = -FixedMul (rw_scalestep, worldtop);
    topfrac = (centeryfrac>>4) - FixedMul (worldtop, rw_scale);

    bottomstep = -FixedMul (rw_scalestep,worldbottom);
    bottomfrac = (centeryfrac>>4) - FixedMul (worldbottom, rw_scale);

    if (backsector)
    {
        worldhigh >>= 4;
        worldlow >>= 4;

        if (worldhigh < worldtop)
        {
            pixhigh = (centeryfrac>>4) - FixedMul (worldhigh, rw_scale);
            pixhighstep = -FixedMul (rw_scalestep,worldhigh);
        }
        if (worldlow > worldbottom)
        {
            pixlow = (centeryfrac>>4) - FixedMul (worldlow, rw_scale);
            pixlowstep = -FixedMul (rw_scalestep,worldlow);
        }
    }

    // render it
    if (markceiling)
    {
        if (ceilingplane)   // killough 4/11/98: add NULL ptr checks
            ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
        else
            markceiling = 0;
    }

    if (markfloor)
    {
        if (floorplane)     // killough 4/11/98: add NULL ptr checks
            /* cph 2003/04/18  - ceilingplane and floorplane might be the same
       * visplane (e.g. if both skies); R_CheckPlane doesn't know about
       * modifications to the plane that might happen in parallel with the check
       * being made, so we have to override it and split them anyway if that is
       * a possibility, otherwise the floor marking would overwrite the ceiling
       * marking, resulting in HOM. */
            if (markceiling && ceilingplane == floorplane)
                floorplane = R_DupPlane (floorplane, rw_x, rw_stopx-1);
            else
                floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);
        else
            markfloor = 0;
    }

    didsolidcol = 0;
    R_RenderSegLoop(rw_x);

    /* cph - if a column was made solid by this wall, we _must_ save full clipping info */
    if (backsector && didsolidcol)
    {
        if (!(ds_p->silhouette & SIL_BOTTOM))
        {
            ds_p->silhouette |= SIL_BOTTOM;
            ds_p->bsilheight = backsector->floorheight;
        }
        if (!(ds_p->silhouette & SIL_TOP))
        {
            ds_p->silhouette |= SIL_TOP;
            ds_p->tsilheight = backsector->ceilingheight;
        }
    }

    // save sprite clipping info
    if ((ds_p->silhouette & SIL_TOP || maskedtexture) && !ds_p->sprtopclip)
    {
        ByteCopy((byte*)_g->lastopening, (const byte*)(ceilingclip+start), sizeof(int16_t)*(rw_stopx-start));
        ds_p->sprtopclip = _g->lastopening - start;
        _g->lastopening += rw_stopx - start;
    }

    if ((ds_p->silhouette & SIL_BOTTOM || maskedtexture) && !ds_p->sprbottomclip)
    {
        ByteCopy((byte*)_g->lastopening, (const byte*)(floorclip+start), sizeof(int16_t)*(rw_stopx-start));
        ds_p->sprbottomclip = _g->lastopening - start;
        _g->lastopening += rw_stopx - start;
    }

    if (maskedtexture && !(ds_p->silhouette & SIL_TOP))
    {
        ds_p->silhouette |= SIL_TOP;
        ds_p->tsilheight = INT32_MIN;
    }

    if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM))
    {
        ds_p->silhouette |= SIL_BOTTOM;
        ds_p->bsilheight = INT32_MAX;
    }

    ds_p++;
}


// killough 1/18/98 -- This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// cph - converted to R_RecalcLineFlags. This recalculates all the flags for
// a line, including closure and texture tiling.

static void R_RecalcLineFlags(void)
{
    linedata_t* linedata = &_g->linedata[linedef->lineno];

    const side_t* side = &_g->sides[curline->sidenum];

    linedata->r_validcount = (_g->gametic & 0xffff);

    /* First decide if the line is closed, normal, or invisible */
    if (!(linedef->flags & ML_TWOSIDED)
            || backsector->ceilingheight <= frontsector->floorheight
            || backsector->floorheight >= frontsector->ceilingheight
            || (
                // if door is closed because back is shut:
                backsector->ceilingheight <= backsector->floorheight

                // preserve a kind of transparent door/lift special effect:
                && (backsector->ceilingheight >= frontsector->ceilingheight ||
                    side->toptexture)

                && (backsector->floorheight <= frontsector->floorheight ||
                    side->bottomtexture)

                // properly render skies (consider door "open" if both ceilings are sky):
                && (backsector->ceilingpic !=_g->skyflatnum ||
                    frontsector->ceilingpic!=_g->skyflatnum)
                )
            )
        linedata->r_flags = (RF_CLOSED | (linedata->r_flags & ML_MAPPED));
    else
    {
        // Reject empty lines used for triggers
        //  and special events.
        // Identical floor and ceiling on both sides,
        // identical light levels on both sides,
        // and no middle texture.
        // CPhipps - recode for speed, not certain if this is portable though
        if (backsector->ceilingheight != frontsector->ceilingheight
                || backsector->floorheight != frontsector->floorheight
                || side->midtexture
                || backsector->ceilingpic != frontsector->ceilingpic
                || backsector->floorpic != frontsector->floorpic
                || backsector->lightlevel != frontsector->lightlevel)
        {
            linedata->r_flags = (linedata->r_flags & ML_MAPPED); return;
        } else
            linedata->r_flags = (RF_IGNORE | (linedata->r_flags & ML_MAPPED));
    }
}


inline static void ByteSet(byte* dest, byte val, uint32_t count)
{
    do
    {
        *dest++ = val;
    } while(--count);
}


inline static void* ByteFind(byte* mem, byte val, uint32_t count)
{
    do
    {
        if(*mem == val)
            return mem;

        mem++;
    } while(--count);

    return NULL;
}


// CPhipps -
// R_ClipWallSegment
//
// Replaces the old R_Clip*WallSegment functions. It draws bits of walls in those
// columns which aren't solid, and updates the solidcol[] array appropriately

static void R_ClipWallSegment(int8_t first, int8_t last, boolean solid)
{
    byte *p;
    while (first < last)
    {
        if (solidcol[first])
        {
            if (!(p = ByteFind(solidcol+first, 0, last-first)))
                return; // All solid

            first = p - solidcol;
        }
        else
        {
            int8_t to;
            if (!(p = ByteFind(solidcol+first, 1, last-first)))
                to = last;
            else
                to = p - solidcol;

            R_StoreWallRange(first, to-1);

            if (solid)
            {
                //memset(solidcol+first,1,to-first);
                ByteSet(solidcol+first, 1, to-first);
            }

            first = to;
        }
    }
}

//
// R_ClearClipSegs
//

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//

static void R_AddLine (const seg_t *line)
{
    int8_t      x1;
    int8_t      x2;
    angle_t  angle1;
    angle_t  angle2;
    angle_t  span;
    angle_t  tspan;

    curline = line;

    angle1 = R_PointToAngle (line->v1.x, line->v1.y);
    angle2 = R_PointToAngle (line->v2.x, line->v2.y);

    // Clip to view edges.
    span = angle1 - angle2;

    // Back side, i.e. backface culling
    if (span >= ANG180)
        return;

    // Global angle needed by segcalc.
    rw_angle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;

    tspan = angle1 + clipangle;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;

        angle1 = clipangle;
    }

    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;
        angle2 = 0-clipangle;
    }

    // The seg is in the view range,
    // but not necessarily visible.

    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;

    // killough 1/31/98: Here is where "slime trails" can SOMETIMES occur:
    x1 = viewangletox(angle1);
    x2 = viewangletox(angle2);

    // Does not cross a pixel?
    if (x1 >= x2)       // killough 1/31/98 -- change == to >= for robustness
        return;

    backsector = SG_BACKSECTOR(line);

    /* cph - roll up linedef properties in flags */
    linedef = &_g->lines[curline->linenum];
    linedata_t* linedata = &_g->linedata[linedef->lineno];

    if (linedata->r_validcount != (_g->gametic & 0xffff))
        R_RecalcLineFlags();

    if (linedata->r_flags & RF_IGNORE)
    {
        return;
    }
    else
        R_ClipWallSegment (x1, x2, linedata->r_flags & RF_CLOSED);
}

//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
// killough 1/31/98 -- made static, polished

static void R_Subsector(int32_t num)
{
    int32_t         count;
    const seg_t       *line;
    subsector_t *sub;

    sub = &_g->subsectors[num];
    frontsector = sub->sector;
    count = sub->numlines;
    line = &_g->segs[sub->firstline];

    if(frontsector->floorheight < viewz)
    {
        floorplane = R_FindPlane(frontsector->floorheight,
                                     frontsector->floorpic,
                                     frontsector->lightlevel                // killough 3/16/98
                                     );
    }
    else
    {
        floorplane = NULL;
    }


    if(frontsector->ceilingheight > viewz || (frontsector->ceilingpic == _g->skyflatnum))
    {
        ceilingplane = R_FindPlane(frontsector->ceilingheight,     // killough 3/8/98
                                       frontsector->ceilingpic,
                                       frontsector->lightlevel
                                       );
    }
    else
    {
        ceilingplane = NULL;
    }

    R_AddSprites(sub, frontsector->lightlevel);
    while (count--)
    {
        R_AddLine (line);
        line++;
        curline = NULL; /* cph 2001/11/18 - must clear curline now we're done with it, so R_ColourMap doesn't try using it for other things */
    }
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//

static const byte checkcoord[12][4] = // killough -- static const
{
  {3,0,2,1},
  {3,0,2,0},
  {3,1,2,0},
  {0},
  {2,0,2,1},
  {0,0,0,0},
  {3,1,3,0},
  {0},
  {2,0,3,1},
  {2,1,3,1},
  {2,1,3,0}
};

// killough 1/28/98: static // CPhipps - const parameter, reformatted
static boolean R_CheckBBox(const int16_t *bspcoord)
{
    angle_t angle1, angle2;

    {
        int32_t        boxpos;
        const byte* check;

        // Find the corners of the box
        // that define the edges from current viewpoint.
        boxpos = (viewx <= ((fixed_t)bspcoord[BOXLEFT]<<FRACBITS) ? 0 : viewx < ((fixed_t)bspcoord[BOXRIGHT]<<FRACBITS) ? 1 : 2) +
                (viewy >= ((fixed_t)bspcoord[BOXTOP]<<FRACBITS) ? 0 : viewy > ((fixed_t)bspcoord[BOXBOTTOM]<<FRACBITS) ? 4 : 8);

        if (boxpos == 5)
            return true;

        check = checkcoord[boxpos];
        angle1 = R_PointToAngle (((fixed_t)bspcoord[check[0]]<<FRACBITS), ((fixed_t)bspcoord[check[1]]<<FRACBITS)) - viewangle;
        angle2 = R_PointToAngle (((fixed_t)bspcoord[check[2]]<<FRACBITS), ((fixed_t)bspcoord[check[3]]<<FRACBITS)) - viewangle;
    }

    // cph - replaced old code, which was unclear and badly commented
    // Much more efficient code now
    if ((int32_t)angle1 < (int32_t)angle2)
    { /* it's "behind" us */
        /* Either angle1 or angle2 is behind us, so it doesn't matter if we
     * change it to the corect sign
     */
        if ((angle1 >= ANG180) && (angle1 < ANG270))
            angle1 = INT32_MAX; /* which is ANG180-1 */
        else
            angle2 = INT32_MIN;
    }

    if ((int32_t)angle2 >=  (int32_t)clipangle) return false; // Both off left edge
    if ((int32_t)angle1 <= -(int32_t)clipangle) return false; // Both off right edge
    if ((int32_t)angle1 >=  (int32_t)clipangle) angle1 = clipangle; // Clip at left edge
    if ((int32_t)angle2 <= -(int32_t)clipangle) angle2 = 0-clipangle; // Clip at right edge

    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    {
        int8_t sx1 = viewangletox(angle1);
        int8_t sx2 = viewangletox(angle2);
        //    const cliprange_t *start;

        // Does not cross a pixel.
        if (sx1 == sx2)
            return false;

        if (!ByteFind(solidcol+sx1, 0, sx2-sx1)) return false;
        // All columns it covers are already solidly covered
    }

    return true;
}

//Render a BSP subsector if bspnum is a leaf node.
//Return false if bspnum is frame node.





static boolean R_RenderBspSubsector(int32_t bspnum)
{
    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
            R_Subsector (0);
        else
            R_Subsector (bspnum & (~NF_SUBSECTOR));

        return true;
    }

    return false;
}

// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.

//Non recursive version.
//constant stack space used and easier to
//performance profile.
#define MAX_BSP_DEPTH 128

static void R_RenderBSPNode(int32_t bspnum)
{
    int32_t stack[MAX_BSP_DEPTH];
    int32_t sp = 0;

    const mapnode_t* bsp;
    int32_t side = 0;

    while(true)
    {
        //Front sides.
        while (!R_RenderBspSubsector(bspnum))
        {
            if(sp == MAX_BSP_DEPTH)
                break;

            bsp = &nodes[bspnum];
            side = R_PointOnSide (viewx, viewy, bsp);

            stack[sp++] = bspnum;
            stack[sp++] = side;

            bspnum = bsp->children[side];
        }

        if(sp == 0)
        {
            //back at root node and not visible. All done!
            return;
        }

        //Back sides.
        side = stack[--sp];
        bspnum = stack[--sp];
        bsp = &nodes[bspnum];

        // Possibly divide back space.
        //Walk back up the tree until we find
        //a node that has a visible backspace.
        while(!R_CheckBBox (bsp->bbox[side^1]))
        {
            if(sp == 0)
            {
                //back at root node and not visible. All done!
                return;
            }

            //Back side next.
            side = stack[--sp];
            bspnum = stack[--sp];

            bsp = &nodes[bspnum];
        }

        bspnum = bsp->children[side^1];
    }
}


static void R_ClearDrawSegs(void)
{
    ds_p = _g->drawsegs;
}

static void R_ClearClipSegs (void)
{
    memset(solidcol, 0, SCREENWIDTH);
}

//
// R_ClearSprites
// Called at frame start.
//

static void R_ClearSprites(void)
{
    num_vissprite = 0;            // killough
}

//
// RDrawPlanes
// At the end of each frame.
//

static void R_DrawPlanes (void)
{
    for (int32_t i=0; i<MAXVISPLANES; i++)
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

static const fixed_t iprojection = 1092; //( (1 << FRACUNIT) / (SCREENWIDTH / 2))

static void R_ClearPlanes(void)
{
    int8_t i;

    // opening / clipping determination
    for (i=0 ; i<SCREENWIDTH ; i++)
        floorclip[i] = viewheight, ceilingclip[i] = -1;


    for (i=0;i<MAXVISPLANES;i++)    // new code -- killough
        for (*_g->freehead = _g->visplanes[i], _g->visplanes[i] = NULL; *_g->freehead; )
            _g->freehead = &(*_g->freehead)->next;

    _g->lastopening = _g->openings;

    basexscale = FixedMul(viewsin,iprojection);
    baseyscale = FixedMul(viewcos,iprojection);
}

//
// R_RenderView
//
void R_RenderPlayerView (player_t* player)
{
    R_SetupFrame (player);

    // Clear buffers.
    R_ClearClipSegs ();
    R_ClearDrawSegs ();
    R_ClearPlanes ();
    R_ClearSprites ();

    // The head node is the last node output.
    R_RenderBSPNode (numnodes-1);

    R_DrawPlanes ();

    R_DrawMasked ();
}


#if 0
static const int8_t viewangletoxTable[4096] =
{
    120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,
    120,120,120,120,120,120,120,120,120,119,119,119,119,119,119,119,
    119,119,119,119,119,118,118,118,118,118,118,118,118,118,118,118,
    117,117,117,117,117,117,117,117,117,117,117,116,116,116,116,116,
    116,116,116,116,116,116,116,115,115,115,115,115,115,115,115,115,
    115,115,115,114,114,114,114,114,114,114,114,114,114,114,114,113,
    113,113,113,113,113,113,113,113,113,113,113,112,112,112,112,112,
    112,112,112,112,112,112,112,112,111,111,111,111,111,111,111,111,
    111,111,111,111,111,110,110,110,110,110,110,110,110,110,110,110,
    110,110,109,109,109,109,109,109,109,109,109,109,109,109,109,108,
    108,108,108,108,108,108,108,108,108,108,108,108,107,107,107,107,
    107,107,107,107,107,107,107,107,107,107,106,106,106,106,106,106,
    106,106,106,106,106,106,106,106,105,105,105,105,105,105,105,105,
    105,105,105,105,105,105,104,104,104,104,104,104,104,104,104,104,
    104,104,104,104,103,103,103,103,103,103,103,103,103,103,103,103,
    103,103,102,102,102,102,102,102,102,102,102,102,102,102,102,102,
    102,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,99,
    99,99,99,99,99,99,99,99,99,99,99,99,99,99,98,98,
    98,98,98,98,98,98,98,98,98,98,98,98,98,98,97,97,
    97,97,97,97,97,97,97,97,97,97,97,97,97,97,96,96,
    96,96,96,96,96,96,96,96,96,96,96,96,96,96,95,95,
    95,95,95,95,95,95,95,95,95,95,95,95,95,95,94,94,
    94,94,94,94,94,94,94,94,94,94,94,94,94,94,94,93,
    93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
    92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,
    92,91,91,91,91,91,91,91,91,91,91,91,91,91,91,91,
    91,91,90,90,90,90,90,90,90,90,90,90,90,90,90,90,
    90,90,90,90,89,89,89,89,89,89,89,89,89,89,89,89,
    89,89,89,89,89,88,88,88,88,88,88,88,88,88,88,88,
    88,88,88,88,88,88,88,87,87,87,87,87,87,87,87,87,
    87,87,87,87,87,87,87,87,87,86,86,86,86,86,86,86,
    86,86,86,86,86,86,86,86,86,86,86,86,85,85,85,85,
    85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,84,
    84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,
    84,83,83,83,83,83,83,83,83,83,83,83,83,83,83,83,
    83,83,83,83,82,82,82,82,82,82,82,82,82,82,82,82,
    82,82,82,82,82,82,82,82,81,81,81,81,81,81,81,81,
    81,81,81,81,81,81,81,81,81,81,81,80,80,80,80,80,
    80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,79,
    79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,79,
    79,79,79,78,78,78,78,78,78,78,78,78,78,78,78,78,
    78,78,78,78,78,78,78,77,77,77,77,77,77,77,77,77,
    77,77,77,77,77,77,77,77,77,77,77,76,76,76,76,76,
    76,76,76,76,76,76,76,76,76,76,76,76,76,76,76,75,
    75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,
    75,75,75,75,74,74,74,74,74,74,74,74,74,74,74,74,
    74,74,74,74,74,74,74,74,74,73,73,73,73,73,73,73,
    73,73,73,73,73,73,73,73,73,73,73,73,73,72,72,72,
    72,72,72,72,72,72,72,72,72,72,72,72,72,72,72,72,
    72,72,71,71,71,71,71,71,71,71,71,71,71,71,71,71,
    71,71,71,71,71,71,71,70,70,70,70,70,70,70,70,70,
    70,70,70,70,70,70,70,70,70,70,70,70,70,69,69,69,
    69,69,69,69,69,69,69,69,69,69,69,69,69,69,69,69,
    69,69,68,68,68,68,68,68,68,68,68,68,68,68,68,68,
    68,68,68,68,68,68,68,67,67,67,67,67,67,67,67,67,
    67,67,67,67,67,67,67,67,67,67,67,67,67,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,65,65,65,65,65,65,65,65,65,65,65,65,65,
    65,65,65,65,65,65,65,65,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,63,63,
    63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,
    63,63,63,63,62,62,62,62,62,62,62,62,62,62,62,62,
    62,62,62,62,62,62,62,62,62,61,61,61,61,61,61,61,
    61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,60,
    60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,
    60,60,60,60,60,59,59,59,59,59,59,59,59,59,59,59,
    59,59,59,59,59,59,59,59,59,59,58,58,58,58,58,58,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,
    57,57,57,57,57,57,56,56,56,56,56,56,56,56,56,56,
    56,56,56,56,56,56,56,56,56,56,56,55,55,55,55,55,
    55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,55,
    55,54,54,54,54,54,54,54,54,54,54,54,54,54,54,54,
    54,54,54,54,54,54,54,53,53,53,53,53,53,53,53,53,
    53,53,53,53,53,53,53,53,53,53,53,53,52,52,52,52,
    52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
    52,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,
    51,51,51,51,51,51,51,50,50,50,50,50,50,50,50,50,
    50,50,50,50,50,50,50,50,50,50,50,50,49,49,49,49,
    49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,
    49,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,
    48,48,48,48,48,47,47,47,47,47,47,47,47,47,47,47,
    47,47,47,47,47,47,47,47,47,47,46,46,46,46,46,46,
    46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,45,
    45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,
    45,45,45,44,44,44,44,44,44,44,44,44,44,44,44,44,
    44,44,44,44,44,44,44,43,43,43,43,43,43,43,43,43,
    43,43,43,43,43,43,43,43,43,43,43,42,42,42,42,42,
    42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,41,
    41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,
    41,41,41,40,40,40,40,40,40,40,40,40,40,40,40,40,
    40,40,40,40,40,40,39,39,39,39,39,39,39,39,39,39,
    39,39,39,39,39,39,39,39,39,39,38,38,38,38,38,38,
    38,38,38,38,38,38,38,38,38,38,38,38,38,37,37,37,
    37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,36,
    36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,
    36,36,35,35,35,35,35,35,35,35,35,35,35,35,35,35,
    35,35,35,35,35,34,34,34,34,34,34,34,34,34,34,34,
    34,34,34,34,34,34,34,33,33,33,33,33,33,33,33,33,
    33,33,33,33,33,33,33,33,33,32,32,32,32,32,32,32,
    32,32,32,32,32,32,32,32,32,32,31,31,31,31,31,31,
    31,31,31,31,31,31,31,31,31,31,31,31,30,30,30,30,
    30,30,30,30,30,30,30,30,30,30,30,30,30,29,29,29,
    29,29,29,29,29,29,29,29,29,29,29,29,29,29,28,28,
    28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,27,
    27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,
    26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,
    25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,
    24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,
    23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
    22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,21,
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,20,20,
    20,20,20,20,20,20,20,20,20,20,20,20,20,19,19,19,
    19,19,19,19,19,19,19,19,19,19,19,19,18,18,18,18,
    18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,
    17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,
    14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,12,
    12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,
    11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,
    10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,
    9,9,9,8,8,8,8,8,8,8,8,8,8,8,8,7,
    7,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,
    6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,
    5,5,5,4,4,4,4,4,4,4,4,4,4,4,3,3,
    3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,
    2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

// ArcTan LUT,
//  maps tan(angle) to angle fast. Gotta search.
//
// Effective size is 2049;
// The +1 size is to handle the case when x==y without additional checking.
static const angle_t _huge tantoangleTable[2049] =
{
    0,333772,667544,1001315,1335086,1668857,2002626,2336395,
    2670163,3003929,3337694,3671457,4005219,4338979,4672736,5006492,
    5340245,5673995,6007743,6341488,6675230,7008968,7342704,7676435,
    8010164,8343888,8677609,9011325,9345037,9678744,10012447,10346145,
    10679838,11013526,11347209,11680887,12014558,12348225,12681885,13015539,
    13349187,13682829,14016464,14350092,14683714,15017328,15350936,15684536,
    16018129,16351714,16685291,17018860,17352422,17685974,18019518,18353054,
    18686582,19020100,19353610,19687110,20020600,20354080,20687552,21021014,
    21354466,21687906,22021338,22354758,22688168,23021568,23354956,23688332,
    24021698,24355052,24688396,25021726,25355046,25688352,26021648,26354930,
    26688200,27021456,27354702,27687932,28021150,28354356,28687548,29020724,
    29353888,29687038,30020174,30353296,30686404,31019496,31352574,31685636,
    32018684,32351718,32684734,33017736,33350722,33683692,34016648,34349584,
    34682508,35015412,35348300,35681172,36014028,36346868,36679688,37012492,
    37345276,37678044,38010792,38343524,38676240,39008936,39341612,39674272,
    40006912,40339532,40672132,41004716,41337276,41669820,42002344,42334848,
    42667332,42999796,43332236,43664660,43997060,44329444,44661800,44994140,
    45326456,45658752,45991028,46323280,46655512,46987720,47319908,47652072,
    47984212,48316332,48648428,48980500,49312548,49644576,49976580,50308556,
    50640512,50972444,51304352,51636236,51968096,52299928,52631740,52963524,
    53295284,53627020,53958728,54290412,54622068,54953704,55285308,55616888,
    55948444,56279972,56611472,56942948,57274396,57605816,57937212,58268576,
    58599916,58931228,59262512,59593768,59924992,60256192,60587364,60918508,
    61249620,61580704,61911760,62242788,62573788,62904756,63235692,63566604,
    63897480,64228332,64559148,64889940,65220696,65551424,65882120,66212788,
    66543420,66874024,67204600,67535136,67865648,68196120,68526568,68856984,
    69187360,69517712,69848024,70178304,70508560,70838776,71168960,71499112,
    71829224,72159312,72489360,72819376,73149360,73479304,73809216,74139096,
    74468936,74798744,75128520,75458264,75787968,76117632,76447264,76776864,
    77106424,77435952,77765440,78094888,78424304,78753688,79083032,79412336,
    79741608,80070840,80400032,80729192,81058312,81387392,81716432,82045440,
    82374408,82703336,83032224,83361080,83689896,84018664,84347400,84676096,
    85004760,85333376,85661952,85990488,86318984,86647448,86975864,87304240,
    87632576,87960872,88289128,88617344,88945520,89273648,89601736,89929792,
    90257792,90585760,90913688,91241568,91569408,91897200,92224960,92552672,
    92880336,93207968,93535552,93863088,94190584,94518040,94845448,95172816,
    95500136,95827416,96154648,96481832,96808976,97136080,97463136,97790144,
    98117112,98444032,98770904,99097736,99424520,99751256,100077944,100404592,
    100731192,101057744,101384248,101710712,102037128,102363488,102689808,103016080,
    103342312,103668488,103994616,104320696,104646736,104972720,105298656,105624552,
    105950392,106276184,106601928,106927624,107253272,107578872,107904416,108229920,
    108555368,108880768,109206120,109531416,109856664,110181872,110507016,110832120,
    111157168,111482168,111807112,112132008,112456856,112781648,113106392,113431080,
    113755720,114080312,114404848,114729328,115053760,115378136,115702464,116026744,
    116350960,116675128,116999248,117323312,117647320,117971272,118295176,118619024,
    118942816,119266560,119590248,119913880,120237456,120560984,120884456,121207864,
    121531224,121854528,122177784,122500976,122824112,123147200,123470224,123793200,
    124116120,124438976,124761784,125084528,125407224,125729856,126052432,126374960,
    126697424,127019832,127342184,127664472,127986712,128308888,128631008,128953072,
    129275080,129597024,129918912,130240744,130562520,130884232,131205888,131527480,
    131849016,132170496,132491912,132813272,133134576,133455816,133776992,134098120,
    134419184,134740176,135061120,135382000,135702816,136023584,136344272,136664912,
    136985488,137306016,137626464,137946864,138267184,138587456,138907664,139227808,
    139547904,139867920,140187888,140507776,140827616,141147392,141467104,141786752,
    142106336,142425856,142745312,143064720,143384048,143703312,144022512,144341664,
    144660736,144979744,145298704,145617584,145936400,146255168,146573856,146892480,
    147211040,147529536,147847968,148166336,148484640,148802880,149121056,149439152,
    149757200,150075168,150393072,150710912,151028688,151346400,151664048,151981616,
    152299136,152616576,152933952,153251264,153568496,153885680,154202784,154519824,
    154836784,155153696,155470528,155787296,156104000,156420624,156737200,157053696,
    157370112,157686480,158002768,158318976,158635136,158951216,159267232,159583168,
    159899040,160214848,160530592,160846256,161161840,161477376,161792832,162108208,
    162423520,162738768,163053952,163369040,163684080,163999040,164313936,164628752,
    164943504,165258176,165572784,165887312,166201776,166516160,166830480,167144736,
    167458912,167773008,168087040,168400992,168714880,169028688,169342432,169656096,
    169969696,170283216,170596672,170910032,171223344,171536576,171849728,172162800,
    172475808,172788736,173101600,173414384,173727104,174039728,174352288,174664784,
    174977200,175289536,175601792,175913984,176226096,176538144,176850096,177161984,
    177473792,177785536,178097200,178408784,178720288,179031728,179343088,179654368,
    179965568,180276704,180587744,180898720,181209616,181520448,181831184,182141856,
    182452448,182762960,183073408,183383760,183694048,184004240,184314368,184624416,
    184934400,185244288,185554096,185863840,186173504,186483072,186792576,187102000,
    187411344,187720608,188029808,188338912,188647936,188956896,189265760,189574560,
    189883264,190191904,190500448,190808928,191117312,191425632,191733872,192042016,
    192350096,192658096,192966000,193273840,193581584,193889264,194196848,194504352,
    194811792,195119136,195426400,195733584,196040688,196347712,196654656,196961520,
    197268304,197574992,197881616,198188144,198494592,198800960,199107248,199413456,
    199719584,200025616,200331584,200637456,200943248,201248960,201554576,201860128,
    202165584,202470960,202776256,203081456,203386592,203691632,203996592,204301472,
    204606256,204910976,205215600,205520144,205824592,206128960,206433248,206737456,
    207041584,207345616,207649568,207953424,208257216,208560912,208864512,209168048,
    209471488,209774832,210078112,210381296,210684384,210987408,211290336,211593184,
    211895936,212198608,212501184,212803680,213106096,213408432,213710672,214012816,
    214314880,214616864,214918768,215220576,215522288,215823920,216125472,216426928,
    216728304,217029584,217330784,217631904,217932928,218233856,218534704,218835472,
    219136144,219436720,219737216,220037632,220337952,220638192,220938336,221238384,
    221538352,221838240,222138032,222437728,222737344,223036880,223336304,223635664,
    223934912,224234096,224533168,224832160,225131072,225429872,225728608,226027232,
    226325776,226624240,226922608,227220880,227519056,227817152,228115168,228413088,
    228710912,229008640,229306288,229603840,229901312,230198688,230495968,230793152,
    231090256,231387280,231684192,231981024,232277760,232574416,232870960,233167440,
    233463808,233760096,234056288,234352384,234648384,234944304,235240128,235535872,
    235831504,236127056,236422512,236717888,237013152,237308336,237603424,237898416,
    238193328,238488144,238782864,239077488,239372016,239666464,239960816,240255072,
    240549232,240843312,241137280,241431168,241724960,242018656,242312256,242605776,
    242899200,243192512,243485744,243778896,244071936,244364880,244657744,244950496,
    245243168,245535744,245828224,246120608,246412912,246705104,246997216,247289216,
    247581136,247872960,248164688,248456320,248747856,249039296,249330640,249621904,
    249913056,250204128,250495088,250785968,251076736,251367424,251658016,251948512,
    252238912,252529200,252819408,253109520,253399536,253689456,253979280,254269008,
    254558640,254848176,255137632,255426976,255716224,256005376,256294432,256583392,
    256872256,257161024,257449696,257738272,258026752,258315136,258603424,258891600,
    259179696,259467696,259755600,260043392,260331104,260618704,260906224,261193632,
    261480960,261768176,262055296,262342320,262629248,262916080,263202816,263489456,
    263776000,264062432,264348784,264635024,264921168,265207216,265493168,265779024,
    266064784,266350448,266636000,266921472,267206832,267492096,267777264,268062336,
    268347312,268632192,268916960,269201632,269486208,269770688,270055072,270339360,
    270623552,270907616,271191616,271475488,271759296,272042976,272326560,272610048,
    272893440,273176736,273459936,273743040,274026048,274308928,274591744,274874432,
    275157024,275439520,275721920,276004224,276286432,276568512,276850528,277132416,
    277414240,277695936,277977536,278259040,278540448,278821728,279102944,279384032,
    279665056,279945952,280226752,280507456,280788064,281068544,281348960,281629248,
    281909472,282189568,282469568,282749440,283029248,283308960,283588544,283868032,
    284147424,284426720,284705920,284985024,285264000,285542912,285821696,286100384,
    286378976,286657440,286935840,287214112,287492320,287770400,288048384,288326240,
    288604032,288881696,289159264,289436768,289714112,289991392,290268576,290545632,
    290822592,291099456,291376224,291652896,291929440,292205888,292482272,292758528,
    293034656,293310720,293586656,293862496,294138240,294413888,294689440,294964864,
    295240192,295515424,295790560,296065600,296340512,296615360,296890080,297164704,
    297439200,297713632,297987936,298262144,298536256,298810240,299084160,299357952,
    299631648,299905248,300178720,300452128,300725408,300998592,301271680,301544640,
    301817536,302090304,302362976,302635520,302908000,303180352,303452608,303724768,
    303996800,304268768,304540608,304812320,305083968,305355520,305626944,305898272,
    306169472,306440608,306711616,306982528,307253344,307524064,307794656,308065152,
    308335552,308605856,308876032,309146112,309416096,309685984,309955744,310225408,
    310494976,310764448,311033824,311303072,311572224,311841280,312110208,312379040,
    312647776,312916416,313184960,313453376,313721696,313989920,314258016,314526016,
    314793920,315061728,315329408,315597024,315864512,316131872,316399168,316666336,
    316933408,317200384,317467232,317733984,318000640,318267200,318533632,318799968,
    319066208,319332352,319598368,319864288,320130112,320395808,320661408,320926912,
    321192320,321457632,321722816,321987904,322252864,322517760,322782528,323047200,
    323311744,323576192,323840544,324104800,324368928,324632992,324896928,325160736,
    325424448,325688096,325951584,326215008,326478304,326741504,327004608,327267584,
    327530464,327793248,328055904,328318496,328580960,328843296,329105568,329367712,
    329629760,329891680,330153536,330415264,330676864,330938400,331199808,331461120,
    331722304,331983392,332244384,332505280,332766048,333026752,333287296,333547776,
    333808128,334068384,334328544,334588576,334848512,335108352,335368064,335627712,
    335887200,336146624,336405920,336665120,336924224,337183200,337442112,337700864,
    337959552,338218112,338476576,338734944,338993184,339251328,339509376,339767296,
    340025120,340282848,340540480,340797984,341055392,341312704,341569888,341826976,
    342083968,342340832,342597600,342854272,343110848,343367296,343623648,343879904,
    344136032,344392064,344648000,344903808,345159520,345415136,345670656,345926048,
    346181344,346436512,346691616,346946592,347201440,347456224,347710880,347965440,
    348219872,348474208,348728448,348982592,349236608,349490528,349744320,349998048,
    350251648,350505152,350758528,351011808,351264992,351518048,351771040,352023872,
    352276640,352529280,352781824,353034272,353286592,353538816,353790944,354042944,
    354294880,354546656,354798368,355049952,355301440,355552800,355804096,356055264,
    356306304,356557280,356808128,357058848,357309504,357560032,357810464,358060768,
    358311008,358561088,358811104,359060992,359310784,359560480,359810048,360059520,
    360308896,360558144,360807296,361056352,361305312,361554144,361802880,362051488,
    362300032,362548448,362796736,363044960,363293056,363541024,363788928,364036704,
    364284384,364531936,364779392,365026752,365274016,365521152,365768192,366015136,
    366261952,366508672,366755296,367001792,367248192,367494496,367740704,367986784,
    368232768,368478656,368724416,368970080,369215648,369461088,369706432,369951680,
    370196800,370441824,370686752,370931584,371176288,371420896,371665408,371909792,
    372154080,372398272,372642336,372886304,373130176,373373952,373617600,373861152,
    374104608,374347936,374591168,374834304,375077312,375320224,375563040,375805760,
    376048352,376290848,376533248,376775520,377017696,377259776,377501728,377743584,
    377985344,378227008,378468544,378709984,378951328,379192544,379433664,379674688,
    379915584,380156416,380397088,380637696,380878176,381118560,381358848,381599040,
    381839104,382079072,382318912,382558656,382798304,383037856,383277280,383516640,
    383755840,383994976,384233984,384472896,384711712,384950400,385188992,385427488,
    385665888,385904160,386142336,386380384,386618368,386856224,387093984,387331616,
    387569152,387806592,388043936,388281152,388518272,388755296,388992224,389229024,
    389465728,389702336,389938816,390175200,390411488,390647680,390883744,391119712,
    391355584,391591328,391826976,392062528,392297984,392533312,392768544,393003680,
    393238720,393473632,393708448,393943168,394177760,394412256,394646656,394880960,
    395115136,395349216,395583200,395817088,396050848,396284512,396518080,396751520,
    396984864,397218112,397451264,397684288,397917248,398150080,398382784,398615424,
    398847936,399080320,399312640,399544832,399776928,400008928,400240832,400472608,
    400704288,400935872,401167328,401398720,401629984,401861120,402092192,402323136,
    402553984,402784736,403015360,403245888,403476320,403706656,403936896,404167008,
    404397024,404626944,404856736,405086432,405316032,405545536,405774912,406004224,
    406233408,406462464,406691456,406920320,407149088,407377760,407606336,407834784,
    408063136,408291392,408519520,408747584,408975520,409203360,409431072,409658720,
    409886240,410113664,410340992,410568192,410795296,411022304,411249216,411476032,
    411702720,411929312,412155808,412382176,412608480,412834656,413060736,413286720,
    413512576,413738336,413964000,414189568,414415040,414640384,414865632,415090784,
    415315840,415540800,415765632,415990368,416215008,416439552,416663968,416888288,
    417112512,417336640,417560672,417784576,418008384,418232096,418455712,418679200,
    418902624,419125920,419349120,419572192,419795200,420018080,420240864,420463552,
    420686144,420908608,421130976,421353280,421575424,421797504,422019488,422241344,
    422463104,422684768,422906336,423127776,423349120,423570400,423791520,424012576,
    424233536,424454368,424675104,424895744,425116288,425336736,425557056,425777280,
    425997408,426217440,426437376,426657184,426876928,427096544,427316064,427535488,
    427754784,427974016,428193120,428412128,428631040,428849856,429068544,429287168,
    429505664,429724064,429942368,430160576,430378656,430596672,430814560,431032352,
    431250048,431467616,431685120,431902496,432119808,432336992,432554080,432771040,
    432987936,433204736,433421408,433637984,433854464,434070848,434287104,434503296,
    434719360,434935360,435151232,435367008,435582656,435798240,436013696,436229088,
    436444352,436659520,436874592,437089568,437304416,437519200,437733856,437948416,
    438162880,438377248,438591520,438805696,439019744,439233728,439447584,439661344,
    439875008,440088576,440302048,440515392,440728672,440941824,441154880,441367872,
    441580736,441793472,442006144,442218720,442431168,442643552,442855808,443067968,
    443280032,443492000,443703872,443915648,444127296,444338880,444550336,444761696,
    444972992,445184160,445395232,445606176,445817056,446027840,446238496,446449088,
    446659552,446869920,447080192,447290400,447500448,447710432,447920320,448130112,
    448339776,448549376,448758848,448968224,449177536,449386720,449595808,449804800,
    450013664,450222464,450431168,450639776,450848256,451056640,451264960,451473152,
    451681248,451889248,452097152,452304960,452512672,452720288,452927808,453135232,
    453342528,453549760,453756864,453963904,454170816,454377632,454584384,454791008,
    454997536,455203968,455410304,455616544,455822688,456028704,456234656,456440512,
    456646240,456851904,457057472,457262912,457468256,457673536,457878688,458083744,
    458288736,458493600,458698368,458903040,459107616,459312096,459516480,459720768,
    459924960,460129056,460333056,460536960,460740736,460944448,461148064,461351584,
    461554976,461758304,461961536,462164640,462367680,462570592,462773440,462976160,
    463178816,463381344,463583776,463786144,463988384,464190560,464392608,464594560,
    464796448,464998208,465199872,465401472,465602944,465804320,466005600,466206816,
    466407904,466608896,466809824,467010624,467211328,467411936,467612480,467812896,
    468013216,468213440,468413600,468613632,468813568,469013440,469213184,469412832,
    469612416,469811872,470011232,470210528,470409696,470608800,470807776,471006688,
    471205472,471404192,471602784,471801312,471999712,472198048,472396288,472594400,
    472792448,472990400,473188256,473385984,473583648,473781216,473978688,474176064,
    474373344,474570528,474767616,474964608,475161504,475358336,475555040,475751648,
    475948192,476144608,476340928,476537184,476733312,476929376,477125344,477321184,
    477516960,477712640,477908224,478103712,478299104,478494400,478689600,478884704,
    479079744,479274656,479469504,479664224,479858880,480053408,480247872,480442240,
    480636512,480830656,481024736,481218752,481412640,481606432,481800128,481993760,
    482187264,482380704,482574016,482767264,482960416,483153472,483346432,483539296,
    483732064,483924768,484117344,484309856,484502240,484694560,484886784,485078912,
    485270944,485462880,485654720,485846464,486038144,486229696,486421184,486612576,
    486803840,486995040,487186176,487377184,487568096,487758912,487949664,488140320,
    488330880,488521312,488711712,488901984,489092160,489282240,489472256,489662176,
    489851968,490041696,490231328,490420896,490610336,490799712,490988960,491178144,
    491367232,491556224,491745120,491933920,492122656,492311264,492499808,492688256,
    492876608,493064864,493253056,493441120,493629120,493817024,494004832,494192544,
    494380160,494567712,494755136,494942496,495129760,495316928,495504000,495691008,
    495877888,496064704,496251424,496438048,496624608,496811040,496997408,497183680,
    497369856,497555936,497741920,497927840,498113632,498299360,498484992,498670560,
    498856000,499041376,499226656,499411840,499596928,499781920,499966848,500151680,
    500336416,500521056,500705600,500890080,501074464,501258752,501442944,501627040,
    501811072,501995008,502178848,502362592,502546240,502729824,502913312,503096704,
    503280000,503463232,503646368,503829408,504012352,504195200,504377984,504560672,
    504743264,504925760,505108192,505290496,505472736,505654912,505836960,506018944,
    506200832,506382624,506564320,506745952,506927488,507108928,507290272,507471552,
    507652736,507833824,508014816,508195744,508376576,508557312,508737952,508918528,
    509099008,509279392,509459680,509639904,509820032,510000064,510180000,510359872,
    510539648,510719328,510898944,511078432,511257856,511437216,511616448,511795616,
    511974688,512153664,512332576,512511392,512690112,512868768,513047296,513225792,
    513404160,513582432,513760640,513938784,514116800,514294752,514472608,514650368,
    514828064,515005664,515183168,515360608,515537952,515715200,515892352,516069440,
    516246432,516423328,516600160,516776896,516953536,517130112,517306592,517482976,
    517659264,517835488,518011616,518187680,518363648,518539520,518715296,518891008,
    519066624,519242144,519417600,519592960,519768256,519943424,520118528,520293568,
    520468480,520643328,520818112,520992800,521167392,521341888,521516320,521690656,
    521864896,522039072,522213152,522387168,522561056,522734912,522908640,523082304,
    523255872,523429376,523602784,523776096,523949312,524122464,524295552,524468512,
    524641440,524814240,524986976,525159616,525332192,525504640,525677056,525849344,
    526021568,526193728,526365792,526537760,526709632,526881440,527053152,527224800,
    527396352,527567840,527739200,527910528,528081728,528252864,528423936,528594880,
    528765760,528936576,529107296,529277920,529448480,529618944,529789344,529959648,
    530129856,530300000,530470048,530640000,530809888,530979712,531149440,531319072,
    531488608,531658080,531827488,531996800,532166016,532335168,532504224,532673184,
    532842080,533010912,533179616,533348288,533516832,533685312,533853728,534022048,
    534190272,534358432,534526496,534694496,534862400,535030240,535197984,535365632,
    535533216,535700704,535868128,536035456,536202720,536369888,536536992,536704000,
    536870912
};

// Tangens LUT.
// Should work with BAM fairly well (12 of 16bit, effectively, by shifting).
static const fixed_t _huge finetangentTable[4096] =
{
    -170910304,-56965752,-34178904,-24413316,-18988036,-15535599,-13145455,-11392683,
    -10052327,-8994149,-8137527,-7429880,-6835455,-6329090,-5892567,-5512368,
    -5178251,-4882318,-4618375,-4381502,-4167737,-3973855,-3797206,-3635590,
    -3487165,-3350381,-3223918,-3106651,-2997613,-2895966,-2800983,-2712030,
    -2628549,-2550052,-2476104,-2406322,-2340362,-2277919,-2218719,-2162516,
    -2109087,-2058233,-2009771,-1963536,-1919378,-1877161,-1836758,-1798063,
    -1760956,-1725348,-1691149,-1658278,-1626658,-1596220,-1566898,-1538632,
    -1511367,-1485049,-1459630,-1435065,-1411312,-1388330,-1366084,-1344537,
    -1323658,-1303416,-1283783,-1264730,-1246234,-1228269,-1210813,-1193846,
    -1177345,-1161294,-1145673,-1130465,-1115654,-1101225,-1087164,-1073455,
    -1060087,-1047046,-1034322,-1021901,-1009774,-997931,-986361,-975054,
    -964003,-953199,-942633,-932298,-922186,-912289,-902602,-893117,
    -883829,-874730,-865817,-857081,-848520,-840127,-831898,-823827,
    -815910,-808143,-800521,-793041,-785699,-778490,-771411,-764460,
    -757631,-750922,-744331,-737853,-731486,-725227,-719074,-713023,
    -707072,-701219,-695462,-689797,-684223,-678737,-673338,-668024,
    -662792,-657640,-652568,-647572,-642651,-637803,-633028,-628323,
    -623686,-619117,-614613,-610174,-605798,-601483,-597229,-593033,
    -588896,-584815,-580789,-576818,-572901,-569035,-565221,-561456,
    -557741,-554074,-550455,-546881,-543354,-539870,-536431,-533034,
    -529680,-526366,-523094,-519861,-516667,-513512,-510394,-507313,
    -504269,-501261,-498287,-495348,-492443,-489571,-486732,-483925,
    -481150,-478406,-475692,-473009,-470355,-467730,-465133,-462565,
    -460024,-457511,-455024,-452564,-450129,-447720,-445337,-442978,
    -440643,-438332,-436045,-433781,-431540,-429321,-427125,-424951,
    -422798,-420666,-418555,-416465,-414395,-412344,-410314,-408303,
    -406311,-404338,-402384,-400448,-398530,-396630,-394747,-392882,
    -391034,-389202,-387387,-385589,-383807,-382040,-380290,-378555,
    -376835,-375130,-373440,-371765,-370105,-368459,-366826,-365208,
    -363604,-362013,-360436,-358872,-357321,-355783,-354257,-352744,
    -351244,-349756,-348280,-346816,-345364,-343924,-342495,-341078,
    -339671,-338276,-336892,-335519,-334157,-332805,-331464,-330133,
    -328812,-327502,-326201,-324910,-323629,-322358,-321097,-319844,
    -318601,-317368,-316143,-314928,-313721,-312524,-311335,-310154,
    -308983,-307819,-306664,-305517,-304379,-303248,-302126,-301011,
    -299904,-298805,-297714,-296630,-295554,-294485,-293423,-292369,
    -291322,-290282,-289249,-288223,-287204,-286192,-285186,-284188,
    -283195,-282210,-281231,-280258,-279292,-278332,-277378,-276430,
    -275489,-274553,-273624,-272700,-271782,-270871,-269965,-269064,
    -268169,-267280,-266397,-265519,-264646,-263779,-262917,-262060,
    -261209,-260363,-259522,-258686,-257855,-257029,-256208,-255392,
    -254581,-253774,-252973,-252176,-251384,-250596,-249813,-249035,
    -248261,-247492,-246727,-245966,-245210,-244458,-243711,-242967,
    -242228,-241493,-240763,-240036,-239314,-238595,-237881,-237170,
    -236463,-235761,-235062,-234367,-233676,-232988,-232304,-231624,
    -230948,-230275,-229606,-228941,-228279,-227621,-226966,-226314,
    -225666,-225022,-224381,-223743,-223108,-222477,-221849,-221225,
    -220603,-219985,-219370,-218758,-218149,-217544,-216941,-216341,
    -215745,-215151,-214561,-213973,-213389,-212807,-212228,-211652,
    -211079,-210509,-209941,-209376,-208815,-208255,-207699,-207145,
    -206594,-206045,-205500,-204956,-204416,-203878,-203342,-202809,
    -202279,-201751,-201226,-200703,-200182,-199664,-199149,-198636,
    -198125,-197616,-197110,-196606,-196105,-195606,-195109,-194614,
    -194122,-193631,-193143,-192658,-192174,-191693,-191213,-190736,
    -190261,-189789,-189318,-188849,-188382,-187918,-187455,-186995,
    -186536,-186080,-185625,-185173,-184722,-184274,-183827,-183382,
    -182939,-182498,-182059,-181622,-181186,-180753,-180321,-179891,
    -179463,-179037,-178612,-178190,-177769,-177349,-176932,-176516,
    -176102,-175690,-175279,-174870,-174463,-174057,-173653,-173251,
    -172850,-172451,-172053,-171657,-171263,-170870,-170479,-170089,
    -169701,-169315,-168930,-168546,-168164,-167784,-167405,-167027,
    -166651,-166277,-165904,-165532,-165162,-164793,-164426,-164060,
    -163695,-163332,-162970,-162610,-162251,-161893,-161537,-161182,
    -160828,-160476,-160125,-159775,-159427,-159079,-158734,-158389,
    -158046,-157704,-157363,-157024,-156686,-156349,-156013,-155678,
    -155345,-155013,-154682,-154352,-154024,-153697,-153370,-153045,
    -152722,-152399,-152077,-151757,-151438,-151120,-150803,-150487,
    -150172,-149859,-149546,-149235,-148924,-148615,-148307,-148000,
    -147693,-147388,-147084,-146782,-146480,-146179,-145879,-145580,
    -145282,-144986,-144690,-144395,-144101,-143808,-143517,-143226,
    -142936,-142647,-142359,-142072,-141786,-141501,-141217,-140934,
    -140651,-140370,-140090,-139810,-139532,-139254,-138977,-138701,
    -138426,-138152,-137879,-137607,-137335,-137065,-136795,-136526,
    -136258,-135991,-135725,-135459,-135195,-134931,-134668,-134406,
    -134145,-133884,-133625,-133366,-133108,-132851,-132594,-132339,
    -132084,-131830,-131576,-131324,-131072,-130821,-130571,-130322,
    -130073,-129825,-129578,-129332,-129086,-128841,-128597,-128353,
    -128111,-127869,-127627,-127387,-127147,-126908,-126669,-126432,
    -126195,-125959,-125723,-125488,-125254,-125020,-124787,-124555,
    -124324,-124093,-123863,-123633,-123404,-123176,-122949,-122722,
    -122496,-122270,-122045,-121821,-121597,-121374,-121152,-120930,
    -120709,-120489,-120269,-120050,-119831,-119613,-119396,-119179,
    -118963,-118747,-118532,-118318,-118104,-117891,-117678,-117466,
    -117254,-117044,-116833,-116623,-116414,-116206,-115998,-115790,
    -115583,-115377,-115171,-114966,-114761,-114557,-114354,-114151,
    -113948,-113746,-113545,-113344,-113143,-112944,-112744,-112546,
    -112347,-112150,-111952,-111756,-111560,-111364,-111169,-110974,
    -110780,-110586,-110393,-110200,-110008,-109817,-109626,-109435,
    -109245,-109055,-108866,-108677,-108489,-108301,-108114,-107927,
    -107741,-107555,-107369,-107184,-107000,-106816,-106632,-106449,
    -106266,-106084,-105902,-105721,-105540,-105360,-105180,-105000,
    -104821,-104643,-104465,-104287,-104109,-103933,-103756,-103580,
    -103404,-103229,-103054,-102880,-102706,-102533,-102360,-102187,
    -102015,-101843,-101671,-101500,-101330,-101159,-100990,-100820,
    -100651,-100482,-100314,-100146,-99979,-99812,-99645,-99479,
    -99313,-99148,-98982,-98818,-98653,-98489,-98326,-98163,
    -98000,-97837,-97675,-97513,-97352,-97191,-97030,-96870,
    -96710,-96551,-96391,-96233,-96074,-95916,-95758,-95601,
    -95444,-95287,-95131,-94975,-94819,-94664,-94509,-94354,
    -94200,-94046,-93892,-93739,-93586,-93434,-93281,-93129,
    -92978,-92826,-92675,-92525,-92375,-92225,-92075,-91926,
    -91777,-91628,-91480,-91332,-91184,-91036,-90889,-90742,
    -90596,-90450,-90304,-90158,-90013,-89868,-89724,-89579,
    -89435,-89292,-89148,-89005,-88862,-88720,-88577,-88435,
    -88294,-88152,-88011,-87871,-87730,-87590,-87450,-87310,
    -87171,-87032,-86893,-86755,-86616,-86479,-86341,-86204,
    -86066,-85930,-85793,-85657,-85521,-85385,-85250,-85114,
    -84980,-84845,-84710,-84576,-84443,-84309,-84176,-84043,
    -83910,-83777,-83645,-83513,-83381,-83250,-83118,-82987,
    -82857,-82726,-82596,-82466,-82336,-82207,-82078,-81949,
    -81820,-81691,-81563,-81435,-81307,-81180,-81053,-80925,
    -80799,-80672,-80546,-80420,-80294,-80168,-80043,-79918,
    -79793,-79668,-79544,-79420,-79296,-79172,-79048,-78925,
    -78802,-78679,-78557,-78434,-78312,-78190,-78068,-77947,
    -77826,-77705,-77584,-77463,-77343,-77223,-77103,-76983,
    -76864,-76744,-76625,-76506,-76388,-76269,-76151,-76033,
    -75915,-75797,-75680,-75563,-75446,-75329,-75213,-75096,
    -74980,-74864,-74748,-74633,-74517,-74402,-74287,-74172,
    -74058,-73944,-73829,-73715,-73602,-73488,-73375,-73262,
    -73149,-73036,-72923,-72811,-72699,-72587,-72475,-72363,
    -72252,-72140,-72029,-71918,-71808,-71697,-71587,-71477,
    -71367,-71257,-71147,-71038,-70929,-70820,-70711,-70602,
    -70494,-70385,-70277,-70169,-70061,-69954,-69846,-69739,
    -69632,-69525,-69418,-69312,-69205,-69099,-68993,-68887,
    -68781,-68676,-68570,-68465,-68360,-68255,-68151,-68046,
    -67942,-67837,-67733,-67629,-67526,-67422,-67319,-67216,
    -67113,-67010,-66907,-66804,-66702,-66600,-66498,-66396,
    -66294,-66192,-66091,-65989,-65888,-65787,-65686,-65586,
    -65485,-65385,-65285,-65185,-65085,-64985,-64885,-64786,
    -64687,-64587,-64488,-64389,-64291,-64192,-64094,-63996,
    -63897,-63799,-63702,-63604,-63506,-63409,-63312,-63215,
    -63118,-63021,-62924,-62828,-62731,-62635,-62539,-62443,
    -62347,-62251,-62156,-62060,-61965,-61870,-61775,-61680,
    -61585,-61491,-61396,-61302,-61208,-61114,-61020,-60926,
    -60833,-60739,-60646,-60552,-60459,-60366,-60273,-60181,
    -60088,-59996,-59903,-59811,-59719,-59627,-59535,-59444,
    -59352,-59261,-59169,-59078,-58987,-58896,-58805,-58715,
    -58624,-58534,-58443,-58353,-58263,-58173,-58083,-57994,
    -57904,-57815,-57725,-57636,-57547,-57458,-57369,-57281,
    -57192,-57104,-57015,-56927,-56839,-56751,-56663,-56575,
    -56487,-56400,-56312,-56225,-56138,-56051,-55964,-55877,
    -55790,-55704,-55617,-55531,-55444,-55358,-55272,-55186,
    -55100,-55015,-54929,-54843,-54758,-54673,-54587,-54502,
    -54417,-54333,-54248,-54163,-54079,-53994,-53910,-53826,
    -53741,-53657,-53574,-53490,-53406,-53322,-53239,-53156,
    -53072,-52989,-52906,-52823,-52740,-52657,-52575,-52492,
    -52410,-52327,-52245,-52163,-52081,-51999,-51917,-51835,
    -51754,-51672,-51591,-51509,-51428,-51347,-51266,-51185,
    -51104,-51023,-50942,-50862,-50781,-50701,-50621,-50540,
    -50460,-50380,-50300,-50221,-50141,-50061,-49982,-49902,
    -49823,-49744,-49664,-49585,-49506,-49427,-49349,-49270,
    -49191,-49113,-49034,-48956,-48878,-48799,-48721,-48643,
    -48565,-48488,-48410,-48332,-48255,-48177,-48100,-48022,
    -47945,-47868,-47791,-47714,-47637,-47560,-47484,-47407,
    -47331,-47254,-47178,-47102,-47025,-46949,-46873,-46797,
    -46721,-46646,-46570,-46494,-46419,-46343,-46268,-46193,
    -46118,-46042,-45967,-45892,-45818,-45743,-45668,-45593,
    -45519,-45444,-45370,-45296,-45221,-45147,-45073,-44999,
    -44925,-44851,-44778,-44704,-44630,-44557,-44483,-44410,
    -44337,-44263,-44190,-44117,-44044,-43971,-43898,-43826,
    -43753,-43680,-43608,-43535,-43463,-43390,-43318,-43246,
    -43174,-43102,-43030,-42958,-42886,-42814,-42743,-42671,
    -42600,-42528,-42457,-42385,-42314,-42243,-42172,-42101,
    -42030,-41959,-41888,-41817,-41747,-41676,-41605,-41535,
    -41465,-41394,-41324,-41254,-41184,-41113,-41043,-40973,
    -40904,-40834,-40764,-40694,-40625,-40555,-40486,-40416,
    -40347,-40278,-40208,-40139,-40070,-40001,-39932,-39863,
    -39794,-39726,-39657,-39588,-39520,-39451,-39383,-39314,
    -39246,-39178,-39110,-39042,-38973,-38905,-38837,-38770,
    -38702,-38634,-38566,-38499,-38431,-38364,-38296,-38229,
    -38161,-38094,-38027,-37960,-37893,-37826,-37759,-37692,
    -37625,-37558,-37491,-37425,-37358,-37291,-37225,-37158,
    -37092,-37026,-36959,-36893,-36827,-36761,-36695,-36629,
    -36563,-36497,-36431,-36365,-36300,-36234,-36168,-36103,
    -36037,-35972,-35907,-35841,-35776,-35711,-35646,-35580,
    -35515,-35450,-35385,-35321,-35256,-35191,-35126,-35062,
    -34997,-34932,-34868,-34803,-34739,-34675,-34610,-34546,
    -34482,-34418,-34354,-34289,-34225,-34162,-34098,-34034,
    -33970,-33906,-33843,-33779,-33715,-33652,-33588,-33525,
    -33461,-33398,-33335,-33272,-33208,-33145,-33082,-33019,
    -32956,-32893,-32830,-32767,-32705,-32642,-32579,-32516,
    -32454,-32391,-32329,-32266,-32204,-32141,-32079,-32017,
    -31955,-31892,-31830,-31768,-31706,-31644,-31582,-31520,
    -31458,-31396,-31335,-31273,-31211,-31150,-31088,-31026,
    -30965,-30904,-30842,-30781,-30719,-30658,-30597,-30536,
    -30474,-30413,-30352,-30291,-30230,-30169,-30108,-30048,
    -29987,-29926,-29865,-29805,-29744,-29683,-29623,-29562,
    -29502,-29441,-29381,-29321,-29260,-29200,-29140,-29080,
    -29020,-28959,-28899,-28839,-28779,-28719,-28660,-28600,
    -28540,-28480,-28420,-28361,-28301,-28241,-28182,-28122,
    -28063,-28003,-27944,-27884,-27825,-27766,-27707,-27647,
    -27588,-27529,-27470,-27411,-27352,-27293,-27234,-27175,
    -27116,-27057,-26998,-26940,-26881,-26822,-26763,-26705,
    -26646,-26588,-26529,-26471,-26412,-26354,-26295,-26237,
    -26179,-26120,-26062,-26004,-25946,-25888,-25830,-25772,
    -25714,-25656,-25598,-25540,-25482,-25424,-25366,-25308,
    -25251,-25193,-25135,-25078,-25020,-24962,-24905,-24847,
    -24790,-24732,-24675,-24618,-24560,-24503,-24446,-24389,
    -24331,-24274,-24217,-24160,-24103,-24046,-23989,-23932,
    -23875,-23818,-23761,-23704,-23647,-23591,-23534,-23477,
    -23420,-23364,-23307,-23250,-23194,-23137,-23081,-23024,
    -22968,-22911,-22855,-22799,-22742,-22686,-22630,-22573,
    -22517,-22461,-22405,-22349,-22293,-22237,-22181,-22125,
    -22069,-22013,-21957,-21901,-21845,-21789,-21733,-21678,
    -21622,-21566,-21510,-21455,-21399,-21343,-21288,-21232,
    -21177,-21121,-21066,-21010,-20955,-20900,-20844,-20789,
    -20734,-20678,-20623,-20568,-20513,-20457,-20402,-20347,
    -20292,-20237,-20182,-20127,-20072,-20017,-19962,-19907,
    -19852,-19797,-19742,-19688,-19633,-19578,-19523,-19469,
    -19414,-19359,-19305,-19250,-19195,-19141,-19086,-19032,
    -18977,-18923,-18868,-18814,-18760,-18705,-18651,-18597,
    -18542,-18488,-18434,-18380,-18325,-18271,-18217,-18163,
    -18109,-18055,-18001,-17946,-17892,-17838,-17784,-17731,
    -17677,-17623,-17569,-17515,-17461,-17407,-17353,-17300,
    -17246,-17192,-17138,-17085,-17031,-16977,-16924,-16870,
    -16817,-16763,-16710,-16656,-16603,-16549,-16496,-16442,
    -16389,-16335,-16282,-16229,-16175,-16122,-16069,-16015,
    -15962,-15909,-15856,-15802,-15749,-15696,-15643,-15590,
    -15537,-15484,-15431,-15378,-15325,-15272,-15219,-15166,
    -15113,-15060,-15007,-14954,-14901,-14848,-14795,-14743,
    -14690,-14637,-14584,-14531,-14479,-14426,-14373,-14321,
    -14268,-14215,-14163,-14110,-14057,-14005,-13952,-13900,
    -13847,-13795,-13742,-13690,-13637,-13585,-13533,-13480,
    -13428,-13375,-13323,-13271,-13218,-13166,-13114,-13062,
    -13009,-12957,-12905,-12853,-12800,-12748,-12696,-12644,
    -12592,-12540,-12488,-12436,-12383,-12331,-12279,-12227,
    -12175,-12123,-12071,-12019,-11967,-11916,-11864,-11812,
    -11760,-11708,-11656,-11604,-11552,-11501,-11449,-11397,
    -11345,-11293,-11242,-11190,-11138,-11086,-11035,-10983,
    -10931,-10880,-10828,-10777,-10725,-10673,-10622,-10570,
    -10519,-10467,-10415,-10364,-10312,-10261,-10209,-10158,
    -10106,-10055,-10004,-9952,-9901,-9849,-9798,-9747,
    -9695,-9644,-9592,-9541,-9490,-9438,-9387,-9336,
    -9285,-9233,-9182,-9131,-9080,-9028,-8977,-8926,
    -8875,-8824,-8772,-8721,-8670,-8619,-8568,-8517,
    -8466,-8414,-8363,-8312,-8261,-8210,-8159,-8108,
    -8057,-8006,-7955,-7904,-7853,-7802,-7751,-7700,
    -7649,-7598,-7547,-7496,-7445,-7395,-7344,-7293,
    -7242,-7191,-7140,-7089,-7038,-6988,-6937,-6886,
    -6835,-6784,-6733,-6683,-6632,-6581,-6530,-6480,
    -6429,-6378,-6327,-6277,-6226,-6175,-6124,-6074,
    -6023,-5972,-5922,-5871,-5820,-5770,-5719,-5668,
    -5618,-5567,-5517,-5466,-5415,-5365,-5314,-5264,
    -5213,-5162,-5112,-5061,-5011,-4960,-4910,-4859,
    -4808,-4758,-4707,-4657,-4606,-4556,-4505,-4455,
    -4404,-4354,-4303,-4253,-4202,-4152,-4101,-4051,
    -4001,-3950,-3900,-3849,-3799,-3748,-3698,-3648,
    -3597,-3547,-3496,-3446,-3395,-3345,-3295,-3244,
    -3194,-3144,-3093,-3043,-2992,-2942,-2892,-2841,
    -2791,-2741,-2690,-2640,-2590,-2539,-2489,-2439,
    -2388,-2338,-2288,-2237,-2187,-2137,-2086,-2036,
    -1986,-1935,-1885,-1835,-1784,-1734,-1684,-1633,
    -1583,-1533,-1483,-1432,-1382,-1332,-1281,-1231,
    -1181,-1131,-1080,-1030,-980,-929,-879,-829,
    -779,-728,-678,-628,-578,-527,-477,-427,
    -376,-326,-276,-226,-175,-125,-75,-25,
    25,75,125,175,226,276,326,376,
    427,477,527,578,628,678,728,779,
    829,879,929,980,1030,1080,1131,1181,
    1231,1281,1332,1382,1432,1483,1533,1583,
    1633,1684,1734,1784,1835,1885,1935,1986,
    2036,2086,2137,2187,2237,2288,2338,2388,
    2439,2489,2539,2590,2640,2690,2741,2791,
    2841,2892,2942,2992,3043,3093,3144,3194,
    3244,3295,3345,3395,3446,3496,3547,3597,
    3648,3698,3748,3799,3849,3900,3950,4001,
    4051,4101,4152,4202,4253,4303,4354,4404,
    4455,4505,4556,4606,4657,4707,4758,4808,
    4859,4910,4960,5011,5061,5112,5162,5213,
    5264,5314,5365,5415,5466,5517,5567,5618,
    5668,5719,5770,5820,5871,5922,5972,6023,
    6074,6124,6175,6226,6277,6327,6378,6429,
    6480,6530,6581,6632,6683,6733,6784,6835,
    6886,6937,6988,7038,7089,7140,7191,7242,
    7293,7344,7395,7445,7496,7547,7598,7649,
    7700,7751,7802,7853,7904,7955,8006,8057,
    8108,8159,8210,8261,8312,8363,8414,8466,
    8517,8568,8619,8670,8721,8772,8824,8875,
    8926,8977,9028,9080,9131,9182,9233,9285,
    9336,9387,9438,9490,9541,9592,9644,9695,
    9747,9798,9849,9901,9952,10004,10055,10106,
    10158,10209,10261,10312,10364,10415,10467,10519,
    10570,10622,10673,10725,10777,10828,10880,10931,
    10983,11035,11086,11138,11190,11242,11293,11345,
    11397,11449,11501,11552,11604,11656,11708,11760,
    11812,11864,11916,11967,12019,12071,12123,12175,
    12227,12279,12331,12383,12436,12488,12540,12592,
    12644,12696,12748,12800,12853,12905,12957,13009,
    13062,13114,13166,13218,13271,13323,13375,13428,
    13480,13533,13585,13637,13690,13742,13795,13847,
    13900,13952,14005,14057,14110,14163,14215,14268,
    14321,14373,14426,14479,14531,14584,14637,14690,
    14743,14795,14848,14901,14954,15007,15060,15113,
    15166,15219,15272,15325,15378,15431,15484,15537,
    15590,15643,15696,15749,15802,15856,15909,15962,
    16015,16069,16122,16175,16229,16282,16335,16389,
    16442,16496,16549,16603,16656,16710,16763,16817,
    16870,16924,16977,17031,17085,17138,17192,17246,
    17300,17353,17407,17461,17515,17569,17623,17677,
    17731,17784,17838,17892,17946,18001,18055,18109,
    18163,18217,18271,18325,18380,18434,18488,18542,
    18597,18651,18705,18760,18814,18868,18923,18977,
    19032,19086,19141,19195,19250,19305,19359,19414,
    19469,19523,19578,19633,19688,19742,19797,19852,
    19907,19962,20017,20072,20127,20182,20237,20292,
    20347,20402,20457,20513,20568,20623,20678,20734,
    20789,20844,20900,20955,21010,21066,21121,21177,
    21232,21288,21343,21399,21455,21510,21566,21622,
    21678,21733,21789,21845,21901,21957,22013,22069,
    22125,22181,22237,22293,22349,22405,22461,22517,
    22573,22630,22686,22742,22799,22855,22911,22968,
    23024,23081,23137,23194,23250,23307,23364,23420,
    23477,23534,23591,23647,23704,23761,23818,23875,
    23932,23989,24046,24103,24160,24217,24274,24331,
    24389,24446,24503,24560,24618,24675,24732,24790,
    24847,24905,24962,25020,25078,25135,25193,25251,
    25308,25366,25424,25482,25540,25598,25656,25714,
    25772,25830,25888,25946,26004,26062,26120,26179,
    26237,26295,26354,26412,26471,26529,26588,26646,
    26705,26763,26822,26881,26940,26998,27057,27116,
    27175,27234,27293,27352,27411,27470,27529,27588,
    27647,27707,27766,27825,27884,27944,28003,28063,
    28122,28182,28241,28301,28361,28420,28480,28540,
    28600,28660,28719,28779,28839,28899,28959,29020,
    29080,29140,29200,29260,29321,29381,29441,29502,
    29562,29623,29683,29744,29805,29865,29926,29987,
    30048,30108,30169,30230,30291,30352,30413,30474,
    30536,30597,30658,30719,30781,30842,30904,30965,
    31026,31088,31150,31211,31273,31335,31396,31458,
    31520,31582,31644,31706,31768,31830,31892,31955,
    32017,32079,32141,32204,32266,32329,32391,32454,
    32516,32579,32642,32705,32767,32830,32893,32956,
    33019,33082,33145,33208,33272,33335,33398,33461,
    33525,33588,33652,33715,33779,33843,33906,33970,
    34034,34098,34162,34225,34289,34354,34418,34482,
    34546,34610,34675,34739,34803,34868,34932,34997,
    35062,35126,35191,35256,35321,35385,35450,35515,
    35580,35646,35711,35776,35841,35907,35972,36037,
    36103,36168,36234,36300,36365,36431,36497,36563,
    36629,36695,36761,36827,36893,36959,37026,37092,
    37158,37225,37291,37358,37425,37491,37558,37625,
    37692,37759,37826,37893,37960,38027,38094,38161,
    38229,38296,38364,38431,38499,38566,38634,38702,
    38770,38837,38905,38973,39042,39110,39178,39246,
    39314,39383,39451,39520,39588,39657,39726,39794,
    39863,39932,40001,40070,40139,40208,40278,40347,
    40416,40486,40555,40625,40694,40764,40834,40904,
    40973,41043,41113,41184,41254,41324,41394,41465,
    41535,41605,41676,41747,41817,41888,41959,42030,
    42101,42172,42243,42314,42385,42457,42528,42600,
    42671,42743,42814,42886,42958,43030,43102,43174,
    43246,43318,43390,43463,43535,43608,43680,43753,
    43826,43898,43971,44044,44117,44190,44263,44337,
    44410,44483,44557,44630,44704,44778,44851,44925,
    44999,45073,45147,45221,45296,45370,45444,45519,
    45593,45668,45743,45818,45892,45967,46042,46118,
    46193,46268,46343,46419,46494,46570,46646,46721,
    46797,46873,46949,47025,47102,47178,47254,47331,
    47407,47484,47560,47637,47714,47791,47868,47945,
    48022,48100,48177,48255,48332,48410,48488,48565,
    48643,48721,48799,48878,48956,49034,49113,49191,
    49270,49349,49427,49506,49585,49664,49744,49823,
    49902,49982,50061,50141,50221,50300,50380,50460,
    50540,50621,50701,50781,50862,50942,51023,51104,
    51185,51266,51347,51428,51509,51591,51672,51754,
    51835,51917,51999,52081,52163,52245,52327,52410,
    52492,52575,52657,52740,52823,52906,52989,53072,
    53156,53239,53322,53406,53490,53574,53657,53741,
    53826,53910,53994,54079,54163,54248,54333,54417,
    54502,54587,54673,54758,54843,54929,55015,55100,
    55186,55272,55358,55444,55531,55617,55704,55790,
    55877,55964,56051,56138,56225,56312,56400,56487,
    56575,56663,56751,56839,56927,57015,57104,57192,
    57281,57369,57458,57547,57636,57725,57815,57904,
    57994,58083,58173,58263,58353,58443,58534,58624,
    58715,58805,58896,58987,59078,59169,59261,59352,
    59444,59535,59627,59719,59811,59903,59996,60088,
    60181,60273,60366,60459,60552,60646,60739,60833,
    60926,61020,61114,61208,61302,61396,61491,61585,
    61680,61775,61870,61965,62060,62156,62251,62347,
    62443,62539,62635,62731,62828,62924,63021,63118,
    63215,63312,63409,63506,63604,63702,63799,63897,
    63996,64094,64192,64291,64389,64488,64587,64687,
    64786,64885,64985,65085,65185,65285,65385,65485,
    65586,65686,65787,65888,65989,66091,66192,66294,
    66396,66498,66600,66702,66804,66907,67010,67113,
    67216,67319,67422,67526,67629,67733,67837,67942,
    68046,68151,68255,68360,68465,68570,68676,68781,
    68887,68993,69099,69205,69312,69418,69525,69632,
    69739,69846,69954,70061,70169,70277,70385,70494,
    70602,70711,70820,70929,71038,71147,71257,71367,
    71477,71587,71697,71808,71918,72029,72140,72252,
    72363,72475,72587,72699,72811,72923,73036,73149,
    73262,73375,73488,73602,73715,73829,73944,74058,
    74172,74287,74402,74517,74633,74748,74864,74980,
    75096,75213,75329,75446,75563,75680,75797,75915,
    76033,76151,76269,76388,76506,76625,76744,76864,
    76983,77103,77223,77343,77463,77584,77705,77826,
    77947,78068,78190,78312,78434,78557,78679,78802,
    78925,79048,79172,79296,79420,79544,79668,79793,
    79918,80043,80168,80294,80420,80546,80672,80799,
    80925,81053,81180,81307,81435,81563,81691,81820,
    81949,82078,82207,82336,82466,82596,82726,82857,
    82987,83118,83250,83381,83513,83645,83777,83910,
    84043,84176,84309,84443,84576,84710,84845,84980,
    85114,85250,85385,85521,85657,85793,85930,86066,
    86204,86341,86479,86616,86755,86893,87032,87171,
    87310,87450,87590,87730,87871,88011,88152,88294,
    88435,88577,88720,88862,89005,89148,89292,89435,
    89579,89724,89868,90013,90158,90304,90450,90596,
    90742,90889,91036,91184,91332,91480,91628,91777,
    91926,92075,92225,92375,92525,92675,92826,92978,
    93129,93281,93434,93586,93739,93892,94046,94200,
    94354,94509,94664,94819,94975,95131,95287,95444,
    95601,95758,95916,96074,96233,96391,96551,96710,
    96870,97030,97191,97352,97513,97675,97837,98000,
    98163,98326,98489,98653,98818,98982,99148,99313,
    99479,99645,99812,99979,100146,100314,100482,100651,
    100820,100990,101159,101330,101500,101671,101843,102015,
    102187,102360,102533,102706,102880,103054,103229,103404,
    103580,103756,103933,104109,104287,104465,104643,104821,
    105000,105180,105360,105540,105721,105902,106084,106266,
    106449,106632,106816,107000,107184,107369,107555,107741,
    107927,108114,108301,108489,108677,108866,109055,109245,
    109435,109626,109817,110008,110200,110393,110586,110780,
    110974,111169,111364,111560,111756,111952,112150,112347,
    112546,112744,112944,113143,113344,113545,113746,113948,
    114151,114354,114557,114761,114966,115171,115377,115583,
    115790,115998,116206,116414,116623,116833,117044,117254,
    117466,117678,117891,118104,118318,118532,118747,118963,
    119179,119396,119613,119831,120050,120269,120489,120709,
    120930,121152,121374,121597,121821,122045,122270,122496,
    122722,122949,123176,123404,123633,123863,124093,124324,
    124555,124787,125020,125254,125488,125723,125959,126195,
    126432,126669,126908,127147,127387,127627,127869,128111,
    128353,128597,128841,129086,129332,129578,129825,130073,
    130322,130571,130821,131072,131324,131576,131830,132084,
    132339,132594,132851,133108,133366,133625,133884,134145,
    134406,134668,134931,135195,135459,135725,135991,136258,
    136526,136795,137065,137335,137607,137879,138152,138426,
    138701,138977,139254,139532,139810,140090,140370,140651,
    140934,141217,141501,141786,142072,142359,142647,142936,
    143226,143517,143808,144101,144395,144690,144986,145282,
    145580,145879,146179,146480,146782,147084,147388,147693,
    148000,148307,148615,148924,149235,149546,149859,150172,
    150487,150803,151120,151438,151757,152077,152399,152722,
    153045,153370,153697,154024,154352,154682,155013,155345,
    155678,156013,156349,156686,157024,157363,157704,158046,
    158389,158734,159079,159427,159775,160125,160476,160828,
    161182,161537,161893,162251,162610,162970,163332,163695,
    164060,164426,164793,165162,165532,165904,166277,166651,
    167027,167405,167784,168164,168546,168930,169315,169701,
    170089,170479,170870,171263,171657,172053,172451,172850,
    173251,173653,174057,174463,174870,175279,175690,176102,
    176516,176932,177349,177769,178190,178612,179037,179463,
    179891,180321,180753,181186,181622,182059,182498,182939,
    183382,183827,184274,184722,185173,185625,186080,186536,
    186995,187455,187918,188382,188849,189318,189789,190261,
    190736,191213,191693,192174,192658,193143,193631,194122,
    194614,195109,195606,196105,196606,197110,197616,198125,
    198636,199149,199664,200182,200703,201226,201751,202279,
    202809,203342,203878,204416,204956,205500,206045,206594,
    207145,207699,208255,208815,209376,209941,210509,211079,
    211652,212228,212807,213389,213973,214561,215151,215745,
    216341,216941,217544,218149,218758,219370,219985,220603,
    221225,221849,222477,223108,223743,224381,225022,225666,
    226314,226966,227621,228279,228941,229606,230275,230948,
    231624,232304,232988,233676,234367,235062,235761,236463,
    237170,237881,238595,239314,240036,240763,241493,242228,
    242967,243711,244458,245210,245966,246727,247492,248261,
    249035,249813,250596,251384,252176,252973,253774,254581,
    255392,256208,257029,257855,258686,259522,260363,261209,
    262060,262917,263779,264646,265519,266397,267280,268169,
    269064,269965,270871,271782,272700,273624,274553,275489,
    276430,277378,278332,279292,280258,281231,282210,283195,
    284188,285186,286192,287204,288223,289249,290282,291322,
    292369,293423,294485,295554,296630,297714,298805,299904,
    301011,302126,303248,304379,305517,306664,307819,308983,
    310154,311335,312524,313721,314928,316143,317368,318601,
    319844,321097,322358,323629,324910,326201,327502,328812,
    330133,331464,332805,334157,335519,336892,338276,339671,
    341078,342495,343924,345364,346816,348280,349756,351244,
    352744,354257,355783,357321,358872,360436,362013,363604,
    365208,366826,368459,370105,371765,373440,375130,376835,
    378555,380290,382040,383807,385589,387387,389202,391034,
    392882,394747,396630,398530,400448,402384,404338,406311,
    408303,410314,412344,414395,416465,418555,420666,422798,
    424951,427125,429321,431540,433781,436045,438332,440643,
    442978,445337,447720,450129,452564,455024,457511,460024,
    462565,465133,467730,470355,473009,475692,478406,481150,
    483925,486732,489571,492443,495348,498287,501261,504269,
    507313,510394,513512,516667,519861,523094,526366,529680,
    533034,536431,539870,543354,546881,550455,554074,557741,
    561456,565221,569035,572901,576818,580789,584815,588896,
    593033,597229,601483,605798,610174,614613,619117,623686,
    628323,633028,637803,642651,647572,652568,657640,662792,
    668024,673338,678737,684223,689797,695462,701219,707072,
    713023,719074,725227,731486,737853,744331,750922,757631,
    764460,771411,778490,785699,793041,800521,808143,815910,
    823827,831898,840127,848520,857081,865817,874730,883829,
    893117,902602,912289,922186,932298,942633,953199,964003,
    975054,986361,997931,1009774,1021901,1034322,1047046,1060087,
    1073455,1087164,1101225,1115654,1130465,1145673,1161294,1177345,
    1193846,1210813,1228269,1246234,1264730,1283783,1303416,1323658,
    1344537,1366084,1388330,1411312,1435065,1459630,1485049,1511367,
    1538632,1566898,1596220,1626658,1658278,1691149,1725348,1760956,
    1798063,1836758,1877161,1919378,1963536,2009771,2058233,2109087,
    2162516,2218719,2277919,2340362,2406322,2476104,2550052,2628549,
    2712030,2800983,2895966,2997613,3106651,3223918,3350381,3487165,
    3635590,3797206,3973855,4167737,4381502,4618375,4882318,5178251,
    5512368,5892567,6329090,6835455,7429880,8137527,8994149,10052327,
    11392683,13145455,15535599,18988036,24413316,34178904,56965752,170910304
};
#endif
