################################################################################
#
# Makefile  : Web2C / doc
# Author    : Fabrice Popineau <Fabrice.Popineau@supelec.fr>
# Platform  : Win32, Microsoft VC++ 6.0, depends upon fpTeX 0.5 sources
# Time-stamp: <02/03/04 00:02:58 popineau>
#
################################################################################
root_srcdir = ..\..\..
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

!include <msvc/common.mak>

infofiles = web2c.info
pdfdocfiles = web2c.pdf
manfiles =
htmldocfiles = web2c.html

docsubdir = web2c

all: doc

!include <msvc/config.mak>
!include <msvc/install.mak>

install:: install-info install-doc

web2c.info: install.texi ref.txi

!include <msvc/clean.mak>

#
# Local Variables:
# mode: makefile
# End:

