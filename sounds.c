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
  { "none"  ,   0,  0 }, // S_sfx[0] needs to be a dummy for odd reasons.
  { "pistol",  64, 18 },
  { "shotgn",  64, 30 },
  { "sgcock",  64, 19 },
  { "sawup" ,  64, 52 },
  { "sawidl", 118, 24 },
  { "sawful",  64, 58 },
  { "sawhit",  64, 26 },
  { "rlaunc",  64, 49 },
  { "rxplod",  70, 46 },
  { "firsht",  70, 47 },
  { "firxpl",  70, 38 },
  { "pstart", 100, 26 },
  { "pstop" , 100, 21 },
  { "doropn", 100, 44 },
  { "dorcls", 100, 45 },
  { "stnmov", 119, 10 },
  { "swtchn",  78, 20 },
  { "swtchx",  78, 18 },
  { "plpain",  96, 48 },
  { "dmpain",  96, 31 },
  { "popain",  96, 28 },
  { "slop"  ,  78, 36 },
  { "itemup",  78,  7 },
  { "wpnup" ,  78, 19 },
  { "oof"   ,  96, 13 },
  { "telept",  32, 49 },
  { "posit1",  98, 17 },
  { "posit2",  98, 36 },
  { "posit3",  98, 35 },
  { "bgsit1",  98, 44 },
  { "bgsit2",  98, 52 },
  { "sgtsit",  98, 36 },
  { "brssit",  94, 44 },
  { "sgtatk",  70, 30 },
  { "claw"  ,  70, 21 },
  { "pldeth",  32, 35 },
  { "pdiehi",  32, 35 },
  { "podth1",  70, 41 },
  { "podth2",  70, 30 },
  { "podth3",  70, 35 },
  { "bgdth1",  70, 23 },
  { "bgdth2",  70, 30 },
  { "sgtdth",  70, 39 },
  { "brsdth",  32, 35 },
  { "posact", 120, 34 },
  { "bgact" , 120, 32 },
  { "dmact" , 120, 38 },
  { "noway" ,  78, 13 },
  { "barexp",  60, 59 },
  { "punch" ,  64,  8 },
  { "chgun" ,  64, 18 },
  { "tink"  ,  60,  1 },
  { "bdopn" , 100, 14 },
  { "bdcls" , 100, 14 },
  { "itmbk" , 100, 18 },
  { "getpow",  60, 26 },
};
