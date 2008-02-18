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

include /usr/share/cdbs/1/rules/debathena-divert.mk

DEBATHENA_CHECK_CONFFILES_DIR = debian/conffile-copies

debathena_check_conffiles_source = $(if $(DEBATHENA_CHECK_CONFFILES_SOURCE_$(1)),$(DEBATHENA_CHECK_CONFFILES_SOURCE_$(1)),$(1))
debathena_check_conffiles_check = $(subst $(DEBATHENA_DIVERT_SUFFIX),,$(call debathena_check_conffiles_source,$(1)))

debathena_check_conffiles = $(patsubst %,$(DEBATHENA_CHECK_CONFFILES_DIR)%,$(1))
undebathena_check_conffiles = $(patsubst $(DEBATHENA_CHECK_CONFFILES_DIR)%,%,$(1))

debathena_check_conffiles_tmp = $(patsubst %,%.tmp,$(call debathena_check_conffiles,$(1)))
undebathena_check_conffiles_tmp = $(call undebathena_check_conffiles,$(patsubst %.tmp,%,$(1)))

$(call debathena_check_conffiles,%): $(call debathena_check_conffiles_tmp,%)
	mv $< $@

$(call debathena_check_conffiles_tmp,%): target = $(call undebathena_check_conffiles_tmp,$@)
$(call debathena_check_conffiles_tmp,%): name = $(call debathena_check_conffiles_check,$(target))
$(call debathena_check_conffiles_tmp,%): truename = $(shell /usr/sbin/dpkg-divert --truename $(name))
$(call debathena_check_conffiles_tmp,%): package = $(shell dpkg -S $(name) | grep -v "^diversion by" | cut -f1 -d:)
$(call debathena_check_conffiles_tmp,%): $(truename)
	[ -n $(package) ]
	mkdir -p $(@D)
	cp "$(truename)" $@
	md5=$$(dpkg-query --showformat='$${Conffiles}\n' --show $(package) | \
	    sed -n 's,^ $(name) \([0-9a-f]*\)$$,\1  $@, p'); \
	if [ -n "$$md5" ]; then \
	    echo "$$md5" | md5sum -c; \
	elif [ -e /var/lib/dpkg/info/$(package).md5sums ]; then \
	    md5=$$(sed -n 's,^\([0-9a-f]*\)  $(patsubst /%,%,$(name))$$,\1  $@, p' \
		/var/lib/dpkg/info/$(package).md5sums); \
	    [ -n "$$md5" ] && echo "$$md5" | md5sum -c; \
	else \
	    echo "warning: $(package) does not include md5sums!"; \
	    echo "warning: md5sum for $(name) not verified."; \
	fi

clean::
	rm -rf $(DEBATHENA_CHECK_CONFFILES_DIR)

endif
