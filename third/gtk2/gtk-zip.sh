#!/bin/sh

# Build zipfiles for GTK on Win32

ZIP=/tmp/gtk+-2.2.0-`date +%Y%m%d`.zip
DEVZIP=/tmp/gtk+-dev-2.2.0-`date +%Y%m%d`.zip
cd /usr

rm $ZIP
zip -r $ZIP -@ <<EOF
COPYING.LIB-2
etc/gtk-2.0
lib/libgdk_pixbuf-2.2-0.dll
lib/libgdk-win32-2.2-0.dll
lib/libgtk-win32-2.2-0.dll
EOF

zip $ZIP lib/gtk-2.0/2.2.0/loaders/*.dll lib/gtk-2.0/2.2.0/immodules/*.dll

zip $ZIP share/themes/*/gtk-2.0/gtkrc share/themes/*/gtk-2.0-key/gtkrc

zip -r $ZIP lib/locale/*/LC_MESSAGES/gtk20.mo

rm $DEVZIP
zip -r $DEVZIP -@ <<EOF
include/gtk-2.0
lib/libgdk_pixbuf-2.2.dll.a
lib/gdk_pixbuf-2.2.lib
lib/libgdk-win32-2.2.dll.a
lib/gdk-win32-2.2.lib
lib/libgtk-win32-2.2.dll.a
lib/gtk-win32-2.2.lib
lib/gtk-2.0/include
lib/pkgconfig/gdk-pixbuf-2.0.pc
lib/pkgconfig/gdk-2.0.pc
lib/pkgconfig/gdk-win32-2.0.pc
lib/pkgconfig/gtk+-2.0.pc
lib/pkgconfig/gtk+-win32-2.0.pc
share/aclocal/gtk-2.0.m4
EOF

