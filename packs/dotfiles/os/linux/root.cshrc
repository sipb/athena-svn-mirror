# $Id: root.cshrc,v 1.2 2000-03-21 17:40:28 tb Exp $

set path=( /srvd/patch /usr/athena/bin /etc/athena /usr/sbin /sbin \
	   /bin/athena /bin /usr/bin /usr/athena/etc /etc /usr/X11R6/bin )
setenv MANPATH /usr/athena/man:/usr/man
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
