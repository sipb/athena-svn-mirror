#!/bin/sh
ROOT=/usr/kerberos

# Try to find the binaries based upon where we were invoked from.
rundir=`echo $0 | sed 's:/configure$::'`
if [ -r $rundir/check-install ]; then
  installdir=$rundir
else
  if [ -r check-install ]; then 
    installdir=`pwd`
  else
    installdir=
  fi
fi

# Make a guess at the installation directory
guess=$ROOT
if [ x$installdir != x ]; then
  installdir=`echo $installdir | sed 's:/install$::'`
  if [ -d $ROOT ] && [ x`cd $ROOT; pwd` = x`cd $installdir; pwd` ]; then
    guess=$ROOT
  else
    guess=$installdir
  fi
fi
guess=`echo $guess | sed 's:/usr/kerberos$::'`
if [ x$guess = x ]; then
  guess=/
fi

echo "If you've unpacked the tape in $guess, just press RETURN;"
echo "If you've unpacked it below some other directory, enter it now."
read PACKDIR

if [ x$PACKDIR = x ]; then
  PACKDIR=$guess
fi

# If the user entered a directory ending in /usr/kerberos, they may not
# have really meant it.
subpackdir=`echo $PACKDIR | sed 's:/usr/kerberos$::'`
if [ x$PACKDIR != x$subpackdir ]; then
  if [ -d $PACKDIR/usr/kerberos ]; then
    :
  else
    PACKDIR=$subpackdir
    if [ x$PACKDIR = x ]; then
      PACKDIR=/
    fi
  fi
fi

if [ x$PACKDIR != x/ ]; then
  echo Copying files from $PACKDIR to $ROOT
  if [ -d $ROOT ]; then
    :
  else
    mkdir $ROOT
    if [ $? -eq 0 ]; then
      :
    else
      echo 1>&2 '***' Can not create $ROOT
      exit 1
    fi
    chmod 755 $ROOT
  fi
  (cd $PACKDIR/usr/kerberos; find . -type f -print) |
    while read f; do
      dir=`echo $f | sed -e 's|/[^/]*$||'`
      par=`echo $dir | sed -e 's|/[^/]*$||'`
      if [ -d $ROOT/$par ]; then
	:
      else
	mkdir $ROOT/$par
	if [ $? -eq 0 ]; then
	  :
	else
	  echo 1>&2 '***' Can not create $ROOT/$par
	  exit 1
	fi
	chmod 755 $ROOT/$par
      fi
      if [ -d $ROOT/$dir ]; then
	:
      else
	mkdir $ROOT/$dir
	if [ $? -eq 0 ]; then
	  :
	else
	  echo 1>&2 '***' Can not create $ROOT/$dir
	  exit 1
	fi
	chmod 755 $ROOT/$dir
      fi
      rm -f $ROOT/$f.n >/dev/null 2>&1
      cp $PACKDIR/usr/kerberos/$f $ROOT/$f.n
      if [ $? -eq 0 ]; then
	:
      else
	echo 1>&2 '***' Can not copy $PACKDIR/usr/kerberos/$f to $ROOT/$f.n
	exit 1
      fi
      chmod a-w $ROOT/$f.n
      mv -f $ROOT/$f $ROOT/$f.o >/dev/null 2>&1
      mv -f $ROOT/$f.n $ROOT/$f
      rm -f $ROOT/$f.o >/dev/null 2>&1
    done
  if [ -d $ROOT/database ]; then
    :
  else
    mkdir $ROOT/database
    chmod 700 $ROOT/database
  fi
fi

$ROOT/install/fixprot /

$ROOT/install/fixsvc

$ROOT/install/check-install / || exit 1

KRBCONF=$ROOT/lib/krb.conf
if [ -r $KRBCONF ]; then
  echo "Existing configuration for realm "`sed 1q $KRBCONF`" preserved."
  echo "To reconfigure it, delete $KRBCONF and re-run configure."
else
  $ROOT/install/ask-config $ROOT/lib || exit 1
  chmod 644 $ROOT/lib/krb.conf
  chmod 644 $ROOT/lib/krb.realms
fi

$ROOT/install/check-inetd

exit 0
