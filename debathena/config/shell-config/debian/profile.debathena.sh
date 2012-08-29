# This is the Debathena /etc/profile configuration, installed by
# debathena-bash-config.

DEBATHENA_PROFILE_DIR=/usr/share/debathena-bash-config/profile.d
DEBATHENA_BASHRC_DIR=/usr/share/debathena-bash-config/bashrc.d

# First source the normal profile. Sometimes this sources bash.bashrc
# (e.g., on Ubuntu); sometimes it doesn't. Let's find out!
debathena_test_if_bashrc_runs=0
. /etc/profile.debathena-orig

# If it didn't, source all bash scripts in our bashrc.d directory.
if [ -n "$PS1" ] && [ -n "$BASH" ]; then
    if [ "$debathena_test_if_bashrc_runs" = 0 ]; then
        for i in `run-parts --list "$DEBATHENA_BASHRC_DIR"`; do . "$i"; done
    fi
fi
unset debathena_test_if_bashrc_runs

# Finally, source all bash scripts in our profile.d directory.
for i in `run-parts --list "$DEBATHENA_PROFILE_DIR"`; do . "$i"; done
