#!/bin/sh

# Copyright 1990,1992 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/instlprogs.sh,v 1.1.1.1 1996-10-07 20:25:49 ghudson Exp $


#
#  must run as root, because of the chowning and chgrping.
#
if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

BINPROGS="enscript ps4014 ps630 pscat pscatmap psdit psplot pslpr psnup \
	  psfax psdraft" 

echo $BINPROGS

LIBPROGS="map psbanner pscomm psdman asc85ec lzwec"
if [ $QMS = "TRUE" ]
then
    LIBPROGS="$LIBPROGS qmscomm"
fi
if [ $PARALLEL = "TRUE" ]
then
    LIBPROGS="$LIBPROGS lpcomm"
fi
if [ $FASTPORT = "TRUE" ]
then
    LIBPROGS="$LIBPROGS fpcomm"
    if [ $SCOSERIAL = "TRUE" ]
    then
        LIBPROGS="$LIBPROGS fpcomm.ser"
    fi
fi

if [ $CAP = "TRUE" ]
then
    LIBPROGS="$LIBPROGS capcomm"
fi

echo "Installing BINDIR programs."
if [ ! -d $BINDIR ]
then
    mkdir $BINDIR
fi

cd $BINDIR
echo "Removing old copies."
rm -f $BINPROGS
echo "Copying in new versions."
for i in $BINPROGS
do
    echo $i
    cp $BUILDDIR/src/$i $i
    strip $i
done
if [ $BUILDPRINTPANEL = "TRUE" ]
then
    echo "ppanel"
    cp $BUILDDIR/src/printpanel/ppanel ppanel
    strip $i
fi
echo "Chown, chgrp, and chmod."
chown $OWNER $BINPROGS
chgrp $GROUP $BINPROGS
chmod 755 $BINPROGS
if [ $BUILDPRINTPANEL = "TRUE" ]
then
    chown $OWNER ppanel
    chgrp $GROUP ppanel
    chmod 755 ppanel
fi

echo "Installing PSLIBDIR programs."
if [ ! -d $PSLIBDIR ]
then
    mkdir $PSLIBDIR
fi

cd $PSLIBDIR
echo "Removing old copies."
rm -f $LIBPROGS
echo "Copying in new versions."
for i in $LIBPROGS
do
    echo $i
    if [ $i = "fpcomm" -o $i = "fpcomm.ser" ]
    then
        if [ $SCOFASTPORT = "TRUE" ] 
        then
            cp $BUILDDIR/src/milan.sco/$i $i
        else
            cp $BUILDDIR/src/milan/$i $i
        fi
    else
        cp $BUILDDIR/src/$i $i
    fi
    strip $i
done
echo "Chown, chgrp, and chmod."
chown $OWNER $LIBPROGS
chgrp $GROUP $LIBPROGS
chmod 755 $LIBPROGS

echo "Installing ptroff and psroff shell scripts."
cp $BUILDDIR/sh/ptroff $BINDIR
cp $BUILDDIR/sh/psroff $BINDIR
chown $OWNER $BINDIR/ptroff $BINDIR/psroff
chgrp $GROUP $BINDIR/ptroff $BINDIR/psroff
chmod 755 $BINDIR/ptroff $BINDIR/psroff

if [ $BUILDPRINTPANEL = "TRUE" ]
then
    echo "Installing resource file."
    cp $SRCDIR/lib/PrintPanelApp $XRES
    chown $OWNER $XRES/PrintPanelApp
    chgrp $GROUP $XRES/PrintPanelApp
    chmod 644 $XRES/PrintPanelApp
    
fi



