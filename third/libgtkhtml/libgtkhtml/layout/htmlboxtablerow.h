#ifndef __HTMLBOXTABLE_ROW_H__
#define __HTMLBOXTABLE_ROW_H__

#include <gdk/gdk.h>
#include "htmlbox.h"

G_BEGIN_DECLS

typedef struct _HtmlBoxTableRow HtmlBoxTableRow;
typedef struct _HtmlBoxTableRowClass HtmlBoxTableRowClass;

#define HTML_TYPE_BOX_TABLE_ROW (html_box_table_row_get_type ())
#define HTML_BOX_TABLE_ROW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), HTML_TYPE_BOX_TABLE_ROW, HtmlBoxTableRow))
#define HTML_BOX_TABLE_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_BOX_TABLE_ROW, HtmlBoxTableRowClasss))
#define HTML_IS_BOX_TABLE_ROW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), HTML_TYPE_BOX_TABLE_ROW))
#define HTML_IS_BOX_TABLE_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_BOX_TABLE_ROW))

struct _HtmlBoxTableRow {
	HtmlBox parent_object;
};

struct _HtmlBoxTableRowClass {
	HtmlBoxClass parent_class;
};

GType html_box_table_row_get_type (void);
HtmlBox *html_box_table_row_new (void);

gint html_box_table_row_get_num_cols     (HtmlBox *self, gint rownum);
gint html_box_table_row_update_spaninfo  (HtmlBoxTableRow *row, gint *spaninfo);
gint html_box_table_row_fill_cells_array (HtmlBox *self, HtmlBox **cells, gint *spaninfo);

G_END_DECLS

#endif /* __HTMLBOXTABLE_ROW_H__ */
