# $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xlogin/Makefile,v 1.1 1990-11-16 17:26:50 mar Exp $
#
# CC can be overridden from driver makefile.  Note the -g flag
# is on the CC macro, not the CFLAGS and LDFLAGS macros, so the
# driver makefile can force optimization instead of debugging.
#
#       CC = gcc -g -DFUNCTION_PROTOTYPES -ansi
       CC = cc  -O
   CFLAGS = -I. -I/usr/include/X11
LINTFLAGS = $(CFLAGS) -b -h -u -v -z

TARGET = xlogin
  SRCS = xlogin.c AriRegAll.c verify.c
  OBJS = xlogin.o AriRegAll.o verify.o
  LIBS = -lXaw -lm ../wcl/libWc.a -lXmu -lXt -lXext -lX11 -lkrb -ldes -lhesiod

all: xlogin timeout

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(TARGET)

timeout: timeout.o
	$(CC) timeout.o -o timeout

lint:
	$(LINT) $(LINTFLAGS) $(SRCS)

clean:
	rm -f $(OBJS) core xlogin timeout timeout.o

install: xlogin timeout
	install -c -s xlogin $(DESTDIR)/etc/athena/xlogin
	-mkdir $(DESTDIR)/etc/athena/login
	install -c check $(DESTDIR)/etc/athena/login/check
	install -c owl $(DESTDIR)/etc/athena/login/owl
	install -c owleyes0 $(DESTDIR)/etc/athena/login/owleyes0
	install -c owleyes1 $(DESTDIR)/etc/athena/login/owleyes1
	install -c owleyes2 $(DESTDIR)/etc/athena/login/owleyes2
	install -c owleyes3 $(DESTDIR)/etc/athena/login/owleyes3
	install -c owleyes4 $(DESTDIR)/etc/athena/login/owleyes4
	install -c owleyes5 $(DESTDIR)/etc/athena/login/owleyes5
	install -c question $(DESTDIR)/etc/athena/login/question
	install -c warning $(DESTDIR)/etc/athena/login/warning
	install -c white $(DESTDIR)/etc/athena/login/white
	install -c Xlogin $(DESTDIR)/etc/athena/login/Xlogin
	install -c -s timeout $(DESTDIR)/usr/athena/timeout
	install -c xlogin.8 $(DESTDIR)/usr/man/man8/xlogin.8
	install -c timeout.1 $(DESTDIR)/usr/man/man1/timeout.1

depend: $(SRCS)
	mkdep $(CFLAGS) $(SRCS)

# DO NOT DELETE THIS LINE -- mkdep uses it.

xlogin.o: xlogin.c /usr/include/stdio.h /usr/include/signal.h
xlogin.o: /usr/include/sys/wait.h /usr/include/sys/time.h
xlogin.o: /usr/include/sys/time.h /usr/include/utmp.h
xlogin.o: /usr/include/X11/Intrinsic.h /usr/include/X11/Xlib.h
xlogin.o: /usr/include/sys/types.h /usr/include/X11/X.h
xlogin.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
xlogin.o: /usr/include/X11/Xos.h /usr/include/strings.h
xlogin.o: /usr/include/sys/file.h /usr/include/X11/Core.h
xlogin.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
xlogin.o: /usr/include/X11/Object.h /usr/include/X11/RectObj.h
xlogin.o: /usr/include/ctype.h ../wcl/WcCreate.h
xlogin.o: /usr/include/X11/IntrinsicP.h /usr/include/X11/CoreP.h
xlogin.o: /usr/include/X11/CompositeP.h /usr/include/X11/ConstrainP.h
xlogin.o: /usr/include/X11/ObjectP.h /usr/include/X11/RectObjP.h
xlogin.o: /usr/include/X11/StringDefs.h /usr/include/X11/Xaw/Label.h
xlogin.o: /usr/include/X11/Xaw/Simple.h /usr/include/X11/Xmu/Converters.h
xlogin.o: /usr/include/X11/Xaw/Text.h /usr/include/X11/Xaw/TextI.h
xlogin.o: /usr/include/X11/Xaw/TextSink.h /usr/include/X11/Xaw/TextSrc.h
xlogin.o: /usr/include/X11/Xaw/AsciiSrc.h /usr/include/X11/Xaw/AsciiSink.h
xlogin.o: /usr/include/X11/Xatom.h
AriRegAll.o: AriRegAll.c /usr/include/X11/Xatom.h
AriRegAll.o: /usr/include/X11/Intrinsic.h /usr/include/X11/Xlib.h
AriRegAll.o: /usr/include/sys/types.h /usr/include/X11/X.h
AriRegAll.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
AriRegAll.o: /usr/include/X11/Xos.h /usr/include/strings.h
AriRegAll.o: /usr/include/sys/file.h /usr/include/sys/time.h
AriRegAll.o: /usr/include/sys/time.h /usr/include/X11/Core.h
AriRegAll.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
AriRegAll.o: /usr/include/X11/Object.h /usr/include/X11/RectObj.h
AriRegAll.o: /usr/include/X11/StringDefs.h /usr/include/X11/Xaw/AsciiText.h
AriRegAll.o: /usr/include/X11/Xaw/Text.h /usr/include/X11/Xaw/TextI.h
AriRegAll.o: /usr/include/X11/Xaw/TextSink.h /usr/include/X11/Xaw/TextSrc.h
AriRegAll.o: /usr/include/X11/Xaw/AsciiSrc.h /usr/include/X11/Xaw/AsciiSink.h
AriRegAll.o: /usr/include/X11/Xaw/Box.h /usr/include/X11/Xmu/Converters.h
AriRegAll.o: /usr/include/X11/Xaw/Clock.h /usr/include/X11/Xaw/Command.h
AriRegAll.o: /usr/include/X11/Xaw/Label.h /usr/include/X11/Xaw/Simple.h
AriRegAll.o: /usr/include/X11/Xaw/Dialog.h /usr/include/X11/Xaw/Form.h
AriRegAll.o: /usr/include/X11/Xaw/Grip.h /usr/include/X11/Xaw/List.h
AriRegAll.o: /usr/include/X11/Xaw/Logo.h /usr/include/X11/Xaw/MenuButton.h
AriRegAll.o: /usr/include/X11/Xaw/Scrollbar.h
AriRegAll.o: /usr/include/X11/Xaw/SimpleMenu.h /usr/include/X11/Shell.h
AriRegAll.o: /usr/include/X11/Xaw/SmeBSB.h /usr/include/X11/Xaw/Sme.h
AriRegAll.o: /usr/include/X11/Xaw/SmeLine.h /usr/include/X11/Xaw/StripChart.h
AriRegAll.o: /usr/include/X11/Xaw/Paned.h /usr/include/X11/Xaw/Toggle.h
AriRegAll.o: /usr/include/X11/Xaw/Viewport.h /usr/include/X11/Xaw/Reports.h
AriRegAll.o: /usr/include/X11/Xaw/Cardinals.h
verify.o: verify.c /usr/include/stdio.h /usr/include/pwd.h
verify.o: /usr/include/strings.h /usr/include/sys/file.h
verify.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
verify.o: /usr/include/signal.h /usr/include/sys/types.h
verify.o: /usr/include/sys/dir.h /usr/include/sys/stat.h
verify.o: /usr/include/sys/time.h /usr/include/sys/time.h /usr/include/utmp.h
verify.o: /usr/include/netdb.h /usr/include/ttyent.h /usr/include/krb.h
verify.o: /usr/include/mit-copyright.h /usr/include/des.h
verify.o: /usr/include/hesiod.h /usr/include/errno.h /usr/include/syslog.h
