#! /bin/sh
#
# $Header: /afs/dev.mit.edu/source/repository/third/sysinfo/metasysinfo.sh,v 1.1.1.1 1996-10-07 20:16:48 ghudson Exp $
#
# This script is borrowed from TOP, by William LeBebvre (phil@eecs.nwu.edu).
#
# Sysinfo is very sensitive to differences in the kernel, so much so that an
# executable created on one sub-architecture may not work on others.  It
# is also quite common for a minor OS revision to require recompilation of
# sysinfo.  Both of these problems are especially prevalent for SunOS 4.*.  For
# example, a sysinfo executable made under SunOS 4.1.1 will not run correctly
# under SunOS 4.1.2, and vice versa.  "metasysinfo" attempts to solve this
# problem by choosing one of several possible sysinfo executables to run then
# executing it.
#
# To use metasysinfo your operating system needs to have the command "uname"
# as part of the standard OS release.  MAKE SURE IT DOES before proceeding.
# It will try to execute the command "sysinfo-`uname -m`-`uname -r`"  For 
# example, on a sparcstation 1 running SunOS 4.1.1, it will try to run
# "sysinfo-sun4c-4.1.1".
#
# INSTALLATION is easy:
#	- Just compile sysinfo as normal by typing "make".
# 	- Then run "make installmeta" (on the same machine!) to install the
#	  runtime files.  Run "make install.man" to install the man pages.
#
# REMEMBER:
# You will need to "make clean" and "make metainstall" on every different
# combination of sub-architecture and OS version that you have.
#

#
# Make sure the installation directory is in our path.
#
InstallDir=#InstallDir#
PATH=$InstallDir:$PATH:/bin:/usr/bin
export PATH

exec $0-`uname -m`-`uname -r` "$@"
