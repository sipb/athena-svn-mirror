* ========================================================================== 
* Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
* Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
* Copyright Laws Of The United States.
* 
* Apollo Computer Inc. reserves all rights, title and interest with respect
* to copying, modification or the distribution of such software programs
* and associated documentation, except those rights specifically granted
* by Apollo in a Product Software Program License, Source Code License
* or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
* Apollo and Licensee.  Without such license agreements, such software
* programs may not be used, copied, modified or distributed in source
* or object code form.  Further, the copyright notice must appear on the
* media, the supporting documentation and packaging as set forth in such
* agreements.  Such License Agreements do not grant any rights to use
* Apollo Computer's name or trademarks in advertising or publicity, with
* respect to the distribution of the software programs without the specific
* prior written permission of Apollo.  Trademark agreements may be obtained
* in a separate Trademark License Agreement.
* ========================================================================== 


        module  set_a0_hack

*
* This is a hack THAT WILL GO AWAY.  This procedure takes its only argument
* and moves it into A0.  This procedure is called from C code that wants to
* return a pointer.  Currently, C returns all values in D0.  Unfortunately,
* Pascal expects functions that return pointers to do so into A0.  You get
* the idea, right?
*
%IF ISP_$A88K %THEN
    end
    %EXIT
%ENDIF

        entry.p set_a0_hack

set_a0_hack equ *
        move.l  4(sp),a0
        rts

        end
