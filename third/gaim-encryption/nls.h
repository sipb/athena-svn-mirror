#ifndef GE_NLS_H
#define GE_NLS_H

#ifdef HAVE_CONFIG_H
# include <gaim-encryption-config.h>
#endif

#ifdef ENABLE_NLS

#ifdef _
# undef _
#endif

#ifdef N_
# undef N_
#endif

#  include <locale.h>
#  include <libintl.h>
#  define _(x) dgettext(ENC_PACKAGE, x)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  include <locale.h>
#  define N_(String) (String)
#  define _(x) (x)
#  define ngettext(Singular, Plural, Number) ((Number == 1) ? (Singular) : (Plural))
#endif

#endif
