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
# MS/DOS makefile for NCK utilities.  Depends on [make] entry in TOOLS.INI.
#

CFLAGS      = $(BASECFLAGS)
LIBNCK      = nck.lib
LIBNET      = nck_$(NET).lib
EXTRALIBS   =
LIBS        = $(LIBNCK) $(LIBNET) $(EXTRALIBS)

O=nck

# *************************************************************************        

$(O)\uuid_gen.obj: $*.c
	$(BUILD_C)

$(NET)\uuid_gen.exe: $(O)\uuid_gen.obj $(LIBS)
	$(LINK) $(O)\uuid_gen,$(NET)\uuid_gen,$(NET)\uuid_gen,$(LIBS) $(LINKFLAGS)

# *************************************************************************        

$(O)\stcode.obj: $*.c
	$(BUILD_C)

$(O)\stcode.exe: $(O)\stcode.obj $(LIBNCK)
	$(LINK) $(O)\stcode,$(O)\stcode,$(O)\stcode,$(LIBNCK) $(LINKFLAGS)

# *************************************************************************        

$(O)\lb_admin.obj: $*.c
	$(BUILD_C)

$(O)\lb_args.obj: $*.c
	$(BUILD_C)

$(O)\uname.obj: $*.c
	$(BUILD_C)

$(O)\b_trees.obj: $*.c
	$(BUILD_C)

$(NET)\lb_admin.exe: $(O)\lb_admin.obj $(O)\lb_args.obj $(O)\uname.obj \
                     $(O)\b_trees.obj $(LIBS)
	$(LINK) $(O)\lb_admin+$(O)\lb_args+$(O)\uname+$(O)\b_trees,$(NET)\lb_admin,$(NET)\lb_admin,$(LIBS) $(LINKFLAGS)

# *************************************************************************        

$(O)\glbd.obj: $*.c
	$(BUILD_C)

$(O)\glb_man.obj: $*.c
	$(BUILD_C)

$(O)\glb_s.obj: $(IDL)\$*.c 
	$(BUILD_IDL_C)

$(NET)\nrglbd.exe: $(O)\glbd.obj $(O)\glb_man.obj $(O)\glb_s.obj $(LIBS)
	$(LINK) $(O)\glbd+$(O)\glb_man+$(O)\glb_s,$(NET)\nrglbd,$(NET)\nrglbd,$(LIBS) $(LINKFLAGS)

