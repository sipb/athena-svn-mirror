@echo off
Rem
Rem The SmallEnv gorp is for those who have too small environment
Rem size which will cause the variables we set to be truncated
Rem but who cannot be bothered to read the Out Of Environment Space
Rem error messages spitted by the stock DOS shell
set XSRC=.
if not "%XSRC%" == "." goto SmallEnv
if "%1" == "" goto inplace
set XSRC=%1
if not "%XSRC%" == "%1" goto SmallEnv
:inplace
if exist .\kpathsea\INSTALL ren .\kpathsea\INSTALL INSTALL.txt
if exist .\web2c\INSTALL ren .\web2c\INSTALL INSTALL.txt
if exist .\dvipsk\INSTALL ren .\dvipsk\INSTALL INSTALL.txt
if exist .\dviljk\INSTALL ren .\dviljk\INSTALL INSTALL.txt
test -d %XSRC%
if not errorlevel 1 goto chkdir
echo %XSRC% is not a directory
goto end
:chkdir
test -f %XSRC%/configure
if not errorlevel 1 goto argsok
echo I cannot find the configure script in directory %XSRC%
goto end
:argsok
rem set SYSROOT=c:
set PATH_SEPARATOR=:
if not "%PATH_SEPARATOR%" == ":" goto SmallEnv
set PATH_EXPAND=y
if not "%PATH_EXPAND%" == "y" goto SmallEnv
if not "%HOSTNAME%" == "" goto hostdone
if "%windir%" == "" goto msdos
set OS=MS-Windows
if not "%OS%" == "MS-Windows" goto SmallEnv
goto haveos
:msdos
set OS=MS-DOS
if not "%OS%" == "MS-DOS" goto SmallEnv
:haveos
if not "%USERNAME%" == "" goto haveuname
if not "%USER%" == "" goto haveuser
echo No USERNAME and no USER found in the environment, using default values
set HOSTNAME=Unknown PC
if not "%HOSTNAME%" == "Unknown PC" goto SmallEnv
:haveuser
set HOSTNAME=%USER%'s PC
if not "%HOSTNAME%" == "%USER%'s PC" goto SmallEnv
goto userdone
:haveuname
set HOSTNAME=%USERNAME%'s PC
if not "%HOSTNAME%" == "%USERNAME%'s PC" goto SmallEnv
:userdone
set HOSTNAME=%HOSTNAME%, %OS%
:hostdone
set OS=
echo Updating configure scripts for DJGPP...
test -f %XSRC%/configure.orig
if not errorlevel 1 goto orig1
cp -p %XSRC%/configure configure.orig
goto patchmain
:orig1
if not exist configure.orig cp -p %XSRC%/configure.orig configure.orig
:patchmain
patch -o configure configure.orig %XSRC%/djgpp/cfgmain.pat
if errorlevel 1 goto PatchError
test -d %XSRC%/kpathsea
if errorlevel 1 goto DviLJk
if not exist kpathsea\nul mkdir kpathsea
if exist kpathsea\configure.orig goto orig2
test -f %XSRC%/kpathsea/configure.orig
if not errorlevel 1 cp -p %XSRC%/kpathsea/configure.orig kpathsea/configure.orig
if not exist kpathsea\configure.orig cp -p %XSRC%/kpathsea/configure kpathsea/configure.orig
:orig2
patch -o kpathsea/configure kpathsea/configure.orig %XSRC%/djgpp/cfgkpath.pat
if errorlevel 1 goto PatchError
:DviLJk
test -d %XSRC%/dviljk
if errorlevel 1 goto DviPSk
if not exist dviljk\nul mkdir dviljk
if exist dviljk\configure.orig goto orig3
test -f %XSRC%/dviljk/configure.orig
if not errorlevel 1 cp -p %XSRC%/dviljk/configure.orig dviljk/configure.orig
if not exist dviljk\configure.orig cp -p %XSRC%/dviljk/configure dviljk/configure.orig
:orig3
patch -o dviljk/configure dviljk/configure.orig %XSRC%/djgpp/cfgdvilj.pat
if errorlevel 1 goto PatchError
:DviPSk
test -d %XSRC%/dvipsk
if errorlevel 1 goto Web2C
if not exist dvipsk\nul mkdir dvipsk
if exist dvipsk\configure.orig goto orig4
test -f %XSRC%/dvipsk/configure.orig
if not errorlevel 1 cp -p %XSRC%/dvipsk/configure.orig dvipsk/configure.orig
if not exist dvipsk\configure.orig cp -p %XSRC%/dvipsk/configure dvipsk/configure.orig
:orig4
patch -o dvipsk/configure dvipsk/configure.orig %XSRC%/djgpp/cfgdvips.pat
if errorlevel 1 goto PatchError
:Web2C
test -d %XSRC%/web2c
if errorlevel 1 goto skipXDVI
if not exist web2c\nul mkdir web2c
if exist web2c\configure.orig goto orig5
test -f %XSRC%/web2c/configure.orig
if not errorlevel 1 cp -p %XSRC%/web2c/configure.orig web2c/configure.orig
if not exist web2c\configure.orig cp -p %XSRC%/web2c/configure web2c/configure.orig
:orig5
patch -o web2c/configure web2c/configure.orig %XSRC%/djgpp/cfgweb2c.pat
if errorlevel 1 goto PatchError
Rem
Rem XDvi is not supported on MS-DOS
GoTo skipXDVI
if not exist xdvik\nul mkdir xdvik
if not exist xdvik\configure.orig cp -p %XSRC%/xdvik/configure xdvik/configure.orig
patch -o xdvik/configure xdvik/configure.orig %XSRC%/djgpp/cfgxdvik.pat
if errorlevel 1 goto PatchError
:skipXDVI
set CONFIG_SHELL=bash.exe
rem set INSTALL=${DJDIR}/bin/ginstall -c
set YACC=bison -y
set LEX=flex
set RANLIB=ranlib
if not "%RANLIB%" == "ranlib" goto SmallEnv
Rem Use a response file to avoid exceeding the 126-character limit
echo --prefix='${DJDIR}' --datadir='${DJDIR}'/share --srcdir=%XSRC% > cfg.rf
echo --without-x --with-editor='emacs +%%d %%s' --with-epsfwin >> cfg.rf
echo Configuring...
sh ./configure i386-pc-msdos.djgppv2 @cfg.rf
echo Done.
goto CleanUp
:SmallEnv
echo Your environment size is too small.  Please enlarge it and run me again.
set HOSTNAME=
set OS=
:CleanUp
set XSRC=
set CONFIG_SHELL=
set INSTALL=
set YACC=
set LEX=
set RANLIB=
set HOSTNAME=
if exist cfg.rf del cfg.rf
goto end
:PatchError
echo Failed to patch one or more configure scripts.  Configure NOT done.
:end
