# $Id: root.cshrc,v 1.3 2001-04-30 15:23:48 ghudson Exp $

set path=( /srvd/patch /usr/athena/bin /etc/athena /usr/sbin /sbin \
	   /bin/athena /bin /usr/bin /usr/athena/etc /etc /usr/X11R6/bin )
setenv MANPATH /usr/athena/man:/usr/share/man
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
