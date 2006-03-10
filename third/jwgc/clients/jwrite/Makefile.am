INCLUDES = -I../../lib/libxode/include -I../../lib/libjwgc/include -I../../lib/libjabber/include
bin_PROGRAMS = jwrite
jwrite_SOURCES = jwrite.c
jwrite_LDADD = ../../lib/libjwgc/libjwgc.a ../../lib/libxode/libxode.a
man_MANS = jwrite.1
EXTRA_DIST = jwrite.pod $(man_MANS) jwrite.html
BUILT_SOURCES = jwrite.1 jwrite.html

jwrite.1: jwrite.pod
	if [ -x "`which pod2man`" ]; then pod2man --section=1 --release=$(VERSION) --center="Jabber Windowgram Client" --date="`date --iso-8601`" jwrite.pod jwrite.1; else echo "Unable to find pod2man to create documentation."; fi

jwrite.html: jwrite.pod
	if [ -x "`which pod2html`" ]; then pod2html --infile jwrite.pod --outfile jwrite.html; else echo "Unable to find pod2html to create documentation."; fi
