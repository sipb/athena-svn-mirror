echo off
:: ========================================================================== 
:: Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
:: Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
:: Copyright Laws Of The United States.
:: 
:: Apollo Computer Inc. reserves all rights, title and interest with respect
:: to copying, modification or the distribution of such software programs
:: and associated documentation, except those rights specifically granted
:: by Apollo in a Product Software Program License, Source Code License
:: or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
:: Apollo and Licensee.  Without such license agreements, such software
:: programs may not be used, copied, modified or distributed in source
:: or object code form.  Further, the copyright notice must appear on the
:: media, the supporting documentation and packaging as set forth in such
:: agreements.  Such License Agreements do not grant any rights to use
:: Apollo Computer's name or trademarks in advertising or publicity, with
:: respect to the distribution of the software programs without the specific
:: prior written permission of Apollo.  Trademark agreements may be obtained
:: in a separate Trademark License Agreement.
:: ========================================================================== 
::  
::  Make an NCK staging area (MS/DOS)
::

if not exist ..\stage\*.*           mkdir ..\stage
if not exist ..\stage\lib\*.*       mkdir ..\stage\lib
if not exist ..\stage\bin\*.*       mkdir ..\stage\bin
if not exist ..\stage\bin\dds\*.*   mkdir ..\stage\bin\dds
if not exist ..\stage\bin\xln\*.*   mkdir ..\stage\bin\xln
if not exist ..\stage\idl\*.*       mkdir ..\stage\idl
if not exist ..\stage\idl\c\*.*     mkdir ..\stage\idl\c
if not exist ..\stage\perf\*.*      mkdir ..\stage\perf
if not exist ..\stage\perf\dds\*.*  mkdir ..\stage\perf\dds
if not exist ..\stage\perf\xln\*.*  mkdir ..\stage\perf\xln
if not exist ..\stage\man\man8\*.*  mkdir ..\stage\man
if not exist ..\stage\man\man8\*.*  mkdir ..\stage\man\man8
if not exist ..\stage\man\cat8\*.*  mkdir ..\stage\man\cat8
if not exist ..\stage\dpci\dpciring\*.*  mkdir ..\stage\dpci
if not exist ..\stage\dpci\dpciring\*.*  mkdir ..\stage\dpci\dpciring
if not exist ..\stage\dpci\dpci503\*.*  mkdir ..\stage\dpci\dpci503

:: Create stage\tmp (no error message if it exists)
xcopy ..\cpyright stage\tmp\
del stage\tmp\cpyright

copy    ..\cpyright                 ..\stage
copy    ..\nck\install.bat          ..\stage    
copy    ..\nck\mk_flop.bat          ..\stage
copy    ..\nck\stcode.db            ..\stage
copy    ..\nck\uuidname.txt         ..\stage
copy    ..\nck\ddshosts.txt         ..\stage
copy    ..\man\doc\msdos.rd         ..\stage\readme.txt

copy    ..\nck\nck.lib              ..\stage\lib
copy    ..\nck\nck_dds.lib          ..\stage\lib
copy    ..\nck\nck_xln.lib          ..\stage\lib

exepack ..\nck\nck\stcode.exe       ..\stage\bin\stcode.exe

exepack ..\lb_test\dds\lb_test.exe  ..\stage\bin\dds\lb_test.exe
exepack ..\nck\dds\uuid_gen.exe     ..\stage\bin\dds\uuid_gen.exe
exepack ..\nck\dds\lb_admin.exe     ..\stage\bin\dds\lb_admin.exe
exepack ..\nck\dds\nrglbd.exe       ..\stage\bin\dds\nrglbd.exe

exepack ..\lb_test\xln\lb_test.exe  ..\stage\bin\xln\lb_test.exe
exepack ..\nck\xln\uuid_gen.exe     ..\stage\bin\xln\uuid_gen.exe
exepack ..\nck\xln\lb_admin.exe     ..\stage\bin\xln\lb_admin.exe
exepack ..\nck\xln\nrglbd.exe       ..\stage\bin\xln\nrglbd.exe

copy    ..\idl\*.idl                ..\stage\idl
copy    ..\idl\*.h                  ..\stage\idl\c

copy    ..\perf\run_c.bat           ..\stage\perf
exepack ..\perf\dds\client.exe      ..\stage\perf\dds\client.exe
exepack ..\perf\dds\server.exe      ..\stage\perf\dds\server.exe
exepack ..\perf\xln\client.exe      ..\stage\perf\xln\client.exe
exepack ..\perf\xln\server.exe      ..\stage\perf\xln\server.exe

copy    ..\man\man8\lb_admin.8      ..\stage\man\man8
copy    ..\man\man8\nrglbd.8        ..\stage\man\man8
copy    ..\man\man8\uuid_gen.8      ..\stage\man\man8
copy    ..\man\cat8\lb_admin.8      ..\stage\man\cat8
copy    ..\man\cat8\nrglbd.8        ..\stage\man\cat8
copy    ..\man\cat8\uuid_gen.8      ..\stage\man\cat8
copy    ..\lb_test\readme           ..\stage\man\lb_test.doc

if exist \dpci\dpciring\xport.exe  goto ring_ok
  echo Can't stage dpciring; not installed on this computer.
  goto end
:ring_ok
xcopy   \dpci\dpciring\xport.exe    ..\stage\dpci\dpciring\

if not exist \dpci\dpci503\xport.exe goto not_v5_dpci
  xcopy \dpci\dpci503\xport.exe     ..\stage\dpci\dpci503\
  goto enet_ok
:not_v5_dpci
if not exist \dpci\dpcienet\xport.exe goto not_v4_dpci
  xcopy \dpci\dpcienet\xport.exe     ..\stage\dpci\dpci503\
  goto enet_ok
:not_v4_dpci
echo Can't stage dpci503; not installed on this computer.
goto end
:enet_ok

:end
