#AUTOMAKE_OPTIONS = foreign dist-tarZ #distdir=$(PACKAGE)$(VERSION)
#AUTOMAKE_OPTIONS = foreign dist-tarZ no-dependencies
AUTOMAKE_OPTIONS = foreign dist-tarZ

SUBDIRS = \
	include \
	libntp	\
	libparse	\
	authstuff	\
	ntpdate	\
	ntpq	\
	ntptrace	\
	parseutil	\
	xntpd	\
	xntpdc	\
	adjtimed	\
	clockstuff	\
	kernel	\
	util

EXTRA_DIST = ChangeLog COPYRIGHT NEWS TODO WHERE-TO-START WINNT-README.html acconfig.h autoconf-2.12.patches automake-1.2.patches config.guess config.h.in config.sub excludes install-sh blddbg.bat bldrel.bat mkdist.bat mksrc.bat
#ETAGS_ARGS = $(srcdir)/Makefile.am $(srcdir)/configure.in
ETAGS_ARGS = Makefile.am configure.in

# DIST_CPDIRS = conf html scripts 
# DIST_MKDIRS = adjtime clockstuff kernel libparse ppsclock
DIST_HOOK_DIRS = conf html patches scripts

# local-dist: dist-tarZ

dist-hook:
	for i in $(DIST_HOOK_DIRS); do \
	  mkdir $(distdir)/$$i ; \
	  cp -rp $(srcdir)/$$i $(distdir) ; \
	done ; \
	cp -rp $(srcdir)/include/winnt $(distdir)/include

dist-export: distdir
	rm $(distdir)/libntp/authdes.c
	cp $(distdir)/libntp/authdes.c.export $(distdir)/libntp/authdes.c
	chmod -R a+r $(distdir)
	mv $(distdir) $(distdir)-export
	$(TAR) chozf $(distdir)-export.tar.gz $(distdir)-export
	rm -rf $(distdir)-export

#Makefile: .warning

.warning:
	@echo "Compiling with GCC now generates lots of new warnings."
	@echo " "
	@echo "Don't be concerned. They're just warnings."
	@echo " "
	@echo "Don't send bug reports about the warnings, either."
	@echo " "
	@echo "Feel free to send patches that fix these warnings, though."
	@echo " "
	@sleep 1
	@touch .warning
