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

%ifidn CPU, i8088
cpu 8086
%elifidn CPU, i286
cpu 286
%else
%error unsupported cpu CPU
%endif

bits 16

PLANEWIDTH equ 60

extern dest

last_pixel_jump_table:
	dw last_pixel0,
	dw last_pixel1,
	dw last_pixel2,
	dw last_pixel3,
	dw last_pixel4,
	dw last_pixel5,
	dw last_pixel6,
	dw last_pixel7

;
; input:
;   al = color
;   dx = yl
;   cx = count		1 <= count <= 128	=>	ch = 0
;

global R_DrawColumnFlat2
R_DrawColumnFlat2:
	push di

	lds di, [dest]					; ds:di = dest

	mov bx, cx						; bx = count
	and bl, 7						; 0 <= count <= 7
	shl bl, 1

	mov ah, al
%ifidn CPU, i8088
	ror ah, 1
	ror ah, 1
	ror ah, 1
	ror ah, 1						; ah = al with nibbles swapped

	shr cx, 1
	shr cx, 1
	shr cx, 1						; 0 <= cx <= 16
%else
	ror ah, 4						; ah = al with nibbles swapped

	shr cx, 3						; 0 <= cx <= 16
%endif

	shr dx, 1						; if yl is odd
	jnc label_a						;  then jump to label_a
	xchg ah, al						; swap al and ah

label_a:
	jcxz last_pixels				; if cx = 0 then jump to last_pixels


loop_pixels:
	mov [di + PLANEWIDTH * 0], al	; write pixel
	mov [di + PLANEWIDTH * 1], ah
	mov [di + PLANEWIDTH * 2], al
	mov [di + PLANEWIDTH * 3], ah
	mov [di + PLANEWIDTH * 4], al
	mov [di + PLANEWIDTH * 5], ah
	mov [di + PLANEWIDTH * 6], al
	mov [di + PLANEWIDTH * 7], ah
	add  di , PLANEWIDTH * 8		; point to next line

	loop loop_pixels				; if --cx != 0 then jump to loop_pixels


last_pixels:
	cs jmp last_pixel_jump_table[bx]


last_pixel7:
	mov [di], al
	xchg ah, al
	add di, PLANEWIDTH

last_pixel6:
	mov [di], al
	xchg ah, al
	add di, PLANEWIDTH

last_pixel5:
	mov [di], al
	xchg ah, al
	add di, PLANEWIDTH

last_pixel4:
	mov [di], al
	xchg ah, al
	add di, PLANEWIDTH

last_pixel3:
	mov [di], al
	xchg ah, al
	add di, PLANEWIDTH

last_pixel2:
	mov [di], al
	xchg ah, al
	add di, PLANEWIDTH

last_pixel1:
	mov [di], al

last_pixel0:
	pop di
	mov ax, ss
	mov ds, ax
	retf
