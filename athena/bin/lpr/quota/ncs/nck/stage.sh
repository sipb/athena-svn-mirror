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
# Make an NCK staging area (Unix)
#
                                 
STAGE=../stage

/bin/mkdir ${STAGE} ${STAGE}/man ${STAGE}/nck ${STAGE}/perf ${STAGE}/lb_test
/bin/mkdir ${STAGE}/idl ${STAGE}/man/man8 ${STAGE}/man/cat8 
/bin/mkdir ${STAGE}/cps

/bin/cp ../cpyright ${STAGE}

/bin/cp ../nck/install.sh   ${STAGE}    

/bin/cp ../nck/libnck.a ../nck/stcode.db ${STAGE}/nck

/bin/cp ../nck/uuid_gen ../nck/lb_admin ../nck/nrglbd ../nck/llbd ../nck/stcode ${STAGE}/nck     
/bin/cp ../nck/uuid_gen-out.o ../nck/lb_admin-out.o ../nck/nrglbd-out.o ../nck/llbd-out.o ../nck/stcode-out.o ${STAGE}/nck

/bin/cp ../idl/*.h ${STAGE}/idl
/bin/cp ../idl/*.idl ${STAGE}/idl

/bin/cp ../perf/client-out.o ../perf/server-out.o ${STAGE}/perf
/bin/cp ../perf/client ../perf/server ${STAGE}/perf
/bin/cp ../perf/run_client ${STAGE}/perf

/bin/cp ../lb_test/lb_test ../lb_test/lb_test-out.o ${STAGE}/lb_test    

/bin/cp ../man/man8/*.8 ${STAGE}/man/man8
/bin/cp ../man/cat8/*.8 ${STAGE}/man/cat8

if [ -f /bin/sun ]; then
    /bin/cp ../man/doc/sun.rd    ${STAGE}/README
else
    /bin/cp ../man/doc/ultrix.rd ${STAGE}/README
fi

CPS=../../cps
if [ -d ${CPS} ]; then
    /bin/cp ${CPS}/libcps.a ${STAGE}/cps
    /bin/cp ${CPS}/cps/cpsio.h ${CPS}/cps/mutex.h ${CPS}/cps/ec2.h ${CPS}/cps/task.h ${CPS}/cps/time.h ${STAGE}/cps 
else
    echo "Warning: CPS tree not found -- not staging CPS"
fi

GLB=../../glb
if [ -d ${GLB} ]; then
    /bin/cp ${GLB}/glb/glbd ${GLB}/glb/drm_admin ${GLB}/glb/libdrm.a ${STAGE}/nck
else
    echo "Warning: GLB tree not found -- not staging GLB"
fi
