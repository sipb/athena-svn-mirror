# kept from the ftp release...[eichin:19911219.0812EST]
#
# Makefile for error-table routines
#
# Copyright 1987 MIT Student Information Processing Board
# For copyright info, see mit-sipb-copyright.h.
#
#	$Header: /afs/dev.mit.edu/source/repository/third/cns/src/util/et/Makefile,v 1.1.1.1 1996-09-06 00:47:55 ghudson Exp $
#	$Locker:  $
#

DEPEND=$(BUILDTOP)/util/makedepend/makedepend

CFLAGS=	-O
LINTFLAGS= -uhv
LIBOBJS= error_message.o et_name.o init_et.o # perror.o
.SUFFIXES: .h .et

.et.o:
	./compile_et $*.et

.et.h:
	./compile_et $*.et

.c.o:
	${CC} -c -pg ${CFLAGS} $*.c
	mv $*.o profiled/$*.o
	${CC} -c ${CFLAGS} $*.c

all:	compile_et libcom_err.a libcom_err_p.a

lint:	llib-lcom_err.ln

compile_et:	compile_et.o error_table.o
	cc ${CFLAGS} -o compile_et compile_et.o error_table.o -ll

archive:	et.ar

FILES=	Makefile et_name.c error_message.c compile_et.c \
		error_table.y et_lex.lex.l  init_et.c \
		error_table.h \
		test.c test1.et test2.et mit-sipb-copyright.h \
		compiler.h internal.h
CFILES=	compile_et.c error_table.c error_message.c et_name.c  \
	init_et.c

et.ar:	$(FILES)
	ar cruv et.ar $(FILES)

tags:	TAGS

TAGS:	et_name.c error_message.c compile_et.c error_table.c \
		lex.yy.c init_et.c
	etags et_name.c error_message.c compile_et.c \
		error_table.c  init_et.c

libcom_err.a:	$(LIBOBJS)
	ar cruv libcom_err.a $(LIBOBJS)
	ranlib libcom_err.a

libcom_err_p.a:	$(LIBOBJS)
	(cd profiled; ar cruv ../libcom_err_p.a $(LIBOBJS); \
		ranlib ../libcom_err_p.a)

com_err.o:	$(LIBOBJS)
	ld -r -s -o com_err.o $(LIBOBJS)
	chmod -x com_err.o

LINTFILES= error_message.c et_name.c init_et.c # perror.c

llib-lcom_err.ln: $(LINTFILES)
	lint -Ccom_err $(LINTFLAGS) $(LINTFILES)

clean:
	rm -f *~ \#* *.bak \
		*.o profiled/*.o libcom_err.a libcom_err_p.a \
		com_err.o compile_et \
		et.ar TAGS y.tab.c lex.yy.c error_table.c \
		test1.h test2.h test \
		eddep makedep

install: libcom_err.a libcom_err_p.a compile_et
	install -c -m 644 libcom_err.a ${DESTDIR}/usr/athena/lib/libcom_err.a
	install -c -m 644 libcom_err_p.a ${DESTDIR}/usr/athena/lib/libcom_err_p.a
	ranlib ${DESTDIR}/usr/athena/lib/libcom_err.a
	ranlib ${DESTDIR}/usr/athena/lib/libcom_err_p.a
	install -c -s -m 755 compile_et ${DESTDIR}/usr/athena

config:
	-mkdir $(DESTDIR)/util/et
	-mkdir ${DESTDIR}/util/et/profiled
	for i in $(FILES); do \
		rm -f ${DESTDIR}/util/et/`basename $$i`; \
		ln -s $(SRCDIR)/util/et/$$i ${DESTDIR}/util/et/`basename $$i`; done
	rm -f ${DESTDIR}/util/et/Makefile
	cp ${SRCDIR}/util/et/Makefile ${DESTDIR}/util/et/Makefile
	chmod 644 ${DESTDIR}/util/et/Makefile

# for testing
test:	test.o test1.o test2.o libcom_err.a
	cc ${CFLAGS} -o test test.o test1.o test2.o libcom_err.a
test.o:	test1.h test2.h
# 'make depend' code
depend: ${CFILES} et_lex.lex.c
	@echo "### Now computing dependencies"
	@$(DEPEND) -s "# DO NOT DELETE" $(CFLAGS) $(CFILES)
#
# the last line in the makefile should be...
# DO NOT DELETE THIS LINE

compile_et.o: /usr/include/stdio.h /usr/include/sys/types.h
compile_et.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
compile_et.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
compile_et.o: /usr/include/sys/stat.h /usr/include/string.h
compile_et.o: /usr/include/sys/param.h /usr/include/machine/param.h
compile_et.o: /usr/include/sys/signal.h /usr/include/vm/faultcode.h
compile_et.o: mit-sipb-copyright.h compiler.h
error_table.o: /usr/include/stdio.h /usr/include/string.h
error_table.o: /usr/include/sys/stdtypes.h /usr/include/assert.h
error_table.o: /usr/include/ctype.h /usr/include/sys/types.h
error_table.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
error_table.o: /usr/include/sys/time.h internal.h mit-sipb-copyright.h
error_table.o: error_table.h et_lex.lex.c
error_message.o: /usr/include/stdio.h error_table.h mit-sipb-copyright.h
error_message.o: internal.h
et_name.o: error_table.h mit-sipb-copyright.h internal.h
init_et.o: /usr/include/stdio.h error_table.h mit-sipb-copyright.h
