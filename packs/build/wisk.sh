#!/bin/athena/tcsh

umask 2

set machine=`machtype`

#set the path to include the right xmkmf and imake
if ($machine == "sun4") then
	set AFS="sun4m_53"
else if ($machine == "decmips") then
	set AFS="pmax_ul4"
else if ($machine == "rsaix" ) then
	setenv  GCC_EXEC_PREFIX /mit/cygnus-930630/rsaix/lib/gcc-lib/
	set AFS = "rs_aix32"
	attach -n cygnus
endif
if ($machine == "sun4") then
setenv LD_LIBRARY_PATH /usr/openwin/lib
endif

if ( $machine == "sun4" ) then
	set path=( /usr/ccs/bin /build/bin /build/supported/afs/$AFS/dest/bin $path /mit/sunsoft/sun4bin  /usr/gcc/bin /usr/gcc/lib)
else
	set path=( /build/bin /build/supported/afs/$AFS/dest/bin $path)
endif
rehash
echo $path

#this script assumes that a dependency list has been generated from somewhere.
#At the moment that just might be a hard coded list.

set libs1=" athena/lib/et athena/lib/ss athena/lib/hesiod athena/lib/kerberos1 "

set tools="athena/etc/synctree"

set third="third/supported/afs third/supported/X11R5 third/supported/X11R4 third/supported/xfonts third/supported/motif third/supported/tcsh6 third/supported/emacs-18.59 third/unsupported/perl third/supported/tex third/unsupported/top third/unsupported/sysinfo third/unsupported/rcs third/unsupported/patch third/unsupported/tac third/unsupported/tools third/supported/mh.6.8"

switch ( $machine )
  case decmips
    set machthird="third/unsupported/ditroff third/unsupported/transcript-v2.1"
    breaksw

  case sun4
    set machthird="third/unsupported/transcript-v2.1 third/unsupported/ansi third/unsupported/jove third/unsupported/learn"
    breaksw

  case rsaix
    set machthird="third/unsupported/ansi third/unsupported/jove third/unsupported/learn"
    breaksw
endsw

set libs2=" athena/lib/kerberos2 athena/lib/acl athena/lib/gdb athena/lib/gdss athena/lib/zephyr.p4 athena/lib/moira.dev athena/lib/neos"

set etcs="athena/etc/track athena/etc/rvd athena/etc/newsyslog athena/etc/cleanup athena/etc/ftpd athena/etc/inetd athena/etc/netconfig athena/etc/gettime athena/etc/traceroute athena/etc/xdm athena/etc/scripts athena/etc/timed athena/etc/snmpd"

set bins=" athena/bin/session athena/bin/olc.dev athena/bin/finger athena/bin/ispell athena/bin/Ansi athena/bin/sendbug athena/bin/just athena/bin/rep athena/bin/cxref athena/bin/tarmail athena/bin/access athena/bin/mon athena/bin/olh athena/bin/dent athena/bin/xquota athena/bin/attach athena/bin/dash athena/bin/xmore athena/bin/mkserv athena/bin/cal athena/bin/xps athena/bin/scripts athena/bin/afs-nfs athena/bin/xdsc athena/bin/rkinit.76 athena/bin/xprint athena/bin/xversion athena/bin/viewscribe athena/bin/kerberometer athena/bin/discuss athena/bin/from athena/bin/delete athena/bin/getcluster athena/bin/gms athena/bin/hostinfo athena/bin/machtype athena/bin/login athena/bin/tcsh athena/bin/write athena/bin/tar athena/bin/tinkerbell"

set end="config/dotfiles config/config"

# athena/bin/inittty is not listed now. Hopefully we have a better
# solution now.
 
set outfile="/usr/tmp/washlog.`date '+%y.%m.%d.%H'`"
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

echo starting `date` > $outfile
echo on a $machine >> $outfile

#start by building and installing imake in /build/bin


#I need to split kerberos up into phase 1 and phase 2 
# need to add in motif. Once that is done I can proceed onto the bin directory

if ($machine == "sun4") then

set packages = setup $machine $libs1 $tools $third $machthird $libs2 $etcs $bins

else if ($machine == "rsaix" ) then

set packages = setup $libs1 $tools $third $machthird $libs2 $etcs $bins

else

# if ($machine == "decmips") then...

set packages=(decmips/kits/install_srvd setup athena/lib/syslog decmips/lib/resolv $libs1 $tools $third $machthird $libs2 $etcs $bins $machine athena/etc/nfsc)

# at the moment, lib/resolv gets built twice...

endif

if ($installman == 1) then
  foreach package ( $packages )
    echo "Installing man in $package" >>& $outfile
    switch($package)
      case third/supported/X11R4
      case third/supported/xfonts
      case athena/lib/kerberos1
      case athena/lib/moira.dev
        breaksw
      case third/supported/X11R5
        (cd /build/$package/mit ; make install.man SUBDIRS="util clients demos" >>& $outfile)
        breaksw
      case athena/lib/kerberos2
        set package="athena/lib/kerberos"
      default:
        (cd /build/$package ; make install.man >>& $outfile)
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

switch ($package)
	case setup
	(echo In setup >>& $outfile)

	mkdir /build/bin
	cd /build/support/imake
		((make -f Makefile.ini clean >>& $outfile) && \
			(make -f Makefile.ini >>& $outfile ) && \
			(cp imake /build/bin >>& $outfile) && \
			(chmod 755 /build/bin/imake >>& $outfile) && \
			(cp /source/xmkmf /build/bin >>& $outfile))
		if ($status == 1 ) then
			echo "We bombed in imake" >>& $outfile
		endif
	if ($machine == "sun4" ) then
		(cd /build/sun4/include; make install)
	endif

	rehash
	cd /build/support/install
	((echo "In install" >>& $outfile) &&\
	(xmkmf . >>& $outfile) &&\
	(make clean >>& $outfile) &&\
	(make >>& $outfile))
	if ($status == 1 ) then
	        echo "We bombed in install" >>& $outfile
		exit -1
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

# following used to be below...

	# Probably want this in build/bin... change Imake.tmpl...
	mkdir $SRVD/usr/athena
	mkdir $SRVD/usr/athena/bin
	cp -p /source/third/supported/X11R5/mit/util/scripts/mkdirhier.sh $SRVD/usr/athena/bin/mkdirhier
	# Hack...
	if ($machine == "decmips") then
		(cp -p /source/decmips/etc/named/bin/mkdep.ultrix /build/bin/mkdep >>& $outfile)
	endif
	rehash

	(((cd /build/setup; xmkmf . ) >>& $outfile) && \
	((cd /build/setup; make install DESTDIR=$SRVD ) >>& $outfile) )
	if ($status == 1 ) then
	        echo "We bombed in install" >>& $outfile
		exit -1
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
		
	case third/supported/tex
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

	case third/unsupported/perl
	case third/unsupported/perl-4.036
	(echo In $package >>& $outfile)
	set PERLFILES = "  ./x2p/s2p.SH ./x2p/find2perl.SH ./x2p/Makefile.SH ./x2p/cflags.SH ./Makefile.SH ./config_h.SH ./c2ph.SH ./h2ph.SH ./makedepend.SH ./cflags.SH ./makedir.SH"
if ( $installonly == "0" ) then
	( cd /build/$package ; cp config.sh.$machine config.sh) 
	( cd /build/$package  ; cp cppstdin.$machine cppstdin ) 
	 cd /build/$package ; \
		  foreach p ($PERLFILES)
		   sh $p >>& $outfile
		  end 
	(( cd /build/$package ;  make clean >>& $outfile) && \
	( cd /build/$package ;  make depend >>& $outfile) && \
	( cd /build/$package ;  make >>& $outfile) && \
	( cd /build/$package ;  make test >>& $outfile))
        if ( $status == 1 ) then
                echo "We bombed in $package" >> & $outfile
                exit -1
        endif
endif #installonly

	cd /build/$package
	(( make install DESTDIR=$SRVD >>& $outfile) &&\
	 (( sed -e "s:/usr/:$SRVD/usr/:" h2ph > h2phsrvd ) >>& $outfile) && \
	 ( chmod 755 h2phsrvd >>& $outfile ) && \
	 (( cd $SRVD/usr/include; /build/$package/h2phsrvd * */* ) >>& $outfile ) && \
	 (( sed -e "s:/usr/include:/usr/athena/include:" h2phsrvd > hath2phsrvd ) >>& $outfile) && \
	 ( chmod 755 hath2phsrvd >>& $outfile ) && \
	 (( cd $SRVD/usr/athena/include; /build/$package/hath2phsrvd * */* ) >>& $outfile) && \
	 ( rm -f h2phsrvd hath2phsrvd >>& $outfile ))
        if ( $status == 1 ) then
                echo "We bombed in $package" >> & $outfile
                exit -1
        endif
	breaksw

	case third/unsupported/sysinfo
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	( cd /build/$package ; make clean >>& $outfile )
	( cd /build/$package ; make >>& $outfile )
endif # installonly
	(cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile )
	rehash
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

# Ummm... We don't do ls anymore, right?
# We might like to call it something different and let people
# alias it? What's the deal here anyway?
	case athena/bin/ls
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	(( cd /build/$package ; make clean  >>& $outfile ) && \
	( cd /build/$package ; make -f Makefile.$machine >>& $outfile))
        if ( $status == 1 ) then
                echo "We bombed in $package" >> & $outfile
                exit -1
        endif
endif # installonly
	( cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile)
        if ( $status == 1 ) then
                echo "We bombed in $package" >> & $outfile
                exit -1
        endif

	breaksw

	case third/supported/tcsh6
	(echo In $package >>& $outfile)
if ( $installonly == "0" ) then
	( cd /build/$package ; /usr/athena/bin/xmkmf >>& $outfile )
	( cd /build/$package ; make clean  >>& $outfile )
	( cd /build/$package ; make config.h >>& $outfile)
	( cd /build/$package ; make >>& $outfile)
endif # installonly
	if ($machine != "decmips" ) then
		 ( cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile)
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

	case third/supported/X11R4
	(echo In $package >> & $outfile) 
if ( $installonly == "0" ) then
	(cd /build/$package ; make -k World >>& $outfile)
endif # installonly
	(cd /build/$package ; make -k install SUBDIRS="include lib extensions" DESTDIR=$SRVD >>& $outfile)
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

	case third/supported/motif
	(echo In $package >> & $outfile)
if ( $installonly == "0" ) then
	(cd /build/$package ; imake -DTOPDIR=. -I./config/ >>& $outfile)
	(cd /build/$package ; make -k World >>& $outfile)
endif # installonly
	(cd /build/$package ; make -k install DESTDIR=$SRVD >>& $outfile)
	breaksw

	case third/supported/afs
	(echo In $package >> & $outfile )
if ( $installonly == "0" ) then
	(cd /build/$package ; xmkmf . >>& $outfile)
	(cd /build/$package ; make >>& $outfile)
	cp -rp /build/$package/$AFS/dest/lib /usr/transarc
	cp -rp /build/$package/$AFS/dest/include /usr/transarc
endif # installonly
	(cd /build/$package; make install DESTDIR=$SRVD >>& $outfile)
	breaksw	

	case third/supported/X11R5
	(echo In $package >> & $outfile)
if ( $installonly == "0" ) then
	if ( $machine == "sun4" ) then
	(cd /build/$package/mit ; make -f Makefile.ini -k World 'BOOTSTRAPCFLAGS="-DSVR4"' >>& $outfile)
	else
	(cd /build/$package/mit ; make -k World >>& $outfile)
	endif
endif # installonly
	(cd /build/$package/mit ;  make -k install SUBDIRS="util clients demos" DESTDIR=$SRVD >>& $outfile)
	(cd /build/$package/mit/include/bitmaps; make install INCDIR=$SRVD/usr/athena/lib/X11 >>& $outfile)
	rehash
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
if ($machine != "decmips") then
	((cd /build/$package;make depend) >>& $outfile)
	if ($status == 1) then
		echo "We bombed in $package" >>& $outfile
		exit -1
	endif
endif
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

# Mark? Are you piping to true??
# Changed to || (true)
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
		switch (`cat /build/$package/.rule`)

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
cp -p $outfile "/build/washlog.`date '+%y.%m.%d.%H'`"
