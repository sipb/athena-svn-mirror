AUTOMAKE_OPTIONS = foreign

encryptdir = $(libdir)/gaim

encrypt_la_LDFLAGS = -module -avoid-version

encrypt_la_LIBADD = $(NSS_LIBS)

encrypt_LTLIBRARIES = encrypt.la
noinst_LIBRARIES =


EXTRA_DIST	= \
	Makefile.mingw \
	po/Makefile.mingw \
	COPYING      \
	CHANGELOG    \
	INSTALL      \
	NOTES        \
	README       \
	VERSION      \
	WISHLIST     \
	gaim-encryption-installer.nsi \
	nsis/header.bmp  \
	nsis/install.ico \
	$(wildcard nsis/translations/*.nsh)

encrypt_la_SOURCES = \
	nls.h                    \
	ge_blist.c               \
	ge_blist.h               \
	config_ui.c              \
	config_ui.h              \
	cryptproto.c             \
	cryptproto.h             \
	cryptutil.c              \
	cryptutil.h              \
	encrypt.c                \
	encrypt.h                \
	gpg.c                    \
	gpg.h                    \
	icon_capable.xpm         \
	icon_in_lock.xpm         \
	icon_in_unlock.xpm       \
	icon_lock.xpm            \
	icon_out_capable.xpm     \
	icon_out_lock.xpm        \
	icon_out_unlock.xpm      \
	icon_unlock.xpm          \
	keys.c                   \
	keys.h                   \
	keys_ui.c                \
	keys_ui.h                \
	nonce.c                  \
	nonce.h                  \
	nss_mgf1.c               \
	nss_mgf1.h               \
	nss_oaep.c               \
	nss_oaep.h               \
	nss_pss.c                \
	nss_pss.h                \
	prefs.c                  \
	prefs.h                  \
	rsa_nss.c                \
	rsa_nss.h                \
	state.c                  \
	state.h                  \
	state_ui.c               \
   state_ui.h               \
   ge_ui.c                  \
   ge_ui.h

AM_CFLAGS = \
	-Wall -fno-strict-aliasing

AM_CPPFLAGS = \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DLIBDIR=\"$(libdir)/gaim/\"      \
	-DDATADIR=\"$(datadir)\"          \
	$(GLIB_CFLAGS)                    \
	$(GAIM_CFLAGS)                    \
	$(NSS_CFLAGS)


BUILT_SOURCES = gaim-encryption-config.h

$(OBJECTS): $(BUILT_SOURCES)

gaim-encryption-config.h: pre-config.h
	sed 's/#define PACKAGE/#define GE_PACKAGE/g' pre-config.h > $@

testprog: $(encrypt_la_SOURCES) testmain.c
	$(CC) -g -Wall -lgdk -lgtk $(AM_CPPFLAGS) $(CPPFLAGS) $(NSS_INCLUDES) $(NSS_CFLAGS) -o testprog testmain.c  $(NSS_LIBS)

SUBDIRS = po intl

ACLOCAL_AMFLAGS = -I m4
