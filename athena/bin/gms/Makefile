#  This file is part of the Project Athena Global Message System.
#  Created by: Mark W. Eichin <eichin@athena.mit.edu>
#  $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/Makefile,v $
#  $Author: raeburn $
# 
# 	Copyright (c) 1988 by the Massachusetts Institute of Technology.
# 	For copying and distribution information, see the file
# 	"mit-copyright.h". 
#
# $Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/Makefile,v 1.2 1988-11-01 16:38:48 raeburn Exp $
# Generic one project, one target makefile.
#

PROJECT= gms
TARGET= get_message
SERVER= messaged

CSRCS= get_a_message.c get_fallback_file.c get_message.c get_message_from_server.c get_servername.c gethost_errors.c hesiod_errors.c put_fallback_file.c read_to_memory.c view_message_by_tty.c view_message_by_zephyr.c check_viewable.c

ETSRCS= gethost_err.et globalmessage_err.et hesiod_err.et

ETINCS= gethost_err.h globalmessage_err.h hesiod_err.h

ETOBJS= gethost_err.o globalmessage_err.o hesiod_err.o

OBJS= get_a_message.o get_fallback_file.o get_message.o get_message_from_server.o get_servername.o gethost_errors.o hesiod_errors.o put_fallback_file.o read_to_memory.o view_message_by_tty.o view_message_by_zephyr.o check_viewable.o


INCLUDE= -I.

ALLSRCS= $(CSRCS) $(ETSRCS)


CFLAGS= ${INCLUDE}

DEPEND=/usr/athena/makedepend
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


$(LIB):	$(OBJS) $(ETOBJS)
	-rm -f $(LIB)
	ar cqv $(LIB) $(OBJS) $(ETOBJS)
	ranlib $(LIB)

.SUFFIXES: .o .h .et

$(ETINCS):	$(ETSRCS)
	$(COMPILE_ET) $*.et

$(ETOBJS):	$(ETSRCS)
	$(COMPILE_ET) $*.et

depend: $(ETINCS)
	${DEPEND} -v ${CFLAGS} -s'# DO NOT DELETE' $(CSRCS) $(MODS)

# DO NOT DELETE THIS LINE

check_viewable.o: /usr/include/mit-copyright.h globalmessage.h
# globalmessage.h includes:
#	errno.h
#	globalmessage_err.h
check_viewable.o: /usr/include/errno.h globalmessage_err.h
check_viewable.o: /usr/include/strings.h /usr/include/sys/types.h
check_viewable.o: /usr/include/sys/file.h /usr/include/pwd.h
get_a_message.o: /usr/include/mit-copyright.h globalmessage.h
get_a_message.o: /usr/include/errno.h globalmessage_err.h
get_fallback_file.o: /usr/include/mit-copyright.h globalmessage.h
get_fallback_file.o: /usr/include/errno.h globalmessage_err.h
get_fallback_file.o: /usr/include/sys/file.h
get_message.o: /usr/include/mit-copyright.h globalmessage.h
get_message.o: /usr/include/errno.h globalmessage_err.h /usr/include/stdio.h
get_message.o: /usr/include/sys/types.h
get_message_from_server.o: /usr/include/mit-copyright.h globalmessage.h
get_message_from_server.o: /usr/include/errno.h globalmessage_err.h
get_message_from_server.o: /usr/include/sys/types.h /usr/include/sys/socket.h
get_message_from_server.o: /usr/include/netinet/in.h /usr/include/netdb.h
get_message_from_server.o: /usr/include/hesiod.h /usr/include/sys/time.h
# /usr/include/sys/time.h includes:
#	time.h
get_message_from_server.o: /usr/include/sys/time.h
get_servername.o: /usr/include/mit-copyright.h globalmessage.h
get_servername.o: /usr/include/errno.h globalmessage_err.h
get_servername.o: /usr/include/hesiod.h
gethost_errors.o: /usr/include/mit-copyright.h gethost_err.h
gethost_errors.o: /usr/include/netdb.h
hesiod_errors.o: /usr/include/mit-copyright.h hesiod_err.h
hesiod_errors.o: /usr/include/hesiod.h
message_daemon.o: /usr/include/mit-copyright.h globalmessage.h
message_daemon.o: /usr/include/errno.h globalmessage_err.h
message_daemon.o: /usr/include/fcntl.h
put_fallback_file.o: /usr/include/mit-copyright.h globalmessage.h
put_fallback_file.o: /usr/include/errno.h globalmessage_err.h
put_fallback_file.o: /usr/include/sys/file.h
read_to_memory.o: /usr/include/mit-copyright.h globalmessage.h
read_to_memory.o: /usr/include/errno.h globalmessage_err.h
view_message_by_tty.o: /usr/include/mit-copyright.h globalmessage.h
view_message_by_tty.o: /usr/include/errno.h globalmessage_err.h
view_message_by_zephyr.o: /usr/include/mit-copyright.h globalmessage.h
view_message_by_zephyr.o: /usr/include/errno.h globalmessage_err.h
view_message_by_zephyr.o: /usr/include/pwd.h /usr/include/stdio.h
view_message_by_zephyr.o: /usr/include/strings.h
