export CONFCHG CONFVARS AUXDEVS OLDBINS OLDLIBS DEADFILES PACKAGES
export CONFDIR LIBDIR SERVERDIR PATH HOSTTYPE CPUTYPE OSCONFCHG
export PATCHES OLDPKGS OLDPTCHS OSPRESERVE
export MIT_CORE_PACKAGES MIT_NONCORE_PACKAGES MIT_OLD_PACKAGES

CONFCHG=$UPDATE_ROOT/var/athena/update.confchg
CONFVARS=$UPDATE_ROOT/var/athena/update.confvars
AUXDEVS=$UPDATE_ROOT/var/athena/update.auxdevs
OLDBINS=$UPDATE_ROOT/var/athena/update.oldbins
OLDLIBS=$UPDATE_ROOT/var/athena/update.oldlibs
OLDPKGS=$UPDATE_ROOT/var/athena/update.oldpkgs
OLDPTCHS=$UPDATE_ROOT/var/athena/update.oldptchs
DEADFILES=$UPDATE_ROOT/var/athena/update.deadfiles
PACKAGES=$UPDATE_ROOT/var/athena/update.packages
PATCHES=$UPDATE_ROOT/var/athena/update.patches
MIT_CORE_PACKAGES=$UPDATE_ROOT/var/athena/update.mitcorepkgs
MIT_NONCORE_PACKAGES=$UPDATE_ROOT/var/athena/update.mitnoncorepkgs
MIT_OLD_PACKAGES=$UPDATE_ROOT/var/athena/update.mitoldpkgs
CONFDIR=$UPDATE_ROOT/etc/athena
LIBDIR=/srvd/usr/athena/lib/update
SERVERDIR=$UPDATE_ROOT/var/server
PATH=/bin:/etc:/usr/sbin:/sbin:/usr/bin:/usr/ucb:/usr/bsd:/os/bin:/os/etc:/srvd/etc/athena:/srvd/bin/athena:/os/usr/bin:/srvd/usr/athena/etc:/os/usr/ucb:/os/usr/bsd:$LIBDIR
HOSTTYPE=`$UPDATE_ROOT/bin/athena/machtype`
CPUTYPE=`$UPDATE_ROOT/bin/athena/machtype -c`
OSCONFCHG=$UPDATE_ROOT/var/athena/update.osconfchg
OSPRESERVE=$UPDATE_ROOT/var/athena/update.preserve
