# (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
# ALL RIGHTS RESERVED 
# 
# Motif Release 1.2
#
# Makefile.ini
#
# This is a toplevel Makefile for bootstrapping an initial build of Motif
# Release 1.2.  It provides two targets: Makefile (for the cautious) and
# World (for the bold).  After the full Makefile has been produced, that
# should be used.  However, if it is accidentally corrupted, or a build
# is initiated on a new platform, Makefile.ini can be used to restart.
#
       SHELL = /bin/sh
          RM = rm -f
          MV = mv
        MAKE = make
         TOP = .
 CURRENT_DIR = .
   CONFIGSRC = $(TOP)/config
    IMAKESRC = $(CONFIGSRC)
    IRULESRC = $(CONFIGSRC)
       IMAKE = $(IMAKESRC)/imake
   IMAKE_CMD = $(NEWTOP)$(IMAKE) -I$(NEWTOP)$(IRULESRC) $(IMAKE_DEFINES)
       FLAGS = $(MFLAGS) -f Makefile.ini BOOTSTRAPCFLAGS="$(BOOTSTRAPCFLAGS)"

all:
	@echo "Please begin by using make World or make Makefile.  You may"
	@echo "find it necessary to set the make variable BOOTSTRAPCFLAGS in"
	@echo "order to compile imake.  One common example is"
	@echo "\"BOOTSTRAPCFLAGS=-I<dir>\", where <dir> is the top of your"
	@echo "X11R5 source tree.  Remember to check the configuration"
	@echo "parameters in the config directory."
	@echo
	@echo "We recommend that you read the release notes carefully before"
	@echo "proceeding; they can be found under the doc directory.  If"
	@echo "the top-level Makefile gets corrupted, copy Makefile.ini to"
	@echo "Makefile and try again.  Do not name your log file make.log or"
	@echo "it will be deleted."

Makefile::
	cd $(IMAKESRC); $(MAKE) $(FLAGS)
	-@if [ -f Makefile ]; then set -x; \
	$(RM) Makefile.bak; $(MV) Makefile Makefile.bak; \
	else exit 0; fi
	$(IMAKE_CMD) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)

World:
	@echo ""
	@echo "Building Release 1.2 of OSF/Motif"
	@echo ""
	@date
	cd $(IMAKESRC); $(MAKE) $(FLAGS) clean; $(MAKE) $(FLAGS)
	-$(RM) Makefile.bak; $(MV) Makefile Makefile.bak
	$(IMAKE_CMD) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)
	$(MAKE) $(MFLAGS) Makefiles
	$(MAKE) $(MFLAGS) testsMakefile
	$(MAKE) $(MFLAGS) clean
	$(MAKE) $(MFLAGS) includes
	$(MAKE) $(MFLAGS) depend
	$(MAKE) $(MFLAGS) $(WORLDOPTS)
	@echo ""
	@date
	@echo ""
	@echo "Full build of Release 1.2 of OSF/Motif complete."
	@echo ""
