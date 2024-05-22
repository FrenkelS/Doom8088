;
; Copyright (C) 2024 Frenkel Smeijers
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <https://www.gnu.org/licenses/>.
;

cpu 8086

bits 16


extern XMSControl


; Allocate Extended Memory Block
;
; input:
;   ax = size in kibibytes
;
; output:
;   ax = handle

global Z_AllocateExtendedMemoryBlock
Z_AllocateExtendedMemoryBlock:
	mov		dx, ax
	mov		ah, 09h
	call	far [XMSControl]
	mov		ax, dx
	retf


; Free Extended Memory Block
;
; input:
;   ax = handle

global Z_FreeExtendedMemoryBlock
Z_FreeExtendedMemoryBlock:
	mov		dx, ax
	mov		ah, 0Ah
	call	far [XMSControl]
	retf


; Move Extended Memory Block
;
; input:
;   dx:ax = far pointer to ExtMemMoveStruct

global Z_MoveExtendedMemoryBlock
Z_MoveExtendedMemoryBlock:
	push	si

	mov		si, ax
	mov		ds, dx
	mov		ah, 0Bh
	call	far [XMSControl]

	pop		si

	push	ss
	pop		ds

	retf
