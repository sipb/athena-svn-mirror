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

set third="third/supported/afs third/supported/X11R5 third/supported/X11R4 third/supported/motif third/supported/tcsh6 third/supported/emacs-18.59 third/unsupported/top third/unsupported/sysinfo  third/unsupported/perl"

set libs2=" athena/lib/kerberos2 athena/lib/acl athena/lib/gdb athena/lib/gdss athena/lib/zephyr.p4 athena/lib/moira.dev athena/lib/neos"

set etcs="athena/etc/track athena/etc/rvd athena/etc/nfsc athena/etc/newsyslog athena/etc/cleanup athena/etc/synctree athena/etc/ftpd athena/etc/inetd athena/etc/netconfig athena/etc/gettime athena/etc/traceroute athena/etc/xdm athena/etc/scripts athena/etc/timed athena/etc/snmpd"

set bins=" athena/bin/session athena/bin/olc.dev athena/bin/finger athena/bin/ispell athena/bin/Ansi athena/bin/sendbug athena/bin/just athena/bin/rep athena/bin/cxref athena/bin/tarmail athena/bin/access athena/bin/mon athena/bin/olh athena/bin/dent athena/bin/xquota athena/bin/attach athena/bin/dash athena/bin/xmore athena/bin/mkserv athena/bin/cal athena/bin/xps athena/bin/scripts athena/bin/afs-nfs athena/bin/xdsc athena/bin/rkinit.76 athena/bin/xprint athena/bin/xversion athena/bin/viewscribe athena/bin/kerberometer athena/bin/discuss athena/bin/from athena/bin/delete athena/bin/getcluster athena/bin/gms athena/bin/hostinfo athena/bin/machtype athena/bin/login athena/bin/ls athena/bin/tcsh athena/bin/write athena/bin/tar athena/bin/tinkerbell"

#I removed athena/bin/inittty as there was no Imakefile there
 
set outfile="/usr/tmp/washlog.`date '+%y.%m.%d.%H'`"
set SRVD="/srvd"
set X="X11R4"
set MOTIF="motif"
echo starting `date` > $outfile
echo on a $machine >> $outfile

#start by building and installing imake in /build/bin


#I need to split kerberos up into phase 1 and phase 2 
# need to add in motif. Once that is done I can proceed onto the bin directory

if ($machine == "sun4") then
foreach package ( setup $machine $libs1 $third $libs2 $etcs $bins)

else if ($machine == "rsaix" ) then

foreach package ( setup $libs1 $third $libs2 $etcs $bins )
else

foreach package ( $machine  setup athena/lib/syslog $libs1 $third $libs2 $etcs $bins )

endif
switch ($package)
	case setup
	(echo in setup >>& $outfile)
	cd /build/support/imake
		((make -f Makefile.ini clean >>& $outfile) && \
			(make -f Makefile.ini >>& $outfile ) && \
			(cp imake /build/bin >>& $outfile) && \
			(chmod 755 /build/bin/imake >>& $outfile))
		if ($status == 1 ) then
			echo "We bombed in imake" >>& $outfile
		endif
	if ($machine == "sun4" ) then
		(cd /build/sun4/include; make install)
	endif

	cd /build/support/install
	((echo "In install" >>& $outfile) &&\
	(xmkmf . >>& $outfile) &&\
	(make clean >>& $outfile) &&\
	(make >>& $outfile))
	if ($status == 1 ) then
	        echo "We bombed in install" >>& $outfile
	endif
	breaksw

	case decmips
#	This is scary.
#	Unmount /srvd remove the directory make a link then at the end 
# reverse the process
	umount /srvd >>& $outfile
	rmdir /srvd >>& $outfile
	ln -s /afs/rel-eng/system/pmax_ul4/srvd /srvd >>& $outfile
	mkdir /srvd.tmp >>& $outfile
	mount /dev/rz3d /srvd.tmp >>& $outfile
	(echo In $package >>& $outfile)
	( cd /build/$package ; make Makefiles >>& $outfile ) 
	(echo $package : make clean >>& $outfile )
	( cd /build/$package ; make clean >>& $outfile )
	(echo $package : make depend >>& $outfile )
	( cd /build/$package ; make depend >>& $outfile ) 
	(echo $package : make all >>& $outfile )
	( cd /build/$package ; make all >>& $outfile ) 
	(echo $package : make install >>& $outfile )
	( cd /build/$package ; make install DESTDIR=/srvd.tmp >>& $outfile )
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

	case third/unsupported/perl
	case third/unsupported/perl-4.036
	(echo In $package >>& $outfile)
	set PERLFILES = "  ./x2p/s2p.SH ./x2p/find2perl.SH ./x2p/Makefile.SH ./x2p/cflags.SH ./Makefile.SH ./config_h.SH ./c2ph.SH ./h2ph.SH ./makedepend.SH ./cflags.SH ./makedir.SH"
	( cd /build/$package ; cp config.sh.$machine config.sh)
	( cd /build/$package  ; cp cppstdin.sh.$machine cppstdin )
	 cd /build/$package ; \
		  foreach p ($PERLFILES)
		   sh $p >>& $outfile
		  end 
	( cd /build/$package ;  make clean >>& $outfile)
	( cd /build/$package ;  make depend >>& $outfile)
	( cd /build/$package ;  make >>& $outfile)
	( cd /build/$package ;  make test >>& $outfile)
	breaksw

	case third/unsupported/sysinfo
	(echo In $package >>& $outfile)
	( cd /build/$package ; make clean >>& $outfile )
	( cd /build/$package ; make >>& $outfile )
	(cd /build/$package ; make install DESTDIR=/srvd >>& $outfile )
	breaksw

	case third/unsupported/top
	(echo In $package >>& $outfile)
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

	if ($machine != "rsaix" ) then
	( cd /build/$package ; make clean >>& $outfile )
	( cd /build/$package ; make >>& $outfile )
        ( cd /build/$package ; make install DESTDIR=/srvd >>& $outfile )
	endif
        breaksw

	case athena/bin/ls
	(echo In $package >>& $outfile)
	(( cd /build/$package ; make clean  >>& $outfile ) && \
	( cd /build/$package ; make -f Makefile.$machine >>& $outfile) && \
	( cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile))
        if ( $status == 1 ) then
                echo "We bombed in $package" >> & $outfile
                exit -1
        endif

	breaksw

	case third/supported/tcsh6
	(echo In $package >>& $outfile)
	( cd /build/$package ; /usr/athena/bin/xmkmf >>& $outfile )
	( cd /build/$package ; make clean  >>& $outfile )
	( cd /build/$package ; make config.h >>& $outfile)
	( cd /build/$package ; make >>& $outfile)
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
		((cd /build/$package ; imake -DTOPDIR=. -I./util/imake.includes >>& $outfile ) && \
		(cd /build/$package ; make Makefiles >>& $outfile) && \
		(cd /build/$package ; make clean >>& $outfile) && \
		(cd /build/$package ; make depend SUBDIRS="util include lib admin " >> & $outfile) &&\
		(cd /build/$package ;make  SUBDIRS="include lib" >>& $outfile) && \
		(cd /build/$package ; make install SUBDIRS="include lib" DESTDIR=$SRVD >>& $outfile) )
		if ($status == 1) then
			echo "We bombed at $package" >>& $outfile
			exit -1
		endif
		breaksw

	case athena/lib/kerberos2
# There is one thing to note about kerberos
# kerberos needs afs and afs needs kerberos. this has to be 
# addressed at some point
		set package="athena/lib/kerberos"
		(echo In $package >>& $outfile)
		((cd /build/$package ;make depend SUBDIRS="appl kuser server slave kadmin man" >>& $outfile) &&\
		(cd /build/$package ;make all >>& $outfile) && \
		(cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile) )
		if ($status == 1) then
			echo "We bombed at $package" >>& $outfile
			exit -1
		endif
		breaksw
	case third/supported/X11R4
		(echo In $package >> & $outfile) 
		(cd /build/$package ; make -k World >>& $outfile)  
		(cd /build/$package ; make -k install SUBDIRS="include lib extensions" DESTDIR=$SRVD >>& $outfile)
		breaksw
	case third/supported/motif
		(echo In $package >> & $outfile)
		(cd /build/$package ; imake -DTOPDIR=. -I./config/ >>& $outfile)
		(cd /build/$package ; make -k World >>& $outfile)
		(cd /build/$package ; make -k install DESTDIR=$SRVD >>& $outfile)
		breaksw
	case third/supported/afs
		(echo In $package >> & $outfile )
			(cd /build/$package ; xmkmf . >>& $outfile)
			(cd /build/$package ; make >>& $outfile)
			cp -rp /build/$package/$AFS/dest/lib /usr/transarc
			cp -rp /build/$package/$AFS/dest/include /usr/transarc
	breaksw	
	case third/supported/X11R5
		(echo In $package >> & $outfile)
		if ( $machine == "sun4" ) then
		(cd /build/$package/mit ; make -f Makefile.ini -k World 'BOOTSTRAPCFLAGS="-DSVR4"' >>& $outfile)
		else
		(cd /build/$package/mit ; make -k World >>& $outfile)
		endif
		(cd /build/$package/mit ;  make -k install SUBDIRS="clients demos" >>& $outfile)
		breaksw
	case athena/bin/ls
		(echo In $package >> & $outfile)
		( cd /build/$package ; make clean >>& $outfile )
		( cd /build/$package ; make all >>& $outfile )
		( cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile )
	breaksw

	case athena/bin/dash
		(echo In $package >> & $outfile)
		((cd /build/$package ; /usr/athena/bin/xmkmf >>& $outfile) &&\
		(cd /build/$package ; make Makefiles >>& $outfile) && \
		(cd /build/$package ; make clean >>& $outfile) && \
		(cd /build/$package ; make depend >>& $outfile) && \
		(cd /build/$package ; make all >>& $outfile ) && \
		(cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile))
		if ($status == 1) then
			echo "We bombed at $package">>& $outfile
			exit -1
		endif
		breaksw		
	case athena/lib/zephyr.p4
		(echo In $package >>& $outfile)
                ((cd /build/$package ; /usr/athena/bin/xmkmf $cwd  >>& $outfile ) && \
                (cd /build/$package ;make World >>& $outfile) && \
                (cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile))
                if ($status == 1) then
                        echo "We bombed at $package" >>& $outfile
                        exit -1
                endif
                breaksw
	case athena/bin/olc.dev
		(echo In $package >>& $outfile)
                ((cd /build/$package ; imake -DTOPDIR=. -I./config >>& $outfile ) && \
		(cd /build/$package ; make Makefiles >>& $outfile) && \
		(cd /build/$package ; make clean >>& $outfile) && \
		(cd /build/$package ; make world >>& $outfile) && \
                (cd /build/$package ; make install DESTDIR=$SRVD >>& $outfile))
                if ($status == 1) then
                        echo "We bombed at $package" >>& $outfile
                        exit -1
                endif
		breaksw
	case athena/bin/olh
		(echo In $package >>& $outfile)
                ((cd /build/$package ; imake -DTOPDIR=. -I./config >>& $outfile ) && \
		(cd /build/$package ; make Makefiles >>& $outfile) && \
		(cd /build/$package ; make clean >>& $outfile) && \
                (cd /build/$package ;make world >>& $outfile))
                if ($status == 1) then
                        echo "We bombed at $package" >>& $outfile
                        exit -1
                endif
		breaksw
	case third/supported/emacs-18.59
		((cd /build/$package ; xmkmf >>& $outfile) &&\
		(cd /build/$package/etc ; xmkmf >> $outfile) &&\
		(cd /build/$package/etc ; make clean >>& $outfile) &&\
		(cd /build/$package/etc ; make all >>& $outfile) &&\
		(cd /build/$package/etc ; make install DESTDIR=$SRVD >>& $outfile))
		if ($status == 1) then
			echo "We bombed at $package" >>& $outfile
			exit -1
		endif
		breaksw
	case athena/lib/moira.dev
		(echo In $package >>& $outfile)
		((cd /build/$package ; imake -DTOPDIR=. -I./util/imake.includes >>& $outfile) &&\
		(cd /build/$package; make Makefiles >>& $outfile) &&\
		(cd /build/$package; make clean >>& $outfile) &&\
		((cd /build/$package; make depend >>& $outfile) | (true)) &&\
		(cd /build/$package; make all >>& $outfile) &&\
		(cd /build/$package; make install SUBDIRS="lib gdb" DESTDIR=$SRVD))
		if ($status == 1) then
			echo "We bombed at $package" >>& $outfile
			exit -1
		endif
		breaksw

	case athena/bin/tinkerbell
	if ($machine == "sun4") then
		((echo In $package >>& $outfile ) && \
		(cd /build/$package;xmkmf . >>& $outfile ) && \
		(cd /build/$package;make clean >>& $outfile ) && \
		(cd /build/$package;make depend >>& $outfile) && \
		(cd /build/$package;make all >>& $outfile ) && \
		(cd /build/$package;make install DESTDIR=$SRVD >>& $outfile ))
		if ($status == 1 ) then
			echo "We bombed at $package"  >>& $outfile
			exit -1
		endif
	endif
	breaksw

	default:
		switch (`cat /build/$package/.rule`)

		case complex
 ((echo In $package : make Makefile  >>& $outfile ) && \
 ((cd /build/$package;xmkmf . ) >>& $outfile ) && \
 (echo In $package : make Makefiles>>& $outfile ) && \
 ((cd /build/$package;make Makefiles) >>& $outfile )  && \
 (echo In $package : make clean >>& $outfile ) && \
 ((cd /build/$package;make clean) >>& $outfile ) && \
 (echo In $package : make depend >>& $outfile) && \
 ((cd /build/$package;make depend) >>& $outfile) && \
 (echo In $package : make all >>& $outfile ) && \
 ((cd /build/$package;make all) >> & $outfile ) && \
 (echo In $package : make install >>& $outfile ) && \
 ((cd /build/$package;make install DESTDIR=$SRVD) >> & $outfile ))  
   if ($status == 1 ) then
	echo "We bombed at $package"  >>& $outfile
	exit -1
   endif
		breaksw
		case simple
		default:
 ((echo In $package : make Makefile  >>& $outfile ) && \
 ((cd /build/$package;xmkmf . ) >>& $outfile ) && \
 (echo In $package : make clean >>& $outfile ) && \
 ((cd /build/$package;make clean) >>& $outfile ) && \
 (echo In $package : make depend >>& $outfile) && \
 ((cd /build/$package;make depend) >>& $outfile) && \
 (echo In $package : make all >>& $outfile ) && \
 ((cd /build/$package;make all) >> & $outfile ) && \
 (echo In $package : make install >>& $outfile ) && \
 ((cd /build/$package;make install DESTDIR=$SRVD) >> & $outfile ))
   if ($status == 1 ) then
        echo "We bombed at $package"  >>& $outfile
        exit -1
   endif

		breaksw
		endsw
	breaksw
endsw
end
echo ending `date` >>& $outfile
