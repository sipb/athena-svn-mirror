
# Copyright 1991, 1992 (C) Adobe Systems Incorporated. All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/instlman.sh,v 1.1.1.1 1996-10-07 20:25:22 ghudson Exp $

# install man pages

COM="enscript ps4014 ps630 pscat psdit psfonts psplot psroff pssymbols \
ptroff transcript pslpr psnup psfax psdraft ppanel"

SYS="pscomm psdman pscatmap"

if [ -r ./config ] 
then
    . ./config
else
    echo "No config file!"
    exit 1
fi

if [ $QMS = "TRUE" ]
then
    SYS="$SYS qmscomm"
fi
if [ $PARALLEL = "TRUE" ]
then
    SYS="$SYS lpcomm"
fi
if [ $FASTPORT = "TRUE" ]
then
   SYS="$SYS fpcomm"
fi
if [ $CAP = "TRUE" ]
then
    SYS="$SYS capcomm"
fi
if [ $APPSOCKET = "TRUE" ]
then
    SYS="$SYS ascomm"
fi

FILES="afm postscript"

if [ $NROFF = "TRUE" ] 
then
    where=cat
else
    where=man
fi

if [ ! -d $where ]
then
    mkdir $where
fi

cd $where

for j in `ls $SRCDIR/$where`
do
    sed -e s,XPSLIBDIRX,$PSLIBDIR,g \
        -e s,XTROFFFONTDIRX,$TROFFFONTDIR,g \
        -e s,XPSTEMPDIRX,$PSTEMPDIR,g \
        -e s,XDITDIRX,$DITDIR,g \
        $SRCDIR/$where/$j > $j
done

if [ $OPSYS="BSD" ]
then
    rm pscomm.sysv
    mv pscomm.bsd pscomm.8p
    rm qmscomm.sysv
    mv qmscomm.bsd qmscomm.8p
    SYS="$SYS psint"
else
    rm pscomm.bsd
    mv pscomm.sysv pscomm.8p
    rm qmscomm.bsd
    mv qmscomm.sysv qmscomm.8p
    SYS="$SYS psinterface"
fi

for j in $COM
do
    cp $j.1p $MANCOM/$j.$EXTCOM
done
for j in $SYS
do
    cp $j.8p $MANSYS/$j.$EXTSYS
done
for j in $FILES
do
    cp $j.7p $MANFILES/$j.$EXTFILES
done


    
