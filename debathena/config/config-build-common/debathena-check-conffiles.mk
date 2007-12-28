# -*- mode: makefile; coding: utf-8 -*-

ifneq ($(DEBATHENA_CHECK_CONFFILES),)
# Deprecated interface.

CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), debathena-config-build-common (>= 3.3~)

$(DEBATHENA_CHECK_CONFFILES) :: %: FORCE
	[ -e "$@" ]
	dpkg-query --showformat='$${Conffiles}\n' --show "$$(dpkg -S $@ | cut -f1 -d:)" | \
	    grep -F " $@ " | sed 's/^ \(.*\) \(.*\)$$/\2  \1/' | \
	    md5sum -c  # Check that the file has not been modified.

FORCE:

.PRECIOUS: $(DEBATHENA_CHECK_CONFFILES)

else

CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), debathena-config-build-common (>= 3.5~)

DEBATHENA_CHECK_CONFFILES_DIR = debian/conffile-copies

debathena_check_conffiles = $(patsubst %,$(DEBATHENA_CHECK_CONFFILES_DIR)%,$(1))
undebathena_check_conffiles = $(patsubst $(DEBATHENA_CHECK_CONFFILES_DIR)%,%,$(1))

debathena_check_conffiles_tmp = $(patsubst %,%.tmp,$(call debathena_check_conffiles,$(1)))
undebathena_check_conffiles_tmp = $(call undebathena_check_conffiles,$(patsubst %.tmp,%,$(1)))

$(call debathena_check_conffiles,%): $(call debathena_check_conffiles_tmp,%)
	mv $< $@

$(call debathena_check_conffiles_tmp,%): name = $(call undebathena_check_conffiles_tmp,$@)
$(call debathena_check_conffiles_tmp,%): truename = $(shell /usr/sbin/dpkg-divert --truename $(name))
$(call debathena_check_conffiles_tmp,%): package = $(shell dpkg -S $(name) | grep -v "^diversion by" | cut -f1 -d:)
$(call debathena_check_conffiles_tmp,%): $(truename)
	mkdir -p $(@D)
	cp "$(truename)" $@
	dpkg-query --showformat='$${Conffiles}\n' --show $(package) | \
	    sed -n 's,^ $(name) \(.*\)$$,\1  $@, p' | \
	    md5sum -c  # Check that the file has not been modified.

clean::
	rm -rf $(DEBATHENA_CHECK_CONFFILES_DIR)

endif
