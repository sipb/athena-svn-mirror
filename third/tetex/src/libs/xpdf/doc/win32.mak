################################################################################
#
# Makefile  : XPdf/doc subdirectory
# Author    : Fabrice Popineau <Fabrice.Popineau@supelec.fr>
# Platform  : Win32, Microsoft VC++ 6.0, depends upon fpTeX 0.5 sources
# Time-stamp: <02/01/28 16:32:16 popineau>
#
################################################################################
root_srcdir = ..\..\..
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

!include <msvc/common.mak>

manfiles = pdfimages.1 pdfinfo.1 pdftops.1 pdftotext.1
#	 pdftopbm.1

all:

!include <msvc/config.mak>
!include <msvc/install.mak>

depend:

!include <msvc/clean.mak>

#
# Local Variables:
# mode: makefile
# End:

