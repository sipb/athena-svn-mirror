################################################################################
#
# Makefile  : LibWWW
# Author    : Fabrice Popineau <Fabrice.Popineau@supelec.fr>
# Platform  : Win32, Microsoft VC++ 6.0, depends upon fpTeX 0.5 sources
# Time-stamp: <02/09/23 17:40:52 popineau>
#
################################################################################
root_srcdir = ..\..
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

USE_GNUW32 = 1
USE_WWW = 1
USE_REGEX = 1
USE_ZLIB = 1
USE_ADVAPI = 1

!include <msvc/common.mak>

programs = $(wwwdll)
libfiles = $(wwwlib)
includefiles = 
installdirs = $(bindir) $(includedir) $(libdir)

objects = \
	$(objdir)\HTAccess.obj \
	$(objdir)\HTAnsi.obj \
	$(objdir)\HTAssoc.obj \
	$(objdir)\HTAtom.obj \
	$(objdir)\HTBind.obj \
	$(objdir)\HTDNS.obj \
	$(objdir)\HTEscape.obj \
	$(objdir)\HTFTP.obj \
	$(objdir)\HTGopher.obj \
	$(objdir)\HTGuess.obj \
	$(objdir)\HTProxy.obj \
	$(objdir)\HTReader.obj \
	$(objdir)\HTReqMan.obj \
	$(objdir)\HTTPGen.obj \
	$(objdir)\HTTeXGen.obj \
	$(objdir)\HTUTree.obj \
	$(objdir)\HTAABrow.obj \
	$(objdir)\HTAAUtil.obj \
	$(objdir)\HTAlert.obj \
	$(objdir)\HTAnchor.obj \
	$(objdir)\HTArray.obj \
	$(objdir)\HTBInit.obj \
	$(objdir)\HTBound.obj \
	$(objdir)\HTBufWrt.obj \
	$(objdir)\HTCache.obj \
	$(objdir)\HTChannl.obj \
	$(objdir)\HTChunk.obj \
	$(objdir)\HTConLen.obj \
	$(objdir)\HTDialog.obj \
	$(objdir)\HTDigest.obj \
	$(objdir)\HTDir.obj \
	$(objdir)\HTError.obj \
	$(objdir)\HTEvent.obj \
	$(objdir)\HTEvtLst.obj \
	$(objdir)\HTFSave.obj \
	$(objdir)\HTFTPDir.obj \
	$(objdir)\HTFWrite.obj \
	$(objdir)\HTFile.obj \
	$(objdir)\HTFilter.obj \
	$(objdir)\HTFormat.obj \
	$(objdir)\HTHInit.obj \
	$(objdir)\HTHeader.obj \
	$(objdir)\HTHome.obj \
	$(objdir)\HTHost.obj \
	$(objdir)\HTIcons.obj \
	$(objdir)\HTInet.obj \
	$(objdir)\HTInit.obj \
	$(objdir)\HTLib.obj \
	$(objdir)\HTLink.obj \
	$(objdir)\HTList.obj \
	$(objdir)\HTLocal.obj \
	$(objdir)\HTLog.obj \
	$(objdir)\HTMIME.obj \
	$(objdir)\HTMIMERq.obj \
	$(objdir)\HTMIMImp.obj \
	$(objdir)\HTMIMPrs.obj \
	$(objdir)\HTML.obj \
	$(objdir)\HTMLGen.obj \
	$(objdir)\HTMLPDTD.obj \
	$(objdir)\HTMemory.obj \
	$(objdir)\HTMerge.obj \
	$(objdir)\HTMethod.obj \
	$(objdir)\HTMulti.obj \
	$(objdir)\HTNDir.obj \
	$(objdir)\HTNet.obj \
	$(objdir)\HTNews.obj \
	$(objdir)\HTNewsLs.obj \
	$(objdir)\HTNewsRq.obj \
	$(objdir)\HTNoFree.obj \
	$(objdir)\HTPEP.obj \
	$(objdir)\HTParse.obj \
	$(objdir)\HTPlain.obj \
	$(objdir)\HTProfil.obj \
	$(objdir)\HTProt.obj \
	$(objdir)\HTResponse.obj \
	$(objdir)\HTRules.obj \
	$(objdir)\HTSChunk.obj \
	$(objdir)\HTStream.obj \
	$(objdir)\HTString.obj \
	$(objdir)\HTTCP.obj \
	$(objdir)\HTTChunk.obj \
	$(objdir)\HTTP.obj \
	$(objdir)\HTTPReq.obj \
	$(objdir)\HTTee.obj \
	$(objdir)\HTTelnet.obj \
	$(objdir)\HTTimer.obj \
	$(objdir)\HTTrace.obj \
	$(objdir)\HTTrans.obj \
	$(objdir)\HTUU.obj \
	$(objdir)\HTUser.obj \
	$(objdir)\HTWWWStr.obj \
	$(objdir)\HTWriter.obj \
	$(objdir)\HText.obj \
	$(objdir)\HTZip.obj \
	$(objdir)\SGML.obj \
	$(objdir)\md5.obj \
	$(NULL)

default: all

all: $(objdir) $(www)

lib: $(objdir) $(wwwlib)

!ifdef WWW_DLL
DEFS = $(DEFS) -DMAKE_WWW_DLL

$(wwwlib): $(objdir) $(objects)
	$(archive) /DEF:libwww.def $(objects)

$(wwwdll): $(objects) $(objdir)\libwww.res $(gnuw32lib) $(regexlib) $(zliblib)
	$(link_dll) $(**) $(wwwlib:.lib=.exp) wsock32.lib $(conlibs)
!else
$(wwwlib): $(objdir) $(objects) $(gnuw32lib) $(regexlib) $(zliblib)
	$(archive) $(objects) $(gnuw32lib) $(regexlib) $(zliblib) wsock32.lib
!ENDIF

!include <msvc/config.mak>

!include <msvc/install.mak>

install:: install-exec install-include install-lib

!include <msvc/clean.mak>

!include <msvc/rdepend.mak>
!include "./depend.mak"

# 
# Local Variables:
# mode: makefile
# End:
