#!/bin/athena/tcsh

# tcsh -x is the useful option.

# $Revision: 1.35 $

umask 2

set machine=`machtype`

#set the path to include the right xmkmf and imake
if ($machine == "sun4") then
	set comp=compiler-80
	set AFS="sun4m_53"
	attach -n $comp sunsoft
	setenv GCC_EXEC_PREFIX /mit/$comp/${machine}/lib/gcc-lib/
else if ($machine == "decmips") then
	set AFS="pmax_ul4"
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
	set path=( /usr/ccs/bin /build/bin /build/supported/afs/$AFS/dest/bin $path /mit/sunsoft/sun4bin  /mit/$comp/sun4bin)
else
	set path=( /build/bin /build/supported/afs/$AFS/dest/bin $path)
endif
rehash
echo $path

# If no owner is specified to install, default to root.
setenv INSTOPT "-o root" 

#this script assumes that a dependency list has been generated from somewhere.
#At the moment that just might be a hard coded list.

set libs1=" athena/lib/et athena/lib/ss athena/lib/hesiod athena/lib/kerberos1 third/supported/kerberos5 "

set tools="athena/etc/synctree"

set third="third/supported/afs third/supported/X11R5 third/supported/X11R4 third/supported/xfonts third/supported/motif third/supported/tcsh6 third/supported/emacs-19.28 third/supported/emacs-18.59 third/unsupported/perl-4.036 third/supported/tex third/unsupported/top third/unsupported/sysinfo third/unsupported/rcs third/unsupported/patch third/unsupported/tac third/unsupported/tools third/supported/mh.6.8"

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
    set machthird="athena/ucb/look"
endsw

set libs2=" athena/lib/kerberos2 athena/lib/acl athena/lib/gdb athena/lib/gdss athena/lib/zephyr athena/lib/moira.dev athena/lib/neos"

set etcs="athena/etc/track athena/etc/rvd athena/etc/newsyslog athena/etc/cleanup athena/etc/ftpd athena/etc/inetd athena/etc/netconfig athena/etc/gettime athena/etc/traceroute athena/etc/xdm athena/etc/scripts athena/etc/timed athena/etc/snmpd athena/etc/desync"

set bins=" athena/bin/session athena/bin/olc.dev athena/bin/finger athena/bin/ispell athena/bin/Ansi athena/bin/sendbug athena/bin/just athena/bin/rep athena/bin/cxref athena/bin/tarmail athena/bin/access athena/bin/mon athena/bin/olh athena/bin/dent athena/bin/xquota athena/bin/attach athena/bin/dash athena/bin/xmore athena/bin/mkserv athena/bin/cal athena/bin/xps athena/bin/scripts athena/bin/afs-nfs athena/bin/xdsc athena/bin/rkinit.76 athena/bin/xprint athena/bin/xversion athena/bin/kerberometer athena/bin/discuss athena/bin/from athena/bin/delete athena/bin/getcluster athena/bin/gms athena/bin/hostinfo athena/bin/machtype athena/bin/login athena/bin/tcsh athena/bin/write athena/bin/tar athena/bin/tinkerbell athena/ucb/lpr athena/ucb/quota"

set end="athena/man athena/dotfiles athena/config"

# athena/bin/inittty is not listed now. Hopefully we have a better
# solution now.
 
mkdir /build/LOGS
set outfile="/build/LOGS/washlog.`date '+%y.%m.%d.%H'`"
set SRVD="/srvd"
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

#start by building and installing imake in /build/bin


#I need to split kerberos up into phase 1 and phase 2 
# need to add in motif. Once that is done I can proceed onto the bin directory

switch ( $machine )
  case sun4
    set packages =(setup $machine $libs1 $tools $third $machthird $libs2 $etcs $bins)
    breaksw

  case rsaix
    set packages =(setup $libs1 $tools $third $machthird $libs2 $etcs $bins)
    breaksw

  case decmips
    set packages=(decmips/kits/install_srvd setup athena/lib/syslog decmips/lib/resolv $libs1 $tools $third $machthird $libs2 $etcs $bins $machine athena/etc/nfsc)

  case sgi
    set packages =(setup $libs1 $tools $third $machthird $libs2 $etcs $bins)
endsw

# at the moment, lib/resolv gets built twice...

endif

if ($installman == 1) then
  foreach package ( $packages )
    echo "Installing man in $package" >>& $outfile

    if (-e /build/$package/.build) then
	source /build/$package/.build
    else
        switch($package)
          case third/supported/xfonts
          case athena/lib/kerberos1
          case athena/lib/moira.dev
            breaksw
          case athena/lib/kerberos2
            set package="athena/lib/kerberos"
          default:
            (cd /build/$package ; make install.man >>& $outfile)
        endsw
    endif
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
	cp -p /source/third/supported/X11R5/mit/util/scripts/mkdirhier.sh $SRVD/usr/athena/bin/mkdirhier

	mkdir /build/bin
	cd /build/support/imake
		((make -f Makefile.ini clean >>& $outfile) && \
			(make -f Makefile.ini >>& $outfile ) && \
			(cp imake /build/bin >>& $outfile) && \
			(chmod 755 /build/bin/imake >>& $outfile) && \
			(cp /source/xmkmf /build/bin >>& $outfile))
		if ($status == 1 ) then
			echo "We bombed in imake" >>& $outfile
			exit -1
		endif

	rehash

	(((cd /build/setup; xmkmf . ) >>& $outfile) && \
	((cd /build/setup; make install DESTDIR=$SRVD ) >>& $outfile) )
	if ($status == 1 ) then
	        echo "We bombed in install" >>& $outfile
		exit -1
	endif

	if ($machine == "sun4" ) then
		(cd /build/sun4/include; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1 ) then
	        echo "We bombed in sun4/include" >>& $outfile
		exit -1
	endif
	endif

	if ($machine == "sun4" ) then
	cd /build/sun4/libresolv
	echo "In sun4/libresolv" >>& $outfile
		((make clean >>& $outfile) && \
		(make >>& $outfile ) && \
		(make install DESTDIR=$SRVD >>& $outfile ))
		if ($status == 1 ) then
			echo "We bombed in libresolv" >>& $outfile
			exit -1
		endif
	endif

	cd /build/support/install
	((echo "In install" >>& $outfile) &&\
	(xmkmf . >>& $outfile) &&\
	(make clean >>& $outfile) &&\
	(make >>& $outfile))
	if ($status == 1 ) then
	        echo "We bombed in install" >>& $outfile
		exit -1
	endif

# We if on these machines because we don't expect to do any of this
# for future machines, so this shouldn't require changes for new ports.
if ($machine == "decmips" || machine == "sun4" || $machine == "rsaix") then
	source /source/support/scripts/X.csh
endif

# Mark, why don't we just do everything in build/support?
#	cd /build/support/makedepend
#	((echo "In makedepend" >>& $outfile) &&\
#	(imake -I/source/third/supported/X11R5/mit/config -DTOPDIR=. -DCURDIR=. >>& $outfile) &&\
#	(make clean >>& $outfile) &&\
#	(make >>& $outfile) &&\
#	(cp makedepend /build/bin >>& $outfile))
#	if ($status == 1 ) then
#	        echo "We bombed in makedepend" >>& $outfile
#		exit -1
#	endif
# I'm sick of this for now...

	cp -p /afs/athena/system/@sys/srvd.76/usr/athena/bin/makedepend /build/bin

	rehash

	# Hack...
	if ($machine == "decmips") then
		(cp -p /source/decmips/etc/named/bin/mkdep.ultrix /build/bin/mkdep >>& $outfile)
	endif

	if ($machine == "sun4") then
		cd /build/bin
		rm -f cc
		ln -s /mit/compiler-80/sun4bin/gcc cc
		rm -f suncc
		cp -p /source/sun4/suncc .
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
	ln -s /afs/rel-eng/system/pmax_ul4/srvd /srvd >>& $outfile
	mkdir /srvd.tmp >>& $outfile
  if ($zap == 1) then			# Accident prevention
	newfs /dev/rrz3d fuji2266 >>& $outfile
  endif
	mount /dev/rz3d /srvd.tmp >>& $outfile

	(echo In $package >>& $outfile)
	( cd /build/$package ; make base update setup1 DESTDIR=/srvd.tmp >>& $outfile )
	umount /srvd.tmp >>& $outfile
	fsck /dev/rz3d >>& $outfile
	rmdir /srvd.tmp >>& $outfile
	rm /srvd >>& $outfile
	mkdir /srvd
	mount /srvd >>& $outfile
#	if ( $status == 1 ) then
#		echo "We bombed in $package" >> & $outfile
#		exit -1
#	endif
	breaksw

	case decmips
# This is gross. Same as complex, no depend. The Imakefile in
# decmips/sys is, um, kind of impressive, and can't do a make
# depend before a make all. Need to install includes before
# building.

if ( $installonly == "0" ) then
	((echo In $package : install headers >>& $outfile ) && \
	((cd /build/$package/include; make install DESTDIR=$SRVD ) >>& $outfile ) && \
	(echo In $package : make Makefile >>& $outfile ) && \
	((cd /build/$package;xmkmf . ) >>& $outfile ) && \
	(echo In $package : make Makefiles>>& $outfile ) && \
	((cd /build/$package;make Makefiles) >>& $outfile )  && \
	(echo In $package : make clean >>& $outfile ) && \
	((cd /build/$package;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd /build/$package;make all) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
endif # installonly
	((echo In $package : make install >>& $outfile ) && \
	((cd /build/$package;make install DESTDIR=$SRVD) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
	breaksw

	case third/supported/kerberos5
	((echo In $package : configure >>& $outfile) && \
	((cd /build/$package/src; ./configure --with-krb4=/usr/athena --enable-athena) >>& $outfile) && \
	(echo In $package : make clean >>& $outfile ) && \
	((cd /build/$package/src;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd /build/$package/src;make all) >> & $outfile ))

# At the moment, we only care that we've got libraries.
	if ($status == 1 ) then
		echo "K5 did not build to completion."  >>& $outfile
	endif

	if ( (! -r /build/$package/src/lib/libkrb5.a) || \
	     (! -r /build/$package/src/lib/libcrypto.a) ) then
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
	((cd /build/$package;xmkmf . ) >>& $outfile ) && \
	(echo In $package : make Makefiles>>& $outfile ) && \
	((cd /build/$package;make Makefiles) >>& $outfile )  && \
	(echo In $package : make clean >>& $outfile ) && \
	((cd /build/$package;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd /build/$package;make all) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
endif # installonly
	((echo In $package : make install >>& $outfile ) && \
	((cd /build/$package;make install DESTDIR=$SRVD) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
	breaksw
		
	case athena/lib/syslog
if ( $installonly == "0" ) then
	((echo In $package : make clean >>& $outfile ) && \
	((cd /build/$package;make clean) >>& $outfile ) && \
	(echo In $package : make all >>& $outfile ) && \
	((cd /build/$package;make all) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
endif # installonly
	((echo In $package : make install >>& $outfile ) && \
	((cd /build/$package;make install DESTDIR=$SRVD) >> & $outfile ))
	if ($status == 1 ) then
		echo "We bombed in $package"  >>& $outfile
		exit -1
	endif
	breaksw

	case third/unsupported/top
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	if ( $machine == "sun4" ) then
	(cd /build/$package ; cp Makefile.sun4 Makefile)
	(cd /build/$package ; ln -s machine/m_sunos5.c machine.c )
	(cd /build/$package ; ln -s machine/m_sunos5.man top.1)
	else if ( $machine == "decmips" ) then
	(cd /build/$package ; cp Makefile.decmips Makefile)
	(cd /build/$package ; ln -s machine/m_ultrix4.c machine.c )
	(cd /build/$package ; ln -s machine/m_ultrix4.man top.1)
	else
		echo " No top on this platform"
	endif
endif # installonly

	if ($machine != "rsaix" ) then
if ( $installonly == "0" ) then
	( cd /build/$package ; make clean >>& $outfile )
	( cd /build/$package ; make >>& $outfile )
endif # installonly
        ( cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile )
	endif
        breaksw

	case third/supported/tcsh6
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	(( cd /build/$package ; /usr/athena/bin/xmkmf >>& $outfile ) && \
	( cd /build/$package ; make clean  >>& $outfile ) && \
	( cd /build/$package ; make config.h >>& $outfile) && \
	( cd /build/$package ; make >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	if ($machine != "decmips" ) then
		 ( cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile)
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
	((cd /build/$package ; imake -DTOPDIR=. -I./util/imake.includes >>& $outfile ) && \
	(cd /build/$package ; make Makefiles >>& $outfile) && \
	(cd /build/$package ; make clean >>& $outfile) && \
	(cd /build/$package ; make depend SUBDIRS="util include lib admin " >> & $outfile) &&\
	(cd /build/$package ;make  SUBDIRS="include lib" >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd /build/$package ; make install SUBDIRS="include lib" DESTDIR=$SRVD >>& $outfile)
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
	((cd /build/$package ;make depend SUBDIRS="appl kuser server slave kadmin man" >>& $outfile) &&\
	(cd /build/$package ;make all >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile)
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
		((cd /build/$package; xmkmf . >>& $outfile) && \
		(cd /build/$package; make Makefiles >>& $outfile) && \
		(cd /build/$package; make all >>& $outfile))
		if ($status == 1) then
			echo "We bombed in $package" >>& $outfile
			exit -1
		endif
endif # installonly
		(cd /build/$package; make install DESTDIR=$SRVD >>& $outfile)
		if ($status == 1) then
			echo "We bombed in $package" >>& $outfile
			exit -1
		endif
	endif
	breaksw

	case third/supported/afs
	(echo In $package >> & $outfile )
if ( $installonly == "0" ) then
	(cd /build/$package ; xmkmf . >>& $outfile)
	(cd /build/$package ; make >>& $outfile)
	cp -rp /build/$package/$AFS/dest/lib /build/transarc
	cp -rp /build/$package/$AFS/dest/include /build/transarc
endif # installonly
	(cd /build/$package; make install DESTDIR=$SRVD >>& $outfile)
	breaksw	

	case athena/lib/zephyr.p4
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd /build/$package ; /usr/athena/bin/xmkmf $cwd  >>& $outfile ) && \
	((cd /build/$package;make Makefiles) >>& $outfile )  && \
	((cd /build/$package;make clean) >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
#if ($machine != "decmips") then
#	((cd /build/$package;make depend) >>& $outfile)
#	if ($status == 1) then
#		echo "We bombed in $package" >>& $outfile
#		exit -1
#	endif
#endif
	((cd /build/$package;make all) >> & $outfile )
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly

	(cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/bin/olc.dev
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd /build/$package ; imake -DTOPDIR=. -I./config >>& $outfile ) && \
	(cd /build/$package ; make Makefiles >>& $outfile) && \
	(cd /build/$package ; make clean >>& $outfile) && \
	(cd /build/$package ; make world >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
        (cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/bin/olh
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
        ((cd /build/$package ; imake -DTOPDIR=. -I./config >>& $outfile ) && \
	(cd /build/$package ; make Makefiles >>& $outfile) && \
	(cd /build/$package ; make clean >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd /build/$package ;make world >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case third/supported/emacs-18.59
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
# Temporarily create compat symlinks
	if ($machine == "decmips") then
		ln -s /usr/athena/include/X11 $SRVD/usr/include/X11
		ln -s /usr/athena/lib/lib[MWX]* $SRVD/usr/lib
	endif
		((cd /build/$package ; xmkmf . >>& $outfile) &&\
		(cd /build/$package/etc ; xmkmf >> $outfile) &&\
		(cd /build/$package ; make clean >>& $outfile) &&\
		(cd /build/$package ; make all >>& $outfile))
		if ($status == 1) then
			echo "We bombed in $package" >>& $outfile
			exit -1
		endif
	if ($machine == "decmips") then
		rm $SRVD/usr/lib/lib[MWX]* $SRVD/usr/include/X11
	endif
endif # installonly
	(cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	rehash
	breaksw

	case athena/lib/moira.dev
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	((cd /build/$package ; imake -DTOPDIR=. -I./util/imake.includes >>& $outfile) &&\
	(cd /build/$package; make Makefiles >>& $outfile) &&\
	(cd /build/$package; make clean >>& $outfile) &&\
	((cd /build/$package; make depend >>& $outfile) || (true)) &&\
	(cd /build/$package; make all >>& $outfile))
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif # installonly
	(cd /build/$package; make install SUBDIRS="lib gdb" DESTDIR=$SRVD)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
	breaksw

	case athena/bin/tinkerbell
	if ($machine == "sun4") then
		(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
		((cd /build/$package;xmkmf . >>& $outfile ) && \
		(cd /build/$package;make clean >>& $outfile ) && \
		(cd /build/$package;make depend >>& $outfile) && \
		(cd /build/$package;make all >>& $outfile ))
		if ( $status == 1 ) then
			echo "We bombed in $package"  >>& $outfile
			exit -1
		endif
endif # installonly
		(cd /build/$package;make install DESTDIR=$SRVD >>& $outfile )
		if ( $status == 1 ) then
			echo "We bombed in $package"  >>& $outfile
			exit -1
		endif
	endif
	breaksw

	default:
		if (-e /build/$package/.build) then
			source /build/$package/.build
			if ($status != 0) exit $status
		else
			if (-e /build/$package/.rule) then
				set rule = `cat /build/$package/.rule`
			else
				set rule = simple
			endif
		endif

		switch ($rule)

		case complex
if ( $installonly == "0" ) then
 ((echo In $package : make Makefile  >>& $outfile ) && \
 ((cd /build/$package;xmkmf . ) >>& $outfile ) && \
 (echo In $package : make Makefiles>>& $outfile ) && \
 ((cd /build/$package;make Makefiles) >>& $outfile )  && \
 (echo In $package : make clean >>& $outfile ) && \
 ((cd /build/$package;make clean) >>& $outfile ) && \
 (echo In $package : make depend >>& $outfile) && \
 ((cd /build/$package;make depend) >>& $outfile) && \
 (echo In $package : make all >>& $outfile ) && \
 ((cd /build/$package;make all) >> & $outfile ))
   if ($status == 1 ) then
	echo "We bombed in $package"  >>& $outfile
	exit -1
   endif
endif # installonly

 ((echo In $package : make install >>& $outfile ) && \
 ((cd /build/$package;make install DESTDIR=$SRVD) >> & $outfile ))
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
 ((cd /build/$package;xmkmf . ) >>& $outfile ) && \
 (echo In $package : make clean >>& $outfile ) && \
 ((cd /build/$package;make clean) >>& $outfile ) && \
 (echo In $package : make depend >>& $outfile) && \
 ((cd /build/$package;make depend) >>& $outfile) && \
 (echo In $package : make all >>& $outfile ) && \
 ((cd /build/$package;make all) >> & $outfile ))
   if ($status == 1 ) then
        echo "We bombed in $package"  >>& $outfile
        exit -1
   endif
endif # installonly

 ((echo In $package : make install >>& $outfile ) && \
 ((cd /build/$package;make install DESTDIR=$SRVD) >> & $outfile ))
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
