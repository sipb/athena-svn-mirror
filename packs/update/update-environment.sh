export CONFCHG CONFVARS AUXDEVS OLDBINS DEADFILES LOCALPACKAGES
export LINKPACKAGES CONFDIR LIBDIR SERVERDIR PATH HOSTTYPE CPUTYPE

CONFCHG=$UPDATE_ROOT/var/athena/update.confchg
CONFVARS=$UPDATE_ROOT/var/athena/update.confvars
AUXDEVS=$UPDATE_ROOT/var/athena/update.auxdevs
OLDBINS=$UPDATE_ROOT/var/athena/update.oldbins
DEADFILES=$UPDATE_ROOT/var/athena/update.deadfiles
LOCALPACKAGES=$UPDATE_ROOT/var/athena/update.localpackages
LINKPACKAGES=$UPDATE_ROOT/var/athena/update.linkpackages
CONFDIR=$UPDATE_ROOT/etc/athena
LIBDIR=/srvd/usr/athena/lib/update
SERVERDIR=$UPDATE_ROOT/var/server
PATH=/bin:/etc:/usr/sbin:/sbin:/usr/bin:/usr/ucb:/usr/bsd:/os/bin:/os/etc:/srvd/etc/athena:/srvd/bin/athena:/os/usr/bin:/srvd/usr/athena/etc:/os/usr/ucb:/os/usr/bsd:$LIBDIR
HOSTTYPE=`$UPDATE_ROOT/bin/athena/machtype`
CPUTYPE=`$UPDATE_ROOT/bin/athena/machtype -c`
