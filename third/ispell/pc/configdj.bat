@echo off
echo Configuring Ispell for DJGPP...
update pc/local.djgpp local.h
if not exist Makefile.orig ren Makefile Makefile.orig
sed -f pc/cfgmain.sed Makefile.orig > Makefile
if "%TMPDIR%"=="" set TMPDIR=.
set PATH_SEPARATOR=:
set TEST_FINDS_EXE=y
rem if not exist %SYSROOT%\tmp\nul md %SYSROOT%\tmp
echo You are now ready to run Make
:End
