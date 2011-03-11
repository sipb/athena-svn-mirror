#!/bin/sh

if [ "--daemons" = "$1" ]; then
    [ -e /etc/init.d/cups ] && rcname=cups || rcname=cupsys
    echo $rcname
    exit
fi

case "$2" in
    \(*\))
        exit 101
        ;;
esac

# If nobody's logged in, follow the default policy
if ! [ -e /var/run/athena-login ]; then
    exit 0
elif [ -e /ClusterLogin ]; then
    case "$1" in
        cups|cupsys)
            exit 0
            ;;
        *)
            exit 101
            ;;
    esac
else
    case "$1" in
        cups|cupsys)
            exit 101
            ;;
        *)
            exit 0
            ;;
    esac
fi
