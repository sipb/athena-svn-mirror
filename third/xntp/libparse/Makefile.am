#AUTOMAKE_OPTIONS = no-dependencies
noinst_LIBRARIES = @MAKE_LIBPARSE@ @MAKE_LIBPARSE_KERNEL@
EXTRA_LIBRARIES = libparse.a libparse_kernel.a
noinst_PROGRAMS = @MAKE_PARSEKMODULE@
EXTRA_PROGRAMS = parse
CLEANFILES = libparse.a libparse_kernel.a

libparse_a_SOURCES = parse_conf.c clk_meinberg.c clk_schmid.c \
	parse.c clk_rawdcf.c clk_trimtsip.c \
	clk_dcf7000.c clk_trimtaip.c clk_rcc8000.c clk_hopf6021.c \
	clk_computime.c

libparse_kernel_a_SOURCES = kparse_conf.c clk_meinberg.c clk_schmid.c \
	kparse.c kclk_rawdcf.c kclk_trimtsip.c \
	clk_dcf7000.c clk_trimtaip.c clk_rcc8000.c clk_hopf6021.c \
	clk_computime.c

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/kernel
ETAGS_ARGS = Makefile.am

EXTRA_DIST = parsesolaris.c parsestreams.c

kparse_conf.o: parse_conf.c
kparse.o: parse.c
kclk_rawdcf.o: clk_rawdcf.c
kclk_trimtsip.o: clk_trimtsip.c

parsestreams.o: parsestreams.c
	$(CC) -c -DKERNEL -DVDDRV $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(srcdir)/parsestreams.c

parsestreams.loadable_module.o: parsestreams.o libparse_kernel.a
	$(LD) -r -o $@ parsestreams.o libparse_kernel.a

parsesolaris.o: parsesolaris.c
	$(CC) -c -D_KERNEL  $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(srcdir)/parsesolaris.c

parse: parsesolaris.o libparse_kernel.a
	$(LD) -r -o $@ parsesolaris.o libparse_kernel.a
