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
$! Compile all the NCK base modules.
$!  
$ write sys$output "Making LIBNCK"
$ @bld1 float               libnck
$ @bld1 rpc_client          libnck
$ @bld1 rpc_server          libnck
$ @bld1 rpc_lsn             libnck
$ @bld1 rpc_util            libnck
$ @bld1 socket              libnck
$ @bld1 uuid                libnck
$ @bld1 u_pfm               libnck
$ @bld1 uname               libnck
$ @bld1 balanced_trees      libnck
$ @bld1 vms                 libnck
$ @bld1 rpc_seq             libnck
$ @bld1 error               libnck
$ @bld1 glb                 libnck
$ @bld1 lb                  libnck
$ @bld1 llb                 libnck
$ @bld1 [-.idl]conv_sstub   libnck
$ @bld1 [-.idl]conv_cstub   libnck
$ @bld1 [-.idl]rrpc_cstub   libnck
$ @bld1 [-.idl]rrpc_sstub   libnck
$ @bld1 [-.idl]llb_cstub    libnck
$ @bld1 [-.idl]llb_cswtch   libnck
$ @bld1 [-.idl]glb_cstub    libnck
$ @bld1 [-.idl]glb_cswtch   libnck
$
$ write sys$output "Making LIBUUID"
$ @bld1 uuid_gen            libuuid
$
$ write sys$output "Making LIBGLBD"
$ @bld1 glbd                libglbd
$ @bld1 glb_man             libglbd
$ @bld1 [-.idl]glb_sstub    libglbd
$
$ write sys$output "Making LIBLLBD"
$ @bld1 llbd                libllbd
$ @bld1 llb_man             libllbd
$ @bld1 [-.idl]llb_sstub    libllbd
$
$ write sys$output "Making LIBLBA"
$ @bld1 lb_admin            liblba 
$ @bld1 lb_args             liblba
$
$ write sys$output "Compiling STCODE and MKEDB"
$ @bld1 stcode
$ @bld1 mkedb
