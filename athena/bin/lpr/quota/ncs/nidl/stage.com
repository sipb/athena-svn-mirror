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
$! VMS DCL script.
$!
$! Make a NIDL staging area.
$!
$! A sensible thing to do to back up a staging area is something like:
$!
$!      SET FILE [-]STAGE.DIR/ENTER=[000000]NIDL-DISTRIBUTION.DIR
$!      BACKUP [NIDL-DISTRIBUTION...] MUA0:
$!      SET FILE [000000]NIDL-DISTRIBUTION.DIR/REMOVE
$!
$! This will result in "pretty" names being in the backup save set.
$!
$ if f$search("[-]stage.dir")                         .eqs. "" then create /dir/log [-.stage]
$ if f$search("[-.stage]nidl.dir")                    .eqs. "" then create /dir/log [-.stage.nidl]
$ if f$search("[-.stage]idl.dir")                     .eqs. "" then create /dir/log [-.stage.idl]
$ if f$search("[-.stage]man.dir")                     .eqs. "" then create /dir/log [-.stage.man]
$ if f$search("[-.stage.man]cat1.dir")                .eqs. "" then create /dir/log [-.stage.man.cat1]
$ if f$search("[-.stage.man]man1.dir")                .eqs. "" then create /dir/log [-.stage.man.man1]
$ if f$search("[-.stage]examples.dir")                .eqs. "" then create /dir/log [-.stage.examples]
$ if f$search("[-.stage.examples]bank.dir")           .eqs. "" then create /dir/log [-.stage.examples.bank]
$ if f$search("[-.stage.examples]binop.dir")          .eqs. "" then create /dir/log [-.stage.examples.binop]
$ if f$search("[-.stage.examples.binop]binop_fw.dir") .eqs. "" then create /dir/log [-.stage.examples.binop.binop_fw]
$ if f$search("[-.stage.examples.binop]binop_lu.dir") .eqs. "" then create /dir/log [-.stage.examples.binop.binop_lu]
$ if f$search("[-.stage.examples.binop]binop_wk.dir") .eqs. "" then create /dir/log [-.stage.examples.binop.binop_wk]
$ if f$search("[-.stage.examples]lb_test.dir")        .eqs. "" then create /dir/log [-.stage.examples.lb_test]
$ if f$search("[-.stage.examples]mandel.dir")         .eqs. "" then create /dir/log [-.stage.examples.mandel]
$ if f$search("[-.stage.examples]nidltest.dir")       .eqs. "" then create /dir/log [-.stage.examples.nidltest]
$ if f$search("[-.stage.examples]perf.dir")           .eqs. "" then create /dir/log [-.stage.examples.perf]
$ if f$search("[-.stage.examples]rrpc.dir")           .eqs. "" then create /dir/log [-.stage.examples.rrpc]
$ if f$search("[-.stage.examples]splash.dir")         .eqs. "" then create /dir/log [-.stage.examples.splash]
$!
$ copy /log [-]cpyright.                   [-.stage]
$ copy /log install.com                    [-.stage]
$ copy /log nidl.exe                       [-.stage.nidl]
$!
$ copy /log [-.idl]*.idl                   [-.stage.idl]
$ copy /log [-.idl]*.h                     [-.stage.idl]
$!
$ copy /log [-.man.man1]*.1                [-.stage.man.man1]
$ copy /log [-.man.cat1]*.1                [-.stage.man.cat1]
$ copy /log [-.man.doc]vms.rd              [-.stage]readme.txt
$!
$ copy /log [-.examples]readme.            [-.stage.examples]
$ copy /log [-.examples.bank]*.*           [-.stage.examples.bank]
$ copy /log [-.examples.binop.binop_fw]*.* [-.stage.examples.binop.binop_fw]
$ copy /log [-.examples.binop.binop_lu]*.* [-.stage.examples.binop.binop_lu]
$ copy /log [-.examples.binop.binop_wk]*.* [-.stage.examples.binop.binop_wk]
$ copy /log [-.examples.lb_test]*.*        [-.stage.examples.lb_test]
$ copy /log [-.examples.mandel]*.*         [-.stage.examples.mandel]
$ copy /log [-.examples.nidltest]*.*       [-.stage.examples.nidltest]
$ copy /log [-.examples.perf]*.*           [-.stage.examples.perf]
$ copy /log [-.examples.rrpc]*.*           [-.stage.examples.rrpc]
$ copy /log [-.examples.splash]*.*         [-.stage.examples.splash]
$!
$ purge [-.stage...]
