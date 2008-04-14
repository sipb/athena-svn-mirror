#!/bin/sh

# Wrapper script to auto-create an MIT Chat account the first time
# pidgin is run.

# If .purple directory does not exist, create it and make it private.
if [ ! -d "$HOME/.purple" ]; then
  mkdir "$HOME/.purple" || exit 1
  fs sa "$HOME/.purple" system:anyuser none > /dev/null 2>&1
fi

# Set up an MIT Chat account if no account settings exist.
if [ ! -f "$HOME/.purple/accounts.xml" ]; then
  cat > $HOME/.purple/accounts.xml << EOF
<?xml version='1.0' encoding='UTF-8' ?>

<accounts version='1.0'>
 <account>
  <protocol>prpl-jabber</protocol>
  <name>$USER@mit.edu/Pidgin</name>
  <settings>
   <setting name='athena' type='bool'>1</setting>
   <setting name='require_tls' type='bool'>1</setting>
  </settings>
  <settings ui='gtk-gaim'>
   <setting name='auto-login' type='bool'>1</setting>
  </settings>
 </account>
</accounts>
EOF
fi

# Set the buddy list to visible if no prefs file exists.  (Ordinarily
# handled by the startup wizard, which we are bypassing.)
if [ ! -f "$HOME/.purple/prefs.xml" ]; then
  cat > $HOME/.purple/prefs.xml << "EOF"
<?xml version='1.0' encoding='UTF-8' ?>

<pref version='1.0' name='/'>
	<pref name='pidgin'>
		<pref name='blist'>
			<pref name='list_visible' type='bool' value='1' />
		</pref>
	</pref>
</pref>
EOF
fi

exec /usr/bin/pidgin.debathena-orig "$@"
