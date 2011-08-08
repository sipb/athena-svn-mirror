#!/bin/bash
#
# This is a horrible horrible hack and should be reverted once we have a
# better solution to Trac #1020

UPD_START=$(stat -c "%Y" /var/run/athena-nologin 2>/dev/null)
[ -z "$UPD_START" ] && exit 0
NOW=$(date +"%s")
ELAPSED=$(expr $NOW - $UPD_START)
# Updates do not take an hour on modern machines.
if [ $ELAPSED -gt 3480 ]; then
   pkill -f athena-auto-update
   rm -f /var/run/athena-nologin
   if [ "$(machtype -L)" != "debathena-cluster" ]; then
       echo -e "To: root\nFrom: root\nSubject: athena-auto-update on $(hostname)\n\nathena-auto-update had become unresponsive.  It has been killed and will retry within 6 hours.  Repeated failures may merit further investigation." | sendmail root
   fi
fi
exit 0
