$!  
$!  Install the NIDL compiler
$! 
$!  ========================================================================== 
$!  Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
$!  Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
$!  Copyright Laws Of The United States.
$! 
$!  Apollo Computer Inc. reserves all rights, title and interest with respect 
$!  to copying, modification or the distribution of such software programs and
$!  associated documentation, except those rights specifically granted by Apollo
$!  in a Product Software Program License, Source Code License or Commercial
$!  License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
$!  Licensee.  Without such license agreements, such software programs may not
$!  be used, copied, modified or distributed in source or object code form.
$!  Further, the copyright notice must appear on the media, the supporting
$!  documentation and packaging as set forth in such agreements.  Such License
$!  Agreements do not grant any rights to use Apollo Computer's name or trademarks
$!  in advertising or publicity, with respect to the distribution of the software
$!  programs without the specific prior written permission of Apollo.  Trademark 
$!  agreements may be obtained in a separate Trademark License Agreement.
$!  ========================================================================== 
$! 
$ write sys$output "*************************************************************"
$ write sys$output "*                                                           *"
$ write sys$output "*    Installing NIDL                                        *"
$ write sys$output "*                                                           *"
$ write sys$output "*   Before this installation can successfully complete,     *"
$ write sys$output "*   the following logical names need to be defined:         *"
$ write sys$output "*                                                           *"
$ write sys$output "*       ncs$exe     - directory for NCS executables         *"
$ write sys$output "*       ncs$include - directory for NCS header files        *"
$ write sys$output "*       ncs$idl     - directory for NCS .idl files          *"
$ write sys$output "*       ncs$idl_c   - directory for header files generated  *"
$ write sys$output "*                     from the .idl files                   *"
$ write sys$output "*       ncs$man     - directory for NCS man pages           *"
$ write sys$output "*                                                           *"
$ write sys$output "*************************************************************"
$!
$ exists = f$logical("ncs$exe")
$ if exists .nes. "" then goto try_include
$   write sys$output "***"
$   write sys$output "*** Please define ncs$exe:"
$   write sys$output "***"
$!
$ try_include:
$ exists = f$logical("ncs$include")
$ if exists .nes. "" then goto try_idl
$   write sys$output "***"
$   write sys$output "*** Please define ncs$include:"
$   write sys$output "***"
$!
$ try_idl:
$ exists = f$logical("ncs$idl")
$ if exists .nes. "" then goto try_idl_c
$   write sys$output "***"
$   write sys$output "*** Please define ncs$idl:"
$   write sys$output "***"
$!
$ try_idl_c:
$ exists = f$logical("ncs$idl_c")
$ if exists .nes. "" then goto try_man
$   write sys$output "***"
$   write sys$output "*** Please define ncs$idl_c:"
$   write sys$output "***"
$!
$ try_man:
$ exists = f$logical("ncs$man")
$ if exists .nes. "" then goto do_copies
$   write sys$output "***"
$   write sys$output "*** Please define ncs$man:"
$   write sys$output "***"
$!
$ exit
$!
$ do_copies:
$ write sys$output "Copying files into ncs$exe, ncs$idl, ncs$man:"
$ set ver
$ copy [.nidl]nidl.exe    ncs$exe
$ copy [.idl]*.idl        ncs$idl
$ copy [.idl]*.h          ncs$idl_c
$ copy [.idl]pfm.h        ncs$include
$ copy [.idl]ppfm.h       ncs$include
$ copy [.man.man1]*.1     ncs$man
$ copy [.man.cat1]*.1     ncs$man
$ delete ncs$idl_c:pfm.h;*
$ delete ncs$idl_c:ppfm.h;*
$ set nover
$ write sys$output "*************************************************************"
$ write sys$output "*                                                           *"
$ write sys$output "*    NIDL has now been installed                            *"
$ write sys$output "*                                                           *"
$ write sys$output "*    Don't forget to add the ncs logical names              *"
$ write sys$output "*    your system startup  script                            *"
$ write sys$output "*                                                           *"
$ write sys$output "*************************************************************"
