//
//
// Copyright (C) 2023-2025 Frenkel Smeijers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __COMPILER__
#define __COMPILER__

#if defined _M_I86
//16-bit
#include <i86.h>

#define D_MK_FP  MK_FP
#define D_FP_SEG FP_SEG
#if defined __WATCOMC__
#define D_FP_OFF FP_OFF
#else
#define D_FP_OFF(p) ((uint16_t)((uint32_t)(p)))
#endif

typedef uint16_t segment_t;
#define SIZE_OF_SEGMENT_T 2

#else
//32-bit
#define D_MK_FP(s,o) (void*)((s<<4)+o)
#define D_FP_SEG(p)  (((uint32_t)p)>>4)
#define D_FP_OFF(p)  (((uint32_t)p)&15)

typedef uint32_t segment_t;
#define SIZE_OF_SEGMENT_T 4

#define __far

#define _fmemchr	memchr
#define _fmemcmp	memcmp
#define _fmemcpy	memcpy
#define _fmemset	memset
#define _fstrcpy	strcpy
#define _fstrlen	strlen

#endif



#if defined __IA16_SYS_MSDOS
//gcc-ia16
#define _chain_intr(func) func()
#endif



#if defined __DJGPP__
//DJGPP
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>

//DJGPP doesn't inline inp, outp and outpw,
//but it does inline inportb, outportb and outportw
#define inp(port)			inportb(port)
#define outp(port,data)		outportb(port,data)

#define __interrupt

#define replaceInterrupt(OldInt,NewInt,vector,handler)				\
_go32_dpmi_get_protected_mode_interrupt_vector(vector, &OldInt);	\
																	\
NewInt.pm_selector = _go32_my_cs(); 								\
NewInt.pm_offset = (int32_t)handler;								\
_go32_dpmi_allocate_iret_wrapper(&NewInt);							\
_go32_dpmi_set_protected_mode_interrupt_vector(vector, &NewInt)

#define restoreInterrupt(vector,OldInt,NewInt)						\
_go32_dpmi_set_protected_mode_interrupt_vector(vector, &OldInt);	\
_go32_dpmi_free_iret_wrapper(&NewInt);

#define _chain_intr(OldInt)		\
asm								\
(								\
	"cli \n"					\
	"pushfl \n"					\
	"lcall *%0"					\
	:							\
	: "m" (OldInt.pm_offset)	\
)



#else
//Watcom and gcc-ia16
#define __djgpp_nearptr_enable()
#define __djgpp_conventional_base 0

#if defined _M_I386
#define int86 int386
#endif

#define replaceInterrupt(OldInt,NewInt,vector,handler)	\
OldInt = _dos_getvect(vector);							\
_dos_setvect(vector, handler)

#define restoreInterrupt(vector,OldInt,NewInt)	_dos_setvect(vector,OldInt)



#endif

#endif
