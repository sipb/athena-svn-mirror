INSTALL = install
CC = gcc
CFLAGS = -O2 -Wall
LD = ld

ALL_CFLAGS = $(CFLAGS) -fPIC
ALL_LDFLAGS = $(LDFLAGS) -shared -Wl,-x

all: pam_debathena_home_type.so linktest

pam_debathena_home_type.so: pam_debathena_home_type.o
	$(CC) -o $@ $(ALL_LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)

%.o: %.c
	$(CC) -c $(ALL_CFLAGS) $(CPPFLAGS) $<

linktest: pam_debathena_home_type.so
	$(LD) --entry=0 -o /dev/null $^ -lpam

install: pam_debathena_home_type.so
	$(INSTALL) -d $(DESTDIR)/lib/security
	$(INSTALL) -m a+r,u+w pam_debathena_home_type.so $(DESTDIR)/lib/security/

clean:
	rm -f *.so *.o

.PHONY: all linktest install clean
