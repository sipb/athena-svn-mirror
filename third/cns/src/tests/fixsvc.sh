#!/bin/sh

servfile=/etc/services
if [ -f $servfile ]; then
  label=no
  OIFS=$IFS
  IFS=X
  while read service; do
    name=`echo $service | sed -e 's|^\([^ 	]*\)[ 	].*$|\1|'`
    type=`echo $service | sed -e 's|^[^ 	]*[ 	][ 	]*[0-9]*/\([^ 	]*\)[ 	].*$|\1|'`
    if egrep "^$name[ 	][^/]*/$type[ 	]" $servfile >/dev/null 2>&1; then
      :
    else
      if [ x$label = xno ]; then
	echo "Updating $servfile (backup in $servfile.bak):"
	cp $servfile $servfile.bak
	label=yes
      fi
      echo " Adding $name $type"
      echo $service >> $servfile
    fi
  done << ---EOF---
klogin		543/tcp				# Kerberos authenticated rlogin
eklogin		2105/tcp			# Kerberos encrypted rlogin
kerberos	750/udp 	kdc		# Kerberos authentication--udp
kerberos	750/tcp 	kdc		# Kerberos authentication--tcp
kerberos_master	751/udp 			# Kerberos authentication
kerberos_master	751/tcp 			# Kerberos authentication
passwd_server	752/udp				# Kerberos passwd server
krb_prop	754/tcp				# Kerberos slave propagation
knetd		2053/tcp			# Kerberos de-multiplexor
kpop		1109/tcp			# Pop with Kerberos
kshell		544/tcp		cmd		# and remote shell
---EOF---
  IFS=$OIFS
else
  echo There is no $servfile file, so it will not be updated.
fi

