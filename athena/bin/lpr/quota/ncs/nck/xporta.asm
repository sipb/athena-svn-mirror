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
; X P O R T A
;
; Interface to the Microsoft MSNET XPORT driver.  (MS/DOS)
;
; xport_$command(tcb)
; xport_$null_handler()

DOSSEG
.MODEL LARGE

include compiler.mac

.CODE

; xport_$command(tcb)
CPROC xport_$command
	push es

if @codesize
	les bx,dword ptr arg1
else
	mov bx,arg1
	push ds
	pop es
endif
	mov ax,0100h	; xport resets this if installed, otherwise non-zero return
	int 5bH

	pop es
CRET xport_$command

; xport_$null_handler()
; es:[bx] points to acb
; sets the command_status word given the cid
; note always a far return from this function
public _xport_$null_handler
_xport_$null_handler PROC FAR
	ret
_xport_$null_handler endp


ifndef NOT_YET

; xport_$anr_copy(dest_acb, src_acb, size)
; acb* far dest_acb
; acb* far src_acb
; int size
CPROC xport_$anr_copy
	push ds
	push es

	les di,dword ptr arg1
	lds si,dword ptr arg3
	mov cx,arg5
	rep movsb

	pop es
	pop ds
CRET xport_$anr_copy

endif

end
