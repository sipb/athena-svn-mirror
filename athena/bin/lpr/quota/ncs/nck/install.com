$! ========================================================================== 
$! Copyright  1987 by Apollo Computer Inc., Chelmsford, Massachusetts
$! 
$! All Rights Reserved
$! 
$! All Apollo source code software programs, object code software programs,
$! documentation and copies thereof shall contain the copyright notice above
$! and this permission notice.  Apollo Computer Inc. reserves all rights,
$! title and interest with respect to copying, modification or the
$! distribution of such software programs and associated documentation,
$! except those rights specifically granted by Apollo in a Product Software
$! Program License or Source Code License between Apollo and Licensee.
$! Without this License, such software programs may not be used, copied,
$! modified or distributed in source or object code form.  Further, the
$! copyright notice must appear on the media, the supporting documentation
$! and packaging.  A Source Code License does not grant any rights to use
$! Apollo Computer's name or trademarks in advertising or publicity, with
$! respect to the distribution of the software programs without the specific
$! prior written permission of Apollo.  Trademark agreements may be obtained
$! in a separate Trademark License Agreement.
$! 
$! Apollo disclaims all warranties, express or implied, with respect to
$! the Software Programs including the implied warranties of merchantability
$! and fitness, for a particular purpose.  In no event shall Apollo be liable
$! for any special, indirect or consequential damages or any damages
$! whatsoever resulting from loss of use, data or profits whether in an
$! action of contract or tort, arising out of or in connection with the
$! use or performance of such software programs.
$! ========================================================================== 
$!
$! VMS DCL script.
$!
$! Installation script for NCK.
$!
$! This script should be executed from either the staging area top level directory.
$!
$! Define the logical names:
$!
$!      - NCS$EXE to point to the directory to hold NCS executables.
$!      - NCS$LIB to point to the directory to hold NCK libraries.
$!      - NCS$INCLUDE to point to the directory to hold NCK include files.
$!
$! usage: @install <network type>
$!
$ do_exit = ""
$ if p1 .eqs. "TWG" .or p1 .eqs. "EXCELAN" .or p1 .eqs. "UCX" then goto l1
$ usage: write sys$output "usage: install [twg | excelan | ucx]"
$ exit   
$ l1:
$ 
$ chk_exe:
$ exe_dir = f$trnlnm("ncs$exe")
$ if exe_dir .nes. "" then goto chk_lib
$   write sys$output "Please define NCS$EXE"
$   do_exit := "exit"
$
$ chk_lib:
$ lib_dir = f$trnlnm("ncs$lib")
$ if lib_dir .nes. "" then goto chk_inc
$   write sys$output "Please define NCS$LIB"
$   do_exit = "exit"
$
$ chk_inc:
$ inc_dir = f$trnlnm("ncs$include")
$ if inc_dir .nes. "" then goto chk_man
$   write sys$output "Please define NCS$INCLUDE"
$   do_exit = "exit"
$
$ chk_man:
$ man_dir = f$trnlnm("ncs$man")
$ if man_dir .nes. "" then goto chk_idl
$   write sys$output "Please define NCS$MAN"
$   do_exit = "exit"
$
$ chk_idl:
$ idl_dir = f$trnlnm("ncs$idl")
$ if idl_dir .nes. "" then goto chk_idl_c
$   write sys$output "Please define NCS$IDL"
$   do_exit = "exit"
$
$ chk_idl_c:
$ idl_c_dir = f$trnlnm("ncs$idl_c")
$ if idl_c_dir .nes. "" then goto do_links
$   write sys$output "Please define NCS$IDL_c"
$   do_exit = "exit"
$
$ do_links:
$ if do_exit .eqs. "exit" then exit
$ set def [.nck]
$ 
$ write sys$output "*****************************************************************"
$ write sys$output "*"
$ write sys$output "*   Installing VMS NCK/''p1' to:"
$ write sys$output "*"
$ write sys$output "*       NCS$EXE     => " + exe_dir
$ write sys$output "*       NCS$LIB     => " + lib_dir
$ write sys$output "*       NCS$INCLUDE => " + inc_dir
$ write sys$output "*       NCS$MAN     => " + man_dir
$ write sys$output "*       NCS$IDL     => " + idl_dir
$ write sys$output "*       NCS$IDL_C   => " + idl_c_dir
$ write sys$output "*"
$ write sys$output "*****************************************************************"
$ write sys$output ""
$
$ exe_pfx = f$extract(0, f$length(exe_dir) - 1, exe_dir)
$ 
$ copy/log libnck.*         ncs$lib:
$ copy/log lib'p1'.*        ncs$lib:
$ copy/log stcode.exe       ncs$exe:
$ copy/log stcode.db        ncs$exe:
$ copy/log [-.idl]pfm.h     ncs$include:
$ copy/log [-.idl]ppfm.h    ncs$include:
$ copy/log [-.man.cat8]*.*  ncs$man:
$ copy/log [-.idl]*.idl     ncs$idl:
$ copy/log [-.idl]*.h       ncs$idl_c:
$ delete ncs$idl_c:pfm.h;*
$ delete ncs$idl_c:ppfm.h;*
$
$ write sys$output "*****************************************************************"
$ write sys$output "*"
$ write sys$output "*   Linking NCK utilities (LB_ADMIN, LLBD, NRGLBD, UUID_GEN)."
$ write sys$output "*"
$ write sys$output "*****************************************************************"
$
$ if f$search("''p1'.dir") .eqs "" then create /dir [.'p1']
$ @link_utl 'p1'
$
$ copy/log [.'p1']*.exe ncs$exe:
$
$ lb_admin :== $ ncs$exe:lb_admin.exe
$ uuid_gen :== $ ncs$exe:uuid_gen.exe
$
$ write sys$output "*****************************************************************"
$ write sys$output "*"
$ write sys$output "*   Linking the perf test (CLIENT, SERVER)."
$ write sys$output "*"
$ write sys$output "*****************************************************************"
$
$ set def [-.perf]
$ if f$search("''p1'.dir") .eqs "" then create /dir [.'p1']
$ @link 'p1'
$
$ perfexe = exe_pfx + ".perf]"
$ if f$search("ncs$exe:perf.dir") .eqs "" then create /dir 'perfexe'
$
$ copy/log [.'p1']*.exe 'perfexe'
$ copy/log run_client.com 'perfexe'
$
$ client == "$ " + perfexe + "client.exe"
$ server == "$ " + perfexe + "server.exe"
$
$ write sys$output "*****************************************************************"
$ write sys$output "*"
$ write sys$output "*   Linking the lb test (LB_TEST)."
$ write sys$output "*"
$ write sys$output "*****************************************************************"
$
$ set def [-.lb_test]
$ if f$search("''p1'.dir") .eqs "" then create /dir [.'p1']
$ @link 'p1'
$
$ copy/log [.'p1]*.exe ncs$exe:
$
$ lb_test :== $ ncs$exe:lb_test.exe
$
$ write sys$output "*****************************************************************"
$ write sys$output "*"
$ write sys$output "*   NCK has now been installed."
$ write sys$output "*"
$ write sys$output "*   You should add the following to your system startup script:"
$ write sys$output "*"
$ write sys$output "*   $ DEFINE /SYSTEM NCS$EXE     " + exe_dir
$ write sys$output "*   $ DEFINE /SYSTEM NCS$LIB     " + lib_dir
$ write sys$output "*   $ DEFINE /SYSTEM NCS$INCLUDE " + inc_dir
$ write sys$output "*   $ RUN /DETACHED /PROCESS_NAME=LLBD /PRIV=CMKRNL NCS$EXE:LLBD"
$ write sys$output "*"
$ write sys$output "*   You should add the following to your system login script:"
$ write sys$output "*"
$ write sys$output "*   $ LB_ADMIN :== $ NCS$EXE:LB_ADMIN.EXE"
$ write sys$output "*   $ UUID_GEN :== $ NCS$EXE:UUID_GEN.EXE"
$ write sys$output "*   $ STCODE   :== $ NCS$EXE:STCODE.EXE"
$ write sys$output "*"
$ write sys$output "*****************************************************************"
$
$ set def [-]
$
$ purge ncs$exe:
$ purge ncs$lib:
$ purge ncs$include:
$ purge ncs$man:
$ purge ncs$idl:
$ purge ncs$idl_c:
