$! ========================================================================== 
$! Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
$! Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
$! Copyright Laws Of The United States.
$! 
$! Apollo Computer Inc. reserves all rights, title and interest with respect
$! to copying, modification or the distribution of such software programs
$! and associated documentation, except those rights specifically granted
$! by Apollo in a Product Software Program License, Source Code License
$! or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
$! Apollo and Licensee.  Without such license agreements, such software
$! programs may not be used, copied, modified or distributed in source
$! or object code form.  Further, the copyright notice must appear on the
$! media, the supporting documentation and packaging as set forth in such
$! agreements.  Such License Agreements do not grant any rights to use
$! Apollo Computer's name or trademarks in advertising or publicity, with
$! respect to the distribution of the software programs without the specific
$! prior written permission of Apollo.  Trademark agreements may be obtained
$! in a separate Trademark License Agreement.
$! ========================================================================== 
$!
$! VMS DCL script.
$!
$! Make an NCK staging area.
$!
$! A sensible thing to do to back up a staging area is something like:
$!
$!      SET FILE [-]STAGE.DIR/ENTER=[000000]NCK-1.5.1-DISTRIBUTION.DIR
$!      BACKUP [NCK-1.5.1-DISTRIBUTION...] MUA0:
$!      SET FILE [000000]NCK-1.5.1-DISTRIBUTION.DIR/REMOVE
$!
$! This will result in "pretty" names being in the backup save set.
$!
$ if f$search("[-]stage.dir") .eqs. "" then create /dir/log [-.stage]
$
$ copy /log [-]cpyright [-.stage]
$
$ copy /log [-.nck]install.com [-.stage]
$!
$! Copy the NCK runtime
$!
$ if f$search("[-.stage]nck.dir") .eqs. "" then create /dir/log [-.stage.nck]
$ copy /log [-.nck]*.com,[-.nck]*.olb,*.opt [-.stage.nck]
$ copy /log [-.nck]stcode.exe [-.stage.nck]
$ copy /log [-.nck]stcode.db [-.stage.nck]
$!
$! Copy the IDL files
$!
$ if f$search("[-.stage]idl.dir") .eqs. "" then create /dir/log [-.stage.idl]
$ copy /log [-.idl]*.idl,[-.idl]*.h [-.stage.idl]
$!
$! Copy perf
$!
$ if f$search("[-.stage]perf.dir") .eqs. "" then create /dir/log [-.stage.perf]
$ copy /log [-.perf]*.olb,[-.perf]*.com [-.stage.perf]
$!
$! Copy lb_test
$!
$ if f$search("[-.stage]lb_test.dir") .eqs. "" then create /dir/log [-.stage.lb_test]
$ copy /log [-.lb_test]*.olb,[-.lb_test]*.com [-.stage.lb_test]
$!
$! Copy man pages
$!
$ if f$search("[-.stage]man.dir") .eqs. "" then create /dir/log [-.stage.man]
$ if f$search("[-.stage.man]man8.dir") .eqs. "" then create /dir/log [-.stage.man.man8]
$ copy /log [-.man.man8]*.* [-.stage.man.man8]
$ if f$search("[-.stage.man]cat8.dir") .eqs. "" then create /dir/log [-.stage.man.cat8]
$ copy /log [-.man.cat8]*.* [-.stage.man.cat8]
$
$ copy /log [-.man.doc]vms.rd [-.stage]readme.txt
$ purge [-.stage...]
