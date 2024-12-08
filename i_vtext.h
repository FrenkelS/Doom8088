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
 *      Text mode specific code
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_TEXT_MODE__
#define __I_TEXT_MODE__

void V_DrawCharacter(int16_t x, int16_t y, uint8_t color, char c);
void V_DrawSTCharacter(int16_t x, int16_t y, uint8_t color, char c);
void V_DrawCharacterForeground(int16_t x, int16_t y, uint8_t color, char c);
void V_DrawString(int16_t x, int16_t y, uint8_t color, const char* s);
void V_DrawSTString(int16_t x, int16_t y, uint8_t color, const char* s);
void V_SetSTPalette(void);
void V_ClearString(int16_t y, size_t len);
void I_InitScreenPage(void);
void I_InitScreenPages(void);

#endif
