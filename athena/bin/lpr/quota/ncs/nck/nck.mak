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
# MS/DOS makefile for NCK library.  Depends on [make] entry in TOOLS.INI.
#

CFLAGS = $(BASECFLAGS)

O=nck

# *************************************************************************        

$(O)\msdosa.obj: $*.asm
	$(BUILD_ASM)

# *************************************************************************        

$(O)\msdos.obj: $*.c msdos.h
	$(BUILD_C)

$(O)\rpc_c.obj: $*.c 
	$(BUILD_C)

$(O)\rpc_s.obj: $*.c 
	$(BUILD_C)

$(O)\rpc_lsn.obj: $*.c 
	$(BUILD_C)

$(O)\rpc_util.obj: $*.c
	$(BUILD_C)

$(O)\float.obj: $*.c 
	$(BUILD_C)

$(O)\uuid.obj: $*.c 
	$(BUILD_C)

$(O)\u_pfm.obj: $*.c
	$(BUILD_C)

$(O)\lb.obj: $*.c
	$(BUILD_C)

$(O)\llb.obj: $*.c 
	$(BUILD_C)

$(O)\llb_man.obj: $*.c 
	$(BUILD_C)

$(O)\glb.obj: $*.c
	$(BUILD_C)

$(O)\error.obj: $*.c
	$(BUILD_C)

# *************************************************************************        

$(O)\conv_c.obj: $(IDL)\$*.c
	$(BUILD_IDL_C)

$(O)\conv_s.obj: $(IDL)\$*.c
	$(BUILD_IDL_C)

$(O)\llb_c.obj: $(IDL)\$*.c 
	$(BUILD_IDL_C)

$(O)\llb_s.obj: $(IDL)\$*.c 
	$(BUILD_IDL_C)

$(O)\rrpc_s.obj: $(IDL)\$*.c 
	$(BUILD_IDL_C)

$(O)\rrpc_c.obj: $(IDL)\$*.c 
	$(BUILD_IDL_C)

$(O)\glb_c.obj: $(IDL)\$*.c 
	$(BUILD_IDL_C)

# *************************************************************************        

nck.lib: \
    $(O)\msdos.obj      $(O)\msdosa.obj     \
    $(O)\rpc_c.obj      $(O)\rpc_s.obj      $(O)\rpc_util.obj \
    $(O)\float.obj      $(O)\uuid.obj       $(O)\u_pfm.obj \
    $(O)\error.obj      $(O)\lb.obj         $(O)\llb.obj \
    $(O)\glb.obj        $(O)\llb_man.obj    $(O)\conv_c.obj \
    $(O)\conv_s.obj     $(O)\llb_c.obj      $(O)\llb_s.obj \
    $(O)\rrpc_c.obj     $(O)\rrpc_s.obj     $(O)\glb_c.obj \
    $(O)\rpc_lsn.obj
	del nck.lib
	$(LIB) @nck.lbc
