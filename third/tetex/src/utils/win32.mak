################################################################################
#
# Makefile  : fpTeX utils subdirectory
# Author    : Fabrice Popineau <Fabrice.Popineau@supelec.fr>
# Platform  : Win32, Microsoft VC++ 6.0, depends upon fpTeX 0.5 sources
# Time-stamp: <02/08/05 00:47:19 popineau>
#
################################################################################
root_srcdir = ..
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

!include <msvc/common.mak>

# Package subdirectories, the library, and all subdirectories.
subdirs = \
	win32-cmd       \
	bzip2		\
	gzip		\
	jpeg2ps 	\
#	libpng-contrib	\
#	netpbm 		\
	psutils		\
	t1utils		\
	texinfo		\
#	tiff-prog	\
#	tiff2png	\
#	bmp2png		\
#	libwmf		\
	mminstance      \
	vlna		\
	TeXSetup	\
	TeXLive

!include <msvc/subdirs.mak>
!include <msvc/clean.mak>

#
# Local Variables:
# mode: Makefile
# End:
# DO NOT DELETE
