#!/bin/sh
#
# A config file for OpenConnect.  Based on vpnc-script from vpnc.
#

set -x

if [ -f /etc/default/mit-vpn ]; then
    . /etc/default/mit-vpn
fi

if [ ! -d "/var/run/mit-vpn" ]; then
    mkdir -p /var/run/mit-vpn
fi

if [ ! -e /dev/net/tun ]; then
    echo "ERROR: /dev/net/tun does not exist."
fi

DEFAULT_ROUTE_FILE=/var/run/mit-vpn/defaultroute
RESOLV_CONF_BACKUP=/var/run/mit-vpn/resolv.conf-backup


# =========== tunnel interface handling ====================================

do_ifconfig() {
    if [ -n "$INTERNAL_IP4_MTU" ]; then
	MTU=$INTERNAL_IP4_MTU
    fi

    if [ -z "$MTU" ]; then
	MTU=1406
	echo "W: No MTU value in default settings or from VPN; using $MTU"
    fi

    ifconfig "$TUNDEV" inet "$INTERNAL_IP4_ADDRESS" pointopoint "$INTERNAL_IP4_ADDRESS" netmask 255.255.255.255 mtu ${MTU} up
    
}

# =========== route handling ====================================

fix_ip_get_output () {
    sed 's/cache//;s/metric \?[0-9]\+ [0-9]\+//g;s/hoplimit [0-9]\+//g'
}

set_vpngateway_route() {
    ip route add `ip route get "$VPNGATEWAY" | fix_ip_get_output`
    ip route flush cache
}
	
del_vpngateway_route() {
    ip route del "$VPNGATEWAY"
    ip route flush cache
}
	
set_default_route() {
    ip route | grep '^default' | fix_ip_get_output > "$DEFAULT_ROUTE_FILE"
    ip route replace default dev "$TUNDEV"
    ip route flush cache
}
	
reset_default_route() {
    if [ -s "$DEFAULT_ROUTE_FILE" ]; then
	ip route replace `cat "$DEFAULT_ROUTE_FILE"`
	ip route flush cache
	rm -f -- "$DEFAULT_ROUTE_FILE"
    else
	echo "WARNING: Could not reset default route"
    fi
}
	
# === resolv.conf handling via /sbin/resolvconf =========

modify_resolvconf_manager() {
    NEW_RESOLVCONF=""
    for i in $INTERNAL_IP4_DNS; do
	NEW_RESOLVCONF="$NEW_RESOLVCONF
nameserver $i"
    done
    if [ -n "$CISCO_DEF_DOMAIN" ]; then
	NEW_RESOLVCONF="$NEW_RESOLVCONF
domain $CISCO_DEF_DOMAIN"
    fi
    echo "$NEW_RESOLVCONF" | /sbin/resolvconf -a $TUNDEV
}

restore_resolvconf_manager() {
    /sbin/resolvconf -d $TUNDEV
}

do_connect() {
    if [ -n "$MIT_VPN_PRECONNCET" -a -x "$MIT_VPN_PRECONNECT" ]; then
	$MIT_VPN_PRECONNECT
    fi
    
    do_ifconfig
    set_vpngateway_route
    set_default_route
    
    if [ -n "$INTERNAL_IP4_DNS" ]; then
	modify_resolvconf_manager
    fi
    
    if [ -n "$MIT_VPN_POSTCONNECT" -a -x "$MIT_VPN_POSTCONNECT" ]; then
	$MIT_VPN_POSTCONNECT
    fi

    echo <<EOF

----------------------------------------------------------------
           You are now connected to the MIT VPN.  
           Press Ctrl-C to disconnect
----------------------------------------------------------------
EOF

}

do_disconnect() {
    if [ -n "$MIT_VPN_PREDISCONNCET" -a -x "$MIT_VPN_PREDISCONNECT" ]; then
	$MIT_VPN_PREDISCONNECT
    fi
    
    reset_default_route
    del_vpngateway_route
    
    if [ -n "$INTERNAL_IP4_DNS" ]; then
	restore_resolvconf_manager
    fi

    if [ -n "$MIT_VPN_POSTDISCONNCET" -a -x "$MIT_VPN_POSTDISCONNECT" ]; then
	$MIT_VPN_POSTDISCONNECT
    fi
    
    echo "Disconnected from the MIT VPN."

}

#### Main

if [ -z "$reason" ]; then
	echo "This script was called incorrectly." 1>&2
	exit 1
fi

case "$reason" in
    connect)
	do_connect
	;;
    disconnect)
	do_disconnect
	;;
    *)
	echo "unknown reason '$reason'. " 1>&2
	exit 1
	;;
esac

exit 0
