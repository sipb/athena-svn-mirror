# Prototype user .bashrc file
#
# $Id: dot.bashrc,v 1.1 1999-06-11 15:28:10 tb Exp $

# This file sources a system-wide bashrc file, which:
#      - sets up standard environment variables
#      - sources user file ~/.bash_environment, if it exists
#      - sets standard path
#      - sets up standard shell variables, aliases, etc.
#      - source user file ~/.bashrc.mine, if it exists

initdir=/usr/athena/lib/init

if [ -r $initdir/bashrc ]; then
        . $initdir/bashrc
else
	if [ -n "$PS1" ]; then
	  echo "Warning: System-wide initialization files not found."
	  echo "Shell initialization has not been performed."
	  stty sane
	fi
	# set some basic defaults if failed initialization
	umask 077
fi


# If you want to ADJUST the bash initialization sequence, create any of 
# the following files (as appropriate) in your home directory, with commands
# to achieve the effect listed:
#
#      .bash_environment - setup session environment (set environmental
#			   variables, attach lockers, etc.)
#      .bashrc.mine  - setup bash environment (set shell variables, aliases,
#                     unset system defaults, etc.)
                      
# If you want to CHANGE the bash initialization sequence, revise this .bashrc
# file (the one you're reading now).  You may want to copy the contents of
# the system-wide bashrc file as a starting point.
# 
# WARNING: If you revise this .bashrc file, you will not automatically
# get any changes that Athena may make to the system-wide file at a
# later date.  Be sure you know what you are doing.
