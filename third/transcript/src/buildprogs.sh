
# Copyright 1990, 1992 (C) Adobe Systems Incorporated.  All rights
# reserved. 
# GOVERNMENT END USERS: See notice of rights in Notice file in release
# directory. 
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/buildprogs.sh,v 1.1.1.1 1996-10-07 20:25:47 ghudson Exp $

if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

if [ ! -d src ]
then
    mkdir src
fi

cd src

if [ $BUILDPRINTPANEL = "TRUE" ]
then
     if [ ! -d printpanel ]
     then
          mkdir printpanel
     fi
fi
if [ $FASTPORT = "TRUE" ]
then
     if [ ! -d milan ]
     then
          mkdir milan
     fi
fi

if [ ! -f config.h ]
then
    echo "Creating config.h"
    echo "#ifndef CONFIG" > config.h
    echo "#define CONFIG 1" >> config.h
    echo "char *PSLibDir = \"$PSLIBDIR/\";" >> config.h
    echo "char *PPDDir = \"$PPDDIR/\";" >> config.h
    echo "char *TroffFontDir = \"$TROFFFONTDIR/$TROFFDEFAULT/\";" >> config.h
    echo "char *DitDir = \"$DITDIR/$DITDEFAULT/\";" >> config.h
    echo "char *TempDir = \"$PSTEMPDIR/\";" >> config.h
    echo "char *ResourceDir = \"$DEFRESPATH/\";" >> config.h
    echo "char *lp = \"$LP\";" >> config.h
    echo "char *bindir = \"$BINDIR/\";" >> config.h
    echo "#endif" >> config.h
fi

if [ ! -f .ready ] 
then
    if [ $LINKS = "TRUE" ]
    then
        echo "Linking source files."
        for i in `ls $SRCDIR/src`
        do
            if [ ! -d $SRCDIR/src/$i ]
            then
                ln -s $SRCDIR/src/$i $i
            fi
        done
        if [ $FASTPORT = "TRUE" ]
        then
            for i in `ls $SRCDIR/src/milan`
            do
                ln -s $SRCDIR/src/milan/$i milan/$i
            done
        fi            
        if [ $BUILDPRINTPANEL = "TRUE" ]
        then
            for i in `ls $SRCDIR/src/printpanel`
            do
                ln -s $SRCDIR/src/printpanel/$i printpanel/$i
            done
        fi            
        echo > .ready
    else
        echo "Copying source files."
        for i in `ls $SRCDIR/src`
        do
            if [ ! -d $SRCDIR/src/$i ]
            then
                 cp $SRCDIR/src/$i $i
            fi
        done
        if [ $BUILDPRINTPANEL = "TRUE" ]
        then
            for i in `ls $SRCDIR/src/printpanel`
            do
                cp $SRCDIR/src/printpanel/$i printpanel/$i
            done
        fi            
        echo > .ready
    fi
fi

if [ $OPSYS = "BSD" ]
then
    mv pscomm.bsd pscomm.c
    mv psbanner.bsd psbanner.c
    mv qmscomm.bsd qmscomm.c
else
    mv pscomm.sysv pscomm.c
    mv psbanner.sysv psbanner.c
    if [ $BSDSOCK = "TRUE" ]
    then 
        mv qmscomm.bsd qmscomm.c
        rm qmscomm.sysv
    else
        mv qmscomm.sysv qmscomm.c
    fi
fi

echo "Making programs."
FLAGS="$SETCFLAGS -D$OPSYS"
if [ $XPG3 = "TRUE" ]
then
    FLAGS="$FLAGS -DXPG3"
fi
CCFLAGS=$FLAGS
export CCFLAGS
make

if [ $QMS = "TRUE" ]
then
    CCFLAGS=$FLAGS
    export CCFLAGS
    if [ $OPSYS = "BSD" -o $BSDSOCK = "TRUE" ]
    then
	make -f Makeqms.bsd
    else
	make -f Makeqms.sysv
    fi
fi
if [ $PARALLEL = "TRUE" ]
then
    CCFLAGS=$FLAGS
    export CCFLAGS
    make -f Makelp
fi

if [ $FASTPORT = "TRUE" ] 
then
    CCFLAGS="$FLAGS"
    if [ $UDPSTATUS = "TRUE" ]
    then
        CCFLAGS="$CCFLAGS -DUDPSTATUS"
    fi
    if [ $SCOFASTPORT = "TRUE" ]
    then
        CCFLAGS="$CCFLAGS -DSCO"
    else
        cd milan
    fi
    if [ $SCOSERIAL = "TRUE" -a $SCOFASTPORT = "TRUE" ]
    then
        CCFLAGS="$CCFLAGS -DSLOWSCO"
    fi
    export RESOLVLIB CCFLAGS
    make 
    cd ..
fi

if [ $CAP = "TRUE" ]
then
   CCFLAGS="$FLAGS $CAPINCL"
   export CCFLAGS CAPLIB
   make -f Makecap
fi

if [ $APPSOCKET = "TRUE" ]
then
    CCFLAGS="$FLAGS"
    export CCFLAGS
    make -f Makeas
fi
    

if [ $BUILDPRINTPANEL = "TRUE" ] 
then
    CCFLAGS="$FLAGS $XINCLS"
    LDFLAGS="$XLIBS"
    export CCFLAGS LDFLAGS
    cd printpanel
    make
fi

echo "Handle shell scripts."
cd $BUILDDIR

if [ ! -d sh ]
then
    mkdir sh
fi

cd sh

if [ ! -f .ready ]
then
   if [ $OPSYS = "BSD" ]
   then
       cp $SRCDIR/sh/ptroff.bsd ptroff.sh
       cp $SRCDIR/sh/psroff.bsd psroff.sh
   else
       cp $SRCDIR/sh/ptroff.sysv ptroff.sh
       cp $SRCDIR/sh/psroff.sysv psroff.sh
   fi
   echo >.ready
fi

sed -e s,TROFFFONTDIR,$TROFFFONTDIR,g ptroff.sh >ptroff

if [ $DWB20 = "TRUE" ]
then
    sed -e s,DITDIR,$DITDIR,g -e s,DITFLAGS,,g psroff.sh >psroff
else
    sed -e s,DITDIR,$DITDIR,g -e s,DITFLAGS,-t,g psroff.sh > psroff
fi

