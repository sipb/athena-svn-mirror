#!/bin/sh
exec /usr/lib/saf/ttymon -g -h -p "`uname -n` console login: " -T sun \
-d /dev/console -l console -m ldterm,ttcompat
echo "ttymon exec failed"
exit 1
