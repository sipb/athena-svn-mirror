DESTDIR=

CFLAGS= -O

all: llib-lacl.ln libacl.a 

install: all
	install libacl.a ${DESTDIR}/usr/athena/lib/libacl.a
	ranlib ${DESTDIR}/usr/athena/lib/libacl.a
	cp llib-lacl.ln ${DESTDIR}/usr/athena/lib/llib-lacl.ln

clean:
	rm -f libacl.a acl_files.o llib-lacl.ln *~ #* 

libacl.a: acl_files.o
	ar crv libacl.a acl_files.o
	ranlib libacl.a

acl_files.o: acl_files.c

lint: llib-lacl.ln

llib-lacl.ln: acl_files.c
	lint -bh -Cacl acl_files.c
