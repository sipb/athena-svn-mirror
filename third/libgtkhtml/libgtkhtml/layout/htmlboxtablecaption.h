#ifndef __HTMLBOXTABLE_CAPTION_H__
#define __HTMLBOXTABLE_CAPTION_H__

#include <gdk/gdk.h>
#include "htmlboxblock.h"

G_BEGIN_DECLS

typedef struct _HtmlBoxTableCaption HtmlBoxTableCaption;
typedef struct _HtmlBoxTableCaptionClass HtmlBoxTableCaptionClass;

#define HTML_TYPE_BOX_TABLE_CAPTION (html_box_table_caption_get_type ())
#define HTML_BOX_TABLE_CAPTION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), HTML_TYPE_BOX_TABLE_CAPTION, HtmlBoxTableCaption))
#define HTML_BOX_TABLE_CAPTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_BOX_TABLE_CAPTION, HtmlBoxTableCaptionClasss))
#define HTML_IS_BOX_TABLE_CAPTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), HTML_TYPE_BOX_TABLE_CAPTION))
#define HTML_IS_BOX_TABLE_CAPTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_BOX_TABLE_CAPTION))

struct _HtmlBoxTableCaption {
	HtmlBoxBlock parent_object;
	gint width, height;
};

struct _HtmlBoxTableCaptionClass {
	HtmlBoxBlockClass parent_class;
};

GType html_box_table_caption_get_type (void);
HtmlBox *html_box_table_caption_new (void);

void html_box_table_caption_relayout_width (HtmlBoxTableCaption *caption, HtmlRelayout *relayout, gint width);

G_END_DECLS

#endif /* __HTMLBOXTABLE_CAPTION_H__ */





