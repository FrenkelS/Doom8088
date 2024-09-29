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
 *  Copyright 2023, 2024 by
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
 *  Refresh of things, i.e. objects represented by sprites.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "d_player.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "v_video.h"
#include "i_system.h"

#include "globdata.h"


// variables used to look up and range check thing_t sprites patches
spritedef_t __far* sprites;


static int8_t maxframe;

#define MAX_SPRITE_FRAMES 29
static spriteframe_t sprtemp[MAX_SPRITE_FRAMES];

static int16_t firstspritelump;
static int16_t numentries;


//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
//

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//

static void R_InstallSpriteLump(int16_t lump, uint8_t frame,
                                uint8_t rotation, boolean flipped)
{
  if (frame >= MAX_SPRITE_FRAMES || rotation > 8)
    I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

  if ((int8_t) frame > maxframe)
    maxframe = frame;


  if (rotation == 0)
  {    // the lump should be used for all rotations
      int8_t r;

      sprtemp[frame].flipmask = 0;

      for (r=0; r<8; r++)
      {
          if (sprtemp[frame].lump[r]==-1)
          {
              sprtemp[frame].lump[r] = lump;

              if(flipped)
                sprtemp[frame].flipmask |= (1 << r);

              sprtemp[frame].rotate = false; //jff 4/24/98 if any subbed, rotless
          }
      }
      return;
  }

  // the lump is only used for one rotation

  if (sprtemp[frame].lump[--rotation] == -1)
  {
      sprtemp[frame].lump[rotation] = lump;

      if(flipped)
        sprtemp[frame].flipmask |= (1 << rotation);
      else
        sprtemp[frame].flipmask &= (~(1 << rotation));

      sprtemp[frame].rotate = true; //jff 4/24/98 only change if rot used
  }
}

//
// R_InitSprites
// Pass a null terminated list of sprite names
// (4 chars exactly) to be used.
//
// Builds the sprite rotation matrixes to account
// for horizontally flipped sprites.
//
// Will report an error if the lumps are inconsistent.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
//
// A sprite that is flippable will have an additional
//  letter/number appended.
//
// The rotation character can be 0 to signify no rotations.
//
// 1/25/98, 1/31/98 killough : Rewritten for performance
//
// Empirically verified to have excellent hash
// properties across standard Doom sprites:

#define R_SpriteNameHash(s) ((uint16_t)((s)[0]-((s)[1]*3-(s)[3]*2-(s)[2])*2))

void R_InitSprites(void)
{
  struct { int16_t index, next; } __far* hash;
  int16_t i;

  if (!numentries || !*sprnames)
    return;

  sprites = Z_MallocStatic(NUMSPRITES *sizeof(*sprites));

  _fmemset(sprites, 0, NUMSPRITES *sizeof(*sprites));

  // Create hash table based on just the first four letters of each sprite
  // killough 1/31/98

  hash = Z_MallocStatic(sizeof(*hash)*numentries); // allocate hash table

  for (i=0; i<numentries; i++)             // initialize hash table as empty
    hash[i].index = -1;

  for (i=0; i<numentries; i++)             // Prepend each sprite to hash chain
    {                                      // prepend so that later ones win
      const char __far* sn = W_GetNameForNum(i + firstspritelump);

      int16_t j = R_SpriteNameHash(sn) % numentries;
      hash[i].next = hash[j].index;
      hash[j].index = i;
    }

  // scan all the lump names for each of the names,
  //  noting the highest frame letter.

  for (i=0 ; i<NUMSPRITES ; i++)
    {
      const char *spritename = sprnames[i];
      int16_t j = hash[R_SpriteNameHash(spritename) % numentries].index;

      if (j >= 0)
        {
          memset(sprtemp, -1, sizeof(sprtemp));
          maxframe = -1;
          do
            {
              const char __far* sn = W_GetNameForNum(j + firstspritelump);

              // Fast portable comparison -- killough
              // (using int32_t pointer cast is nonportable):

              if (!((sn[0] ^ spritename[0]) |
                    (sn[1] ^ spritename[1]) |
                    (sn[2] ^ spritename[2]) |
                    (sn[3] ^ spritename[3])))
                {
                  R_InstallSpriteLump(j + firstspritelump,
                                      sn[4] - 'A',
                                      sn[5] - '0',
                                      false);
                  if (sn[6])
                    R_InstallSpriteLump(j + firstspritelump,
                                        sn[6] - 'A',
                                        sn[7] - '0',
                                        true);
                }
            }
          while ((j = hash[j].next) >= 0);

          // check the frames that were found for completeness
          if ((sprites[i].numframes = ++maxframe))  // killough 1/31/98
            {
              int8_t frame;
              for (frame = 0; frame < maxframe; frame++)
                switch (sprtemp[frame].rotate)
                  {
                  default:
                    // no rotations were found for that frame at all
                    I_Error ("R_InitSprites: No patches found "
                             "for %.8s frame %c", sprnames[i], frame+'A');
                    break;

                  case false:
                    // only the first rotation is needed
                    break;

                  case true:
                    // must have all 8 frames
                    {
                      int8_t rotation;
                      for (rotation=0 ; rotation<8 ; rotation++)
                        if (sprtemp[frame].lump[rotation] == -1)
                          I_Error ("R_InitSprites: Sprite %.8s frame %c "
                                   "is missing rotations",
                                   sprnames[i], frame+'A');
                      break;
                    }
                  }
              // allocate space for the frames present and copy sprtemp to it
              sprites[i].spriteframes = Z_MallocStatic(maxframe * sizeof(spriteframe_t));
              _fmemcpy(sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
            }
        }
    }

  Z_Free(hash);           // free hash table
}


void R_InitSpriteLumps(void)
{
	firstspritelump        = W_GetNumForName("S_START") + 1;
	int16_t lastspritelump = W_GetNumForName("S_END")   - 1;

	numentries = lastspritelump - firstspritelump + 1;
}
