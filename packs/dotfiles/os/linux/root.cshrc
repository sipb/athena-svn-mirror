# $Id: root.cshrc,v 1.6 2004-09-14 22:28:54 rbasch Exp $

set path=( /srvd/patch /usr/athena/bin /usr/athena/etc /etc/athena /usr/sbin \
	   /sbin /bin/athena /bin /usr/bin /etc /usr/X11R6/bin )
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
