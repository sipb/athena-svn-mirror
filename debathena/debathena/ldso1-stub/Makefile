all: ld-linux-1

ld-linux-1: ld-linux-1.c
	$(CC) -static -fPIC -m32 -o ld-linux-1 ld-linux-1.c

install: all
	install -d $(DESTDIR)/lib
	install -m755 ld-linux-1 $(DESTDIR)/lib/ld-linux.so.1

clean:
	-rm ld-linux-1

.PHONY: clean all
