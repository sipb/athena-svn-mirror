#!/bin/sh
# $Id: machtype_linux.sh,v 1.9 2003-08-12 21:47:50 jweiss Exp $

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
#  -q     : Print out "quickstation" or "not_quickstation" depending on if this
#           workstation is a quickstation or not

PATH=/bin:/usr/bin:/sbin:/usr/sbin
QUICKSTATION_FILE=/afs/athena.mit.edu/system/config/quick/quickstations

while getopts cdk:m:rvACELMNPSq i; do
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
	q)
		quickstation=1
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
	echo "@ATHENA_MAJOR_VERSION@.@ATHENA_MINOR_VERSION@"
	printed=1
fi

if [ $syspacks ]; then
	echo "Linux does not use system packs." >&2
	printed=1
fi

if [ $ath_vers ]; then
	for meta in cluster workstation login-graphical login standard clients locker athena-libraries extra-software extra-software-nox thirdparty; do
		if dpkg-query --showformat '${Status}\n' -W "debathena-$meta" 2>/dev/null | grep -q ' installed$'; then
			echo "debathena-$meta"
			printed=1
			if [ ! $verbose ]; then
				break
			fi
		fi
	done
        if [ $printed -eq '0' ]; then
            echo "debathena"
            printed=1
        fi
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
	lsb_release -sd
	printed=1
fi

if [ $ath_sys_name ]; then
	echo "@ATHENA_SYS@"
	printed=1
fi

if [ $ath_sys_compat ]; then
	echo "@ATHENA_SYS_COMPAT@"
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
	lspci | awk -F: '/VGA/ {print $3}' | sed -n -e 's/^ //' -e p
	printed=1
fi

if [ $rdsk ]; then
	for d in /sys/block/[fhs]d*; do
	    echo $(basename "$d"): \
		$(xargs -I @ expr @ '*' 8 / 15625 < "$d/size")MB \
		$(cat "$d/device/model" ||
		  cat "/proc/ide/$(basename "$d")/model")
	done 2>/dev/null
	printed=1
fi

if [ $memory ] ; then
	if [ $verbose ]; then
		awk '/^MemTotal:/ { printf "user=%d, phys=%d (%d M)\n",
					   $2, $2, $2/1024 }' \
		    /proc/meminfo
	else
		awk '/^MemTotal:/ { printf "%d\n", $2 }' /proc/meminfo
	fi
	printed=1
fi

if [ $quickstation ]; then
	hostname=$(hostname -f)
	if ( [ -n "$hostname" ] &&
		[ -e "$QUICKSTATION_FILE" ] &&
		mount | grep -q '^AFS on /afs' && 
		grep -Fxqi "$hostname" "$QUICKSTATION_FILE") ||
	    [ "$FORCE_QUICKSTATION" = 1 ]; then
		echo quickstation
	else
		echo not_quickstation
	fi
	printed=1
fi

if [ $printed -eq '0' ] ; then
	echo linux
fi
exit 0
