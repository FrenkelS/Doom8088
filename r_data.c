/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2002 by
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
 *      Preparation of data for rendering,
 *      generation of lookups, caching, retrieval by name.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "m_fixed.h"
#include "i_system.h"
#include "r_things.h"
#include "p_tick.h"
#include "p_tick.h"

#include "globdata.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//

//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//

typedef PACKEDATTR_PRE struct
{
  int16_t originx;
  int16_t originy;
  int16_t patch;
  int16_t stepdir;         // unused in Doom but might be used in Phase 2 Boom
  int16_t colormap;        // unused in Doom but might be used in Phase 2 Boom
} PACKEDATTR_POST mappatch_t;


typedef PACKEDATTR_PRE struct
{
  char       name[8];      // unused
  char       pad2[4];      // unused
  int16_t      width;
  int16_t      height;
  char       pad[4];       // unused in Doom but might be used in Boom Phase 2
  int16_t      patchcount;
  mappatch_t patches[1];
} PACKEDATTR_POST maptexture_t;

// A maptexture_t describes a rectangular texture, which is composed
// of one or more mappatch_t structures that arrange graphic patches.

static const texture_t **textures;

static void R_LoadTexture(int16_t texture_num)
{
    const byte    *pnames = W_GetLumpByName("PNAMES");
    const int32_t *maptex = W_GetLumpByName("TEXTURE1");
    const int32_t *directory = maptex+1;

    const maptexture_t *mtexture = (const maptexture_t *) ((const byte *)maptex + directory[texture_num]);

    texture_t* texture = Z_MallocLevel(sizeof(const texture_t) + sizeof(const texpatch_t)*(mtexture->patchcount-1), (void**)&textures[texture_num]);

    texture->width      = mtexture->width;
    texture->height     = mtexture->height;
    texture->patchcount = mtexture->patchcount;
    //texture->name       = mtexture->name;
    int16_t w = 1;
    while (w * 2 <= texture->width)
        w <<= 1;
    texture->widthmask  = w - 1;


    texpatch_t* patch = texture->patches;
    const mappatch_t* mpatch = mtexture->patches;

    texture->overlapped = false;

    //Skip to list of names.
    pnames += 4;

    for (uint8_t j = 0; j < texture->patchcount; j++, mpatch++, patch++)
    {
        patch->originx = mpatch->originx;
        patch->originy = mpatch->originy;

        char pname[8];
        strncpy(pname, (const char*)&pnames[mpatch->patch * 8], 8);

        patch->patch_num = W_GetNumForName(pname);
    }

    pnames -= 4;
    Z_Free(pnames);
    Z_Free(maptex);

    for (uint8_t j = 0; j < texture->patchcount; j++)
    {
        const texpatch_t* patch = &texture->patches[j];

        //Check for patch overlaps.
        int16_t l1 = patch->originx;
        int16_t r1 = l1 + V_NumPatchWidth(patch->patch_num);

        for (uint8_t k = j + 1; k < texture->patchcount; k++)
        {
            if (k == j)
                continue;

            const texpatch_t* p2 = &texture->patches[k];

            //Check for patch overlaps.
            int16_t l2 = p2->originx;
            int16_t r2 = l2 + V_NumPatchWidth(p2->patch_num);

            if (r1 > l2 && l1 < r2)
            {
                texture->overlapped = true;
                break;
            }
        }

        if (texture->overlapped)
            break;
    }

    textureheight[texture_num] = ((int32_t)texture->height) << FRACBITS;

    texturetranslation[texture_num] = texture_num;

    textures[texture_num] = texture;
}

static int16_t numtextures;

const texture_t* R_GetTexture(int16_t texture)
{
#ifdef RANGECHECK
    if (texture >= numtextures)
        I_Error("R_GetTexture: Texture %d not in range.", texture);
#endif
texture = 46; //FIXME not everything has to be the same texture
    if (!textures[texture])
        R_LoadTexture(texture);

    return textures[texture];
}

static int16_t R_GetTextureNumForName(const char* tex_name)
{
    //Convert name to uppercase for comparison.
    char tex_name_upper[9];

    strncpy(tex_name_upper, tex_name, 8);
    tex_name_upper[8] = 0; //Ensure null terminated.

    strupr(tex_name_upper);

    const int32_t *maptex = W_GetLumpByName("TEXTURE1");
    const int32_t *directory = maptex+1;

    for (int16_t i = 0; i < numtextures; i++)
    {
        int32_t offset = *directory++;

        const maptexture_t* mtexture = (const maptexture_t *) ( (const byte *)maptex + offset);

        if (!strncmp(tex_name_upper, mtexture->name, 8))
        {
            Z_Free(maptex);
            return i;
        }
    }

    I_Error("R_GetTextureNumForName: texture name: %s not found.", tex_name);
    return -1;
}




//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
// Rewritten by Lee Killough to use hash table for fast lookup. Considerably
// reduces the time needed to start new levels. See w_wad.c for comments on
// the hashing algorithm, which is also used for lump searches.
//
// killough 1/21/98, 1/31/98
//

#define NO_TEXTURE 0

int16_t PUREFUNC R_CheckTextureNumForName (const char *tex_name)
{
    // "NoTexture" marker.
    if (tex_name[0] == '-')
        return NO_TEXTURE;

    return R_GetTextureNumForName(tex_name);
}


//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//

static void R_InitTextures()
{
	const int32_t* mtex1 = W_GetLumpByName("TEXTURE1");
	numtextures = *mtex1;
	Z_Free(mtex1);

	textures = Z_MallocStatic(numtextures*sizeof*textures);
	memset(textures, 0, numtextures*sizeof*textures);

	textureheight = Z_MallocStatic(numtextures*sizeof*textureheight);
	memset(textureheight, 0, numtextures*sizeof*textureheight);

	texturetranslation = Z_MallocStatic((numtextures + 1)*sizeof*texturetranslation);

	for (int16_t i = 0; i < numtextures; i++)
		texturetranslation[i] = i;
}


//
// R_InitColormaps
//
static void R_InitColormaps (void)
{
	colormaps = W_GetLumpByName("COLORMAP"); // Never freed
}

//
// R_Init
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//

void R_Init(void)
{
  R_InitTextures();
  R_InitFlats();
  R_InitSky();
  R_InitSpriteLumps();
  R_InitColormaps();                    // killough 3/20/98
}
