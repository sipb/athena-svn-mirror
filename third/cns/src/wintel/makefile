# makefile: Constructs the kerborized telnet client
OBJS = telnet.obj negotiat.obj auth.obj edit.obj emul.obj \
   font.obj intern.obj screen.obj

##### Options
DEBUG = 0
NAME = telnet

##### C Compiler
CC = cl
INCLUDES = /I..\include
CFLAGS_RELEASE = /nologo /W3 /AL /GA /Gx /O1 /DNDEBUG
CFLAGS_DEBUG = /nologo /W3 /AL /GA /Gx /G2 /Zi /Od /D_DEBUG /Fd"TELNET.PDB"
!if $(DEBUG)
CFLAGS = $(CFLAGS_DEBUG) $(INCLUDES)
!else
CFLAGS = $(CFLAGS_RELEASE) $(INCLUDES)
!endif

##### RC Compiler
RC = rc
RFLAGS_RELEASE = 
RFLAGS_DEBUG = /D_DEBUG
!if $(DEBUG)
RFLAGS = $(RFLAGS_DEBUG) $(INCLUDES)
!else
RFLAGS = $(RFLAGS_RELEASE) $(INCLUDES)
!endif

##### Linker
LINK = link	 
LIBS = ..\lib\krb\kerberos.lib ..\lib\krb\winsock.lib
SYSLIBS = libw llibcew commdlg
!if $(DEBUG)
LFLAGS = /co /nologo /nod /packc:61440 /stack:32768 /align:16 /onerror:noexe
!else
LFLAGS = /nologo /nod /packc:61440 /stack:32768 /align:16 /onerror:noexe
!endif

all: makefile $(NAME).exe

$(NAME).exe: $*.def $*.res $(OBJS) $(LIBS)
	$(LINK) $(LFLAGS) $(OBJS), $@, $*.map, $(LIBS) $(SYSLIBS), $*.def
	$(RC) $(RFLAGS) /k $*.res $@

install:
	copy telnet.exe ..\floppy

clean:
	-del *.obj
	-del *.exe
	-del *.res
        -del *.map
	-del *.pdb
	-del *.err
	-del ..\floppy\telnet.exe

telnet.obj: telnet.h dialog.h screen.h struct.h proto.h ini.h ..\include\winsock.h 

negotiat.obj: telnet.h dialog.h screen.h struct.h proto.h ini.h ..\include\winsock.h 

auth.obj: telopts.h telnet.h dialog.h screen.h struct.h proto.h ini.h ..\include\winsock.h ..\include\kerberos.h 

edit.obj: screen.h

emul.obj: screen.h

font.obj: screen.h ini.h

intern.obj: screen.h

screen.obj: screen.h ini.h

telnet.res: screen.h dialog.h telnet.dlg ncsa.ico terminal.ico

