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

depend: 
	makedepend ${CFLAGS} acl_files.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

acl_files.o: acl_files.c /usr/include/stdio.h /usr/include/strings.h
acl_files.o: /usr/include/sys/file.h /usr/include/sys/types.h
acl_files.o: /usr/include/sys/stat.h /usr/include/sys/errno.h
acl_files.o: /usr/include/ctype.h /usr/include/krb.h
acl_files.o: /usr/include/mit-copyright.h /usr/include/des.h
acl_files.o: /usr/include/des_conf.h
