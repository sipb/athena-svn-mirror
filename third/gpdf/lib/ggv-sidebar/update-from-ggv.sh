#!/bin/sh

function die() {
  echo $*
  exit 1
}

if test -z "$GGVDIR"; then
   echo "Must set GGVDIR"
   exit 1
fi

if test -z "$GGVFILES"; then
   echo "Must set GGVFILES"
   exit 1
fi

for FILE in $GGVFILES; do
  if cmp -s $GGVDIR/$FILE `basename $FILE`; then
     echo "File $FILE is unchanged"
  else
     cp $GGVDIR/$FILE . || die "Could not copy $GGVDIR/$FILE"
     echo "Updated $FILE"
  fi
done
