# $Id: root.cshrc,v 1.8 1998-05-19 17:15:59 ghudson Exp $

set path=( /srvd/patch /usr/athena/bin /etc/athena /usr/sbin /sbin \
	   /bin/athena /usr/bin /usr/ccs/bin /usr/athena/etc /usr/ucb \
	   /usr/openwin/bin /etc)
if ($?prompt) then
	set prompt="`uname -n`# "
endif

set add_flags="-a -h -n"
alias add 'eval `/bin/athena/attach -Padd $add_flags \!:*`'

# source user's .cshrc if WHO variable is set
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 022
