#
# Copyright (c) 1992-1998 Michael A. Cooper.
# This software may be freely used and distributed provided it is not sold 
# for profit or used for commercial gain and the author is credited 
# appropriately.
#
# $Revision: 1.1.1.3 $
#
# Sysinfo Makefile
#
# To compile, just run "make".  This should automatically figure out
# your OS type.
#

#
# The ${PROG} variable defines the name the sysinfo software is installed
# as.  If you want to install sysinfo to use a name other than "sysinfo"
# you should just need to change this value.
#
PROG 		= sysinfo

#
# The ${PREFIX} variable determines the top level directory that
# things are installed under.
#
PREFIX		= /var/local

#
# The bin directory that user's will run the program from.
#
# A good place is something like /usr/local/bin.
#
BIN 		= ${PREFIX}/bin

#
# Directory containing sysinfo.cf configuration files
#
#CONFDIR		= /usr/local/lib/${PROG}
CONFDIR		= ${PREFIX}/${PROG}/conf

#
# The LIBDIR directory is where all the sysinfo binary executables
# will be installed.
#
LIBDIR		= ${PREFIX}/${PROG}/lib

#
# Man sections and directories
#
ManDir 		= ${PREFIX}/man
ManSect1	= 1
ManSect5	= 5
ManDir1		= ${ManDir}/man${ManSect1}
ManDir5		= ${ManDir}/man${ManSect5}

#
# The shell that commands from this Makefile will run from.
# This _MUST_ be a bourne shell compatible shell.
#
SHELL		= /bin/sh

#
# Installation options
#
# Sysinfo needs to be able to read your unix image (/vmunix usually),
# /dev/kmem, and device files in /dev for things like disk drives and tapes.
# Under SunOS, /dev/kmem is owned by group "kmem", but the disk files
# (e.g. /dev/rsd0a) are owned by group "operator".  Therefor, you usually
# can't make setgid to a group, but need it setuid root.
#
# You should set INSTALLMODE and INSTALLOWN appropriately to allow sysinfo 
# to read everything it needs to.  Try running "sysinfo -debug" to find out 
# the special files it reads.
#
INSTALLMODE	= -m 4555
INSTALLOWN 	= -o root -g bin
INSTALLOPTS 	= $(INSTALLMODE) $(INSTALLOWN)

#
# Name of the program that installs files.
# This program should support the BSD-style install(1) options.
# The install.sh script from the sysinfo distribution should work fine 
# on most systems.
#
INSTALLPROG	= sh ./install.sh

###########################################################################
#
# 	END OF NORMAL SITE CONFIGURATION
#
###########################################################################

#
# The make program to use
#
MAKE		= make

#
# The name of the kvm library to use.
# For SunOS this should be "-lkvm".  If you don't have one, add
# "kvmlib.o" to MISSINGFILES below and leave LIBKVM blank.
#
LIBKVM		= #-lkvm

#
# SunOS 4.0, 4.0.1, and 4.0.3 requires using the System V cc and libc
# in order to get uname().  Enable the two lines below if your OS is one
# of these.
#
# Do NOT set CC to /usr/ucb/cc for SunOS 5.x.  This enables BSD 
# compatibility mode in SunC which breaks sysinfo.
#
#CC		= gcc		# Use GCC
#STATICFLAG	= -static	# Use GCC
LIBS 		= $(OSLIBS)

#
# Defines
#
DEFINES		= $(OSDEFINES) -DCONFIG_DIR=\"$(CONFDIR)\"

#
# Optimization or debugging flag
#
OPT		= -g

#
# Include directories
#
INCLUDES	= -I.

CFLAGS 		= ${OPT} ${INCLUDES} ${DEFINES} $(OSCFLAGS)
LDFLAGS		= ${OPT} ${OSLDFLAGS}

NetIf-Files	= netif.o netif-info.o
Env-Files	= getenv.o putenv.o setenv.o

#
# Distribution Files
#
BUILDINFO	= build/buildinfo
VERCMD 		= `build/vername ./$(PROG) -version -level terse`
PLATFORMCMD	= `${BUILDINFO} -platform`
MANFILES	= sysinfo.man sysinfo.cf.man
MAKEFILES	= Makefile
BINDistFiles	= README INSTALL.html INSTALL.txt \
			RELEASE_NOTES.html RELEASE_NOTES.txt \
			COPYRIGHT CHANGES \
			$(MANFILES) metasysinfo.sh  install.sh \
			build/buildinfo build/mkversion \
			build/vername build/mkdirhier \
			config/*.cf config/*.ddb $(MAKEFILES)

#
# OS Specific Files
#
AIX-OS-Files	= os-aix.o
IRIX-OS-Files	= os-irix.o ${NetIf-Files} myscsi.o
SunOS-OS-Files	= os-sunos.o sunos-fb.o sunos-kbd.o sunos-kdt.o sunos-obp.o \
			${NetIf-Files}
SunOS4-OS-Files	= os-sunos4.o ${SunOS-OS-Files} myscsi.o
SunOS5-OS-Files	= os-sunos5.o dlpi.o sunos-ddb.o ${SunOS-OS-Files} myscsi.o \
			dospart.o edid.o pci.o
HPUX-OS-Files	= os-hpux.o ${NetIf-Files} myscsi.o
SPPUX-OS-Files	= os-sppux.o
BSD-Files	= bsd.o bsd-sysctl.o dospart.o ${NetIf-Files} myscsi.o
BSDOS-OS-Files	= os-bsdos.o ${BSD-Files}
FreeBSD-OS-Files= os-freebsd.o pci.o ${BSD-Files}
Linux-OS-Files	= os-linux.c

#
# Select the OS specific files to compile
#
OSFILES		=

#
# Missing files
#
# List of files of things your system may not have.
#
MISSINGFILES	=

OBJS 		= sysinfo.o devices.o memory.o run.o run-data.o mcl.o kvm.o \
			mcregex.o options.o virtmem.o getosver.o getosname.o \
			getosdist.o getcpu.o \
			gethost.o getmodel.o psi.o getmisc.o boottime.o \
			kernel.o general.o class.o var.o report.o misc.o msg.o\
			cf.o define.o devdefine.o strutil.o namelist.o \
			classtype.o type.o \
			getarch.o getkernver.o getman.o macinfo.o sysconf.o \
			${MISSINGFILES} ${OSFILES}

#
# Macro to set OS info variables
#
SetOS = \
	set `${BUILDINFO}`; \
	osname=`${BUILDINFO} -osname`; \
	osver=`${BUILDINFO} -osvernodot`; \
	ostype=`${BUILDINFO} -osnamemver`; \
	osmver=`${BUILDINFO} -osmver`; \
	kisa=`${BUILDINFO} -kisa`; \
	case "$${ostype}" in \
	hpux*)		os=hpux$${osmver};; \
	sunos*)		os=sunos$${osmver};; \
	*)		os=$${osname};; \
	esac
OSmakeDefs 	= OSname=$$osname OSver=$$osver OSmver=$$osmver \
			OStype=$$ostype KISA=$$kisa
MKPARAMS	= PREFIX=$(PREFIX)

#
# Default
#
default:
	@if [ -f sysinfo -a ! -f sysinfo.c ]; then \
		echo 'This is a binary distribution.  There is nothing to compile.'; \
	else \
		$(SetOS); \
		$(MAKE) $(MFLAGS) \
			CONFDIR=${CONFDIR} BIN=${BIN} CC=${CC} \
			LIBDIR=${LIBDIR}\
			$(OSmakeDefs) $${ostype}; \
	fi

all: ${PROG}

${PROG}: ${OBJS} buildinfo.o
	rm -f $@
	${CC} ${LDFLAGS} -o $@ ${OBJS} buildinfo.o ${LIBS} ${LIBKVM}

buildinfo.c: $(OBJS) build/mkversion
	@build/mkversion -file $@
#	@build/mkversion -host magnicomp.com -user mcooper -file $@

bindist: $(PROG) $(BINDistFiles)
	sysver=$(VERCMD); \
	vername=`${BUILDINFO} -distname -disttype bin -prodver $$sysver`; \
	filelist=filelist.tmp; \
	rm -f $$filelist; \
	touch $$filelist; \
	chmod 664 $$filelist; \
	for f in $(PROG) $(BINDistFiles); do \
		echo $$vername/$${f} >> $$filelist; \
	done; \
	rm -f $$vername; \
	ln -s . $$vername; \
	strip $$vername/$(PROG); \
	tar cf - `cat $$filelist` | gzip --best > \
		$$vername.tar.gz; \
	rm -f $$vername $$filelist; \
	echo DIST File is $$vername.tar.gz

install: $(LIBDIR) $(BIN) metasysinfo
	@$(MAKE) $(MFLAGS) $(MKPARAMS) install.cf CONFDIR=${CONFDIR}
	@if [ ! -z "$(LIBDIR)" ]; then \
		RTPLATFORM=`${BUILDINFO} -rtplatform`; \
		build/mkdirhier $(LIBDIR)/$${RTPLATFORM}; \
		echo "$(INSTALLPROG) -c -m 555 $(INSTALLOWN) metasysinfo $(BIN)/$(PROG)"; \
		$(INSTALLPROG) -c -m 555 $(INSTALLOWN) metasysinfo $(BIN)/$(PROG); \
		echo "$(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} $(LIBDIR)/$${RTPLATFORM}/${PROG}"; \
		$(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} $(LIBDIR)/$${RTPLATFORM}/${PROG}; \
		strip $(LIBDIR)/$${RTPLATFORM}/${PROG}; \
	else \
		echo $(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} ${BIN}/${PROG}; \
		$(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} ${BIN}/${PROG}; \
		strip ${BIN}/${PROG}; \
	fi

metasysinfo: metasysinfo.sh
	rm -f $@
	sed -e "s;#LIBDIR#;$(LIBDIR);" < metasysinfo.sh > $@
	chmod 555 $@

install.man: $(MANFILES) $(ManDir1) $(ManDir5)
	$(INSTALLPROG) -c -m 444 -o bin -g bin sysinfo.man \
		${ManDir1}/${PROG}.${ManSect1}
	$(INSTALLPROG) -c -m 444 -o bin -g bin sysinfo.cf.man \
		${ManDir5}/${PROG}.cf.${ManSect5}

$(BIN):
	build/mkdirhier ${BIN}

$(LIBDIR):
	build/mkdirhier ${LIBDIR}

$(CONFDIR):
	build/mkdirhier ${CONFDIR}

$(ManDir1):
	build/mkdirhier ${ManDir1}

$(ManDir5):
	build/mkdirhier ${ManDir5}

install.cf: $(CONFDIR)
	@for file in config/*.cf config/*.ddb ; do \
		cf=`echo $$file | sed -e 's;.*/;;'`; \
		echo $(INSTALLPROG) -c -m 444 $(INSTALLOWN) $$file $(CONFDIR)/$$cf; \
		$(INSTALLPROG) -c -m 444 $(INSTALLOWN) $$file $(CONFDIR)/$$cf; \
	done

clean:
	rm -f *.o ${PROG} *~ *% \#* core a.out metasysinfo \
		buildinfo.c sysinfo.tar sysinfo.tar.Z sysinfo.tar.gz

${OBJS}: os.h defs.h mconfig.h declare.h

test: default
	@echo "Running sysinfo as a test . . ."
	@echo "NOTE: You must be root to run this test successfully."
	./$(PROG) -cfdir ./config -level all

#
# OS targets
#

#
# SunOS 4.x (Solaris 1.x)
#
# NOTE: We compile statically (-Bstatic) to avoid LD_LIBRARY_PATH
#	security problems.  Overriding this will open a major security
#	whole if sysinfo is run set{u,g}id.
#
sunos4 solaris1:
	@echo "Making ${PROG} for SunOS 4.x ... "; \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM=-lkvm \
		MISSINGFILES="strcasecmp.o strdup.o strerror.o snprintf.o" \
		OSDEFINES="-Dsunos -DOSMVER=4 -D`/bin/arch -k`" \
		OSFILES="${SunOS4-OS-Files}" \
		OSLDFLAGS="${STATICFLAG}" \
		OSLIBS="-lresolv" \
		PROG=${PROG} \
		all

#
# SunOS 5.x (Solaris 2.x)
#
sunos5 solaris2:
	@echo "Making ${PROG} for SunOS 5.x ... "; \
	echo "Make sure that \`cc' is not \`/usr/ucb/cc' or sysinfo may not work."; \
	if [ $$OSver -lt 54 ]; then \
		OtherLibs="-lgen"; \
	fi; \
	if [ $$OSver -lt 56 ]; then \
		OtherMissing="snprintf.o"; \
	fi; \
	if [ $$OSver -ge 56 ]; then \
		OtherLibs="-lvolmgt"; \
	fi; \
	if [ $$OSver -ge 57 -a "$$KISA" = "sparcv9" ]; then \
		xarch="-xarch=v9"; \
	fi; \
	$(MAKE)	$(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM=-lkvm \
		MISSINGFILES="setreuid.o $$OtherMissing" \
		OSCFLAGS="$$xarch" \
		OSDEFINES="-Dsunos -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSFILES="${SunOS5-OS-Files}" \
		OSLDFLAGS="-i -R/usr/lib $$xarch" \
		OSLIBS="-lsocket -lnsl -lelf -ladm -lresolv -lgen $$OtherLibs" \
		PROG=${PROG} \
		all

#
# AIX 3.x for RS/6000
# AIX 4.x for RS/6000
#
aix3 aix4:
	@echo "Making ${PROG} for AIX ... "; \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="strdup.o kvmlib.o" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver}" \
		OSFILES="${AIX-OS-Files}" \
		OSLIBS="-lodm -lcfg" \
		PROG=${PROG} \
		all

#
# HP-UX 9.x
#
hpux9:
	@echo "Making ${PROG} for HP-UX 9.x ... "; \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o" \
		OPT="" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver}" \
		OSFILES="${HPUX-OS-Files}" \
		OSLIBS="" \
		PROG=${PROG} \
		all

#
# HP-UX 10 and later
#
hpux10 hpux11:
	@echo "Making ${PROG} for HP-UX ${OSmver} ... "; \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o" \
		OPT="" \
		OSCFLAGS="-Ae" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver}" \
		OSFILES="${HPUX-OS-Files}" \
		OSLIBS="" \
		PROG=${PROG} \
		all

#
# Convex SPP-UX
#
sppux3:
	@echo "Making ${PROG} for SPP-UX ... "; \
	echo "Ignore errors regarding /usr/include/sys/wait.h"; \
	$(MAKE) $(MFLAGS) \
		CC=/usr/convex/bin/cc \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o" \
		OPT="-cxdb" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver}" \
		OSFILES="${SPPUX-OS-Files}" \
		OSLIBS="/lib/cnx_syscall.a" \
		OSCFLAGS="-or none" \
		PROG=${PROG} \
		all

#
# IRIX 5.x (Does this still work?)
#
irix5:	
	@echo "Making ${PROG} for IRIX 5.x ... "; \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o" \
		OPT="-g" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSCFLAGS="-woff 1174 -woff 1552 -woff 1204 -woff 1009" \
		OSFILES="${IRIX-OS-Files}" \
		OSLIBS="-lmld -lelf" \
		PROG=${PROG} \
		all

#
# IRIX 6.x (32-bit Kernel)
#
# Add "-UHAVE_GRAPHICS_HDRS" to OSDEFINES below if you don't have
# any of the IRIX graphics .h files installed in /usr/include.
#
irix6:	
	@echo "Making ${PROG} for IRIX (32-bit) 6.x ... "; \
	if [ $$OSver -lt 65 ]; then \
		OtherMissing="snprintf.o"; \
	fi; \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o $$OtherMissing" \
		OPT="-g" \
		OSLDFLAGS="" \
		OSCFLAGS="" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSFILES="${IRIX-OS-Files}" \
		OSLIBS="-lelf" \
		PROG=${PROG} \
		all

#
# IRIX 6.x (64-bit Kernel)
#
# Add "-UHAVE_GRAPHICS_HDRS" to OSDEFINES below if you don't have
# any of the IRIX graphics .h files installed in /usr/include.
#
irix646:
	@echo "Making ${PROG} for IRIX (64-bit) 6.x ... "; \
	if [ $$OSver -lt 65 ]; then \
		OtherMissing="snprintf.o"; \
	fi; \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o $$OtherMissing" \
		OPT="-g" \
		OSLDFLAGS="" \
		OSCFLAGS="" \
		OSDEFINES="-Dirix -D${OSname} -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSFILES="${IRIX-OS-Files}" \
		OSLIBS="-lelf" \
		PROG=${PROG} \
		all

#
# BSD/OS
#
# Known to work on BSD/OS 3.x and later.
#
bsdos3 bsdos4:
	@echo "Making ${PROG} for BSD/OS ... "; \
	$(MAKE) $(MFLAGS) \
		CC="gcc" \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="" \
		OPT="" \
		OSLDFLAGS="" \
		OSCFLAGS="" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSFILES="${BSDOS-OS-Files}" \
		OSLIBS="-lkvm" \
		PROG=${PROG} \
		all

#
# FreeBSD 2.x
#
# Known to work on FreeBSD 2.2.6 and later.
#
freebsd2:
	@echo "Making ${PROG} for FreeBSD ... "; \
	$(MAKE) $(MFLAGS) \
		CC="gcc" \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="" \
		OPT="" \
		OSLDFLAGS="" \
		OSCFLAGS="-I/sys" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSFILES="${FreeBSD-OS-Files}" \
		OSLIBS="-lkvm" \
		PROG=${PROG} \
		all

#
# Linux
#
linux linux2:
	@echo "Making ${PROG} for Linux ... "; \
	$(MAKE) $(MFLAGS) \
		CC="cc" \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="" \
		OPT="" \
		OSLDFLAGS="" \
		OSCFLAGS="" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSFILES="${Linux-OS-Files}" \
		OSLIBS="" \
		PROG=${PROG} \
		all
