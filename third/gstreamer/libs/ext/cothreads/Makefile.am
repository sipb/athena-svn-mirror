SUBDIRS=cothreads tests

EXTRA_DIST = ltmain.sh mkinstalldirs config.guess config.sub install-sh missing depcomp cothreads.pc.in cothreads-uninstalled.pc.in autogen.sh common m4

dist-hook:
	rm -rf `find $(distdir)/common -name CVS`
	rm -rf `find $(distdir)/m4 -name CVS`
