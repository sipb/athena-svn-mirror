INSTALL = install
CC = gcc
CFLAGS = -O2 -Wall
LD = ld

ALL_CFLAGS = $(CFLAGS) -fPIC
ALL_CPPFLAGS = $(CPPFLAGS)
ALL_LDFLAGS = $(LDFLAGS) -shared -Wl,-x
ALL_LDLIBS = $(LDLIBS)

ALL_LDFLAGS += -L/usr/lib/afs
ALL_LDLIBS += -lprot -lauth -lrxkad -lubik -lsys -lrx -llwp -lafsutil

all: pam_athena_locker.so linktest

pam_athena_locker.so: pam_athena_locker.o
	$(CC) -o $@ $(ALL_LDFLAGS) $^ $(LOADLIBES) $(ALL_LDLIBS)

%.o: %.c
	$(CC) -c $(ALL_CFLAGS) $(ALL_CPPFLAGS) $<

linktest: pam_athena_locker.so
	$(LD) --entry=0 -o /dev/null $^ -lpam

install: pam_athena_locker.so
	$(INSTALL) -d $(DESTDIR)/lib/security
	$(INSTALL) -m a+r,u+w pam_athena_locker.so $(DESTDIR)/lib/security/

clean:
	rm -f *.so *.o

.PHONY: all linktest install clean
