#!/bin/sh

mailer=
if [ -f $HOME/.athena-pine-noask ]; then
  mailer=pine
elif [ -f $HOME/.athena-sipb-pine-noask ]; then
  mailer=sipb-pine
elif [ ! -f $HOME/.pinerc ]; then
  # User has never run pine before; just run Athena pine.
  mailer=pine
  touch $HOME/.athena-pine-noask
else
  while :; do
    echo "The Athena release now contains the Pine program. By default, it"
    echo "behaves somewhat differently from the SIPB installation of pine you"
    echo "have probably used in the past; the Athena version stores your saved"
    echo "mail on your Post Office server where it can be accessed from Mac"
    echo "and PC clients as well as from Athena. If you prefer, you can"
    echo "continue to use the SIPB version of pine."
    echo
    echo "Please choose one of the following options:"
    echo "  1. Let me try the Athena pine, but ask me again next time"
    echo "  2. Let me use the SIPB pine for now, but ask me again next time"
    echo "  3. I want to use the Athena version of pine; don't ask me again"
    echo "  4. I want to keep using the SIPB pine; don't ask me again"
    echo "  5. Cancel"
    echo ""
    printf "Please choose an option (1-5): "
    read choice
    case $choice in
    1)
      mailer=pine
      ;;
    2)
      mailer=sipb-pine
      ;;
    3)
      mailer=pine
      touch $HOME/.athena-pine-noask
      ;;
    4)
      mailer=sipb-pine
      touch $HOME/.athena-sipb-pine-noask
      ;;
    5)
      exit
      ;;
    *)
      echo
      echo "Option $choice not recognized; please try again."
      echo
      ;;
    esac
    if [ -n "$mailer" ]; then
      break
    fi
  done
fi

case $mailer in
pine)
  exec pine.real "$@"
  ;;
sipb-pine)
  exec /bin/athena/attachandrun sipb pine pine "$@"
  ;;
esac
