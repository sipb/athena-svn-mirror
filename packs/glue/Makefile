# $Id: Makefile,v 1.58 2004-03-14 22:37:22 zacheiss Exp $

SHELL=/bin/sh
ATHRBINDIR=/bin/athena
ATHBINDIR=/usr/athena/bin
ATHETCDIR=/usr/athena/etc
ATHMANDIR=/usr/athena/man
ATHLIBEXECDIR=/usr/athena/libexec
ATTACHRUN=acroread addusr blanche chfn chpobox chsh dcmmaint gs lastlog \
	listmaint mailmaint mitch moira mrcheck namespace stanley stella \
	usermaint show-application show-audio show-binhex show-excel \
	show-image show-msword show-pdf show-postscript show-text show-video \
	store-binhex afmdit pscat pscatmap psdit psdraft pslpr psnup psplot \
	psrev psroff
MOIRA=/afs/athena.mit.edu/system/moira
MKSERV=/afs/athena.mit.edu/system/mkserv
TELLME=/afs/athena.mit.edu/system/config/tellme
PSUTILS=/afs/athena.mit.edu/astaff/project/psutils

# We make a lot of symlinks here, so we're going to make a hack to make it
# a little more readable.
SYMLINKS=	${MOIRA}/man/man1/addusr.1	${ATHMANDIR}/man1/addusr.1 \
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
		${MOIRA}/man/man1/listmaint.1	${ATHMANDIR}/man1/listmaint.1 \
		${MOIRA}/man/cat1/listmaint.1	${ATHMANDIR}/cat1/listmaint.1 \
		${MOIRA}/man/man1/mailmaint.1	${ATHMANDIR}/man1/mailmaint.1 \
		${MOIRA}/man/cat1/mailmaint.1	${ATHMANDIR}/cat1/mailmaint.1 \
		${MOIRA}/man/man1/mitch.1	${ATHMANDIR}/man1/mitch.1 \
		${MOIRA}/man/cat1/mitch.1	${ATHMANDIR}/cat1/mitch.1 \
		${MOIRA}/man/man1/moira.1	${ATHMANDIR}/man1/moira.1 \
		${MOIRA}/man/cat1/moira.1	${ATHMANDIR}/cat1/moira.1 \
		${MOIRA}/man/man1/mrcheck.1	${ATHMANDIR}/man1/mrcheck.1 \
		${MOIRA}/man/cat1/mrcheck.1	${ATHMANDIR}/cat1/mrcheck.1 \
		${MOIRA}/man/man1/stanley.1	${ATHMANDIR}/man1/stanley.1 \
		${MOIRA}/man/cat1/stanley.1	${ATHMANDIR}/cat1/stanley.1 \
		${MOIRA}/man/man1/stella.1	${ATHMANDIR}/man1/stella.1 \
		${MOIRA}/man/cat1/stella.1	${ATHMANDIR}/cat1/stella.1 \
		${MOIRA}/man/man1/usermaint.1	${ATHMANDIR}/man1/usermaint.1 \
		${MOIRA}/man/cat1/usermaint.1	${ATHMANDIR}/cat1/usermaint.1 \
		${MKSERV}/man/man8/mkserv.8	${ATHMANDIR}/man8/mkserv.8 \
		${MKSERV}/man/cat8/mkserv.8	${ATHMANDIR}/cat8/mkserv.8 \
	${MOIRA}/man/man8/update_server.8  ${ATHMANDIR}/man8/update_server.8 \
	${MOIRA}/man/cat8/update_server.8  ${ATHMANDIR}/cat8/update_server.8 \
		${PSUTILS}/man/man1/ps4014.1	${ATHMANDIR}/man1/ps4014.1 \
		${PSUTILS}/man/cat1/ps4014.1	${ATHMANDIR}/cat1/ps4014.1 \
		${PSUTILS}/man/man1/ps630.1	${ATHMANDIR}/man1/ps630.1 \
		${PSUTILS}/man/cat1/ps630.1	${ATHMANDIR}/cat1/ps630.1 \
		${PSUTILS}/man/man1/pscat.1	${ATHMANDIR}/man1/pscat.1 \
		${PSUTILS}/man/cat1/pscat.1	${ATHMANDIR}/cat1/pscat.1 \
		${PSUTILS}/man/man1/psdit.1	${ATHMANDIR}/man1/psdit.1 \
		${PSUTILS}/man/cat1/psdit.1	${ATHMANDIR}/cat1/psdit.1 \
		${PSUTILS}/man/man1/psdraft.1	${ATHMANDIR}/man1/psdraft.1 \
		${PSUTILS}/man/cat1/psdraft.1	${ATHMANDIR}/cat1/psdraft.1 \
		${PSUTILS}/man/man1/psfax.1	${ATHMANDIR}/man1/psfax.1 \
		${PSUTILS}/man/cat1/psfax.1	${ATHMANDIR}/cat1/psfax.1 \
		${PSUTILS}/man/man1/psfonts.1	${ATHMANDIR}/man1/psfonts.1 \
		${PSUTILS}/man/cat1/psfonts.1	${ATHMANDIR}/cat1/psfonts.1 \
		${PSUTILS}/man/man1/pslpr.1	${ATHMANDIR}/man1/pslpr.1 \
		${PSUTILS}/man/cat1/pslpr.1	${ATHMANDIR}/cat1/pslpr.1 \
		${PSUTILS}/man/man1/psnup.1	${ATHMANDIR}/man1/psnup.1 \
		${PSUTILS}/man/cat1/psnup.1	${ATHMANDIR}/cat1/psnup.1 \
		${PSUTILS}/man/man1/psplot.1	${ATHMANDIR}/man1/psplot.1 \
		${PSUTILS}/man/cat1/psplot.1	${ATHMANDIR}/cat1/psplot.1 \
		${PSUTILS}/man/man1/psrev.1	${ATHMANDIR}/man1/psrev.1 \
		${PSUTILS}/man/cat1/psrev.1	${ATHMANDIR}/cat1/psrev.1 \
		${PSUTILS}/man/man1/psroff.1	${ATHMANDIR}/man1/psroff.1 \
		${PSUTILS}/man/cat1/psroff.1	${ATHMANDIR}/cat1/psroff.1 \
		${PSUTILS}/man/man1/pssymbols.1	${ATHMANDIR}/man1/pssymbols.1 \
		${PSUTILS}/man/cat1/pssymbols.1	${ATHMANDIR}/cat1/pssymbols.1 \
	${PSUTILS}/man/man1/transcript.1	${ATHMANDIR}/man1/transcript.1 \
	${PSUTILS}/man/cat1/transcript.1	${ATHMANDIR}/cat1/transcript.1 \
		${PSUTILS}/man/man7/afm.7	${ATHMANDIR}/man7/afm.7 \
		${PSUTILS}/man/cat7/afm.7	${ATHMANDIR}/cat7/afm.7 \
	${PSUTILS}/man/man7/postscript.7	${ATHMANDIR}/man7/postscript.7 \
	${PSUTILS}.man/cat7/postscript.7	${ATHMANDIR}/cat7/postscript.7 \
		${PSUTILS}/man/man8/pscatmap.8	${ATHMANDIR}/man8/pscatmap.8 \
		${PSUTILS}/man/cat8/pscatmap.8	${ATHMANDIR}/cat8/pscatmap.8 \
		${PSUTILS}/man/man8/psdman.8	${ATHMANDIR}/man8/psdman.8 \
		${PSUTILS}/man/cat8/psdman.8	${ATHMANDIR}/cat8/psdman.8 \
		${PSUTILS}/man/man8/psint.8	${ATHMANDIR}/man8/psint.8 \
		${PSUTILS}/man/cat8/psint.8	${ATHMANDIR}/cat8/psint.8

all: ${ATTACHRUN}
	cd os/${OS} && ${MAKE} $@

${ATTACHRUN}: specs template.sh
	set `grep '^$@[ 	]' specs`; sed -e "s|@LOCKER@|$$2|g" \
		-e "s|@PROGRAM@|$@|g" template.sh > $@

check:
	cd os/${OS} && ${MAKE} $@

install:
	mkdir -p ${DESTDIR}/usr
	mkdir -p ${DESTDIR}${ATHRBINDIR}
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${ATHETCDIR}
	mkdir -p ${DESTDIR}${ATHLIBEXECDIR}
	mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	mkdir -p ${DESTDIR}${ATHMANDIR}/cat1
	mkdir -p ${DESTDIR}${ATHMANDIR}/man7
	mkdir -p ${DESTDIR}${ATHMANDIR}/cat7
	mkdir -p ${DESTDIR}${ATHMANDIR}/man8
	mkdir -p ${DESTDIR}${ATHMANDIR}/cat8
	install -c -m 555 acroread ${DESTDIR}${ATHBINDIR}
	install -c -m 555 addusr ${DESTDIR}${ATHBINDIR}
	install -c -m 555 blanche ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chfn ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chpobox ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chsh ${DESTDIR}${ATHBINDIR}
	install -c -m 555 dcmmaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 gs ${DESTDIR}${ATHBINDIR}
	install -c -m 555 help.sh ${DESTDIR}${ATHBINDIR}/help
	install -c -m 555 htmlview.sh ${DESTDIR}${ATHBINDIR}/htmlview
	install -c -m 555 ksrvutil.sh ${DESTDIR}${ATHBINDIR}/ksrvutil
	install -c -m 555 mkserv.sh ${DESTDIR}${ATHBINDIR}/mkserv
	install -c -m 555 lastlog ${DESTDIR}${ATHBINDIR}
	install -c -m 555 listmaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 mailmaint ${DESTDIR}${ATHBINDIR}
	install -c -m 444 mh.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 555 mitch ${DESTDIR}${ATHBINDIR}
	install -c -m 555 moira ${DESTDIR}${ATHBINDIR}
	install -c -m 555 mrcheck ${DESTDIR}${ATHBINDIR}
	install -c -m 555 namespace ${DESTDIR}${ATHBINDIR}
	install -c -m 555 olc_answers.sh ${DESTDIR}${ATHBINDIR}/olc_answers
	install -c -m 555 psgrind.sh ${DESTDIR}${ATHBINDIR}/psgrind
	install -c -m 555 sis.sh ${DESTDIR}${ATHBINDIR}/sis
	install -c -m 555 stanley ${DESTDIR}${ATHBINDIR}
	install -c -m 555 stella ${DESTDIR}${ATHBINDIR}
	install -c -m 555 usermaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 vacation.sh ${DESTDIR}${ATHBINDIR}/vacation
	install -c -m 555 verify-message.sh \
		${DESTDIR}${ATHBINDIR}/verify-message
	install -c -m 555 xlock.sh ${DESTDIR}${ATHBINDIR}/xlock
	install -c -m 555 show-application ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-audio ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-binhex ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-excel ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-image ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-msword ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-pdf ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-postscript ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-text ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 show-video ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 store-binhex ${DESTDIR}${ATHLIBEXECDIR}
	install -c -m 555 afmdit ${DESTDIR}${ATHBINDIR}
	install -c -m 555 pscat ${DESTDIR}${ATHBINDIR}
	install -c -m 555 pscatmap ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psdit ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psdraft ${DESTDIR}${ATHBINDIR}
	install -c -m 555 pslpr ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psnup ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psplot ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psrev ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psroff ${DESTDIR}${ATHBINDIR}
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
	cd os/${OS} && ${MAKE} $@

clean:
	rm -f ${ATTACHRUN}
	cd os/${OS} && ${MAKE} $@

distclean:
	rm -f ${ATTACHRUN}
	cd os/${OS} && ${MAKE} $@
