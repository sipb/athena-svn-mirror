# copyright (C) 2001 Sun Microsystems, Inc.

scrollkeeper_data_dir=$(datadir)/scrollkeeper/Templates/de_AT
scrollkeeper_data_link_dir=../de
install-data-local:
	rm -f $(DESTDIR)$(scrollkeeper_data_dir)
	$(LN_S) -f $(scrollkeeper_data_link_dir) $(DESTDIR)$(scrollkeeper_data_dir)
