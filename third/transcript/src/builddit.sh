
# Copyright 1990 (C) Adobe Systems Incorporated.  All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/builddit.sh,v 1.1.1.1 1996-10-07 20:25:47 ghudson Exp $

if [ -r ./config ]
then
    . ./config
else 
    echo "config file missing!"
    exit 1
fi

DESCFONTS="R I B BI H HB C CB S SS"

TimesMap="R I B BI TR TI TB TD"
HelveticaMap="H HO HB HD"
CourierMap="C CO CB CD"
HelvNarrowMap="hn hN Hn HN"
AvantGardeMap="ag aG Ag AG"
BookmanMap="bo bO Bo BO"
GaramondMap="ga gA Ga GA"
LubalinMap="lu lU Lu LU"
NewCenturyMap="nc nC Nc NC"
OptimaMap="op oP Op OP"
PalatinoMap="PA PI PB PX"
SouvenirMap="sv sV Sv SV"
ZapfMap="ZC"


if [ ! -d ditroff ]
then
    mkdir ditroff
fi

cd ditroff

echo "Creating DESC"
rm -f DESC
echo "# ditroff device description for PostScript" >> DESC
echo "# PostScript is a registered trademark of Adobe Systems Incorporated" >>DESC
echo $DESCFONTS | awk '{print "fonts", NF, $0}' >> DESC
cat $SRCDIR/lib/ditroff.font/devspecs >>DESC
echo "charset" >> DESC
cat $SRCDIR/lib/ditroff.font/charset >> DESC

echo "Building Times-Roman"
# do Times first, we'll build off of that
cwd=`pwd`
rm -rf Times
mkdir Times
cd Times
mkdir devpsc
cd devpsc
timesdir=`pwd`
cp ../../DESC .


for i in $FONTFAMILIES
do
    if [ $i = "Courier" ]
    then
        nolig="-n"
    else
        nolig=""
    fi
    ditfonts=\$${i}Map
    ditlist=`eval echo $ditfonts`
    for j in $ditlist
    do
        cp $SRCDIR/lib/ditroff.font/$j.map .
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
done

# handle special fonts (Symbol and DIThacks)
afmfilename=`$BUILDDIR/src/map Symbol`
cp $SRCDIR/lib/ditroff.font/S.map .
$BUILDDIR/src/afmdit -a $afmfilename -o S -m S.map -x S.aux
$MAKEDEV S

afmfilename=`$BUILDDIR/src/map DIThacks`
cp $SRCDIR/lib/ditroff.font/SS.map .
$BUILDDIR/src/afmdit -a $afmfilename -o SS -m SS.map -x SS.aux
$MAKEDEV SS
   
$MAKEDEV DESC

cd $cwd

set $1 $TimesMap
for i in $FONTFAMILIES
do
    if [ $i = "Times" ]
    then
           continue
    fi
    echo "Building $i"
    rm -rf $i
    mkdir $i $i/devpsc
    cd $i/devpsc
    ln $timesdir/* .
    ditfonts=\$${i}Map
    ditlist=`eval echo $ditfonts`
    if [ $i = "Zapf" ]
    then
        ditlist="ZC TR TB TD"
    fi
    set $1 $ditlist
    for s in '' .aux .map .out
    do
        rm -rf R$s I$s B$s BI$s
        ln $timesdir/$2$s R$s
        ln $timesdir/$3$s I$s
        ln $timesdir/$4$s B$s
        ln $timesdir/$5$s BI$s
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
    cd $cwd
done

rm DESC


































