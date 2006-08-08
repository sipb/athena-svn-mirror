#!/bin/sh

errorout() {
    echo "$@" >&2
    exit 1
}

###
### STEP 0: Find out version and cluster package list information
###

. /etc/athena/rc.conf

if [ "$PUBLIC" != true ]; then
    errorout "$0 is only for a public workstations."
fi

athenaversion=`awk '{ a = $5; } END { print a; }' /etc/athena/version`

case "$athenaversion" in
[0-9]*)
    # Peachy
    ;;
Layered)
    errorout "$0 is only for use on standard non-layered Athena."
    ;;
*)
    errorout "$0: Cannot determine Athena version number."
    ;;
esac

# Fetch cluster variables for the version we are currently running.

unset NEW_TESTING_RELEASE NEW_PRODUCTION_RELEASE SYSPREFIX SYSCONTROL
eval `AUTOUPDATE=false getcluster -b "$athenaversion"`
if [ "${SYSPREFIX+set}" != set ]; then
  errorout "$0: Cannot find Linux clusterinfo for this machine."
fi
config=$SYSPREFIX/config/$athenaversion
if [ ! -d "$config" ]; then
  errorout "$0: Cannot find config area for $athenaversion."
fi

# Find the files list from the control file.

cd "$SYSPREFIX" || errorout "$0: Can't find system area $SYSPREFIX."

exec 3< "$SYSCONTROL" || errorout "$0: Can't read `pwd`/$SYSCONTROL."
unset rpmlist
while read version filename extra_garbage <&3; do
    if [ "x$version" = "x$athenaversion" ]; then
	rpmlist=$filename
	break
    fi
done
exec 3<&-


###
### STEP 1: Make sure the right set of packages is installed
###

/etc/athena/rpmupdate -p /dev/null $rpmlist || errorout "$0: rpmupdate failed"

###
### STEP 2: Run RPM verification; re-install any broken packages
###

# Find a list of files that don't match.  A potential problem here is
# that we assume that the relevant rpm databases have not gotten
# corrupted somehow.

total=`rpm -qa | wc -l`
n=0
for rpm in `rpm -qa`; do
  failures="$failures `rpm -V --nomd5 $rpm | awk '{print $NF}'`"
  n=$[$n+1]
  printf "\rVerifying: %3d%% of packages" $[$n*100/$total]
done
echo

# Invoke version-dependent exceptions scriptlet to filter $failures
# into $realfailures.
. $config/exceptions

# For each failed file which is not an exception, add its package
# to the failing package list.
unset pkglist
for failure in $realfailures; do
  ls -ld $failure
  ls -ld $failure 2>&1 | logger -t verify -p auth.notice
  for pkg in `rpm -q -f $failure`; do
    pkg=`grep $pkg $rpmlist | awk '{print $1}'`
    pkglist="$pkglist $pkg"
  done
done

# Uniquify the package list and install it

pkglist=`echo $pkglist | tr ' ' '\012' | sort | uniq`
if [ "$pkglist" ] ; then
    echo "$0: Force installing $pkglist"
    rpm -i --force $pkglist || errorout "$0: rpm package installation failed"
else
    echo "$0: No rpm packages to install."
fi

# STEP 3: Copy generic public master config files

for i in man.config services syslog.conf inittab info-dir \
        xinetd.conf krb5.conf; do
    cp $config/etc/$i /etc/$i
done

rsync -q -r --delete $config/etc/xinetd.d /etc

for i in /etc/passwd /etc/shadow /etc/group; do
    if [ -r $config$i ]; then
	syncupdate -c $i.local.new $config$i $i.local
	if [ -r $i.local ]; then
	    syncupdate -c $i.new $i.local $i
	fi
    fi
done
chown 0:0 /etc/passwd /etc/passwd.local /etc/shadow /etc/shadow.local \
    /etc/group /etc/group.local
chmod 644 /etc/passwd /etc/passwd.local /etc/group /etc/group.local
chmod 600 /etc/shadow /etc/shadow.local

cp $config/etc/X11/prefdm /etc/X11/prefdm
cp $config/etc/X11/fs/config /etc/X11/fs/config
cp $config/usr/X11R6/lib/X11/app-defaults/XTerm \
    /usr/X11R6/lib/X11/app-defaults/XTerm
cp $config/etc/athena/athinfo.access /etc/athena
cp $config/etc/athena/local-lockers.conf /etc/athena

if [ -r $config/etc/athena/rc.conf ]; then
  sed -e "s#^HOST=[^;]*#HOST=$HOST#" \
      -e "s#^ADDR=[^;]*#ADDR=$ADDR#" \
      -e "s#^NETDEV=[^;]*#NETDEV=$NETDEV#" \
      -e "s#^MACHINE=[^;]*#MACHINE=$MACHINE#" \
      -e "s#^SYSTEM=[^;]*#SYSTEM=$SYSTEM#" \
    $config/etc/athena/rc.conf > /etc/athena/rc.conf
fi
