## Makefile for the local installation of the CTAN Catalogue:
##
## 1997/07/25 -- Ulrik Vieth <vieth@thphy.uni-duesseldorf.de>
##

## Usage:
##
## Say 'make' or 'make all' to patch CTAN-relative directories in the
## distributed Catalogue files as downloaded from CTAN for local use.
##

## Name of fixed URL path to replace:
ORIG	= http://theory.uwinnipeg.ca/cgi-bin/CTAN

## Name of prefered CTAN host to use:
CTAN	= ftp://ftp.dante.de/tex-archive

##
GZ_PROG	= gzip

default: all

## replace relative path ../../  -> $(CTAN)
## replace absolute path $(ORIG) -> $(CTAN)
all:
	for f in ctfull.html ct[a-z].html ; do \
	  sed -e 's@\.\./\.\./@$(CTAN)/@g' \
	      -e 's@$(ORIG)/@$(CTAN)/@g' \
	      < $$f > $$f.tmp; mv $$f.tmp $$f; \
	done
	$(GZ_PROG) -c ctfull.html > ctfull.html.gz

