#       This is the file Makefile for the Xquota. 
#       
#       Created: 	October 22, 1987
#       By:		Chris D. Peterson
#    
#       $Source: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v $
#       $Author: epeisach $
#       $Header: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v 1.4 1989-05-24 14:08:56 epeisach Exp $
#       
#          Copyright 1987, 1988 by the Massachusetts Institute of Technology.
#    
#       For further information on copyright and distribution 
#       see the file mit-copyright.h
    
LIBDIR= /usr/lib/X11/app-defaults

HELPFILES= -DDEFAULTS_FILE=\"${LIBDIR}/Xquota.ad\"\
	   -DTOP_HELP_FILE=\"${LIBDIR}/top_help.txt\"\
	   -DPOPUP_HELP_FILE=\"${LIBDIR}/pop_help.txt\"

DESTDIR=
CFLAGS= -O ${INCFLAGS} ${HELPFILES}
SFLAGS= ${INCFLAGS} 
INCFLAGS= -I.
LDFLAGS=
LIBS= -lXaw -lXt -lXmu -lX11 -lhesiod -lrpcsvc
SRCS=handler.c main.c quota.c widgets.c
OBJS=handler.o main.o quota.o widgets.o
INCLUDE= xquota.h
CFILES=handler.c main.c quota.c widgets.c

all:		xquota
xquota:		${OBJS}
		cc ${CFLAGS} ${LDFLAGS} ${OBJS} -o xquota ${LIBS}
clean: 	;
	rm -f *~ *.o xquota xquota.shar core xquota.cat 

install: 	xquota
	install -c -s xquota ${DESTDIR}/usr/athena/xquota
	install -c Xquota.ad ${DESTDIR}${LIBDIR}Xquota.ad
	install -c top_help.txt ${DESTDIR}${LIBDIR}top_help.txt
	install -c pop_help.txt ${DESTDIR}${LIBDIR}pop_help.txt

depend:
	makedepend -o "" -s "# DO NOT DELETE THIS LINE -- make depend uses it"\
                $(CFLAGS) $(INCFLAGS) `for i in ${SRCS}; do \
                                        echo $$i; done`

# DO NOT DELETE THIS LINE -- make depend uses it

handler: /usr/include/stdio.h /usr/include/sys/time.h /usr/include/sys/time.h
handler: /usr/include/sys/param.h /usr/include/machine/machparam.h
handler: /usr/include/sys/signal.h /usr/include/sys/types.h
handler: /usr/include/ufs/quota.h /usr/include/X11/Intrinsic.h
handler: /usr/include/X11/Xlib.h /usr/include/X11/X.h
handler: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
handler: /usr/include/X11/Xos.h /usr/include/strings.h
handler: /usr/include/sys/file.h /usr/include/X11/Core.h
handler: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h xquota.h
main: /usr/include/stdio.h /usr/include/X11/Intrinsic.h
main: /usr/include/X11/Xlib.h /usr/include/sys/types.h /usr/include/X11/X.h
main: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
main: /usr/include/X11/Xos.h /usr/include/strings.h /usr/include/sys/file.h
main: /usr/include/sys/time.h /usr/include/sys/time.h /usr/include/X11/Core.h
main: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
main: /usr/include/pwd.h /usr/include/hesiod.h /usr/include/ctype.h xquota.h
quota: /usr/include/stdio.h /usr/include/mntent.h /usr/include/strings.h
quota: /usr/include/sys/param.h /usr/include/machine/machparam.h
quota: /usr/include/sys/signal.h /usr/include/sys/types.h
quota: /usr/include/sys/file.h /usr/include/sys/stat.h
quota: /usr/include/ufs/quota.h /usr/include/rpc/rpc.h
quota: /usr/include/rpc/types.h /usr/include/netinet/in.h
quota: /usr/include/rpc/xdr.h /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
quota: /usr/include/rpc/rpc_msg.h /usr/include/rpc/auth_unix.h
quota: /usr/include/rpc/svc.h /usr/include/rpc/svc_auth.h
quota: /usr/include/rpc/pmap_prot.h /usr/include/sys/socket.h
quota: /usr/include/netdb.h /usr/include/rpcsvc/rquota.h
quota: /usr/include/sys/time.h /usr/include/sys/time.h xquota.h
quota: /usr/include/X11/Intrinsic.h /usr/include/X11/Xlib.h
quota: /usr/include/X11/X.h /usr/include/X11/Xutil.h
quota: /usr/include/X11/Xresource.h /usr/include/X11/Xos.h
quota: /usr/include/X11/Core.h /usr/include/X11/Composite.h
quota: /usr/include/X11/Constraint.h
widgets: /usr/include/stdio.h /usr/include/X11/Intrinsic.h
widgets: /usr/include/X11/Xlib.h /usr/include/sys/types.h
widgets: /usr/include/X11/X.h /usr/include/X11/Xutil.h
widgets: /usr/include/X11/Xresource.h /usr/include/X11/Xos.h
widgets: /usr/include/strings.h /usr/include/sys/file.h
widgets: /usr/include/sys/time.h /usr/include/sys/time.h
widgets: /usr/include/X11/Core.h /usr/include/X11/Composite.h
widgets: /usr/include/X11/Constraint.h /usr/include/X11/StringDefs.h
widgets: /usr/include/X11/AsciiText.h /usr/include/X11/copyright.h
widgets: /usr/include/X11/Text.h /usr/include/X11/Command.h
widgets: /usr/include/X11/Label.h /usr/include/X11/Simple.h
widgets: /usr/include/X11/Xmu.h /usr/include/X11/Scroll.h
widgets: /usr/include/X11/Shell.h /usr/include/X11/Paned.h
widgets: /usr/include/X11/Constraint.h xquota.h
