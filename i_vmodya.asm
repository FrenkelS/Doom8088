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

PLANEWIDTH equ 80

extern source
extern nearcolormap
extern dest

;
; input:
;   ax = fracstep
;   dx = frac
;   cx = count		1 <= count <= 128	=>	ch = 0
;

global R_DrawColumn2
R_DrawColumn2:
	push si
	push di
	push es
	push bp

	xchg bp, ax						; bp = fracstep

	les di, [dest]					; es:di = dest
	lds si, [source]				; ds:si = source

	mov ah, cl						; ah = count
	shr cl, 1
	shr cl, 1						; 0 <= cl <= 32 && ch = 0

	jcxz last_pixels				; if cx = 0 then jump to last_pixels

loop_pixels:
	mov al, dh						; al = hi byte of frac
	shr al, 1						; 0 <= al <= 127
	mov bx, si						; bx = source
	xlat							; al = source[al]
	mov bx, nearcolormap			; bx = nearcolormap
	ss xlat							; al = nearcolormap[al]
	stosb							; write pixel line 1
	add dx, bp						; frac += fracstep

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, nearcolormap
	ss xlat
	mov es:[di+PLANEWIDTH*1-1], al	; write pixel line 2
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, nearcolormap
	ss xlat
	mov es:[di+PLANEWIDTH*2-1], al	; write pixel line 3
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, nearcolormap
	ss xlat
	mov es:[di+PLANEWIDTH*3-1], al	; write pixel line 4
	add dx, bp

	add di, PLANEWIDTH*4-1			; dest = next line
	loop loop_pixels				; if --cx != 0 then jump to loop_pixels

last_pixels:
	and ah, 3						; 0 <= count <= 3
	cmp ah, 3						; if count = 3
	je last_pixel3					;  then jump to last_pixel3
	cmp ah, 2						; if count = 2
	je last_pixel2					;  then jump to last_pixel2
	or ah, ah						; if count = 1
	jnz last_pixel1					;  then jump to last_pixel1

	pop bp							; else return
	pop es
	pop di
	pop si
	push ss
	pop ds
	retf


last_pixel3:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, nearcolormap
	ss xlat
	stosb							; write third to last pixel
	add dx, bp
	add di, PLANEWIDTH-1

last_pixel2:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, nearcolormap
	ss xlat
	stosb							; write second to last pixel
	add dx, bp
	add di, PLANEWIDTH-1

last_pixel1:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, nearcolormap
	ss xlat
	stosb							; write last pixel

	pop bp
	pop es
	pop di
	pop si
	push ss
	pop ds
	retf
