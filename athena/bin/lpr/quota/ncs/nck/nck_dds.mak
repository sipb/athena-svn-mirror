# ========================================================================== 
# Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
# Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
# Copyright Laws Of The United States.
# 
# Apollo Computer Inc. reserves all rights, title and interest with respect
# to copying, modification or the distribution of such software programs
# and associated documentation, except those rights specifically granted
# by Apollo in a Product Software Program License, Source Code License
# or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
# Apollo and Licensee.  Without such license agreements, such software
# programs may not be used, copied, modified or distributed in source
# or object code form.  Further, the copyright notice must appear on the
# media, the supporting documentation and packaging as set forth in such
# agreements.  Such License Agreements do not grant any rights to use
# Apollo Computer's name or trademarks in advertising or publicity, with
# respect to the distribution of the software programs without the specific
# prior written permission of Apollo.  Trademark agreements may be obtained
# in a separate Trademark License Agreement.
# ========================================================================== 


#
# MS/DOS makefile for NCK DDS-family specific library.  Depends on [make] 
# entry in TOOLS.INI.
#

CFLAGS = $(BASECFLAGS) /DDDS

O = dds

# *************************************************************************        

$(O)\ms_ddsa.obj: ms_ddsa.asm
	$(BUILD_ASM)

$(O)\xporta.obj: xporta.asm
	$(BUILD_ASM)

# *************************************************************************        

$(O)\ms_dds.obj: $*.c msdos.h
	$(BUILD_C)

$(O)\xport.obj: $*.c 
	$(BUILD_C)

$(O)\socket.obj: $*.c 
	$(BUILD_C)

$(O)\socket_d.obj: $*.c 
	$(BUILD_C)

# *************************************************************************        

nck_dds.lib: $(O)\ms_$(O).obj $(O)\ms_$(O)a.obj $(O)\xport.obj $(O)\xporta.obj \
    $(O)\socket.obj $(O)\socket_d.obj
	del nck_dds.lib
	$(LIB) nck_dds.lib +$(O)\ms_dds+$(O)\ms_ddsa+$(O)\xport+$(O)\xporta+$(O)\socket+$(O)\socket_d;
