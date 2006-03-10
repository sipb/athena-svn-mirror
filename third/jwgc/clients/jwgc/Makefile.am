INCLUDES = -I../../lib/libxode/include -I../../lib/libjabber/include \
-I../../lib/libjwgc/include 
AM_CFLAGS = -DDATADIR=\"${sysconfdir}\"
AM_YFLAGS=-d
INSTANTIATE = $(srcdir)/instantiate

bin_PROGRAMS = jwgc
BUILT_SOURCES = port_dictionary.h pointer_dictionary.h ulong_dictionary.h \
string_dictionary.h int_dictionary.h char_stack.h string_stack.h xmode_stack.h \
parser.c parser.h port_dictionary.c pointer_dictionary.c ulong_dictionary.c \
string_dictionary.c int_dictionary.c jwgc.1 jwgc.html
jwgc_SOURCES = buffer.c character_class.c parser.y \
error.c eval.c exec.c file.c formatter.c jabber_handler.c \
jwgc_variables.c lexer.c main.c mux.c new_string.c node.c notice.c port.c \
regexp.c standard_ports.c status.c string_dictionary_aux.c substitute.c \
text_operations.c tty_filter.c variables.c xcut.c X_driver.c xerror.c \
X_fonts.c X_gram.c xmark.c xrevstack.c xselect.c xshow.c port_dictionary.c \
pointer_dictionary.c ulong_dictionary.c string_dictionary.c int_dictionary.c \
jwgc_handler.c
jwgc_LDADD = $(jwgc_DEPENDENCIES) ../../lib/libjwgc/libjwgc.a \
../../lib/libjabber/libjabber.a ../../lib/libxode/libxode.a
sysconf_DATA = jwgc.desc jwgc_resources jwgc.vars
man_MANS = jwgc.1
EXTRA_DIST = dictionary.c buffer.h lexer.h port.h xerror.h character_class.h \
main.h regexp.h X_fonts.h dictionary.h mit-copyright.h stack.h X_gram.h \
error.h mux.h string_dictionary_aux.h xmark.h eval.h new_string.h \
substitute.h xrevstack.h exec.h node.h text_operations.h xselect.h file.h \
notice.h variables.h xshow.h formatter.h pointer.h X_driver.h instantiate \
parser.h $(sysconf_DATA) jwgc.pod jwgc.html $(man_MANS) \
port_dictionary.h pointer_dictionary.h ulong_dictionary.h string_dictionary.h \
int_dictionary.h char_stack.h string_stack.h xmode_stack.h

jwgc.1: jwgc.pod
	if [ -x "`which pod2man`" ]; then pod2man --section=1 --release=$(VERSION) --center="Jabber Windowgram Client" --date="`date --iso-8601`" jwgc.pod jwgc.1; else echo "Unable to find pod2man to create documentation."; fi

jwgc.html: jwgc.pod
	if [ -x "`which pod2html`" ]; then pod2html --infile jwgc.pod --outfile jwgc.html; else echo "Unable to find pod2html to create documentation."; fi

port_dictionary.c port_dictionary.h: dictionary.c dictionary.h string_stack.h
	$(INSTANTIATE) $(srcdir) dictionary port port.h

pointer_dictionary.c pointer_dictionary.h: dictionary.c dictionary.h
	$(INSTANTIATE) $(srcdir) dictionary pointer pointer.h

ulong_dictionary.c ulong_dictionary.h: dictionary.c dictionary.h
	$(INSTANTIATE) $(srcdir) dictionary ulong sysdep.h

string_dictionary.c string_dictionary.h: dictionary.c dictionary.h
	$(INSTANTIATE) $(srcdir) dictionary string new_string.h

int_dictionary.c int_dictionary.h: dictionary.c dictionary.h
	$(INSTANTIATE) $(srcdir) dictionary int

char_stack.h: stack.h
	$(INSTANTIATE) $(srcdir) stack char

string_stack.h: stack.h
	$(INSTANTIATE) $(srcdir) stack string

xmode_stack.h: stack.h
	$(INSTANTIATE) $(srcdir) stack xmode
