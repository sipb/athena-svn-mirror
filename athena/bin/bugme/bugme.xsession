installed_status="$(dpkg-query -W -f='${Status}\n' debathena-bugme 2>&1 | grep '\(not-installed\|No packages found\)')"
if [ -z "$installed_status" ]; then
   if [ "$(machtype -q)" = "quickstation" ] && [ -x /usr/bin/bugme ]; then
       STARTUP="/usr/bin/bugme $STARTUP"
   fi
fi
