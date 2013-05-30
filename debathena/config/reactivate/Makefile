all: session-wrapper command-not-found.mo

session-wrapper: session-wrapper.c
	$(CC) -o session-wrapper session-wrapper.c

command-not-found.mo:
	msgfmt -o command-not-found.mo command-not-found.po

install: all
	install -d $(DESTDIR)/usr/lib/debathena-reactivate
	install -m4755 session-wrapper $(DESTDIR)/usr/lib/debathena-reactivate

clean:
	rm -f session-wrapper
	rm -f command-not-found.mo

.PHONY: clean all
