
# Copyright 1990 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/buildmisc.sh,v 1.1.1.1 1996-10-07 20:25:47 ghudson Exp $


if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

cd $BUILDDIR

if [ ! -d lib ]
then
    mkdir lib
fi

cd lib

if [ $OPSYS = "BSD" ]
then
    sed -e s,XPSLIBDIRX,$PSLIBDIR,g \
        -e s,XPSTEMPDIRX,$PSTEMPDIR,g \
        -e s,XBANNERFIRSTX,$BANNERFIRST,g \
        -e s,XBANNERLASTX,$BANNERLAST,g \
        -e s,XREVERSEX,$REVERSE,g \
        -e s,XVERBOSELOGX,$VERBOSELOG,g \
	-e s,XBINDIRX,$BINDIR,g \
       $SRCDIR/lib/psint.bsd >psint.sh
else
    sed -e s,XPSLIBDIRX,$PSLIBDIR,g \
        -e s,XPSTEMPDIRX,$PSTEMPDIR,g \
        -e s,XBANNERFIRSTX,$BANNERFIRST,g \
        -e s,XBANNERLASTX,$BANNERLAST,g \
        -e s,XREVERSEX,$REVERSE,g \
        -e s,XVERBOSELOGX,$VERBOSELOG,g \
	-e s,XBINDIRX,$BINDIR,g \
       $SRCDIR/lib/psint.sysv >psinterface
fi

         

