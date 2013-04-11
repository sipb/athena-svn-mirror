if [ "$GDMSESSION" = "000debathena-nocalls" ]; then
    export NOCALLS=1
    message "You have chosen to log in without your customizations." \
   	"Your dotfiles will be ignored in favor of the system" \
        "default settings.  You can use this session to fix any" \
	"problems with your dotfiles."
fi

# You don't actually want nocalls as your default session
# We don't remove it from the lightdm cache because we can't actually
# read the lightdm cache.  But with our lightdm-greeter, an incorrect
# value will never get cached anyway (because we always pass a session
# name, so the cache is not consulted)
DMRCFILES="$HOME/.dmrc \
           /var/cache/gdm/$USER/dmrc"

for f in $DMRCFILES; do
    [ -e "$f" ] && \
	grep -q 000debathena-nocalls "$f" && \
	rm "$f"
done
