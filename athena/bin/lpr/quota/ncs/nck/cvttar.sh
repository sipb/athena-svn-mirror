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
# Script to convert the NCK source tar file into a tar file whose contents
# are appropriate for MS/DOS.  Conversions include renaming the necessary
# files to have MS/DOS-valid names, removing file that will confuse MS/DOS
# (e.g. VMS .com files), and changing "/n" to "/r/n" in batch files (other
# files don't need conversion).
#
# usage:  cvttar <input tar file name> <output tar file name>
# 
# ==========================================================================

if [ "$2" = "" ]; then
    OUTTAR=/tmp/pcnck.tar 
else
    OUTTAR=$2
fi

# ==========================================================================

echo "Untaring $1"
cd /tmp

mkdir nck$$
cd nck$$
tar xf $1

# ==========================================================================

echo "Renaming long-named files"

mv nck/balanced_trees.h    nck/b_trees.h 
mv nck/balanced_trees.c    nck/b_trees.c 
mv nck/rpc_client.c        nck/rpc_c.c 
mv nck/rpc_server.c        nck/rpc_s.c 
mv nck/socket_dds.c        nck/socket_d.c 
mv nck/socket_inet.c       nck/socket_i.c 

# ==========================================================================

echo "Renaming NIDL-generated files"

for dir in `echo *`
do
    for name in `echo ${dir}/*.idl`
    do
        bname=${dir}/`basename ${name} .idl`
        if [ -f ${bname}_cstub.c ]; then
            mv ${bname}_cstub.c  ${bname}_c.c
            mv ${bname}_sstub.c  ${bname}_s.c
            mv ${bname}_cswtch.c ${bname}_w.c
        fi
    done
done

# ==========================================================================

echo "Deleting unneeded files"

rm -f */*.com */*.sh */*.o */*.*.*
rm -r cpp         

# ==========================================================================

echo "Creating new directory structure"

mkdir nck/nck nck/dds nck/ftp nck/xln 
mkdir perf/dds perf/ftp perf/xln
mkdir lb_test/dds lb_test/ftp lb_test/xln

# ==========================================================================

echo "Building CR-LF hacks"

cat > /tmp/icr$$.c << EOFEOF
#include <stdio.h>

main()
{
    char c;

    while ((c = getchar()) != EOF) {
        if (c == '\012')
            putchar('\015');
        putchar(c);
    }
}
EOFEOF

cc -o /tmp/icr$$ /tmp/icr$$.c

# ==========================================================================

cat > /tmp/icr$$.sh << EOFEOF
/tmp/icr$$ <\$1 >/tmp/icr\$\$.dat
cp /tmp/icr\$\$.dat \$1
rm /tmp/icr\$\$.dat
EOFEOF

chmod +x /tmp/icr$$.sh

# ==========================================================================

echo "CR-LF converting .bat files"

find . -name '*.bat' -exec /tmp/icr$$.sh \{\} \;

# ==========================================================================

echo "CR-LF converting doc files"

find man -exec /tmp/icr$$.sh \{\} \;
/tmp/icr$$.sh cpyright
/tmp/icr$$.sh lb_test/readme

# ==========================================================================

echo "CR-LF converting .txt files"

find . -name '*.txt' -exec /tmp/icr$$.sh \{\} \;

rm /tmp/icr$$*

# ==========================================================================

echo "Retaring into ${OUTTAR}"

tar cf ${OUTTAR} .
cd ..

echo "Deleting untar'd tree"

rm -rf nck$$
