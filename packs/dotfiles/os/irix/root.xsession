#!/bin/csh -f
# $Id: root.xsession,v 1.1 1998-12-25 19:33:18 ghudson Exp $

# Source the user's .cshrc if one exists otherwise 
# do some minimal stuff

if (-r ~/.cshrc) then
	source ~/.cshrc
else
	set path=(/srvd/patch /usr/athena/bin /etc/athena /bin/athena \
		  /usr/sbin /usr/bsd /sbin /usr/bin /bin /etc \
		  /usr/athena/etc /usr/etc /usr/bin/X11)
endif

# Start up an xterm window
exec xterm  -ls -geometry 80x56+0+30
