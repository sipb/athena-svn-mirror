INCLUDES = -I../../lib/libxode/include -I../../lib/libjwgc/include -I../../lib/libjabber/include
bin_PROGRAMS = jstat
jstat_SOURCES = jstat.c
jstat_LDADD = ../../lib/libjwgc/libjwgc.a ../../lib/libxode/libxode.a
man_MANS = jstat.1
EXTRA_DIST = jstat.pod $(man_MANS) jstat.html
BUILT_SOURCES = jstat.1 jstat.html

jstat.1: jstat.pod
	if [ -x "`which pod2man`" ]; then pod2man --section=1 --release=$(VERSION) --center="Jabber Windowgram Client" --date="`date --iso-8601`" jstat.pod jstat.1; else echo "Unable to find pod2man to create documentation."; fi

jstat.html: jstat.pod
	if [ -x "`which pod2html`" ]; then pod2html --infile jstat.pod --outfile jstat.html; else echo "Unable to find pod2html to create documentation."; fi
