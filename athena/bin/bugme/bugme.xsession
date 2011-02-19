if [ "$(machtype -q)" = "quickstation" ] && [ -x /usr/bin/bugme ]; then
   STARTUP="/usr/bin/bugme $STARTUP"
fi

