include ../Makefile.config

sources = lrmi.c
objects = lrmi.o
all = vbetest mode3 vga_reset vbemodeinfo

all: $(all)

vbetest: vbetest.o lrmi.o

mode3: mode3.o lrmi.o

vbemodeinfo: vbemodeinfo.o lrmi.o

vga_reset: vga_reset.o lrmi.o

install: mode3 vga_reset
	install mode3 /sbin
	install vga_reset /sbin

.PHONY: clean all install
clean:
	$(RM) *.o $(all) core
