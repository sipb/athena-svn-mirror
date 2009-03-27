: ${FORCE_QUICKSTATION=0}

grep -iq $(hostname --fqdn) /afs/athena/system/config/quick/quickstations
if [ $? -eq 0 -o $FORCE_QUICKSTATION -eq 1 ] && [ -x /usr/bin/bugme ]; then
    STARTUP="/usr/bin/bugme $STARTUP"
fi
