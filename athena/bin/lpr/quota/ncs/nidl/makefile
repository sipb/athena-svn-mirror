#
# NIDL Makefile
#

# 
#  ========================================================================== 
#  Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
#  Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
#  Copyright Laws Of The United States.
# 
#  Apollo Computer Inc. reserves all rights, title and interest with respect 
#  to copying, modification or the distribution of such software programs and
#  associated documentation, except those rights specifically granted by Apollo
#  in a Product Software Program License, Source Code License or Commercial
#  License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
#  Licensee.  Without such license agreements, such software programs may not
#  be used, copied, modified or distributed in source or object code form.
#  Further, the copyright notice must appear on the media, the supporting
#  documentation and packaging as set forth in such agreements.  Such License
#  Agreements do not grant any rights to use Apollo Computer's name or trademarks
#  in advertising or publicity, with respect to the distribution of the software
#  programs without the specific prior written permission of Apollo.  Trademark 
#  agreements may be obtained in a separate Trademark License Agreement.
#  ========================================================================== 
# 

IDL = ../include/idl
SHELL = /bin/sh
OS = BSD
EXTRACFLAGS =
CFLAGS = -DDEBUG -I$(IDL) -DYYDEBUG -D$(OS) $(EXTRACFLAGS)
YFLAGS = -d
LFLAGS =

.c.o:
	../cpp/cpp $(CFLAGS) $*.c x.c
	$(CC) -c $(CFLAGS) x.c
	mv x.o $*.o

NIDL_OBJECTS = \
	astp.o backend.o checker.o cspell.o errors.o files.o frontend.o \
	getflags.o main.o nametbl.o pspell.o sysdep.o utils.o y.tab.o lex.yy.o

NIDL_SOURCES = \
	ast.h astp.c astp.h backend.c backend.h checker.c checker.h cspell.c \
	cspell.h errors.c errors.h files.c files.h frontend.c frontend.h getflags.c \
	getflags.h main.c nametbl.c nametbl.h nidl.h nidl.l nidl.y pspell.c \
	sysdep.c sysdep.h utils.c utils.h

LEX_DERIVED = lex.yy.c

YACC_DERIVED = y.tab.c y.tab.h

VMS_SCRIPTS = bld1.com bldall.com install.com link_nidl.com nidl.opt stage.com

MSDOS_SCRIPTS = nidl.mak cut.bat install.bat stage.bat nidl.lnt \
	nidlcust.c yycustom.c

.SUFFIXES: .h .idl


nidl: $(NIDL_OBJECTS) 
	${CC} -o nidl $(NIDL_OBJECTS)

$(LEX_DERIVED): nidl.l y.tab.h $(IDL)/idl_base.h utils.h nametbl.h ast.h astp.h sysdep.h
	lex nidl.l
                               
$(YACC_DERIVED): nidl.y $(IDL)/base.h  $(IDL)/idl_base.h nametbl.h ast.h astp.h utils.h sysdep.h
	yacc $(YFLAGS) nidl.y

lex.yy.o: $(LEX_DERIVED) $(YACC_DERIVED)

astp.o: $(IDL)/idl_base.h utils.h nametbl.h ast.h astp.h sysdep.h $(IDL)/base.h $(IDL)/socket.h

backend.o: $(IDL)/rpc.h utils.h nametbl.h ast.h astp.h backend.h sysdep.h

checker.o: $(IDL)/base.h $(IDL)/idl_base.h utils.h ast.h astp.h checker.h frontend.h nametbl.h sysdep.h
         
cspell.o: ast.h astp.h cspell.h backend.h sysdep.h utils.h 

errors.o: $(IDL)/idl_base.h utils.h nametbl.h sysdep.h

files.o: $(IDL)/idl_base.h files.h nametbl.h sysdep.h

frontend.o: $(IDL)/idl_base.h files.h nametbl.h ast.h astp.h frontend.h backend.h utils.h getflags.h sysdep.h

getflags.o: $(IDL)/idl_base.h getflags.h sysdep.h

main.o: ast.h nidl.h

nametbl.o: $(IDL)/idl_base.h utils.h nametbl.h sysdep.h

pspell.o: $(IDL)/rpc.h nametbl.h ast.h astp.h backend.h sysdep.h utils.h

sysdep.o: $(IDL)/rpc.h sysdep.h utils.h

utils.o: $(IDL)/idl_base.h utils.h ast.h

clean:
	rm -f $(NIDL_OBJECTS) $(LEX_DERIVED) $(YACC_DERIVED)

clobber: clean
	rm -f nidl.exe

install: nidl
	install nidl /usr/local/bin
	-mkdir /usr/include/idl
	cp $(IDL)/nbase.idl $(IDL)/base.idl $(IDL)/socket.idl $(IDL)/uuid.idl $(IDL)/rpc.idl $(IDL)/conv.idl $(IDL)/rrpc.idl $(IDL)/[gl]lb.idl $(IDL)/lb.idl /usr/include/idl
	-mkdir /usr/include/idl/c
	cp $(IDL)/idl_base.h $(IDL)/nbase.h $(IDL)/base.h $(IDL)/socket.h $(IDL)/uuid.h $(IDL)/rpc.h $(IDL)/conv.h $(IDL)/[gl]lb.h $(IDL)/lb.h /usr/include/idl/c


msdos_source: $(NIDL_SOURCES) $(LEX_DERIVED) $(YACC_DERIVED)
	-mkdir msdos_source
	cp $(NIDL_SOURCES) msdos_source
	cp $(MSDOS_SCRIPTS) msdos_source
	sed -e "s/y.tab.h/y_tab.h/" lex.yy.c > msdos_source/lex_yy.c
	sed \
		-e "s/short \(yy.*\[\]=\)/static short huge \1/" \
		-e "s/#endif not lint/#endif/" \
		y.tab.c > msdos_source/y_tab.c
	cp y.tab.h msdos_source/y_tab.h
	@echo 
	@echo A ./msdos_source directory has been created\; to build NIDL under MSDOS
	@echo copy it to a directory named NIDL on your MSDOS system and copy the
	@echo directory ../idl to a sibling directory named IDL\; then use NIDL\\nidl.mak
	@echo to build NIDL with Microsoft Make and Microsoft C under MSDOS.  You should
	@echo increase the number of open files specified in your config.sys,
	@echo 
	@echo     FILES = 32


vms_source: $(NIDL_SOURCES) $(LEX_DERIVED) $(YACC_DERIVED)
	-mkdir vms_source
	cp $(NIDL_SOURCES) vms_source
	cp $(VMS_SCRIPTS) vms_source
	sed \
		-e "s/\"stdio.h\"/<stdio.h>/" \
		-e "s/ ={stdin}//" \
		-e "s/ ={stdout}//"  \
		-e "s/y.tab.h/y_tab.h/" \
		lex.yy.c > vms_source/lex_yy.c
	cp y.tab.c vms_source/y_tab.c
	cp y.tab.h vms_source/y_tab.h
	@echo 
	@echo A ./vms_source directory has been created\; to build NIDL under VMS copy
	@echo it to a directory named NIDL on your VMS system and copy the directory
	@echo ../idl to a sibling directory named IDL\; then use [NIDL]bldall.com to
	@echo build NIDL under VMS.  You will need to define the logical name idl\$dev
	@echo to point the IDL directory.
