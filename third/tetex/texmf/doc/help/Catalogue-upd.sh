#!/bin/sh

umask 022
ctan=ftp.dante.de

: ${C=/t/ctan}

test -d $C/help/Catalogue || exit 1

rm -rf Catalogue
cp -a $C/help/Catalogue Catalogue
rm -rf Catalogue/*/*.xml Catalogue/support

sed 's@\.\./\.\./@'ftp://$ctan/pub/tex/@g < Catalogue/hier.html > Catalogue/hier.html-tmp$$
touch -r Catalogue/hier.html Catalogue/hier.html-tmp$$
rm -f Catalogue/hier.html
mv Catalogue/hier.html-tmp$$ Catalogue/hier.html

for i in Catalogue/*/*.html; do
  echo $i
  sed 's@\.\./\.\./\.\./@'ftp://$ctan/pub/tex/@g < $i > $i-tmp$$
  touch -r $i $i-tmp$$
  rm -f $i
  mv $i-tmp$$ $i
done
test -f Catalogue/index.html || ln -s catalogue.html Catalogue/index.html
