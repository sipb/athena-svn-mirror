
# Copyright 1990, 1992 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/instlmisc.sh,v 1.1.1.1 1996-10-07 20:25:22 ghudson Exp $

if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

if [ ! -d $PSLIBDIR ]
then
    mkdir $PSLIBDIR
fi

cd $PSLIBDIR

rm -f *.pro bogusmsg.ps ehandler.ps Notice ps?f psbad.sh psint.sh 
echo "Prologs.."
cp $SRCDIR/lib/*.pro .
echo "error handlers.."
cp $SRCDIR/lib/bogusmsg.ps .
cp $SRCDIR/lib/ehandler.ps .
cp $SRCDIR/lib/Notice .
cp $SRCDIR/lib/psbad.sh .

echo "AFM files."
if [ ! -d /usr/psres ]
then
	mkdir /usr/psres
fi
if [ ! -d /usr/psres/tsafm ]
then
	mkdir /usr/psres/tsafm
fi

cp $SRCDIR/lib/*.afm /usr/psres/tsafm
cp $SRCDIR/lib/afmfiles.upr /usr/psres

if [ $PREVIEWLOC ]
then
    echo $PREVIEWLOC >$PSLIBDIR/preview.info
    chmod 644 $PSLIBDIR/preview.info
fi


if [ $OPSYS = "BSD" ]
then
    cp $SRCDIR/lib/banner.bsd	 banner.pro
    cp $BUILDDIR/lib/psint.sh    psint.sh
    if [ $LINKS = "TRUE" ]
    then
        ln -s psint.sh psif
        ln -s psint.sh psof
        ln -s psint.sh psnf
        ln -s psint.sh pstf
        ln -s psint.sh psgf
        ln -s psint.sh psvf
        ln -s psint.sh psdf
        ln -s psint.sh pscf
        ln -s psint.sh psrf
        ln -s psbad.sh psbad
    else
        ln psint.sh psif
        ln psint.sh psof
        ln psint.sh psnf
        ln psint.sh pstf
        ln psint.sh psgf
        ln psint.sh psvf
        ln psint.sh psdf
        ln psint.sh pscf
        ln psint.sh psrf
        ln psbad.sh psbad
   fi
else
    cp $SRCDIR/lib/banner.sysv   banner.pro
fi

chown $OWNER *
chgrp $GROUP *
chmod 644 *.afm *.ps Notice *.pro *.upr > /dev/null 2>&1
chmod 755 *.sh

chown $OWNER /usr/psres/tsafm /usr/psres/tsafm/* /usr/psres/afmfiles.upr
chgrp $GROUP /usr/psres/tsafm /usr/psres/tsafm/* /usr/psres/afmfiles.upr
chmod 644 /usr/psres/tsafm/* /usr/psres/afmfiles.upr


