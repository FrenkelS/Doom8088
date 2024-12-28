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
 *      Video hardware specific code
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "r_defs.h"
#include "r_main.h"

void I_InitGraphicsHardwareSpecificCode(void);
void I_ShutdownGraphics(void);
void I_ReloadPalette(void);
void I_SetPalette(int8_t pal);
void I_FinishUpdate(void);


void R_DrawColumnSprite(const draw_column_vars_t *dcvars);
void R_DrawColumnWall(const draw_column_vars_t *dcvars);
void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars);
void R_DrawFuzzColumn (const draw_column_vars_t *dcvars);

#if !defined FLAT_SPAN
typedef struct {
  uint32_t             position;
  uint32_t             step;
  const byte    __far* source; // start of a 64*64 tile image
  const uint8_t* colormap;
} draw_span_vars_t;

void R_DrawSpan(uint16_t y, uint16_t x1, uint16_t x2, const draw_span_vars_t *dsvars);
#endif


void V_ClearViewWindow(void);
void V_InitDrawLine(void);
void V_ShutdownDrawLine(void);
void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);
void V_DrawBackground(int16_t backgroundnum);
void V_DrawRaw(int16_t num, uint16_t offset);

void ST_Drawer(void);

void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch);
void V_DrawPatchScaled(   int16_t x, int16_t y, const patch_t __far* patch);


void wipe_StartScreen(void);
void D_Wipe(void);

#endif
