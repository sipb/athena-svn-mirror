$!
$!  Bldall.com - build all NIDL source files and link to produce
$!               an executable NIDL
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
$ write sys$output "*****************************************"
$ write sys$output "*                                       *"
$ write sys$output "*  Starting NIDL compiler build         *
$ write sys$output "*                                       *"
$ write sys$output "*****************************************"
$ idl_dir = f$parse("[-.idl]",,,"DIRECTORY")
$ define idl$dev 'idl_dir'
$ @bld1 astp
$ @bld1 backend
$ @bld1 checker
$ @bld1 cspell
$ @bld1 errors
$ @bld1 files
$ @bld1 frontend
$ @bld1 getflags
$ @bld1 main
$ @bld1 nametbl
$ @bld1 pspell
$ @bld1 sysdep
$ @bld1 utils
$ @bld1 lex_yy
$ @bld1 y_tab
$ @link_nidl                                                  
$ deassign idl$dev
$ write sys$output "*****************************************"
$ write sys$output "*                                       *"
$ write sys$output "*   NIDL compiler successfully built    *"
$ write sys$output "*                                       *"
$ write sys$output "*****************************************"
