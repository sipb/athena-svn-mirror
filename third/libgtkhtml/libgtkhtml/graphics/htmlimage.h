#ifndef __HTML_IMAGE_H__
#define __HTML_IMAGE_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include "libgtkhtml/util/htmlstream.h"

G_BEGIN_DECLS

typedef struct _HtmlImage HtmlImage;
typedef struct _HtmlImageClass HtmlImageClass;

#define HTML_IMAGE_TYPE (html_image_get_type ())
#define HTML_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), HTML_IMAGE_TYPE, HtmlImage))
#define HTML_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_IMAGE_TYPE, HtmlImageClass))
#define HTML_IMAGE_GET_CLASS(klass) (G_TYPE_INSTANCE_GET_CLASS ((klass), HTML_IMAGE_TYPE, HtmlImageClass))

struct _HtmlImage {
	GObject parent;

	GdkPixbuf *pixbuf;

	gchar *uri;
	gboolean broken;
	gboolean loading;

	HtmlStream *stream;

	GdkPixbufLoader *loader;
};

struct _HtmlImageClass {
	GObjectClass parent_class;

	void (* last_unref) (HtmlImage *image);
	
	void (* repaint_image) (HtmlImage *image, gint x, gint y, gint width, gint height);
	void (* resize_image) (HtmlImage *image);
};

GType html_image_get_type (void);

gint html_image_get_width (HtmlImage *image);
gint html_image_get_height (HtmlImage *image);

G_END_DECLS

#endif /* __HTML_IMAGE_H__ */
