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

#
# MS/DOS makefile for NIDL.
#

# *************************************************************************        

CC              = cl
LIB             = lib
LINK            = link

TMP             = c:\ncs\tmp

# *************************************************************************        

IDL             = ..\idl

OPTFLAG         =                       # Optimization: -O[dastx]
CMODELFLAG      = /AL                   # Storage model: -A[SMCLH]
CDEBUGFLAG      =                       # Codeview stuff: -Zi
CFLAGS          = /Gt6 /I$(IDL) \
                  $(CMODELFLAG) $(CDEBUGFLAG) $(OPTFLAG)


# *************************************************************************        

.c.obj:
	$(CC) $(CFLAGS) /c $*.c >$*.txt
	type $*.txt

# *************************************************************************        

OBJS = \
	astp.obj \
	backend.obj \
	checker.obj \
	cspell.obj \
	errors.obj \
	files.obj \
	frontend.obj \
	getflags.obj \
	main.obj \
	nametbl.obj \
	pspell.obj \
	sysdep.obj \
	utils.obj \
	lex_yy.obj \
	y_tab.obj

astp.obj:

backend.obj:

checker.obj:

cspell.obj:

errors.obj:

files.obj:

frontend.obj:

getflags.obj:

main.obj:

nametbl.obj:

pspell.obj:

sysdep.obj:

utils.obj:

lex_yy.obj:
                               
y_tab.obj:

# Make sure tmp directory exists
$(TMP):
	xcopy nidl.mak $(TMP)
	del $(TMP)\nidl.mak

nidl.exe: $(OBJS)
	del nidl.exe
	echo astp+cspell+backend+pspell+errors+    >$(TMP)\nidl.lnk
	echo files+frontend+lex_yy+y_tab+checker+ >>$(TMP)\nidl.lnk
	echo getflags+main+nametbl+sysdep+utils   >>$(TMP)\nidl.lnk
	echo nidl.exe                             >>$(TMP)\nidl.lnk
	echo nidl.map /STACK:8000 /MAP;           >>$(TMP)\nidl.lnk
	$(LINK) @$(TMP)\nidl.lnk

