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
; M S D O S A
;
; MS/DOS-specific assembly language routines.  (MS/DOS)
;

; setjmp_nck(jbuf)
; longjmp_nck(jbuf, val)
; alarm_$isr()
; alarm_$open(stack_top, stack_bottom)
; alarm_$close()
; alarm_$time()
; xlong_$add_long(xl, l)
; xlong_$add_xlong(xl, xl2)
; xlong_$mult_long(xl, l)
; swab_$short(s)
; swab_$long(l)
; rpc_$swab_header(pkt)

DOSSEG
.MODEL LARGE

include compiler.mac

.DATA
;;; Offset of the stack bottom for stack probe
extrn STKHQQ:word

.CODE

;===========================================================================
;                     Setjmp functions
; setjmp_nck(jbuf)
; jmp_buf jbuf
;
; Fill in the jump buf with neccessary register values to continue after the
; call to this function.  jmp_buf format is:
;
;     Large model:    di,si,sp,bp,ds,es,ss,ret_off,ret_seg,flags  (total: 20 bytes)
;
; Notes:
; - This function returns a long value.  The setjmp in the MSC runtime library
;   returns an int.
; - longjmp_nck() takes a long arg (instead of an int).
; - The MSC RTL jmp_buf is not big enough, use one in std.h.
; - Unlike MSC, this setjmp/longjmp saves/restores the processor flags.  This
;   (and hopefully only this) is required to longjmp from an interrupt routine.
; - We are no longer longjmp'ing from interrupt routines, but will leave the
;   processor flag save/restore code.

_setjmp_nck PROC
	public _setjmp_nck
	mov ax,bp  ; stash caller's bp
	mov dx,ds  ; stash caller's ds
	mov bp,sp

	;;; Get jbuf into ds:bx
if @datasize
if @codesize
	lds bx,[bp+4]
else
	lds bx,[bp+2]
endif
else  ;!LDATA
if @codesize
	mov bx,[bp+4]
else
	mov bx,[bp+2]
endif
endif
	mov [bx+00h],di  ; di
	mov [bx+02h],si  ; si
	mov [bx+04h],sp  ; sp (just after call)
	mov [bx+06h],ax  ; caller's bp
	mov [bx+08h],dx  ; caller's ds
	mov [bx+0Ah],es  ; es  (prob not necc for MSC)
	mov [bx+0Ch],ss  ; ss

	;;; Get return address offset
	mov cx,[bp+00h]  ; from caller's stack
	mov [bx+0Eh],cx

	;;; Get return address segment
if @codesize
	mov cx,[bp+02h]  ; from caller's stack
	mov [bx+10h],cx 
else
	mov [bx+10h],cs
endif

	;;; Get flags
	pushf
	pop [bx+12h]

if @datasize
	mov ds,dx  ; restore old ds
endif
	mov bp,ax  ; restore caller's bp

	;;; Return 0L
	xor dx,dx
	xor ax,ax
	ret
_setjmp_nck endp

; longjmp_nck(jbuf, val)
; jumpbuf* jbuf
; long val
; - val should != 0. If it does, it is changed to 1.
CPROC longjmp_nck
if @datasize
	mov ax,arg3
	mov dx,arg4
else
	mov ax,arg2
	mov dx,arg3
endif
	or ax,ax
	jnz not_zero
	or dx,dx
	jnz not_zero
	inc ax           ; make sure don't return a zero value
not_zero:
	;;; Get jbuf into ds:bx
if @datasize
	lds bx,arg1
else
	mov bx,arg1
endif

	;;; From here on, longjmp() caller's stack is not used

	mov di,[bx+00h]  ; di
	mov si,[bx+02h]  ; si
	mov bp,[bx+06h]  ; setjmp() caller's bp
	mov es,[bx+0Ah]  ; es (prob not necc for MSC)

	;;; Disable interrupts while manipulating the stack address
	cli              
	mov ss,[bx+0Ch]  ; ss
	mov sp,[bx+04h]  ; sp (just after call to setjmp())
	
	;;; Pop setjmp() caller's return addr  (Note: addr isn't there any more)
if @codesize
	add sp,4
else
	add sp,2
endif

	;;; Set up flags and far return address on stack in preparation for IRET
	push [bx+12h]  ; flags
	push [bx+10h]  ; ret_seg
	push [bx+0Eh]  ; ret_off

	;;; Done using ds, so restore setjmp() caller's
	mov ds,[bx+08h]  ; ds

	;;; Return to setjmp() caller and restore flags in a single blow
	;;; Note: use IRET instead of POPF, because POPF doesn't work correctly
	;;;   on 286 when interrupts are disabled.
	iret
_longjmp_nck endp

;===========================================================================
;                       Alarm functions
;

public _alarm_$reenter_cnt

if @codesize
extrn _alarm_$handler:far
else
extrn _alarm_$handler:near
endif

;;; Note these are in this **code** segment
_in_isr               DW 0
_old_timer_isr        DD 0
_alarm_stack_top      DD 0
_alarm_stack_bottom   DW 0
_tick_count           DD 0
_alarm_$reenter_cnt   DD 0

; alarm_$isr()
; Catches the 18.2 Hz hardware clock interrupt directly, fakes a call to
; the BIOS handler, arranging the stack so that the BIOS will return to
; alarm_$isr().  After the BIOS (and the INT 1Ch timer chain) runs, the
; stack is switched to a private NCK interrupt stack and the C-lang handler
; is called.  A flag is set around the call to alarm_$handler to prevent
; reentry.  If alarm_$handler takes longer than 55msec and another timer
; interrupt occurs, the timer chain will still be executed but alarm_$handler
; will not be called.
; - Note that the NCK alarm enabling/disabling mechanism is not provided at
; this level.  Alarm_$handler implements this mechanism.
; - The previous version of the interrupt catching inserted itself in the
; INT 1Ch timer chain.  However sometimes the processing done at interrupt
; time took longer than 55msec.  Since the EOI command is not sent to the
; interrupt controller chip until after the interrupt chain is executed 
; (in the IBM BIOS version), all other interrupts including the timer 
; interrupt are effectively disabled and timer ticks are lost.  This causes 
; the time-of-day to slip.  Also, since packets are sent and received in the 
; interrupt, if the network code required interrupts to be enabled in order 
; to complete, the computer would hang.  Since the network dependent code
; can be provided by a 3rd party which may not be prepared to deal with the
; above issues, the current interrupt scheme seems safer.

_alarm_$isr proc far	; must be far procedure
	;;; Increment interrupt counter
	;;; Note: don't need to save flags since will be restored on IRET
	add word ptr cs:[_tick_count],1
	adc word ptr cs:[_tick_count+2],0

	;;; Fake an interrupt to BIOS timer ISR
	;;; After BIOS handler runs, will return here
	pushf
	call dword ptr cs:[_old_timer_isr]

	;;; By now CPU and INT controller chip should both be enabled and
	;;; all TSRs in the sysclock interrupt chain have run.

	;;; Disable interrupts and test if would reenter the alarm_$handler
	cli
	test cs:[_in_isr],1
	jz not_reentering

is_reentering:
	;;; Increment long counter of number of times tried (and failed) to reenter 
	add word ptr cs:[_alarm_$reenter_cnt],1
	adc word ptr cs:[_alarm_$reenter_cnt+2],0

	;;; Return from interrupt, since not safe to call C-lang handler
	iret

not_reentering:
	;;; Set flag so will detect any later attempt to reenter
	mov cs:[_in_isr],1

	;;; Push some registers on interrupted thread's stack
	push ax
	push bx
	
	;;; Get new stack (alarm_stack_top points to it)
	;;; Note interrupts are disabled while messing with stack.
	mov ax,ss
	mov bx,sp
	mov ss,word ptr cs:[_alarm_stack_top+2]
	mov sp,word ptr cs:[_alarm_stack_top]

	;;; Safe now to reenable interrupts
	sti

	;;; Push old stack address on new stack
	push ax		; old ss
	push bx		; old sp

	;;; Push rest of regs on new alarm stack
	push cx
	push dx
	push bp
	push si
	push di
	push ds
	push es

	;;; Set up segments
	mov cx,DGROUP
	mov ds,cx
	mov es,cx		; prob not necc.

	;;; Set new limit for stack probe
	push [STKHQQ]	; save old value
	mov ax,cs:[_alarm_stack_bottom]
	mov [STKHQQ],ax

	;;; Call the C-lang handler
	call _alarm_$handler

	;;; Restore old stack limit for stack probe
	pop [STKHQQ]

	;;; Restore registers from alarm stack
	pop es
	pop ds
	pop di
	pop si
	pop bp
	pop dx
	pop cx

	;;; Restore old stack
	cli			; disable interrupts while manipulating stack
	pop bx		; old sp
	pop ax		; old ss
	mov ss,ax
	mov sp,bx	
	sti			; reenable interrupts

	;;; Reset reentering flag, safe to handle another interrupt now
	mov cs:[_in_isr],0

	;;; Restore registers from interrupted thread's stack
	pop bx
	pop ax

	iret
_alarm_$isr endp

; alarm_$open(stack_top, stack_bottom)
; char* stack_top
; char* stack_bottom
; - should be only called once
; - alarm stack must be in DGROUP
; - not callable if interrupts are disabled (will enable them)
CPROC alarm_$open
	push es

	;;; Stash value of stack top for use in interrupt
	mov bx,arg1
	mov word ptr cs:[_alarm_stack_top],bx
if @datasize
	mov bx,arg2
	mov word ptr cs:_alarm_stack_top+2,bx
else
	mov word ptr cs:[_alarm_stack_top+2],ds
endif

	;;; Stash value of stack bottom offset for use in interrupt
if @datasize
	mov bx,arg3
else
	mov bx,arg2
endif
 	add bx,512	; Add some slop (to report error, cleanup, and exit)
	mov cs:[_alarm_stack_bottom],bx

	xor bx,bx
	mov es,bx	; es = 0

	cli			; disable interrupt while swap vectors

	;;; Save old INT vector
	;;; Probably should use msdos functions 0x35 and 0x25 here ???
	mov bx,word ptr es:[08h*4]
	mov word ptr cs:[_old_timer_isr],bx
	mov bx,word ptr es:[08h*4+2]
	mov word ptr cs:[_old_timer_isr+2],bx

	;;; Revector INT to alarm_isr()
	mov bx,OFFSET _alarm_$isr
	mov word ptr es:[08h*4],bx
	mov bx,SEG _alarm_$isr
	mov word ptr es:[08h*4+2],bx

	sti			; reenable interrupts

	pop es
CRET alarm_$open

; alarm_$close()
; - stop catching system clock interrupt, restoring old state
; - does NOT check if someone else has also intercepted the interrupt after
;   alarm_$open.
; - not callable if interrupts are disabled (will enable them)
CPROC alarm_$close
	push es

	xor bx,bx
	mov es,bx	; es = 0

	cli			; disable interrupt while restore vector

	;;; Restore old interrupt vector (assume it's still active)
	mov bx,word ptr cs:[_old_timer_isr]
	mov word ptr es:[08h*4],bx
	mov bx,word ptr cs:[_old_timer_isr+2]
	mov word ptr es:[08h*4+2],bx

	sti			; reenable interrupts

	pop es
CRET alarm_$close

; long alarm_$time()
; - not callable if interrupts are disabled (will enable them)
CPROC alarm_$time
	cli			; disable interrupts to get consistent time
	mov ax,word ptr cs:[_tick_count]
	mov dx,word ptr cs:[_tick_count+2]
	sti			; reenable interrupts
CRET alarm_$time

;===========================================================================
;			Xlong (64 bit arithmetic)
;

; xlong_$add_long(xl, l)
; xlong* xl
; u_long l
; - add 64 bit and 32 bit numbers
CPROC xlong_$add_long
	push di
if @datasize
	push ds
	lds di,arg1
	mov ax,arg3
	mov bx,arg4
else
	mov di,arg1
	mov ax,arg2
	mov bx,arg3
endif

	add [di],ax
	adc [di+2],bx
	adc word ptr [di+4],0
	adc word ptr [di+6],0

if @datasize
	pop ds
endif
	pop di
CRET xlong_$add_long

; xlong_$add_xlong(xl, xl2)
; xlong* xl
; xlong* xl2
; - add two 64 bit numbers
; - result stored in xl, ignore overflow
CPROC xlong_$add_xlong
	push di
if @datasize
	push ds
	lds di,arg3
	mov ax,[di]
	mov bx,[di+2]
	mov cx,[di+4]
	mov dx,[di+6]
	lds di,arg1
else
	mov di,arg2
	mov ax,[di]
	mov bx,[di+2]
	mov cx,[di+4]
	mov dx,[di+6]
	mov di,arg1
endif

	add [di],ax
	adc [di+2],bx
	adc [di+4],cx
	adc [di+6],dx

if @datasize
	pop ds
endif
	pop di
CRET xlong_$add_xlong

; xlong_$mult_long(xl, l)
; xlong* xl
; u_long l
; - multiply 64 bit number by 32 bit number with 64 bit result
; - don't worry about overflow
CPROC xlong_$mult_long
	use_local 4		; 8 bytes for tmp
	push di
if @datasize
	push ds
	lds di,arg1
	mov bx,arg3
	mov cx,arg4
else
	mov di,arg1
	mov bx,arg2
	mov cx,arg3
endif

	;;; Mult with low word
	mov ax,bx		; tmp = l[0] * xl[0]
	mul word ptr [di]
	mov auto1,ax
	mov auto2,dx
	xor ax,ax
	mov auto3,ax
	mov auto4,ax

	mov ax,bx		; tmp += (l[0] * xl[1]) << 16
	mul word ptr [di+2]
	add auto2,ax
	adc auto3,dx
	adc auto4,0

	mov ax,bx		; tmp += (l[0] * xl[2]) << 32
	mul word ptr [di+4]
	add auto3,ax
	adc auto4,dx

	mov ax,bx		; tmp += (l[0] * xl[3]) << 48
	mul word ptr [di+6]
	add auto4,ax

	;;; Mult with high word
	mov ax,cx		; tmp += (l[1] * xl[0]) << 16
	mul word ptr [di]
	add auto2,ax
	adc auto3,dx
	adc auto4,0

	mov ax,cx		; tmp += (l[1] * xl[1]) << 32
	mul word ptr [di+2]
	add auto3,ax
	adc auto4,dx

	mov ax,cx		; tmp += (l[1] * xl[2]) << 48
	mul word ptr [di+4]
	add auto4,ax

	mov ax,auto1	; copy tmp to xl
	mov [di],ax
	mov ax,auto2
	mov [di+2],ax
	mov ax,auto3
	mov [di+4],ax
	mov ax,auto4
	mov [di+6],ax

if @datasize
	pop ds
endif
	pop di
CRET xlong_$mult_long

;===========================================================================
;                        Swap bytes routines
;

; swab_$short(s)
; short s
CPROC swab_$short
	mov ax,arg1
	xchg al,ah
CRET swab_$short

; swab_$long(l)
; long l
CPROC swab_$long
	mov ax,arg1
	mov dx,arg2
	xchg al,dh
	xchg ah,dl
CRET swab_$long


SWAB_16 macro
	mov ax,[di]
	xchg al,ah
	mov [di],ax
	add di,2
endm

SWAB_32 macro
	mov ax,[di]
	mov bx,[di+2]
	xchg al,bh
	xchg ah,bl
	mov [di],ax
	mov [di+2],bx
	add di,4
endm

SWAB_UUID macro
	SWAB_32
	SWAB_16
	add di,10
endm

; rpc_$swab_header(pkt)
; rpc_$pkt_t* pkt
; - swap bytes of fields larger than 1 byte in NCS packet
CPROC rpc_$swab_header
	push di
if @datasize
	push ds
	lds di,arg1
else
	mov di,arg1
endif

	add di,8
	SWAB_UUID
	SWAB_UUID
	SWAB_UUID
	SWAB_32
	SWAB_32
	SWAB_32
	SWAB_16
	SWAB_16
	SWAB_16
	SWAB_16
	SWAB_16

if @datasize
	pop ds
endif
	pop di
CRET rpc_$swab_header

end
