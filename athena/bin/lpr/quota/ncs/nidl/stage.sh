#!/bin/sh

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

# Run this script from the NIDL directory after building NIDL from the
# the source.  It creates ../stage, the "staging area" from which you
# can install NIDL and its associated files.

case $1 in
    sun) ;;
    ultrix) ;;
    *) 
        echo 'usage: stage.sh [sun|ultrix]'
        exit ;;
esac 

mkdir ../stage
mkdir ../stage/nidl
mkdir ../stage/idl
mkdir ../stage/man
mkdir ../stage/man/man1
mkdir ../stage/man/cat1

cp ../cpyright ../stage
cp install.sh ../stage
cp nidl ../stage/nidl

cp ../idl/*.idl ../stage/idl
cp ../idl/*.h ../stage/idl

cp ../man/man1/*.1 ../stage/man/man1
cp ../man/cat1/*.1 ../stage/man/cat1
cp ../man/doc/$1.rd ../stage/README

cp -r ../examples ../stage

