#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/Makefile,v 1.5 1990-07-03 16:15:17 epeisach Exp $
#
#
#     Copyright 1990 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".
DESTDIR=

#CFLAGS=-g -DHESIOD -DKERBEROS -DZEPHYR -DNDBM -DMALTRACE_FNAME -DMALTRACE \
#		-I./ncs/include/idl -I. -I../gdbm-1.3 ../malloc
#LIBS= ./ncs/${MACHINE}lib/libnck.a ../gdbm-1.3/gdbm.a ../malloc/maltrace.a \
#	-lhesiod -lzephyr -lacl -lkrb -ldes -lcom_err

# NDBM is now in config.h
CFLAGS=-g -DHESIOD -DKERBEROS -DZEPHYR -DV1COMPAT \
		-I./ncs/include/idl -I. -I..
LIBS= ./ncs/nck/libnck.a  -lhesiod \
	-lzephyr -lacl -lkrb -ldes -lcom_err

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

LSRCS= quota_dba.c gquota_dba.c qdb_create.c
LOBJS= quota_dba.o gquota_dba.o qdb_create.o

SSRCS= s_chkaf.c quota_ncs_cstub.c quota_ncs_cswtch.c printcap.c 

SOBJS= s_chkaf.o quota_ncs_cstub.o quota_ncs_cswtch.o printcap.o

ALL=	lpqd lpquota qdb_create s_chkaf

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

gdb_test: gquota_dba.o gdb_test.o
	${CC} -o gdb_test gquota_dba.o gdb_test.o ${LIBS}

dump_db: dump_db.o quota_dba.o
	${CC} -o dump_db dump_db.o quota_dba.o ${LIBS}

saber_gdb_test:
	#load ${CFLAGS} gquota_dba.c gdb_test.c ${LIBS}

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
	   nidl_structs_sstub.c nidl_structs_cstub.c nidl_structs_cswtch.c

depend: ${SRCS} ${CSRCS} ${LSRCS}
	touch Make.depend; mkdep -fMake.depend ${CFLAGS} ${SRCS} \
	${CSRCS} ${LSRCS}

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

logger_ncs_sstub_v1.c: logger_ncs_v1.h
logger_ncs_cstub_v1.c: logger_ncs_v1.h
logger_ncs_cswtch_v1.c: logger_ncs_v1.h
logger_ncs_v1.h: logger_ncs_v1.idl logger.h
