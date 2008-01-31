# -*- mode: makefile; coding: utf-8 -*-

include /usr/share/cdbs/1/rules/debathena-check-conffiles.mk
include /usr/share/cdbs/1/rules/debathena-divert.mk

DEBATHENA_REPLACE_CONFFILES = $(foreach package,$(DEB_ALL_PACKAGES),$(DEBATHENA_REPLACE_CONFFILES_$(package)))

CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), debathena-config-build-common (>= 3.6~)

debathena_conffile_dest = $(if $(DEBATHENA_CONFFILE_DEST_$(1)),$(DEBATHENA_CONFFILE_DEST_$(1)),$(1))

common-build-indep:: $(foreach file,$(DEBATHENA_REPLACE_CONFFILES),debian/conffile-new$(file))

debian/conffile-new%$(DEBATHENA_DIVERT_SUFFIX): $(call debathena_check_conffiles,%)
	debian/transform_$(notdir $(call debathena_conffile_dest,$(subst debian/conffile-new,,$@))) < $< \
	    > debian/$(notdir $(call debathena_conffile_dest,$(subst debian/conffile-new,,$@)))

$(patsubst %,binary-install/%,$(DEB_ALL_PACKAGES)) :: binary-install/%:
	$(foreach file,$(DEBATHENA_REPLACE_CONFFILES_$(cdbs_curpkg)), \
		install -d $(DEB_DESTDIR)/$(dir $(call debathena_conffile_dest,$(file))); \
		cp -a debian/$(notdir $(call debathena_conffile_dest,$(file))) \
		    $(DEB_DESTDIR)/$(dir $(call debathena_conffile_dest,$(file)));)

clean::
	$(foreach file,$(DEBATHENA_REPLACE_CONFFILES),rm -f debian/$(notdir $(file)))

