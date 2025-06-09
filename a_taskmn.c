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
   module: TASK_MAN.C

   author: James R. Dose
   date:   July 25, 1994

   Low level timer task scheduler.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include "doomtype.h"
#include "compiler.h"
#include "doomdef.h"
#include "a_taskmn.h"


/*---------------------------------------------------------------------
   Global variables
---------------------------------------------------------------------*/

typedef struct
{
	void				(*taskService)(void);
	int32_t				rate;
	volatile int32_t	count;
	boolean				active;
} task_t;


#define MAX_TASKS 2
#define MAX_SERVICE_RATE 0x10000L

static task_t tasks[MAX_TASKS];


#if defined __DJGPP__
static _go32_dpmi_seginfo OldInt8, NewInt8;
#else
static void __interrupt __far (*OldInt8)(void);
#endif


static volatile int32_t taskServiceRate  = MAX_SERVICE_RATE;
static volatile int32_t taskServiceCount = 0;

static boolean isTS_Installed = false;


/*---------------------------------------------------------------------
   Function: TS_SetClockSpeed

   Sets the rate of the 8253 timer.
---------------------------------------------------------------------*/

static void TS_SetClockSpeed(int32_t speed)
{
	_disable();

	if (0 < speed && speed < MAX_SERVICE_RATE)
		taskServiceRate = speed;
	else
		taskServiceRate = MAX_SERVICE_RATE;

	outp(0x43, 0x36);
	outp(0x40, LOBYTE(taskServiceRate));
	outp(0x40, HIBYTE(taskServiceRate));

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_ServiceSchedule

   Interrupt service routine
---------------------------------------------------------------------*/

static void __interrupt __far TS_ServiceSchedule (void)
{
	for (int16_t i = 0; i < MAX_TASKS; i++)
	{
		task_t *task = &tasks[i];
		if (task->active)
		{
			task->count += taskServiceRate;

			while (task->count >= task->rate)
			{
				task->count -= task->rate;
				task->taskService();
			}
		}
	}

	taskServiceCount += taskServiceRate;
	if (taskServiceCount > 0xffffL)
	{
		taskServiceCount &= 0xffff;
		_chain_intr(OldInt8);
	} else {
		outp(0x20, 0x20);
	}
}


/*---------------------------------------------------------------------
   Function: TS_Startup

   Sets up the task service routine.
---------------------------------------------------------------------*/

#define TIMERINT 8

static void TS_Startup(void)
{
	if (!isTS_Installed)
	{
		taskServiceRate  = MAX_SERVICE_RATE;
		taskServiceCount = 0;

		replaceInterrupt(OldInt8, NewInt8, TIMERINT, TS_ServiceSchedule);

		isTS_Installed = true;
	}
}


/*---------------------------------------------------------------------
   Function: TS_Shutdown

   Ends processing of all tasks.
---------------------------------------------------------------------*/

void TS_Shutdown(void)
{
	if (isTS_Installed)
	{
		TS_SetClockSpeed(0);

		restoreInterrupt(TIMERINT, OldInt8, NewInt8);

		isTS_Installed = false;
	}
}


/*---------------------------------------------------------------------
   Function: TS_SetTimer

   Calculates the rate at which a task will occur and sets the clock
   speed if necessary.
---------------------------------------------------------------------*/

static uint16_t TS_SetTimer(int16_t tickBase)
{
	uint16_t speed = 1193182L / tickBase;
	if (speed < taskServiceRate)
		TS_SetClockSpeed(speed);

	return speed;
}


/*---------------------------------------------------------------------
   Function: TS_ScheduleTask

   Schedules a new task for processing.
---------------------------------------------------------------------*/

void TS_ScheduleTask(void (*function)(void), int16_t rate, int16_t priority)
{
	if (!isTS_Installed)
		TS_Startup();

	tasks[priority].taskService = function;
	tasks[priority].rate        = TS_SetTimer(rate);
	tasks[priority].count       = 0;
	tasks[priority].active      = false;


	_disable();

	tasks[priority].active = true;

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_SetTimerToMaxTaskRate

   Finds the fastest running task and sets the clock to operate at
   that speed.
---------------------------------------------------------------------*/

static void TS_SetTimerToMaxTaskRate(void)
{
	_disable();

	int32_t maxServiceRate = MAX_SERVICE_RATE;

	for (int16_t i = 0; i < MAX_TASKS; i++)
		if (tasks[i].rate < maxServiceRate)
			maxServiceRate = tasks[i].rate;

	if (taskServiceRate != maxServiceRate)
		TS_SetClockSpeed(maxServiceRate);

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_Terminate

   Ends processing of a specific task.
---------------------------------------------------------------------*/

void TS_Terminate(int16_t priority)
{
	_disable();

	tasks[priority].rate = MAX_SERVICE_RATE;

	TS_SetTimerToMaxTaskRate();

	_enable();
}
