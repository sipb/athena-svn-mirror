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
::   Build the NCK libraries (MS/DOS)
::     Usage: build [-log {logfile}] [-codeview] [-max_debug]

set LOGFILE=con
set CODEVIEW=false
set MAX_DEBUG=false
set CDEBUGFLAG=
set LDEBUGFLAG=

:more_args
if $%1 == $  goto no_more_args

if not %1 == -log  goto not_log
  if not $%2 == $  goto logfile_ok
    echo Missing log file name
    goto usage
  :logfile_ok
  set LOGFILE=%2
  shift
  shift
  goto more_args
:not_log

if not %1 == -codeview  goto not_codeview
  set CODEVIEW=true
  shift
  goto more_args
:not_codeview

if not %1 == -max_debug  goto not_max_debug
  set MAX_DEBUG=true
  shift
  goto more_args
:not_max_debug

if $%1 == $  goto no_more_args
  echo Unknown option: %1
  goto usage

:no_more_args

if %CODEVIEW% == false goto notB_codeview
  if %MAX_DEBUG% == true   set CDEBUGFLAG=/Od /Zi /DMAX_DEBUG
  if %MAX_DEBUG% == false  set CDEBUGFLAG=/Od /Zi
  set LDEBUGFLAG=/CO
  goto nextA
:notB_codeview
  if %MAX_DEBUG% == true   set CDEBUGFLAG=/DMAX_DEBUG
:nextA

::===========================================================================
::  Begin build
if not %LOGFILE% == con  echo Building NCK
echo ********** Building NCK ********** >>%LOGFILE%
if not exist nck\*.*  mkdir nck
make "CDEBUGFLAG=%CDEBUGFLAG%" "LDEBUGFLAG=%LDEBUGFLAG%" nck.mak >>%LOGFILE%

if not %LOGFILE% == con  echo Building NCK/DDS
echo ********** Building NCK/DDS ********** >>%LOGFILE%
if not exist dds\*.*  mkdir dds
make "CDEBUGFLAG=%CDEBUGFLAG%" "LDEBUGFLAG=%LDEBUGFLAG%" nck_dds.mak >>%LOGFILE%
make NET=dds "CDEBUGFLAG=%CDEBUGFLAG%" "LDEBUGFLAG=%LDEBUGFLAG%" nck_utl.mak >>%LOGFILE%

if not %LOGFILE% == con  echo Building NCK/XLN
echo ********** Building NCK/XLN ********** >>%LOGFILE%
if not exist xln\*.*  mkdir xln
make "CDEBUGFLAG=%CDEBUGFLAG%" "LDEBUGFLAG=%LDEBUGFLAG%" nck_xln.mak >>%LOGFILE%
make NET=xln "CDEBUGFLAG=%CDEBUGFLAG%" "LDEBUGFLAG=%LDEBUGFLAG%" nck_utl.mak >>%LOGFILE%

set LOGFILE=
set CODEVIEW=
set MAX_DEBUG=
set CDEBUGFLAG=
set LDEBUGFLAG=
goto end

:usage
echo Usage: build [-log {logfile}] [-codeview] [-max_debug]

:end
