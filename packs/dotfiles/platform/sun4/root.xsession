#!/bin/csh -f
# $Id: root.xsession,v 1.2 1997-12-28 17:51:18 ghudson Exp $

# Source the users .cshrc if one exists otherwise 
# do some minimal stuff

if (-r ~/.cshrc) then
	source ~/.cshrc
else
	set path=(/usr/athena/bin /etc /etc/athena /bin/athena /usr/bin \
                  /usr/ucb /usr/sbin \
		  /usr/local . /usr/athena/etc /sbin /usr/openwin/bin)
	set prompt="`uname -n`# "
endif

# Start up an xterm window
exec xterm  -ls -geometry 80x56+0+30
