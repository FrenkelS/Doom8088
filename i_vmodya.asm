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

PLANEWIDTH equ 80

extern source
extern colormap
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

	mov bx, cx						; bx = count

	mov ah, cl						; 1 <= ah <= 128
%ifidn CPU, i8088
	mov cl, 4
	shr ah, cl						; 0 <= ah <= 8
%else
	shr ah, 4						; 0 <= ah <= 8
%endif

	mov cx, [colormap]

	les di, [dest]					; es:di = dest
	lds si, [source]				; ds:si = source

	or ah, ah						; if ah = 0
	jz last_pixels					;  then jump to last_pixels

	push bx							; push count

loop_pixels:
	mov al, dh						; al = hi byte of frac
	shr al, 1						; 0 <= al <= 127
	mov bx, si						; bx = source
	xlat							; al = source[al]
	mov bx, cx						; bx = colormap
	ss xlat							; al = colormap[al]
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

	dec ah
	jnz loop_pixels					; if --ah != 0 then jump to loop_pixels

	pop bx							; pop count


last_pixels:
	and bl, 15						; 0 <= count <= 15
	shl bl, 1
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
	mov dx, ss
	mov ds, dx
	xlat
	stosb
	pop bp
	pop es
	pop di
	pop si
	retf

last_pixel0:
	pop bp
	pop es
	pop di
	pop si
	mov ax, ss
	mov ds, ax
	retf


;
; input:
;   al = color
;   dl = don't care
;   cx = count		1 <= count <= 128	=>	ch = 0
;

global R_DrawColumnFlat2
R_DrawColumnFlat2:
	push di

	lds di, [dest]					; ds:di = dest

	mov bx, cx
	and bl, 15
	shl bl, 1

%ifidn CPU, i8088
	shr cx, 1
	shr cx, 1
	shr cx, 1
	shr cx, 1
%else
	shr cx, 4
%endif
	jcxz last_pixels_flat


lab:
	mov [di + PLANEWIDTH *  0], al
	mov [di + PLANEWIDTH *  1], al
	mov [di + PLANEWIDTH *  2], al
	mov [di + PLANEWIDTH *  3], al
	mov [di + PLANEWIDTH *  4], al
	mov [di + PLANEWIDTH *  5], al
	mov [di + PLANEWIDTH *  6], al
	mov [di + PLANEWIDTH *  7], al
	mov [di + PLANEWIDTH *  8], al
	mov [di + PLANEWIDTH *  9], al
	mov [di + PLANEWIDTH * 10], al
	mov [di + PLANEWIDTH * 11], al
	mov [di + PLANEWIDTH * 12], al
	mov [di + PLANEWIDTH * 13], al
	mov [di + PLANEWIDTH * 14], al
	mov [di + PLANEWIDTH * 15], al
	add di, PLANEWIDTH * 16
	loop lab


last_pixels_flat:
	cs jmp last_pixel_flat_jump_table[bx]


last_pixel_flat15:
	mov [di + PLANEWIDTH * 14], al

last_pixel_flat14:
	mov [di + PLANEWIDTH * 13], al

last_pixel_flat13:
	mov [di + PLANEWIDTH * 12], al

last_pixel_flat12:
	mov [di + PLANEWIDTH * 11], al

last_pixel_flat11:
	mov [di + PLANEWIDTH * 10], al

last_pixel_flat10:
	mov [di + PLANEWIDTH *  9], al

last_pixel_flat9:
	mov [di + PLANEWIDTH *  8], al

last_pixel_flat8:
	mov [di + PLANEWIDTH *  7], al

last_pixel_flat7:
	mov [di + PLANEWIDTH *  6], al

last_pixel_flat6:
	mov [di + PLANEWIDTH *  5], al

last_pixel_flat5:
	mov [di + PLANEWIDTH *  4], al

last_pixel_flat4:
	mov [di + PLANEWIDTH *  3], al

last_pixel_flat3:
	mov [di + PLANEWIDTH *  2], al

last_pixel_flat2:
	mov [di + PLANEWIDTH *  1], al

last_pixel_flat1:
	mov [di + PLANEWIDTH *  0], al

last_pixel_flat0:
	pop di
	mov ax, ss
	mov ds, ax
	retf
