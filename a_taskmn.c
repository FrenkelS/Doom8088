/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2023 Frenkel Smeijers

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
#include <malloc.h>
#include <stdint.h>

#include "doomtype.h"
#include "compiler.h"
#include "a_taskmn.h"


/*---------------------------------------------------------------------
   Global variables
---------------------------------------------------------------------*/

static task HeadTask;
static task *TaskList = &HeadTask;

#if defined __DJGPP__
static _go32_dpmi_seginfo OldInt8, NewInt8;
#else
static void (__interrupt *OldInt8)(void);
#endif

static volatile int32_t TaskServiceRate  = 0x10000L;
static volatile int32_t TaskServiceCount = 0;

static boolean TS_Installed = false;


/*---------------------------------------------------------------------
   Function: TS_FreeTaskList

   Terminates all tasks and releases any memory used for control
   structures.
---------------------------------------------------------------------*/

static void TS_FreeTaskList(void)
{
	_disable();

	task *node = TaskList->next;
	while (node != TaskList)
	{
		task *next = node->next;
		free(node);
		node = next;
	}

	TaskList->next = TaskList;
	TaskList->prev = TaskList;

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_SetClockSpeed

   Sets the rate of the 8253 timer.
---------------------------------------------------------------------*/

#define LOBYTE(w)	(((uint8_t *)&w)[0])
#define HIBYTE(w)	(((uint8_t *)&w)[1])

static void TS_SetClockSpeed(int32_t speed)
{
	_disable();

	if (0 < speed && speed < 0x10000L)
		TaskServiceRate = speed;
	else
		TaskServiceRate = 0x10000L;

	outp(0x43, 0x36);
	outp(0x40, LOBYTE(TaskServiceRate));
	outp(0x40, HIBYTE(TaskServiceRate));

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_SetTimer

   Calculates the rate at which a task will occur and sets the clock
   speed if necessary.
---------------------------------------------------------------------*/

static int32_t TS_SetTimer(int32_t TickBase)
{
	int32_t speed = 1193182L / TickBase;
	if (speed < TaskServiceRate)
		TS_SetClockSpeed(speed);

	return speed;
}


/*---------------------------------------------------------------------
   Function: TS_SetTimerToMaxTaskRate

   Finds the fastest running task and sets the clock to operate at
   that speed.
---------------------------------------------------------------------*/

static void TS_SetTimerToMaxTaskRate(void)
{
	_disable();

	int32_t MaxServiceRate = 0x10000L;

	task *ptr = TaskList->next;
	while (ptr != TaskList)
	{
		if (ptr->rate < MaxServiceRate)
			MaxServiceRate = ptr->rate;

		ptr = ptr->next;
	}

	if (TaskServiceRate != MaxServiceRate)
		TS_SetClockSpeed(MaxServiceRate);

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_ServiceSchedule

   Interrupt service routine
---------------------------------------------------------------------*/

static void __interrupt TS_ServiceSchedule (void)
{
	task *ptr = TaskList->next;
	while (ptr != TaskList)
	{
		task *next = ptr->next;

		if (ptr->active)
		{
			ptr->count += TaskServiceRate;

			while (ptr->count >= ptr->rate)
			{
				ptr->count -= ptr->rate;
				ptr->TaskService(ptr);
			}
		}
		ptr = next;
	}

	TaskServiceCount += TaskServiceRate;
	if (TaskServiceCount > 0xffffL)
	{
		TaskServiceCount &= 0xffff;
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
	if (!TS_Installed)
	{
		TaskList->next = TaskList;
		TaskList->prev = TaskList;

		TaskServiceRate  = 0x10000L;
		TaskServiceCount = 0;

		replaceInterrupt(OldInt8, NewInt8, TIMERINT, TS_ServiceSchedule);

		TS_Installed = true;
	}
}


/*---------------------------------------------------------------------
   Function: TS_Shutdown

   Ends processing of all tasks.
---------------------------------------------------------------------*/

void TS_Shutdown(void)
{
	if (TS_Installed)
	{
		TS_FreeTaskList();

		TS_SetClockSpeed(0);

		restoreInterrupt(TIMERINT, OldInt8, NewInt8);

		TS_Installed = false;
	}
}


/*---------------------------------------------------------------------
   Function: TS_AddTask

   Adds a new task to our list of tasks.
---------------------------------------------------------------------*/

static void TS_AddTask(task *node)
{
	task *ptr = TaskList->next;
	while ((ptr != TaskList) && (node->priority > ptr->priority))
		ptr = ptr->next;

	node->next = ptr;
	node->prev = ptr->prev;
	ptr->prev->next = node;
	ptr->prev = node;
}


/*---------------------------------------------------------------------
   Function: TS_ScheduleTask

   Schedules a new task for processing.
---------------------------------------------------------------------*/

task *TS_ScheduleTask(void (*Function)(task *), int16_t rate, int16_t priority, int16_t taskId)
{
	task *ptr = malloc(sizeof(task));
	if (ptr != NULL)
	{
		if (!TS_Installed)
			TS_Startup();

		ptr->TaskService = Function;
		ptr->taskId      = taskId;
		ptr->rate        = TS_SetTimer(rate);
		ptr->count       = 0;
		ptr->priority    = priority;
		ptr->active      = false;

		TS_AddTask(ptr);
	}

	return ptr;
}


/*---------------------------------------------------------------------
   Function: TS_Terminate

   Ends processing of a specific task.
---------------------------------------------------------------------*/

void TS_Terminate(task *NodeToRemove)
{
	_disable();

	task *ptr = TaskList->next;
	while (ptr != TaskList)
	{
		task *next = ptr->next;

		if (ptr == NodeToRemove)
		{
			NodeToRemove->prev->next = NodeToRemove->next;
			NodeToRemove->next->prev = NodeToRemove->prev;
			NodeToRemove->next = NULL;
			NodeToRemove->prev = NULL;
			free(NodeToRemove);

			TS_SetTimerToMaxTaskRate();

			break;
		}

		ptr = next;
	}

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_Dispatch

   Begins processing of all inactive tasks.
---------------------------------------------------------------------*/

void TS_Dispatch(void)
{
	_disable();

	task *ptr = TaskList->next;
	while (ptr != TaskList)
	{
		ptr->active = true;
		ptr = ptr->next;
	}

	_enable();
}
