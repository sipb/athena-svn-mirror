# $Id: Makefile,v 1.8 2000-04-28 14:33:27 ghudson Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHRBINDIR=/bin/athena
ATHRETCDIR=/etc/athena
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
	catman -M ${DESTDIR}${ATHMANDIR}

clean:

distclean:
