EXTRA_WEB_FILES = doxygen.css \
  doxygen.png

HTML_FILES = annotated.html\
  files.html\
  functions.html\
  globals.html\
  group__activation.html\
  group__callbacks.html\
  group__currents.html\
  group__debugerr.html\
  group__enum.html\
  group__lookup.html\
  group__props.html\
  group__settings.html\
  group__wininfo.html\
  group__xkbevents.html\
  group__xkbgroup.html\
  group__xkbinfo.html\
  group__xklconfig.html\
  group__xklconfiginitterm.html\
  group__xklinitterm.html\
  index.html\
  modules.html\
  struct__XklConfigItem.html\
  struct__XklConfigItem-members.html\
  struct__XklConfigRec.html\
  struct__XklConfigRec-members.html\
  structXklState.html\
  structXklState-members.html\
  xklavier_8h.html\
  xklavier_8h-source.html\
  xklavier__config_8h.html\
  xklavier__config_8h-source.html

DOXYGEN_OUT_FILES = $(EXTRA_WEB_FILES) $(HTML_FILES)

doc_to = $(datadir)/doc/$(PACKAGE)-$(VERSION)/html

doc_files = $(DOXYGEN_OUT_FILES)

EXTRA_DIST= $(doc_files)

install:
	@$(NORMAL_INSTALL)
	if test "x$(DO_DOXYGEN)" = "xyes" ; then \
	  $(mkinstalldirs) $(DESTDIR)$(doc_to); \
	  for p in $(doc_files); do \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(doc_to)/$$p; \
	  done; \
	fi

uninstall:
	@$(NORMAL_UNINSTALL)
	if test "x$(DO_DOXYGEN)" = "xyes" ; then \
	  for p in $(doc_files); do \
	    rm -f $(DESTDIR)$(doc_to)/$$p; \
	  done \
	fi

MAINTAINERCLEANFILES = $(DOXYGEN_OUT_FILES)

