echo off
:: ========================================================================== 
:: Copyright  1987 by Apollo Computer Inc., Chelmsford, Massachusetts
:: 
:: All Rights Reserved
:: 
:: All Apollo source code software programs, object code software programs,
:: documentation and copies thereof shall contain the copyright notice above
:: and this permission notice.  Apollo Computer Inc. reserves all rights,
:: title and interest with respect to copying, modification or the
:: distribution of such software programs and associated documentation,
:: except those rights specifically granted by Apollo in a Product Software
:: Program License or Source Code License between Apollo and Licensee.
:: Without this License, such software programs may not be used, copied,
:: modified or distributed in source or object code form.  Further, the
:: copyright notice must appear on the media, the supporting documentation
:: and packaging.  A Source Code License does not grant any rights to use
:: Apollo Computer's name or trademarks in advertising or publicity, with
:: respect to the distribution of the software programs without the specific
:: prior written permission of Apollo.  Trademark agreements may be obtained
:: in a separate Trademark License Agreement.
:: 
:: Apollo disclaims all warranties, express or implied, with respect to
:: the Software Programs including the implied warranties of merchantability
:: and fitness, for a particular purpose.  In no event shall Apollo be liable
:: for any special, indirect or consequential damages or any damages
:: whatsoever resulting from loss of use, data or profits whether in an
:: action of contract or tort, arising out of or in connection with the
:: use or performance of such software programs.
:: ========================================================================== 
::
::   Make floppies for NCK from staging area (MS/DOS)
::     Usage: mk_flop [-high_density] [-test target_dir] [-no_verify]

set PRODUCT=NCK
set HIGH_DENSITY=false
set FLOP_SIZE=360k
set TEST=false
set VERIFY=/v

:more_args
if $%1 == $  goto no_more_args

if not %1 == -high_density  goto not_high_density
  set HIGH_DENSITY=true
  set FLOP_SIZE=1.2M
  shift
  goto more_args
:not_high_density

if not %1 == -test  goto not_test
  set TEST=true
  if not $%2 == $  goto target_ok
    echo Missing target_dir
    goto usage
  :target_ok
  set TARGET_DIR=%2
  if not exist %TARGET_DIR%\disk1\*.*  mkdir %TARGET_DIR%
  shift
  shift
  goto more_args
:not_test

if not %1 == -no_verify  goto not_no_verify
  set VERIFY=
  shift
  goto more_args
:not_no_verify

if $%1 == $  goto no_more_args
  echo Unknown option: %1
  goto usage

:no_more_args

set DEST=a:

::===========================================================================
:: Disk 1
if %HIGH_DENSITY% == true  goto do1_hi
  set NEW_DISK=true
  set DISK_NUM=1
  goto no1_hi
:do1_hi
  set NEW_DISK=true
  set DISK_NUM=1
:no1_hi

if %NEW_DISK% == false  goto no1_new
  if %TEST% == true  goto do1_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no1_test
  :do1_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no1_test
:no1_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
copy  readme.txt              %DEST% %VERIFY%
copy  install.bat             %DEST% %VERIFY%
xcopy lib                     %DEST%\lib\ /s %VERIFY%

::===========================================================================
:: Disk 2
if %HIGH_DENSITY% == true  goto do2_hi
  set NEW_DISK=true
  set DISK_NUM=2
  goto no2_hi
:do2_hi
  set NEW_DISK=false
  set DISK_NUM=1
:no2_hi

if %NEW_DISK% == false  goto no2_new
  if %TEST% == true  goto do2_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no2_test
  :do2_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no2_test
:no2_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
xcopy idl                     %DEST%\idl\ /s %VERIFY%
xcopy man                     %DEST%\man\ /s %VERIFY%

::===========================================================================
:: Disk 3
if %HIGH_DENSITY% == true  goto do3_hi
  set NEW_DISK=true
  set DISK_NUM=3
  goto no3_hi
:do3_hi
  set NEW_DISK=false
  set DISK_NUM=1
:no3_hi

if %NEW_DISK% == false  goto no3_new
  if %TEST% == true  goto do3_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no3_test
  :do3_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no3_test
:no3_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
copy  uuidname.txt            %DEST% %VERIFY%
xcopy stcode.db               %DEST% %VERIFY%
xcopy bin\stcode.exe          %DEST%\bin\ %VERIFY%

::===========================================================================
:: Disk 4
if %HIGH_DENSITY% == true  goto do4_hi
  set NEW_DISK=true
  set DISK_NUM=4
  goto no4_hi
:do4_hi
  set NEW_DISK=true
  set DISK_NUM=2
:no4_hi

if %NEW_DISK% == false  goto no4_new
  if %TEST% == true  goto do4_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no4_test
  :do4_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no4_test
:no4_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
xcopy perf\run_c.bat          %DEST%\perf\ %VERIFY%
xcopy perf\dds                %DEST%\perf\dds\ %VERIFY%

::===========================================================================
:: Disk 5
if %HIGH_DENSITY% == true  goto do5_hi
  set NEW_DISK=true
  set DISK_NUM=5
  goto no5_hi
:do5_hi
  set NEW_DISK=false
  set DISK_NUM=2
:no5_hi

if %NEW_DISK% == false  goto no5_new
  if %TEST% == true  goto do5_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no5_test
  :do5_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no5_test
:no5_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
xcopy bin\dds\lb_admin.exe    %DEST%\bin\dds\ %VERIFY%
xcopy bin\dds\lb_test.exe     %DEST%\bin\dds\ %VERIFY%

::===========================================================================
:: Disk 6
if %HIGH_DENSITY% == true  goto do6_hi
  set NEW_DISK=true
  set DISK_NUM=6
  goto no6_hi
:do6_hi
  set NEW_DISK=false
  set DISK_NUM=2
:no6_hi

if %NEW_DISK% == false  goto no6_new
  if %TEST% == true  goto do6_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no6_test
  :do6_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no6_test
:no6_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
xcopy ddshosts.txt            %DEST% %VERIFY%
xcopy bin\dds\nrglbd.exe      %DEST%\bin\dds\ %VERIFY%
xcopy bin\dds\uuid_gen.exe    %DEST%\bin\dds\ %VERIFY%
xcopy dpci\dpciring           %DEST%\dpci\dpciring\ %VERIFY%
xcopy dpci\dpci503            %DEST%\dpci\dpci503\ %VERIFY%

::===========================================================================
:: Disk 7
if %HIGH_DENSITY% == true  goto do7_hi
  set NEW_DISK=true
  set DISK_NUM=7
  goto no7_hi
:do7_hi
  set NEW_DISK=true
  set DISK_NUM=3
:no7_hi

if %NEW_DISK% == false  goto no7_new
  if %TEST% == true  goto do7_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no7_test
  :do7_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no7_test
:no7_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
xcopy perf\run_c.bat          %DEST%\perf\ %VERIFY%
xcopy perf\xln                %DEST%\perf\xln\ %VERIFY%

::===========================================================================
:: Disk 8
if %HIGH_DENSITY% == true  goto do8_hi
  set NEW_DISK=true
  set DISK_NUM=8
  goto no8_hi
:do8_hi
  set NEW_DISK=false
  set DISK_NUM=3
:no8_hi

if %NEW_DISK% == false  goto no8_new
  if %TEST% == true  goto do8_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no8_test
  :do8_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no8_test
:no8_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
xcopy bin\xln\lb_admin.exe    %DEST%\bin\xln\ %VERIFY%
xcopy bin\xln\lb_test.exe     %DEST%\bin\xln\ %VERIFY%

::===========================================================================
:: Disk 9
if %HIGH_DENSITY% == true  goto do9_hi
  set NEW_DISK=true
  set DISK_NUM=9
  goto no9_hi
:do9_hi
  set NEW_DISK=false
  set DISK_NUM=3
:no9_hi

if %NEW_DISK% == false  goto no9_new
  if %TEST% == true  goto do9_test
    echo Insert formatted %FLOP_SIZE% floppy in drive A: for MSDOS-%PRODUCT% (#%DISK_NUM%) 
    pause
    goto no9_test
  :do9_test
    set DEST=%TARGET_DIR%\disk%DISK_NUM%
    if not exist %DEST%\*.*  mkdir %DEST%
  :no9_test
:no9_new

type  nul                     >%DEST%\disk%DISK_NUM%.id
copy  cpyright                %DEST% %VERIFY%
xcopy bin\xln\nrglbd.exe      %DEST%\bin\xln\ %VERIFY%
xcopy bin\xln\uuid_gen.exe    %DEST%\bin\xln\ %VERIFY%

::===========================================================================
set PRODUCT=
set HIGH_DENSITY=
set FLOP_SIZE=
set TEST=
set VERIFY=
set DEST=
set TARGET_DIR=
set NEW_DISK=
goto end

:usage
echo Usage: mk_flop [-high_density] [-test target_dir] [-no_verify]

:end
