#ifndef __HTMLBOXTABLECELL_H__
#define __HTMLBOXTABLECELL_H__

#include <gdk/gdk.h>
#include "htmlboxblock.h"

G_BEGIN_DECLS

typedef struct _HtmlBoxTableCell HtmlBoxTableCell;
typedef struct _HtmlBoxTableCellClass HtmlBoxTableCellClass;

#define HTML_TYPE_BOX_TABLE_CELL (html_box_table_cell_get_type ())
#define HTML_BOX_TABLE_CELL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), HTML_TYPE_BOX_TABLE_CELL, HtmlBoxTableCell))
#define HTML_BOX_TABLE_CELL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_BOX_TABLE_CELL, HtmlBoxTableCellClasss))
#define HTML_IS_BOX_TABLE_CELL(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), HTML_TYPE_BOX_TABLE_CELL))
#define HTML_IS_BOX_TABLE_CELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_BOX_TABLE_CELL))

struct _HtmlBoxTableCell {
	HtmlBoxBlock parent_object;
	HtmlBoxTable *table;
	gint width, height;
	gint rowspan, colspan;
};

struct _HtmlBoxTableCellClass {
	HtmlBoxBlockClass parent_class;
};

GType html_box_table_cell_get_type (void);
HtmlBox *html_box_table_cell_new (void);

gint html_box_table_cell_get_colspan (HtmlBoxTableCell *cell);
gint html_box_table_cell_get_rowspan (HtmlBoxTableCell *cell);

gint html_box_table_cell_get_min_width (HtmlBoxTableCell *cell, HtmlRelayout *relayout);
gint html_box_table_cell_get_max_width (HtmlBoxTableCell *cell, HtmlRelayout *relayout);

void html_box_table_cell_relayout_width (HtmlBoxTableCell *cell, HtmlRelayout *relayout, gint width);
void html_box_table_cell_do_valign      (HtmlBoxTableCell *cell, gint height);

G_END_DECLS

#endif /* __HTMLBOXTABLECELL_H__ */





