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


; Divide FFFFFFFFh by a 32-bit number.
; Based on https://github.com/FDOS/kernel/blob/master/kernel/ludivmul.inc
;
; input:
;   dx:ax = divisor
;
; output:
;   dx:ax = quotient of division of FFFFFFFFh by divisor

global FixedReciprocal
FixedReciprocal:
	xchg    bx, ax             ; bx = divisor-lo
	mov     ax, 0ffffh         ; ax = FFFFh

	test    dx, dx             ; divisor > 2^16-1 ?
	jnz     big_divisor        ; yes, divisor > 2^16-1

	mov     cx, ax             ; ax = FFFFh, bx = divisor-lo, cx = FFFFh, dx = 0
	div     bx                 ; quotient-hi in ax
	xchg    ax, cx             ; cx = quotient-hi, ax = FFFFh

	div     bx                 ; ax = quotient-lo
	mov     dx, cx             ; dx = quotient-hi
	retf                       ; return quotient in dx:ax

big_divisor:
	mov     cx, dx             ; cx = divisor-hi
	mov     dx, ax             ; ax = FFFFh, bx = divisor-lo, cx = divisor-hi, dx = FFFFh

	push    si                 ; save temp
	push    di                 ;  variables
	mov     si, bx             ; divisor now in
	mov     di, cx             ;  di:si and cx:bx
shift_loop:
	shr     dx, 1              ; shift both dividend = FFFFFFFFh
	shr     cx, 1              ;  and divisor
	rcr     bx, 1              ;   right by 1 bit
	jnz     shift_loop         ;    loop if di non-zero (rcr does not touch ZF)
	div     bx                 ; compute quotient FFFFh:FFFFh>>x / cx:bx>>x (stored in ax; remainder in dx not used)
	mov     cx, ax             ; save quotient
	mul     di                 ; quotient * divisor hi-word (low only)
	not     ax                 ; dividend high - divisor high * quotient, no overflow (carry/borrow) possible here
	mov     bx, ax             ; save dividend high
	mov     ax, cx             ; ax=quotient
	mul     si                 ; quotient * divisor lo-word
	mov     ax, cx             ; get quotient
	sub     bx, dx             ; subtract (divisor-lo * quot.)-hi from dividend-hi
	sbb     dx, dx             ; 0 if remainder > 0, else FFFFFFFFh
	add     ax, dx             ; correct quotient if necessary           ax += dx
	xor     dx, dx             ; clear hi-word of quot (ax<=FFFFh) dx := 0
	pop     di                 ; restore temp  
	pop     si                 ;  variables
	retf                       ; return quotient in dx:ax


; Divide FFFFFFFFh by a 16-bit number.
;
; input:
;   ax = divisor
;
; output:
;   dx:ax = quotient of division of FFFFFFFFh by divisor

global FixedReciprocalSmall
FixedReciprocalSmall:
	xchg    bx, ax             ; bx = divisor
	mov     ax, 0ffffh         ;
	mov     cx, ax             ;
	xor     dx, dx             ; ax = FFFFh, bx = divisor, cx = FFFFh, dx = 0
	div     bx                 ; quotient-hi in ax
	xchg    ax, cx             ; cx = quotient-hi, ax = FFFFh

	div     bx                 ; ax = quotient-lo
	mov     dx, cx             ; dx = quotient-hi
	retf                       ; return quotient in dx:ax


; Divide FFFFFFFFh by a 32-bit number.
;
; input:
;   dx:ax = divisor, dx != 0
;
; output:
;   ax = quotient of division of FFFFFFFFh by divisor

global FixedReciprocalBig
FixedReciprocalBig:
	xchg    bx, ax             ; bx = divisor-lo
	mov     cx, dx             ; cx = divisor-hi
	mov     ax, 0ffffh         ; ax = FFFFh
	mov     dx, ax             ; ax = FFFFh, bx = divisor-lo, cx = divisor-hi, dx = FFFFh

	push    si                 ; save temp
	push    di                 ;  variables
	mov     si, bx             ; divisor now in
	mov     di, cx             ;  di:si and cx:bx
shift_loop_big:
	shr     dx, 1              ; shift both dividend = FFFFFFFFh
	shr     cx, 1              ;  and divisor
	rcr     bx, 1              ;   right by 1 bit
	jnz     shift_loop_big     ;    loop if di non-zero (rcr does not touch ZF)
	div     bx                 ; compute quotient FFFFh:FFFFh>>x / cx:bx>>x (stored in ax; remainder in dx not used)
	mov     cx, ax             ; save quotient
	mul     di                 ; quotient * divisor hi-word (low only)
	not     ax                 ; dividend high - divisor high * quotient, no overflow (carry/borrow) possible here
	mov     bx, ax             ; save dividend high
	mov     ax, cx             ; ax=quotient
	mul     si                 ; quotient * divisor lo-word
	mov     ax, cx             ; get quotient
	sub     bx, dx             ; subtract (divisor-lo * quot.)-hi from dividend-hi
	sbb     dx, dx             ; 0 if remainder > 0, else FFFFFFFFh
	add     ax, dx             ; correct quotient if necessary           ax += dx
	pop     di                 ; restore temp  
	pop     si                 ;  variables
	retf                       ; return quotient in dx:ax
