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

last_pixel_flat_jump_table:
	dw last_pixel_flat0,
	dw last_pixel_flat1,
	dw last_pixel_flat2,
	dw last_pixel_flat3,
	dw last_pixel_flat4,
	dw last_pixel_flat5,
	dw last_pixel_flat6,
	dw last_pixel_flat7,
	dw last_pixel_flat8,
	dw last_pixel_flat9,
	dw last_pixel_flat10,
	dw last_pixel_flat11,
	dw last_pixel_flat12,
	dw last_pixel_flat13,
	dw last_pixel_flat14,
	dw last_pixel_flat15

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
	and bl, 15						; 0 <= count <= 15
	shl bl, 1

	mov ah, al
%ifidn CPU, i8088
	ror ah, 1
	ror ah, 1
	ror ah, 1
	ror ah, 1						; ah = al with nibbles swapped

	shr cx, 1
	shr cx, 1
	shr cx, 1
	shr cx, 1						; 0 <= cx <= 8
%else
	ror ah, 4						; ah = al with nibbles swapped

	shr cx, 4						; 0 <= cx <= 8
%endif

	shr dx, 1						; if yl is odd
	jnc label_a						;  then jump to label_a
	xchg ah, al						; swap al and ah

label_a:
	jcxz last_pixels_flat				; if cx = 0 then jump to last_pixels_flat


lab:
	mov [di + PLANEWIDTH *  0], al	; write pixels
	mov [di + PLANEWIDTH *  1], ah
	mov [di + PLANEWIDTH *  2], al
	mov [di + PLANEWIDTH *  3], ah
	mov [di + PLANEWIDTH *  4], al
	mov [di + PLANEWIDTH *  5], ah
	mov [di + PLANEWIDTH *  6], al
	mov [di + PLANEWIDTH *  7], ah
	mov [di + PLANEWIDTH *  8], al
	mov [di + PLANEWIDTH *  9], ah
	mov [di + PLANEWIDTH * 10], al
	mov [di + PLANEWIDTH * 11], ah
	mov [di + PLANEWIDTH * 12], al
	mov [di + PLANEWIDTH * 13], ah
	mov [di + PLANEWIDTH * 14], al
	mov [di + PLANEWIDTH * 15], ah
	add  di , PLANEWIDTH * 16		; point to next line

	loop lab				; if --cx != 0 then jump to lab


last_pixels_flat:
	cs jmp last_pixel_flat_jump_table[bx]


last_pixel_flat15:
	mov [di + PLANEWIDTH * 14], al

last_pixel_flat14:
	mov [di + PLANEWIDTH * 13], ah

last_pixel_flat13:
	mov [di + PLANEWIDTH * 12], al

last_pixel_flat12:
	mov [di + PLANEWIDTH * 11], ah

last_pixel_flat11:
	mov [di + PLANEWIDTH * 10], al

last_pixel_flat10:
	mov [di + PLANEWIDTH *  9], ah

last_pixel_flat9:
	mov [di + PLANEWIDTH *  8], al

last_pixel_flat8:
	mov [di + PLANEWIDTH *  7], ah

last_pixel_flat7:
	mov [di + PLANEWIDTH *  6], al

last_pixel_flat6:
	mov [di + PLANEWIDTH *  5], ah

last_pixel_flat5:
	mov [di + PLANEWIDTH *  4], al

last_pixel_flat4:
	mov [di + PLANEWIDTH *  3], ah

last_pixel_flat3:
	mov [di + PLANEWIDTH *  2], al

last_pixel_flat2:
	mov [di + PLANEWIDTH *  1], ah

last_pixel_flat1:
	mov [di + PLANEWIDTH *  0], al

last_pixel_flat0:
	pop di
	mov ax, ss
	mov ds, ax
	retf
