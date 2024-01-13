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

last_pixel_jump_table:
	dw last_pixel0,
	dw last_pixel1,
	dw last_pixel2,
	dw last_pixel3,
	dw last_pixel4,
	dw last_pixel5,
	dw last_pixel6,
	dw last_pixel7,
	dw last_pixel8,
	dw last_pixel9,
	dw last_pixel10,
	dw last_pixel11,
	dw last_pixel12,
	dw last_pixel13,
	dw last_pixel14,
	dw last_pixel15

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
	shr cl, 1
	shr cl, 1
	shr cl, 1						; 0 <= cl <= 8 && ch = 0

	or cx, cx						; if cx = 0
	jz last_pixels					;  then jump to last_pixels

loop_pixels:
	push cx
	mov cx, nearcolormap

	mov al, dh						; al = hi byte of frac
	shr al, 1						; 0 <= al <= 127
	mov bx, si						; bx = source
	xlat							; al = source[al]
	mov bx, cx						; bx = nearcolormap
	ss xlat							; al = nearcolormap[al]
	stosb							; write pixel
	add di, PLANEWIDTH-1			; point to next line
	add dx, bp						; frac += fracstep

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

	pop cx
	dec cx
	jnz loop_pixels					; if --cx != 0 then jump to loop_pixels


last_pixels:
	mov cx, nearcolormap
	and ah, 15						; 0 <= count <= 15
	xor bh, bh
	mov bl, ah
	shl bx, 1
	cs jmp last_pixel_jump_table[bx]


last_pixel15:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel14:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel13:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel12:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel11:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel10:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel9:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel8:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel7:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel6:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel5:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel4:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel3:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel2:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb
	add di, PLANEWIDTH-1
	add dx, bp

last_pixel1:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	stosb

last_pixel0:
	pop bp
	pop es
	pop di
	pop si
	push ss
	pop ds
	retf
