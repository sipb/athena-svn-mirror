#
# Makefile to install NYSERNet snmp distribution
#
DIRS=		lib server/src 

DESTDIR=
# customize these to your local configuration
BIN=		${DESTDIR}/${MACHTYPE}bin
MAN1=		${DESTDIR}/man/man1
MAN3=		${DESTDIR}/man/man3
MAN8=		${DESTDIR}/man/man8
LIB=		${DESTDIR}/${MACHTYPE}lib
#
# If you want 'cc' to search for the include files and libraries in some
# other directories beside /usr/include and /usr/lib, add -I and -L
# options to CFLAGS macro.
#
# e.g.
# CFLAGS= -I/usr/myinclude -L/usr/mylib
#
# See manual for 'cc' for more detail
#
CFLAGS=-L../../lib/obj

all:
	@-for i in $(DIRS) ; do \
		cd $$i ; \
		make CFLAGS="$(CFLAGS)" LIB="$(LIB)" ; \
		cd .. ; \
	done

install:
	@-for i in $(DIRS) ; do \
		cd $$i ; \
		make CFLAGS="$(CFLAGS)" BIN="$(BIN)" LIB="$(LIB)" MAN1="$(MAN1)" MAN3="$(MAN3)" MAN8="$(MAN8)" DESTDIR=$(DESTDIR) install ; \
		cd .. ; \
	done

instman:
	@-mkdir ${DESTDIR}/include/arpa/snmp
	@-cp h/*.h ${DESTDIR}/include/arpa/snmp
	@-cp etc/* ${DESTDIR}/etc
	@cd man; \
	make MAN1="$(MAN1)"  MAN3="$(MAN3)" MAN8="$(MAN8)" install

clean:
	@for i in $(DIRS) ; do \
		cd $$i ; \
		make clean ; \
		cd .. ; \
	done
