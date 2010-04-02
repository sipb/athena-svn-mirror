if [ "$(dpkg-query -W -f '${Status}' debathena-bugme 2>/dev/null)" = "install ok installed" ]; then
   if [ "$(machtype -q)" = "quickstation" ] && [ -x /usr/bin/bugme ]; then
       STARTUP="/usr/bin/bugme $STARTUP"
   fi
fi
