#
# $Id: Makefile,v 1.5 2007-02-01 19:58:05 ghudson Exp $
#
# Copyright 1992, 1993, 1999, 2001, 2005, Geoff Kuenning, Claremont, CA
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All modifications to the source code must be clearly marked as
#    such.  Binary redistributions based on modified source code
#    must be clearly marked as modified versions in the documentation
#    and/or other materials provided with the distribution.
# 4. The code that causes the 'ispell -v' command to display a prominent
#    link to the official ispell Web site may not be removed.
# 5. The name of Geoff Kuenning may not be used to endorse or promote
#    products derived from this software without specific prior
#    written permission.
#
# THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# You will have to create a local.h file before building; look over
# config.X to learn what things you may need to define, or use one of
# the sample local.h files shipped.
#
# the argument syntax for buildhash to make alternate dictionary files
# is simply:
#
#   buildhash <dictfile> <affix-file> <outfile>

# $Log: not supported by cvs2svn $
# Revision 1.1.1.2  2007/02/01 19:50:33  ghudson
# Import ispell 3.3.02.
#
# Revision 1.124  2005/05/25 13:56:32  geoff
# Make language subdirectories in reverse order so that the English
# makefiles can install a default dictionary linked to the first-mentioned
# language.
#
# Revision 1.123  2005/05/25 13:22:33  geoff
# Bug fix: make sure HASHSUFFIX set before using it during installation.
#
# Revision 1.122  2005/05/01 23:19:12  geoff
# Make constructed files depend on the Makefile
#
# Revision 1.121  2005/05/01 23:07:59  geoff
# Add EXTEXT usage (courtesy of Eli Zaretskii).
#
# Revision 1.120  2005/05/01 22:35:00  geoff
# Get rid of references to fixispell-a in the installation process (but
# blow away any previously installed copies).  Add BAKEXT to the stuff
# that's in config.sh and that's substituted when editing .X files.
# Sort the list of edit substitutions.  When generating DEFHASH, use the
# configurable has suffix.
#
# Revision 1.119  2005/04/28 01:03:11  geoff
# Improve the generation of defhash.h to set up DEFLANG and to allow all
# definitions to be overridden by local.h.  Clean up defhash.h in "make
# clean".
#
# Revision 1.118  2005/04/28 00:26:06  geoff
# Dynamically generate MASTERHASH and DEFHASH from the first dictionary
# name specified in LANGUAGES.
#
# Revision 1.117  2005/04/27 01:18:34  geoff
# Work around idiotic POSIX incompatibilies in tail.  Proactively use
# the -e switch to sed against the day when they decide to make sed
# incompatible as well.
#
# Revision 1.116  2005/04/21 14:08:58  geoff
# Insert a missing "silencing at".  Change the name of the sample
# local.h file.
#
# Revision 1.115  2005/04/14 15:19:37  geoff
# Fix typos in the special term.o rule.
#
# Revision 1.114  2005/04/14 14:38:23  geoff
# Update license.  Integrate Ed Avis's changes.  Add fixispell-a (broken
# though it is).  Generate ispell.5 dynamically.  Put term.o first to
# improve detection of misconfigurations, and generate a special message
# if it won't compile.  Play funny games with y.tab.c to deal with
# MS-DOS stupidity.
#
# Revision 1.113  2002/06/20 23:46:15  geoff
# Put sq/unsq back, since some dictionaries are still distributed in sq
# format.  Add code to make directory paths when necessary.
#
# Revision 1.112  2001/09/06 00:30:28  geoff
# Many changes from Eli Zaretskii to support DJGPP compilation.
#
# Revision 1.111  2001/07/25 21:51:45  geoff
# Minor license update.
#
# Revision 1.110  2001/07/23 20:24:02  geoff
# Update the copyright and the license.
#
# Revision 1.109  2001/06/07 19:06:33  geoff
# When creating local.h, make it user-writable.
#
# Revision 1.108  2001/06/07 08:02:18  geoff
# Add the deformatters to the default dependencies.
#
# Revision 1.107  1999/01/13 01:34:15  geoff
# Get rid of some leftover references to emacs stuff.
#
# Revision 1.106  1999/01/08  04:32:31  geoff
# Don't try to build or install ispell.el and ispell.info.
#
# Revision 1.105  1999/01/07  01:22:29  geoff
# Update the copyright.  Get rid of old shar-based dictionary building.
#
# Revision 1.104  1995/11/08  05:09:06  geoff
# When invoking "showversion", ignore any errors that happen.  Fix a
# place where I forgot to double a dollar sign in install-languages.
#
# Revision 1.103  1995/10/25  03:35:38  geoff
# Don't assume the presence of "head" in the showversion dependency; use
# old reliable sed instead.  Also make showversion depend on ispell
# itself, since it uses it.
#
# Revision 1.102  1995/10/11  04:30:25  geoff
# Allow DEFHASH and MASTERHASH to be the same.
#
# Revision 1.101  1995/08/05  23:19:33  geoff
# Add the showversion target to ease the job of debugging people's make
# output.  Make sure SHELLDEBUG is set for the entire process of making
# config.sh.
#
# Revision 1.100  1995/03/06  02:42:39  geoff
# Run iwhich explicitly from the shell, so that we don't have to worry
# about PATH problems and whether it's executable or not.  Also fix a
# place where -nw was not removed from the batch emacs switch list.
#
# Revision 1.99  1995/01/08  23:23:23  geoff
# Get rid of an obsolete etags flag.  Add some new variables to config.sh.
#
# Revision 1.98  1994/12/27  23:08:43  geoff
# Use the new "iwhich" script to decide whether to install emacs-related
# stuff.  Make correct.o depend on version.h so that "ispell -a" always
# reports the correct version.
#
# Revision 1.97  1994/11/21  07:02:51  geoff
# Specify default values for the BUILD macros, so that some systems
# don't accidentally override them and make ispell think that the
# dictionaries are missing.
#
# Revision 1.96  1994/10/25  05:45:54  geoff
# Make the installation command configurable.
#
# Revision 1.95  1994/10/18  04:03:17  geoff
# Get rid of DICTVARIANTS, which is obsolete.  Compile term.o first, so
# that errors in it (which are common) will show up first.  Improve the
# rules for generation of msgs.h.
#
# Revision 1.94  1994/09/16  05:06:55  geoff
# Split installation up into basic and dictionary-building tools, so
# that we can have a partial-install target.
#
# Revision 1.93  1994/09/16  04:51:28  geoff
# Handle installations that have ELISPDIR but not TEXINFODIR
#
# Revision 1.92  1994/09/16  02:45:52  geoff
# Don't strip non-binaries.  Fix an accidentally-doubled backslash.
#
# Revision 1.91  1994/08/31  05:58:27  geoff
# Strip binaries before installing them.  Create directories before
# installing into them.  Make sure manual pages are installed with the
# correct protection modes.
#
# Revision 1.90  1994/05/25  04:29:16  geoff
# Don't remove english.4 after the English makefile has carefully
# installed it.
#
# Revision 1.89  1994/05/24  05:31:22  geoff
# Return to the old sed-based method of parsing LANGUAGES, so that things
# will work on broken systems like BSDI.
#
# Revision 1.88  1994/05/24  04:54:30  geoff
# Fix the emacs installation to use emacs batch mode properly, so that a
# terminal isn't required.
#
# Revision 1.87  1994/03/21  01:55:17  geoff
# If a hard link can't be made to msgs.h, copy it instead
#
# Revision 1.86  1994/02/22  06:09:03  geoff
# Add SHELLDEBUG.  Change the language-subdirs target to use the shell
# IFS variable to parse things, simplifying things and improving
# efficiency (thanks to Hagen Ross for the idea and implemenation).
#
# Revision 1.85  1994/02/13  23:25:31  geoff
# Fix multiple-language processing to not pass subsequent specifications to
# the first Makefile.  Also fix the language shell loop to be more flexible.
#
# Revision 1.84  1994/02/07  08:10:40  geoff
# When processing the LANGUAGES configuration variable (from local.h),
# use sed instead of expr to process it.  This gets around versions of
# expr that have 127-character limitations (though it probably still
# limits us to 512 characters with some versions of sed, so further work
# may be needed here).
#
# Revision 1.83  1994/02/07  06:31:20  geoff
# Clarify how to change variables in local.h
#
# Revision 1.82  1994/02/07  06:29:31  geoff
# Add a dummy else clause to shell if-test for Ultrix
#
# Revision 1.81  1994/02/07  05:35:34  geoff
# Make realclean run dictclean
#
# Revision 1.80  1994/01/26  07:44:43  geoff
# Make yacc configurable through local.h.
#
# Revision 1.79  1994/01/25  07:11:11  geoff
# Get rid of all old RCS log lines in preparation for the 3.1 release.
#
#

#
# !!!DO NOT EDIT HERE!!!
#
# Unlike previous versions of ispell, there should be no need to edit
# your Makefile.  Instead, #define the corresponding variables in your
# local.h file; the Makefile will automatically pick them up.  The
# only reason you should need to edit the Makefile might to be to add
# non-English dictionary support.
#
# For example, if you want to set CFLAGS to "-g -Wall", don't put it
# here.  Put:
#
#	#define CFLAGS	"-g -Wall"
#
# in local.h.  Otherwise, it won't have any effect.
#
EXTRADICT = Use_config.sh

SHELL = /bin/sh

#
#	Set this to "-vx" in the make command line if you need to
#	debug the complex shell commands.
#
SHELLDEBUG = +vx

all:	config.sh
all:	programs defmt-programs showversion ispell.1 ispell.5
all:	all-languages

programs: buildhash findaffix tryaffix ispell
programs: icombine ijoin munchlist
programs: subset zapdups

defmt-programs:
	cd deformatters; $(MAKE) all

showversion:	ispell
	-./ispell -v | sed -e 1q

.c.o:
	@. ./config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -c $<

#
# The funny business with y_tab.c is necessary for MS-DOS systems,
# where filenames can't have multiple periods.
#
.y.o:
	@. ./config.sh; \
	  set -x; \
	  $$YACC $<; \
	  [ -f y_tab.c ]  ||  mv y.tab.c y_tab.c; \
	  $$CC $$CFLAGS -c y_tab.c; \
	  mv y_tab.o $@; \
	  rm -f y_tab.c

all-languages:	munchable
	$(MAKE) LANGUAGE_TARGET=all SHELLDEBUG=$(SHELLDEBUG) language-subdirs

install: config.sh all install-basic install-deformatters
install: install-dictbuild install-languages

partial-install: config.sh all install-basic install-languages

install-basic:
	@. ./config.sh; \
	  set -x; \
	  [ -d $$BINDIR ]  ||  $(MAKE) NEWDIR=$$BINDIR mkdirpath; \
	  cd $$BINDIR; \
	  rm -f ispell$$EXEEXT
	@. ./config.sh; \
	  set -x; \
	  $$INSTALL ispell$$EXEEXT $$BINDIR
	@. ./config.sh; \
	  set -x; \
	  cd $$BINDIR; \
	  strip ispell$$EXEEXT; \
	  chmod 755 ispell$$EXEEXT
	@. ./config.sh; \
	  set -x; \
	  [ -d $$MAN1DIR ]  ||  $(MAKE) NEWDIR=$$MAN1DIR mkdirpath; \
	  [ -d $$MAN45DIR ]  ||  $(MAKE) NEWDIR=$$MAN45DIR mkdirpath; \
	  cd $$MAN1DIR; \
	    rm -f ispell$$MAN1EXT; \
	  cd $$MAN45DIR; \
	    rm -f ispell$$MAN45EXT
	@. ./config.sh; \
	  set -x; \
	  $$INSTALL ispell.1 $$MAN1DIR/ispell$$MAN1EXT; \
	  $$INSTALL ispell.5 $$MAN45DIR/ispell$$MAN45EXT
	@. ./config.sh; \
	  set -x; \
	  cd $$MAN1DIR; \
	  chmod 644 ispell$$MAN1EXT; \
	  cd $$MAN45DIR; \
	  chmod 644 ispell$$MAN45EXT

mkdirpath:
	@[ "X$(NEWDIR)" = X ]  &&  (echo NEWDIR unset 1>&2; exit 1); exit 0
	@path=; \
	  umask 22; \
	  for i in `echo $(NEWDIR) | tr / ' '`; do \
	    path="$$path/$$i"; \
	    if [ ! -d $$path ]; then \
		echo mkdir $$path; \
		mkdir $$path; \
	    fi; \
	  done

install-deformatters:
	@. ./config.sh; \
	  set -x; \
	  cd deformatters; \
	  $(MAKE) EXEEXT=$$EXEEXT install

install-dictbuild:
	@. ./config.sh; \
	  set -x; \
	  [ -d $$BINDIR ]  ||  $(MAKE) NEWDIR=$$BINDIR mkdirpath; \
	  cd $$BINDIR; \
	  rm -f buildhash icombine ijoin \
	    munchlist findaffix fixispell-a tryaffix sq unsq; \
	  rm -f $$LIBDIR/icombine
	@. ./config.sh; \
	  set -x; \
	  $$INSTALL buildhash icombine ijoin munchlist findaffix \
	      tryaffix \
	    $$BINDIR
	@. ./config.sh; \
	  set -x; \
	  cd $$BINDIR; \
	  strip buildhash$$EXEEXT icombine$$EXEEXT ijoin$$EXEEXT; \
	  chmod 755 buildhash$$EXEEXT icombine$$EXEEXT ijoin$$EXEEXT \
	    munchlist findaffix tryaffix
	@. ./config.sh; \
	  set -x; \
	  [ -d $$MAN1DIR ]  ||  $(MAKE) NEWDIR=$$MAN1DIR mkdirpath; \
	  [ -d $$MAN45DIR ]  ||  $(MAKE) NEWDIR=$$MAN45DIR mkdirpath; \
	  cd $$MAN1DIR; \
	    rm -f fixispell-a$$MAN1EXT
	@. ./config.sh; \
	  set -x; \
	  $$INSTALL sq.1 $$MAN1DIR/sq$$MAN1EXT; \
	  for m in buildhash munchlist findaffix tryaffix; do \
	    echo ".so `basename $$MAN1DIR`/ispell$$MAN1EXT" \
	      > $$MAN1DIR/$$m$$MAN1EXT; \
	  done; \
	  echo ".so `basename $$MAN1DIR`/sq$$MAN1EXT" \
	    > $$MAN1DIR/unsq$$MAN1EXT 
	@. ./config.sh; \
	  set -x; \
	  cd $$MAN1DIR; \
	  chmod 644 buildhash$$MAN1EXT \
	    munchlist$$MAN1EXT findaffix$$MAN1EXT tryaffix$$MAN1EXT

install-languages:
	$(MAKE) LANGUAGE_TARGET=install SHELLDEBUG=$(SHELLDEBUG) \
	  language-subdirs
	. ./config.sh; \
	  [ -d $$LIBDIR ]  ||  $(MAKE) NEWDIR=$$LIBDIR mkdirpath; \
	  set -x; \
	  cd $$LIBDIR; \
	  if [ $$MASTERHASH != $$DEFHASH ]; then \
	    rm -f $$DEFHASH; \
	    $$LINK $$MASTERHASH $$DEFHASH; \
	  fi

munchable:	findaffix tryaffix munchlist buildhash ispell icombine
munchable:	ijoin

#
#	The following auxiliary dependency is used to make targets in
#	the language directories.  Do you find it intimidating?  No
#	surprise;  remember that this is by the guy who wrote munchlist.
#
LANGUAGE_TARGET	=   Do_not_try_to_make_this_target_yourself
BUILD	=	build
CBUILD	=	build
DBUILD	=	build

language-subdirs:	config.sh
	@. ./config.sh; \
	    set $(SHELLDEBUG); \
	    set +e; \
	    while [ "X$$LANGUAGES" != X ]; do \
		( \
		descriptor=`echo "$$LANGUAGES" \
		  | sed -e 's/.*{\([^}]*\)}[^{]*$$/\1/'`; \
		dir=`echo "$$descriptor" | sed -e 's/\([^,]*\).*/\1/'`; \
		descriptor=`echo "$$descriptor" \
		  | sed -e 's/[^,]*,*\(.*\).*/\1/'`; \
		makeargs=''; \
		while [ "X$$descriptor" != X ]; \
		do \
		    nextvar=`echo "$$descriptor" \
		      | sed -e 's/\([^,]*\).*/\1/'`; \
		    makeargs="$$makeargs '$$nextvar'"; \
		    descriptor=`echo "$$descriptor" \
		      | sed -e 's/[^,]*,*\(.*\).*/\1/'`; \
		done; \
		set -x; \
		cd languages/$$dir; \
		eval $(MAKE) BUILD=$(BUILD) DBUILD=$(DBUILD) CBUILD=$(CBUILD) \
		  SHELLDEBUG=$(SHELLDEBUG) "$$makeargs" $(LANGUAGE_TARGET) \
		    ||  exit 1; \
		)  ||  exit 1; \
		LANGUAGES=`echo "$$LANGUAGES" \
		  | sed -e 's/\(.*\){[^{]*$$/\1/'`; \
		case "$$LANGUAGES" in \
		    ''|*{*}*) \
			;; \
		    *) \
			echo "Bad language specification: '$$LANGUAGES'" \
			  1>&2; \
			exit 2 \
			;; \
		esac; \
	    done; \
	    exit 0

buildhash: config.sh buildhash.o hash.o makedent.o parse.o
	@. ./config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o buildhash buildhash.o hash.o makedent.o parse.o \
	    $$LIBES

icombine: config.sh icombine.o makedent.o parse.o
	@. ./config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o icombine icombine.o makedent.o parse.o \
	    $$LIBES

ijoin: config.sh ijoin.o fields.o
	@. ./config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o ijoin ijoin.o fields.o $$LIBES

EDITFILE	=	notthere
OUTFILE		=	/dev/null

#
# Note: we use "sed -n -e $$p" to achieve "tail -1" here because some
# idiot decided to break backwards compatibility in some versions of
# tail.  I have nothing but contempt for such fools.
#
defhash.h:  config.X local.h Makefile
	set $(SHELLDEBUG); \
	MASTERHASH=`cat config.X local.h \
	  | sed -n -e \
	    's/^#define[ 	]*LANGUAGES[ 	][^}]*HASHFILES=\([^,}]*\).*$$/\1/p' \
	  | sed -n -e '$$p'`; \
	HASHSUFFIX=`cat config.X local.h \
	  | sed -n -e \
	    's/^#define[ 	]*HASHSUFFIX[ 	]*"\(.*\)"/\1/p' \
	  | sed -n -e '$$p'`; \
	case "$$MASTERHASH" in \
	    american*|british*) DEFHASH="english$$HASHSUFFIX";; \
	    *) DEFHASH="$$MASTERHASH";; \
	esac; \
	DEFLANG=`expr "$$DEFHASH" : '\(.*\)\..*$$'`.aff; \
	echo "/* This file is generated by the Makefile.  Don't edit it! */" \
	  > defhash.h; \
	echo '' >> defhash.h; \
	echo '#ifndef MASTERHASH' >> defhash.h; \
	echo '#define MASTERHASH "'"$$MASTERHASH"'"' >> defhash.h; \
	echo '#endif' >> defhash.h; \
	echo '#ifndef DEFHASH' >> defhash.h; \
	echo '#define DEFHASH "'"$$DEFHASH"'"' >> defhash.h; \
	echo '#endif' >> defhash.h; \
	echo '#ifndef DEFLANG' >> defhash.h; \
	echo '#define DEFLANG "'"$$DEFLANG"'"' >> defhash.h; \
	echo '#endif' >> defhash.h

config.sh:  config.X defhash.h local.h Makefile
	set $(SHELLDEBUG); \
	for var in BAKEXT BINDIR CC CFLAGS COUNTSUFFIX DEFDICT DEFHASH \
	  DEFLANG EXEEXT HASHSUFFIX INSTALL \
	  LANGUAGES LIBDIR LIBES LINK LINT LINTFLAGS LOOK_XREF \
	  MAKE_SORTTMP MAN1DIR MAN1EXT MAN45DIR MAN45EXT MAN45SECT MASTERHASH \
	  MSGLANG POUNDBANG REGLIB STATSUFFIX \
	  SPELL_XREF TERMLIB TIB_XREF WORDS YACC \
	  ; do \
	    cat config.X defhash.h local.h \
	      | sed -n -e "s/^#define[ 	]*$$var[ 	]*"'"'"/$$var=/p" \
	      | sed -e 's/"[^"]*$$/'"'/" -e "s/=/='/" -e 's/\\"/"/g' \
	      | sed -n -e '$$p'; \
	  done > config.sh; \
	echo 'case "$$MAKE_SORTTMP" in "") \
	  SORTTMP="-e /!!SORTTMP!!/s/=.*$$/=/";; *) SORTTMP=;; esac' \
	  >> config.sh

doedit:
	. ./config.sh; \
	  set $(SHELLDEBUG); \
	  sed \
	    -e "s@!!BAKEXT!!@$$BAKEXT@g" \
	    -e "s@!!COUNTSUFFIX!!@$$COUNTSUFFIX@g" \
	    -e "s@!!DEFHASH!!@$$DEFHASH@" -e "s@!!DEFLANG!!@$$DEFLANG@" \
	    -e "s@!!HASHSUFFIX!!@$$HASHSUFFIX@g" \
	    -e "s@!!LIBDIR!!@$$LIBDIR@" -e "s@!!DEFDICT!!@$$DEFDICT@" \
	    -e "s@!!LOOK_XREF!!@$$LOOK_XREF@g" \
	    -e "s@!!MAN45SECT!!@$$MAN45SECT@g" \
	    -e "s@!!POUNDBANG!!@$$POUNDBANG@g" \
	    -e "s@!!SPELL_XREF!!@$$SPELL_XREF@g" \
	    -e "s@!!STATSUFFIX!!@$$STATSUFFIX@g" \
	    -e "s@!!TIB_XREF!!@$$TIB_XREF@g" \
	    -e "s@!!WORDS!!@$$WORDS@g" \
	    $$SORTTMP < $(EDITFILE) > $(OUTFILE)

findaffix:	findaffix.X config.sh
	@$(MAKE) EDITFILE=findaffix.X OUTFILE=findaffix doedit
	chmod +x findaffix

ispell.1:	ispell.1X config.sh
	@$(MAKE) EDITFILE=ispell.1X OUTFILE=ispell.1 SHELLDEBUG=$(SHELLDEBUG) \
	  doedit

ispell.5:	ispell.5X config.sh
	@$(MAKE) EDITFILE=ispell.5X OUTFILE=ispell.5 SHELLDEBUG=$(SHELLDEBUG) \
	  doedit

munchlist:	munchlist.X config.sh
	@$(MAKE) EDITFILE=munchlist.X OUTFILE=munchlist \
	  SHELLDEBUG=$(SHELLDEBUG) doedit
	chmod +x munchlist

subset:	subset.X config.sh
	@$(MAKE) EDITFILE=subset.X OUTFILE=subset SHELLDEBUG=$(SHELLDEBUG) \
	  doedit
	chmod +x subset

tryaffix:	tryaffix.X config.sh
	@$(MAKE) EDITFILE=tryaffix.X OUTFILE=tryaffix \
	  SHELLDEBUG=$(SHELLDEBUG) doedit
	chmod +x tryaffix

zapdups:	zapdups.X config.sh
	@$(MAKE) EDITFILE=zapdups.X OUTFILE=zapdups SHELLDEBUG=$(SHELLDEBUG) \
	  doedit
	chmod +x zapdups

#
# We put term.o first because it is the most common cause of
# compilation errors.  Ispell.o is listed second because it's the main
# program.  The remainder of the object files are listed
# alphabetically.
#
OBJS	=	term.o ispell.o correct.o defmt.o dump.o exp_table.o fields.o \
		good.o lookup.o hash.o makedent.o tgood.o tree.o xgets.o

#
# A special rule for term.o to suggest configuration changes
#
term.o:	term.c
	@. ./config.sh; \
	  echo $$CC $$CFLAGS -c term.c 1>&2; \
	  $$CC $$CFLAGS -c term.c \
	    ||  (echo \
		   "term.c wouldn't compile.  Try inverting USG in local.h" \
		    1>&2; \
		exit 1)

ispell: config.sh $(OBJS)
	@. ./config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o ispell $(OBJS) $$TERMLIB $$REGLIB $$LIBES

sq:	config.sh msgs.h sq.c
	@@. ./config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o sq sq.c

unsq:	config.sh msgs.h unsq.c
	@@. ./config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o unsq unsq.c

$(OBJS) buildhash.o icombine.o hash.o parse.o: config.h ispell.h local.h
$(OBJS) buildhash.o icombine.o hash.o parse.o: proto.h msgs.h config.sh
$(OBJS) buildhash.o icombine.o hash.o parse.o: defhash.h
exp_table.o tgood.o: exp_table.h
fields.o: fields.h
ijoin.o: config.sh config.h ispell.h local.h
ijoin.o: proto.h fields.h
buildhash.o correct.o ispell.o: version.h

config.h:	config.X local.h
	cp config.X config.h
	chmod u+w config.h
	echo '' >> config.h
	echo '/* AUTOMATICALLY-GENERATED SYMBOLS */' >> config.h
	cat local.h config.X \
	  | egrep '^#define[ 	]*SIGNAL_TYPE' \
	  | sed -e 's/TYPE[ 	]*/TYPE_STRING "/' -e 's/$$/"/' -e 1q \
	  >> config.h
	cat local.h config.X \
	  | egrep '^#define[ 	]*MASKTYPE' \
	  | sed -e 's/TYPE[ 	]*/TYPE_STRING "/' -e 's/$$/"/' -e 1q \
	  >> config.h

#	Create a sample local.h if no such file currently exists
local.h:
	set +e; \
	  [ -r local.h ]  ||  (cp local.h.generic local.h; chmod u+w local.h)

msgs.h:	config.sh FRC
	@. ./config.sh; \
	  set $(SHELLDEBUG); \
	  set +e; \
	  if [ -r languages/$$MSGLANG/msgs.h ]; then \
	    msgs=languages/$$MSGLANG/msgs.h; \
	  else \
	    msgs=languages/english/msgs.h; \
	  fi; \
	  if cmp -s msgs.h $$msgs; then \
	    :; \
	  else \
	    set -x; \
	    rm -f msgs.h; $$LINK $$msgs msgs.h  ||  cp $$msgs msgs.h; \
	  fi

FRC:

tags:	config.h *.[chy]
	ctags -w -t *.[chy]
	sed -e s/config.h/config.X/ tags > ntags
	mv ntags tags

TAGS:	config.h *.[chy]
	etags *.[chy]
	sed -e s/config.h/config.X/ TAGS > NTAGS
	mv NTAGS TAGS

#
# The funny business with y_tab.c is necessary for MS-DOS systems,
# where filenames can't have multiple periods.
#
lint:	languages/*/msgs.h
lint:	config.sh config.h ispell.h proto.h *.[cy]
	@. ./config.sh; \
	  $$LINT $$LINTFLAGS ispell.c correct.c defmt.c dump.c exp_table.c \
	    good.c hash.c lookup.c makedent.c tgood.c term.c tree.c xgets.c; \
	  $$YACC parse.y; \
	  [ -f y_tab.c ] || mv y.tab.c y_tab.c; \
	  $$LINT $$LINTFLAGS buildhash.c hash.c makedent.c y_tab.c; \
	  $$LINT $$LINTFLAGS icombine.c makedent.c y_tab.c; \
	  $$LINT $$LINTFLAGS ijoin.c fields.c
	@rm -f y_tab.c

clean:	config.sh clean-deformatters clean-languages
	@. ./config.sh; \
	  set -x; \
	  rm -f $$DEFHASH $$FOREIGNHASHES
	rm -f *.o core a.out mon.out hash.out y.tab.c y_tab.c *.stat *.cnt \
		config.h defhash.h msgs.h unpacked
	rm -f buildhash findaffix tryaffix ispell icombine ijoin \
		munchlist subset sq unsq zapdups ispell.1 ispell.5 ispell.info

clean-deformatters:
	cd deformatters; $(MAKE) clean

clean-languages:
	$(MAKE) LANGUAGE_TARGET=clean SHELLDEBUG=$(SHELLDEBUG) language-subdirs

realclean veryclean:	clean dictclean
	rm -f config.sh

#
#	The following target allows you to clean out the combined
#	dictionary files.  For safety, so you don't lose your files,
#	it makes sure that there is something to work from, but it can
#	only be so smart, so be careful!
#
dictclean:
	$(MAKE) LANGUAGE_TARGET=dictclean SHELLDEBUG=$(SHELLDEBUG) \
	  language-subdirs
