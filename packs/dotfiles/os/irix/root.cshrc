# $Id: root.cshrc,v 1.4 1999-08-03 04:55:10 jweiss Exp $

set path=(/srvd/patch /usr/athena/bin /etc/athena /bin/athena /usr/sbin \
	  /usr/bsd /sbin /usr/bin /bin /etc /usr/athena/etc /usr/etc \
	  /usr/bin/X11)
setenv MANPATH /usr/athena/man:/usr/freeware/catman:/usr/share/catman:/usr/share/man:/usr/catman:/usr/man
if ( (! $?ENVONLY) && $?prompt ) then
	set prompt="`uname -n`# "
	set nostat = (/afs/)
endif

# Default man pager is ul -b | more -s -f.  Remove the ul -b part
# to work around a bug where pager input is read in line mode.
setenv MANPAGER "more -s -f"

set add_flags="-a -h -n"
alias add 'eval `/bin/athena/attach -Padd $add_flags \!:*`'

# source user's .cshrc if WHO variable is set
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 022
