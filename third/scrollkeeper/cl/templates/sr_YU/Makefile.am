# copyright (C) 2001 Sun Microsystems, Inc.

scrollkeeper_data_dir=$(datadir)/scrollkeeper/Templates/sr_YU
scrollkeeper_data_link_dir=../sr

install-data-local:
	rm -f $(DESTDIR)$(scrollkeeper_data_dir)
	$(LN_S) -f $(scrollkeeper_data_link_dir) $(DESTDIR)$(scrollkeeper_data_dir)
