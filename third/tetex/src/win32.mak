#
# Top Level Makefile for TeXLive for Win32
#
root_srcdir = .
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

!include <msvc/common.mak>

# Kpathsea needs to be build before 
!ifdef DEVELOPMENT
subdirs = libs.development    \
	  utils.development   \
	  texk.development
!else
subdirs = libs                \
	  utils		      \
	  texk
!endif

!include <msvc/subdirs.mak>
!include <msvc/clean.mak>

#
# Local Variables:
# mode: makefile
# End:
