#!/bin/sh

if grep klogin /etc/inetd.conf >/dev/null 2>&1; then
  :
else
  cat << ---EOF---
To enable Kerberos access to this host, add any of

klogin	stream	tcp	nowait	root	$ROOT/etc/klogind   klogind
eklogin	stream	tcp	nowait	root	$ROOT/etc/klogind   eklogind
kpop	stream	tcp	nowait	root	$ROOT/etc/popper    popper
kshell	stream	tcp	nowait	root	$ROOT/etc/kshd      kshd

   to /etc/inetd.conf, restart inetd, and get a srvtab installed.
---EOF---
fi
