;
; Copyright (C) 2023 Frenkel Smeijers
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


; input:
;   ffff ffff = dividend (was dx:ax)
;   dx:ax = divisor (was cx:bx)
;
; output:
;   dx:ax = quotient of division of dividend by divisor
;   cx:bx = remainder of division of dividend by divisor

global FixedReciprocal
FixedReciprocal:
	test    dx, dx             ; divisor > 2^16-1 ?
	jnz     big_divisor        ; yes, divisor > 2^16-1

	mov     bx, ax             ;
	mov     ax, 0ffffh         ; 
	mov     cx, ax             ; ax = FFFFh, bx = divisor-lo, cx = FFFFh, dx = 0
	div     bx                 ; quotient-hi in ax
	xchg    ax, cx             ; cx = quotient-hi, ax = FFFFh

	div     bx                 ; ax = quotient-lo
	mov     dx, cx             ; dx = quotient-hi (quotient in dx:ax)
	ret

big_divisor:
	mov     cx, dx
	mov     bx, ax
	mov     dx, 0ffffh
	mov     ax, dx

	push    si                 ; save temp
	push    di                 ;  variables
	push    dx                 ; save
	push    ax                 ;  dividend
	mov     si, bx             ; divisor now in
	mov     di, cx             ;  di:si and cx:bx
shift_loop:
	shr     dx, 1              ; shift both
	rcr     ax, 1              ;  dividend
	shr     cx, 1              ;   and divisor
	rcr     bx, 1              ;    right by 1 bit
	jnz     shift_loop         ;     loop if di non-zero (rcr does not touch ZF)
	div     bx                 ; compute quotient dx:ax>>x / cx:bx>>x (stored in ax; remainder in dx not used)
	pop     bx                 ; get dividend lo-word
	mov     cx, ax             ; save quotient
	mul     di                 ; quotient * divisor hi-word (low only)
	pop     dx                 ; dividend high
	sub     dx, ax             ; dividend high - divisor high * quotient, no overflow (carry/borrow) possible here
	push    dx                 ; save dividend high
	mov     ax, cx             ; ax=quotient
	mul     si                 ; quotient * divisor lo-word
	sub     bx, ax             ; dividend-lo - (quot.*divisor-lo)-lo
	mov     ax, cx             ; get quotient
	pop     cx                 ; restore dividend hi-word
	sbb     cx, dx             ; subtract (divisor-lo * quot.)-hi from dividend-hi
	sbb     dx, dx             ; 0 if remainder > 0, else FFFFFFFFh
	and     si, dx             ; nothing to add
	and     di, dx             ;  back if remainder positive di:si := di:si(cx:bx) & dx:dx
	add     bx, si             ; correct remainder           cx:bx += di:si
	adc     cx, di             ;  and
	add     ax, dx             ;   quotient if necessary           ax += dx
	xor     dx, dx             ; clear hi-word of quot (ax<=FFFFh) dx := 0
	pop     di                 ; restore temp  
	pop     si                 ;  variables
	ret
