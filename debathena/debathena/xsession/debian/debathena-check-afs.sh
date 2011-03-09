youlose() {
    zenity --warning --text="Your AFS home directory appears to be unavailable, and this login session cannot continue.\n\nThis could be due to an AFS outage, a network problem, or an issue with this specific workstation.  We recommend you try another workstation, or a different cluster.\n\nIf the problem appears to only be on this workstation, please notify hotline@mit.edu and mention this machine's hostname ($(hostname -f)).  If the problem persists on multiple workstations, please contact the IS&amp;T Helpdesk (helpdesk@mit.edu or x3-1101)."
    exit 0
}
if [ "$DEBATHENA_HOME_TYPE" = "afs" ]; then
    [ -d "$HOME" ] || youlose
elif [ "$DEBATHENA_HOME_TYPE" = "local" ] && echo "$(id -Gn | sed -e 's/ /\n/g')" | grep -q ^nss-nonlocal-users$; then
    # DEBATHENA_HOME_TYPE=local if AFS is not running, awesome
    # This code can get removed when Trac #838 is fixed
    REALHOME=$HOME
    [ -h $HOME ] && REALHOME=$(readlink $HOME)
    if echo $REALHOME | grep -q ^/afs; then
	fs whichcell $REALHOME || youlose
    fi
fi
	
