#!/bin/sh
#
# ifup-local -- called from Red Hat's ifup-post script 
#
# $1: ethernet device
#

exec /etc/athena/network-scripts/net-event net-up "$1"
