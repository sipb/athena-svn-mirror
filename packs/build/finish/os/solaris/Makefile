# $Id: Makefile,v 1.7 1999-04-12 17:08:56 danw Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHRBINDIR=/bin/athena
ATHRETCDIR=/etc/athena
ATHLIBDIR=/usr/athena/lib
ATHMANDIR=/usr/athena/man

STRIPFILES=${DESTDIR}${ATHRBINDIR}/* ${DESTDIR}${ATHRETCDIR}/* \
	${DESTDIR}/usr/lib/sendmail

all:

check:

# The for loop here detects when a file tracked onto local disk is an
# unstripped binary.  If so, it updates the .unstripped version and
# strips the file.
install:
	for f in ${STRIPFILES}; do \
		case $$f in \
		*.unstripped) \
			;; \
		*) \
			u=$$f.unstripped; \
			case "`/usr/bin/file -h $$f`" in \
			*"not stripped") \
				rm -f $$u; \
				cp -p $$f $$u || exit 1; \
				chmod u+w $$f; \
				strip $$f || exit 1; \
				chmod u-w $$f; \
				;; \
			esac \
			;; \
		esac \
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
