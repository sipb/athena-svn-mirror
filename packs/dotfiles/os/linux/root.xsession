#!/bin/csh -f
# $Id: root.xsession,v 1.1 1999-06-11 15:27:12 tb Exp $

# Source the users .cshrc if one exists otherwise 
# do some minimal stuff

if (-r ~/.cshrc) then
	source ~/.cshrc
else
	set path=(/usr/athena/bin /etc /etc/athena /bin/athena /usr/bin /bin \
		/usr/sbin /usr/X11R6/bin \
		. /usr/athena/etc /sbin)
	set prompt="`uname -n`# "
endif

# Start up an xterm window
exec xterm  -ls -geometry 80x56+0+30
