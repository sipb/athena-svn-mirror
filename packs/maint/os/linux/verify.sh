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
eval `AUTOUPDATE=false getcluster -b -l /etc/athena/cluster.local \
	"$HOST" $athenaversion`

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
  failures="$failures `rpm -V $rpm | awk '{print $NF}'`"
  n=$[$n+1]
  printf "\rVerifying: %3d%% of packages" $[$n*100/$total]
done
echo

# For each failing file, see if it's an exception.  If it's not, then
# find out its package, and make sure the package file is on the
# failing packages list.

unset pkglist
for failure in $failures; do
  case $failure in

    # These are all config files that we handle below in STEP 3.
    /etc/passwd | \
    /etc/shadow | \
    /etc/group | \
    /etc/services  | \
    /etc/syslog.conf | \
    /etc/inittab | \
    /etc/X11/prefdm | \
    /etc/X11/fs/config | \
    /etc/info-dir | \
    /usr/X11R6/lib/X11/app-defaults/XTerm | \
    /etc/xinetd.conf | \
    /etc/xinetd.d/* | \
    /etc/athena/rc.conf | \
    /etc/conf.linuxconf | \
    /etc/man.config )
        ;;

    # These are managed by other parts of the system which work to make
    # sure they are correct.
    /etc/motd | \
    /usr/vice/etc/CellServDB | \
    /usr/lib/umb-scheme/slibcat | \
    /etc/sysconfig/openafs | \
    /usr/vice/etc/cacheinfo | \
    /usr/vice/cache | \
    /boot/kernel.h | \
    /var/spool/at/.SEQ | \
    /etc/DIR_COLORS | \
    /etc/X11/XF86Config-4 | \
    /var/lib/rpm/__db.* )
        ;;

    # These are all files that we simply tolerate changes in without
    # comment.  They probably shouldn't be generating conflicts, but
    # it's easier to just ignore theme here than to fix their rpms.
    /etc/X11/fs | \
    /dev/* | \
    /var/log/* | \
    /var/state/* | \
    /usr/vice/cache | \
    /usr/X11R6/lib/X11/fonts/*/fonts.dir | \
    /usr/X11R6/lib/X11/fonts/*/encodings.dir | \
    /usr/share/fonts/default/Type1/fonts.dir | \
    /usr/share/fonts/default/Type1/fonts.scale | \
    /etc/bashrc | \
    /etc/profile | \
    /etc/csh.cshrc | \
    /etc/csh.login | \
    /etc/filesystems | \
    /etc/X11/XftConfig | \
    /etc/skel/.bash* | \
    /etc/pwdb.conf | \
    /usr/share/ssl/openssl.cnf | \
    /usr/lib/libglide3.so.3 | \
    /var/lock | \
    /usr/sbin/lockdev | \
    /etc/rc.d/rc.local | \
    /etc/sysconfig/apmd | \
    /etc/sysconfig/arpwatch | \
    /etc/sysconfig/init | \
    /etc/sysconfig/kudzu | \
    /etc/sysconfig/syslog | \
    /etc/sysctl.conf | \
    /etc/pcmcia/*.opts | \
    /usr/lib/mc/bin/cons.saver | \
    /etc/pam.d/system-auth | \
    /etc/issue | \
    /etc/issue.net | \
    /etc/X11/xinit/xinitrc.d/xinput | \
    /etc/logrotate.conf )
	;;

    *)
	for pkg in `rpm -q -f $failure`; do
	    pkg=`grep $pkg $rpmlist | awk '{print $1}'`
	    pkglist="$pkglist $pkg"
	done
	;;
  esac
done

# Uniquify the package list and install it

pkglist=`echo $pkglist | tr ' ' '\012' | sort | uniq`
echo "$0: Force installing $pkglist"
rpm -i --force $pkglist || errorout "$0: rpm package installation failed"


# STEP 3: Copy generic public master config files

config=$SYSPREFIX/config/$athenaversion

for i in man.config services syslog.conf inittab info-dir xinetd.conf; do
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

if [ -r $config/etc/athena/rc.conf ]; then
  sed -e "s#^HOST=[^;]*#HOST=$HOST#" \
      -e "s#^ADDR=[^;]*#ADDR=$ADDR#" \
      -e "s#^NETDEV=[^;]*#NETDEV=$NETDEV#" \
      -e "s#^MACHINE=[^;]*#MACHINE=$MACHINE#" \
      -e "s#^SYSTEM=[^;]*#SYSTEM=$SYSTEM#" \
    $config/etc/athena/rc.conf > /etc/athena/rc.conf
fi
