# Makefile for libversit.a

# $Id: Makefile.am,v 1.1.1.3 2003-07-13 04:51:12 ghudson Exp $

VERSIT_SRC =		\
	vcc.y		\
	vcc.h		\
	vobject.c	\
	vobject.h	\
	port.h		\
	vcaltmp.c	\
	vcaltmp.h

privlib_LTLIBRARIES = libversit.la

libversit_la_SOURCES = $(VERSIT_SRC)

EXTRA_DIST = README.TXT vcaltest.c vctest.c

YFLAGS=-pversit_
