prefix=/usr
exec_prefix=${prefix}

if test "${prefix}/include" != /usr/include ; then
  LIBGLADE_INCLUDEDIR="-I$ATHTOOLROOT${prefix}/include -I$ATHTOOLROOT/usr/include/gnome-xml"
else
  LIBGLADE_INCLUDEDIR="-I$ATHTOOLROOT/usr/include/gnome-xml"
fi
LIBGLADE_LIBDIR="-L$ATHTOOLROOT${exec_prefix}/lib -rdynamic -L/usr/lib -L/usr/X11R6/lib"
LIBGLADE_LIBS="-lglade-gnome -lglade -L/usr/lib -lxml -lz -rdynamic -lgnomeui -lart_lgpl -lgdk_imlib -lSM -lICE -lgtk -lgdk -lgmodule -lXext -lX11 -lgnome -lgnomesupport -lesd -laudiofile -lm -ldb -lglib -ldl"
MODULE_VERSION="libglade-0.15"

