/*
 * This file provides the hack to load the proper libgnome file
 * to get the proper macros for calling the internationalization
 * functions in gnome-print
 *
 */

#ifndef __GNOME_PRINT_I18N_H__
#define __GNOME_PRINT_I18N_H__

#include <bonobo/bonobo-i18n.h>

#ifdef ENABLE_NLS
#    undef _
#    define _(String)  libgnomeprint_gettext (String)
#endif

char *libgnomeprint_gettext (const char *msgid);

#endif /* __GNOME_PRINT_I18N_H__ */
