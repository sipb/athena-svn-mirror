# Makefile for OLC/Motif
#
#  This makefile depends on the macro MACHINE.  On Athena, this is
#  defined automatically.  If it is not defined on your system, you can
#  either add a MACHINE macro to this file or set an environment
#  variable to the correct machine type.  On Athena, this would be done
#   by:		setenv  MACHTYPE  ${hosttype}
#   or:		setenv  MACHTYPE  `machtype`
# 

MOTIFDIR  =	/afs/athena/astaff/project/motifdev
OLCDEV    =	/afs/athena/astaff/project/olcdev
OLCLIB    =	${OLCDEV}/${MACHINE}/clients/lib
BINDIR    =	${MOTIFDIR}/${MACHINE}bin
INCDIR    =	-I${MOTIFDIR}/include \
		-I${OLCDEV}/src/include -I${OLCDEV}/src/clients/olc
LIBDIR    =	-L${MOTIFDIR}/${MACHINE}lib -L${OLCDEV}/${MACHINE}lib

CC	  =	/afs/athena/project/gnu/${MACHINE}bin/gcc
LINT      =	/usr/bin/lint
DEBUG     =	/usr/ucb/dbx
DEPEND    =	/usr/athena/makedepend
RM        =	/bin/rm -f
TAR       =	/bin/tar
COMPRESS  =	/usr/ucb/compress -fv

CFLAGS    =	-g ${INCDIR} -DHESIOD -DZEPHYR -DKERBEROS -DATHENA
LDFLAGS   =	${LIBDIR}
LINTFLAGS =	 -haucbx ${INCDIR} ${LIBS}

RCSDIR    =	${MOTIFDIR}/projects/olc/RCS
BACKUPDIR =	/mit/motifdev/backups
BACKUPFIL =	${BACKUPDIR}/olc.rcs.tar
ERRORLOG  =	errorlog

SOURCES   =	main.c procs.c visual.c \
		x_ask.c x_instance.c x_motd.c x_resolve.c \
		x_send.c x_topic.c x_utils.c
OBJFOOS   =	main.o procs.o visual.o \
		x_ask.o x_instance.o x_motd.o x_resolve.o \
		x_send.o x_topic.o x_utils.o
OBJECTS   =	${OBJFOOS}

TARGET    =	xolc
LIBS      =	-lMu \
		-lMrm -lXm -lXt -lX11 \
		-lolc \
		-lhesiod -lzephyr -lcom_err -lkrb -ldes

all:  ${TARGET}

link:	
	${CC} ${LDFLAGS} -o ${TARGET} ${OBJECTS} ${LIBS}

${TARGET}:	${OBJFOOS}
	${CC} ${LDFLAGS} -o ${TARGET} ${OBJECTS} ${LIBS}

.c.o:
	${CC} ${CFLAGS} -c $*.c
 
clean:
	${RM} *~ *.bak *.o ${TARGET} 

lint:
	${LINT} ${LINTFLAGS} ${CFLAGS} ${SOURCES}

dbx:	${TARGET}
	${DEBUG} ${TARGET}

backup:
	${TAR} -cvf ${BACKUPFIL} ${RCSDIR}
	${COMPRESS} ${BACKUPFIL}

depend: 
	${DEPEND} ${CFLAGS} ${SOURCES}


# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: main.c xolc.h /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
main.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
main.o: /usr/include/mit-copyright.h /usr/include/des.h
main.o: /usr/include/des_conf.h /usr/include/hesiod.h
main.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
main.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
main.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
main.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
main.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
main.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
main.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
main.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
main.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
main.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
main.o: /usr/include/errno.h /usr/include/sys/types.h
main.o: /usr/include/netinet/in.h /usr/include/sys/time.h
main.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
main.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
main.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
main.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
main.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
main.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
main.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
main.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
main.o: /afs/athena/astaff/project/motifdev/include/Mu.h
main.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
main.o: /usr/include/netdb.h data.h
procs.o: procs.c xolc.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
procs.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
procs.o: /usr/include/mit-copyright.h /usr/include/des.h
procs.o: /usr/include/des_conf.h /usr/include/hesiod.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
procs.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
procs.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
procs.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
procs.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
procs.o: /usr/include/errno.h /usr/include/sys/types.h
procs.o: /usr/include/netinet/in.h /usr/include/sys/time.h
procs.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
procs.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
procs.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
procs.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
procs.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
procs.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
procs.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
procs.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
procs.o: /afs/athena/astaff/project/motifdev/include/Mu.h
procs.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
procs.o: /usr/include/netdb.h data.h
visual.o: visual.c /afs/athena/astaff/project/motifdev/include/Xm/Form.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
visual.o: /usr/include/sys/types.h /usr/include/X11/Xlib.h
visual.o: /usr/include/X11/X.h /usr/include/X11/Xutil.h
visual.o: /usr/include/X11/Xresource.h /usr/include/X11/Xos.h
visual.o: /usr/include/strings.h /usr/include/sys/file.h
visual.o: /usr/include/sys/time.h /usr/include/sys/time.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/PushBG.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/SeparatoG.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/LabelG.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/Frame.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/RowColumn.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/MessageB.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/Text.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/List.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/PanedW.h
visual.o: /afs/athena/astaff/project/motifdev/include/Xm/DialogS.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h visual.h
visual.o: xolc.h /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
visual.o: /usr/include/stdio.h /usr/include/krb.h
visual.o: /usr/include/mit-copyright.h /usr/include/des.h
visual.o: /usr/include/des_conf.h /usr/include/hesiod.h
visual.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
visual.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
visual.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
visual.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
visual.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
visual.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
visual.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
visual.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
visual.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
visual.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
visual.o: /usr/include/errno.h /usr/include/netinet/in.h
visual.o: /usr/include/X11/cursorfont.h
visual.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
visual.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
visual.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
visual.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
visual.o: /afs/athena/astaff/project/motifdev/include/Mu.h
visual.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
visual.o: /usr/include/netdb.h
x_ask.o: x_ask.c xolc.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
x_ask.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_ask.o: /usr/include/mit-copyright.h /usr/include/des.h
x_ask.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
x_ask.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
x_ask.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
x_ask.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
x_ask.o: /usr/include/errno.h /usr/include/sys/types.h
x_ask.o: /usr/include/netinet/in.h /usr/include/sys/time.h
x_ask.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
x_ask.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_ask.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_ask.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_ask.o: /afs/athena/astaff/project/motifdev/include/Mu.h
x_ask.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
x_ask.o: /usr/include/netdb.h
x_instance.o: x_instance.c
x_instance.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
x_instance.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_instance.o: /usr/include/mit-copyright.h /usr/include/des.h
x_instance.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_instance.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
x_instance.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
x_instance.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
x_instance.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
x_instance.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
x_instance.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
x_motd.o: x_motd.c xolc.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
x_motd.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_motd.o: /usr/include/mit-copyright.h /usr/include/des.h
x_motd.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
x_motd.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
x_motd.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
x_motd.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
x_motd.o: /usr/include/errno.h /usr/include/sys/types.h
x_motd.o: /usr/include/netinet/in.h /usr/include/sys/time.h
x_motd.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
x_motd.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_motd.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_motd.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Mu.h
x_motd.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
x_motd.o: /usr/include/netdb.h
x_resolve.o: x_resolve.c xolc.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
x_resolve.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_resolve.o: /usr/include/mit-copyright.h /usr/include/des.h
x_resolve.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
x_resolve.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
x_resolve.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
x_resolve.o: /usr/include/zephyr/zephyr_err.h
x_resolve.o: /usr/include/zephyr/zephyr_conf.h /usr/include/errno.h
x_resolve.o: /usr/include/sys/types.h /usr/include/netinet/in.h
x_resolve.o: /usr/include/sys/time.h /usr/include/sys/time.h
x_resolve.o: /usr/include/X11/cursorfont.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
x_resolve.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_resolve.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_resolve.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_resolve.o: /afs/athena/astaff/project/motifdev/include/Mu.h
x_resolve.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
x_resolve.o: /usr/include/netdb.h data.h
x_send.o: x_send.c xolc.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
x_send.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_send.o: /usr/include/mit-copyright.h /usr/include/des.h
x_send.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
x_send.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
x_send.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
x_send.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
x_send.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
x_send.o: /usr/include/errno.h /usr/include/sys/types.h
x_send.o: /usr/include/netinet/in.h /usr/include/sys/time.h
x_send.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
x_send.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
x_send.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_send.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_send.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_send.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
x_send.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_send.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_send.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_send.o: /afs/athena/astaff/project/motifdev/include/Mu.h
x_send.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
x_send.o: /usr/include/netdb.h
x_topic.o: x_topic.c xolc.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
x_topic.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_topic.o: /usr/include/mit-copyright.h /usr/include/des.h
x_topic.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
x_topic.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
x_topic.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
x_topic.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
x_topic.o: /usr/include/errno.h /usr/include/sys/types.h
x_topic.o: /usr/include/netinet/in.h /usr/include/sys/time.h
x_topic.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
x_topic.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_topic.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_topic.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_topic.o: /afs/athena/astaff/project/motifdev/include/Mu.h
x_topic.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
x_topic.o: /usr/include/netdb.h
x_utils.o: x_utils.c xolc.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc.h
x_utils.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_utils.o: /usr/include/mit-copyright.h /usr/include/des.h
x_utils.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/macros.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/structs.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/requests.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/procs.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/status.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_tty.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/include/olc/olc_parser.h
x_utils.o: /afs/athena/astaff/project/olcdev/src/clients/olc/olc.h
x_utils.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
x_utils.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
x_utils.o: /usr/include/errno.h /usr/include/sys/types.h
x_utils.o: /usr/include/netinet/in.h /usr/include/sys/time.h
x_utils.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmAppl.h
x_utils.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_utils.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_utils.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmPublic.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/Mrm/MrmDecls.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_utils.o: /afs/athena/astaff/project/motifdev/include/Mu.h
x_utils.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
x_utils.o: /usr/include/netdb.h
