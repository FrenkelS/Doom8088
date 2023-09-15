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
 * DESCRIPTION:  Head up display
 *
 *-----------------------------------------------------------------------------*/

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__


#define HU_FONT_HEIGHT       7
#define HU_FONT_SPACE_WIDTH  4

#define HU_FONTSTART    '!'     /* the first font characters */
#define HU_FONTEND      '_' /*jff 2/16/98 '_' the last font characters */

#define HU_FONTSTART_LUMP "STCFN033"

/* Calculate # of glyphs in font. */
#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1)


#define HU_MAXLINELENGTH  31

/* Text Line widget
 *  (parent of Scrolling Text and Input Text widgets) */
typedef struct
{
  // left-justified position of scrolling text window
  int16_t   x;
  int16_t   y;

  int16_t   linelen;
  char  l[HU_MAXLINELENGTH+1]; // line of text
  int16_t   len;                            // current line length

  // whether this line needs to be updated
  int16_t   needsupdate;

} hu_textline_t;


// Scrolling Text window widget
//  (child of Text Line widget)
typedef struct
{
  hu_textline_t l; // text line to draw

  // pointer to boolean stating whether to update window
  boolean*    on;
  boolean   laston;             // last value of *->on.

} hu_stext_t;


/*
 * Heads up text
 */
void HU_Init(void);
void HU_Start(void);
void HU_Ticker(void);
void HU_Drawer(void);
void HU_Erase(void);

#endif
