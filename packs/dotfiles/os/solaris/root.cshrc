# $Id: root.cshrc,v 1.2 1999-04-03 01:26:58 jweiss Exp $

set path=( /srvd/patch /usr/athena/bin /etc/athena /usr/sbin /sbin \
	   /bin/athena /usr/bin /usr/ccs/bin /usr/athena/etc /usr/ucb \
	   /usr/openwin/bin /etc)
setenv MANPATH /usr/athena/man:/usr/openwin/man:/usr/dt/man:/usr/man
if ($?prompt) then
	set prompt="`uname -n`# "
	set nostat = (/afs/)
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
