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
# Install NCK from a staging area (Unix)
#

echo "Installing NCK"

INSTALL=${INSTALL-/usr/bin/install}
CC=${CC-/bin/cc}
RANLIB=${RANLIB-/usr/bin/ranlib}

#
# If "libcg" exists, we must be on a VAX -- use it instead of "libc".
#

if [ -f /usr/lib/libcg.a ]; then
    LIBC=-lcg
    LIBM=-lmg
    echo "*** Using VAX G-float C and math libraries"
else
    LIBC=-lc
    LIBM=-lm
    echo "*** Using standard C and math library"
fi

#
# Install include and IDL files
#

cd idl
echo "*** Installing NCK .h and .idl files"

if [ ! -d /usr/include/idl ]; then
    mkdir /usr/include/idl 
fi
if [ ! -d /usr/include/idl/c ]; then
    mkdir /usr/include/idl/c
fi
for file in *.h; do
    ${INSTALL} -c ${file} /usr/include/idl/c
done
for file in *.idl; do 
    ${INSTALL} -c ${file} /usr/include/idl
done
mv /usr/include/idl/c/pfm.h  /usr/include
mv /usr/include/idl/c/ppfm.h /usr/include       

cd ..

#
# Install libnck.a, stcode.db (into /usr/lib), and the utilities (into
# /etc/ncs).  We build the final binaries for the runnable programs in
# the field to make sure we get the latest C runtime library.  
#

cd nck
echo "*** Installing NCK library and commands"

${INSTALL} -c stcode.db /usr/lib
${INSTALL} -c libnck.a /usr/lib
${RANLIB} /usr/lib/libnck.a

if [ ! -d /etc/ncs ]; then
    mkdir /etc/ncs
fi

if [ -f glbd ]; then
    GLBSTUFF="glbd drm_admin"
fi

for file in nrglbd llbd lb_admin uuid_gen stcode ${GLBSTUFF}; do
    if [ -f ${file}-out.o ]; then
        ${CC} -o ${file}.new ${file}-out.o ${LIBC}
        xfile=${file}.new
    else
    if [ -f ${file} ]; then
        xfile=${file}
    fi
    fi
    echo "    Installing ${file}"
    ${INSTALL} ${xfile} /etc/ncs/${file}
done

cd ..

#
# Install CPS
#

cd cps
echo "*** Installing CPS"

if [ -f libcps.a ]; then
    ${INSTALL} -c libcps.a /usr/lib
    ${RANLIB} /usr/lib/libcps.a
    if [ ! -d /usr/include/cps ]; then
        mkdir /usr/include/cps
    fi
    for file in *.h; do
        ${INSTALL} -c ${file} /usr/include/cps
    done;
fi

cd ..

#
# Install man pages into /usr/man/{cat?,man?}.
#

cd man
echo "*** Installing man pages"

for file in lb_admin uuid_gen llbd nrglbd; do
    if [ -d /usr/man/cat8 ]; then
        ${INSTALL} -c cat8/${file}.8 /usr/man/cat8/${file}.8
    fi
    if [ -d /usr/man/man8 ]; then
        ${INSTALL} -c man8/${file}.8 /usr/man/man8/${file}.8
    fi
done

cd ..

#
# Install perf client and server into /etc/ncs/perf
#

cd perf
echo "*** Installing perf"

if [ ! -d /etc/ncs/perf ]; then
    mkdir /etc/ncs/perf
fi

for file in client server; do
    ${CC} -o ${file}.new ${file}-out.o ${LIBC} ${LIBM}
    echo "    Installing perf/${file}"
    ${INSTALL} ${file}.new /etc/ncs/perf/${file}
done
${INSTALL} -c run_client /etc/ncs/perf/run_client

cd ..

#
# Install lb_test into /etc/ncs/perf
#

cd lb_test
echo "*** Installing lb_test"

for file in lb_test; do
    ${CC} -o ${file}.new ${file}-out.o ${LIBC}
    ${INSTALL} ${file}.new /etc/ncs/${file}
done

cd ..

echo "NCK installation complete"

