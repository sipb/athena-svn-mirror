#
# Copyright (c) 1992-1994 Michael A. Cooper.
# This software may be freely distributed provided it is not sold for 
# profit and the author is credited appropriately.
#
# $Id: Makefile,v 1.2 1996-10-10 21:03:55 ghudson Exp $
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
PREFIX		= /usr/athena

#
# The shell that commands from this Makefile will run from.
# This _MUST_ be a bourne shell compatible shell.
#
SHELL		= /bin/sh

#
# Location of your Kernel specific directory.
# This is usually only for SunOS 4.x.  If you don't have
# one, set this to nothing.
#
#KVMDIR		= /usr/kvm
KVMDIR		=

#
# The bin directory that user's will run the program from.
# For systems with a $(KVMDIR) (above), a symlink will be put 
# in $(BIN) pointing to $(KVMDIR)/${PROG}.  On systems without 
# a $(KVMDIR), the program will be copied to $(BIN).
#
# A good place is something like /usr/local/bin.
#
BIN 		= ${PREFIX}/bin

#
# Man sections and directories
#
ManDir 		= ${PREFIX}/man
ManSect1	= 1
ManSect5	= 5
ManDir1		= ${ManDir}/man${ManSect1}
ManDir5		= ${ManDir}/man${ManSect5}

#
# Directory containing sysinfo.cf configuration files
#
#CONFDIR		= /usr/local/lib/${PROG}
CONFDIR		= ${PREFIX}/lib/${PROG}

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
#CC		= gcc
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

NetIf-Files	= netif.o info-netif.o
Env-Files	= getenv.o putenv.o setenv.o

#
# OS Specific Files
#
AIX-OS-Files	= os-aix.o
IRIX-OS-Files	= os-irix.o ${NetIf-Files}
SunOS-OS-Files	= os-sunos.o kbd-sun.o kdt-sunos.o obp.o ${NetIf-Files}
SunOS4-OS-Files	= os-sunos4.o ${SunOS-OS-Files}
SunOS5-OS-Files	= os-sunos5.o dlpi.o ${SunOS-OS-Files}
HPUX-OS-Files	= os-hpux.o
SPPUX-OS-Files	= os-sppux.o

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

OBJS 		= sysinfo.o devices.o memory.o run.o run-data.o kvm.o \
			options.o virtmem.o getosver.o getosname.o getcpu.o \
			gethost.o getmodel.o psi.o getmisc.o boottime.o \
			kernel.o general.o class.o var.o report.o misc.o \
			cf.o define.o devdefine.o strutil.o namelist.o type.o \
			getarch.o getkernver.o getman.o macinfo.o sysconf.o \
			${MISSINGFILES} ${OSFILES}

#
# Macro to set OS info variables
#
SetOS = \
	set `./ostype`; \
	osname=$$1; \
	osver=$$3; \
	ostype=$$4; \
	osmver=$$5; \
	case "$${ostype}" in \
	hpux*)		os=hpux$${osmver};; \
	sunos*)		os=sunos$${osmver};; \
	*)		os=$${osname};; \
	esac
OSmakeDefs 	= OSname=$$osname OSver=$$osver OSmver=$$osmver OStype=$$ostype
MKPARAMS	= PREFIX=$(PREFIX)

#
# Default
#
default:
	@$(SetOS); \
	$(MAKE) $(MFLAGS) \
		CONFDIR=${CONFDIR} BIN=${BIN} KVMDIR=${KVMDIR}\
		$(OSmakeDefs) $${ostype}

all: ${PROG}

${PROG}: ${OBJS} versinfo.o
	rm -f $@
	${CC} ${LDFLAGS} -o $@ ${OBJS} versinfo.o ${LIBS} ${LIBKVM}

versinfo.c: $(OBJS) mkversion
	@./mkversion $@

install: default
	@make $(MFLAGS) $(MKPARAMS) install.cf CONFDIR=${CONFDIR}
	@if [ ! -z "$(KVMDIR)" ]; then \
		echo "$(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} $(KVMDIR)/${PROG}"; \
		$(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} $(KVMDIR)/${PROG}; \
		echo "cd ${BIN} && rm -f ${PROG} && ln -s $(KVMDIR)/${PROG} ${PROG}"; \
		(cd ${BIN} && rm -f ${PROG} && ln -s $(KVMDIR)/${PROG} ${PROG}); \
	else \
		echo $(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} ${DESTDIR}${BIN}/${PROG}; \
		$(INSTALLPROG) -c ${INSTALLOPTS} ${PROG} \
			${DESTDIR}${BIN}/${PROG}; \
	fi

metasysinfo: metasysinfo.sh
	rm -f $@
	sed -e "s;#InstallDir#;$(BIN);" < metasysinfo.sh > $@
	chmod 555 $@

#
# See the file "metasysinfo" for more information on "installmeta".
#
installmeta: default metasysinfo
	@make $(MFLAGS) $(MKPARAMS) install.cf
	$(INSTALLPROG) -c -m 555 $(INSTALLOWN) metasysinfo $(BIN)/$(PROG)
	@$(INSTALLPROG) -c $(INSTALLOPTS) $(PROG) \
			$(BIN)/$(PROG)-`uname -m`-`uname -r`; \
		echo $(INSTALLPROG) -c $(INSTALLOPTS) $(PROG) \
			$(BIN)/$(PROG)-`uname -m`-`uname -r`

install.man: sysinfo.man sysinfo.cf.man
	$(INSTALLPROG) -c -m 444 -o bin -g bin sysinfo.man \
		${DESTDIR}${ManDir1}/${PROG}.${ManSect1}
	$(INSTALLPROG) -c -m 444 -o bin -g bin sysinfo.cf.man \
		${DESTDIR}${ManDir5}/${PROG}.cf.${ManSect5}

$(CONFDIR):
	mkdir $(CONFDIR)
	chmod 755 $(CONFDIR)

install.cf: $(CONFDIR)
	@for file in config/*.cf ; do \
		cf=`echo $$file | sed -e 's;.*/;;'`; \
		echo $(INSTALLPROG) -c -m 444 $(INSTALLOWN) $$file ${DESTDIR}$(CONFDIR)/$$cf; \
		$(INSTALLPROG) -c -m 444 $(INSTALLOWN) $$file \
			${DESTDIR}$(CONFDIR)/$$cf; \
	done

install.compat:
	$(INSTALLPROG) -c -m 555 -o bin -g bin cpumodel.sh /usr/lsd/etc/cpumodel
	rm -f $MAN/${PROG}.8 $MAN/cpumodel.1
	(cd /usr/usc/bin; rm -f sysinfo; ln -s ${BIN}/sysinfo)

clean:
	rm -f *.o ${PROG} *~ *% \#* core a.out metasysinfo \
		versinfo.c sysinfo.tar sysinfo.tar.Z

${OBJS}: os.h defs.h

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
		KVMDIR="$(KVMDIR)" \
		CONFDIR=${CONFDIR} \
		LIBKVM=-lkvm \
		MISSINGFILES="strcasecmp.o strdup.o strerror.o" \
		OSDEFINES="-Dsunos -DOSMVER=4 -D`/bin/arch -k`" \
		OSFILES="${SunOS4-OS-Files}" \
		OSLDFLAGS="-Bstatic" \
		OSLIBS="" \
		PROG=${PROG} \
		all

#
# SunOS 5.x (Solaris 2.x)
#
sunos5 solaris2:
	@echo "Making ${PROG} for SunOS 5.x ... "; \
	echo "Make sure that \`cc' is not \`/usr/ucb/cc' or sysinfo may not work.";
	if [ $$OSver -lt 54 ]; then \
		OtherLibs="-lgen"; \
	fi; \
	$(MAKE)	$(MFLAGS) \
		KVMDIR="" \
		CONFDIR=${CONFDIR} \
		LIBKVM=-lkvm \
		MISSINGFILES="setreuid.o" \
		OSDEFINES="-Dsunos -DOSMVER=${OSmver} -DOSVER=${OSver}" \
		OSFILES="${SunOS5-OS-Files}" \
		OSLDFLAGS="-i -R/usr/lib" \
		OSLIBS="-lsocket -lnsl -lelf -ladm $$OtherLibs" \
		PROG=${PROG} \
		all

#
# AIX 3.x for RS/6000
# AIX 4.x for RS/6000
#
aix3 aix4:
	@echo "Making ${PROG} for AIX ... "; \
	$(MAKE) $(MFLAGS) \
		KVMDIR="" \
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
		KVMDIR="" \
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
# HP-UX 10.x
#
hpux10:
	@echo "Making ${PROG} for HP-UX 10.x ... "; \
	$(MAKE) $(MFLAGS) \
		KVMDIR="" \
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
# Convex SPP-UX
#
sppux3:
	@echo "Making ${PROG} for SPP-UX ... "; \
	echo "Ignore errors regarding /usr/include/sys/wait.h"; \
	$(MAKE) $(MFLAGS) \
		KVMDIR="" \
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
# IRIX
#
irix5:	
	@echo "Making ${PROG} for IRIX 5.x ... "; \
	$(MAKE) $(MFLAGS) \
		KVMDIR="" \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o" \
		OPT="-g" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver}" \
		OSCFLAGS="-woff 1174 -woff 1552 -woff 1204 -woff 1009" \
		OSFILES="${IRIX-OS-Files}" \
		OSLIBS="-lmld -lelf" \
		PROG=${PROG} \
		all

irix6 irix64:	
	@echo "Making ${PROG} for IRIX 6.x ... "; \
	$(MAKE) $(MFLAGS) \
		KVMDIR="" \
		CONFDIR=${CONFDIR} \
		LIBKVM="" \
		MISSINGFILES="kvmlib.o" \
		OPT="-g" \
		OSCFLAGS="-woff 1174 -woff 1552 -woff 1204 -woff 1009" \
		OSDEFINES="-D${OSname} -DOSMVER=${OSmver}" \
		OSFILES="${IRIX-OS-Files}" \
		OSLIBS="-lelf" \
		PROG=${PROG} \
		all

