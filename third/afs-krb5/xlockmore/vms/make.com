$ save_verify='f$verify(0)
$! set ver 
$!
$!
$!      Compile and link xlock
$!
$! USAGE:
$! @make [link debug clobber clean]
$!       link : linking only
$!       debug : compile with degugger switch
$!       clean : clean all except executable
$!       clobber : clean all
$!
$! If you have
$!              XPM library
$!              XVMSUTILS library (VMS6.2 or lower)
$!              Mesa GL library
$! insert the correct directory instead of X11 or GL:
$ xvmsf="X11:XVMSUTILS.OLB"
$ xpmf="X11:LIBXPM.OLB"
$ glf="GL:LIBMESAGL.OLB"
$!
$! Assumes you have the readable bitmaps.  Easily corrected if you have
$! permissions.  Many systems are distributed with a nulls at the end of
$! these include files which causes shapes.c not to compile.
$ cvb=0
$! Assumes corrupt VMS bitmaps.  
$! cvb=1
$!
$! Hackers locks.  Experimental!
$ hack=0
$! hack=1
$!
$! Already assumes DEC C on Alpha.
$! Assume VAX C on VAX.
$ decc=0
$! Assume DEC C on VAX.
$! decc=1
$!
$! if vrt<>0 the use of the root window is enabled
$ vrt=1
$! vrt=0
$!
$!
$! NOTHING SHOULD BE MODIFIED BELOW
$!
$ if p1 .eqs. "CLEAN" then goto Clean
$ if p1 .eqs. "CLOBBER" then goto Clobber
$!
$ xpm=f$search("''xpmf'") .nes. ""
$ gl=f$search("''glf'") .nes. ""
$ axp=f$getsyi("HW_MODEL") .ge. 1024
$ sys_ver=f$edit(f$getsyi("version"),"compress")
$ if f$extract(0,1,sys_ver) .nes. "V"
$ then
$   type sys$input
You appear to be running a Field Test version of VMS. This script will 
assume that the operating system version is at least V7.0. 
$!
$   sys_ver="V7.0"
$ endif
$ sys_maj=0+f$extract(1,1,sys_ver)
$ if sys_maj .lt. 7 then xvms=f$search("''xvmsf'") .nes. ""
$!
$!
$! Create .opt file
$ close/nolog optf
$ open/write optf xlock.opt
$!
$ defs=="VMS"
$ if xpm then defs=="''defs',HAS_XPM"
$ if gl then defs=="''defs',HAS_GL"
$ if axp then defs=="''defs',VMS_PLAY"
$ if sys_maj .lt. 7
$ then
$   if xvms then defs=="''defs',XVMSUTILS"
$ endif
$ if cvb then defs=="''defs',CORRUPT_VMS_BITMAPS"
$ if vrt then defs=="''defs',USE_VROOT"
$ if hack then defs=="''defs',USE_HACKERS"
$! If your system does not have bitmaps, you should
$! decw$bitmaps:==[-.bitmaps]
$!
$!
$! Establish the Compiling Environment
$!
$!
$! Set compiler command
$! Put in /include=[] for local include file like a pwd.h ...
$!   not normally required.
$ if axp
$ then
$   cc=="cc/standar=vaxc/define=(''defs')"
$ else
$   if decc
$   then
$     cc=="cc/decc/standard=vaxc/define=(''defs')"
$   else ! VAX C
$     cc=="cc/define=(''defs')"
$   endif
$ endif
$ if p1 .eqs. "DEBUG" .or. p2 .eqs. "DEBUG" .or. p3 .eqs. "DEBUG"
$ then
$   if axp
$   then
$     cc=="cc/deb/noopt/standar=vaxc/define=(''defs')/list"
$   else
$     if decc
$     then
$       cc=="cc/deb/noopt/decc/standar=vaxc/define=(''defs')/list"
$     else ! VAX C
$       cc=="cc/deb/noopt/define=(''defs')/list"
$     endif
$   endif
$   link=="link/deb"
$ endif
$!
$ if p1 .nes. "LINK"
$ then
$   if axp .or. .not. decc
$   then
$     define/nolog sys sys$library
$   endif
$!
$   write sys$output "Copying Include Files"
$   call make flag.h     "copy [.flags]flag-vms.h flag.h"      [.flags]flag-vms.h
$!   call make eyes.xbm  "copy [.bitmaps]m-x11.xbm eyes.xbm"   [.bitmaps]m-x11.xbm
$   call make eyes.xbm   "copy [.bitmaps]m-grelb.xbm eyes.xbm" [.bitmaps]m-grelb.xbm
$   call make image.xbm  "copy [.bitmaps]m-x11.xbm image.xbm"  [.bitmaps]m-x11.xbm
$   call make life.xbm   "copy [.bitmaps]s-grelb.xbm life.xbm" [.bitmaps]s-grelb.xbm
$!   call make life.xbm  "copy [.bitmaps]s-x11.xbm life.xbm"   [.bitmaps]s-x11.xbm
$   call make life1d.xbm "copy [.bitmaps]t-x11.xbm life1d.xbm" [.bitmaps]t-x11.xbm
$   call make maze.xbm   "copy [.bitmaps]l-x11.xbm maze.xbm"   [.bitmaps]l-x11.xbm
$   call make puzzle.xbm "copy [.bitmaps]l-x11.xbm puzzle.xbm" [.bitmaps]l-x11.xbm
$   if hack
$   then
$!     call make ghost.xbm  "copy [.bitmaps]m-x11.xbm ghost.xbm"   [.bitmaps]m-x11.xbm
$     call make ghost.xbm   "copy [.bitmaps]m-ghost.xbm ghost.xbm" [.bitmaps]m-ghost.xbm
$   endif
$   if xpm
$   then
$     call make image.xpm  "copy [.pixmaps]m-x11.xpm image.xpm"  [.pixmaps]m-x11.xpm
$     call make puzzle.xpm "copy [.pixmaps]m-x11.xpm puzzle.xpm" [.pixmaps]m-x11.xpm
$   endif
$   if hack
$   then
$     call make ball.c    "copy [.hackers]ball.c ball.c"       [.hackers]ball.c
$     call make cartoon.c "copy [.hackers]cartoon.c cartoon.c" [.hackers]cartoon.c
$     call make flamen.c  "copy [.hackers]flamen.c flamen.c"   [.hackers]flamen.c
$     call make huskers.c "copy [.hackers]huskers.c huskers.c" [.hackers]huskers.c
$     call make julia.c   "copy [.hackers]julia.c julia.c"     [.hackers]julia.c
$     call make pacman.c  "copy [.hackers]pacman.c pacman.c"   [.hackers]pacman.c
$     call make polygon.c "copy [.hackers]polygon.c polygon.c" [.hackers]polygon.c
$     call make roll.c    "copy [.hackers]roll.c roll.c"       [.hackers]roll.c
$     call make turtle.c  "copy [.hackers]turtle.c turtle.c"   [.hackers]turtle.c
$   endif
$!
$   write sys$output "Compiling XLock"
$   call make xlock.obj     "cc xlock.c"     xlock.c xlock.h mode.h vroot.h
$   call make passwd.obj    "cc passwd.c"    passwd.c xlock.h
$   call make resource.obj  "cc resource.c"  resource.c xlock.h mode.h
$   call make utils.obj     "cc utils.c"     utils.c xlock.h
$   call make logout.obj    "cc logout.c"    logout.c xlock.h
$   call make mode.obj      "cc mode.c"      mode.c xlock.h mode.h
$   call make ras.obj       "cc ras.c"       ras.c xlock.h
$   call make xbm.obj       "cc xbm.c"       xbm.c xlock.h
$   call make color.obj     "cc color.c"     color.c xlock.h
$   call make ant.obj       "cc ant.c"       ant.c xlock.h mode.h
$   call make bat.obj       "cc bat.c"       bat.c xlock.h mode.h
$   call make blot.obj      "cc blot.c"      blot.c xlock.h mode.h
$   call make bouboule.obj  "cc bouboule.c"  bouboule.c xlock.h mode.h
$   call make bounce.obj    "cc bounce.c"    bounce.c xlock.h mode.h
$   call make braid.obj     "cc braid.c"     braid.c xlock.h mode.h
$   call make bug.obj       "cc bug.c"       bug.c xlock.h mode.h
$   call make clock.obj     "cc clock.c"     clock.c xlock.h mode.h
$   call make daisy.obj     "cc daisy.c"     daisy.c xlock.h mode.h
$   call make dclock.obj    "cc dclock.c"    dclock.c xlock.h mode.h
$   call make demon.obj     "cc demon.c"     demon.c xlock.h mode.h
$   call make eyes.obj      "cc eyes.c"      eyes.c xlock.h mode.h
$   call make flag.obj      "cc flag.c"      flag.c xlock.h mode.h
$   call make flame.obj     "cc flame.c"     flame.c xlock.h mode.h
$   call make forest.obj    "cc forest.c"    forest.c xlock.h mode.h
$   call make galaxy.obj    "cc galaxy.c"    galaxy.c xlock.h mode.h
$   call make gear.obj      "cc gear.c"      gear.c xlock.h mode.h
$   call make geometry.obj  "cc geometry.c"  geometry.c xlock.h mode.h
$   call make grav.obj      "cc grav.c"      grav.c xlock.h mode.h
$   call make helix.obj     "cc helix.c"     helix.c xlock.h mode.h
$   call make hop.obj       "cc hop.c"       hop.c xlock.h mode.h
$   call make hyper.obj     "cc hyper.c"     hyper.c xlock.h mode.h
$   call make image.obj     "cc image.c"     image.c xlock.h mode.h ras.h
$   call make kaleid.obj    "cc kaleid.c"    kaleid.c xlock.h mode.h
$   call make laser.obj     "cc laser.c"     laser.c xlock.h mode.h
$   call make life.obj      "cc life.c"      life.c xlock.h mode.h
$   call make life1d.obj    "cc life1d.c"    life1d.c xlock.h mode.h
$   call make life3d.obj    "cc life3d.c"    life3d.c xlock.h mode.h
$   call make lightning.obj "cc lightning.c" lightning.c xlock.h mode.h
$   call make lissie.obj    "cc lissie.c"    lissie.c xlock.h mode.h
$   call make marquee.obj   "cc marquee.c"   marquee.c xlock.h mode.h
$   call make maze.obj      "cc maze.c"      maze.c xlock.h mode.h
$   call make mountain.obj  "cc mountain.c"  mountain.c xlock.h mode.h
$   call make nose.obj      "cc nose.c"      nose.c xlock.h mode.h
$   call make qix.obj       "cc qix.c"       qix.c xlock.h mode.h
$   call make penrose.obj   "cc penrose.c"   penrose.c xlock.h mode.h
$   call make petal.obj     "cc petal.c"     petal.c xlock.h mode.h
$   call make puzzle.obj    "cc puzzle.c"    puzzle.c xlock.h mode.h ras.h
$   call make pyro.obj      "cc pyro.c"      pyro.c xlock.h mode.h
$   call make rotor.obj     "cc rotor.c"     rotor.c xlock.h mode.h
$   call make shape.obj     "cc shape.c"     shape.c xlock.h mode.h
$   call make slip.obj      "cc slip.c"      slip.c xlock.h mode.h
$   call make sphere.obj    "cc sphere.c"    sphere.c xlock.h mode.h
$   call make spiral.obj    "cc spiral.c"    spiral.c xlock.h mode.h
$   call make spline.obj    "cc spline.c"    spline.c xlock.h mode.h
$   call make star.obj      "cc star.c"      star.c xlock.h mode.h
$   call make swarm.obj     "cc swarm.c"     swarm.c xlock.h mode.h
$   call make swirl.obj     "cc swirl.c"     swirl.c xlock.h mode.h
$   call make tri.obj       "cc tri.c"       tri.c xlock.h mode.h
$   call make triangle.obj  "cc triangle.c"  triangle.c xlock.h mode.h
$   call make wator.obj     "cc wator.c"     wator.c xlock.h mode.h
$   call make wire.obj      "cc wire.c"      wire.c xlock.h mode.h
$   call make world.obj     "cc world.c"     world.c xlock.h mode.h
$   call make worm.obj      "cc worm.c"      worm.c xlock.h mode.h
$   call make blank.obj     "cc blank.c"     blank.c xlock.h mode.h
$   call make bomb.obj      "cc bomb.c"      bomb.c xlock.h mode.h
$   call make random.obj    "cc random.c"    random.c xlock.h mode.h
$! AMD and VMS_PLAY for SOUND uncomment VMS_PLAY in xlock.h
$   if axp
$   then
$     call make amd.obj       "cc amd.c"       amd.c amd.h
$     call make vms_play.obj  "cc vms_play.c"  vms_play.c amd.h
$   endif
$   if hack
$   then
$     write sys$output "Compiling XLock Hackers modes, Caution: Experimental!"
$     call make ball.obj      "cc ball.c"      ball.c xlock.h mode.h
$     call make cartoon.obj   "cc cartoon.c"   cartoon.c xlock.h mode.h
$     call make flamen.obj    "cc flamen.c"    flamen.c xlock.h mode.h
$     call make huskers.obj   "cc huskers.c"   huskers.c xlock.h mode.h
$     call make julia.obj     "cc julia.c"     julia.c xlock.h mode.h
$     call make pacman.obj    "cc pacman.c"    pacman.c xlock.h mode.h
$     call make polygon.obj   "cc polygon.c"   polygon.c xlock.h mode.h
$     call make roll.obj      "cc roll.c"      roll.c xlock.h mode.h
$     call make turtle.obj    "cc turtle.c"    turtle.c xlock.h mode.h
$   endif

$ endif
$!
$! Get libraries
$ if xpm then write optf "''xpmf'/lib"
$ if gl then write optf "''glf'/lib"
$ if sys_maj .lt. 7
$ then
$   if xvms then write optf "''xvmsf'/lib"
$ endif
$! if .not. axp then write optf "sys$library:vaxcrtl/lib"
$ write optf "sys$library:vaxcrtl/lib"
$ if axp then write optf "sys$library:ucx$ipc_shr/share"
$ if axp then write optf "sys$share:decw$xextlibshr/share"
$ if axp then write optf "sys$share:decw$xtlibshrr5/share"
$ if .not. axp then write optf "sys$library:ucx$ipc/lib"
$ write optf "sys$share:decw$dxmlibshr/share"
$ write optf "sys$share:decw$xlibshr/share"
$ close optf
$!
$! LINK
$ write sys$output "Linking XLock"
$ link/map xlock/opt
$!
$! Create .opt file
$ open/write optf xmlock.opt
$ call make xmlock "copy [.hackers]xmlock.c xmlock.c" [.hackers]xmlock.c
$ write sys$output "Compiling XmLock, Caution: Experimental!"
$ call make xmlock.obj "cc xmlock.c" xmlock.c
$! Get libraries
$! if .not. axp then write optf "sys$library:vaxcrtl/lib"
$ write optf "sys$library:vaxcrtl/lib"
$ if axp then write optf "sys$library:ucx$ipc_shr/share"
$ if axp then write optf "sys$share:decw$xextlibshr/share"
$ if axp then write optf "sys$share:decw$xtlibshrr5/share"
$ if .not. axp then write optf "sys$library:ucx$ipc/lib"
$! write optf "sys$share:decw$dxmlibshr/share"
$ write optf "sys$share:decw$xmlibshr12/share"
$ write optf "sys$share:decw$xlibshr/share"
$ close optf
$!
$! LINK
$ write sys$output "Linking XmLock, Caution: Experimental!"
$ link/map xmlock/opt
$!
$ set noverify
$ exit
$!
$Clobber:      ! Delete executables, Purge directory and clean up object files
$!                and listings
$ delete/noconfirm/log xlock.exe;*
$ delete/noconfirm/log xmlock.exe;*
$ delete/noconfirm/log ball.c;*
$ delete/noconfirm/log cartoon.c;*
$ delete/noconfirm/log flamen.c;*
$ delete/noconfirm/log huskers.c;*
$ delete/noconfirm/log julia.c;*
$ delete/noconfirm/log pacman.c;*
$ delete/noconfirm/log polygon.c;*
$ delete/noconfirm/log turtle.c;*
$ delete/noconfirm/log xmlock.c;*
$!
$Clean:        ! Purge directory, clean up object files and listings
$ close/nolog optf
$ purge
$ delete/noconfirm/log *.lis;*
$ delete/noconfirm/log *.obj;*
$ delete/noconfirm/log *.opt;*
$ delete/noconfirm/log *.map;*
$ delete/noconfirm/log flag.h;*
$ delete/noconfirm/log eyes.xbm;*
$ delete/noconfirm/log image.xbm;*
$ delete/noconfirm/log ghost.xbm;*
$ delete/noconfirm/log life.xbm;*
$ delete/noconfirm/log life1d.xbm;*
$ delete/noconfirm/log maze.xbm;*
$ delete/noconfirm/log puzzle.xbm;*
$ delete/noconfirm/log image.xpm;*
$ delete/noconfirm/log puzzle.xpm;*
$!
$ exit
$!
! SUBROUTINE TO CHECK DEPENDENCIES
$ make: subroutine
$   v='f$verify(0)
$!   p1       What we are trying to make
$!   p2       Command to make it
$!   p3 - p8  What it depends on
$
$   if (f$extract(0,3,p2) .eqs. "cc ") then write optf "''p1'"
$   if (f$extract(0,3,p2) .eqs. "CC ") then write optf "''p1'"
$
$   if f$search(p1) .eqs. "" then goto MakeIt
$   time=f$cvtime(f$file(p1,"RDT"))
$   arg=3
$Loop:
$   argument=p'arg
$   if argument .eqs. "" then goto Exit
$   el=0
$Loop2:
$   file=f$element(el," ",argument)
$   if file .eqs. " " then goto Endl
$   afile=""
$Loop3:
$   ofile=afile
$   afile=f$search(file)
$   if afile .eqs. "" .or. afile .eqs. ofile then goto NextEl
$   if f$cvtime(f$file(afile,"RDT")) .ges. time then goto MakeIt
$   goto Loop3
$NextEL:
$   el=el+1
$   goto Loop2
$EndL:
$   arg=arg+1
$   if arg .le. 8 then goto Loop
$   goto Exit
$
$MakeIt:
$   set verify
$   'p2
$   vv='f$verify(0)
$Exit:
$   if v then set verify
$ endsubroutine
