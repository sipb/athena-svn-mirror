#!/bin/sh
# $Id: offlinehome.sh,v 1.2 2004-05-16 21:24:09 ghudson Exp $

# offlinehome - Set up an offline homedir.  This differs from a truly
# local account (which you would create with useradd and mark in
# /etc/athena/access with the 'L' flag) in that it is associated with
# an Athena account and begins life with standard Athena dotfiles, but
# still lives on local disk.

usage="offlinehome [-d homedir] [-u uid] [-g gid] [-c comment] [-s shell]"
usage="$usage username"

PATH=/usr/athena/bin:/bin/athena:$PATH

# Process options to set $username and $homedir.
unset homedir uid gid comment
while getopts d:u:g:c: opt; do
  case "$opt" in
  d)
    homedir=$OPTARG
    ;;
  u)
    uid=$OPTARG
    ;;
  g)
    gid=$OPTARG
    ;;
  c)
    comment=$OPTARG
    ;;
  s)
    shell=$OPTARG
    ;;
  \?)
    echo "$usage" >&2
    exit 1
    ;;
  esac
done
shift `expr $OPTIND - 1`
if [ $# -ne 1 ]; then
  echo "$usage" >&2
  exit 1
fi
username=$1
: ${homedir=/home/$username}

# Sanity checks.
if [ `id -u` != 0 ]; then
  echo "This script must be run as root." >&2
  exit 1
fi
if [ -e "$homedir" ]; then
  echo "$homedir already exists; remove it or move it aside first." >&2
  exit 1
fi
if [ -s "/var/athena/sessions/$username" ]; then
  echo "$username is currently logged in; cannot proceed." >&2
  exit 1
fi
if [ ! -e /etc/passwd -o ! -e /etc/shadow ]; then
  echo "/etc/passwd or /etc/shadow does not exist!  Cannot proceed." >&2
  exit 1
fi
if { [ -e /etc/passwd.local ] && grep -q "^$username" /etc/passwd.local; } || \
   { [ -e /etc/shadow.local ] && grep -q "^$username" /etc/shadow.local; } || \
   grep -q "^$username" /etc/passwd || grep -q "^$username" /etc/shadow; then
  echo "$username exists in one or more of /etc/passwd, /etc/passwd.local," >&2
  echo "/etc/shadow, and /etc/shadow.local; remove those entries first." >&2
  exit 1
fi

# Get passwd and shadow entries from hesiod data or command-line options.
if [ "${uid+set}${gid+set}${comment+set}${shell+set}" != setsetsetset ]; then
  hes=`hesinfo $username passwd 2>/dev/null`
  if [ -z "$hes" ]; then
    if [ "${uid+set}" != set ]; then
      echo "Cannot get hesiod passwd information for $username.  To use" >&2
      echo "offlinehome while off the network, you must specify at least" >&2
      echo "the uid (with the -u option) and ideally the gid, comment, and" >&2
      echo "shell fields (-g, -c, -s options).  If you are not familiar" >&2
      echo "with these terms, connect to the network before running" >&2
      echo "offlinehome." >&2
      exit 1
    fi
    echo "Cannot get hesiod passwd information for $username.  Using" >&2
    echo "default values for some passwd fields." >&2
    echo >&2
    : ${gid=101}
    : ${comment=}
    : ${shell=/bin/athena/tcsh}
  fi
  : ${uid=`printf "%s" "$hes" | cut -d: -f3`}
  : ${gid=`printf "%s" "$hes" | cut -d: -f4`}
  : ${comment=`printf "%s" "$hes" | cut -d: -f5`}
  : ${shell=`printf "%s" "$hes" | cut -d: -f7`}
fi

# /etc/passwd.local needs to exist to support a "log in with Athena
# home directory" option.  Make sure it does.  If we create
# /etc/passwd.local, also create /etc/shadow.local, but if
# /etc/passwd.local exists and /etc/shadow.local does not, leave that
# alone.
if [ ! -e /etc/passwd.local ]; then
  echo "Warning: /etc/passwd.local does not exist, and is necessary for" >&2
  echo "offline homedirs to work properly.  I will copy /etc/passwd to" >&2
  echo "/etc/passwd.local and proceed." >&2
  cp -p /etc/passwd /etc/passwd.local
  if [ ! -e /etc/shadow.local ]; then
   echo "Also copying /etc/shadow to /etc/shadow.local." >&2
   cp -p /etc/shadow /etc/shadow.local
  fi
  echo >&2
fi

# Create the homedir.
useradd -c "$comment" -d "$homedir" -g "$gid" -m -k /usr/prototype_user \
  -n -s "$shell" -u "$uid" "$username"

# Update passwd.local and shadow.local
grep "^$username:" /etc/passwd >> /etc/passwd.local
grep "^$username:" /etc/shadow >> /etc/shadow.local

echo "Created offline homedir for $username."
echo
echo "To be able to log in while off the network, you must set a local"
echo "password.  The password should be the same as the Athena password if"
echo "possible, but it does not have to be."
echo
if [ -t 0 ]; then
  echo "(If you want to do this later, just press ^C now and run"
  echo "'passwd -l $username' when you are ready.)"
  echo
  passwd -l "$username"
else
  echo "To set a local password, run 'passwd -l $username'."
fi
