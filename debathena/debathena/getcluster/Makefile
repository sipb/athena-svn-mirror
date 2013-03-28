all:

getcluster: getcluster.py
	cp getcluster.py getcluster
	chmod 755 getcluster

check:	getcluster
	sh tests.sh

install:
	mkdir -p ${DESTDIR}/bin
	mkdir -p ${DESTDIR}/usr/share/man/man1
	install -m 755 getcluster ${DESTDIR}/bin
	install -m444 getcluster.1 ${DESTDIR}/usr/share/man/man1

clean:
	rm -f getcluster

distclean: clean