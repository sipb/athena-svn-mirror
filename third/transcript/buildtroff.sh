
# Copyright 1991 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/buildtroff.sh,v 1.1.1.1 1996-10-07 20:25:22 ghudson Exp $


if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

if [ ! -d troff ]
then
    mkdir troff
fi

cd troff

first=R
second=I
third=B
fourth=S

filelist="R I B S"
set $1 $filelist

for i in $FONTFAMILIES
do
    mkdir $i
    cd $i
    cp $SRCDIR/lib/troff.font/chartab.inc .
    $BUILDDIR/src/pscatmap $SRCDIR/lib/troff.font/$i.map
    rm -f chartab.inc
    filelist=`awk -f $SRCDIR/lib/troff.font/doto.awk $SRCDIR/lib/troff.font/$i.map`
    echo $filelist
    mv $i.ct font.ct
    if [ $OPSYS = "BSD" -a $TROFFASCIIFONTS != "TRUE" ]
    then
        for file in $filelist
        do
            cc -c $file.c
            mv $file.o $file
            strip $file
            rm $file.c
        done
     else
         if [ $TROFFASCIIFONTS = "TRUE" ]
         then
             for file in $filelist
             do
                 awk -F, '/^[0-9]/ {print $1}' $file.c |\
                      awk -F+ '{$1=$1+($2/100)*64;print $1}' > $file
                 rm $file.c
             done
         fi
     fi
     cp $SRCDIR/lib/troff.font/font.head .
     chmod 644 font.head
     if [ $i = "Courier" ]
     then
         echo ".lg 0" >> font.head
     fi
     set $1 $filelist
     if [ $2 != ft$first ]
     then
         ln $2 ft$first
     fi
     if [ $3 != ft$second ]
     then
          ln $3 ft$second
     fi
     if [ $4 != ft$third ]
     then
         ln $4 ft$third
     fi
     if [ $5 != ft$fourth ]
     then
         ln $5 ft$fourth
     fi
     cd ..
done
