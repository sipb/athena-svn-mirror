#ifndef GNOME_VFS_I18N_H
#define GNOME_VFS_I18N_H

/* The i18n defines */
#ifdef ENABLE_NLS
#    include <libintl.h>
#    undef _
#    define _(String) dgettext (GETTEXT_PACKAGE, String)
#    ifdef gettext_noop
#        define N_(String) gettext_noop (String)
#    else
#        define N_(String) (String)
#    endif
#else
/* Stubs that do something close enough.  */
#    define textdomain(String) (String)
#    define gettext(String) (String)
#    define dgettext(Domain,Message) (Message)
#    define dcgettext(Domain,Message,Type) (Message)
#    define bindtextdomain(Domain,Directory) (Domain)
#    define _(String) (String)
#    define N_(String) (String)
#endif

#endif
