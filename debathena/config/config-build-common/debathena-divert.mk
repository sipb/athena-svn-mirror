# -*- mode: makefile; coding: utf-8 -*-

CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), debathena-config-build-common (>= 3.7~)

DEBATHENA_DIVERT_SCRIPT = /usr/share/debathena-config-build-common/divert.sh.in

DEBATHENA_DIVERT_PACKAGES += $(foreach package,$(DEB_ALL_PACKAGES), \
    $(if $(DEBATHENA_DIVERT_FILES_$(package)),$(package)))

DEBATHENA_DIVERT_PACKAGES += $(foreach package,$(DEB_ALL_PACKAGES), \
    $(if $(DEBATHENA_REPLACE_CONFFILES_$(package)),$(package)))

ifeq ($(DEBATHENA_DIVERT_SUFFIX),)
DEBATHENA_DIVERT_SUFFIX = .debathena
endif

DEBATHENA_DIVERT_ENCODER = /usr/share/debathena-config-build-common/encode

debathena-divert/%: package = $(subst debathena-divert/,,$@)
debathena-divert/%: replace_inputs = $(DEBATHENA_REPLACE_CONFFILES_$(package))
debathena-divert/%: replace_files = $(foreach file,$(replace_inputs),$(file))
debathena-divert/%: divert_files = $(DEBATHENA_DIVERT_FILES_$(package)) $(replace_files)
$(patsubst %,debathena-divert/%,$(DEBATHENA_DIVERT_PACKAGES)) :: debathena-divert/%:
	( \
	    sed 's/#PACKAGE#/$(cdbs_curpkg)/g; s/#DEBATHENA_DIVERT_SUFFIX#/$(DEBATHENA_DIVERT_SUFFIX)/g' $(DEBATHENA_DIVERT_SCRIPT); \
	    $(if $(divert_files), \
		echo 'if [ "$$1" = "configure" ]; then'; \
		$(foreach file,$(divert_files), \
		    echo "    divert_link $(subst $(DEBATHENA_DIVERT_SUFFIX), ,$(file))";) \
		echo 'fi'; \
	    ) \
	) >> $(CURDIR)/debian/$(cdbs_curpkg).postinst.debhelper
	( \
	    sed 's/#PACKAGE#/$(cdbs_curpkg)/g; s/#DEBATHENA_DIVERT_SUFFIX#/$(DEBATHENA_DIVERT_SUFFIX)/g' $(DEBATHENA_DIVERT_SCRIPT); \
	    $(if $(divert_files), \
		echo 'if [ "$$1" = "remove" ]; then'; \
		$(foreach file,$(divert_files), \
		    echo "    undivert_unlink $(subst $(DEBATHENA_DIVERT_SUFFIX), ,$(file))";) \
		echo 'fi'; \
	    ) \
	) >> $(CURDIR)/debian/$(cdbs_curpkg).prerm.debhelper
	( \
	    echo -n "divert:Diverted="; \
	    $(foreach file,$(divert_files),\
		${DEBATHENA_DIVERT_ENCODER} "$(subst $(DEBATHENA_DIVERT_SUFFIX),,$(file))"; \
		echo -n ", ";) \
	    echo \
	) >> $(CURDIR)/debian/$(cdbs_curpkg).substvars

$(patsubst %,binary-fixup/%,$(DEBATHENA_DIVERT_PACKAGES)) :: binary-fixup/%: debathena-divert/%
