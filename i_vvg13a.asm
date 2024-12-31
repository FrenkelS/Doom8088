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

SCREENWIDTH equ 240

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
	dw last_pixel_flat7

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

	mov al, cl						; 1 <= al <= 128
%ifidn CPU, i8088
	mov cl, 4
	shr al, cl						; 0 <= al <= 8
%else
	shr al, 4						; 0 <= al <= 8
%endif

	mov cx, [colormap]

	les di, [dest]					; es:di = dest
	lds si, [source]				; ds:si = source

	or al, al						; if al = 0
	jz last_pixels					;  then jump to last_pixels

	push bx							; push count

loop_pixels:
	push ax

	mov al, dh						; al = hi byte of frac
	shr al, 1						; 0 <= al <= 127
	mov bx, si						; bx = source
	xlat							; al = source[al]
	mov bx, cx						; bx = colormap
	ss xlat							; al = colormap[al]
	mov ah, al
	stosw							; write pixel
	stosw
	add di, SCREENWIDTH-4			; point to next line
	add dx, bp						; frac += fracstep

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

	pop ax
	dec al
	jnz loop_pixels					; if --al != 0 then jump to loop_pixels

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
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel14:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel13:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel12:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel11:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel10:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel9:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel8:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel7:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel6:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel5:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel4:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel3:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
	add dx, bp

last_pixel2:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	ss xlat
	mov ah, al
	stosw
	stosw
	add di, SCREENWIDTH-4
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
	mov ah, al
	stosw
	stosw
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
;   dl = dontcare
;   cx = count		1 <= count <= 128	=>	ch = 0
;

global R_DrawColumnFlat2
R_DrawColumnFlat2:
	push di

	lds di, [dest]

	mov ah, al

	mov bx, cx
	and bl, 7
	shl bl, 1

%ifidn CPU, i8088
	shr cx, 1
	shr cx, 1
	shr cx, 1
%else
	shr cx, 3
%endif

	jcxz last_pixels_flat


lab:
	mov [di + SCREENWIDTH *  0    ], ax
	mov [di + SCREENWIDTH *  0 + 2], ax
	mov [di + SCREENWIDTH *  1    ], ax
	mov [di + SCREENWIDTH *  1 + 2], ax
	mov [di + SCREENWIDTH *  2    ], ax
	mov [di + SCREENWIDTH *  2 + 2], ax
	mov [di + SCREENWIDTH *  3    ], ax
	mov [di + SCREENWIDTH *  3 + 2], ax
	mov [di + SCREENWIDTH *  4    ], ax
	mov [di + SCREENWIDTH *  4 + 2], ax
	mov [di + SCREENWIDTH *  5    ], ax
	mov [di + SCREENWIDTH *  5 + 2], ax
	mov [di + SCREENWIDTH *  6    ], ax
	mov [di + SCREENWIDTH *  6 + 2], ax
	mov [di + SCREENWIDTH *  7    ], ax
	mov [di + SCREENWIDTH *  7 + 2], ax
	add  di , SCREENWIDTH *  8
	loop lab


last_pixels_flat:
	cs jmp last_pixel_flat_jump_table[bx]


last_pixel_flat7:
	mov [di + SCREENWIDTH *  6    ], ax
	mov [di + SCREENWIDTH *  6 + 2], ax

last_pixel_flat6:
	mov [di + SCREENWIDTH *  5    ], ax
	mov [di + SCREENWIDTH *  5 + 2], ax

last_pixel_flat5:
	mov [di + SCREENWIDTH *  4    ], ax
	mov [di + SCREENWIDTH *  4 + 2], ax

last_pixel_flat4:
	mov [di + SCREENWIDTH *  3    ], ax
	mov [di + SCREENWIDTH *  3 + 2], ax

last_pixel_flat3:
	mov [di + SCREENWIDTH *  2    ], ax
	mov [di + SCREENWIDTH *  2 + 2], ax

last_pixel_flat2:
	mov [di + SCREENWIDTH *  1    ], ax
	mov [di + SCREENWIDTH *  1 + 2], ax

last_pixel_flat1:
	mov [di + SCREENWIDTH *  0    ], ax
	mov [di + SCREENWIDTH *  0 + 2], ax

last_pixel_flat0:
	pop di
	mov ax, ss
	mov ds, ax
	retf
