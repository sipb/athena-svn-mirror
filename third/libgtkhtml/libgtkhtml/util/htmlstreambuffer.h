#ifndef __HTMLSTREAMBUFFER_H__
#define __HTMLSTREAMBUFFER_H__

#include <glib.h>
#include <libgtkhtml/util/htmlstream.h>

G_BEGIN_DECLS

typedef void (* HtmlStreamBufferCloseFunc) (const gchar *str, gint len, gpointer user_data);

HtmlStream *html_stream_buffer_new (HtmlStreamBufferCloseFunc close_func, gpointer user_data);

G_END_DECLS

#endif /* __HTMLSTREAMBUFFER_H__ */
