#!/bin/sh
# $Id: local-lockers.sh,v 1.2 2002-11-12 22:55:51 ghudson Exp $

# local-lockers - Bring lockers local using athlsync

conf=/etc/athena/local-lockers.conf
localdir=/var/athena/local
validdir=/var/athena/local-validated
margin=524288		# Keep 512MB free

umask 022
PATH=/bin/athena:/usr/athena/etc:$PATH

if [ ! -r $conf ]; then
  exit
fi

mkdir -p $localdir $validdir
cflist=
for l in `awk '/^[^#]/ { print; }' $conf`; do
  case $l in
  */*)
    # "lockername/symname" means to read the symlink "symname" in the locker
    # "lockername" to find the locker to copy.  This "indirect locker"
    # case is for IS-maintained third-party software lockers.
    il=`dirname $l`
    sym=`basename $l`
    path=`hesinfo "$il" filsys | sort -nk 5 | awk '/^AFS/ { print $2; exit; }'`
    if [ ! -n "$path" ] || [ ! -d "$path" ] || [ ! -h "$path/$sym" ]; then
      continue
    fi
    l=`ls -l "$path/$sym" | sed -e 's/.* -> //'`
    ;;
  esac

  # Keep a list of configured locker names so we don't nuke them later.
  cflist="$cflist $l "

  path=`hesinfo "$l" filsys | sort -nk 5 | awk '/^AFS/ { print $2; exit; }'`
  localpath=$localdir/$l
  validpath=$validdir/$l
  if [ -n "$path" ] && [ -d "$path" ]; then
    space=`athlsync -s "$path" "$localpath"`
    freespace=`df -k $localdir | awk '{print $(NF-2);}'`
    if [ -n "$space" ] && [ "$freespace" -ge `expr "$space" + $margin` ]; then
      # Sync and validate the local copy.
      if athlsync "$path" "$localpath"; then
	rm -f "$validpath"
	ln -s "$localpath" "$validpath"
      fi
    else
      # Not enough room.  De-validate the local copy if there is one,
      # unless there were no changes to sync.
      [ "$space" = 0 ] || rm -f "$validpath"
    fi
  fi
done

for d in `ls $localdir` ; do
  case $cflist in
  *" $d "*)
    # This is one of the lockers we wanted local, so keep it.
    continue
    ;;
  esac
  if [ ! -h "/var/athena/attachtab/locker/$d" ]; then
    # Not attached and no longer wanted local; nuke it.
    rm -rf "$validdir/$d" "$localdir/$d"
  fi
done
