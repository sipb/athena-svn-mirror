
noinst_PROGRAMS = doesnothing

if WITH_BDB4
doesnothing_LDADD = ../src/libical/libical-evolution.la ../src/libicalss/libicalss-evolution.la ../src/libicalvcal/libicalvcal-evolution.la @BDB_DIR_LIB@/@BDB_LIB@
else
doesnothing_LDADD = ../src/libical/libical-evolution.la ../src/libicalss/libicalss-evolution.la ../src/libicalvcal/libicalvcal-evolution.la
endif

LIBS = @PTHREAD_LIBS@

INCLUDES = -I. 				\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/src		\
	-I$(top_builddir)/src		\
	-I$(top_srcdir)/src/libical	\
	-I$(top_builddir)/src/libical

doesnothing_SOURCES = \
	access_components.c \
	access_properties_and_parameters.c \
	errors.c \
	main.c \
	parse_text.c 

