#ifndef __GP_TRUETYPE_UTILS_H__
#define __GP_TRUETYPE_UTILS_H__

#include <glib/gmacros.h>

G_END_DECLS

#include <glib.h>

GSList *gp_tt_split_file (const guchar *buf, guint len);

G_END_DECLS

#endif

