#!/bin/sh
#
# apmcontinue -- called from Red Hat's cardmgr script to signal an
#		 APM change.
#
# $1: apm event type
#

/etc/athena/network-scripts/net-event "$1"
