# $Id: root.cshrc,v 1.6 1997-12-28 17:49:17 ghudson Exp $

set path=( /srvd/patch /usr/athena/bin /etc/athena /usr/sbin /sbin \
	   /bin/athena /usr/bin /usr/ccs/bin /usr/athena/etc /usr/ucb \
	   /usr/openwin/bin /etc)
if ($?prompt) then
	set prompt="`uname -n`# "
endif

# source user's .cshrc if WHO variable is set
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 022
