#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/Makefile,v $
#	$Author: ilham $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/Makefile,v 1.7 1990-07-13 14:12:57 ilham Exp $
#
#
#     Copyright 1990 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".

DESTDIR=

# NDBM is now in config.h
CFLAGS=-O -DHESIOD -DKERBEROS -DZEPHYR -DV1COMPAT \
		-I./ncs/include/idl -I. -I..
LIBS= ./ncs/nck/libnck.a  -lhesiod \
	-lzephyr -lacl -lkrb -ldes -lcom_err
DBLIBS            = -lkrb

NIDL=./ncs/nidl/nidl
NIDLOPTS=-idir ./ncs/include/idl -m
.SUFFIXES: .idl .h
CPP=./ncs/cpp/cpp

.c.o:
	 ${CPP} $(CFLAGS) $*.c x.c; $(CC) -c $(CFLAGS) x.c; mv x.o $*.o

.idl.h:
	$(NIDL) $*.idl $(NIDLOPTS)


SRCS=	gquota_dba.c uid_strings.c \
	qmain.c quota_dba.c quota_ncs_sstub.c quota_server.c quotacap.c \
	server_krb.c quota_notify.c quota_logger.c strings.c parser.c \
	journal.c user.c periodic.c logger_ncs_sstub.c logger_server.c \
	${V1SRC}

OBJS=	qmain.o quota_dba.o quota_ncs_sstub.o quota_server.o quotacap.o \
	server_krb.o quota_notify.o quota_logger.o strings.o parser.o \
	journal.o user.o periodic.o logger_ncs_sstub.o logger_server.o \
	gquota_dba.o uid_strings.o ${V1OBJ}

V1SRC=  quota_ncs_v1_sstub.c quota_server_v1.c \
	logger_ncs_v1_sstub.c logger_server_v1.c

V1OBJ=  quota_ncs_v1_sstub.o quota_server_v1.o \
	logger_ncs_v1_sstub.o logger_server_v1.o

CSRCS=	lpquota.c quota_ncs_cstub.c quota_ncs_cswtch.c quota_err.c \
	logger_ncs_cstub.c logger_ncs_cswtch.c printcap.c
COBJS=	lpquota.o quota_ncs_cstub.o quota_ncs_cswtch.o quota_err.o \
	logger_ncs_cstub.o logger_ncs_cswtch.o printcap.o

LSRCS=  quota_dba.c gquota_dba.c qdb_create.c
LOBJS=  quota_dba.o gquota_dba.o qdb_create.o

SSRCS=  s_chkaf.c quota_ncs_cstub.c quota_ncs_cswtch.c printcap.c 
SOBJS=  s_chkaf.o quota_ncs_cstub.o quota_ncs_cswtch.o printcap.o

DBPROGS         = dump_db restore_db bill_db
DBGPROGS        = gdump_db grestore_db gbill_db
DBPROGRAMS      = $(DBPROGS) $(DBGPROGS)

DBSRCS          = dump_db.c restore_db.c bill_db.c \
                  gdump_db.c grestore_db.c gbill_db.c \
                  quota_dba.c gquota_dba.c
DBOBJS          = dump_db.o restore_db.o bill_db.o \
                  gdump_db.o grestore_db.o gbill_db.o \
                  quota_dba.o gquota_dba.o


ALL=	lpqd lpquota qdb_create s_chkaf $(DBPROGRAMS)

all:	./ncs/cpp/cpp ./ncs/nidl/nidl ./ncs/nck/libnck.a ${ALL} 

./ncs/cpp/cpp:
	(cd ncs/cpp; make MFLAGS=$(MFLAGS) )

./ncs/nidl/nidl:
	(cd ncs/nidl; make MFLAGS=$(MFLAGS) )

./ncs/nck/libnck.a:
	(cd ncs/nck; make MFLAGS=$(MFLAGS) )

s_chkaf: ${SOBJS}
	${CC} -o s_chkaf ${SOBJS} ${LIBS}

lpqd:	${OBJS}
	if [ `/bin/athena/machtype` = "decmips" ]; then \
		${CC} -o lpqd ${OBJS} ${LIBS} -ldbm ; \
	else \
		${CC} -o lpqd ${OBJS} ${LIBS} ; \
	fi

lint_lpqd: ${SRCS}
	lint -u ${CFLAGS}  ${SRCS}

saber_lpqd:
	#load ${CFLAGS} ${SRCS} ${LIBS}

saber_lpquota:
	#load ${CFLAGS} ${CSRCS} ${LIBS}

lpquota: ${COBJS}
	${CC} -o lpquota ${COBJS} ${LIBS}

consist: consist.o journal.o user.o parser.o strings.o
	${CC} -o consist consist.o journal.o user.o parser.o strings.o ${LIBS}

lint_lpquota:
	lint -u ${CFLAGS} lpquota.c 

lint_s_chkaf:
	lint -u ${CFLAGS} s_chkaf.c

qdb_create: ${LOBJS}
	${CC} -o qdb_create ${LOBJS} ${LIBS}


$(DBOBJS):
	$(CC) $(CFLAGS) -c $*.c

$(DBPROGS): quota_dba.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $* $*.o quota_dba.o $(DBLIBS)

$(DBGPROGS): gquota_dba.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $* $*.o gquota_dba.o $(DBLIBS)

dump_db: dump_db.o
gdump_db: gdump_db.o
restore_db: restore_db.o
grestore_db: grestore_db.o
bill_db: bill_db.o
gbill_db: gbill_db.o


install:
	install -s -c lpquota ${DESTDIR}/usr/athena/lpquota
	install -s -c lpqd ${DESTDIR}/usr/etc/lpqd
	install -s -c s_chkaf ${DESTDIR}/usr/etc/s_chkaf

clean:
	rm -f quota_ncs_sstub.c quota_ncs_cswtch.c quota_ncs_cstub.c \
	   logger_ncs_sstub.c logger_ncs_cswtch.c logger_ncs_cstub.c \
	   logger_ncs.h quota_ncs.h quota_err.[ch] *.o *~ ${ALL} \
	   x.c printcap.c \
	   quota_ncs_v1_sstub.c quota_ncs_v1_cswtch.c quota_ncs_v1_cstub.c \
	   logger_ncs_v1_sstub.c logger_ncs_v1_cswtch.c logger_ncs_v1_cstub.c \
	   logger_ncs_v1.h quota_ncs_v1.h nidl_structs.h \
	   nidl_structs_sstub.c nidl_structs_cstub.c nidl_structs_cswtch.c \
	   $(DBOBJS) $(DBPROGRAMS)

depend: ${SRCS} ${CSRCS} ${LSRCS}
	touch Make.depend; mkdep -fMake.depend ${CFLAGS} ${SRCS} \
	${CSRCS} ${LSRCS} ${DBSRCS}

printcap.c:
	rm -f printcap.c
	ln -s ../printcap.c 

quota_ncs_sstub.c: quota_ncs.h
quota_ncs_cstub.c: quota_ncs.h
quota_ncs_cswtch.c: quota_ncs.h
quota_ncs.h: quota_ncs.idl

logger_ncs_sstub.c: logger_ncs.h
logger_ncs_cstub.c: logger_ncs.h
logger_ncs_cswtch.c: logger_ncs.h
logger_ncs.h: logger_ncs.idl logger.h

quota_err.h quota_err.c:quota_err.et
	compile_et quota_err.et

qmain.o: quota_ncs.h logger_ncs.h quota_ncs_v1.h logger_ncs_v1.h  
#qmain.o: quota_ncs.h logger_ncs.h
quota_server.o: quota_err.h
parser.o: logger_ncs.h logger_ncs_v1.h
#parser.o: logger_ncs.h

$(OBJS): config.h
####### For version_1 compatibility ########

quota_ncs_v1_sstub.c: quota_ncs_v1.h nidl_structs.h
quota_ncs_v1_cstub.c: quota_ncs_v1.h nidl_structs.h
quota_ncs_v1_cswtch.c: quota_ncs_v1.h nidl_structs.h 
quota_ncs_v1.h: quota_ncs_v1.idl nidl_structs.h

logger_ncs_v1_sstub.c: logger_ncs_v1.h logger_ncs_v1_sstub.c.save
	cp logger_ncs_v1_sstub.c.save logger_ncs_v1_sstub.c
logger_ncs_v1_cstub.c: logger_ncs_v1.h logger_ncs_v1_cstub.c.save
	cp logger_v1_ncs_cstub.c.save logger_ncs_v1_cstub.c
logger_ncs_v1_cswtch.c: logger_ncs_v1.h logger_ncs_v1_cswtch.c.save
	cp logger_ncs_v1_cswtch.c.save logger_ncs_v1_cswtch.c
logger_ncs_v1.h: logger_ncs_v1.idl logger_ncs_v1.h.save logger.h
	cp logger_ncs_v1.h.save logger_ncs_v1.h
