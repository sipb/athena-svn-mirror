#       This is the file Makefile for the Xquota. 
#       
#       Created: 	October 22, 1987
#       By:		Chris D. Peterson
#    
#       $Source: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v $
#       $Author: probe $
#       $Header: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v 1.7 1989-06-13 18:28:51 probe Exp $
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
CC=pcc		# hc1.4 can't deal with widget.c

all:		xquota
xquota:		${OBJS}
		cc ${CFLAGS} ${LDFLAGS} ${OBJS} -o xquota ${LIBS}
clean: 	;
	rm -f *~ *.o xquota xquota.shar core xquota.cat 

install: 	xquota
	install -c -s xquota ${DESTDIR}/usr/athena/xquota
	install -c Xquota.ad ${DESTDIR}${LIBDIR}/Xquota.ad
	install -c top_help.txt ${DESTDIR}${LIBDIR}/top_help.txt
	install -c pop_help.txt ${DESTDIR}${LIBDIR}/pop_help.txt

depend:
	makedepend  -s "# DO NOT DELETE THIS LINE -- make depend uses it"\
                $(CFLAGS) $(INCFLAGS) ${SRCS}

# DO NOT DELETE THIS LINE -- make depend uses it

handler.o: /usr/include/stdio.h /usr/include/sys/time.h
handler.o: /usr/include/sys/time.h /usr/include/sys/param.h
handler.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
handler.o: /usr/include/sys/types.h /usr/include/ufs/quota.h
handler.o: /usr/include/X11/Intrinsic.h /usr/include/X11/Xlib.h
handler.o: /usr/include/X11/X.h /usr/include/X11/Xutil.h
handler.o: /usr/include/X11/Xresource.h /usr/include/X11/Xos.h
handler.o: /usr/include/strings.h /usr/include/sys/file.h
handler.o: /usr/include/X11/Core.h /usr/include/X11/Composite.h
handler.o: /usr/include/X11/Constraint.h xquota.h
main.o: /usr/include/stdio.h /usr/include/X11/Intrinsic.h
main.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h /usr/include/X11/X.h
main.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
main.o: /usr/include/X11/Xos.h /usr/include/strings.h /usr/include/sys/file.h
main.o: /usr/include/sys/time.h /usr/include/sys/time.h
main.o: /usr/include/X11/Core.h /usr/include/X11/Composite.h
main.o: /usr/include/X11/Constraint.h /usr/include/pwd.h
main.o: /usr/include/hesiod.h /usr/include/ctype.h xquota.h
quota.o: /usr/include/stdio.h /usr/include/mntent.h /usr/include/strings.h
quota.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
quota.o: /usr/include/sys/signal.h /usr/include/sys/types.h
quota.o: /usr/include/sys/file.h /usr/include/sys/stat.h
quota.o: /usr/include/ufs/quota.h /usr/include/rpc/rpc.h
quota.o: /usr/include/rpc/types.h /usr/include/netinet/in.h
quota.o: /usr/include/rpc/xdr.h /usr/include/rpc/auth.h
quota.o: /usr/include/rpc/clnt.h /usr/include/rpc/rpc_msg.h
quota.o: /usr/include/rpc/auth_unix.h /usr/include/rpc/svc.h
quota.o: /usr/include/rpc/svc_auth.h /usr/include/rpc/pmap_prot.h
quota.o: /usr/include/sys/socket.h /usr/include/netdb.h
quota.o: /usr/include/rpcsvc/rquota.h /usr/include/sys/time.h
quota.o: /usr/include/sys/time.h xquota.h /usr/include/X11/Intrinsic.h
quota.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
quota.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
quota.o: /usr/include/X11/Xos.h /usr/include/X11/Core.h
quota.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
widgets.o: /usr/include/stdio.h /usr/include/X11/Intrinsic.h
widgets.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
widgets.o: /usr/include/X11/X.h /usr/include/X11/Xutil.h
widgets.o: /usr/include/X11/Xresource.h /usr/include/X11/Xos.h
widgets.o: /usr/include/strings.h /usr/include/sys/file.h
widgets.o: /usr/include/sys/time.h /usr/include/sys/time.h
widgets.o: /usr/include/X11/Core.h /usr/include/X11/Composite.h
widgets.o: /usr/include/X11/Constraint.h /usr/include/X11/StringDefs.h
widgets.o: /usr/include/X11/AsciiText.h /usr/include/X11/copyright.h
widgets.o: /usr/include/X11/Text.h /usr/include/X11/Command.h
widgets.o: /usr/include/X11/Label.h /usr/include/X11/Simple.h
widgets.o: /usr/include/X11/Xmu.h /usr/include/X11/Scroll.h
widgets.o: /usr/include/X11/Shell.h /usr/include/X11/Paned.h
widgets.o: /usr/include/X11/Constraint.h xquota.h
