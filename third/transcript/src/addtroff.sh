
# Copyright 1991 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/addtroff.sh,v 1.1.1.1 1996-10-07 20:25:46 ghudson Exp $

# shell script to add additional troff font width files.
# It expects a familyname as the first argument, and a mapfilename as the 
# second argument.  This mapfile maps the typefaces of the family to the
# appropriate troff font holders.  See transcript/lib/troff.font/*.map 
# (from the distribution) for an example of a mapfile, and a description of
# the format.

# addtroff.sh familyname mapfilename

if [ -r ./config ]
then
    . ./config
else
    echo "config file missing!"
    exit 1
fi

family=$1
mapfile=$2

cd troff
mkdir $family
cd $family

first=R
second=I
third=B
fourth=S

filelist="R I B S"
set $1 $filelist

echo "Building $family with $mapfile"

cp $SRCDIR/lib/troff.font/chartab.inc .
$BUILDDIR/src/pscatmap $mapfile
rm -f chartab.inc
filelist=`awk -f $SRCDIR/lib/troff.font/doto.awk $mapfile`
echo $filelist
mv $family.ct font.ct
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
       awk -F, '/^[0-9]/ {print $1}' $file.c |\
            awk -F+ '{$1=$1+($2/100)*64;print $1}' > $file
   fi
fi
cp $SRCDIR/lib/troff.font/font.head .
chmod 644 font.head
if [ $family = "Courier" ]
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
