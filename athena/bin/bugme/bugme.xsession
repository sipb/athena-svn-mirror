: ${FORCE_QUICKSTATION=0}

# Use IGNORECASE to make the match case insensitive
# Use ret to set a default exit code, and then override it if there's
# a match
awk -v IGNORECASE=1 -v host="$(hostname --fqdn)" -v ret=1 '$0 == host { ret=0 } END { exit ret }' /afs/athena.mit.edu/system/config/quick/quickstations
if [ $? -eq 0 -o $FORCE_QUICKSTATION -eq 1 ] && [ -x /usr/bin/bugme ]; then
    STARTUP="/usr/bin/bugme $STARTUP"
fi
