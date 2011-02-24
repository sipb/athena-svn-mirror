if [ "$GDMSESSION" = "000debathena-nocalls" ]; then
    export NOCALLS=1
    message "You have chosen to log in without your customizations." \
   	"Your dotfiles will be ignored in favor of the system" \
        "default settings.  You can use this session to fix any" \
	"problems with your dotfiles."
fi

# You don't actually want nocalls as your default session
[ -e "$HOME/.dmrc" ] && \
  grep -q 000debathena-nocalls "$HOME/.dmrc" && \
  rm "$HOME/.dmrc"
[ -e "/var/cache/gdm/$USER/dmrc" ] && \
  grep -q 000debathena-nocalls "/var/cache/gdm/$USER/dmrc" && \
  rm "/var/cache/gdm/$USER/dmrc"
