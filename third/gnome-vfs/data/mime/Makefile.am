NULL=

keys_in_files = gnome-vfs.keys.in
keys_files = $(keys_in_files:.keys.in=.keys)

mimedir = $(datadir)/mime-info

mime_DATA = \
	$(keys_files) \
	gnome-vfs.mime \
	$(NULL)

applicationsdir = $(datadir)/application-registry

applications_DATA = \
	gnome-vfs.applications \
	$(NULL)

sysconf_DATA = gnome-vfs-mime-magic

@XML_I18N_MERGE_KEYS_RULE@

EXTRA_DIST = $(keys_in_files) $(mime_DATA) $(sysconf_DATA) $(applications_DATA)
