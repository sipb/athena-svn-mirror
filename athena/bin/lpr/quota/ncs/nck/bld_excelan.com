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
$! Build Excelan-specific modules.
$!  
$ write sys$output "Making LIBEXCELAN"
$
$ if f$search("excelan.dir") .eqs. "" then create /dir [.excelan]
$
$ cc_defines := EXCELAN
$
$ @bld1 socket_inet     libexcelan  [.excelan]
$ @bld1 vms_excelan     libexcelan  [.excelan]
$ @bld1 vms_select      libexcelan  [.excelan]
$
$ if f$search("rhost.c")     .eqs. "" then copy exos$etc:rhost.c rhost.c
$ if f$search("raddr.c")     .eqs. "" then copy exos$etc:raddr.c raddr.c
$ if f$search("exos_util.c") .eqs. "" then copy exos$etc:util.c  exos_util.c
$
$ @bld1 rhost           libexcelan  [.excelan]
$ @bld1 raddr           libexcelan  [.excelan]
$ @bld1 exos_util       libexcelan  [.excelan]

