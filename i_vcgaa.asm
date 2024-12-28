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

;
; input:
;   al = color
;   dx = odd
;   cx = count		1 <= count <= 128	=>	ch = 0
;

global R_DrawColumnFlat2
R_DrawColumnFlat2:
	push di
	push es

	les di, [dest]					; es:di = dest

	mov ah, al
	ror ah, 4

	or dx, dx						; if dx = 0
	jz label_a						;  then jump to label_a
	xchg ah, al

label_a:
	mov bx, cx						; bx = count
	shr cx, 1						; 0 <= cx <= 64
	jcxz last_pixel1				;  then jump to last_pixel1

loop_pixels:
	stosb							; write pixel
	xchg ah, al
	add di, PLANEWIDTH-1			; point to next line

	stosb
	xchg ah, al
	add di, PLANEWIDTH-1

	loop loop_pixels				; if --cx != 0 then jump to loop_pixels


	or bx, bx						; if bx = 0
	jz last_pixel0					;  then jump to last_pixel0


last_pixel1:
	stosb

last_pixel0:
	pop es
	pop di
	retf
