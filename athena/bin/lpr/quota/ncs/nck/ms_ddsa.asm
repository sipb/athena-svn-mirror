;
; ==========================================================================
; Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
; Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
; Copyright Laws Of The United States.
;
; Apollo Computer Inc. reserves all rights, title and interest with respect
; to copying, modification or the distribution of such software programs
; and associated documentation, except those rights specifically granted
; by Apollo in a Product Software Program License, Source Code License
; or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
; Apollo and Licensee.  Without such license agreements, such software
; programs may not be used, copied, modified or distributed in source
; or object code form.  Further, the copyright notice must appear on the
; media, the supporting documentation and packaging as set forth in such
; agreements.  Such License Agreements do not grant any rights to use
; Apollo Computer's name or trademarks in advertising or publicity, with
; respect to the distribution of the software programs without the specific
; prior written permission of Apollo.  Trademark agreements may be obtained
; in a separate Trademark License Agreement.
; ==========================================================================
;
; M S _ D D S A
;
; MS/DOS / DDS-family -specific assembly language routines.  (MS/DOS)
;

; dds_$recv_handler()
; dds_$null_handler()
;   - asynchronous notification routines for completion of XPORT command

DOSSEG
.MODEL LARGE

include compiler.mac

STACK_SIZE equ 256			; must match anr stack size in ms_dds.c

if @codesize
extrn _dds_$anr_handler:far
else
extrn _dds_$anr_handler:near
endif

_bss      segment
comm near	_dds_$anr_stack:	byte:	 256
_bss      ends

; Macro to save all volatile registers
pushall macro
	push es
	push ds
	push di
	push si
	push bp
	push dx
	push cx
	push bx
	push ax
endm

; Macro to restore all volatile registers
popall macro
	pop ax
	pop bx
	pop cx
	pop dx
	pop bp
	pop si
	pop di
	pop ds
	pop es
endm

.CODE

; dds_$recv_handler()
; es:[bx] points to acb
; sets the recv_status word given the cid
; note the far return from this function
public _dds_$recv_handler
_dds_$recv_handler PROC FAR
	pushall

	mov ax,ss
	mov cx,sp
	mov dx,dgroup
	mov ss,dx
	mov sp,OFFSET dgroup:_dds_$anr_stack + STACK_SIZE ; get top of stack
	push ax		; the old ss
	push cx		; the old sp
	
	push es		; push acb far ptr arg
	push bx

	mov ax,dgroup
	mov ds,ax
	mov es,ax
if @codesize
	call far ptr _dds_$anr_handler
else
	call near ptr _dds_$anr_handler
endif
	add sp,4

	pop cx		; the old sp
	pop ax		; the old ss
	mov ss,ax	; use old stack
	mov sp,cx

	popall
	ret
_dds_$recv_handler endp

; dds_$null_handler()
;  the do nothing ANR
;  must be far return irregardless of model
public _dds_$null_handler
_dds_$null_handler PROC FAR
	ret
_dds_$null_handler endp

end
