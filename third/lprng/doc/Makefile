###########################################################################
# LPRng - An Extended Print Spooler System
#
# Copyright 1988-1995 Patrick Powell, San Diego State University
#     papowell@sdsu.edu
# See LICENSE for conditions of use.
#
###########################################################################
# MODULE: UTILS/Makefile
# PURPOSE: generate utilities
# $Id: Makefile,v 1.1.1.2 2000-03-31 15:56:06 mwhitson Exp $
########################################################################## 


all:
	$(MAKE) MAKETARGET=$@ $(ALLDIRS)

.PHONY: all clean ci install FRC realclean mostlyclean distclean

realclean mostlyclean distclean clean:
	$(MAKE) MAKETARGET=$@ $(ALLDIRS)
install:
	$(MAKE) MAKETARGET=$@ $(ALLDIRS)

ALLDIRS = Macros

$(ALLDIRS): FRC
	(cd $@; $(MAKE) $(MAKETARGET))
FRC:


Install.txt: Install.nroff
	typeset -O$@ $^
Install.ps: Install.nroff
	typeset -rt1 -Tps -O$@ $^

DIRS= Install Intro


ci cifast:
	for i in $(ALLDIRS) ; do \
		if test ! -d $$i/RCS ; then \
			mkdir $$i/RCS ; \
		fi; \
	done;
	checkin() { \
		ci $(CI) -l -mUpdate -t-Initial $$1; \
	}; \
	for i in * ; do \
		if [ -f "$$i" ] ; then \
			case $$i in \
			*.txt | *.ps ) ;; \
			* ) checkin $$i ;; \
			esac; \
		fi; \
	done;
	checkin() { \
		ci $(CI) -l -mUpdate -t-Initial $$1; \
	}; \
	for i in */* ; do \
		if [ -f "$$i" ] ; then \
			case $$i in \
			*.txt | *.ps | *RCS* | *,v ) ;; \
			* ) checkin $$i ;; \
			esac; \
		fi; \
	done;

update:

dist_list:
	echo RCS >/tmp/X
	if [ -n "$$NO" ]; then \
		for i in $$NO ; do \
			echo "*/$$i"  >>/tmp/X; \
		done; \
	fi;
	echo '*core' >>/tmp/X
	echo '*.orig' >>/tmp/X
	echo '*.bak' >>/tmp/X
	echo '*.rej' >>/tmp/X
	echo '?' >>/tmp/X
	echo '*.o' >>/tmp/X
	echo '*.a' >>/tmp/X
	echo '*,v' >>/tmp/X

#	cat /tmp/X

dist: dist_list
	DIR=`pwd | sed 's,.*/,,' `; \
		cd ..; \
		tar zXcf /tmp/X $${DIR}.tgz $${DIR}; \
		md5 $${DIR}.tgz | pgp -fast -u papowell@astart > $${DIR}.tgz.md5
