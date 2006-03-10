INCLUDES = -Iinclude -I../libxode/include -I../libjabber/include
AM_CFLAGS = -DDATADIR=\"${sysconfdir}\"
noinst_LIBRARIES = libjwgc.a
libjwgc_a_SOURCES = \
JContact.c      JExpat.c        JHashTable.c    JSha.c          JXMLComm.c \
JDebug.c        JFile.c         JNetSock.c      JStr.c \
JEncryption.c   JForm.c         JQuery.c        JVariables.c
CLEANFILES = Makefile
EXTRA_DIST = include/libjwgc.h include/libjwgc_debug.h include/libjwgc_types.h
