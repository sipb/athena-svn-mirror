#!/bin/sh
#

u=${USER-the_olc_builder}
h=`hostname`
t=`date`

umask 002
/bin/echo "#define VERSION_INFO \"(${t}) ${u}@${h}\"" >version.h
