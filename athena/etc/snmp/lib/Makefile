#
# Makefile to build NYSERNet snmp library
#
# Compile src files
LIBDIRS=	parser builder comm server util obj

all:
	@for i in $(LIBDIRS) ; do \
		cd $$i; \
		make LIB="$(LIB)" CFLAGS="$(CFLAGS)" ; \
		cd .. ; \
	done

install:
#	@for i in $(LIBDIRS) ; do \
#		cd $$i ; \
#		make LIB="$(LIB)" CFLAGS="$(CFLAGS)" install ; \
#		cd .. ; \
#	done

clean:
	@for i in $(LIBDIRS) ; do \
		cd $$i ; \
		make clean ; \
		cd .. ; \
	done
