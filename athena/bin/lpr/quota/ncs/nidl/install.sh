#!/bin/sh
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
# Install NIDL from a staging area (Unix)
#

#
# Create directories to install NIDL into.
#
mkdir /usr/local
mkdir /usr/local/bin
mkdir /usr/include/idl
mkdir /usr/include/idl/c

cd nidl
/usr/bin/install -c nidl /usr/local/bin
cd ..

#
# Install standard IDL files for NCS interfaces and their derived .h files
#
cd idl

set *.idl
for file in $@
do
    /usr/bin/install -c ${file} /usr/include/idl
done

set *.h
for file in $@
do
    /usr/bin/install -c ${file} /usr/include/idl/c
done

cd ..

mv /usr/include/idl/c/pfm.h  /usr/include
mv /usr/include/idl/c/ppfm.h /usr/include       

#
# Install man pages
#

cd man
for file in nidl
do
    if [ -d /usr/man/cat1 ]; then
        /usr/bin/install -c cat1/${file}.1 /usr/man/cat1/${file}.1
    fi
    if [ -d /usr/man/man1 ]; then
        /usr/bin/install -c man1/${file}.1 /usr/man/man1/${file}.1
    fi
done


echo NIDL installation completed.
