DESTDIR=

CFLAGS= -O

all: libacl.a

install: all
	install libacl.a ${DESTDIR}/usr/athena/lib/libacl.a

clean:
	rm -f libacl.a acl_files.o *~ #* 

libacl.a: acl_files.o
	ar crv libacl.a acl_files.o
	ranlib libacl.a

acl_files.o: acl_files.c
