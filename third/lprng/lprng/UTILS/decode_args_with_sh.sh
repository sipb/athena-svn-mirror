#!/bin/sh 
# this is an example of how to use /bin/sh and LPRng
# to get the options
# it is also a really nasty lpf filter
while [ "X$1" != "X" ] ; do
	v=`expr "$1" : '-\(.\).*'`;
	s=`expr "$1" : '-.\(.*\)`;
	case "$v" in
		c ) c=1;;
		?* )
			if [ "X$s" = "X" ] ; then shift; s=$1; fi;
			eval "$v=\"$s\"";
			;;
	esac;
	shift;
done
set
if [ "$c" = "1" ]; then 
   cat 
else 
   sed -e 's/$//' -e '$s/$//'
fi
