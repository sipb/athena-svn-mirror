INCLUDES = -I../../lib/libxode/include -I../../lib/libjwgc/include -I../../lib/libjabber/include
bin_PROGRAMS = jctl
jctl_SOURCES = jctl.c
jctl_LDADD = ../../lib/libjwgc/libjwgc.a ../../lib/libxode/libxode.a
man_MANS = jctl.1
EXTRA_DIST = jctl.pod $(man_MANS) jctl.html
BUILT_SOURCES = jctl.1 jctl.html

jctl.1: jctl.pod
	if [ -x "`which pod2man`" ]; then pod2man --section=1 --release=$(VERSION) --center="Jabber Windowgram Client" --date="`date --iso-8601`" jctl.pod jctl.1; else echo "Unable to find pod2man to create documentation."; fi

jctl.html: jctl.pod
	if [ -x "`which pod2html`" ]; then pod2html --infile jctl.pod --outfile jctl.html; else echo "Unable to find pod2html to create documentation."; fi
