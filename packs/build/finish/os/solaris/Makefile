# $Id: Makefile,v 1.6 1999-03-30 16:09:00 ghudson Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHRBINDIR=/bin/athena
ATHRETCDIR=/etc/athena
ATHLIBDIR=/usr/athena/lib
ATHMANDIR=/usr/athena/man

all:

check:

# The for loop here detects when a file tracked onto local disk is an
# unstripped binary.  If so, it updates the .unstripped version and
# strips the file.
install:
	for f in ${DESTDIR}${ATHRBINDIR}/* ${DESTDIR}${ATHRETCDIR}/*; do \
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
