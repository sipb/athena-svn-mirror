youlose() {
    zenity --warning --text="Your AFS home directory appears to be unavailable, and this login session cannot continue.\n\nThis could be due to an AFS outage, a network problem, or an issue with this specific workstation.  We recommend you try another workstation, or a different cluster.\n\nIf the problem appears to only be on this workstation, please notify hotline@mit.edu and mention this machine's hostname ($(hostname -f)).  If the problem persists on multiple workstations, please contact the IS&amp;T Helpdesk (helpdesk@mit.edu or x3-1101)."
    exit 0
}
youmightlose() {
    if ! zenity --question --text="Your home directory type could not be determined.  This might indicate problems with the AFS servers, or you might have an error in your AFS permissions.\n\nIf you know what you're doing, it's possible that this error is benign and you can continue.  However, it's more than likely that this login session will fail and you'll need to reboot the workstation to log out.  With that in mind, would you like to continue?"; then
	exit 0
    fi
}
case "$DEBATHENA_HOME_TYPE" in
    missing)
	youlose
	;;
    unknown)
	youmightlose
	;;
    afs)
	[ -d "$HOME" ] || youlose
	;;
esac
	
