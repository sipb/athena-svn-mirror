INCLUDES = -I../../lib/libxode/include -I../../lib/libjwgc/include -I../../lib/libjabber/include
bin_PROGRAMS = jlocate
jlocate_SOURCES = jlocate.c
jlocate_LDADD = ../../lib/libjwgc/libjwgc.a ../../lib/libxode/libxode.a
man_MANS = jlocate.1
EXTRA_DIST = jlocate.pod $(man_MANS) jlocate.html
BUILT_SOURCES = jlocate.1 jlocate.html

jlocate.1: jlocate.pod
	if [ -x "`which pod2man`" ]; then pod2man --section=1 --release=$(VERSION) --center="Jabber Windowgram Client" --date="`date --iso-8601`" jlocate.pod jlocate.1; else echo "Unable to find pod2man to create documentation."; fi

jlocate.html: jlocate.pod
	if [ -x "`which pod2html`" ]; then pod2html --infile jlocate.pod --outfile jlocate.html; else echo "Unable to find pod2html to create documentation."; fi
