# $Id: Makefile,v 1.3 1999-03-21 21:01:46 ghudson Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHLIBDIR=/usr/athena/lib
ATHMANDIR=/usr/athena/man
STRIPFILES=/etc/athena/console /bin/athena/attach /bin/athena/attachandrun \
	/bin/athena/detach /bin/athena/fsid /bin/athena/zinit \
	/bin/athena/tcsh /etc/athena/xlogin /etc/athena/named \
	/etc/athena/named-xfer /etc/athena/sshd /etc/athena/xntpd

all:

check:

# The for loop here detects when a file in ${STRIPFILES} is newer than
# its .unstripped version.  If so, it updates the .unstripped version,
# strips the file itself, and touches the .unstripped version to make
# sure that the .unstripped version is now at least as new.
install:
	for file in ${STRIPFILES}; do \
		f=${DESTDIR}$$file; \
		u=$$f.unstripped; \
		if [ ! -f $$u -o "`find $$f -newer $$u 2>&1`" = $$f ]; then \
			rm -f $$u; \
			cp -p $$f $$u || exit 1; \
			strip $$f; \
			touch $$u; \
		fi; \
	done
	mkdir -p ${DESTDIR}${ATHLIBDIR}/stats
	${DESTDIR}${ATHETCDIR}/track -w -F/os -W${DESTDIR}${ATHLIBDIR} \
		-s stats/os_rvd slists/os_rvd
	${DESTDIR}${ATHETCDIR}/track -w -F/os/kernel -W${DESTDIR}${ATHLIBDIR} \
		-s stats/kernel_rvd slists/kernel_rvd
	${DESTDIR}${ATHETCDIR}/track -w -F/os/usr/kernel \
		-W${DESTDIR}${ATHLIBDIR} -s stats/usr_kernel_rvd \
		slists/usr_kernel_rvd
	catman -M ${DESTDIR}${ATHMANDIR}

clean:

distclean:
