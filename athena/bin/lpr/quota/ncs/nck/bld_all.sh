#!/bin/sh
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
# Build the NCK subsystem (Unix)
#

# Figure out whether we have CPS

HAVECPS=${HAVECPS-"no"}

# If on Ultrix or a Sun-[23], then we have CPS
if [ -f /usr/lib/libcg.a -o \( -f /bin/sun -a -f /bin/m68k \) ]; then
    HAVECPS=yes
fi

# Build basic NCK
build.sh

# Build perf
cd ../perf
build.sh ../idl ../nck

# Build lb_test
cd ../lb_test
build.sh ../idl ../nck

cd .. 

# If we have CPS, build the replicated GLB as well, if it's there.
if [ -d ../glb -a ${HAVECPS} = "yes" ]; then
    cd ../glb/glb
    echo "Building replicated GLB"
    make
fi
