#!/bin/csh -f
# $Id: root.xsession,v 1.2 2004-09-11 16:49:17 rbasch Exp $

# Source the users .cshrc if one exists otherwise 
# do some minimal stuff

if (-r ~/.cshrc) then
	source ~/.cshrc
else
	set path=(/usr/athena/bin /etc /etc/athena /bin/athena /usr/bin \
                  /usr/sfw/bin /usr/ucb /usr/sbin \
		  /usr/local . /usr/athena/etc /sbin /usr/openwin/bin)
	set prompt="`uname -n`# "
endif

# Start up an xterm window
exec xterm  -ls -geometry 80x56+0+30
