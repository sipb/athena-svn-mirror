
# Copyright 1991 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/instltroff.sh,v 1.1.1.1 1996-10-07 20:25:22 ghudson Exp $

# Shell script for installing troff font width files.  If no arguments are
# given, install all font families specified in FONTFAMILIES in config,
# else install the font family specified on the command line.
# Must run as root, because of the chowning and chgrping.
#
# instltroff.sh [familyname]

if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

if [ $# = "2" ] 
then
     famlist=$2
else
     famlist=$FONTFAMILIES
fi

if [ $OPSYS = "SYSV" ]
then
     if [ ! -d /usr/lib/font ]
     then
         mkdir /usr/lib/font
     fi
     if [ ! -d /usr/lib/font/ps ]
     then
         mkdir /usr/lib/font/ps
     fi
     cd /usr/lib/font/ps
else
     if [ ! -d $TROFFFONTDIR ]
     then
         mkdir $TROFFFONTDIR
     fi
     cd $TROFFFONTDIR
fi

for i in $famlist
do
    rm -rf $i
    mkdir $i
    cp $BUILDDIR/troff/$i/* $i
    chmod 755 $i
    chmod 644 $i/*
    chown $OWNER $i $i/*
    chgrp $GROUP $i $i/*
done

chmod 755 . 
chown $OWNER . 
chgrp $GROUP . 

