#!/bin/sh
#
# ifdown-pre-local -- called from Red Hat's ifdown script 
#
# $1: ethernet device
#

exec /etc/athena/network-scripts/net-event net-down "$1"
