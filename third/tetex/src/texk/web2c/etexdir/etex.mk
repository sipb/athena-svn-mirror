# Makefile fragment for e-TeX and web2c. --infovore@xs4all.nl. Public domain.
# This fragment contains the parts of the makefile that are most likely to
# differ between releases of e-TeX.

Makefile: $(srcdir)/etexdir/etex.mk

# The C sources.
etex_c = etexini.c etex0.c etex1.c etex2.c
etex_o = etexini.o etex0.o etex1.o etex2.o etexextra.o

# Generation of the web and ch file.
etex.web: tie tex.web etexdir/etex.ch
	./tie -m etex.web $(srcdir)/tex.web $(srcdir)/etexdir/etex.ch
etex.ch: tie etex.web tex.ch etexdir/tex.ech
	./tie -c etex.ch etex.web $(srcdir)/tex.ch $(srcdir)/etexdir/tex.ech

# Tests...
etestdir = $(srcdir)/etexdir/etrip
etestenv = TEXMFCNF=$(etestdir)

etrip: pltotf tftopl etex dvitype
	@echo ">>> See $(etestdir)/etrip.diffs for example of acceptable diffs." >&2
	@echo "*** TRIP test for e-TeX in compatibility mode ***."
	./pltotf $(testdir)/trip.pl trip.tfm
	./tftopl ./trip.tfm trip.pl
	-diff $(testdir)/trip.pl trip.pl
	rm -f trip.tex; $(LN) $(testdir)/trip.tex . # get same filename in log
	-$(SHELL) -c '$(etestenv) ./etex --progname=einitex <$(testdir)/trip1.in >ctripin.fot'
	mv trip.log ctripin.log
	-diff $(testdir)/tripin.log ctripin.log
	-$(SHELL) -c '$(etestenv) ./etex <$(testdir)/trip2.in >ctrip.fot'
	mv trip.log ctrip.log
	-diff $(testdir)/trip.fot ctrip.fot
	-$(DIFF) $(DIFFFLAGS) $(testdir)/trip.log ctrip.log
	$(SHELL) -c '$(etestenv) ./dvitype $(dvitype_args) trip.dvi >ctrip.typ'
	-$(DIFF) $(DIFFFLAGS) $(testdir)/trip.typ ctrip.typ
	@echo "*** TRIP test for e-TeX in extended mode ***."
	-$(SHELL) -c '$(etestenv) ./etex --progname=einitex <$(etestdir)/etrip1.in >xtripin.fot'
	mv trip.log xtripin.log
	-diff ctripin.log xtripin.log
	-$(SHELL) -c '$(etestenv) ./etex <$(etestdir)/trip2.in >xtrip.fot'
	mv trip.log xtrip.log
	-diff ctrip.fot xtrip.fot
	-$(DIFF) $(DIFFFLAGS) ctrip.log xtrip.log
	$(SHELL) -c '$(etestenv) ./dvitype $(dvitype_args) trip.dvi >xtrip.typ'
	-$(DIFF) $(DIFFFLAGS) ctrip.typ xtrip.typ
	@echo "*** e-TeX specific part of e-TRIP test ***."
	./pltotf $(etestdir)/etrip.pl etrip.tfm
	./tftopl ./etrip.tfm etrip.pl
	-diff $(etestdir)/etrip.pl etrip.pl
	rm -f etrip.tex; $(LN) $(etestdir)/etrip.tex . # get same filename in log
	-$(SHELL) -c '$(etestenv) ./etex --progname=einitex <$(etestdir)/etrip2.in >etripin.fot'
	mv etrip.log etripin.log
	-diff $(etestdir)/etripin.log etripin.log
	-$(SHELL) -c '$(etestenv) ./etex <$(etestdir)/etrip3.in >etrip.fot'
	-diff $(etestdir)/etrip.fot etrip.fot
	-$(DIFF) $(DIFFFLAGS) $(etestdir)/etrip.log etrip.log
	diff $(etestdir)/etrip.out etrip.out
	$(SHELL) -c '$(etestenv) ./dvitype $(dvitype_args) etrip.dvi >etrip.typ'
	-$(DIFF) $(DIFFFLAGS) $(etestdir)/etrip.typ etrip.typ

etex-check: etrip etex.efmt
# Test truncation (but don't bother showing the warning msg).
	./etex --output-comment="`cat $(srcdir)/PROJECTS`" \
	  $(srcdir)/tests/hello 2>/dev/null \
	  && ./dvitype hello.dvi | grep olaf@infovore.xs4all.nl >/dev/null
# \openout should show up in \write's.
	./etex $(srcdir)/tests/openout && grep xfoo openout.log
# one.two.tex -> one.two.log
	./etex $(srcdir)/tests/one.two && ls -l one.two.log
# uno.dos -> uno.log
	./etex $(srcdir)/tests/uno.dos && ls -l uno.log
	./etex $(srcdir)/tests/just.texi && ls -l just.log
	-./etex $(srcdir)/tests/batch.tex
	./etex --shell $(srcdir)/tests/write18 | grep echo
# tcx files are a bad idea.
#	./etex --translate-file=$(srcdir)/share/isol1-t1.tcx \
#	  $(srcdir)/tests/eight && ./dvitype eight.dvi >eigh.typ
	./etex --mltex --progname=einitex $(srcdir)/tests/mltextst
	-./etex </dev/null
	-PATH=`pwd`:$(kpathsea_dir):$(kpathsea_srcdir):$$PATH \
	  WEB2C=$(kpathsea_srcdir) TMPDIR=.. \
	  ./etex '\nonstopmode\font\foo=nonesuch\end'

# Distfiles ...
@MAINT@triptrapdiffs: etexdir/etrip/etrip.diffs
@MAINT@etexdir/etrip/etrip.diffs: etex
@MAINT@	$(MAKE) etrip | tail +1 >etexdir/etrip/etrip.diffs

# end of etex.mk
