#ifndef __HTMLCOLOR_H__
#define __HTMLCOLOR_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _HtmlColor HtmlColor;

struct _HtmlColor {
	gint refcount;
	
	gushort red;
	gushort green;
	gushort blue;
	gushort transparent;
};

HtmlColor* html_color_ref (HtmlColor *color);
void html_color_unref (HtmlColor *color);

HtmlColor *html_color_new_from_name (const gchar *color_name);
HtmlColor *html_color_new_from_rgb (gushort red, gushort green, gushort blue);
void html_color_destroy (HtmlColor *color);
gboolean html_color_equal (HtmlColor *color1, HtmlColor *color2);
HtmlColor *html_color_transform (HtmlColor *color, gfloat ratio);
HtmlColor *html_color_dup (HtmlColor *color);
HtmlColor *html_color_transparent_new (void);

G_END_DECLS

#endif /* __HTMLCOLOR_H__ */
