#!/bin/sh
#
# $Id: install.sh,v 1.1.1.1 1996-10-07 20:16:48 ghudson Exp $
#
# This script implements a subset of the normal BSD install(1) program.
# It's certainly not complete.
#

Copy=1					# Copy src to target
Strip=0					# Strip installed target
Group=staff				# Default group
Mode=0755				# Default file mode
dst=""
src=""
Prog=$0

#
# Find the locations of various commands we'll need
#
if [ -x /usr/bin/cp ]; then
	cp=/usr/bin/cp
elif [ -x /bin/cp ]; then
	cp=/bin/cp
else
	echo "${Prog}: Cannot locate the cp program."
	exit 1
fi

if [ -x /usr/bin/mv ]; then
	mv=/usr/bin/mv
elif [ -x /bin/mv ]; then
	mv=/bin/mv
else
	echo "${Prog}: Cannot locate the mv program."
	exit 1
fi

if [ -x /usr/bin/rm ]; then
	rm=/usr/bin/rm
elif [ -x /bin/rm ]; then
	rm=/bin/rm
else
	echo "${Prog}: Cannot locate the rm program."
	exit 1
fi

if [ -x /usr/bin/chmod ]; then
	chmod=/usr/bin/chmod
elif [ -x /bin/chmod ]; then
	chmod=/bin/chmod
else
	echo "${Prog}: Cannot locate the chmod program."
	exit 1
fi

if [ -x /usr/bin/strip ]; then
	strip=/usr/bin/strip
elif [ -x /bin/strip ]; then
	strip=/bin/strip
elif [ -x /usr/ccs/bin/strip ]; then
	strip=/usr/ccs/bin/strip
else
	echo "${Prog}: Cannot locate the strip program."
	exit 1
fi

if [ -x /usr/bin/chgrp ]; then
	chgrp=/usr/bin/chgrp
elif [ -x /bin/chgrp ]; then
	chgrp=/bin/chgrp
elif [ -x /etc/chgrp ]; then
	chgrp=/etc/chgrp
else
	echo "${Prog}: Cannot locate the chgrp program."
	exit 1
fi

if [ -x /usr/etc/chown ]; then
	chown=/usr/etc/chown
elif [ -x /usr/sbin/chown ]; then
	chown=/usr/sbin/chown
elif [ -x /usr/ucb/chown ]; then
	chown=/usr/ucb/chown
elif [ -x /bin/chown ]; then
	chown=/bin/chown
elif [ -x /usr/bin/chown ]; then
	chown=/usr/bin/chown
elif [ -x /etc/chown ]; then
	chown=/etc/chown
else
	echo "${Prog}: Cannot locate the chown program."
	exit 1
fi

#
# Parse command line options
#
while [ x$1 != x ]; do
    case $1 in 
	-c)	# Option for backwards compatibility
		shift
		continue;;

	-m)
		Mode=$2
		shift
		shift
		continue;;

	-g)
		Group=$2
		shift
		shift
		continue;;

	-o)
		Owner=$2
		shift
		shift
		continue;;

	-s)
		Strip=1
		shift
		continue;;

	-*)
		echo "${Prog}: Unknown option $2."
		exit 1
		;;

	*)
		if [ x$src = x ]; then
			src=$1
		else
			dst=$1
		fi
		shift
		continue;;

    esac
done

if [ x$src = x ]; then
	echo "${Prog}: No source file was specified."
	exit 1
fi
if [ x$dst = x ]; then
	echo "${Prog}: No destination file was specified."
	exit 1
fi

if [ -d $dst ]; then
	base=`basename $src`
	Target=$dst/$base
else
	Target=$dst
fi

if [ -f $Target ]; then
	$rm -f $Target
fi

if [ $Copy -eq 1 ]; then
	InstallProg=$cp
else
	InstallProg=$mv
fi

$InstallProg $src $Target && { 
	if [ x$Owner != x ]; then
		$chown $Owner $Target
	fi
} && {
	$chgrp $Group $Target
} && {
	$chmod $Mode $Target
} && {
	if [ $Strip -gt 0 ]; then
		$strip $Target
	fi
}

