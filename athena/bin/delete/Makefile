TARGETS = delete undelete lsdel expunge purge
DESTDIR =
CC = cc
CFLAGS = -g
SRCS = delete.c undelete.c directories.c

all: $(TARGETS)

install:
	for i in $(TARGETS)
	do
	cp $i $(DESTDIR)
	strip $(DESDTIR)/$i
	done

delete: delete.o
	cc $(CFLAGS) -o delete delete.o

undelete: undelete.o directories.o
	cc $(CFLAGS) -o undelete undelete.o directories.o

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
delete.o: /usr/include/sys/file.h
undelete.o: /usr/include/stdio.h /usr/include/sys/types.h
undelete.o: /usr/include/sys/dir.h /usr/include/sys/param.h
undelete.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
undelete.o: /usr/include/strings.h /usr/include/sys/stat.h directories.h
directories.o: /usr/include/sys/types.h /usr/include/sys/stat.h
directories.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
directories.o: /usr/include/sys/signal.h /usr/include/sys/dir.h
directories.o: /usr/include/strings.h /usr/include/errno.h directories.h
