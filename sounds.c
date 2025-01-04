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
 *      Created by a sound utility.
 *      Kept as a sample, DOOM2 sounds.
 *
 *-----------------------------------------------------------------------------*/

#include <stddef.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomtype.h"
#include "sounds.h"

//
// Information about all the sfx
//

const sfxinfo_t S_sfx[] = {
  { 0,   0,  0 }, // none   S_sfx[0] needs to be a dummy for odd reasons.
  { 0,  64, 18 }, // pistol
  { 0,  64, 30 }, // shotgn
  { 0,  64, 19 }, // sgcock
  { 0,  64, 52 }, // sawup
  { 0, 118, 24 }, // sawidl
  { 0,  64, 58 }, // sawful
  { 0,  64, 26 }, // sawhit
  { 0,  64, 49 }, // rlaunc
  { 0,  70, 46 }, // rxplod
  { 0,  70, 47 }, // firsht
  { 0,  70, 38 }, // firxpl
  { 0, 100, 26 }, // pstart
  { 0, 100, 21 }, // pstop
  { 0, 100, 44 }, // doropn
  { 0, 100, 45 }, // dorcls
  { 0, 119, 10 }, // stnmov
  { 0,  78, 20 }, // swtchn
  { 0,  78, 18 }, // swtchx
  { 0,  96, 48 }, // plpain
  { 0,  96, 31 }, // dmpain
  { 0,  96, 28 }, // popain
  { 0,  78, 36 }, // slop
  { 0,  78,  7 }, // itemup
  { 0,  78, 19 }, // wpnup
  { 0,  96, 13 }, // oof
  { 0,  32, 49 }, // telept
  { 0,  98, 17 }, // posit1
  { 0,  98, 36 }, // posit2
  { 0,  98, 35 }, // posit3
  { 0,  98, 44 }, // bgsit1
  { 0,  98, 52 }, // bgsit2
  { 0,  98, 36 }, // sgtsit
  { 0,  94, 44 }, // brssit
  { 0,  70, 30 }, // sgtatk
  { 0,  70, 21 }, // claw
  { 0,  32, 35 }, // pldeth
  { 0,  32, 35 }, // pdiehi
  { 0,  70, 41 }, // podth1
  { 0,  70, 30 }, // podth2
  { 0,  70, 35 }, // podth3
  { 0,  70, 23 }, // bgdth1
  { 0,  70, 30 }, // bgdth2
  { 0,  70, 39 }, // sgtdth
  { 0,  32, 35 }, // brsdth
  { 0, 120, 34 }, // posact
  { 0, 120, 32 }, // bgact
  { 0, 120, 38 }, // dmact
  { 0,  78, 13 }, // noway
  { 0,  60, 59 }, // barexp
  { 0,  64,  8 }, // punch
  { 1,  64, 18 }, // chgun
  { 0,  60,  1 }, // tink 
  { 0,  60, 26 }, // getpow
};
