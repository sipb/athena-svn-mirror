
# Copyright 1990 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/adddit.sh,v 1.1.1.1 1996-10-07 20:25:21 ghudson Exp $

# Add additional fonts for use by ditroff.  This script requires that 
# builddit.sh has been run at least once to create the Times family.
# This script does not build in place; i.e. the build site used in
# builddit.sh must still exist, since adding a new font for use by ditroff
# involves adding it to all existing font families.  After adddit.sh, you
# should run instldit.sh again to re-install all the font families.

# Adddit.sh takes as arguments the name of the font family, the
# two-character names you want to use to access the font in ditroff (in the
# order roman (the "roman" or "regular" version of the font), italic, bold,
# and bold-italic.  If a font family does not have one of these font faces,
# substitute the Times equivalents (TR TI TB TD).  You should also create a
# map file for each of these two-character name, named with the
# two-character name and the extension .map, e.g. PA.map.  This map file
# should contain the "real" name of the font, e.g. Palatino-Roman.  If
# these map files are not located in the $SRCDIR/lib/ditroff.font
# directory, you should specify the directory in which to find them.

# adddit.sh familyname Rname Iname Bname BIname [mapdir]



if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

DESCFONTS="R I B BI H HB C CB S SS"

family=$1
r=$2
i=$3
b=$4
bi=$5

if [ $# = "6" ]
then
    dir=$6
else
    dir=$SRCDIR/lib/ditroff.font
fi

if [ $family = "Times" ]
then
    echo "Times can't be added in with this script; must be done at"
    echo "initial installation"
    exit 0
fi

cd ditroff

# do Times first, we'll build off of that
echo "Building font in Times family first"
cwd=`pwd`
cd Times/devpsc
timesdir=`pwd`


if [ $family = "Courier" ]
then
    nolig="-n"
else
    nolig=""
fi
for j in $r $i $b $bi
do
    cp $dir/$j.map .
    realname=`head -1 $j.map`
    afmfilename=`$BUILDDIR/src/map $realname`
    $BUILDDIR/src/afmdit -a $afmfilename -o $j \
                         -m $j.map -x $j.aux $nolig
    $MAKEDEV $j
    if [ $j = "C" ]
    then
        ln C CW
        ln C.aux CW.aux
        ln C.map CW.map
        ln C.out CW.out
    fi
done

cd $cwd

echo "Add to other font families."
for others in `ls`
do
    if [ $others = "Times" ]
    then
        continue
    fi
    if [ $others = $family ]
    then
        continue
    fi
    echo $others
    cd $others/devpsc
    ln $timesdir/$r* .
    ln $timesdir/$i* .
    ln $timesdir/$b* .
    ln $timesdir/$bi* .
    cd $cwd
done

echo "Building $family"
rm -rf $family
mkdir $family $family/devpsc
cd $family/devpsc
ln $timesdir/* .
for s in '' .aux .map .out
do
    rm -rf R$s I$s B$s BI$s
    ln $timesdir/$r$s R$s
    ln $timesdir/$i$s I$s
    ln $timesdir/$b$s B$s
    ln $timesdir/$bi$s BI$s
done
for t in R I B BI
do
    mv $t /tmp/fam.$$
    rm $t.out
    sed -e "/^name/s/.*/name $t/" /tmp/fam.$$ > $t
done
rm DESC.out
$MAKEDEV DESC
rm -f /tmp/fam.$$

































