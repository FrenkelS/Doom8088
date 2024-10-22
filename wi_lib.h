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
 *  Intermission screens.
 *  Text mode version
 *
 *-----------------------------------------------------------------------------*/

#ifndef __WI_LIB__
#define __WI_LIB__

void WI_drawLF(int16_t map);
void WI_drawEL(int16_t map);

int16_t WI_getColonWidth(void);
void WI_drawColon(int16_t x, int16_t y);
void WI_drawSucks(int16_t x, int16_t y);
void WI_drawPercentSign(int16_t x, int16_t y);

int16_t WI_drawNum(int16_t x, int16_t y, int16_t n, int16_t digits);
void WI_drawStats(int16_t cnt_kills, int16_t cnt_items, int16_t cnt_secret, int32_t cnt_time, int32_t cnt_total_time, int16_t cnt_par);

void WI_drawSplat(int16_t i);
void WI_drawYouAreHere(int16_t i);

void WI_drawPercent(int16_t x, int16_t y, int16_t p);
void WI_drawTime(int16_t x, int16_t y, int32_t t);

#endif
