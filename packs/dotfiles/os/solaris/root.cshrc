# $Id: root.cshrc,v 1.5 2004-09-11 16:50:23 rbasch Exp $

set path=( /srvd/patch /usr/athena/bin /usr/athena/etc /etc/athena /usr/sbin \
	   /sbin /bin/athena /usr/bin /usr/ccs/bin /usr/sfw/bin /usr/ucb \
	   /usr/openwin/bin /etc)
setenv MANPATH /usr/athena/man:/usr/openwin/man:/usr/dt/man:/usr/man:/usr/sfw/man
if ($?prompt) then
	set prompt="`uname -n`# "
	set nostat = (/afs/)
	set autolist
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
