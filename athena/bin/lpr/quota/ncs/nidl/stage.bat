echo off
:: 
::  ========================================================================== 
::  Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
::  Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
::  Copyright Laws Of The United States.
:: 
::  Apollo Computer Inc. reserves all rights, title and interest with respect 
::  to copying, modification or the distribution of such software programs and
::  associated documentation, except those rights specifically granted by Apollo
::  in a Product Software Program License, Source Code License or Commercial
::  License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
::  Licensee.  Without such license agreements, such software programs may not
::  be used, copied, modified or distributed in source or object code form.
::  Further, the copyright notice must appear on the media, the supporting
::  documentation and packaging as set forth in such agreements.  Such License
::  Agreements do not grant any rights to use Apollo Computer's name or trademarks
::  in advertising or publicity, with respect to the distribution of the software
::  programs without the specific prior written permission of Apollo.  Trademark 
::  agreements may be obtained in a separate Trademark License Agreement.
::  ========================================================================== 
:: 

:: Run this script from the NIDL directory after building NIDL from the
:: the source.  It creates ..\stage, the "staging area" from which you
:: can install NIDL and its associated files.

if not exist ..\stage\*.*           mkdir ..\stage
if not exist ..\stage\bin\*.*       mkdir ..\stage\bin
if not exist ..\stage\idl\*.*       mkdir ..\stage\idl
if not exist ..\stage\idl\c\*.*     mkdir ..\stage\idl\c
if not exist ..\stage\man\man1\*.*  mkdir ..\stage\man
if not exist ..\stage\man\man1\*.*  mkdir ..\stage\man\man1
if not exist ..\stage\man\cat1\*.*  mkdir ..\stage\man\cat1
if not exist ..\stage\examples\*.*  mkdir ..\stage\examples

copy    ..\cpyright          ..\stage
copy    install.bat          ..\stage
copy    cut.bat              ..\stage
exepack nidl.exe             ..\stage\bin\nidl.exe

copy    ..\idl\*.idl         ..\stage\idl
copy    ..\idl\*.h           ..\stage\idl\c

copy    ..\man\man1\*.1      ..\stage\man\man1
copy    ..\man\cat1\*.1      ..\stage\man\cat1
copy    ..\man\doc\msdos.rd  ..\stage\readme.txt

xcopy   ..\examples          ..\stage\examples /S /E

:: Excise VMS build scripts because .com files upset MSDOS
del ..\stage\examples\bank\*.com
del ..\stage\examples\binop\binop_fw\*.com
del ..\stage\examples\binop\binop_lu\*.com
del ..\stage\examples\binop\binop_wk\*.com
del ..\stage\examples\lb_test\*.com
del ..\stage\examples\mandel\*.com
del ..\stage\examples\nidltest\*.com
del ..\stage\examples\perf\*.com
