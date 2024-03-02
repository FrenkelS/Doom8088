/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2023-2024 Frenkel Smeijers

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

#define PCFX_PRIORITY 1

#define SND_TICRATE     140     // tic rate for updating sound

static int16_t	PCFX_LengthLeft;
static uint8_t	*PCFX_Sound = NULL;
static uint16_t	PCFX_LastSample;

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
	PCFX_LengthLeft = 0;
	PCFX_LastSample = 0;

	_enable();
}


/*---------------------------------------------------------------------
   Function: PCFX_Service

   Task Manager routine to perform the playback of a sound effect.
---------------------------------------------------------------------*/

static void PCFX_Service(void)
{
	uint16_t value;

	if (PCFX_Sound)
	{
		value = *(uint16_t *)PCFX_Sound;
		PCFX_Sound += sizeof(uint16_t);

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
			PCFX_Stop();
	}
}


/*---------------------------------------------------------------------
   Function: PCFX_Play

   Starts playback of a Muse sound effect.
---------------------------------------------------------------------*/

typedef	struct
{
	uint16_t	length;
	uint8_t		data[];
} PCSound;

#define PCFX_MinVoiceHandle 1
static int16_t	PCFX_VoiceHandle = PCFX_MinVoiceHandle;

static int16_t ASS_PCFX_Play(PCSound *sound)
{
	PCFX_Stop();

	PCFX_VoiceHandle++;
	if (PCFX_VoiceHandle < PCFX_MinVoiceHandle)
		PCFX_VoiceHandle = PCFX_MinVoiceHandle;

	_disable();

	PCFX_LengthLeft = sound->length;
	PCFX_Sound = &sound->data[0];

	_enable();

	return PCFX_VoiceHandle;
}

static const uint16_t divisors[] = {
	0,
	6818, 6628, 6449, 6279, 6087, 5906, 5736, 5575,
	5423, 5279, 5120, 4971, 4830, 4697, 4554, 4435,
	4307, 4186, 4058, 3950, 3836, 3728, 3615, 3519,
	3418, 3323, 3224, 3131, 3043, 2960, 2875, 2794,
	2711, 2633, 2560, 2485, 2415, 2348, 2281, 2213,
	2153, 2089, 2032, 1975, 1918, 1864, 1810, 1757,
	1709, 1659, 1612, 1565, 1521, 1478, 1435, 1395,
	1355, 1316, 1280, 1242, 1207, 1173, 1140, 1107,
	1075, 1045, 1015,  986,  959,  931,  905,  879,
	 854,  829,  806,  783,  760,  739,  718,  697,
	 677,  658,  640,  621,  604,  586,  570,  553,
	 538,  522,  507,  493,  479,  465,  452,  439,
	 427,  415,  403,  391,  380,  369,  359,  348,
	 339,  329,  319,  310,  302,  293,  285,  276,
	 269,  261,  253,  246,  239,  232,  226,  219,
	 213,  207,  201,  195,  190,  184,  179,
};

typedef struct {
	uint16_t	length;
	uint16_t	data[0x92];
} pcspkmuse_t;

static pcspkmuse_t pcspkmuse;

typedef struct {
	uint16_t	type; // 0 = PC Speaker
	uint16_t	length;
	uint8_t		data[];
} dmxpcs_t;

int16_t PCFX_Play(const void __far* vdata)
{
	dmxpcs_t __far* dmxpcs = (dmxpcs_t __far* )vdata;
	static segment_t soundsegment = 0;

	if (soundsegment != D_FP_SEG(vdata))
	{
		pcspkmuse.length = dmxpcs->length;
		for (uint_fast16_t i = 0; i < dmxpcs->length; i++)
			pcspkmuse.data[i] = divisors[dmxpcs->data[i]];
		soundsegment = D_FP_SEG(vdata);
	}

	return ASS_PCFX_Play((PCSound *)&pcspkmuse);
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
	TS_Dispatch();

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
