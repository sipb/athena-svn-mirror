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
$! Build the NCK runtime
$!
$ write sys$output "Building NCK"
$
$ @bld_nck
$ @link_stcode
$
$ if f$search("sys$manager:ucx$startup.com") .eqs. "" then goto l2          
$ @bld_ucx
$ @link_utl ucx
$ l2:
$
$ if f$trnlnm("twg$tcp") .eqs. "" then goto l3
$ @bld_twg
$ @link_utl twg
$ l3:
$
$ if f$trnlnm("exos$etc") .eqs. "" then goto l4
$ @bld_excelan
$ @link_utl excelan
$ l4:
$
$ write sys$output "Building status code/text database"
$ mkedb = "$ " + f$environment("default") + "mkedb.exe"
$ mkedb errors.txt stcode.db
