################################################################################
#
# Makefile  : Web2C
# Author    : Fabrice Popineau <Fabrice.Popineau@supelec.fr>
# Platform  : Win32, Microsoft VC++ 6.0, depends upon fpTeX 0.5 sources
# Time-stamp: <03/01/20 21:50:38 popineau>
#
################################################################################
root_srcdir=..\..
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

version = 7.3.2.2

USE_KPATHSEA = 1
USE_GNUW32 = 1
USE_ZLIB = 1
USE_PNG=1
USE_JPEG = 1
# USE_TIFF = 1
USE_XPDF = 1
USE_TEX = 1
MAKE_TEX = 1

!include <msvc/common.mak>

# Compilation options.
DEFS = -I. $(DEFS) -DHAVE_CONFIG_H -DOEM -DJOBTIME -DTIME_STATS \
	-DOUTPUT_DIR -DHALT_ON_ERROR

!ifdef TEX_DLL
DEFS = $(DEFS) -DMAKE_TEX_DLL
!endif

# pdfTeX version
verpdftexdir = pdftexdir
verpdfetexdir = pdfetexdir

# With --enable-ipc, TeX may need to link with -lsocket.
socketlibs = delayimp.lib /delayload:wsock32.dll # @socketlibs@

proglib = lib\$(objdir)\lib.lib
windowlib = window\$(objdir)\window.lib
pdflib = $(verpdftexdir)\$(objdir)\libpdf.lib
pdftoepdflib = $(verpdftexdir)\$(objdir)\libpdftoepdf.lib
pdftexlibs = $(pdflib) $(pdftoepdflib) $(pnglib) $(zliblib) # $(tifflib) 
pdftexlibsdep = $(pdflib) $(pdftoepdflib) $(png) $(zlib) $(xpdf) # $(tiff)

# The .bat script that does the conversion:
web2c = web2c\convert.bat $(objdir)
# Additional dependencies:
web2c_aux = $(win32)/convert.bat web2c/common.defines
web2c_programs = web2c/$(objdir)/fixwrites.exe	\
	web2c/$(objdir)/splitup.exe		\
	web2c/$(objdir)/web2c.exe

# These lines define the memory dumps that fmts/bases/mems will make and
# install-fmts/install-bases/install-mems will install. plain.* is
# created automatically (as a link).  See the Formats node in
# doc/web2c.texi for details on the fmts.
fmts = latex.fmt # amstex.fmt eplain.fmt texinfo.fmt
efmts = elatex.efmt
cfmts = latex.efmt
pdffmts = pdflatex.fmt
pdfefmts = pdfelatex.efmt
ofmts = lambda.fmt
bases = # I do not recommend building cmmf.base.
mems =  # mfplain.mem is probably not generally useful.

# Unfortunately, suffix rules can't have dependencies, or multiple
# targets, and we can't assume all makes supports pattern rules.
.SUFFIXES: .p .c .ch .p .res .rc
.p.c: # really depends on $(web2c_programs), and generates .h.
	 $(web2c) $*
.ch.p: # really depends on ./tangle; for mf/mp/tex, also generates .pool
	.\$(objdir)\tangle $*.web $<

# These definitions have to come before the rules that expand them.
# The *{ini,[0-2]}.c files are created by splitup, run as part of convert.
# {mf,mp,tex}extra.c are created from lib/texmfmp.c, below.
mf_c = mf.c
mf_o = $(objdir)\mf.obj
#mf_nowin_o = $(objdir)\mfnowin.obj
mp_c = mp.c
mp_o = $(objdir)\mp.obj
tex_c = tex.c
tex_o = $(objdir)\tex.obj

other_c = bibtex.c ctangle.c cweave.c cweb.c dvicopy.c dvitomp.c dvitype.c	\
	gftodvi.c gftopk.c gftype.c mft.c  patgen.c pktogf.c pktype.c pltotf.c	\
	pooltype.c  tangle.c tftopl.c vftovp.c vptovf.c weave.c
all_c = $(other_c) $(mf_c) $(mp_c) $(tex_c) $(etex_c) $(pdftex_c) $(omega_c) \
  $(omegaware_c)

# Prevent Make from deleting the intermediate forms.
.PRECIOUS: %.ch %.p %.c

#mf_nowin = mf
mf_win = mf

etex = $(objdir)\etex.exe
#mfn = $(objdir)\$(mf_nowin).exe
mfw = $(objdir)\$(mf_win).exe
mpost = $(objdir)\mpost.exe
odvicopy = $(objdir)\odvicopy.exe
odvitype = $(objdir)\odvitype.exe
omega = $(objdir)\omega.exe
otangle = $(objdir)\otangle.exe
pdfetex = $(objdir)\pdfetex.exe
pdftex = $(objdir)\pdftex.exe
pdftosrc = $(objdir)\pdftosrc.exe
tex = $(objdir)\tex.exe
ttf2afm = $(objdir)\ttf2afm.exe
!ifdef TEX_DLL
etex = $(etex) $(objdir)\$(library_prefix)etex.dll
#mfn = $(mfn) $(objdir)\$(library_prefix)$(mf_nowin).dll
mfw = $(mfw) $(objdir)\$(library_prefix)$(mf_win).dll
mpost = $(mpost) $(objdir)\$(library_prefix)mp.dll
omega = $(omega) $(objdir)\$(library_prefix)omeg.dll
pdfetex = $(pdfetex) $(objdir)\$(library_prefix)pdfe.dll
pdftex = $(pdftex) $(objdir)\$(library_prefix)pdft.dll
tex = $(tex) $(objdir)\$(library_prefix)tex.dll
!endif
programs = $(objdir)\tie.exe	\
	$(objdir)\bibtex.exe	\
	$(objdir)\ctangle.exe	\
	$(objdir)\cweave.exe	\
	$(objdir)\dvicopy.exe	\
	$(objdir)\dvitomp.exe	\
	$(objdir)\dvitype.exe	\
	$(etex)			\
	$(objdir)\gftodvi.exe	\
	$(objdir)\gftopk.exe	\
	$(objdir)\gftype.exe	\
	$(mfn)			\
	$(mfw)			\
	$(objdir)\mft.exe	\
	$(mpost)		\
	$(odvicopy)		\
	$(odvitype)		\
	$(omega)		\
	$(otangle)		\
	$(objdir)\patgen.exe	\
	$(pdfetex)		\
	$(pdftex)		\
	$(objdir)\pktogf.exe	\
	$(objdir)\pktype.exe	\
	$(objdir)\pltotf.exe	\
	$(objdir)\pooltype.exe	\
	$(objdir)\tangle.exe	\
	$(tex)			\
	$(objdir)\tftopl.exe	\
	$(ttf2afm)		\
	$(objdir)\vftovp.exe	\
	$(objdir)\vptovf.exe	\
	$(objdir)\weave.exe	\
	$(pdftosrc)

mpware = mpware\$(objdir)\dmp.exe	\
	mpware\$(objdir)\makempx.exe	\
	mpware\$(objdir)\mpto.exe	\
	mpware\$(objdir)\newer.exe

mpware_sources = mpware\dmp.c mpware\makempx.in mpware\mpto.c mpware\newer.c

otps_programs = otps\$(objdir)\otp2ocp.exe otps\$(objdir)\outocp.exe
omegafonts_programs = omegafonts\$(objdir)\omfonts.exe

#  
default: all

all:: $(objdir) programs # formats # manpages doc/web2c.info

programs: $(programs) $(mpware) $(otps_programs) $(omegafonts_programs)

# Additional dependencies for relinking.
$(web2c_programs) $(programs) $(objdir)\otangle.exe $(objdir)\tangle.exe $(objdir)\tangleboot.exe: $(kpathsealib) $(proglib)

# Makefile fragments:
!include "etexdir/etex.mak"
!include "omegadir/omega.mak"
!include "pdftexdir/pdftex.mak"
!include "pdfetexdir/pdfetex.mak"

!include <msvc/config.mak>

# Rules to link each program. I wish we could use a suffix rule for
# this, since it's so repetitive, but null suffixes don't work. And old
# makes don't support pattern rules. Doomed to forever cater to obsolesence.
$(objdir)\bibtex.exe: $(objdir)\bibtex.obj $(objdir)\bibtex.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\dvicopy.exe: $(objdir)\dvicopy.obj $(objdir)\dvicopy.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\dvitomp.exe: $(objdir)\dvitomp.obj $(objdir)\dvitomp.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\dvitype.exe: $(objdir)\dvitype.obj $(objdir)\dvitype.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

!ifdef TEX_DLL
$(objdir)\$(library_prefix)etex.exp: $(objdir)\$(library_prefix)etex.lib

$(objdir)\$(library_prefix)etex.lib: $(etex_o)
	$(archive) /DEF $(etex_o)

$(objdir)\$(library_prefix)etex.dll: $(etex_o) $(objdir)\$(library_prefix)etex.exp $(objdir)\etex.res $(kpathsealib) $(proglib)
	$(link_dll) $(**) $(socketlibs) $(conlibs)

$(objdir)\etex.exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)etex.lib $(proglib)
	$(link) $(**) $(conlibs)
!else
$(objdir)\etex.exe: $(etex_o) $(objdir)\win32main.obj $(objdir)\etex.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)
!endif

$(objdir)\gftodvi.exe: $(objdir)\gftodvi.obj $(objdir)\gftodvi.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\gftopk.exe: $(objdir)\gftopk.obj $(objdir)\gftopk.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\gftype.exe: $(objdir)\gftype.obj $(objdir)\gftype.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

!ifdef TEX_DLL
$(objdir)\$(library_prefix)$(mf_win).exp: $(objdir)\$(library_prefix)$(mf_win).lib

$(objdir)\$(library_prefix)$(mf_win).lib: $(mf_o)
	$(archive) /DEF $(mf_o)

$(objdir)\$(library_prefix)$(mf_win).dll: $(mf_o) $(mfextra_o) $(objdir)\$(library_prefix)$(mf_win).exp $(objdir)\mf.res $(windowlib) $(kpathsealib) $(proglib)
	$(link_dll) $(**) gdi32.lib $(conlibs)

$(objdir)\$(mf_win).exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)$(mf_win).lib $(proglib)
	$(link) $(**) $(conlibs)
!else
$(objdir)\$(mf_win).exe: $(mf_o) $(objdir)\win32main.obj $(windowlib) $(objdir)\mf.res $(kpathsealib) $(proglib)
	$(link) $(**) gdi32.lib $(conlibs)
!endif

# !ifdef TEX_DLL
# $(objdir)\$(library_prefix)$(mf_nowin).exp: $(objdir)\$(library_prefix)$(mf_nowin).lib

# $(objdir)\$(library_prefix)$(mf_nowin).lib: $(mf_nowin_o) window\$(objdir)\trap.obj
# 	$(archive) /DEF $(**)

# $(objdir)\$(library_prefix)$(mf_nowin).dll: $(mf_nowin_o) window\$(objdir)\trap.obj $(objdir)\$(library_prefix)$(mf_nowin).exp $(objdir)\mf.res $(kpathsealib) $(proglib)
# 	$(link_dll) $(**) $(conlibs)

# $(objdir)\$(mf_nowin).exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)$(mf_nowin).lib $(proglib)
# 	$(link) $(**) $(conlibs)
# !else
# $(objdir)\$(mf_nowin).exe: $(mf_nowin_o) $(objdir)\win32main.obj window\$(objdir)\trap.obj $(objdir)\mf.res $(kpathsealib) $(proglib)
# 	$(link) $(**) $(conlibs)
# !endif

$(objdir)\mft.exe: $(objdir)\mft.obj $(objdir)\mft.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

!ifdef TEX_DLL
$(objdir)\$(library_prefix)mp.exp: $(objdir)\$(library_prefix)mp.lib

$(objdir)\$(library_prefix)mp.lib: $(mp_o)
	$(archive) /DEF $(mp_o)

$(objdir)\$(library_prefix)mp.dll: $(mp_o) $(mpostextra_o) $(objdir)\$(library_prefix)mp.lib $(objdir)\$(library_prefix)mp.exp $(objdir)\mpost.res $(kpathsealib) $(proglib)
	$(link_dll) $(**) $(conlibs)

$(objdir)\mpost.exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)mp.lib $(proglib)
	$(link) $(**) $(conlibs)
!else
$(objdir)\mpost.exe: $(mp_o) $(objdir)\win32main.obj $(objdir)\mpost.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)
!endif

$(objdir)\odvicopy.exe: $(objdir)\odvicopy.obj $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\odvitype.exe: $(objdir)\odvitype.obj $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

!ifdef TEX_DLL
$(ojbdir)\$(library_prefix)omeg.exp: $(objdir)\$(library_prefix)omeg.lib

$(objdir)\$(library_prefix)omeg.lib: $(omega_o)
	$(archive) /DEF $(omega_o)

$(objdir)\$(library_prefix)omeg.dll: $(omega_o) $(omegaextra_o) $(objdir)\$(library_prefix)omeg.exp $(objdir)\omega.res $(omegalibsdep) $(kpathsealib) $(proglib)
	$(link_dll) $(**) $(conlibs)

$(objdir)\omega.exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)omeg.lib $(proglib)
	$(link) $(**) $(socketslib) $(conlibs)
!else
$(objdir)\omega.exe: $(omega_o) $(objdir)\win32main.obj $(kpathsealib) $(proglib)
	$(link) $(**) $(socketlibs) $(conlibs)
!endif

$(objdir)\otangle.exe: $(objdir)\otangle.obj $(objdir)\otangle.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\patgen.exe: $(objdir)\patgen.obj $(objdir)\patgen.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

!ifdef TEX_DLL
$(objdir)\$(library_prefix)pdft.exp: $(objdir)\$(library_prefix)pdft.lib

$(objdir)\$(library_prefix)pdft.lib: $(pdftex_o)
	$(archive) /DEF $(pdftex_o)

$(objdir)\$(library_prefix)pdft.dll: $(pdftex_o) $(pdftexextra_o) $(objdir)\$(library_prefix)pdft.exp $(objdir)\pdftex.res $(pdftexlibs) $(kpathsealib) $(proglib)
	$(link_dll) $(**) $(socketlibs) $(conlibs)

$(objdir)\pdftex.exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)pdft.lib $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)
!else
$(objdir)\pdftex.exe: $(pdftex_o) $(pdftexextra_o) $(objdir)\win32main.obj $(objdir)\pdftex.res $(pdftexlibs) $(kpathsealib) $(proglib)
	$(link) $(**) $(socketlibs) $(conlibs)
!endif

!ifdef TEX_DLL
$(objdir)\$(library_prefix)pdfe.exp: $(objdir)\$(library_prefix)pdfe.lib

$(objdir)\$(library_prefix)pdfe.lib: $(pdfetex_o)
	$(archive) /DEF $(pdfetex_o)

$(objdir)\$(library_prefix)pdfe.dll: $(pdfetex_o) $(pdfetexextra_o) $(objdir)\$(library_prefix)pdfe.exp $(objdir)\pdfetex.res $(pdftexlibs) $(kpathsealib) $(proglib)
	$(link_dll) $(**) $(socketlibs) $(conlibs)

$(objdir)\pdfetex.exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)pdfe.lib $(proglib)
	$(link) $(**) $(conlibs)
!else
$(objdir)\pdfetex.exe: $(pdfetex_o) $(pdfetexextra_o) $(objdir)\win32main.obj $(objdir)\pdfetex.res $(pdftexlibs) $(kpathsealib) $(proglib)
	$(link) $(**) $(socketlibs) $(conlibs)
!endif

$(objdir)\pktogf.exe: $(objdir)\pktogf.obj $(objdir)\pktogf.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\pktype.exe: $(objdir)\pktype.obj $(objdir)\pktype.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\pltotf.exe: $(objdir)\pltotf.obj $(objdir)\pltotf.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\pooltype.exe: $(objdir)\pooltype.obj $(objdir)\pooltype.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

!ifdef TEX_DLL
$(objdir)\$(library_prefix)tex.exp: $(objdir)\$(library_prefix)tex.lib

$(objdir)\$(library_prefix)tex.lib: $(tex_o)
	$(archive) /DEF $(tex_o)

$(objdir)\$(library_prefix)tex.dll: $(tex_o) $(objdir)\$(library_prefix)tex.exp $(objdir)\tex.res $(kpathsealib) $(proglib)
	$(link_dll) $(**) $(socketlibs) $(conlibs)

$(objdir)\tex.exe: $(objdir)\win32main.obj $(objdir)\$(library_prefix)tex.lib $(proglib)
	$(link) $(**) $(conlibs)
!else
$(objdir)\tex.exe: $(tex_o) $(objdir)\win32main.obj $(objdir)\tex.res $(kpathsealib) $(proglib)
	$(link) $(**) $(socketlibs) $(conlibs)
!endif

$(objdir)\tie.exe: $(objdir)\tie.obj $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\tftopl.exe: $(objdir)\tftopl.obj $(objdir)\tftopl.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\vftovp.exe: $(objdir)\vftovp.obj $(objdir)\vftovp.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\vptovf.exe: $(objdir)\vptovf.obj $(objdir)\vptovf.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

$(objdir)\weave.exe: $(objdir)\weave.obj $(objdir)\weave.res $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)

# The stub with main() for win32
$(objdir)\win32main.obj: $(objdir) .\lib\win32main.c config.h
	$(compile) -UMAKE_TEX_DLL .\lib\win32main.c

# We put some programs (written directly in C) in a subdirectory.
$(mpware): $(mpware_sources)
	-@echo $(verbose) & ( \
		pushd mpware & $(make) all & popd \
	)

# Additional dependencies for retangling.
bibtex.p dvicopy.p dvitomp.p dvitype.p etex.p gftopk.p gftodvi.p gftype.p \
  mf.p mft.p mp.p odvicopy.p odvitype.p omega.p patgen.p                  \
  pdfetex.p pdftex.p pktogf.p pktype.p pltotf.p  \
  pooltype.p tex.p tftopl.p vftovp.p vptovf.p weave.p: $(objdir)\tangle.exe
# We need to be explicit for a number of programs because there is a
# $(srcdir) in the suffix rule.  This also means we can use a different
# suffix for the change file than .ch, if we want to.
bibtex.p: bibtex.web bibtex.ch
dvicopy.p: dvicopy.web dvicopy.ch
dvitomp.p: dvitomp.web dvitomp.ch
dvitype.p: dvitype.web dvitype.ch
etex.p etex.pool: etex.web etex.ch
	$(objdir)\tangle etex.web etex.ch
gftodvi.p: gftodvi.web gftodvi.ch
gftopk.p: gftopk.web gftopk.ch
gftype.p: gftype.web gftype.ch
mf.p mf.pool: mf.web mf-w32.ch
	$(objdir)\tangle mf.web mf-w32.ch
mf-w32.ch: $(objdir)\tie.exe mf.ch mf-supp-w32.ch
	$(objdir)\tie.exe -c mf-w32.ch mf.web mf.ch mf-supp-w32.ch
mp.p mp.pool: mp.web mp.ch mp-w32.ch
	$(objdir)\tangle mp.web mp-w32.ch
mp-w32.ch: $(objdir)\tie.exe mp.ch mp-supp-w32.ch
	$(objdir)\tie.exe -c mp-w32.ch mp.web mp.ch mp-supp-w32.ch
mft.p: mft.web mft.ch
odvicopy.p: odvicopy.web odvicopy.ch
	.\$(objdir)\tangle odvicopy.web odvicopy.ch
odvitype.p: odvitype.web odvitype.ch
	.\$(objdir)\tangle odvitype.web odvitype.ch
otangle.p: otangle.web otangle.ch
	.\$(objdir)\tangle otangle.web otangle.ch
omega.p omega.pool: $(objdir)\otangle.exe omega.web omega.ch
	$(objdir)\otangle omega.web omega.ch
patgen.p: patgen.web patgen.ch
pdftex.p pdftex.pool: pdftex.web pdftex.ch
	$(objdir)\tangle pdftex.web pdftex.ch
pdfetex.p pdfetex.pool: pdfetex.web pdfetex.ch
	$(objdir)\tangle pdfetex.web pdfetex.ch
pktogf.p: pktogf.web pktogf.ch
pktype.p: pktype.web pktype.ch
pltotf.p: pltotf.web pltotf.ch
pooltype.p: pooltype.web pooltype.ch
tex.p tex.pool: tex.web tex-w32.ch
	$(objdir)\tangle tex.web tex-w32.ch
tex-w32.ch: $(objdir)\tie.exe tex.ch tex-supp-w32.ch
	$(objdir)\tie.exe -c tex-w32.ch tex.web tex.ch tex-supp-w32.ch
tftopl.p: tftopl.web tftopl.ch
vftovp.p: vftovp.web vftovp.ch
vptovf.p: vptovf.web vptovf.ch
weave.p: weave.web weave.ch

# Additional dependencies for reconverting to C.
$(other_c): $(web2c_aux) $(web2c_programs)
bibtex.c: web2c\cvtbib.sed
c-sources: $(all_c)

# Metafont and TeX generate more than .c file.
web2c_texmf = $(web2c_aux) $(web2c_programs) web2c\texmf.defines
$(etex_c) etexcoerce.h: etex.p $(web2c_texmf)
	$(web2c) etex
$(mf_c) mfcoerce.h: mf.p $(web2c_texmf) web2c\cvtmf1.sed web2c\cvtmf2.sed
	$(web2c) mf
$(mp_c) mpcoerce.h: mp.p $(web2c_texmf) web2c\cvtmf1.sed web2c\cvtmf2.sed
	$(web2c) mp
$(omega_c) omegacoerce.h: omega.p $(web2c_texmf)
	$(web2c) omega
$(pdftex_c) pdftexcoerce.h: pdftex.p $(web2c_texmf)
	$(web2c) pdftex
$(pdfetex_c) pdfetexcoerce.h: pdfetex.p $(web2c_texmf)
	$(web2c) pdfetex
$(tex_c) texcoerce.h.h: tex.p $(web2c_texmf)
	$(web2c) tex

etexd.h: $(etex_c) $(web2c_texmf)
	web2c\$(objdir)\splitup etex < $(etex_c)
mfd.h: $(mf_c) $(web2c_texmf)
	web2c\$(objdir)\splitup mf < $(mf_c)
mpd.h: $(mp_c) $(web2c_texmf)
	web2c\$(objdir)\splitup mp < $(mp_c)
omegad.h: $(omega_c) $(web2c_texmf)
	web2c\$(objdir)\splitup omega < $(omega_c)
pdftexd.h: $(pdftex_c) $(web2c_texmf)
	web2c\$(objdir)\splitup pdftex < $(pdftex_c)
pdfetexd.h: $(pdfetex_c) $(web2c_texmf)
	web2c\$(objdir)\splitup pdfetex < $(pdfetex_c)
texd.h: $(tex_c) $(web2c_texmf)
	web2c\$(objdir)\splitup tex < $(tex_c)

# As long as we have to have separate rules to create these, might as well do
# a little work to avoid separate compilation rules, too.
etexextra.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/etex/ $(srcdir)\lib\texmfmp.c >$@
mfextra.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/mf/ $(srcdir)\lib\texmfmp.c >$@
mfnowin.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/mf/ $(srcdir)\lib\texmfmp.c >$@
mpextra.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/mp/ $(srcdir)\lib\texmfmp.c >$@
omegaextra.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/omega/ $(srcdir)\lib\texmfmp.c >$@
pdftexextra.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/pdftex/ $(srcdir)\lib\texmfmp.c >$@
pdfetexextra.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/pdfetex/ $(srcdir)\lib\texmfmp.c >$@
texextra.c: lib\texmfmp.c
	$(sed) s/TEX-OR-MF-OR-MP/tex/ $(srcdir)\lib\texmfmp.c >$@

# Special rules for cweb
cweave.c: $(objdir)\ctangle.exe cwebdir\cweave.w cwebdir\cweav-w2c.ch
	-$(del) $@
	set CWEBINPUTS=.;$(srcdir)/cwebdir
	$(objdir)\ctangle cweave.w cweav-w2c.ch

# A special rule for tie
tie.c: $(objdir)\ctangle.exe tiedir\tie.w tiedir\tie-w2c.ch
	-@$(del) -f $@
	$(objdir)\ctangle.exe $(srcdir)\tiedir\tie.w $(srcdir)\tiedir\tie-w2c.ch
	-@$(move) $(srcdir)\tiedir\tie.c $(srcdir)

# Dependencies for recompiling the C code, needed for i386 based systems.
$(objdir)\mfextra.obj $(objdir)\mpextra.obj: lib\mfmpw32.c

$(objdir)\mfnowin.obj: mfnowin.c lib\mfmpw32.c
	$(compile) -DMFNOWIN mfnowin.c

# Additional dependencies for recompiling the C code are generated
# automatically, included at the end.

# 
# Bootstrapping tangle requires making it with itself.  We use the opportunity
# to create an up-to-date tangleboot as well.
$(objdir)\tangle.exe: $(objdir)\tangle.obj $(objdir)\tangle.res
	$(link) $(objdir)\tangle.obj $(objdir)\tangle.res $(kpathsealib) $(proglib) $(conlibs)
	$(make) tangleboot.p
# tangle.p is a special case, since it is needed to compile itself.  We
# convert and compile the (distributed) tangleboot.p to make a tangle
# which we use to make the other programs.
tangle.p: $(objdir)\tangleboot.exe tangle.web tangle.ch
	.\$(objdir)\tangleboot $(srcdir)\tangle.web $(srcdir)\tangle.ch
tangle.web:
	@echo "You seem to be missing tangle.web, perhaps because you" >&2
	@echo "didn't retrieve web.tar.gz, only web2c.tar.gz." >&2
	@echo "You need both." >&2
	@echo >&2
	@echo "web.tar.gz should be available from the" >&2
	@echo "same place that you got web2c.tar.gz." >&2
	@echo "In any case, you can get it from" >&2
	@echo "ftp://ftp.tug.org/tex/web.tar.gz." >&2
	false

$(objdir)\tangleboot.exe: $(objdir) $(objdir)\tangleboot.obj
	$(link) $(objdir)\tangleboot.obj $(kpathsealib) $(proglib) $(conlibs)
tangleboot.c tangleboot.h: stamp-tangle $(web2c_programs) $(web2c_aux)
	$(web2c) tangleboot
# tangleboot.p is in the distribution
stamp-tangle: tangleboot.p
	date /t >stamp-tangle & time /t >>stamp-tangle
# This is not run unless tangle.web or tangle.ch is changed.
tangleboot.p: tangle.web tangle.ch
	tangle $(srcdir)\tangle.web $(srcdir)\tangle.ch
	copy tangle.p tangleboot.p & $(del) tangle.p
	date /t >stamp-tangle & time /t >>stamp-tangle
	$(make) $(objdir)\tangle.exe

# Bootstrapping ctangle requires making it with itself.  We use the opportunity
# to create an up-to-date ctangleboot as well.
$(objdir)\ctangle.exe: $(objdir)\ctangle.obj $(objdir)\cweb.obj
	$(link) $(**) $(conlibs)
	$(make) ctangleboot.c 
	$(make) cwebboot.c
# ctangle.c is a special case, since it is needed to compile itself.
ctangle.c: $(objdir)\ctangleboot.exe cwebdir\ctangle.w cwebdir\ctang-w2c.ch
	set CWEBINPUTS=.;$(srcdir)/cwebdir 
	$(objdir)\ctangleboot ctangle.w ctang-w2c.ch
cweb.c: $(objdir)\ctangleboot.exe cwebdir\common.w cwebdir\comm-w2c.ch
	set CWEBINPUTS=.;$(srcdir)\cwebdir 
	$(objdir)\ctangleboot common.w comm-w2c.ch cweb.c

$(objdir)\ctangleboot.exe: $(objdir)\ctangleboot.obj $(objdir)\cwebboot.obj $(kpathsealib) $(proglib)
	$(link) $(**) $(conlibs)
# ctangleboot.c is in the distribution
stamp-ctangle: ctangleboot.c cwebboot.c
# This is not run unless tangle.web or tangle.ch is changed.
# Only try to run ./tangle if it actually exists, otherwise
# just touch tangleboot.p and build tangle.
ctangleboot.c: cwebdir\ctangle.w cwebdir\ctang-w2c.ch
	set CWEBINPUTS=.;$(srcdir)\cwebdir 
	if exist $(objdir)\ctangle.exe ( \
	  .\$(objdir)\ctangle ctangle.w ctang-w2c.ch \
	  & copy ctangle.c ctangleboot.c \
	  & del ctangle.c \
	) else ( \
	  touch ctangleboot.c \
	)
	date /t >stamp-ctangle & time /t >>stamp-ctangle
	$(make) $(objdir)\ctangle.exe
cwebboot.c: cwebdir\common.w cwebdir\comm-w2c.ch
	set CWEBINPUTS=.;$(srcdir)\cwebdir 
	if exist $(objdir)\ctangle.exe ( \
	  $(objdir)\ctangle common.w comm-w2c.ch cweb.c \
	  & copy cweb.c cwebboot.c \
	  & del cweb.c \
	) else ( \
	  touch cwebboot.c \
	)
	date /t >stamp-ctangle & time /t >>stamp-ctangle
	$(make) $(objdir)\ctangle.exe

# Even web2c itself uses the library.
# It's annoying to have to give all the filenames here, 
# but texmfmp.c is an exception.
lib_sources = lib\alloca.c lib\basechsuffix.c lib\chartostring.c \
  lib\eofeoln.c lib\fprintreal.c lib\input2int.c lib\inputint.c lib\main.c \
  lib\openclose.c lib\printversion.c lib\uexit.c lib\usage.c lib\version.c \
  lib\zround.c 
$(proglib): $(lib_sources) c-auto.h # stamp-auto
	pushd lib & $(make) & popd

# No exceptions in this library.
window_sources = $(srcdir)\window\*.c
$(windowlib): mfd.h $(window_sources)
	-@echo $(verbose) & ( \
		pushd window & $(make) all & popd \
	)
window\$(objdir)\trap.obj: $(srcdir)\window\trap.c
	-@echo $(verbose) & ( \
		pushd window & $(make) all & popd \
	)
pdflib_sources = $(srcdir)\$(verpdftexdir)\*.c
$(pdflib): $(pdflib_sources)
	-@echo $(verbose) & ( \
		pushd $(verpdftexdir) & $(make) all & popd \
	)
pdftoepdflib_sources = $(verpdftexdir)\pdftoepdf.cc $(verpdftexdir)\epdf.h \
# 	 $(xpdfdir)\xpdf\*.c* $(xpdfdir)\xpdf\*.h \
# 	 $(xpdfdir)\goo\*.c* $(xpdfdir)\goo\*.h

$(pdftoepdflib): $(pdftoepdflib_sources) $(xpdflib)
        pushd $(verpdftexdir) & $(make) all & popd
# pnglib_sources = $(pngdir)\*.c $(pngdir)\libpng.def
# $(pnglib): $(pnglib_sources)
# 	cd $(pngdir) & $(make) all
# tifflib_sources = $(tiffdir)\*.c $(tiffdir)\libtiff.def
# $(tifflib): $(tifflib_sources)
# 	cd $(TIFFLIBDIR) & $(make) all
# zlib_sources = $(zlibdir)\*.c $(zlibdir)\zlib.def
# $(zlib): $(zlib_sources)
# 	cd $(ZLIBDIR) & $(make) all
#$(xpdflib): $(XPDFDIR)\*.c*
#	-(RM) $(XPDFDIR)\pdftoepdf.cc $(XPDFDIR)\epdf.h
#	$(copy) $(srcdir)\pdftexdir\epdf.h $(XPDFDIR)
#	$(copy) $(srcdir)\pdftexdir)\pdftoepdf.cc $(XPDFDIR)
#	pushd $(XPDFDIR) & $(make) $(xpdflib) & popd
#$(xpdfextralib): $(XPDFEXTRADIR)\*.c*
#	pushd $(XPDFEXTRADIR) & $(make) $(xpdfextralib) & popd

# The web2c program consists of several executables.
$(web2c_programs): c-auto.h
	-@echo $(verbose) & ( \
		pushd web2c & $(make) all & popd \
	)

installdirs =  $(fmtdir) $(basedir) $(memdir) \
	$(texpooldir) $(mfpooldir) $(mppooldir) \
	$(web2cdir) $(fontnamedir)

!include <msvc/install.mak>

# 
# Making formats and bases.
all_fmts = tex.fmt $(fmts)
all_efmts = etex.efmt $(efmts)
all_cfmts = tex.efmt $(cfmts)
all_ofmts = # omega.fmt $(ofmts)
all_pdffmts = pdftex.fmt $(pdffmts)
all_pdfefmts = pdfetex.efmt $(pdfefmts)
all_bases = mf.base $(bases)
all_mems = mpost.mem $(mems)

# Win32 Only. Stamp formats, to not rebuild them every time.
# Does not work. Instead, set dummy vars before running nmake
# and rely on vars defines in make/w2cwin32.mk.
# dumpenv = # set TEXMFCNF=../kpathsea & set TEXMF=$(texmf)

# formats: fmts efmts cfmts ofmts pdffmts pdfefmts bases mems
fmts: $(all_fmts)
efmts: $(all_efmts)
cfmts: $(all_cfmts)
ofmts: $(all_ofmts)
pdffmts: $(all_pdffmts)
pdfefmts: $(all_pdfefmts)
bases: $(all_bases)
mems: $(all_mems)

# tex.fmt: $(objdir)\tex.exe
# 	$(make) $(makeargs) files="--progname=tex plain.tex cmr10.tfm" prereq-check
# 	.\$(objdir)\tex --fmt=tex --ini "\input plain \dump" < nul

# latex.fmt: $(objdir)\tex.exe
# 	$(make) $(makeargs) files="--progname=latex latex.ltx" prereq-check
# 	.\$(objdir)\tex --progname=latex --ini "\input latex.ltx" < nul

# etex.efmt: $(objdir)\etex.exe
# 	$(make) files="--progname=etex etex.src plain.tex cmr10.tfm" prereq-check
# 	.\$(objdir)\etex --efmt=etex --ini "*\input etex.src \dump" <nul

# elatex.efmt: $(objdir)\etex.exe
# 	$(make) files="--progname=elatex latex.ltx" prereq-check
# 	.\$(objdir)\etex --efmt=elatex --ini "*\input latex.ltx" <nul

# tex.efmt: $(objdir)\etex.exe
# 	$(make) files="--progname=tex plain.tex cmr10.tfm" prereq-check
# 	.\$(objdir)\etex --efmt=tex --ini "\input plain \dump" <nul

# latex.efmt: $(objdir)\etex.exe
# 	$(make) files="--progname=latex latex.ltx" prereq-check
# 	.\$(objdir)\etex --progname=latex --ini "\input latex.ltx" <nul

# omega.fmt: $(objdir)\omega.exe
# 	$(make) files="--progname=omega omega.tex" prereq-check
# 	.\$(objdir)\omega --ini "\input omega.tex \dump" <nul

# lambda.fmt: $(objdir)\omega.exe
# 	$(make) files="--progname=lambda lambda.tex" prereq-check
# 	.\$(objdir)\omega --ini --progname=lambda "\input lambda.tex" <nul

# pdftex.fmt: $(objdir)\pdftex.exe
# 	$(make) files="--progname=pdftex plain.tex cmr10.tfm" prereq-check
# 	.\$(objdir)\pdftex --fmt=pdftex --ini "\pdfoutput=1 \input plain \dump" <nul

# pdflatex.fmt: $(objdir)\pdftex.exe
# 	$(make) files="--progname=pdflatex latex.ltx" prereq-check
# 	.\$(objdir)\pdftex --fmt=pdflatex --ini "\pdfoutput=1 \input latex.ltx" <nul

# pdftexinfo.fmt: $(objdir)\pdftex.exe
# 	$(make) files="--progname=pdftexinfo pdftexinfo.ini" prereq-check
# 	.\$(objdir)\pdftex --progname=pdftexinfo --ini pdftexinfo.ini <nul

# pdfetex.efmt: $(objdir)\pdfetex.exe
# 	$(make) files="--progname=pdfetex etex.src plain.tex cmr10.tfm" prereq-check
# 	.\$(objdir)\pdfetex --efmt=pdfetex --ini "*\input etex.src \dump" <nul

# pdfelatex.efmt: $(objdir)\pdfetex.exe
# 	$(make) files="--progname=pdfelatex latex.ltx" prereq-check
# 	.\$(objdir)\pdfetex --efmt=pdfelatex --ini "*\input latex.ltx" <nul

# mltex.fmt: $(objdir)\tex.exe
# 	$(make) files="--progname=mltex plain.tex cmr10.tfm" prereq-check
# 	.\$(objdir)\tex --mltex --fmt=mltex --ini "\input plain \dump" <nul

# mllatex.fmt: $(objdir)\tex.exe
# 	$(make) files="--progname=mllatex latex.ltx" prereq-check
# 	.\$(objdir)\tex --mltex --fmt=mllatex --ini \input latex.ltx <nul

# mf.base: $(objdir)\mf.exe
# 	$(make) $(makeargs) files="plain.mf cmr10.mf $(localmodes).mf" prereq-check
# 	.\$(objdir)\mf --base=mf --ini "\input plain input $(localmodes) dump" < nul

# mpost.mem: $(objdir)\mpost.exe
# 	$(make) $(makeargs) files=plain.mp prereq-check
# 	.\$(objdir)\mpost --mem=mpost --ini "\input plain dump" < nul

# # This is meant to be called recursively, with $(files) set.
# prereq-check: $(kpathseadir)\$(objdir)\kpsewhich.exe
# 	-$(kpathseadir)\$(objdir)\kpsewhich $(files) > nul
# 	if ERRORLEVEL 1 $(make) prereq-lose

# prereq-lose:
# 	@echo "You seem to be missing input files necessary to make the" >&2
# 	@echo "basic formats (some or all of: $(files))." >&2
# 	@echo "Perhaps you've defined the default paths incorrectly, or" >&2
# 	@echo "perhaps you have environment variables set pointing" >&2
# 	@echo "to an incorrect location.  See ../kpathsea/BUGS." >&2
# 	@echo >&2
# 	@echo "If you simply do not have the files, you can" >&2
# 	@echo "retrieve a minimal set of input files from" >&2
# 	@echo "ftp://ftp.tug.org/tex/texklib.tar.gz, mirrored on" >&2
# 	@echo "CTAN hosts in systems/web2c." >&2
# 	false

# $(kpathseadir)\$(objdir)\kpsewhich.exe:
# 	-@echo $(verbose) & ( \
# 		for %%d in ($(kpathseadir)) do \
# 			echo Entering %%d for all \
# 			& pushd %%d & $(make) all & popd \
# 	)

# amstex.fmt: $(objdir)\tex.exe
# 	.\$(objdir)\tex --progname=amstex --ini amstex.ini <nul

# # Texinfo changes the escape character from `\' to `@'.
# texinfo.fmt: tex.fmt
# 	.\$(objdir)\tex --progname=texinfo --ini texinfo @dump <nul

# eplain.fmt: tex.fmt
# 	$(TOUCH) eplain.aux # Makes cross-reference warnings work right.
# 	.\$(objdir)\tex --progname=eplain --ini &./tex eplain \dump <nul

# 
install:: install-exec install-data install-aux install-doc install-man
# The actual binary executables and pool files.
install-exec:: $(programs)
	-@echo $(verbose) & ( \
		for %d in (mpware $(verpdftexdir) otps omegafonts) do \
			pushd %d & $(make) install-exec & popd \
	)
install-exec:: install-links
# install-data:: install-formats

# #Correct this one for pdftex, etex, omega.
# # The links to {mf,mp,tex} for each format and for {ini,vir}{mf,mp,tex}.
install-links:
# TeX
	pushd $(bindir) \
	& $(del) .\initex.exe .\virtex.exe \
	& $(lnexe) .\tex.exe $(bindir)\initex.exe \
	& $(lnexe) .\tex.exe $(bindir)\virtex.exe \
	& popd
# e-TeX
	pushd $(bindir) \
	& $(del) .\einitex.exe .\evirtex.exe \
	& $(lnexe) .\etex.exe $(bindir)\einitex.exe \
	& $(lnexe) .\etex.exe $(bindir)\evirtex.exe \
	& popd
# e-TeX in compatible mode
!ifdef eTeX_as_TeX
	pushd $(bindir) \
	& $(del) .\tex.exe .\initex.exe .\virtex.exe \
	& $(lnexe) .\etex.exe $(bindir)\tex.exe \
	& $(lnexe) .\etex.exe $(bindir)\initex.exe \
	& $(lnexe) .\etex.exe $(bindir)\virtex.exe \
	& popd
!endif
# Omega	
	pushd $(bindir) \
	& $(del) .\iniomega.exe .\viromega.exe \
	& $(lnexe) .\omega.exe $(bindir)\iniomega.exe \
	& $(lnexe) .\omega.exe $(bindir)\viromega.exe \
	& popd
# pdfTeX
	pushd $(bindir) \
	& $(del) .\pdfinitex.exe .\pdfvirtex.exe \
	& $(lnexe) .\pdftex.exe $(bindir)\pdfinitex.exe \
	& $(lnexe) .\pdftex.exe $(bindir)\pdfvirtex.exe \
	& popd
# pdfeTeX
	pushd $(bindir) \
	& $(del) .\pdfeinitex.exe .\pdfevirtex.exe \
	& $(lnexe) .\pdfetex.exe $(bindir)\pdfeinitex.exe \
	& $(lnexe) .\pdfetex.exe $(bindir)\pdfevirtex.exe \
	& popd
# MF
	pushd $(bindir) \
	& $(del) .\inimf.exe .\virmf.exe \
	& $(lnexe) .\mf.exe $(bindir)\inimf.exe \
	& $(lnexe) .\mf.exe $(bindir)\virmf.exe \
	& popd
# MPost
	pushd $(bindir) \
	& $(del) .\inimpost.exe .\virmpost.exe \
	& $(lnexe) .\mpost.exe $(bindir)\inimpost.exe \
	& $(lnexe) .\mpost.exe $(bindir)\virmpost.exe \
	& popd
	if NOT "$(fmts)"=="" \
	for %%i in ($(fmts)) do \
          pushd $(bindir)       \
	  & $(del) .\%%~ni.exe \
	  & $(lnexe) .\tex.exe $(bindir)\%%~ni.exe \
	  & popd
	if NOT "$(efmts)"=="" \
	for %%i in ($(efmts)) do \
          pushd $(bindir)       \
          & $(del) .\%%~ni.exe \
	  & $(lnexe) .\etex.exe $(bindir)\%%~ni.exe \
	  & popd
# FIXME : e-TeX in compatible mode
!ifdef eTeX_as_TeX
	if not "$(cfmts)"=="" \
	for %%i in ($(cfmts)) do \
          pushd $(bindir)       \
          & $(del) .\%%~ni.exe \
	  & $(lnexe) .\etex.exe $(bindir)\%%~ni.exe \
	  & popd
!endif
	if not "$(ofmts)"=="" \
	for %%i in ($(ofmts)) do \
          pushd $(bindir)       \
          & $(del) .\%%~ni.exe \
	  & $(lnexe) .\omega.exe $(bindir)\%%~ni.exe \
	  & popd
	if not "$(pdffmts)"=="" \
	for %%i in ($(pdffmts)) do \
          pushd $(bindir)       \
          & $(del) .\%%~ni.exe \
	  & $(lnexe) .\pdftex.exe $(bindir)\%%~ni.exe \
	  & popd
	if not "$(pdfefmts)"=="" \
	for %%i in ($(pdfefmts)) do \
          pushd $(bindir)       \
          & $(del) .\%%~ni.exe \
	  & $(lnexe) .\pdfetex.exe $(bindir)\%%~ni.exe \
	  & popd
	if not "$(bases)"=="" \
	for %%i in ($(bases)) do \
          pushd $(bindir)       \
          & $(del) .\%%~ni.exe \
	  & $(lnexe) .\mf.exe $(bindir)\%%~ni.exe \
	  & popd
	if not "$(mems)"=="" \
	for %%i in ($(mems)) do \
          pushd $(bindir)       \
          & $(del) .\%%~ni.exe \
	  & $(lnexe) .\mpost.exe $(bindir)\%%~ni.exe \
	  & popd

# # Always do plain.*, so examples from the TeXbook (etc.) will work.
# install-formats: install-fmts install-bases install-mems
# install-fmts: $(all_fmts) $(all_efmts) $(all_cfmts) $(all_ofmts) $(all_pdffmts) $(all_pdfefmts)
# 	$(mktexdir) $(fmtdir)
# 	for %%f in ($(all_fmts) $(all_efmts) $(all_cfmts) $(all_ofmts) \
# 		$(all_pdffmts) $(all_pdfefmts)) \
# 	do $(copy) %%f $(fmtdir)\%%f
# 	$(del) -f $(fmtdir)\plain.fmt & $(LN) tex.fmt $(fmtdir)\plain.fmt
# # FIXME e-TeX / compatible mode	
# #	$(del) -f $(fmtdir)\plain.efmt & $(LN) tex.efmt $(fmtdir)\plain.efmt

# install-bases: $(all_bases)
# 	$(mktexdir) $(basedir)
# 	for %%f in ($(all_bases)) do $(copy) %%f $(basedir)\%%f
# 	$(del) -f $(basedir)\plain.base & $(LN) mf.base $(basedir)\plain.base

# install-mems: $(all_mems)
# 	$(mktexdir) $(memdir)
# 	for %%f in ($(all_mems)) do $(copy) %%f $(memdir)\%%f
# 	$(del) -f $(memdir)\plain.mem & $(LN) mpost.mem $(memdir)\plain.mem

# Auxiliary files.
install-data::
	$(copy) tex.pool $(texpooldir)\tex.pool
	$(copy) etex.pool $(texpooldir)\etex.pool
	$(copy) omega.pool $(texpooldir)\omega.pool
	$(copy) pdftex.pool $(texpooldir)\pdftex.pool
	$(copy) pdfetex.pool $(texpooldir)\pdfetex.pool
	$(copy) mf.pool $(mfpooldir)\mf.pool
	$(copy) mp.pool $(mppooldir)\mp.pool
# 	for %f in ($(srcdir)\share\*.tcx) do \
# 	  $(copy) %f $(web2cdir)\%~nxf
# 	for %f in ($(srcdir)\share\*.map) do \
# 	  $(copy) %f $(fontnamedir)\%~nxf

install-doc::
	pushd doc & $(make) $@ & popd

install-man::
	pushd man & $(make) $@ & popd

install-aux:: fmtutil.cnf
	-@echo $(verbose) & ( \
		$(copy) fmtutil.cnf $(web2cdir)\fmtutil.cnf $(redir_stdout) \
	)

fmtutil.cnf: fmtutil.in
	$(sed) -e "1,$$s/@TEXBIN@/tex/g" < fmtutil.in \
	| $(sed) "1,$$s/^@[^@]*@//g" > $@

# The distribution comes with up-to-date .info* files,
# so this should never be used unless something goes wrong
# with the unpacking, or you modify the manual.
doc\web2c.info:
	cd doc & $(make) info
info dvi:
	cd doc & $(make) $@

# Manual pages
manpages:
	cd man & $(make) all

# 
# make dist won't work for anyone but me. Sorry.
!ifdef MAINT
distname = web2c
program_files = PROJECTS *.ac *.ch tangleboot.p

# The files that omega places in the main directory.
omega_files = omegamem.h {odvicopy,odvitype,otangle}.{web,ch}

triptrapdiffs: triptrap\trip.diffs triptrap\mftrap.diffs \
  triptrap\mptrap.diffs 
triptrap\trip.diffs: tex
	$(make) trip | tail +1 >triptrap\trip.diffs
triptrap\mftrap.diffs: mf
	$(make) trap | tail +1 >triptrap\mftrap.diffs
triptrap\mptrap.diffs: mpost
	$(make) mptrap | tail +1 >triptrap\mptrap.diffs

tests\check.log: $(programs)
	$(make) check | tail +1 >tests\check.log
!endif

# 
# Testing, including triptrap. The `x' filenames are for output.
tex_check = tex-check
etex_check = etex-check
check: bibtex-check dvicopy-check dvitomp-check dvitype-check \
       $(etex_check) gftodvi-check gftopk-check gftype-check \
       mf-check mft-check mpost-check patgen-check pktogf-check \
       pktype-check pltotf-check pooltype-check $(tex_check) tftopl-check \
       vftovp-check vptovf-check weave-check

trip = trip
etrip = etrip
triptrap: $(trip) trap mptrap $(etrip)
testdir = $(srcdir)\triptrap
testenv = set TEXMFCNF=$(testdir)
dvitype_args = -output-level=2 -dpi=72.27 -page-start=*.*.*.*.*.*.*.*.*.*
trip: $(objdir)\pltotf.exe $(objdir)\tftopl.exe $(objdir)\tex.exe $(objdir)\dvitype.exe
	@echo ">>> See $(testdir)/trip.diffs for example of acceptable diffs."
	set TEXMFCNFOLD=$(TEXMFCNF)
	set TEXMFCNF=$(testdir)
	.\$(objdir)\pltotf $(testdir)\trip.pl trip.tfm
	.\$(objdir)\tftopl .\trip.tfm trip.pl
	-$(diff) $(testdir)\trip.pl trip.pl
# get same filename in log
	$(del) trip.tex & $(copy) $(testdir)\trip.tex . 
	echo $(PATH)
	-.\$(objdir)\tex -progname=initex < $(testdir)\trip1.in >tripin.fot
	$(move) trip.log tripin.log
	-$(diff) $(testdir)\tripin.log tripin.log
# May as well test non-ini second time through.
	-.\$(objdir)\tex < $(testdir)\trip2.in >trip.fot
	-$(diff) $(testdir)\trip.fot trip.fot
# We use $(diff) instead of `diff' only for those files where there
# might actually be legitimate numerical differences.
	-$(diff) $(diffflags) $(testdir)\trip.log trip.log
	-.\$(objdir)\dvitype $(dvitype_args) trip.dvi >trip.typ
	-$(diff) $(diffflags) $(testdir)\trip.typ trip.typ
	set TEXMFCNF=$(TEXMFCNFOLD)

# Can't run trap and mptrap in parallel, because both write trap.{log,tfm}.
trap: $(objdir)\mf.exe $(objdir)\tftopl.exe $(objdir)\gftype.exe
	@echo ">>> See $(testdir)/mftrap.diffs for example of acceptable diffs."
	set TEXMFCNFOLD=$(TEXMFCNF)
	set TEXMFCNF=$(testdir)
	$(del) trap.mf & $(copy) $(testdir)\trap.mf .
	-.\$(objdir)\mf -progname=inimf < $(testdir)\mftrap1.in > mftrapin.fot
	$(move) trap.log mftrapin.log
	-$(diff) $(testdir)\mftrapin.log mftrapin.log
	-.\$(objdir)\mf <$(testdir)\mftrap2.in >mftrap.fot
	$(move) trap.log mftrap.log
	$(move) trap.tfm mftrap.tfm
	-$(diff) $(testdir)\mftrap.fot mftrap.fot
	-$(diff) $(testdir)\mftrap.log mftrap.log
	.\$(objdir)\tftopl .\mftrap.tfm mftrap.pl
	-$(diff) $(testdir)\mftrap.pl mftrap.pl
	-.\$(objdir)\gftype -m -i .\trap.72270gf >trap.typ
	-$(diff) $(testdir)\trap.typ trap.typ
	set TEXMFCNF=$(TEXMFCNFOLD)

mptrap: $(objdir)\pltotf.exe $(objdir)\mpost.exe $(objdir)\tftopl.exe
	@echo ">>> See $(testdir)/mptrap.diffs for example of acceptable diffs." >&2
	set TEXMFCNFOLD=$(TEXMFCNF)
	set TEXMFCNF=$(testdir)
# get same filename in log
	$(del) mtrap.mp & $(copy) $(testdir)\mtrap.mp .
	.\$(objdir)\pltotf $(testdir)\trapf.pl trapf.tfm
	-.\$(objdir)\mpost -progname=inimpost mtrap
	-$(diff) $(testdir)\mtrap.log mtrap.log
	-$(diff) $(testdir)\mtrap.0 mtrap.0
	-$(diff) $(testdir)\mtrap.1 mtrap.1
	-$(diff) $(testdir)\writeo writeo
	-$(diff) $(testdir)\writeo.2 writeo.2
	$(del) trap.mp & $(copy) $(testdir)\trap.mp .
	$(del) trap.mpx & $(copy) $(testdir)\trap.mpx .
	-.\$(objdir)\mpost -progname=inimpost<$(testdir)\mptrap1.in >mptrapin.fot
	-$(move) trap.log mptrapin.log
	-$(diff) $(testdir)\mptrapin.log mptrapin.log
# Must run inimp or font_name[null_font] is not initialized, leading to diffs.
	-.\$(objdir)\mpost -progname=inimpost<$(testdir)\mptrap2.in >mptrap.fot
	-$(move) trap.log mptrap.log
	-$(move) trap.tfm mptrap.tfm
	-$(diff) $(testdir)\mptrap.fot mptrap.fot
	-$(diff) $(testdir)\mptrap.log mptrap.log
	-$(diff) $(testdir)\trap.5 trap.5
	-$(diff) $(testdir)\trap.6 trap.6
	-$(diff) $(testdir)\trap.148 trap.148
	-$(diff) $(testdir)\trap.149 trap.149
	-$(diff) $(testdir)\trap.150 trap.150
	-$(diff) $(testdir)\trap.151 trap.151
	-$(diff) $(testdir)\trap.197 trap.197
	-$(diff) $(testdir)\trap.200 trap.200
	.\$(objdir)\tftopl .\mptrap.tfm mptrap.pl
	-$(diff) $(testdir)\mptrap.pl mptrap.pl
	set TEXMFCNF=$(TEXMFCNFOLD)

# Ad hoc tests.
bibtex-check: $(objdir)\bibtex.exe
#	if EXIST tests\exampl.aux $(copy) $(srcdir)\tests\exampl.aux tests\exampl.aux
	@set BSTINPUTS=$(srcdir)\tests 
	.\$(objdir)\bibtex tests\exampl
	@set BSTINPUTS=

dvicopy-check: $(objdir)\dvicopy.exe
	.\$(objdir)\dvicopy $(srcdir)\tests\story tests\xstory.dvi
# Redirect stderr so the terminal output will end up in the log file.
	@set TFMFONTS=$(srcdir)/tests;
	.\$(objdir)\dvicopy < $(srcdir)\tests\pplr.dvi > tests\xpplr.dvi
	@set TFMFONTS=

dvitomp-check: $(objdir)\dvitomp.exe
	set TFMFONTS=$(srcdir)/tests;$(texmf)/fonts/tfm//
	set VFFONTS=$(srcdir)/tests;$(texmf)/fonts/vf//
	.\$(objdir)\dvitomp $(srcdir)\tests\story.dvi tests\xstory.mpx
	.\$(objdir)\dvitomp $(srcdir)\tests\ptmr
	$(move) ptmr.mpx tests\xptmr.mpx

dvitype-check: $(objdir)\dvitype.exe
	.\$(objdir)\dvitype -show-opcodes $(srcdir)\tests\story >tests\xstory.dvityp
	.\$(objdir)\dvitype --p=*.*.2 $(srcdir)\tests\pagenum.dvi >tests\xpagenum.typ

gftodvi-check: $(objdir)\gftodvi.exe
	set TFMFONTS=$(srcdir)/tests;$(texmf)/fonts/tfm//
	.\$(objdir)\gftodvi -verbose $(srcdir)/tests/cmr10.600gf
	$(move) cmr10.dvi tests\xcmr10.dvi

gftopk-check: $(objdir)\gftopk.exe
	.\$(objdir)\gftopk -verbose $(srcdir)\tests\cmr10.600gf tests\xcmr10.600pk
	.\$(objdir)\gftopk $(srcdir)\tests\cmr10.600gf cmr10.pk & rm cmr10.pk

gftype-check: $(objdir)\gftype.exe
	.\$(objdir)\gftype $(srcdir)\tests\cmr10.600gf >tests\xcmr10.gft1
	.\$(objdir)\gftype -m -i $(srcdir)\tests\cmr10.600gf >tests\xcmr10.gft2

mf-check: trap mf.base
	.\$(objdir)\mf "&./mf \tracingstats:=1; end."
	.\$(objdir)\mf "&./mf $(srcdir)\tests\online"
	.\$(objdir)\mf "&./mf $(srcdir)\tests\one.two"
	.\$(objdir)\mf "&./mf $(srcdir)\tests\uno.dos"

mft-check: $(objdir)\mft.exe
	.\$(objdir)\mft $(srcdir)/tests/io & $(move) io.tex tests\io.tex

mpost-check: mptrap mpost.mem
	.\$(objdir)\mpost "&./mpost \tracingstats:=1 ; end." & cd..
	set TFMFONTS=$(srcdir)/tests;$(texmf)/fonts/tfm//
	set MAKEMPX_BINDIR=.;..\contrib\$(objdir)
	set MPXCOMMAND=..\contrib\$(objdir)\makempx.exe
	.\$(objdir)\mpost $(srcdir)/tests/mptest
	set MAKEMPX_BINDIR=
	set MPXCOMMAND=
	.\$(objdir)\mpost $(srcdir)\tests\one.two
	.\$(objdir)\mpost $(srcdir)\tests\uno.dos

patgen-check: $(objdir)\patgen.exe
	.\$(objdir)\patgen $(srcdir)/tests/dict $(srcdir)/tests/patterns \
	 $(srcdir)/tests/xout $(srcdir)/tests/translate <$(srcdir)/tests/patgen.in

pktogf-check: $(objdir)\pktogf.exe
	.\$(objdir)\pktogf -verbose $(srcdir)\tests\cmr10.pk tests\xcmr10.600gf
	.\$(objdir)\pktogf $(srcdir)\tests\cmr10.pk & rm cmr10.gf

pktype-check: $(objdir)\pktype.exe
	.\$(objdir)\pktype $(srcdir)\tests\cmr10.pk >tests\xcmr10.pktyp

pltotf-check: $(objdir)\pltotf.exe
	.\$(objdir)\pltotf -verbose $(srcdir)\tests\cmr10 tests\xcmr10

# When tex.pool has not been generated we pooltype etex.pool
pooltype-check: $(objdir)\pooltype.exe
	.\$(objdir)\pooltype tex.pool >tests\xtexpool.typ
# FIXME: etex compatible mode
#	.\$(objdir)\pooltype etex.pool >tests\xtexpool.typ

# No need for tangle-check, since we run it to make everything else.

outcom = "Here are some things left to do.  If you would like to contribute, send mail to me (kb@mail.tug.org) first."

tex-check: trip tex.fmt
# Test truncation (but don't bother showing the warning msg).
	.\$(objdir)\tex --output-comment=$(outcom) $(srcdir)/tests/hello > nul \
	  & .\$(objdir)\dvitype hello.dvi | grep kb@mail.tug.org > nul
# \openout should show up in \write's.
	.\$(objdir)\tex $(srcdir)/tests/openout
	grep xfoo openout.log
# one.two.tex -> one.two.log
	.\$(objdir)\tex $(srcdir)/tests/one.two
	dir one.two.log
# uno.dos -> uno.log
	.\$(objdir)\tex $(srcdir)/tests/uno.dos
	dir uno.log
	.\$(objdir)\tex $(srcdir)/tests/just.texi
	dir just.log
	-.\$(objdir)\tex $(srcdir)/tests/batch.tex
	.\$(objdir)\tex --shell $(srcdir)/tests/write18 | grep echo
# tcx files test (to be included)
#	.\$(objdir)\tex --translate-file=$(srcdir)/share/isol1-t1.tcx tests/eight
#	.\$(objdir)\dvitype eight.dvi > eigh.typ
	.\$(objdir)\tex --mltex --progname=initex $(srcdir)/tests/mltextst
#	-.\$(objdir)\tex <nul
	set PATH=$(kpathseadir);$(kpathsea_srcdir);$(PATH)
	set WEB2C=$(kpathsea_srcdir)
	set TMPDIR=..
	-.\$(objdir)\tex "\nonstopmode\font\foo=nonesuch\end"

tftopl-check: $(objdir)\tftopl.exe
	.\$(objdir)\tftopl -verbose $(srcdir)\tests\cmr10 tests\xcmr10

vftovp-check: $(objdir)\vftovp.exe
	set TFMFONTS=$(srcdir)/tests;$(texmf)/fonts/tfm//
	.\$(objdir)\vftovp -verbose $(srcdir)\tests\ptmr ptmr tests\xptmr
	set TFMFONTS=

vptovf-check: $(objdir)\vptovf.exe
	.\$(objdir)\vptovf $(srcdir)\tests\ptmr $(srcdir)\tests\xptmr $(srcdir)\tests\xptmr

weave-check: $(objdir)\weave.exe
	.\$(objdir)\weave $(srcdir)\pooltype

installcheck:
	cd $(srcdir)\tests &  bibtex allbib & cd..
	mf "\mode:=ljfour; input logo10" & tftopl logo10.tfm >nul
	tex "\nonstopmode \tracingstats=1 \input story \bye"

# 
# Cleaning.
$(verpdftexdir) = $(verpdftexdir)
otps = otps
omegafonts = omegafonts
all_subdirs = doc lib man mpware web2c window $(otps) $(omegafonts) $(verpdftexdir) $(verpdfetexdir)

# Having a multiple-target rule with the subdir loop fails because of
# the dependencies introduced by clean.mk.  Yet, we want the
# dependencies here at the top level so that distclean will run the
# clean rules, etc.  So, sigh, put the subdir loop in each target and
# only run it if we have a Makefile.  Alternatively, we could do as
# Automake does.

mostlyclean::
	-@echo $(verbose) & for %d in ($(all_subdirs)) do \
		echo Entering %d for $@ \
		& pushd %d & $(make) $@ & popd
	-@$(del) $(objdir)\tangleboot.exe $(objdir)\ctangleboot.exe
clean::
	-@echo $(verbose) & for %d in ($(all_subdirs)) do \
		echo Entering %d for $@ \
		& pushd %d & $(make) $@ & popd
	-@$(del) $(all_c) *extra.c omega*.c tangleboot.c
	-@$(del) *.aux *.dvi *.fot *.log *.pl *.tfm *.typ *.ofl
	-@$(del) *.base *.fmt *.efmt *.mem *.oft
	-@$(deldir) tfm
# Have to list generated .h's explicitly since our real headers,
# e.g., help.h, are in this directory too.
	-@$(del) *coerce.h *d.h bibtex.h dvicopy.h dvitomp.h dvitype.h gftodvi.h
	-@$(del) gftopk.h gftype.h mft.h odvicopy.h odvitype.h
	-@$(del) otangle.h patgen.h pktogf.h
	-@$(del) pktype.h pltotf.h pooltype.h tangle.h tftopl.h vftovp.h vptovf.h
	-@$(del) weave.h
# Cleanup from triptrap.
	-@$(del) trip.tex trap.mf mtrap.mp trap.mp trap.mpx
	-@$(del) trip.* tripin.* tripos.tex 8terminal.tex
	-@$(del) trap.* mftrap.* mftrapin.* mptrap.* mptrapin.*
	-@$(del) trapf.* mtrap.* writeo* missfont.log
distclean::
	-@echo $(verbose) & for %d in ($(all_subdirs)) do \
		echo Entering %d for $@ \
		& pushd %d & $(make) $@ & popd
	-@$(del) *.pool *extra.c *.fm
# Have to list .p's explicitly because we can't remove tangleboot.p.
	-@$(del) bibtex.p dvicopy.p dvitomp.p dvitype.p etex.p gftodvi.p gftopk.p
	-@$(del) gftype.p mf.p mft.p mp.p odvicopy.p odvitype.p omega.p
	-@$(del) otangle.p patgen.p pdftex.p
	-@$(del) pdfetex.p pktogf.p pktype.p pltotf.p pooltype.p tangle.p
	-@$(del) tex.p tftopl.p vftovp.p vptovf.p weave.p
	-@$(del) tangleboot.c tangleboot.h
# And we clean up generated web and change files.
	-@$(del) etex.web omega.web pdftex.web pdfetex.web
	-@$(del) etex.ch  omega.ch  pdftex.ch pdfetex.ch
	-@$(del) mf-w32.ch  mp-w32.ch  tex-w32.ch
	-@$(del) odvicopy.web odvitype.web otangle.web
	-@$(del) odvicopy.ch  odvitype.ch  otangle.ch 
# And some miscellaneous files
	-@$(del) etrip.tex omega.c omegamem.h
	-@$(del) macnames.c ttf2afm.c
	-@$(del) c-auto.h tiedir\tie.ps tie.c
	-@$(del) fmtutil.cnf

extraclean::
	-@echo $(verbose) & for %d in ($(all_subdirs)) do \
		echo Entering %d for $@ \
		& pushd %d & $(make) $@ & popd
# Remove triptrap junk here too.
	-@$(del) trip.tex trap.mf mtrap.mp trap.mp trap.mpx
	-@$(del) trip.* tripin.* tripos.tex 8terminal.tex
	-@$(del) trap.* mftrap.* mftrapin.* mptrap.* mptrapin.*
	-@$(del) trapf.* mtrap.* writeo* missfont.log
	-@$(del) *.out *.typ *.fot
# And etrip junk as well.
	-@$(del) etrip.tex
	-@$(del) etrip.* etripin.*
maintainer-clean::
	-@echo $(verbose) & for %d in ($(all_subdirs)) do \
		echo Entering %d for $@ \
		& pushd %d & $(make) $@ & popd

!include <msvc/clean.mak>

depend:: c-sources
	for %i in (lib mpware web2c window pdftexdir $(otps)) do \
	  pushd %i & $(make) $@ & popd

!include <msvc/rdepend.mak>
!include "./depend.mak"

#  
# Local variables:
# page-delimiter: "^# \f"
# mode: Makefile
# End:
