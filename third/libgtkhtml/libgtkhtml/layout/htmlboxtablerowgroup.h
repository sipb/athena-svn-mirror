#ifndef __HTMLBOXTABLE_ROW_GROUP_H__
#define __HTMLBOXTABLE_ROW_GROUP_H__

#include <gdk/gdk.h>
#include "htmlbox.h"

G_BEGIN_DECLS

typedef struct _HtmlBoxTableRowGroup HtmlBoxTableRowGroup;
typedef struct _HtmlBoxTableRowGroupClass HtmlBoxTableRowGroupClass;

#define HTML_TYPE_BOX_TABLE_ROW_GROUP (html_box_table_row_group_get_type ())
#define HTML_BOX_TABLE_ROW_GROUP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), HTML_TYPE_BOX_TABLE_ROW_GROUP, HtmlBoxTableRowGroup))
#define HTML_BOX_TABLE_ROW_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_BOX_TABLE_ROW_GROUP, HtmlBoxTableRowGroupClasss))
#define HTML_IS_BOX_TABLE_ROW_GROUP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), HTML_TYPE_BOX_TABLE_ROW_GROUP))
#define HTML_IS_BOX_TABLE_ROW_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_BOX_TABLE_ROW_GROUP))

struct _HtmlBoxTableRowGroup {
	HtmlBox parent_object;
	HtmlDisplayType type;
};

struct _HtmlBoxTableRowGroupClass {
	HtmlBoxClass parent_class;
};

GType    html_box_table_row_group_get_type (void);
HtmlBox *html_box_table_row_group_new      (HtmlDisplayType type);

G_END_DECLS

#endif /* __HTMLBOXTABLE_ROW_GROUP_H__ */
