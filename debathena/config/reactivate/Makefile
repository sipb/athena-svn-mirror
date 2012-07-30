all: session-wrapper

session-wrapper: session-wrapper.c
	$(CC) -o session-wrapper session-wrapper.c

install: all
	install -d $(DESTDIR)/usr/lib/debathena-reactivate
	install -m4755 session-wrapper $(DESTDIR)/usr/lib/debathena-reactivate

clean:
	rm -f session-wrapper

.PHONY: clean all
