# $Id: root.login,v 1.3 1998-06-08 14:13:21 ghudson Exp $

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

# Set the default X server.
if ($?DISPLAY == 0) then
    if ($?REMOTEHOST) then
	setenv DISPLAY ${REMOTEHOST}:0
    else
	setenv DISPLAY :0
    endif
endif

# Athena tweak
umask 022
