
# Copyright 1990 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/instldit.sh,v 1.1.1.1 1996-10-07 20:25:22 ghudson Exp $

#
#  must run as root, because of the chowning and chgrping.
#
#

if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

if [ ! -d $DITDIR ]
then
     mkdir $DITDIR
fi
cd $DITDIR

rm -rf *

for i in $FONTFAMILIES
do
    mkdir $i
    mkdir $i/devpsc
    cp $BUILDDIR/ditroff/$i/devpsc/* $i/devpsc
done

chmod 755 . * */devpsc
chmod 644 */devpsc/*

chown $OWNER . * */devpsc */devpsc/*
chgrp $GROUP . * */devpsc */devpsc/*

if [ $LINKS = "TRUE" ]
then
    rm -f /usr/lib/font/devpsc.old
    mv /usr/lib/font/devpsc /usr/lib/font/devpsc.old
    ln -s $DITDIR/$DITDEFAULT/devpsc /usr/lib/font/devpsc
else
    rm -rf /usr/lib/font/devpsc
    mkdir /usr/lib/font/devpsc
    ln $DIDIR/$DITDEFAULT/devpsc /usr/lib/font/devpsc
fi
