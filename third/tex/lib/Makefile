# $Id: Makefile,v 1.1 1996-10-25 22:07:20 ghudson Exp $

SHELL=/bin/sh
ATHLIBDIR=/usr/athena/lib
TYPEFACES=/usr/athena/lib/mf/typefaces
MFMACROS=/usr/athena/lib/mf/macros
TEXMACROS=/usr/athena/lib/tex/macros

all:
	cd tex/fonts && ${MAKE} $@

check:
	cd tex/fonts && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${MFMACROS}
	-mkdir -p ${DESTDIR}${TYPEFACES}/ams
	-mkdir -p ${DESTDIR}${TYPEFACES}/cm
	-mkdir -p ${DESTDIR}${TYPEFACES}/cm-ams
	-mkdir -p ${DESTDIR}${TYPEFACES}/cyrillic
	-mkdir -p ${DESTDIR}${TYPEFACES}/euler
	-mkdir -p ${DESTDIR}${TYPEFACES}/latex
	-mkdir -p ${DESTDIR}${TYPEFACES}/ocr
	-mkdir -p ${DESTDIR}${TYPEFACES}/other
	-mkdir -p ${DESTDIR}${TYPEFACES}/tolkien
	-mkdir -p ${DESTDIR}${TEXMACROS}/amslatex
	-mkdir -p ${DESTDIR}${TEXMACROS}/amslatex/fontsel
	-mkdir -p ${DESTDIR}${TEXMACROS}/amslatex/inputs
	-mkdir -p ${DESTDIR}${TEXMACROS}/bib
	-mkdir -p ${DESTDIR}${TEXMACROS}/latex
	-mkdir -p ${DESTDIR}${TEXMACROS}/other/mainz/doc
	-mkdir -p ${DESTDIR}${TEXMACROS}/other/mainz/multicol
	install -c -m 444 mf/macros/*.mf* ${DESTDIR}${MFMACROS}
	install -c -m 444 mf/typefaces/ams/*.mf ${DESTDIR}${TYPEFACES}/ams
	install -c -m 444 mf/typefaces/cm/*.mf ${DESTDIR}${TYPEFACES}/cm
	install -c -m 444 mf/typefaces/cm-ams/*.mf \
		${DESTDIR}${TYPEFACES}/cm-ams
	install -c -m 444 mf/typefaces/cyrillic/*.mf \
		${DESTDIR}${TYPEFACES}/cyrillic
	install -c -m 444 mf/typefaces/euler/*.mf ${DESTDIR}${TYPEFACES}/euler
	install -c -m 444 mf/typefaces/latex/*.mf ${DESTDIR}${TYPEFACES}/latex
	install -c -m 444 mf/typefaces/ocr/*.mf ${DESTDIR}${TYPEFACES}/ocr
	install -c -m 444 mf/typefaces/other/*.mf ${DESTDIR}${TYPEFACES}/other
	install -c -m 444 mf/typefaces/tolkien/*.mf \
		${DESTDIR}${TYPEFACES}/tolkien
	install -c -m 444 tex/macros/*.* ${DESTDIR}${TEXMACROS}
	install -c -m 444 tex/macros/amslatex/fontsel/*.* \
		${DESTDIR}${TEXMACROS}/amslatex/fontsel
	install -c -m 444 tex/macros/amslatex/inputs/*.* \
		${DESTDIR}${TEXMACROS}/amslatex/inputs
	install -c -m 444 tex/macros/bib/*.* ${DESTDIR}${TEXMACROS}/bib
	install -c -m 444 tex/macros/latex/*.* ${DESTDIR}${TEXMACROS}/latex
	install -c -m 444 tex/macros/other/*.* ${DESTDIR}${TEXMACROS}/other
	install -c -m 444 tex/macros/other/mainz/doc/*.* \
		${DESTDIR}${TEXMACROS}/other/mainz/doc
	install -c -m 444 tex/macros/other/mainz/multicol/*.* \
		${DESTDIR}${TEXMACROS}/other/mainz/multicol
	cd tex/fonts && ${MAKE} $@

clean:
	cd tex/fonts && ${MAKE} $@

distclean:
	cd tex/fonts && ${MAKE} $@
