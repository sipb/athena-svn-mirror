# makefile: Constructs the Kerberos for Windows ticket manager
OBJS = cns.obj tktlist.obj

##### Options
DEBUG = 0
NAME = cns

##### C Compiler
CC = cl
INCLUDES = /I..\include
CFLAGS_RELEASE = /f- /nologo /W3 /AL /Gw /Gy /Zp /O2 /DNDEBUG=1
CFLAGS_DEBUG = /f /nologo /W3 /AL /Gw /Gy /Zp /O2 /Od /Zi
!if $(DEBUG)
CFLAGS = $(CFLAGS_DEBUG) $(INCLUDES)
!else
CFLAGS = $(CFLAGS_RELEASE) $(INCLUDES)
!endif

##### RC Compiler
RC = rc
RFLAGS = $(INCLUDES)

##### Linker
LINK = link
LIBS = ..\lib\krb\kerberos.lib ..\lib\krb\winsock.lib
SYSLIBS = libw llibcew
!if $(DEBUG)
LFLAGS = /co /nod /nopackcode /map:full
!else
LFLAGS = /nod /nopackcode
!endif

all: makefile $(NAME).exe

$(NAME).exe: $*.def $*.res $(OBJS) $(LIBS)
	copy ..\lib\krb\kerberos.lib a.lib
	copy ..\lib\krb\winsock.lib  b.lib
	$(LINK) $(LFLAGS) $(OBJS), $@, $*.map, a.lib b.lib $(SYSLIBS), $*.def
	del a.lib
	del b.lib
	$(RC) $(RFLAGS) /k $*.res $@

install:
	copy cns.exe ..\floppy

clean:
	-del *.obj
	-del *.exe
	-del *.res
        -del *.map
	-del *.pdb
	-del *.err
	-del ..\floppy\cns.exe

cns.res: cns.h tktlist.h

cns.obj: cns.h tktlist.h

tktlist.obj: cns.h tktlist.h
