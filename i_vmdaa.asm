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

extern source
extern nearcolormap
extern dest

last_pixel_jump_table:
%assign i 0
%rep VIEWWINDOWHEIGHT+1
	dw last_pixel%+i,
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

	mov di, [dest]					; ds:di = dest
	les si, [source]				; es:si = source

	mov bx, cx						; bx = count

	mov cx, nearcolormap

	shl bl, 1
	cs jmp last_pixel_jump_table[bx]


%assign i VIEWWINDOWHEIGHT
%rep VIEWWINDOWHEIGHT-1
last_pixel%+i:
	mov al, dh						; al = hi byte of frac
	shr al, 1						; 0 <= al <= 127
	mov bx, si						; bx = source
	es xlat							; al = source[al]
	mov bx, cx						; bx = nearcolormap
	xlat							; al = nearcolormap[al]
	mov [di], al					; write pixel
	add di, PLANEWIDTH				; point to next line
	add dx, bp						; frac += fracstep
%assign i i-1
%endrep


last_pixel1:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al

last_pixel0:
	pop bp
	pop es
	pop di
	pop si
	retf