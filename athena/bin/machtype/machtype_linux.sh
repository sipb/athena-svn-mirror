#!/bin/sh
# $Id: machtype_linux.sh,v 1.4 1999-06-30 19:37:37 jweiss Exp $

# We need to support the following options:
# NOTE: c, v, d, L, and M are needed by olc, and it cares what order
#  the output is in.
#  -c     : Processor type
#  -d     : display type 
#  -k     : select the kernel
#  -m     : override memory from /dev/kmem
#  -r     : disk drive type
#  -v     : more verbose -- about memory mainly
#  -A     : print Athena Release
#  -C     : print out compatible Athena System names
#  -E     : print out the version of the Base OS
#  -L     : version of athena from /etc/athena/version
#  -M     : physical memory
#  -N     : print out the name of the base OS
#  -P     : print out Athena System packs (from /srvd/.rvdinfo)
#  -S     : Print out the Athena System name

PATH=/bin:/usr/bin:/usr/sbin

while getopts cdk:m:rvACELMNPS i; do
	case "$i" in
	c)
		cpu=1
		;;
	d)
		display=1
		;;
	k)
		# Doesn't do anything right now.
		kernel=1
		karg=$optarg
		;;
	m)
		# Also doesn't do anything right now.
		mem=1
		memarg=$optarg
		;;
	r)
		rdsk=1
		;;
	v)
		verbose=1
		;;
	A)
		at_rel=1
		;;
	C)
		ath_sys_compat=1
		;;
	E) 	
		base_os_ver=1
		;;
	L)
		ath_vers=1
		;;
	M)
		memory=1
		;;
	N)
		base_os_name=1
		;;
	P)	
		syspacks=1
		;;
	S)
		ath_sys_name=1
		;;
	\?)
		echo "Usage: machtype [-cdrvACELMNPS]" 1>&2
		exit 1
		;;
	esac
done
printed=0

if [ $at_rel ]; then
	if [ $verbose ]; then
		echo -n "Machtype version: "
	fi
	echo "@ATHMAJV@.@ATHMINV@"
	printed=1
fi

if [ $syspacks ]; then
	if [ $verbose ]; then
		cat /srvd/.rvdinfo
	else
		awk '{ v = $5; } END { print v; }' /srvd/.rvdinfo
	fi
	printed=1
fi

if [ $ath_vers ]; then
	if [ $verbose ]; then
		tail -1 /etc/athena/version
	else
		awk '{ v = $5; } END { print v; }' /etc/athena/version
	fi
	printed=1
fi

if [ $base_os_name ]; then
	if [ $verbose ]; then
		uname -sr
	else
		uname -s
	fi
	printed=1
fi

if [ $base_os_ver ]; then
	uname -r
	printed=1
fi

if [ $ath_sys_name ]; then
	echo "@ATHSYS@"
	printed=1
fi

if [ $ath_sys_compat ]; then
	echo "@ATHSYSCOMPAT@"
	printed=1
fi

if [ $cpu ] ; then
	if [ $verbose ]; then
	        echo "`uname -s` `uname -r` on `uname -m`"
	else
		uname -m
	fi
	printed=1
fi

if [ $display ] ; then
	/usr/X11R6/bin/X -showconfig 2>&1 | awk '
		/^XFree86 Version/	{ printf "%s %s ", $1, $3; }
		/^Configured drivers:$/	{ getline drv;
					  split(drv, a, "[ :]*");
					  print a[2]; }'
	printed=1
fi

if [ $rdsk ]; then
	awk '/^SCSI device/ { print; }
	     /^hd[a-z]:/ { print; }
	     /^Floppy/ { for (i=3; i <= NF; i += 3) print $i ": " $(i+2); }' \
	     /var/log/dmesg
	printed=1
fi

if [ $memory ] ; then
	if [ $verbose ]; then
		awk 'BEGIN { FS="[^0-9]+" }
		     /^Memory:/ { printf "user=%d, phys=%d (%d M)\n",
				         $2*1.024, $3*1.024, $3/1000; }' \
		    /var/log/dmesg
	else
		awk 'BEGIN { FS="[^0-9]+" }
		     /^Memory:/ { printf "%d\n", $3*1.024; }' /var/log/dmesg
	fi
	printed=1
fi

if [ $printed -eq '0' ] ; then
	echo linux
fi
exit 0
