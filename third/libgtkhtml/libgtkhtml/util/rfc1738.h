#ifndef __RFC1738_H__
#define __RFC1738_H__

#include <glib.h>

gchar *rfc1738_encode_string (const gchar *str);
gchar *rfc1738_make_full_url (const gchar *base, const gchar *rel);

#endif /* __RFC1738__ */
