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
$! Compile a single file if needed and optionally insert result into a library.
$!
$! usage: @bld1 <C source file name> [<library name>] [<object file directory>]
$!
$! If "library name" is specified, the result of the compile is put into
$! the specified library.  Extra /DEFINE variables can be defined by putting
$! them in a "cc_defines" variable.  Extra CC flags can be put in a "cc_flags"
$! variable.  CC command includes expansion of a "debug_flag" variable, which
$! you can set to "/DEBUG".
$!
$ objname = p3 + p1 + ".obj"
$ cname   = p1 + ".c"
$
$ if f$search(objname) .eqs. "" then goto l1
$ if f$cvtime(f$file_attributes(objname, "RDT")) .lts. f$cvtime(f$file_attributes(cname, "RDT")) then goto l1
$ write sys$output "  " + p1 + ": no compile needed"
$ exit
$ 
$ l1:
$ 
$ defines = "NCK, DEBUG, OLD_HOSTENT_STRUCT, INET"
$ if "''cc_defines'" .nes. "" then defines = defines + ", " + cc_defines
$
$ if "''opt_flag'" .eqs. "" then opt_flag = "/nooptimize"
$
$!
$! Assuming if UCX is installed then we want to compile using UCX include files.
$!
$ if f$search("sys$manager:ucx$startup.com") .eqs. "" then goto twg
$ define /user_mode sys         sys$library:
$ define /user_mode net         sys$library:
$ define /user_mode netinet     sys$library:
$ define /user_mode arpa        sys$library:
$ define /user_mode machine     sys$library:
$ includes = ""
$ goto comp
$
$ twg:
$ define /user_mode sys         twg$tcp:[netdist.include.sys],sys$library:
$ define /user_mode net         twg$tcp:[netdist.include.net]
$ define /user_mode netinet     twg$tcp:[netdist.include.netinet]
$ define /user_mode arpa        twg$tcp:[netdist.include.arpa]
$ define /user_mode machine     twg$tcp:[netdist.include.machine]
$ includes = ", twg$tcp:[netdist.include], exos$etc:"
$
$ comp:
$ write sys$output "  " + p1 + ": compiling"
$ cc 'cc_flags' 'debug_flag' 'opt_flag' /object='objname' /g_float /define=('defines') /include=([-.idl] 'includes') 'p1'
$
$ if p2 .eqs "" then exit
$
$ if f$search("''p2'.olb") .eqs. "" then lib /create 'p2'
$ lib 'p2' /replace 'objname'
