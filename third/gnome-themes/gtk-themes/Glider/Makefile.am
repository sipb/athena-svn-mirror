themedir = $(datadir)/themes/Glider/gtk-2.0

theme_DATA = gtkrc iconrc \
	stock_apply.png stock_bottom.png \
	stock_cancel.png stock_down.png \
	stock_first.png stock_last.png \
	stock_left.png stock_ok.png \
	stock_refresh.png stock_right.png \
	stock_top.png stock_up.png

#Keep this hack in until 2.8.0 is released
install: install-am
	        rm -rf $(DESTDIR)$(datadir)/themes/SimplySmooth/gtk-2.0

EXTRA_DIST = $(theme_DATA)
