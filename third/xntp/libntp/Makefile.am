#AUTOMAKE_OPTIONS = no-dependencies
noinst_LIBRARIES = libntp.a
libntp_a_SOURCES = adjtime.c atoint.c atolfp.c atouint.c auth12crypt.c \
	authdecrypt.c authdes.c authencrypt.c authkeys.c authparity.c \
	authreadkeys.c authusekey.c buftvtots.c caljulian.c calleapwhen.c \
	caltontp.c calyearstart.c clocktime.c dofptoa.c dolfptoa.c emalloc.c \
	fptoa.c fptoms.c getopt.c hextoint.c hextolfp.c humandate.c inttoa.c \
	lib_strbuf.c memmove.c mfptoa.c mfptoms.c modetoa.c mstolfp.c \
	msutotsf.c numtoa.c refnumtoa.c numtohost.c octtoint.c prettydate.c \
	ranny.c tsftomsu.c tstotv.c tvtoa.c tvtots.c uglydate.c uinttoa.c \
	utvtoa.c machines.c clocktypes.c md5.c a_md5encrypt.c a_md5decrypt.c \
	a_md512crypt.c decodenetnum.c systime.c msyslog.c syssignal.c \
	findconfig.c netof.c statestr.c mexit.c
INCLUDES = -I$(top_srcdir)/include
ETAGS_ARGS = Makefile.am

noinst_HEADERS = lib_strbuf.h log.h

EXTRA_DIST = libntp.mak README $(HEADERS) adjtimex.c authdes.c.des authdes.c.export log.c makemc.bat messages.mc
