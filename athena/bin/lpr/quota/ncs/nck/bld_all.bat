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
::   Build the NCK subsystem (MS/DOS)
::     Usage: bld_all [-log {logfile}] [-codeview] [-max_debug]

:: If directory c:\ncs\tmp doesn't exist, create it (quietly)
if exist c:\ncs\tmp\*.*  goto tmp_ok
  xcopy bld_all.bat c:\ncs\tmp\
  del c:\ncs\tmp\bld_all.bat
:tmp_ok

%COMSPEC% /c build %1 %2 %3 %4 %5 %6

cd ..\perf
%COMSPEC% /c build %1 %2 %3 %4 %5 %6 ..\idl ..\nck

cd ..\lb_test
%COMSPEC% /c build %1 %2 %3 %4 %5 %6 ..\idl ..\nck

cd ..\nck
echo Build completed.
