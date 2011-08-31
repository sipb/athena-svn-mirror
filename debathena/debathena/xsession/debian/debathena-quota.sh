if [ "$DEBATHENA_HOME_TYPE" = "afs" ]; then
    QUOTAPCT=$(fs lq $HOME | tail -1 | awk '{print $4}' | sed 's/[^0-9]*//g')
    if [ $QUOTAPCT -ge 100 ]; then
	message "You have exceeded (or are within 0.5% of) your AFS quota.  This login session will likely fail.\n\nIf this login session does fail, try using the 'Athena TTY Session' login session to get a tty-mode login session, and then remove or compress some files to get below your quota.  To do this, you can select 'Athena TTY Session' from the drop-down menu at the bottom of the login screen, after you enter your username, but before you enter your password.\n\nYou may also request a quota increase from User Accounts by calling x3-1325."
    fi
fi
