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
 *      Created by a sound utility.
 *      Kept as a sample, DOOM2 sounds.
 *
 *-----------------------------------------------------------------------------*/

// killough 5/3/98: reformatted

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
  { "none"  ,   0, NULL,  0 }, // S_sfx[0] needs to be a dummy for odd reasons.
  { "pistol",  64, NULL, 18 },
  { "shotgn",  64, NULL, 30 },
  { "sgcock",  64, NULL, 19 },
  { "sawup" ,  64, NULL, 52 },
  { "sawidl", 118, NULL, 24 },
  { "sawful",  64, NULL, 58 },
  { "sawhit",  64, NULL, 26 },
  { "rlaunc",  64, NULL, 49 },
  { "rxplod",  70, NULL, 46 },
  { "firsht",  70, NULL, 47 },
  { "firxpl",  70, NULL, 38 },
  { "pstart", 100, NULL, 26 },
  { "pstop" , 100, NULL, 21 },
  { "doropn", 100, NULL, 44 },
  { "dorcls", 100, NULL, 45 },
  { "stnmov", 119, NULL, 10 },
  { "swtchn",  78, NULL, 20 },
  { "swtchx",  78, NULL, 18 },
  { "plpain",  96, NULL, 48 },
  { "dmpain",  96, NULL, 31 },
  { "popain",  96, NULL, 28 },
  { "slop"  ,  78, NULL, 36 },
  { "itemup",  78, NULL,  7 },
  { "wpnup" ,  78, NULL, 19 },
  { "oof"   ,  96, NULL, 13 },
  { "telept",  32, NULL, 49 },
  { "posit1",  98, NULL, 17 },
  { "posit2",  98, NULL, 36 },
  { "posit3",  98, NULL, 35 },
  { "bgsit1",  98, NULL, 44 },
  { "bgsit2",  98, NULL, 52 },
  { "sgtsit",  98, NULL, 36 },
  { "brssit",  94, NULL, 44 },
  { "sgtatk",  70, NULL, 30 },
  { "claw"  ,  70, NULL, 21 },
  { "pldeth",  32, NULL, 35 },
  { "pdiehi",  32, NULL, 35 },
  { "podth1",  70, NULL, 41 },
  { "podth2",  70, NULL, 30 },
  { "podth3",  70, NULL, 35 },
  { "bgdth1",  70, NULL, 23 },
  { "bgdth2",  70, NULL, 30 },
  { "sgtdth",  70, NULL, 39 },
  { "brsdth",  32, NULL, 35 },
  { "posact", 120, NULL, 34 },
  { "bgact" , 120, NULL, 32 },
  { "dmact" , 120, NULL, 38 },
  { "noway" ,  78, NULL, 13 },
  { "barexp",  60, NULL, 59 },
  { "punch" ,  64, NULL,  8 },
  { "chgun" ,  64, &S_sfx[sfx_pistol], 18 },
  { "tink"  ,  60, NULL,  1 },
  { "bdopn" , 100, NULL, 14 },
  { "bdcls" , 100, NULL, 14 },
  { "itmbk" , 100, NULL, 18 },
  { "getpow",  60, NULL, 26 },
};
