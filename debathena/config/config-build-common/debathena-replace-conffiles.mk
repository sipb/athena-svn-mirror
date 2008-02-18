# -*- mode: makefile; coding: utf-8 -*-

include /usr/share/cdbs/1/rules/debathena-check-conffiles.mk

DEBATHENA_REPLACE_CONFFILES = $(foreach package,$(DEB_ALL_PACKAGES),$(DEBATHENA_REPLACE_CONFFILES_$(package)))

DEBATHENA_REPLACE_CONFFILES_DIR=debian/replace_file_copies

debathena_replace_conffiles = $(patsubst %,$(DEBATHENA_REPLACE_CONFFILES_DIR)%,$(1))
undebathena_replace_conffiles = $(patsubst $(DEBATHENA_REPLACE_CONFFILES_DIR)%,%,$(1))

common-build-indep:: $(foreach file,$(DEBATHENA_REPLACE_CONFFILES),$(call debathena_replace_conffiles,$(file)))

$(call debathena_replace_conffiles,%): $(call debathena_check_conffiles,%)
	mkdir -p $(@D)
	$(if $(DEBATHENA_TRANSFORM_SCRIPT_$(call undebathena_replace_conffiles,$@)), \
	    $(DEBATHENA_TRANSFORM_SCRIPT_$(call undebathena_replace_conffiles,$@)), \
	    debian/transform_$(notdir $(call undebathena_replace_conffiles,$@))) < $< > $@

$(patsubst %,binary-install/%,$(DEB_ALL_PACKAGES)) :: binary-install/%:
	$(foreach file,$(DEBATHENA_REPLACE_CONFFILES_$(cdbs_curpkg)), \
		install -d $(DEB_DESTDIR)/$(dir $(file)); \
		cp -a $(DEBATHENA_REPLACE_CONFFILES_DIR)$(file) \
		    $(DEB_DESTDIR)/$(dir $(file));)

clean::
	$(foreach file,$(DEBATHENA_REPLACE_CONFFILES),rm -f debian/$(notdir $(file)))
	rm -rf $(DEBATHENA_REPLACE_CONFFILES_DIR)

