#undef PACKAGE
#undef VERSION
#undef HAVE_LIBSM
#undef ENABLE_NLS
#undef HAVE_CATGETS
#undef HAVE_GETTEXT
#undef HAVE_LC_MESSAGES
#undef HAVE_STPCPY
#undef HAVE_ORBIT
#undef HAVE_GTK_SELECTION_ADD_TARGET
#undef HAVE_GUILE
#undef ENABLE_BONOBO
#undef HAVE_IEEEFP_H
#undef ENABLE_GNOME
#undef HAVE_GLIBWWW
#undef HAVE_GHTTP
#undef HAVE_GNU_REGEX
#undef USING_OAF
#undef GTK_HTML_USE_XIM
#undef GNOME_GTKHTML_EDITOR_SHLIB
#undef EDITOR_API_VERSION
#undef GETTEXT_PACKAGE
#undef GTKHTML_RELEASE
#undef GTKHTML_RELEASE_STRING
#undef GNOME_EXPLICIT_TRANSLATION_DOMAIN

@BOTTOM@
/* This is from libglade */

#ifdef DEBUG
#  define debug(stmnt) stmnt
#else
#  define debug(stmnt) /* nothing */
#endif
