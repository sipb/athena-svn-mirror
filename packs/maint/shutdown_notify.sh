#!/bin/sh
# Script to notify people that a server is going down.
# This is run by shutdown
#
#       $Source: /afs/dev.mit.edu/source/repository/packs/maint/shutdown_notify.sh,v $
#       $Author: probe $
#       $Header: /afs/dev.mit.edu/source/repository/packs/maint/shutdown_notify.sh,v 1.2 1990-05-01 18:09:21 probe Exp $

exec /etc/athena/zshutdown_notify
