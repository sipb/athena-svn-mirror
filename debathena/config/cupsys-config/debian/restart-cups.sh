restart_cups()
{
	# Handle Debian package name change from cupsys to cups.
	[ -e /etc/init.d/cups ] && rcname=cups || rcname=cupsys
	# Restart cupsd if it is running
	if /etc/init.d/$rcname status; then
	    if hash invoke-rc.d; then
		invoke="invoke-rc.d $rcname"
	    else
		invoke="/etc/init.d/$rcname"
	    fi
	    # Flush remote.cache to deal with things like changing the
	    # server we BrowsePoll against. But don't do so if
	    # policy-rc.d is going to prevent us from restarting cupsd
	    # (for instance, if reactivate is installed and running;
	    # in that case, the next logout will flush remote.cache and restart
	    # cupsd anyway).
	    if $invoke stop; then
		rm -f /var/cache/cups/remote.cache
		$invoke start
	    fi
	fi
}
