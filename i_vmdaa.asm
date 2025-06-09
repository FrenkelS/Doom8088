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

PLANEWIDTH equ 160
VIEWWINDOWHEIGHT equ 25

extern source
extern colormap
extern attribute
extern dest

last_pixel_jump_table:
%assign i 0
%rep VIEWWINDOWHEIGHT+1
	dw last_pixel%+i,
%assign i i+1
%endrep

last_pixel_flat_jump_table:
%assign i 0
%rep VIEWWINDOWHEIGHT+1
	dw last_pixel_flat%+i,
%assign i i+1
%endrep

;
; input:
;   ax = fracstep
;   dx = frac
;   cx = count		1 <= count <= VIEWWINDOWHEIGHT	=>	ch = 0
;

global R_DrawColumn2
R_DrawColumn2:
	push si
	push di
	push es
	push bp

	xchg bp, ax						; bp = fracstep

	mov bx, cx						; bx = count

	mov cx, [colormap]
	mov ah, [attribute]

	les di, [dest]					; es:di = dest
	lds si, [source]				; ds:si = source

	shl bl, 1
	cs jmp last_pixel_jump_table[bx]


%assign i VIEWWINDOWHEIGHT
%rep VIEWWINDOWHEIGHT-1
last_pixel%+i:
	mov al, dh						; al = hi byte of frac
	shr al, 1						; 0 <= al <= 127
	mov bx, si						; bx = source
	xlat							; al = source[al]
	mov bx, cx						; bx = colormap
	ss xlat							; al = colormap[al]
	stosw							; write pixel
	add di, PLANEWIDTH-2			; point to next line
	add dx, bp						; frac += fracstep
%assign i i-1
%endrep


last_pixel1:
	mov al, dh
	shr al, 1
	mov bx, si
	xlat
	mov bx, cx
	mov dx, ss
	mov ds, dx
	xlat
	stosw

last_pixel0:	; count > 0, so the code never jumps to this label
	pop bp
	pop es
	pop di
	pop si
	retf


;
; input:
;   al = color
;   dx = count		1 <= count <= VIEWWINDOWHEIGHT	=>	dh = 0
;

global R_DrawColumnFlat2
R_DrawColumnFlat2:
	push di

	lds di, [dest]

	mov ah, 7

	mov bx, dx
	shl bx, 1
	cs jmp last_pixel_flat_jump_table[bx]

%assign i VIEWWINDOWHEIGHT
%rep VIEWWINDOWHEIGHT
last_pixel_flat%+i:
	mov [di + PLANEWIDTH * (i - 1)], ax
%assign i i-1
%endrep

last_pixel_flat0:
	pop di
	mov ax, ss
	mov ds, ax
	retf
