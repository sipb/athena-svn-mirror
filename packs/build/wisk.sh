#!/bin/athena/tcsh 

# tcsh -x is the useful option.

# usage: wisk [-install | -installman ] [-zap] [startpackage [endpackage]]
#
#	-install	only do make installs
#	-installman	only do make install.man (the only thing that does it)
#	-zap		on DECstation, run a newfs on the /srvd partition,
#			if you're using a local one. Uses a hard coded
#			value that was once right, but probably isn't now.
#	startpackage	the name (such as third/supported/emacs) of the
#			package in the package list to start building at
#	endpackage	the name of the package in the package list to
#			stop building at

# $Revision: 1.57 $

umask 2

set machine=`machtype`

set SRVD="/srvd"
set BUILD="/build"
set SOURCE="/source"

#set the path to include the right xmkmf and imake
if ($machine == "sun4") then
	set comp=compiler-80
	set AFS="sun4m_53"
	attach -n $comp sunsoft
	setenv GCC_EXEC_PREFIX /mit/$comp/${machine}/lib/gcc-lib/
else if ($machine == "decmips") then
	set AFS="pmax_ul4"
	setenv OSVER `uname -r | sed s/\\.//`
	echo $OSVER
else if ($machine == "rsaix" ) then
#	setenv  GCC_EXEC_PREFIX /mit/cygnus-930630/rsaix/lib/gcc-lib/
	set AFS = "rs_aix32"
#	attach -n compiler-80
else if ($machine == "sgi") then
	set AFS="sgi_52"
endif
if ($machine == "sun4") then
setenv LD_LIBRARY_PATH /usr/openwin/lib
endif

if ( $machine == "sun4" ) then
	set path=( /usr/ccs/bin $BUILD/bin $BUILD/supported/afs/$AFS/dest/bin /usr/athena/bin /bin/athena $path /mit/sunsoft/sun4bin  /mit/$comp/sun4bin)
else
	set path=( $BUILD/bin $BUILD/supported/afs/$AFS/dest/bin /usr/athena/bin /bin/athena $path /usr/bin/X11)
endif
rehash
echo $path

# If no owner is specified to install, default to root.
setenv INSTOPT "-o root" 

#this script assumes that a dependency list has been generated from somewhere.
#At the moment that just might be a hard coded list.

set libs1=" athena/lib/et athena/lib/ss athena/lib/hesiod athena/lib/kerberos1 third/supported/kerberos5 "

set tools="athena/etc/synctree"

set third="third/supported/afs third/supported/X11R5 third/supported/X11R4 third/supported/xfonts third/supported/motif third/supported/wcl athena/lib/Mu third/supported/tcsh6 third/supported/emacs-19.28 third/supported/emacs-18.59 third/unsupported/perl-4.036 third/supported/tex third/unsupported/top third/unsupported/sysinfo third/unsupported/rcs third/unsupported/patch third/unsupported/tac third/unsupported/tools third/supported/mh.6.8"

switch ( $machine )
  case decmips
    set machthird="third/unsupported/ditroff third/unsupported/transcript-v2.1 third/supported/saber-3.0.1 athena/ucb/tn3270 third/unsupported/gcore"
    breaksw

  case sun4
    set machthird="third/unsupported/transcript-v2.1 third/unsupported/ansi third/unsupported/jove third/unsupported/learn"
    breaksw

  case rsaix
    set machthird="third/unsupported/ansi third/unsupported/jove third/unsupported/learn"
    breaksw

  case sgi
    set machthird="third/unsupported/transcript-v2.1 athena/ucb/look athena/lib/AL"
endsw

set libs2=" athena/lib/kerberos2 athena/lib/acl athena/lib/gdb athena/lib/gdss athena/lib/zephyr athena/lib/neos"

# athena/lib/moira.dev ; I think this is not ours at the moment.

set etcs="athena/etc/track athena/etc/rvd athena/etc/newsyslog athena/etc/cleanup athena/etc/ftpd athena/etc/inetd athena/etc/inetd.new athena/etc/listsuidcells athena/etc/netconfig athena/etc/gettime athena/etc/traceroute athena/etc/xdm athena/etc/scripts athena/etc/timed athena/etc/snmpd athena/etc/desync"

# Decomissioned 9/95 by fiat
# athena/bin/xps athena/bin/afs-nfs athena/bin/xprint athena/bin/kerberometer

set bins=" athena/bin/session athena/bin/olc.dev athena/bin/finger athena/bin/ispell athena/bin/Ansi athena/bin/sendbug athena/bin/just athena/bin/rep athena/bin/cxref athena/bin/tarmail athena/bin/access athena/bin/mon athena/bin/dent athena/bin/attach athena/bin/dash athena/bin/xmore athena/bin/mkserv athena/bin/cal athena/bin/scripts athena/bin/xdsc athena/bin/rkinit.76 athena/bin/xversion athena/bin/discuss athena/bin/from athena/bin/delete athena/bin/getcluster athena/bin/gms athena/bin/hostinfo athena/bin/lert athena/bin/machtype athena/bin/login athena/bin/tcsh athena/bin/write athena/bin/tinkerbell athena/bin/athdir athena/ucb/lpr athena/ucb/quota"

set machbins=""
switch ( $machine )
  case sun4
    set machbins="athena/bin/xquota"
    breaksw
endsw

# athena/bin/tar is leftover from vax & rt
# athena/bin/olh removed, superseded by web browsers

set end="athena/man athena/dotfiles athena/config"

# athena/bin/inittty is not listed now. Hopefully we have a better
# solution now.
 
mkdir $BUILD/LOGS
set outfile="$BUILD/LOGS/washlog.`date '+%y.%m.%d.%H'`"
set X="X11R4"
set MOTIF="motif"
set found=0
set installonly=0
set installman=0
set done=0
set zap=0

while ( $#argv > 0 )
  switch( $1 )

    case -install:
      set installonly=1
      shift
      breaksw

    case -installman:
      set installman=1
      shift
      breaksw

    case -zap:
      set zap=1
      shift
      breaksw

    default:
      break

    endsw
end

echo ======== >>! $outfile
echo starting `date` >> $outfile
echo on a $machine >> $outfile

#start by building and installing imake in $BUILD/bin


#I need to split kerberos up into phase 1 and phase 2 
# need to add in motif. Once that is done I can proceed onto the bin directory

switch ( $machine )
  case sun4
    set packages =(setup $machine $libs1 $tools $third $machthird $libs2 \
			$etcs $bins $machbins)
    breaksw

  case rsaix
    set packages =(setup $libs1 $tools $third $machthird $libs2 $etcs $bins \
			$machbins)
    breaksw

  case decmips
    set packages=(decmips/kits/install_srvd setup athena/lib/syslog \
	decmips/lib/resolv $libs1 $tools $third $machthird $libs2 $etcs $bins \
	$machbins $machine athena/etc/nfsc athena/etc/checkfpu athena/bin/AL \
	athena/bin/telnet $end)

    breaksw

  case sgi
    set packages =(setup $libs1 $tools $third $machthird $libs2 $etcs $bins)
endsw

# at the moment, lib/resolv gets built twice...

endif

if ($installman == 1) then
	foreach package ( $packages )
		switch($package)
			case athena/lib/kerberos1
			case athena/lib/moira.dev
				breaksw
			case athena/lib/kerberos2
			        set package="athena/lib/kerberos"
			default:
				echo "Installing man in $package" >>& $outfile

				if (-e $BUILD/$package/.build) then
				   source $BUILD/$package/.build
				   if ($status != 0) exit $status
				else
				   if (-e $BUILD/$package/.rule) then
					set rule = `cat $BUILD/$package/.rule`
			  	   else
					set rule = simple
				   endif
				endif

				if ($rule != "skip") then
				   (cd $BUILD/$package ; make install.man >>& $outfile)
			    	endif
		   endsw
	end
	exit 0
endif


foreach package ( $packages )

# The following code before the switch should be changed to filter
# packages, and be moved to right after packages is generated. Then
# it will apply to both installman and what it works for now.

if ($found == "0" && $1 != "") then
  if ($1 == $package) then
    set found=1
  else
    continue
  endif
endif

if ($done == 1) then
  break
endif

if ($done == 0 && $2 != "") then
  if ($2 == $package) then
    set done=1
  endif
endif

echo "**********************" >>& $outfile
echo "***** In $package" >>& $outfile

switch ($package)
	case setup
	(echo In setup >>& $outfile)

	# Probably want this in build/bin... change Imake.tmpl...
	mkdir -p $SRVD/usr/athena/bin
	cp -p $SOURCE/third/supported/X11R5/mit/util/scripts/mkdirhier.sh $SRVD/usr/athena/bin/mkdirhier

# The wrong hack for now. Should make a copy of what we use for posterity,
# and link to that.
if ($machine == "sun4" || $machine == "sgi") then
	rm $BUILD/transarc
	ln -s /afs/athena/astaff/project/afsdev/dist/@sys $BUILD/transarc
endif

	mkdir $BUILD/bin
	cd $BUILD/support/imake
		((make -f Makefile.ini clean >>& $outfile) && \
			(make -f Makefile.ini >>& $outfile ) && \
			(cp imake $BUILD/bin >>& $outfile) && \
			(chmod 755 $BUILD/bin/imake >>& $outfile) && \
			(cp $SOURCE/xmkmf $BUILD/bin >>& $outfile))
		if ($status == 1 ) then
			echo "We bombed in imake" >>& $outfile
			exit -1
		endif

	rehash

	(((cd $BUILD/setup; xmkmf . ) >>& $outfile) && \
	((cd $BUILD/setup; make install DESTDIR=$SRVD ) >>& $outfile) )
	if ($status == 1 ) then
	        echo "We bombed in setup install" >>& $outfile
		exit -1
	endif

	if ($machine == "sun4" ) then
		(cd $BUILD/sun4/include; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1 ) then
	        echo "We bombed in sun4/include" >>& $outfile
		exit -1
	endif
	endif

	if ($machine == "sun4" ) then
	cd $BUILD/sun4/libresolv
	echo "In sun4/libresolv" >>& $outfile
		((make clean >>& $outfile) && \
		(make >>& $outfile ) && \
		(make install DESTDIR=$SRVD >>& $outfile ))
		if ($status == 1 ) then
			echo "We bombed in libresolv" >>& $outfile
			exit -1
		endif
	endif

	cd $BUILD/support/install
	((echo "In install" >>& $outfile) &&\
	(xmkmf . >>& $outfile) &&\
	(make clean >>& $outfile) &&\
	(make >>& $outfile) &&\
	(./pinstall -c pinstall $BUILD/bin/install) &&\
	(make install DESTDIR=$SRVD >>& $outfile))
	if ($status == 1 ) then
	        echo "We bombed in install" >>& $outfile
		exit -1
	endif

# We if on these machines because we don't expect to do any of this
# for future machines, so this shouldn't require changes for new ports.
if ($machine == "decmips" || machine == "sun4" || $machine == "rsaix") then
	source $BUILD/support/scripts/X.csh
endif

# Mark, why don't we just do everything in build/support?
#	cd $BUILD/support/makedepend
#	((echo "In makedepend" >>& $outfile) &&\
#	(imake -I$SOURCE/third/supported/X11R5/mit/config -DTOPDIR=. -DCURDIR=. >>& $outfile) &&\
#	(make clean >>& $outfile) &&\
#	(make >>& $outfile) &&\
#	(cp makedepend $BUILD/bin >>& $outfile))
#	if ($status == 1 ) then
#	        echo "We bombed in makedepend" >>& $outfile
#		exit -1
#	endif
# I'm sick of this for now...

	cp -p /afs/athena/system/@sys/srvd.76/usr/athena/bin/makedepend $BUILD/bin

	rehash

	# Hack...
	if ($machine == "decmips") then
		(cp -p $SOURCE/decmips/etc/named/bin/mkdep.ultrix $BUILD/bin/mkdep >>& $outfile)
	endif

	if ($machine == "sun4") then
		cd $BUILD/bin
		rm -f cc
		ln -s /mit/compiler-80/sun4bin/gcc cc
		rm -f suncc
		cp -p $SOURCE/sun4/suncc .
	endif

	breaksw

	case decmips/kits/install_srvd
#	This is scary.
#	Unmount /srvd remove the directory make a link then at the end 
#	reverse the process.
#WARNING: There's a newfs here. Make sure it gets changed appropriately.

# Not technically correct, but it's what I want right now.
if ( $installonly == "1" ) then
  continue
endif # installonly

	umount /srvd >>& $outfile
	rmdir /srvd >>& $outfile
	ln -s /afs/dev/system/pmax_ul4/srvd /srvd >>& $outfile
	mkdir /srvd.tmp >>& $outfile
  if ($zap == 1) then			# Accident prevention
	newfs /dev/rrz1e fuji2266 >>& $outfile
  endif
	mount /dev/rz1e /srvd.tmp >>& $outfile

	(echo In $package >>& $outfile)
	( cd $BUILD/$package ; make base update setup1 DESTDIR=/srvd.tmp >>& $outfile )
	umount /srvd.tmp >>& $outfile
	fsck /dev/rz1e >>& $outfile
	rmdir /srvd.tmp >>& $outfile
	rm /srvd >>& $outfile
	mkdir /srvd
	mount /dev/rz1e /srvd >>& $outfile
	if ( $status == 1 ) then
		echo "We bombed in $package" >> & $outfile
		exit -1
	endif
	breaksw

	case decmips
# This is gross. Same as complex, no depend. The Imakefile in
# decmips/sys is, um, kind of impressive, and can't do a make
# depend before a make all. Need to install includes before
# building.

if ( $installonly == "0" ) then
	((echo In $package : install headers >>& $outfile ) && \
	((cd $BUILD/$package/include; make install DESTDIR=$SRVD ) >>& $outfile ) && \
	(echo In $package : make Makefile >>& $outfile ) && \
	((cd $BUILD/$package;xmkmf . ) >>& $outfile ) && \
	(echo In $package : make Makefiles>>& $outfile ) && \
	((cd $BUILD/$package;make Makefiles) >>& $outfile )  && \
	(echo In $package : make clean >>& $outfile ) && \
	((cd $BUILD/$package;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd $BUILD/$package;make all) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
endif # installonly
	((echo In $package : make install >>& $outfile ) && \
	((cd $BUILD/$package;make install DESTDIR=$SRVD) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
	breaksw

	case third/supported/kerberos5
	((echo In $package : configure >>& $outfile) && \
	((cd $BUILD/$package/src; ./configure --with-ccopts=-O --with-krb4=/usr/athena --enable-athena) >>& $outfile) && \
	(echo In $package : make clean >>& $outfile ) && \
	((cd $BUILD/$package/src;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd $BUILD/$package/src;make all) >> & $outfile ))

# At the moment, we only care that we've got libraries.
	if ($status == 1 ) then
		echo "K5 did not build to completion."  >>& $outfile
	endif

	if ( (! -r $BUILD/$package/src/lib/libkrb5.a) || \
	     (! -r $BUILD/$package/src/lib/libcrypto.a) ) then
			echo "We bombed in $package"  >>& $outfile
			exit -1
	else
		echo "K5 libraries exist." >>& $outfile
	endif
	breaksw

	case third/supported/tex
	case third/supported/mh.6.8
# Same as complex, no depend.

if ( $installonly == "0" ) then
	((echo In $package : make Makefile >>& $outfile ) && \
	((cd $BUILD/$package;xmkmf . ) >>& $outfile ) && \
	(echo In $package : make Makefiles>>& $outfile ) && \
	((cd $BUILD/$package;make Makefiles) >>& $outfile )  && \
	(echo In $package : make clean >>& $outfile ) && \
	((cd $BUILD/$package;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd $BUILD/$package;make all) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
endif # installonly
	((echo In $package : make install >>& $outfile ) && \
	((cd $BUILD/$package;make install DESTDIR=$SRVD) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
	breaksw
		
	case athena/lib/syslog
if ( $installonly == "0" ) then
	((echo In $package : make clean >>& $outfile ) && \
	((cd $BUILD/$package;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd $BUILD/$package;make all) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
endif # installonly
	((echo In $package : make install >>& $outfile ) && \
	((cd $BUILD/$package;make install DESTDIR=$SRVD) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
	breaksw

	case third/supported/tcsh6
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	(( cd $BUILD/$package ; /usr/athena/bin/xmkmf >>& $outfile ) && \
	( cd $BUILD/$package ; make clean  >>& $outfile ) && \
	( cd $BUILD/$package ; make config.h >>& $outfile) && \
	( cd $BUILD/$package ; make >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	if ($machine != "decmips" ) then
		 ( cd $BUILD/$package ; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	endif
	breaksw

	case athena/lib/kerberos1
# There is one thing to note about kerberos
# kerberos needs afs and afs needs kerberos. this has to be 
# addressed at some point
	set package="athena/lib/kerberos"
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd $BUILD/$package ; imake -DTOPDIR=. -I./util/imake.includes >>& $outfile ) && \
	(cd $BUILD/$package ; make Makefiles >>& $outfile) && \
	(cd $BUILD/$package ; make clean >>& $outfile) && \
	(cd $BUILD/$package ; make depend SUBDIRS="util include lib admin " >> & $outfile) &&\
	(cd $BUILD/$package ;make  SUBDIRS="include lib" >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd $BUILD/$package ; make install SUBDIRS="include lib" DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/lib/kerberos2
# There is one thing to note about kerberos
# kerberos needs afs and afs needs kerberos. this has to be 
# addressed at some point
	set package="athena/lib/kerberos"
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd $BUILD/$package ;make depend SUBDIRS="appl kuser server slave kadmin man" >>& $outfile) &&\
	(cd $BUILD/$package ;make all >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd $BUILD/$package ; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	rehash
	breaksw

	case third/supported/xfonts
	if ($machine == "decmips") then
		echo In $package >>& $outfile
if ($installonly == 0) then
		((cd $BUILD/$package; xmkmf . >>& $outfile) && \
		(cd $BUILD/$package; make Makefiles >>& $outfile) && \
		(cd $BUILD/$package; make all >>& $outfile))
		if ($status == 1) then
			echo "We bombed in $package" >>& $outfile
			exit -1
		endif
endif # installonly
		(cd $BUILD/$package; make install DESTDIR=$SRVD >>& $outfile)
		if ($status == 1) then
			echo "We bombed in $package" >>& $outfile
			exit -1
		endif
	endif
	breaksw

	case third/supported/afs
	(echo In $package >> & $outfile )
if ( $installonly == "0" ) then
	(cd $BUILD/$package ; xmkmf . >>& $outfile)
	(cd $BUILD/$package ; make >>& $outfile)
endif # installonly
	mkdir $BUILD/transarc
	cp -rp $BUILD/$package/$AFS/dest/lib $BUILD/transarc >>& $outfile
	cp -rp $BUILD/$package/$AFS/dest/include $BUILD/transarc  >>& $outfile
	(cd $BUILD/$package; make install DESTDIR=$SRVD >>& $outfile)
	breaksw	

	case athena/lib/zephyr
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd $BUILD/$package ; /usr/athena/bin/xmkmf $cwd  >>& $outfile ) && \
	((cd $BUILD/$package;make Makefiles) >>& $outfile )  && \
	((cd $BUILD/$package;make clean) >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
#if ($machine != "decmips") then
#	((cd $BUILD/$package;make depend) >>& $outfile)
#	if ($status == 1) then
#		echo "We bombed in $package" >>& $outfile
#		exit -1
#	endif
#endif
	((cd $BUILD/$package;make all) >> & $outfile )
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly

	(cd $BUILD/$package ; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/lib/Mu
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd $BUILD/$package ; /usr/athena/bin/xmkmf  >>& $outfile ) && \
	((cd $BUILD/$package;make Makefiles) >>& $outfile )  && \
	((cd $BUILD/$package;make clean) >>& $outfile) && \
	((cd $BUILD/$package;make depend) >>& $outfile) && \
	((cd $BUILD/$package;make all) >> & $outfile ))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	if ($machine == "decmips") then
		(cd $BUILD/$package ; make install \
			INCROOT=$SRVD/usr/athena/include \
			USRLIBDIR=/mit/motif/decmipslib  >>& $outfile)
	else
		(cd $BUILD/$package ; make install DESTDIR=$SRVD >>& $outfile)
	endif
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/bin/olc.dev
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd $BUILD/$package ; imake -DTOPDIR=. -I./config >>& $outfile ) && \
	(cd $BUILD/$package ; make Makefiles >>& $outfile) && \
	(cd $BUILD/$package ; make clean >>& $outfile) && \
	(cd $BUILD/$package ; make world >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
        (cd $BUILD/$package ; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/bin/olh
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
        ((cd $BUILD/$package ; imake -DTOPDIR=. -I./config >>& $outfile ) && \
	(cd $BUILD/$package ; make Makefiles >>& $outfile) && \
	(cd $BUILD/$package ; make clean >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd $BUILD/$package ;make world >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/lib/moira.dev
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd $BUILD/$package ; imake -DTOPDIR=. -I./util/imake.includes >>& $outfile) &&\
	(cd $BUILD/$package; make Makefiles >>& $outfile) &&\
	(cd $BUILD/$package; make clean >>& $outfile) &&\
	((cd $BUILD/$package; make depend >>& $outfile) || (true)) &&\
	(cd $BUILD/$package; make all >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd $BUILD/$package; make install SUBDIRS="lib gdb" DESTDIR=$SRVD)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/bin/tinkerbell
	if ($machine == "sun4") then
		(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
		((cd $BUILD/$package;xmkmf . >>& $outfile ) && \
		(cd $BUILD/$package;make clean >>& $outfile ) && \
		(cd $BUILD/$package;make depend >>& $outfile) && \
		(cd $BUILD/$package;make all >>& $outfile ))
		if ( $status == 1 ) then
			echo "We bombed in $package"  >>& $outfile
			exit -1
		endif
endif # installonly
		(cd $BUILD/$package;make install DESTDIR=$SRVD >>& $outfile )
		if ( $status == 1 ) then
			echo "We bombed in $package"  >>& $outfile
			exit -1
		endif
	endif
	breaksw

	default:
		if (-e $BUILD/$package/.build) then
			source $BUILD/$package/.build
			if ($status != 0) exit $status
		else
			if (-e $BUILD/$package/.rule) then
				set rule = `cat $BUILD/$package/.rule`
			else
				set rule = simple
			endif
		endif

		switch ($rule)

		case complex
if ( $installonly == "0" ) then
 ((echo In $package : make Makefile  >>& $outfile ) && \
 ((cd $BUILD/$package;xmkmf . ) >>& $outfile ) && \
 (echo In $package : make Makefiles>>& $outfile ) && \
 ((cd $BUILD/$package;make Makefiles) >>& $outfile )  && \
 (echo In $package : make clean >>& $outfile ) && \
 ((cd $BUILD/$package;make clean) >>& $outfile ) && \
 (echo In $package : make depend >>& $outfile) && \
 ((cd $BUILD/$package;make depend) >>& $outfile) && \
 (echo In $package : make all >>& $outfile ) && \
 ((cd $BUILD/$package;make all) >> & $outfile ))
   if ($status == 1 ) then
	echo "We bombed in $package"  >>& $outfile
	exit -1
   endif
endif # installonly

 ((echo In $package : make install >>& $outfile ) && \
 ((cd $BUILD/$package;make install DESTDIR=$SRVD) >> & $outfile ))
   if ($status == 1 ) then
	echo "We bombed in $package"  >>& $outfile
	exit -1
   endif
 rehash
		breaksw

		case skip
		breaksw

		case simple
		default:
if ( $installonly == "0" ) then
 ((echo In $package : make Makefile  >>& $outfile ) && \
 ((cd $BUILD/$package;xmkmf . ) >>& $outfile ) && \
 (echo In $package : make clean >>& $outfile ) && \
 ((cd $BUILD/$package;make clean) >>& $outfile ) && \
 (echo In $package : make depend >>& $outfile) && \
 ((cd $BUILD/$package;make depend) >>& $outfile) && \
 (echo In $package : make all >>& $outfile ) && \
 ((cd $BUILD/$package;make all) >> & $outfile ))
   if ($status == 1 ) then
        echo "We bombed in $package"  >>& $outfile
        exit -1
   endif
endif # installonly

 ((echo In $package : make install >>& $outfile ) && \
 ((cd $BUILD/$package;make install DESTDIR=$SRVD) >> & $outfile ))
   if ($status == 1 ) then
        echo "We bombed in $package"  >>& $outfile
        exit -1
   endif
 rehash
		breaksw
		endsw
	breaksw
endsw
end
echo ending `date` >>& $outfile
