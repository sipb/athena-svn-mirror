# Makefile for OLC/Motif

MACHTYPE  =	vax

MOTIFDIR  =	/afs/athena/astaff/project/motifdev
OLCDEV    =	/mit/olcdev
OLCLIB    =	${OLCDEV}/${MACHTYPE}/clients/lib
BINDIR    =	${MOTIFDIR}/${MACHTYPE}bin
INCDIR    =	-I${MOTIFDIR}/include -I${MOTIFDIR}/include/X11 \
		-I${MOTIFDIR}/include/Mrm -I${MOTIFDIR}/include/Xm \
		-I${OLCDEV}/src/include -I${OLCDEV}/src/clients/olc
LIBDIR    =	-L${MOTIFDIR}/${MACHTYPE}lib -L${OLCDEV}/${MACHTYPE}lib
UILINCDIR =	-I${MOTIFDIR}/include/uil

CC        =	/bin/cc
UIL       =	${BINDIR}/uil
LINT      =	/usr/bin/lint
DEBUG     =	/usr/ucb/dbx
DEPEND    =	/usr/athena/makedepend
RM        =	/bin/rm -f
TAR       =	/bin/tar
COMPRESS  =	/usr/ucb/compress -f

CFLAGS    =	-g ${INCDIR} -DHESIOD -DZEPHYR -DKERBEROS -DATHENA -DTEST
LDFLAGS   =	${LIBDIR}
LINTFLAGS =	 -haucbx ${INCDIR} ${LIBS}
UILFLAGS  =	${UILINCDIR}

RCSDIR    =	${MOTIFDIR}/projects/olc/RCS
BACKUPDIR =	/mit/motifdev/projects/olc/Backup
BACKUPFIL =	${BACKUPDIR}/olc.rcs.tar
ERRORLOG  =	errorlog

SOURCES   =	main.c procs.c widget_create.c x_motd.c x_hand_resp.c
OBJFOOS   =	main.o procs.o widget_create.o x_motd.o x_hand_resp.o \
		${OLCLIB}/../parser/p_utils.o
OBJECTS   =	${OBJFOOS} \
		${OLCLIB}/ask.o		${OLCLIB}/connect.o \
		${OLCLIB}/consult.o	${OLCLIB}/generic.o \
		${OLCLIB}/instance.o	${OLCLIB}/io.o \
		${OLCLIB}/messages.o	${OLCLIB}/motd.o \
		${OLCLIB}/queue.o	${OLCLIB}/resolve.o \
		${OLCLIB}/send.o	${OLCLIB}/topic.o \
		${OLCLIB}/utils.o	${OLCLIB}/status.o \
		${OLCLIB}/../parser/p_utils.o \
		${OLCLIB}/../tty/t_utils.o \
		${OLCLIB}/../../common/io.o \
		${OLCLIB}/../../common/string_utils.o

TARGET    =	xolc
LIBS      =	-lMrm -lXm -lXt -lX11 \
		-lolc \
		-lhesiod -lzephyr -lcom_err -lkrb -ldes

UIL_SRC   =	olc.uil
UID       =	olc.uid


all:  ${UID} ${TARGET} ${OBJFOOS}
#	${CC} ${LDFLAGS} -o test ${OBJFOOS} ${LIBS}

##########  all:	${UID} ${TARGET} ${OBJFOOS}

uil:	${UID}
c:	${TARGET}

${UID}:	${UIL_SRC}
	${UIL} ${UILFLAGS} -o ${UID} ${UIL_SRC}

#${TARGET}:	${OBJECTS}
${TARGET}:	${OBJFOOS}
	${CC} ${LDFLAGS} -o ${TARGET} ${OBJFOOS} ${LIBS}

.c.o:
	${CC} ${CFLAGS} -c $*.c
 
clean:
	${RM} *~ *.bak *.uid *.o ${UID} ${TARGET} 

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

main.o: main.c xolc.h /mit/olcdev/src/include/olc/olc.h /usr/include/stdio.h
main.o: /usr/include/strings.h /usr/include/krb.h
main.o: /usr/include/mit-copyright.h /usr/include/des.h
main.o: /usr/include/des_conf.h /usr/include/hesiod.h
main.o: /mit/olcdev/src/include/olc/macros.h
main.o: /mit/olcdev/src/include/olc/structs.h
main.o: /mit/olcdev/src/include/olc/requests.h
main.o: /mit/olcdev/src/include/olc/procs.h
main.o: /mit/olcdev/src/include/olc/status.h
main.o: /mit/olcdev/src/include/olc/olc_tty.h
main.o: /mit/olcdev/src/include/olc/olc_parser.h
main.o: /mit/olcdev/src/clients/olc/olc.h /usr/include/zephyr/zephyr.h
main.o: /usr/include/zephyr/mit-copyright.h /usr/include/zephyr/zephyr_err.h
main.o: /usr/include/zephyr/zephyr_conf.h /usr/include/errno.h
main.o: /usr/include/sys/types.h /usr/include/netinet/in.h
main.o: /usr/include/sys/time.h /usr/include/sys/time.h
main.o: /afs/athena/astaff/project/motifdev/include/Mrm/XmAppl.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
main.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xutil.h
main.o: /usr/include/X11/Xresource.h /usr/include/X11/Xos.h
main.o: /usr/include/sys/file.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
main.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMPublic.h
main.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMDecls.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
main.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
main.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
main.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
main.o: /usr/include/netdb.h
procs.o: procs.c xolc.h /mit/olcdev/src/include/olc/olc.h
procs.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
procs.o: /usr/include/mit-copyright.h /usr/include/des.h
procs.o: /usr/include/des_conf.h /usr/include/hesiod.h
procs.o: /mit/olcdev/src/include/olc/macros.h
procs.o: /mit/olcdev/src/include/olc/structs.h
procs.o: /mit/olcdev/src/include/olc/requests.h
procs.o: /mit/olcdev/src/include/olc/procs.h
procs.o: /mit/olcdev/src/include/olc/status.h
procs.o: /mit/olcdev/src/include/olc/olc_tty.h
procs.o: /mit/olcdev/src/include/olc/olc_parser.h
procs.o: /mit/olcdev/src/clients/olc/olc.h /usr/include/zephyr/zephyr.h
procs.o: /usr/include/zephyr/mit-copyright.h /usr/include/zephyr/zephyr_err.h
procs.o: /usr/include/zephyr/zephyr_conf.h /usr/include/errno.h
procs.o: /usr/include/sys/types.h /usr/include/netinet/in.h
procs.o: /usr/include/sys/time.h /usr/include/sys/time.h
procs.o: /afs/athena/astaff/project/motifdev/include/Mrm/XmAppl.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
procs.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
procs.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
procs.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
procs.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMPublic.h
procs.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMDecls.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
procs.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
procs.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
procs.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
procs.o: /usr/include/netdb.h
widget_create.o: widget_create.c xolc.h /mit/olcdev/src/include/olc/olc.h
widget_create.o: /usr/include/stdio.h /usr/include/strings.h
widget_create.o: /usr/include/krb.h /usr/include/mit-copyright.h
widget_create.o: /usr/include/des.h /usr/include/des_conf.h
widget_create.o: /usr/include/hesiod.h /mit/olcdev/src/include/olc/macros.h
widget_create.o: /mit/olcdev/src/include/olc/structs.h
widget_create.o: /mit/olcdev/src/include/olc/requests.h
widget_create.o: /mit/olcdev/src/include/olc/procs.h
widget_create.o: /mit/olcdev/src/include/olc/status.h
widget_create.o: /mit/olcdev/src/include/olc/olc_tty.h
widget_create.o: /mit/olcdev/src/include/olc/olc_parser.h
widget_create.o: /mit/olcdev/src/clients/olc/olc.h
widget_create.o: /usr/include/zephyr/zephyr.h
widget_create.o: /usr/include/zephyr/mit-copyright.h
widget_create.o: /usr/include/zephyr/zephyr_err.h
widget_create.o: /usr/include/zephyr/zephyr_conf.h /usr/include/errno.h
widget_create.o: /usr/include/sys/types.h /usr/include/netinet/in.h
widget_create.o: /usr/include/sys/time.h /usr/include/sys/time.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/Mrm/XmAppl.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
widget_create.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
widget_create.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
widget_create.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMPublic.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMDecls.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
widget_create.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
widget_create.o: /usr/include/signal.h /usr/include/sys/stat.h
widget_create.o: /usr/include/pwd.h /usr/include/netdb.h
x_motd.o: x_motd.c xolc.h /mit/olcdev/src/include/olc/olc.h
x_motd.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_motd.o: /usr/include/mit-copyright.h /usr/include/des.h
x_motd.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_motd.o: /mit/olcdev/src/include/olc/macros.h
x_motd.o: /mit/olcdev/src/include/olc/structs.h
x_motd.o: /mit/olcdev/src/include/olc/requests.h
x_motd.o: /mit/olcdev/src/include/olc/procs.h
x_motd.o: /mit/olcdev/src/include/olc/status.h
x_motd.o: /mit/olcdev/src/include/olc/olc_tty.h
x_motd.o: /mit/olcdev/src/include/olc/olc_parser.h
x_motd.o: /mit/olcdev/src/clients/olc/olc.h /usr/include/zephyr/zephyr.h
x_motd.o: /usr/include/zephyr/mit-copyright.h
x_motd.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
x_motd.o: /usr/include/errno.h /usr/include/sys/types.h
x_motd.o: /usr/include/netinet/in.h /usr/include/sys/time.h
x_motd.o: /usr/include/sys/time.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Mrm/XmAppl.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_motd.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_motd.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_motd.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMPublic.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMDecls.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_motd.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_motd.o: /usr/include/signal.h /usr/include/sys/stat.h /usr/include/pwd.h
x_motd.o: /usr/include/netdb.h
x_hand_resp.o: x_hand_resp.c xolc.h /mit/olcdev/src/include/olc/olc.h
x_hand_resp.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/krb.h
x_hand_resp.o: /usr/include/mit-copyright.h /usr/include/des.h
x_hand_resp.o: /usr/include/des_conf.h /usr/include/hesiod.h
x_hand_resp.o: /mit/olcdev/src/include/olc/macros.h
x_hand_resp.o: /mit/olcdev/src/include/olc/structs.h
x_hand_resp.o: /mit/olcdev/src/include/olc/requests.h
x_hand_resp.o: /mit/olcdev/src/include/olc/procs.h
x_hand_resp.o: /mit/olcdev/src/include/olc/status.h
x_hand_resp.o: /mit/olcdev/src/include/olc/olc_tty.h
x_hand_resp.o: /mit/olcdev/src/include/olc/olc_parser.h
x_hand_resp.o: /mit/olcdev/src/clients/olc/olc.h /usr/include/zephyr/zephyr.h
x_hand_resp.o: /usr/include/zephyr/mit-copyright.h
x_hand_resp.o: /usr/include/zephyr/zephyr_err.h
x_hand_resp.o: /usr/include/zephyr/zephyr_conf.h /usr/include/errno.h
x_hand_resp.o: /usr/include/sys/types.h /usr/include/netinet/in.h
x_hand_resp.o: /usr/include/sys/time.h /usr/include/sys/time.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/Mrm/XmAppl.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_hand_resp.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
x_hand_resp.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
x_hand_resp.o: /usr/include/X11/Xos.h /usr/include/sys/file.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Core.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Composite.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Constraint.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Object.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/RectObj.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/WindowObj.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/CompObj.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMPublic.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/Mrm/MRMDecls.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/StringDefs.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Shell.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Vendor.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/Xm/Xm.h
x_hand_resp.o: /afs/athena/astaff/project/motifdev/include/X11/Intrinsic.h
x_hand_resp.o: /usr/include/signal.h /usr/include/sys/stat.h
x_hand_resp.o: /usr/include/pwd.h /usr/include/netdb.h
