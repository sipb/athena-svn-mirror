## $Id: Makefile.am,v 1.2 2005-02-01 20:20:40 ghudson Exp $
##
## Copyright (c) 2003  Noah Levitt
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
##

AM_CFLAGS = $(GTK_CFLAGS) -Wall

noinst_LIBRARIES = libkbdraw.a
libkbdraw_a_SOURCES = keyboard-marshal.c keyboard-marshal.h \
                       keyboard-drawing.c keyboard-drawing.h

keyboard-marshal.h: keyboard-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=keyboard_marshal keyboard-marshal.list \
                            --header > keyboard-marshal.tmp \
          && mv keyboard-marshal.tmp keyboard-marshal.h ) \
        || ( rm -f keyboard-marshal.tmp && exit 1 )
 
keyboard-marshal.c: keyboard-marshal.h
	( @GLIB_GENMARSHAL@ --prefix=keyboard_marshal keyboard-marshal.list \
                            --body > keyboard-marshal.tmp \
          && mv keyboard-marshal.tmp keyboard-marshal.c ) \
        || ( rm -f keyboard-marshal.tmp && exit 1 )

EXTRA_DIST = keyboard-marshal.list

