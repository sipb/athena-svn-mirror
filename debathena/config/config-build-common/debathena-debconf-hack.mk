# -*- mode: makefile; coding: utf-8 -*-

ifndef _cdbs_rules_debathena_debconf_hack
_cdbs_rules_debathena_debconf_hack = 1

CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), debathena-config-build-common (>= 3.2~)

DEBATHENA_DEBCONF_HACK_SCRIPT = /usr/share/debathena-config-build-common/debconf-hack.sh

DEBATHENA_DEBCONF_HACK_PACKAGES += $(foreach package,$(DEB_ALL_PACKAGES), \
    $(if $(wildcard debian/$(package).debconf-hack),$(package)))

$(patsubst %,debathena-debconf-hack/%,$(DEBATHENA_DEBCONF_HACK_PACKAGES)) :: debathena-debconf-hack/%:
	( \
	    cat $(DEBATHENA_DEBCONF_HACK_SCRIPT); \
	    echo 'if [ ! -f /var/cache/$(cdbs_curpkg).debconf-save ]; then'; \
	    echo '    debconf_get $(shell cut -d'	' -f2 debian/$(cdbs_curpkg).debconf-hack) >/var/cache/$(cdbs_curpkg).debconf-save'; \
	    echo '    debconf_set <<EOF'; \
	    sed 's/$$/	true/' debian/$(cdbs_curpkg).debconf-hack; \
	    echo 'EOF'; \
	    echo 'fi'; \
	) >> $(CURDIR)/debian/$(cdbs_curpkg).preinst.debhelper
	( \
	    cat $(DEBATHENA_DEBCONF_HACK_SCRIPT); \
	    echo 'if [ -f /var/cache/$(cdbs_curpkg).debconf-save ]; then'; \
	    echo '    debconf_set </var/cache/$(cdbs_curpkg).debconf-save'; \
	    echo '    rm -f /var/cache/$(cdbs_curpkg).debconf-save'; \
	    echo 'fi'; \
	) >> $(CURDIR)/debian/$(cdbs_curpkg).postinst.debhelper
	( \
	    cat $(DEBATHENA_DEBCONF_HACK_SCRIPT); \
	    echo 'if [ -f /var/cache/$(cdbs_curpkg).debconf-save ]; then'; \
	    echo '    debconf_set </var/cache/$(cdbs_curpkg).debconf-save'; \
	    echo '    rm -f /var/cache/$(cdbs_curpkg).debconf-save'; \
	    echo 'fi'; \
	) >> $(CURDIR)/debian/$(cdbs_curpkg).postrm.debhelper

$(patsubst %,binary-fixup/%,$(DEBATHENA_DEBCONF_HACK_PACKAGES)) :: binary-fixup/%: debathena-debconf-hack/%

endif
