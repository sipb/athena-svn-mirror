#
# Location for installing files
#
audio_ulaw_helpdir = $(datadir)/gnome/bonobo/docs/audio_ulaw/C

# 
# Module configuration information
#
module = audio_ulaw
main_module_file = $(module).sgml
module_install_dir = $(audio_ulaw_helpdir)

SGML_FILES = \
	audio_ulaw.sgml

EXTRA_DIST=$(SGML_FILES)

all:

$(module).html: $(builddir)/$(module)/$(module).html
	-cp $(builddir)/$(module)/index.html .

$(builddir)/$(module)/$(module).html: $(SGML_FILES)
	-db2html $(srcdir)/$(main_module_file)

dist-hook:
	mkdir $(distdir)/$(module)
	-cp $(builddir)/$(module)/*.html $(builddir)/$(module)/*.css $(distdir)/$(module)
	-cp $(builddir)/$(module).html $(distdir)
	mkdir $(distdir)/images
	-cp $(srcdir)/images/*.gif $(srcdir)/images/*.jpg $(distdir)/images

install-data-local: $(module).html
	$(mkinstalldirs) $(DESTDIR)$(module_install_dir)/images
	-for file in $(builddir)/$(module)/*.html $(builddir)/$(module)/*.css; do \
	basefile=`basename $$file`; \
	$(INSTALL_DATA) $(builddir)/$$file $(DESTDIR)$(module_install_dir)/$$basefile; \
	done
	-for file in $(builddir)/images/*.jpg $(builddir)/images/*.gif; do \
	$(INSTALL_DATA) $(builddir)/$$file $(DESTDIR)$(module_install_dir)/images;\
	done

