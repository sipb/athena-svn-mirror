# This is sourced by sh, not executed
#
# Ensure that networking is up

require_network() {
    # Ensure that eth0 is up
    ifup eth0 2>/dev/null
    gw=$(route -n | grep ^0.0.0.0\ | awk '{print $2}')
    if [ -z "$gw" ]; then
	echo "Something is wrong with the networking config (no default route)"
	return 1
    fi
    # Re-verify connection to gateway... one ping only.
    ping -w 5 -c 1 $gw >/dev/null 2>&1
    if [ $? != 0 ]; then
	echo "Could not ping gateway after trying for 5 seconds"
	return 1
    fi
    if ! grep nameserver /etc/resolv.conf | grep -qv 127.0.0.1$; then
	echo "Trying to start bind..."
	service bind9 start
    fi
    host web.mit.edu >/dev/null 2>&1
    if [ $? != 0 ]; then
	echo "Couldn't look up web.mit.edu - DNS may be broken"
	return 1
    fi
}
