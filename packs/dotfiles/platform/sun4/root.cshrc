# $Id: root.cshrc,v 1.9 1998-09-24 17:16:05 danw Exp $

set path=( /srvd/patch /usr/athena/bin /etc/athena /usr/sbin /sbin \
	   /bin/athena /usr/bin /usr/ccs/bin /usr/athena/etc /usr/ucb \
	   /usr/openwin/bin /etc)
setenv MANPATH /usr/athena/man:/usr/openwin/man:/usr/dt/man:/usr/man
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
