#ifndef __CSSPARSER_H__
#define __CSSPARSER_H__

#include <glib.h>

#include "cssstylesheet.h"

G_BEGIN_DECLS

CssStylesheet *css_parser_parse_stylesheet (const gchar *str, gint len);
CssRuleset *css_parser_parse_style_attr (const gchar *buffer, gint len);

G_END_DECLS

#endif /* __CSSPARSER_H__ */
