# root's csh settings
#
# "$Revision: 1.1 $"

# List directories in columns and show hidden files
alias ls 'ls -CA'

# Remember last 100 commands
set history = 100

# For interactive shells, set the prompt to show the host name and event number.
# The sed command removes the domain from the host name.  `hostname -s`
# accomplishes the same but is not available when /usr is not mounted.
if ( (! $?ENVONLY) && $?prompt ) then
	if ( -o /bin/su ) then
		set prompt=`sed -e '/^ *$/d' -e 's/\..*//' /etc/sys_id`" \!# "
	else
		set prompt=`sed -e '/^ *$/d' -e 's/\..*//' /etc/sys_id`" \!% "
	endif
endif

#
# Athena tweaks
#
set cdpath=(/)
set path=(/srvd/patch /usr/athena/bin /etc/athena /bin/athena /usr/sbin /usr/bsd /sbin /usr/bin /bin /etc /usr/athena/etc /usr/etc /usr/bin/X11)

#
# source user's .cshrc if WHO variable is set
#
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 2
