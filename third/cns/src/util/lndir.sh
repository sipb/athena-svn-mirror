#!/bin/sh

DIRFROM=$1
DIRTO=$2

USAGE="Usage: $0 fromdir todir"

case "$DIRFROM" in
/*)	;;
*)	echo "dir \"$DIRFROM\" must begin with a /"
	exit 1
	;;
esac

if [ ! -d $DIRFROM -a ! -d $DIRTO ]
then
	echo "$USAGE"
	exit 1
fi

REALFROM=

DIRLIST=`(cd $DIRFROM; find * \( -type d ! -name 'RCS' ! -name 'CVS' \) -print)`

echo ====
echo $DIRLIST
echo ====

cd $DIRTO

if [ `(cd $DIRFROM; pwd)` = `pwd` ]
then
	echo "FROM and TO are identical!"
	exit 1
fi

for dir in $DIRLIST
do
	echo mkdir $dir
	mkdir $dir
done

for file in `ls $DIRFROM`
do
	if [ ! -d $DIRFROM/$file ]; then
		echo ln -s $DIRFROM/$file .
		ln -s $DIRFROM/$file .
	fi;
done

for dir in $DIRLIST
do
	echo $dir:
	(cd $dir;
	 if [ `(cd $DIRFROM/$dir; pwd)` = `pwd` ]
	 then
		echo "FROM and TO are identical!"
		exit 1
	 fi;
	 for file in `ls $DIRFROM/$dir`
	 do
	 if [ ! -d $DIRFROM/$dir/$file ]; then
		echo ln -s $DIRFROM/$dir/$file .
		ln -s $DIRFROM/$dir/$file .
	 fi;
	 done)
done
