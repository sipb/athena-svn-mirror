# $Id: Makefile,v 1.11 1997-03-31 03:37:37 ghudson Exp $

SHELL=/bin/sh
ATHRBINDIR=/bin/athena
ATHBINDIR=/usr/athena/bin
ATHETCDIR=/usr/athena/etc
ATHMANDIR=/usr/athena/man
ATTACHRUN=addusr blanche bos chfn chpobox chsh dcmmaint help kermit klog \
	listmaint mkserv mailmaint moira mrcheck namespace olc_answers olh \
	olh_ascii olh_motif pts sis techinfo usermaint userreg vos xinfo \
	xtechinfo
ANDREW=/afs/athena.mit.edu/astaff/project/andrew/export
MOIRA=/afs/athena.mit.edu/system/moira
HELP=/afs/athena.mit.edu/system/help
MKSERV=/afs/athena.mit.edu/system/mkserv
MODEM=/afs/athena.mit.edu/system/modem
TELLME=/afs/athena.mit.edu/system/config/tellme

# We make a lot of symlinks here, so we're going to make a hack to make it
# a little more readable.
SYMLINKS=	/mit/sipb			/usr/sipb \
		kpasswd				${ATHBINDIR}/passwd \
		/usr/bin/passwd			${ATHBINDIR}/passwd.real \
		${MOIRA}/man/man1/addusr.1	${ATHMANDIR}/man1/addusr.1 \
		${MOIRA}/man/cat1/addusr.1	${ATHMANDIR}/cat1/addusr.1 \
		${MOIRA}/man/man1/blanche.1	${ATHMANDIR}/man1/blanche.1 \
		${MOIRA}/man/cat1/blanche.1	${ATHMANDIR}/cat1/blanche.1 \
		${MOIRA}/man/man1/chfn.1	${ATHMANDIR}/man1/chfn.1 \
		${MOIRA}/man/cat1/chfn.1	${ATHMANDIR}/cat1/chfn.1 \
		${MOIRA}/man/man1/chpobox.1	${ATHMANDIR}/man1/chpobox.1 \
		${MOIRA}/man/cat1/chpobox.1	${ATHMANDIR}/cat1/chpobox.1 \
		${MOIRA}/man/man1/chsh.1	${ATHMANDIR}/man1/chsh.1 \
		${MOIRA}/man/cat1/chsh.1	${ATHMANDIR}/cat1/chsh.1 \
		${MOIRA}/man/man1/dcmmaint.1	${ATHMANDIR}/man1/dcmmaint.1 \
		${MOIRA}/man/cat1/dcmmaint.1	${ATHMANDIR}/cat1/dcmmaint.1 \
		${HELP}/man/man1/help.1		${ATHMANDIR}/man1/help.1 \
		${HELP}/man/cat1/help.1		${ATHMANDIR}/cat1/help.1 \
		${MOIRA}/man/man1/listmaint.1	${ATHMANDIR}/man1/listmaint.1 \
		${MOIRA}/man/cat1/listmaint.1	${ATHMANDIR}/cat1/listmaint.1 \
		${MOIRA}/man/man1/mailmaint.1	${ATHMANDIR}/man1/mailmaint.1 \
		${MOIRA}/man/cat1/mailmaint.1	${ATHMANDIR}/cat1/mailmaint.1 \
		${MODEM}/man/man1/kermit.1	${ATHMANDIR}/man1/kermit.1 \
		${MODEM}/man/cat1/kermit.1	${ATHMANDIR}/cat1/kermit.1 \
		${MOIRA}/man/man1/moira.1	${ATHMANDIR}/man1/moira.1 \
		${MOIRA}/man/cat1/moira.1	${ATHMANDIR}/cat1/moira.1 \
		${MOIRA}/man/man1/mrcheck.1	${ATHMANDIR}/man1/mrcheck.1 \
		${MOIRA}/man/cat1/mrcheck.1	${ATHMANDIR}/cat1/mrcheck.1 \
		${HELP}/man/man1/olh.1		${ATHMANDIR}/man1/olh.1 \
		${HELP}/man/cat1/olh.1		${ATHMANDIR}/cat1/olh.1 \
		${MOIRA}/man/man1/usermaint.1	${ATHMANDIR}/man1/usermaint.1 \
		${MOIRA}/man/cat1/usermaint.1	${ATHMANDIR}/cat1/usermaint.1 \
		${MOIRA}/man/man1/userreg.1	${ATHMANDIR}/man1/userreg.1 \
		${MOIRA}/man/cat1/userreg.1	${ATHMANDIR}/cat1/userreg.1

all: ${ATTACHRUN} verify-message
	cd platform/${HOSTTYPE} && ${MAKE} $@

${ATTACHRUN}: specs template.sh
	set `grep '^$@' specs`; locker=$$2; program=$${3-$$1}; \
		path=`athdir -c -p /mit/$$2 -t bin`/$$program; \
		sed -e "s|@LOCKER@|$$2|g" -e "s|@PATH@|$$path|g" \
			-e "s|@PROGRAM@|$$program|g" template.sh > $@

verify-message: verify-message.in
	path=`athdir -c -p /mit/pgp -t bin`; \
		sed -e 's|@PATH@|$$path|g' verify-message.in > $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}/usr
	-mkdir -p ${DESTDIR}${ATHRBINDIR}
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${ATHETCDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat1
	install -c -m 555 addusr ${DESTDIR}${ATHBINDIR}
	install -c -m 555 blanche ${DESTDIR}${ATHBINDIR}
	install -c -m 555 bos ${DESTDIR}${ATHRBINDIR}
	install -c -m 555 chfn ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chpobox ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chsh ${DESTDIR}${ATHBINDIR}
	install -c -m 555 dcmmaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 emacs19.sh ${DESTDIR}${ATHBINDIR}/emacs19
	install -c -m 555 help ${DESTDIR}${ATHBINDIR}
	install -c -m 555 kermit ${DESTDIR}${ATHBINDIR}
	install -c -m 555 klog ${DESTDIR}${ATHRBINDIR}
	install -c -m 555 mkserv ${DESTDIR}${ATHBINDIR}
	install -c -m 555 listmaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 mailmaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 moira ${DESTDIR}${ATHBINDIR}
	install -c -m 555 mrcheck ${DESTDIR}${ATHBINDIR}
	install -c -m 555 namespace ${DESTDIR}${ATHBINDIR}
	install -c -m 555 olc_answers ${DESTDIR}${ATHBINDIR}
	install -c -m 555 olh ${DESTDIR}${ATHBINDIR}
	install -c -m 555 olh_ascii ${DESTDIR}${ATHBINDIR}
	install -c -m 555 olh_motif ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psgrind.sh ${DESTDIR}${ATHBINDIR}/psgrind
	install -c -m 555 pts ${DESTDIR}${ATHRBINDIR}
	install -c -m 555 sis ${DESTDIR}${ATHBINDIR}
	install -c -m 555 techinfo ${DESTDIR}${ATHBINDIR}
	install -c -m 555 usermaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 userreg ${DESTDIR}${ATHBINDIR}
	install -c -m 555 verify-message ${DESTDIR}${ATHBINDIR}
	install -c -m 555 vos ${DESTDIR}${ATHRBINDIR}
	install -c -m 555 xinfo ${DESTDIR}${ATHBINDIR}
	install -c -m 555 xtechinfo ${DESTDIR}${ATHBINDIR}
	install -c -m 444 sis.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 444 xinfo.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 444 xtechinfo.1 ${DESTDIR}${ATHMANDIR}/man1
	@set ${SYMLINKS}; while [ $$# -ne 0 ]; do \
		echo rm -f ${DESTDIR}$$2; \
		rm -f ${DESTDIR}$$2; \
		echo ln -s $$1 ${DESTDIR}$$2; \
		ln -s $$1 ${DESTDIR}$$2; \
		shift 2; \
	done
	rm -f ${DESTDIR}${ATHETCDIR}/update_server
	ln -s `athdir -c -p ${MOIRA} -t bin`/update_server \
		${DESTDIR}${ATHETCDIR}/update_server
	rm -f ${DESTDIR}${ATHBINDIR}/tellme
	ln -s `athdir -c -p ${TELLME} -t bin`/tellme \
		${DESTDIR}${ATHBINDIR}/tellme
	rm -f ${DESTDIR}/usr/andrew
	. ../build/version; \
		vers=$$major.$$minor; \
		ln -s ${ANDREW}/athena-$$vers/$$HOSTTYPE ${DESTDIR}/usr/andrew
	cd platform/${HOSTTYPE} && ${MAKE} $@

clean:
	rm -f ${ATTACHRUN} verify-message
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	rm -f ${ATTACHRUN} verify-message
	cd platform/${HOSTTYPE} && ${MAKE} $@
