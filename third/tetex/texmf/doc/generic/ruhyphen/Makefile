PATTERNS = al as ct dv vl zn
CYRYO = $(patsubst %,cyryo%.tex,$(PATTERNS))

cyryo: $(CYRYO)

distclean:
	rm -f $(CYRYO)

cyryo%.tex: ruhyph%.tex
	./mkcyryo $< > $@
