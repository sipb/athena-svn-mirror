if [ "$GDMSESSION" = "000debathena-nocalls" ]; then
    export NOCALLS=1
    message "You have chosen to log in without your customizations." \
   	"Your dotfiles will be ignored in favor of the system" \
        "default settings.  You can use this session to fix any" \
	"problems with your dotfiles."
fi

# You don't actually want nocalls as your default session
DMRCFILES="$HOME/.dmrc \
           /var/cache/gdm/$USER/dmrc \
           /var/cache/lightdm/dmrc/$USER.dmrc"

for f in $DMRCFILES; do
    [ -e "$f" ] && \
	grep -q 000debathena-nocalls "$f" && \
	rm "$f"
done
