TOP = ${TOPDIR}
CURRENT_DIR = ${TOP}/synctree
DESTDIR=
CONFIGSRC = ${TOP}/imake

IRULESRC = $(CONFIGSRC)
IMAKE= imake -I${IRULESRC}

Makefile::
	-@if [ -f Makefile ]; then \
	echo "  rm -f Makefile.bak; mv Makefile Makefile.bak"; \
	rm -f Makefile.bak; mv Makefile Makefile.bak; \
	else exit 0; fi
	$(IMAKE) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)
