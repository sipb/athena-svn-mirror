#! /bin/sh
#
# $Revision: 1.1.1.2 $
#
# This is the frontend command to SysInfo.  SysInfo generally can only
# be run on the OS Version it was compiled on.  This wrapper script figures
# out what platform it's being run on and then runs the real sysinfo 
# binary.  All the real sysinfo binaries should exist in LIBDIR.
#

#
# LIBDIR is where all the real executables live
#
if [ ! -z "${SYSINFOLIBDIR}" ]; then
    LIBDIR="${SYSINFOLIBDIR}"
else
    # LIBDIR is filled in during install
    LIBDIR=#LIBDIR#
fi

#
# Make sure we have a consistant execution path
#
PATH=/bin:/usr/bin:/usr/ucb:/sbin
export PATH

Program=`basename $0`
RealProgram=sysinfo
Debug="$METADEBUG"

#
# Find uname program.
#
if [ -f /usr/bin/uname ]; then
	unameprog=/usr/bin/uname
elif [ -f /bin/uname ]; then
	unameprog=/bin/uname
fi

#
# Determine our OS name if we can.
#
if [ "${unameprog}" ]; then
	osname="`${unameprog} -s`"
fi

#
# Check for Convex SPP special handling
#
if [ "${osname}" = "HP-UX" ]; then
	if [ -x /bin/sysinfo ]; then
		altname="`/bin/sysinfo -sv | awk '{print $1}' | sed -e 's;_.*;;' 2>/dev/null`"
		if [ ! -z "${altname}" ]; then
			osname="${altname}"
		fi
	fi
fi

#
# Try stupid file checks
#
if [ -z "${osname}" ]; then
	if [ -d /NextApps ]; then
		if [ -f /usr/bin/hostinfo ]; then
			mver="`/usr/bin/hostinfo | grep -i 'next mach' | awk '{print $3}' | sed -e 's/\..*//'`"
			osname="nextstep${mver}"
		else
			osname="nextstep2"
		fi
	elif [ -d /usr/alliant ]; then
		osname="concentrix"
	else
		echo "Unable to determine your OS type.";
		exit 1;
	fi
fi

osname="`echo ${osname} | tr '[A-Z]' '[a-z]' | sed -e 's;-;;g'`"

#
# Get OS Version
#
case "${osname}" in
sunos)
	if [ -z "${unameprog}" ]; then
		echo "No uname program found."
		exit 1
	fi
	osver="`${unameprog} -r`"
	;;
aix)
	osver="`${unameprog} -v`"
	if [ "$osver" -eq "4" ]; then
	    aixosm="`${unameprog} -r`"
	    osver=${osver}.${aixosm}
	fi
	;;
hpux)
	osver="`${unameprog} -r| sed -e 's;^[A-Za-z]\.;;' -e 's;^[0];;'`"
	;;
sppux)
	osver="`/bin/sysinfo -sv | awk '{print $2}'`"
	;;
concentrix)
	# We don't care what the os version is
	osver=""
	;;
*)
    	# Default is uname -r
	if [ -z "${unameprog}" ]; then
		echo "No uname program found."
		exit 1
	fi
	osver="`${unameprog} -r`"
	;;
esac

if [ ! -z "${osver}" ]; then
	osmver=`echo $osver | sed -e 's;\..*;;g'`
	if [ -z "${osvernodot}" ]; then
	    osvernodot=`echo $osver | sed -e 's;\.;;g'`
	fi    
fi

#
# Get Application Architecture
#
case "${osname}" in
aix)
    # Hard code since we don't know how else to get this
    apparch="powerpc"
    ;;
hpux)
    # Hard code since we don't know how else to get this
    apparch="`${unameprog} -m | sed -e 's;/.*;;' -e 's;9000;pa-risc;'`"
    ;;
linux)
    apparch="`${unameprog} -m`"
    ;;
sunos)
    if [ "${osmver}" -eq "4" ]; then
	apparch="`/bin/mach`"
    else
	apparch="`${unameprog} -p`"
    fi
    ;;
*)
    apparch=unknown
    ;;
esac

#
# Get CPU Architecture
#
case "${osname}" in
sunos)
    if [ "${osmver}" = "5" ]; then
	if [ -x /bin/isainfo ]; then
	    cpuarch="`/bin/isainfo -k`"
	else
	    cpuarch="`${unameprog} -p`"
	fi
    else
	cpuarch="`${unameprog} -m`"
    fi
    ;;
*)
    # Doesn't matter on other platforms
    cpuarch=${apparch}
    ;;
esac

Platform=${cpuarch}-${osname}-${osver}
PlatDir=${LIBDIR}/${Platform}
Bin=${PlatDir}/${RealProgram}

if [ ! -f $Bin ]; then
    echo "${Program}: There is no executable for your platform (${Platform}) in ${PlatDir}" >&2
else
    if [ "$Debug" -gt 0 ]; then
	echo $Bin "$@"
    else
	exec $Bin "$@"
    fi
fi

