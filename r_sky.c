/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023-2025 Frenkel Smeijers
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
 *      Render sky
 *
 *-----------------------------------------------------------------------------*/

#include "r_defs.h"
#include "r_main.h"
#include "w_wad.h"


#if !defined FLAT_SKY_COLOR
#define FLAT_SKY_COLOR 99
#endif


#define ANGLETOSKYSHIFT         22

#define COLEXTRABITS (8 - 1)


#if defined FLAT_SPAN
const int16_t skyflatnum = -2;
#else
int16_t skyflatnum;
#endif

static int16_t skypatchnum;
static uint16_t skywidthmask;


#if defined FLAT_SPAN
static const patch_t __far* skypatch;


void R_LoadSkyPatch(void)
{
	skypatch = W_TryGetLumpByNum(skypatchnum);
}


void R_FreeSkyPatch(void)
{
	if (skypatch)
	{
		Z_ChangeTagToCache(skypatch);
		skypatch = NULL;
	}
}


void R_DrawSky(draw_column_vars_t *dcvars)
{
	if (skypatch == NULL)
		R_DrawColumnFlat(FLAT_SKY_COLOR, dcvars);
	else
	{
		dcvars->texturemid = (SCREENHEIGHT_VGA / 2) * FRACUNIT;

		if (!(dcvars->colormap = fixedcolormap))
			dcvars->colormap = fullcolormap;

		dcvars->fracstep = ((FRACUNIT * SCREENHEIGHT_VGA) / (VIEWWINDOWHEIGHT + 16)) >> COLEXTRABITS;

		int16_t xc = viewangle >> FRACBITS;
		xc += xtoviewangleTable[dcvars->x];
		xc >>= ANGLETOSKYSHIFT - FRACBITS;
		xc &= skywidthmask;

		const column_t __far* column = (const column_t __far*) ((const byte __far*)skypatch + (uint16_t)skypatch->columnofs[xc]);

		dcvars->source = (const byte __far*)column + 3;
		R_DrawColumnWall(dcvars);
	}
}

#else

static void R_DrawSkyFlat(visplane_t __far* pl)
{
	draw_column_vars_t dcvars;

	for (int16_t x = pl->minx; x <= pl->maxx; x++)
	{
		if (pl->top[x] != 0xff &&  pl->top[x] <= pl->bottom[x])
		{
			dcvars.x = x;
			dcvars.yl = pl->top[x];
			dcvars.yh = pl->bottom[x];
			R_DrawColumnFlat(FLAT_SKY_COLOR, &dcvars);
		}
	}
}


void R_DrawSky(visplane_t __far* pl)
{
	const patch_t __far* patch = W_TryGetLumpByNum(skypatchnum);
	if (patch == NULL)
	{
		R_DrawSkyFlat(pl);
		return;
	}

	// Normal Doom sky, only one allowed per level
	draw_column_vars_t dcvars;
	dcvars.texturemid = (SCREENHEIGHT_VGA / 2) * FRACUNIT;    // Default y-offset

	// Sky is always drawn full bright, i.e. colormaps[0] is used.
	// Because of this hack, sky is not affected by INVUL inverse mapping.
	// Until Boom fixed this.

	if (!(dcvars.colormap = fixedcolormap))
		dcvars.colormap = fullcolormap;

	dcvars.fracstep = ((FRACUNIT * SCREENHEIGHT_VGA) / (VIEWWINDOWHEIGHT + 16)) >> COLEXTRABITS;

	for (int16_t x = pl->minx; (dcvars.x = x) <= pl->maxx; x++)
	{
		if ((dcvars.yl = pl->top[x]) != -1 && dcvars.yl <= (dcvars.yh = pl->bottom[x])) // dropoff overflow
		{
			int16_t xc = viewangle >> FRACBITS;
			xc += xtoviewangleTable[x];
			xc >>= ANGLETOSKYSHIFT - FRACBITS;
			xc &= skywidthmask;

			const column_t __far* column = (const column_t __far*) ((const byte __far*)patch + (uint16_t)patch->columnofs[xc]);

			dcvars.source = (const byte __far*)column + 3;
			R_DrawColumn(&dcvars);
		}
	}

	Z_ChangeTagToCache(patch);
}
#endif


// Set the sky map.
void R_InitSky(void)
{
	int16_t skytexture = R_CheckTextureNumForName("SKY1");
	const texture_t __far* tex = R_GetTexture(skytexture);
	skypatchnum  = tex->patches[0].patch_num;
	skywidthmask = tex->widthmask;

#if !defined FLAT_SPAN
	// First thing, we have a dummy sky texture name,
	//  a flat. The data is in the WAD only because
	//  we look for an actual index, instead of simply
	//  setting one.
	skyflatnum = R_FlatNumForName("F_SKY1");
#endif
}
