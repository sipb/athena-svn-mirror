#!/bin/sh

# Build zipfiles for Pango on Win32: separate runtime and developer packages

ZIP=/tmp/pango-1.2.1-`date +%Y%m%d`.zip
DEVZIP=/tmp/pango-dev-1.2.1-`date +%Y%m%d`.zip
cd /usr

rm $ZIP
zip $ZIP -@ <<EOF
bin/pango-querymodules.exe
etc/pango/pango.aliases
etc/pango/pango.modules
lib/libpango-1.0-0.dll
lib/libpangoft2-1.0-0.dll
lib/libpangowin32-1.0-0.dll
EOF

zip $ZIP lib/pango/1.2.0/modules/*.dll

rm $DEVZIP
zip -r $DEVZIP -@ <<EOF
include/pango-1.0
lib/libpango-1.0.dll.a
lib/pango-1.0.lib
lib/libpangoft2-1.0.dll.a
lib/pangoft2-1.0.lib
lib/libpangowin32-1.0.dll.a
lib/pangowin32-1.0.lib
lib/pkgconfig/pango.pc
lib/pkgconfig/pangowin32.pc
lib/pkgconfig/pangoft2.pc
EOF
