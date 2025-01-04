/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2023-2025 Frenkel Smeijers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/**********************************************************************
   module: PCFX.H

   author: James R. Dose
   date:   April 1, 1994

   Public header for PCFX.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __PCFX_H
#define __PCFX_H

typedef struct {
	uint16_t	type; // 0 = PC Speaker
	uint16_t	length;
	uint8_t		data[];
} dmxpcs_t;


void	PCFX_Play(uint16_t length, const uint16_t __far* data);
void	PCFX_Init(void);
void	PCFX_Shutdown(void);

uint16_t __far*	PCFX_Convert(const dmxpcs_t __far* dmxpcs);

#endif
