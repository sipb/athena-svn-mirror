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
; M S _ F T P A
;
; MS/DOS / INET-family / FTP Software -specific assembly language routines.  (MS/DOS)
;
; Currently, the only purpose of this module is to "includelib" the FTP
; libraries (lnetlib, lpc, lsocket) so that anyone who binds with nck_ftp.lib
; will automatically pick up the FTP socket library as well (assuming they
; defined the LIB environment variable correctly).  Too bad you can't do
; this from C.
;

dosseg
.model large

includelib lsocket
includelib lnetlib
includelib lpc

;
; Declare a dummy variable (and reference it in ms_ftp.c so that this module
; will get dragged in by anyone binding with nck_ftp.lib.
;

.data

public _ms_ftpa_dummy
_ms_ftpa_dummy   db  0

end
