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
 *      Renderer main interface.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
#include "r_data.h"


#define MAXVISPLANES 32    /* must be a power of 2 */


typedef struct {
  int32_t                 x;
  int32_t                 yl;
  int32_t                 yh;
  fixed_t             iscale;
  fixed_t             texturemid;

  const byte          __far* source; // first pixel in a column

  const lighttable_t  __far* colormap;

  boolean             odd_pixel;

} draw_column_vars_t;


//Global vars.

extern int32_t numnodes;
extern const mapnode_t __far* nodes;

extern fixed_t  viewx, viewy, viewz;

extern angle_t viewangle;

extern int16_t floorclip[SCREENWIDTH];
extern int16_t ceilingclip[SCREENWIDTH];

extern const lighttable_t __far* fullcolormap;
extern const lighttable_t __far* colormaps;
extern const lighttable_t __far* fixedcolormap;

extern fixed_t   __far* textureheight; //needed for texture pegging (and TFE fix - killough)

extern int16_t       __far* texturetranslation;

extern fixed_t  viewcos, viewsin;

extern boolean highDetail;

//
// Utility functions.
//

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
subsector_t __far* R_PointInSubsector(fixed_t x, fixed_t y);

void R_GetColumn(const texture_t __far* texture, int32_t texcolumn, int16_t* patch_num, int16_t* x_c);

const lighttable_t* R_LoadColorMap(int32_t lightlevel);

//
// REFRESH - the actual rendering functions.
//

void R_RenderPlayerView(player_t *player);   // Called by G_Drawer.

void R_DrawColumn (const draw_column_vars_t *dcvars);
void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars);

void R_DrawPlanes (void);
void R_ClearPlanes(void);

void R_DrawSky(visplane_t __far* pl);

#endif
