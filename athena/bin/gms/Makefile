#  This file is part of the Project Athena Global Message System.
#  Created by: Mark W. Eichin <eichin@athena.mit.edu>
#  $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/Makefile,v $
#  $Author: probe $
# 
# 	Copyright (c) 1988 by the Massachusetts Institute of Technology.
# 	For copying and distribution information, see the file
# 	"mit-copyright.h". 
#
# $Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/Makefile,v 1.6 1990-05-01 05:58:18 probe Exp $
# Generic one project, one target makefile.
#

PROJECT= gms
TARGET= get_message
SERVER= messaged

CSRCS= get_a_message.c get_fallback_file.c get_message.c get_message_from_server.c get_servername.c gethost_errors.c hesiod_errors.c put_fallback_file.c read_to_memory.c view_message_by_tty.c view_message_by_zephyr.c check_viewable.c

ETSRCS= gethost_err.et globalmessage_err.et hesiod_err.et
ETINCS= gethost_err.h globalmessage_err.h hesiod_err.h
ETOBJS= gethost_err.o globalmessage_err.o hesiod_err.o
ETCSRC= gethost_err.c globalmessage_err.c hesiod_err.c

OBJS= get_a_message.o get_fallback_file.o get_message.o get_message_from_server.o get_servername.o gethost_errors.o hesiod_errors.o put_fallback_file.o read_to_memory.o view_message_by_tty.o view_message_by_zephyr.o check_viewable.o


INCLUDE= -I.

ALLSRCS= $(CSRCS) $(ETSRCS)


CFLAGS= ${INCLUDE}

DEPEND=touch Make.depend; /usr/athena/makedepend -fMake.depend 
COMPILE_ET=/usr/athena/compile_et

LIB= lib${PROJECT}.a
LIBS= $(LIB) -lhesiod -lcom_err

all: $(TARGET) $(SERVER)

clean:
	-rm -f $(OBJS) $(LIB) $(TARGET) $(ETINCS) $(ETOBJS) 
	-rm -f $(SERVER) message_daemon.o

$(TARGET): $(TARGET).o $(LIB)
	$(CC) $(CFLAGS) -o $@ $(TARGET).o $(LIBS)

install: $(TARGET) $(SERVER)
	install -c -s $(TARGET) ${DESTDIR}/bin/athena/$(TARGET)
	install -c get_message.1 ${DESTDIR}/usr/man/man1
	install -c -s $(SERVER) ${DESTDIR}/etc/athena/$(SERVER)

server: $(SERVER)

MDFLAGS=-DGMS_SERVER_MESSAGE=\"/site/Message\"

message_daemon.o: message_daemon.c
	$(CC) $(CFLAGS) $(MDFLAGS) -c message_daemon.c

$(SERVER): message_daemon.o $(LIB)
	$(CC) $(CFLAGS) -o $@ message_daemon.o $(LIBS)


$(LIB):	$(ETOBJS) $(OBJS)
	-rm -f $(LIB)
	ar cqv $(LIB) $(OBJS) $(ETOBJS)
	ranlib $(LIB)

.SUFFIXES: .o .h .et

$(ETINCS):	$(ETSRCS)
	rm -f $*.c $*.h
	$(COMPILE_ET) $*.et

$(ETCSRC):	$(ETSRCS)
	rm -f $*.c $*.h
	$(COMPILE_ET) $*.et

$(ETOBJS):	$(ETINCS) $(ETCSRC)

depend: $(ETINCS)
	${DEPEND} -v ${CFLAGS} -s'# DO NOT DELETE' $(CSRCS) $(MODS)

