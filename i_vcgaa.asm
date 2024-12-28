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

bits 16

PLANEWIDTH equ 60

extern dest

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

	mov ah, al
	ror ah, 4

	shr dx, 1						; if yl is odd
	jnc label_a						;  then jump to label_a
	xchg ah, al

label_a:
	mov bx, cx						; bx = count
	shr cx, 1						; 0 <= cx <= 64
	jcxz last_pixel1				; if count = 1 then jump to last_pixel1


loop_pixels:
	mov [di+PLANEWIDTH*0], al		; write pixel
	mov [di+PLANEWIDTH*1], ah		; write pixel
	add di, PLANEWIDTH*2			; point to next line

	loop loop_pixels				; if --cx != 0 then jump to loop_pixels


	shr bx, 1						; if count is odd
	jnc last_pixel0					;  then jump to last_pixel0


last_pixel1:
	mov [di], al

last_pixel0:
	pop di
	mov ax, ss
	mov ds, ax
	retf
