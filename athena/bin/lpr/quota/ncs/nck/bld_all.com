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
$! Build the NCK subsystem
$!
$
$ have_ucx     = 0
$ have_excelan = 0
$ have_twg     = 0
$
$ if f$search("sys$manager:ucx$startup.com") .nes. "" then have_ucx     = 1
$ if f$trnlnm("twg$tcp")                     .nes. "" then have_twg     = 1
$ if f$trnlnm("exos$etc")                    .nes. "" then have_excelan = 1
$ 
$ if have_ucx .or. have_twg then goto bldit
$ write sys$output "You must have at least one of the UCX or TWG TCP/IP software installed to build NCK"
$ exit  
$ 
$ bldit:
$ @build
$
$ write sys$output "Building PERF"
$ set default [-.perf]
$ if have_ucx     then @build ucx     [-.idl] [-.nck]
$ if have_twg     then @build twg     [-.idl] [-.nck]
$ if have_excelan then @build excelan [-.idl] [-.nck]
$
$ write sys$output "Building LB_TEST"
$ set default [-.lb_test]
$ if have_ucx     then @build ucx     [-.idl] [-.nck]
$ if have_twg     then @build twg     [-.idl] [-.nck]
$ if have_excelan then @build excelan [-.idl] [-.nck]
