
# Copyright 1990,1992 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/configure.sh,v 1.1.1.1 1996-10-07 20:25:22 ghudson Exp $


if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

if [ -d $BUILDDIR ]
then
    if [ $OPSYS = "SYSV" ]
    then
	echo "Remove already existing build directory? \c"
    else
	echo -n "Remove already existing build directory? "
    fi
    read check
    if [ $check = "n" ]
    then
	exit 1
    fi
    rm -rf $BUILDDIR
fi

mkdir $BUILDDIR
chmod 777 $BUILDDIR

cp ./config $BUILDDIR

if [ $OPSYS = "BSD" ]
then
	cp ./printer.bsd $BUILDDIR/printer
else
	cp ./printer.sysv $BUILDDIR/printer
fi

if [ $LINKS = "TRUE" ]
then
   ln -s $SRCDIR/buildprogs.sh $BUILDDIR/buildprogs.sh
   ln -s $SRCDIR/buildmisc.sh $BUILDDIR/buildmisc.sh
   ln -s $SRCDIR/instlmisc.sh $BUILDDIR/instlmisc.sh
   ln -s $SRCDIR/instlprogs.sh $BUILDDIR/instlprogs.sh
   ln -s $SRCDIR/buildtroff.sh $BUILDDIR/buildtroff.sh
   ln -s $SRCDIR/instltroff.sh $BUILDDIR/instltroff.sh
   ln -s $SRCDIR/builddit.sh $BUILDDIR/builddit.sh
   ln -s $SRCDIR/instldit.sh $BUILDDIR/instldit.sh
   ln -s $SRCDIR/instlman.sh $BUILDDIR/instlman.sh
   ln -s $SRCDIR/mkprinter.sh $BUILDDIR/mkprinter.sh
else
   cp $SRCDIR/buildprogs.sh $BUILDDIR/buildprogs.sh
   cp $SRCDIR/buildmisc.sh $BUILDDIR/buildmisc.sh
   cp $SRCDIR/instlmisc.sh $BUILDDIR/instlmisc.sh
   cp $SRCDIR/instlprogs.sh $BUILDDIR/instlprogs.sh
   cp $SRCDIR/buildtroff.sh $BUILDDIR/buildtroff.sh
   cp $SRCDIR/instltroff.sh $BUILDDIR/instltroff.sh
   cp $SRCDIR/builddit.sh $BUILDDIR/builddit.sh
   cp $SRCDIR/instldit.sh $BUILDDIR/instldit.sh
   cp $SRCDIR/instlman.sh $BUILDDIR/instlman.sh
   cp $SRCDIR/mkprinter.sh $BUILDDIR/mkprinter.sh
fi

