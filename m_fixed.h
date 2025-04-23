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
 *  Copyright 2023-2025 by
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
 *      Fixed point arithemtics, implementation.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __M_FIXED__
#define __M_FIXED__

#include "config.h"
#include "doomtype.h"


/*
 * Fixed point, 32bit as 16.16.
 */

#define FRACBITS 16
#define FRACUNIT (1L<<FRACBITS)


typedef int32_t fixed_t;

/*
 * Absolute Value
 *
 */

#include <stdlib.h>
#define D_abs labs


/*
 * Fixed Point Multiplication
 */

fixed_t CONSTFUNC FixedMul(fixed_t a, fixed_t b);
fixed_t CONSTFUNC FixedMulAngle(fixed_t a, fixed_t b);
fixed_t CONSTFUNC FixedMul3216(fixed_t a, uint16_t blw);


//Approx Reciprocal of v
// Divide FFFFFFFFh by a number.
#if defined C_ONLY
#define FixedReciprocal(v)      (0xffffffffu/(v))
#define FixedReciprocalBig(v)   (0xffffffffu/(v))
#define FixedReciprocalSmall(v) (0xffffffffu/(uint16_t)(v))
#else
fixed_t  CONSTFUNC FixedReciprocal(fixed_t v);
uint16_t CONSTFUNC FixedReciprocalBig(fixed_t v);
fixed_t  CONSTFUNC FixedReciprocalSmall(uint16_t v);
#endif


/*
 * Fixed Point Division approximation
 */

fixed_t CONSTFUNC FixedApproxDiv(fixed_t a, fixed_t b);

#endif
