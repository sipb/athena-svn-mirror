echo off
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
::  Install the NIDL compiler (MS/DOS)
::    Usage: install [-stage] [-high_density] [-on <drive>:] [-no_idl]
::
::  - One high density floppy = three 360k floppies

:: Check if second pass running from hard disk
if $%1 == $-pass2  goto pass2

set PRODUCT=NIDL
set FROM=floppy
set HIGH_DENSITY=false
set FLOP_SIZE=360k
set DEST=C:\ncs
set INSTALL_IDL=true

:more_args
if $%1 == $  goto no_more_args

if not %1 == -stage goto not_stage
  set FROM=stage
  shift
  goto more_args
:not_stage

if not %1 == -high_density goto not_high_density
  set HIGH_DENSITY=true
  set FLOP_SIZE=1.2M
  shift
  goto more_args
:not_high_density

if not %1 == -on  goto not_on
  if not $%2 == $  goto target_ok
  echo Missing destination disk drive letter
  goto usage
  :target_ok
  set DEST=%2\ncs
  shift
  shift
  goto more_args
:not_on

if not %1 == -no_idl goto not_no_idl
  set INSTALL_IDL=false
  shift
  goto more_args
:not_no_idl

if not %1 == -test goto not_test
  set FROM=test
  shift
  goto more_args
:not_test

if $%1 == $  goto no_more_args
  echo Unknown option: %1
  goto usage

:no_more_args

::===========================================================================
:: Print out what's going to happen

if not %FROM% == floppy  goto notA_floppy
  echo Installing NIDL software in %DEST% from %FLOP_SIZE% floppy disks.
  echo - Your current directory should be located in the \ directory 
  echo   of the floppy drive to install from.
  goto nextA
:notA_floppy
if not %FROM% == stage  goto notA_stage
  echo Installing NIDL software in %DEST% from staging area.
  echo - Your current directory should be located in the staging
  echo   directory to install from.
:notA_stage
if not %FROM% == test  goto notA_test
  echo Installing NIDL software in %DEST% from test area.
  echo - Your current directory should be located in the test
  echo   directory to install from (with the .\disk* subdirectories).
:notA_test
:nextA

if %INSTALL_IDL% == false goto notA_idl
  echo - IDL interface and include files will be installed.
  goto nextB
:notA_idl
  echo - IDL interface and include files will **not** be installed.
:nextB

echo Type ^C to abort the installation, otherwise...
pause

::===========================================================================
:: Check source and dest

if not %FROM% == stage  goto notB_stage
  set SRC=.
  set HIGH_DENSITY=false
:notB_stage
if not %FROM% == floppy  goto notB_floppy
  if exist disk1.id  goto flop_ok
    echo Your current working directory must be on the \ directory of
    echo the MSDOS-%PRODUCT% (#1) %FLOP_SIZE% floppy disk.  Check working
    echo directory and floppy disk and try again.
    goto end
  :flop_ok
  set SRC=.
:notB_floppy
if not %FROM% == test  goto notB_test
  if exist disk1\disk1.id  goto testdir_ok
    echo Your current working directory must in a "test" distribution
    echo directory with .\disk1, .\disk2 ... subdirectories.  Try again.
    goto end
  :testdir_ok
  set SRC=disk1
:notB_test

type %SRC%\cpyright

:: Create ncs\tmp (no error message if it exists)
xcopy %SRC%\cpyright %DEST%\tmp\
del %DEST%\tmp\cpyright

::===========================================================================
:: Disk 1
if %HIGH_DENSITY% == true  goto do1_high
  set NEW_DISK=false
  set DISK_NUM=1
  goto no1_high
:do1_high
  set NEW_DISK=false
  set DISK_NUM=1
:no1_high

if %NEW_DISK% == false  goto not1_new
  if not %FROM% == floppy  goto not1_floppy
    :try1_again
    echo Insert MSDOS-%PRODUCT% (#%DISK_NUM%) %FLOP_SIZE% floppy in drive.
    pause
    if exist disk%DISK_NUM%.id  goto disk1_ok
      echo Incorrect floppy inserted, try again.
      goto try1_again
    :disk1_ok
  :not1_floppy
  if not %FROM% == test  goto not1_test
    set SRC=disk%DISK_NUM%
  :not1_test
:not1_new

::===========================================================================
:: Copy this batch file to hard disk and invoke it.  (Even for cases when
:: already running from hard disk)

copy  %SRC%\install.bat              %DEST%\tmp

if exist %DEST%\tmp\install.bat  goto pass2_ok
  echo Error: couldn't copy install.bat to hard disk.
  goto end
:pass2_ok
%DEST%\tmp\install -pass2

:: *** Never get here ***

:pass2

::===========================================================================
:: The second pass will begin here, executing from a copy of the script on
:: the hard disk (since we want to remove the copy on floppy #1).  Note all
:: the environment variables are inherited.

copy  %SRC%\cpyright                 %DEST%
copy  %SRC%\readme.txt               %DEST%
xcopy %SRC%\bin\nidl.exe             %DEST%\bin\
xcopy %SRC%\examples\bank            %DEST%\examples\bank\
xcopy %SRC%\examples\binop           %DEST%\examples\binop\ /s
xcopy %SRC%\man                      %DEST%\man\ /s

::===========================================================================
:: Disk 2
if %HIGH_DENSITY% == true  goto do2_high
  set NEW_DISK=true
  set DISK_NUM=2
  goto no2_high
:do2_high
  set NEW_DISK=false
  set DISK_NUM=1
:no2_high

if %NEW_DISK% == false  goto not2_new
  if not %FROM% == floppy  goto not2_floppy
    :try2_again
    echo Insert MSDOS-%PRODUCT% (#%DISK_NUM%) %FLOP_SIZE% floppy in drive.
    pause
    if exist disk%DISK_NUM%.id  goto disk2_ok
      echo Incorrect floppy inserted, try again.
      goto try2_again
    :disk2_ok
  :not2_floppy
  if not %FROM% == test  goto not2_test
    set SRC=disk%DISK_NUM%
  :not2_test
:not2_new

xcopy %SRC%\examples\perf            %DEST%\examples\perf\
if %INSTALL_IDL% == false   goto no2_idl
  xcopy %SRC%\idl\c\*.*   %DEST%\include\idl\c\
  del %DEST%\include\idl\c\pfm.h
  del %DEST%\include\idl\c\ppfm.h
  copy %SRC%\idl\c\pfm.h  %DEST%\include 
  copy %SRC%\idl\c\ppfm.h %DEST%\include
:no2_idl

::===========================================================================
:: Disk 3
if %HIGH_DENSITY% == true  goto do3_high
  set NEW_DISK=true
  set DISK_NUM=3
  goto no3_high
:do3_high
  set NEW_DISK=false
  set DISK_NUM=1
:no3_high

if %NEW_DISK% == false  goto not3_new
  if not %FROM% == floppy  goto not3_floppy
    :try3_again
    echo Insert MSDOS-%PRODUCT% (#%DISK_NUM%) %FLOP_SIZE% floppy in drive.
    pause
    if exist disk%DISK_NUM%.id  goto disk3_ok
      echo Incorrect floppy inserted, try again.
      goto try3_again
    :disk3_ok
  :not3_floppy
  if not %FROM% == test  goto not3_test
    set SRC=disk%DISK_NUM%
  :not3_test
:not3_new

if %INSTALL_IDL% == false   goto no3_idl
  xcopy %SRC%\idl\*.*     %DEST%\include\idl\
:no3_idl
xcopy %SRC%\examples\nidltest        %DEST%\examples\nidltest\

::===========================================================================
:: Disk 4
if %HIGH_DENSITY% == true  goto do4_high
  set NEW_DISK=true
  set DISK_NUM=4
  goto no4_high
:do4_high
  set NEW_DISK=true
  set DISK_NUM=2
:no4_high

if %NEW_DISK% == false  goto not4_new
  if not %FROM% == floppy  goto not4_floppy
    :try4_again
    echo Insert MSDOS-%PRODUCT% (#%DISK_NUM%) %FLOP_SIZE% floppy in drive.
    pause
    if exist disk%DISK_NUM%.id  goto disk4_ok
      echo Incorrect floppy inserted, try again.
      goto try4_again
    :disk4_ok
  :not4_floppy
  if not %FROM% == test  goto not4_test
    set SRC=disk%DISK_NUM%
  :not4_test
:not4_new

xcopy %SRC%\examples\lb_test         %DEST%\examples\lb_test\
xcopy %SRC%\examples\mandel          %DEST%\examples\mandel\
xcopy %SRC%\examples\rrpc            %DEST%\examples\rrpc\
xcopy %SRC%\examples\splash          %DEST%\examples\splash\

::===========================================================================
echo %PRODUCT% software installation completed.
set PRODUCT=
set FROM=
set HIGH_DENSITY=
set FLOP_SIZE=
set DEST=
set INSTALL_IDL=
set INSTALL_DDS=
set INSTALL_XLN=
set SRC=
set NEW_DISK=
set DISK_NUM=
goto end

:usage
echo Usage: install [-stage] [-high_density] [-on {drive}:] [-no_idl]
echo                [-dds_only] [-xln_only]

:end
