#!/bin/sh
#

u=${USER-the_olc_builder}
h=`hostname`
t=`date`

umask 002
/bin/echo "#define SERVER_VERSION_STRING \"(${t}) ${u}@${h}\"" >version.h
