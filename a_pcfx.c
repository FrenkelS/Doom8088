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
   module: PCFX.C

   author: James R. Dose
   date:   April 1, 1994

   Low level routines to support PC sound effects created by Muse.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>
#include <stdint.h>
#include <stdlib.h>

#include "compiler.h"
#include "doomtype.h"
#include "doomdef.h"
#include "a_pcfx.h"
#include "a_taskmn.h"
#include "w_wad.h"

#define PCFX_PRIORITY 1

#define SND_TICRATE     140     // tic rate for updating sound


static uint16_t	data[146];
static int16_t	PCFX_LengthLeft;
static const uint16_t *PCFX_Sound = NULL;
static uint16_t	PCFX_LastSample = 0;

static boolean	PCFX_Installed = false;


/*---------------------------------------------------------------------
   Function: PCFX_Stop

   Halts playback of the currently playing sound effect.
---------------------------------------------------------------------*/

static void PCFX_Stop(void)
{
	if (PCFX_Sound == NULL)
		return;

	_disable();

	// Turn off speaker
	outp(0x61, inp(0x61) & 0xfc);

	PCFX_Sound      = NULL;
	PCFX_LastSample = 0;

	_enable();
}


/*---------------------------------------------------------------------
   Function: PCFX_Service

   Task Manager routine to perform the playback of a sound effect.
---------------------------------------------------------------------*/

static void PCFX_Service(void)
{
	if (PCFX_Sound)
	{
		uint16_t value = *PCFX_Sound++;

		if (value != PCFX_LastSample)
		{
			PCFX_LastSample = value;
			if (value)
			{
				outp(0x43, 0xb6);
				outp(0x42, LOBYTE(value));
				outp(0x42, HIBYTE(value));
				outp(0x61, inp(0x61) | 0x3);
			} else
				outp(0x61, inp(0x61) & 0xfc);
		}

		if (--PCFX_LengthLeft == 0)
		{
			// Turn off speaker
			outp(0x61, inp(0x61) & 0xfc);

			PCFX_Sound      = NULL;
			PCFX_LastSample = 0;
		}
	}
}


/*---------------------------------------------------------------------
   Function: PCFX_Play

   Starts playback of a Muse sound effect.
---------------------------------------------------------------------*/

typedef struct {
	uint16_t	length;
	uint16_t	data[];
} pcspkmuse_t;


void PCFX_Play(int16_t lumpnum)
{
	PCFX_Stop();

	const pcspkmuse_t __far* pcspkmuse = W_GetLumpByNum(lumpnum);
	PCFX_LengthLeft = pcspkmuse->length;
	_fmemcpy(data, pcspkmuse->data, pcspkmuse->length * sizeof(uint16_t));
	Z_ChangeTagToCache(pcspkmuse);

	_disable();

	PCFX_Sound = &data[0];

	_enable();
}


/*---------------------------------------------------------------------
   Function: PCFX_Init

   Initializes the sound effect engine.
---------------------------------------------------------------------*/

void PCFX_Init(void)
{
	if (PCFX_Installed)
		return;

	PCFX_Stop();
	TS_ScheduleTask(&PCFX_Service, SND_TICRATE, PCFX_PRIORITY);

	PCFX_Installed = true;
}


/*---------------------------------------------------------------------
   Function: PCFX_Shutdown

   Ends the use of the sound effect engine.
---------------------------------------------------------------------*/

void PCFX_Shutdown(void)
{
	if (PCFX_Installed)
	{
		PCFX_Stop();
		TS_Terminate(PCFX_PRIORITY);
		PCFX_Installed = false;
	}
}
