#!/bin/sh

if [ "${ATHENA_ENVIRONMENT_WINDOW_MANAGER+set}" = set ]; then
  zenity --error --text="This tool cannot be used if you set the WINDOW_MANAGER environment variable in your ~/.environment file.  Change your window manager by editing your ~/.environment file."
  exit
fi

if [ -r ~/.athena-sawfish ]; then

  # User already has Sawfish preference; ask about reverting it.
  if zenity --question --text="Your account is currently configured to use the Sawfish window manager.  Do you wish to revert this preference and use the default window manager (Metacity) for future logins?"; then
    rm ~/.athena-sawfish
    zenity --info --text="Your account is now configured to use the default windowmanager (Metacity).  This change will take effect at your next login session."
  fi

else

  # Ask about installing Sawfish preference.
  if zenity --question --text="Would you like to configure your account to use the Sawfish window manager for future logins?"; then
    touch ~/.athena-sawfish
    zenity --info --text="Your account is now configured to use the Sawfish windowmanager.  This change will take effect at your next login session."
  fi

fi
