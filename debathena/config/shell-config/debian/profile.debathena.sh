# This is the Debathena /etc/profile.d configuration, installed by
# debathena-bash-config.

DEBATHENA_PROFILE_DIR=/usr/share/debathena-bash-config/profile.d
DEBATHENA_BASHRC_DIR=/usr/share/debathena-bash-config/bashrc.d

# /etc/profile has been sourced at this point (in fact, we're called
# as the last step in it) 
# On Ubuntu, this will run /etc/bash.bashrc.  Our bash.bashrc sets a
# variable, so we'll know if that happened.

# If it didn't, source all bash scripts in our bashrc.d directory.
if [ -n "$PS1" ] && [ -n "$BASH" ]; then
    if [ "${debathena_bashrc_sourced:-x}" != 1 ]; then
        for i in `run-parts --list "$DEBATHENA_BASHRC_DIR"`; do . "$i"; done
    fi
fi
unset debathena_bashrc_sourced

# Finally, source all bash scripts in our profile.d directory.
for i in `run-parts --list "$DEBATHENA_PROFILE_DIR"`; do . "$i"; done
