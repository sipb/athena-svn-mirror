# $Id: root.cshrc,v 1.7 1998-04-30 20:04:36 ghudson Exp $

set path=( /srvd/patch /usr/athena/bin /etc/athena /usr/sbin /sbin \
	   /bin/athena /usr/bin /usr/ccs/bin /usr/athena/etc /usr/ucb \
	   /usr/openwin/bin /etc)
if ($?prompt) then
	set prompt="`uname -n`# "
endif

set add_flags="-a -h -n"
alias add 'eval "`/bin/athena/attach -Padd $add_flags \!:*`"'

# source user's .cshrc if WHO variable is set
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 022
