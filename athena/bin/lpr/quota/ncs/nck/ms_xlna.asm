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
; M S _ X L N A
;
; MS/DOS / INET-family / XLN -specific assembly language routines.  (MS/DOS)
;

	dosseg
	.model large

	.code

;
; _xln_anr_a -- 
;

	public	_xln_anr_a
	extrn	_xln_anr:far
_xln_anr_a	proc	far	
	push	bp
	mov	bp, sp
;
; set up DS to the application DS
;
	mov	ax, DGROUP
	mov	ds, ax
;
; set up offset of DCB and retcode on the stack
;
	mov	ax, [bp + 6]	; offset of DCB
	mov	cx, [bp + 8]	; segment of DCB
	mov	bx, [bp + 10]	; retcode
	push	bx		; retcode
	push	cx		; segment
	push	ax	       	; offset
;
; call xln_anr
;
	call	_xln_anr
;
; restore stack
;
	pop	ax
	pop	ax
	pop	ax
;
; return
;	
	pop	bp
	ret
_xln_anr_a	endp

	end
