# $Id: root.login,v 1.4 1998-08-14 04:00:30 ghudson Exp $

if (! $?ENVONLY) then
	# Set the interrupt character to Ctrl-c and do clean backspacing.
	if (-t 0) then
	    stty intr '^C' echoe 
	endif

	# Set the TERM environment variable
	if ( -d /usr/lib/terminfo) then
	    # use text port intelligently
	    if ($?L0) then
		if ("$L0" == NOGRAPHICS) set term="iris-tp"
	    endif
	    eval `tset -s -Q`
	endif

	# save tty state in a file where wsh can find it
	if ((! -f $HOME/.wshttymode) && (-t 0)) then
	    stty -g > $HOME/.wshttymode
	endif
endif

# Athena tweak
umask 022
