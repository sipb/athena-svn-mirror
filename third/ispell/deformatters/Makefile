#
# $Id: Makefile,v 1.1.1.1 2007-02-01 19:50:32 ghudson Exp $
#
# Copyright 2001, Geoff Kuenning, Claremont, CA
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

#
# $Log: not supported by cvs2svn $
# Revision 1.7  2005/05/01 23:04:58  geoff
# Add EXEEXT to executables for DOS portability (courtesy of Eli Zaretskii).
#
# Revision 1.6  2004/06/02 06:30:28  geoff
# Add .c: and .o: rules so proper flags are always used.
#
# Revision 1.5  2004/06/02 06:28:12  geoff
# Minor license update
#
# Revision 1.4  2001/09/06 00:35:18  geoff
# Shorten names so that the deformatters can be built on MS-DOS.
#
# Revision 1.3  2001/07/25 21:51:48  geoff
# *** empty log message ***
#
# Revision 1.2  2001/07/23 20:43:38  geoff
# *** empty log message ***
#
# Revision 1.1  2001/06/07 07:23:54  geoff
# Initial revision
#
#

SHELL = /bin/sh
MAKE = make

PROGRAMS = defmt-c$(EXEEXT) defmt-sh$(EXEEXT)

all:	$(PROGRAMS)

defmt-c: defmt-c.o
defmt-sh: defmt-sh.o

.c.o:
	@. ../config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -c $<

.c:
	@. ../config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o $@ $<

.o:
	@. ../config.sh; \
	  set -x; \
	  $$CC $$CFLAGS -o $@ $<

install: all
	@. ../config.sh; \
	  set -x; \
	  rm -f $$BINDIR/deformat-c $$BINDIR/deformat-sh
	@. ../config.sh; \
	  set -x; \
	  $$INSTALL $(PROGRAMS) $$BINDIR
	@. ../config.sh; \
	  set -x; \
	  cd $$BINDIR; \
	  strip $(PROGRAMS); \
	  chmod 755 $(PROGRAMS)

clean:
	rm -f *.o core a.out mon.out $(PROGRAMS)

realclean veryclean:	clean
