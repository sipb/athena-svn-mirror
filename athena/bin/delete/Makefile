TARGETS = delete undelete lsdel expunge purge
DESTDIR =
CC = cc
CFLAGS = -g
SRCS = delete.c undelete.c directories.c pattern.c util.c

all: $(TARGETS)

install:
	for i in $(TARGETS)
	do
	cp $i $(DESTDIR)
	strip $(DESDTIR)/$i
	done

delete: delete.o util.o
	cc $(CFLAGS) -o delete delete.o util.o

undelete: undelete.o directories.o util.o pattern.o
	cc $(CFLAGS) -o undelete undelete.o directories.o util.o pattern.o

clean:
	-rm -f *~ *.bak *.o delete undelete lsdel expunge purge

depend: delete.c undelete.c
	/usr/athena/makedepend -v $(CFLAGS) -s'# DO NOT DELETE' $(SRCS)

# DO NOT DELETE THIS LINE -- makedepend depends on it

delete.o: /usr/include/sys/types.h /usr/include/stdio.h
delete.o: /usr/include/sys/stat.h /usr/include/sys/dir.h
delete.o: /usr/include/strings.h /usr/include/sys/param.h
# /usr/include/sys/param.h includes:
#	machine/machparam.h
#	signal.h
#	sys/types.h
delete.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
delete.o: /usr/include/sys/file.h util.h delete.h
undelete.o: /usr/include/stdio.h /usr/include/sys/types.h
undelete.o: /usr/include/sys/dir.h /usr/include/sys/param.h
undelete.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
undelete.o: /usr/include/strings.h /usr/include/sys/stat.h directories.h
undelete.o: pattern.h util.h undelete.h
directories.o: /usr/include/sys/types.h /usr/include/sys/stat.h
directories.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
directories.o: /usr/include/sys/signal.h /usr/include/sys/dir.h
directories.o: /usr/include/strings.h /usr/include/errno.h directories.h
directories.o: util.h
pattern.o: /usr/include/stdio.h /usr/include/sys/types.h
pattern.o: /usr/include/sys/dir.h /usr/include/sys/param.h
pattern.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
pattern.o: /usr/include/strings.h /usr/include/sys/stat.h directories.h
pattern.o: pattern.h util.h undelete.h
util.o: /usr/include/stdio.h /usr/include/sys/param.h
util.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
util.o: /usr/include/sys/types.h /usr/include/sys/dir.h
util.o: /usr/include/strings.h util.h
