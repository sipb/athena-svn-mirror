#ifndef __HTMLBOXLISTITEM_H__
#define __HTMLBOXLISTITEM_H__

#include "htmlboxblock.h"

G_BEGIN_DECLS

#define HTML_TYPE_BOX_LIST_ITEM (html_box_list_item_get_type ())
#define HTML_BOX_LIST_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), HTML_TYPE_BOX_LIST_ITEM, HtmlBoxListItem))
#define HTML_BOX_LIST_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_BOX_LIST_ITEM, HtmlBoxListItemClass))
#define HTML_IS_BOX_LIST_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), HTML_TYPE_BOX_LIST_ITEM))
#define HTML_IS_BOX_LIST_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_BOX_LIST_ITEM))

typedef struct _HtmlBoxListItem HtmlBoxListItem;
typedef struct _HtmlBoxListItemClass HtmlBoxListItemClass;

struct _HtmlBoxListItem {
	HtmlBoxBlock parent_object;
	gint counter;
	HtmlBox *label;
	gchar *str;
};

struct _HtmlBoxListItemClass {
	HtmlBoxBlockClass parent_class;
};

static char * toRoman (long decimal);

GType    html_box_list_item_get_type (void);

HtmlBox *html_box_list_item_new      (void);

G_END_DECLS

#endif /* __HTMLBOXLIST_ITEM_H__ */


