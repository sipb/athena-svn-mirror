/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.
    Copyright (C) 2001 Ximian, Inc.
    Authors: Radek Doulik

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include "htmlcluealigned.h"
#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlimage.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-table.h"
#include "htmlengine-edit-tablecell.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmltablepriv.h"
#include "htmlundo.h"

static void delete_table_column (HTMLEngine *e, HTMLUndoDirection dir);
static void insert_table_column (HTMLEngine *e, gboolean after, HTMLTableCell **column_cells, HTMLUndoDirection dir);
static void delete_table_row    (HTMLEngine *e, HTMLUndoDirection dir);
static void insert_table_row    (HTMLEngine *e, gboolean after, HTMLTableCell **row_cells, HTMLUndoDirection dir);

HTMLTable *
html_engine_get_table (HTMLEngine *e)
{
	if (HTML_IS_TABLE (e->cursor->object))
		return HTML_TABLE (e->cursor->object);
	else if (!e->cursor->object->parent
		 || !e->cursor->object->parent->parent
		 || !e->cursor->object->parent->parent->parent)
		return NULL;
	else if (!HTML_IS_TABLE (e->cursor->object->parent->parent->parent))
		return NULL;
	else
		return HTML_TABLE (e->cursor->object->parent->parent->parent);
}

static HTMLTableCell *
new_cell (HTMLEngine *e, HTMLTable *table)
{
	HTMLObject    *cell;
	HTMLObject    *text;
	HTMLObject    *flow;
	
	cell  = html_table_cell_new (1, 1, table->padding);
	flow  = html_clueflow_new (HTML_CLUEFLOW_STYLE_NORMAL, 0, HTML_LIST_TYPE_UNORDERED, 0);
	text  = html_engine_new_text_empty (e);

	html_clue_append (HTML_CLUE (flow), text);
	html_clue_append (HTML_CLUE (cell), flow);

	return HTML_TABLE_CELL (cell);
}

/*
 * Table insertion
 */

/**
 * html_engine_insert_table_1_1:
 * @e: An html engine
 *
 * Inserts new table with one cell containing an empty flow with an empty text. Inserted table has 1 row and 1 column.
 **/

void
html_engine_insert_table_1_1 (HTMLEngine *e)
{
	HTMLObject    *table;

	table = html_table_new (0, 100, 1, 2, 1);

	html_table_add_cell (HTML_TABLE (table), new_cell (e, HTML_TABLE (table)));

	html_engine_append_object (e, table, 2);
	html_cursor_backward (e->cursor, e);
}

/**
 * html_engine_insert_table_1_1:
 * @e: An html engine
 *
 * Inserts new table with @cols columns and @rows rows. Cells contain an empty flow with an empty text.
 **/

void
html_engine_insert_table (HTMLEngine *e, gint cols, gint rows, gint width, gint percent,
			  gint padding, gint spacing, gint border)
{
	HTMLObject *table;
	gint r, c;

	g_return_if_fail (cols >= 0);
	g_return_if_fail (rows >= 0);

	table = html_table_new (width, percent, padding, spacing, border);

	for (r = 0; r < rows; r ++) {
		html_table_start_row (HTML_TABLE (table));
		for (c = 0; c < cols; c ++)
			html_table_add_cell (HTML_TABLE (table), new_cell (e, HTML_TABLE (table)));
		html_table_end_row (HTML_TABLE (table));
	}

	html_engine_append_object (e, table, 1 + rows*cols);
	html_cursor_backward_n (e->cursor, e, rows*cols);
}

/*
 *  Insert Column
 */

static void
insert_column_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir)
{
	delete_table_column (e, html_undo_direction_reverse (dir));
}

static void
insert_column_setup_undo (HTMLEngine *e, HTMLUndoDirection dir)
{
	html_undo_add_action (e->undo,
			      html_undo_action_new ("Insert table column", insert_column_undo_action,
						    NULL, html_cursor_get_position (e->cursor)),
			      dir);
}

static void
insert_table_column (HTMLEngine *e, gboolean after, HTMLTableCell **column, HTMLUndoDirection dir)
{
	HTMLTable *t;
	HTMLTableCell *cell;
	gint c, r, col, delta = 0, row, first_row = -1;

	t = HTML_TABLE (html_object_nth_parent (e->cursor->object, 3));
	if (!t)
		return;

	html_engine_freeze (e);

	cell = HTML_TABLE_CELL (html_object_nth_parent (e->cursor->object, 2)); 
	col  = cell->col + (after ? 1 : 0);
	row  = cell->row;

	html_table_alloc_cell (t, 0, t->totalCols);

	for (r = 0; r < t->totalRows; r ++) {
		for (c = t->totalCols - 1; c > col; c --) {
			HTMLTableCell *cell = t->cells [r][c - 1];

			if (cell && cell->col >= col) {
				if (cell->row == r && cell->col == c - 1)
					html_table_cell_set_position (cell, r, c);
				t->cells [r][c] = cell;
				t->cells [r][c - 1] = NULL;
			}
		}
		if (!t->cells [r][col]) {
			guint len;

			cell = column
				? HTML_TABLE_CELL (html_object_op_copy (HTML_OBJECT (column [r]), e, NULL, NULL, &len))
				: new_cell (e, t);
			html_table_set_cell (t, r, col, cell);
			html_table_cell_set_position (t->cells [r][col], r, col);
			if (r < row || (!after && r == row))
				delta += html_object_get_recursive_length (HTML_OBJECT (cell)) + 1;
			if (first_row == -1)
				first_row = r;
		}
	}

	e->cursor->position += delta;

	/* now we have right position, let move to the first cell of this new column */
	if (first_row != -1) {
		HTMLTableCell *cell;
		gboolean end;
		do {
			cell = HTML_TABLE_CELL (html_object_nth_parent (e->cursor->object, 2));
			if (cell->col == col && cell->row == first_row)
				break;
			if (after && first_row >= row)
				end = html_cursor_forward (e->cursor, e);
			else
				end = html_cursor_backward (e->cursor, e);
		} while (end);
	} else
		g_warning ("no new cells added\n");

	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	insert_column_setup_undo (e, dir);
	html_engine_thaw (e);
}

/**
 * html_engine_insert_table_column:
 * @e: An HTML engine.
 * @after: If TRUE then inserts new column after current one, defined by current cursor position.
 *         If FALSE then inserts before current one.
 *
 * Inserts new column into table after/before current column.
 **/

void
html_engine_insert_table_column (HTMLEngine *e, gboolean after)
{
	insert_table_column (e, after, NULL, HTML_UNDO_UNDO);
}

/*
 * Delete column
 */

struct _DeleteCellsUndo {
	HTMLUndoData data;

	HTMLTableCell **cells;
	gint size;
	gboolean after;
};
typedef struct _DeleteCellsUndo DeleteCellsUndo;

static void
delete_cells_undo_destroy (HTMLUndoData *undo_data)
{
	DeleteCellsUndo *data = (DeleteCellsUndo *) undo_data;
	gint i;

	for (i = 0; i < data->size; i ++)
		html_object_destroy (HTML_OBJECT (data->cells [i]));
}

static DeleteCellsUndo *
delete_cells_undo_new (HTMLTableCell **cells, gint size, gboolean after)
{
	DeleteCellsUndo *data;

	data = g_new0 (DeleteCellsUndo, 1);

	html_undo_data_init (HTML_UNDO_DATA (data));

	data->data.destroy = delete_cells_undo_destroy;
	data->cells        = cells;
	data->after        = after;
	data->size         = size;

	return data;
}

static void
delete_column_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	DeleteCellsUndo *data = (DeleteCellsUndo *) undo_data;

	g_assert (data->size == HTML_TABLE (html_object_nth_parent (e->cursor->object, 3))->totalRows);
	insert_table_column (e, data->after, data->cells, html_undo_direction_reverse (dir));
}

static void
delete_column_setup_undo (HTMLEngine *e, HTMLTableCell **column, gint size, gboolean after, HTMLUndoDirection dir)
{
	DeleteCellsUndo *undo_data;

	undo_data = delete_cells_undo_new (column, size, after);

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Delete table column", delete_column_undo_action,
						    HTML_UNDO_DATA (undo_data), html_cursor_get_position (e->cursor)), dir);
}

static void
delete_table_column (HTMLEngine *e, HTMLUndoDirection dir)
{
	HTMLTable *t;
	HTMLTableCell *cell;
	HTMLTableCell **column;
	HTMLObject *co;
	gint r, c, col, delta = 0;

	t = HTML_TABLE (html_object_nth_parent (e->cursor->object, 3));

	/* this command is valid only in table and when this table has > 1 column */
	if (!t || !HTML_IS_TABLE (HTML_OBJECT (t)) || t->totalCols < 2)
		return;

	html_engine_freeze (e);

	cell   = HTML_TABLE_CELL (html_object_nth_parent (e->cursor->object, 2));
	col    = cell->col;
	column = g_new0 (HTMLTableCell *, t->totalRows);

	/* move cursor after/before this column (always keep it in table!) */
	do {
		if (col != t->totalCols - 1)
			html_cursor_forward (e->cursor, e);
		else
			html_cursor_backward (e->cursor, e);
		co = html_object_nth_parent (e->cursor->object, 2);
	} while (co && co->parent == HTML_OBJECT (t)
		 && HTML_OBJECT_TYPE (co) == HTML_TYPE_TABLECELL && HTML_TABLE_CELL (co)->col == col);

	for (r = 0; r < t->totalRows; r ++) {
		cell = t->cells [r][col];

		/* remove & keep old one */
		if (cell && cell->col == col) {
			HTML_OBJECT (cell)->parent = NULL;
			column [r] = cell;
			t->cells [r][col] = NULL;
			delta += html_object_get_recursive_length (HTML_OBJECT (cell));
			delta ++;
		}

		for (c = col + 1; c < t->totalCols; c ++) {
			cell = t->cells [r][c];
			if (cell && cell->col != col) {
				if (cell->row == r && cell->col == c)
					html_table_cell_set_position (cell, r, c - 1);
				t->cells [r][c - 1] = cell;
				t->cells [r][c]     = NULL;
			}
		}
	}

	if (col != t->totalCols - 1)
		e->cursor->position -= delta;
	t->totalCols --;

	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	delete_column_setup_undo (e, column, t->totalRows, col != t->totalCols - 1, dir);
	html_engine_thaw (e);
}

/**
 * html_engine_delete_table_column:
 * @e: An HTML engine.
 *
 * Deletes current table column.
 **/

void
html_engine_delete_table_column (HTMLEngine *e)
{
	delete_table_column (e, HTML_UNDO_UNDO);
}

/*
 *  Insert Row
 */

static void
insert_row_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir)
{
	delete_table_row (e, html_undo_direction_reverse (dir));
}

static void
insert_row_setup_undo (HTMLEngine *e, HTMLUndoDirection dir)
{
	html_undo_add_action (e->undo,
			      html_undo_action_new ("Insert table row", insert_row_undo_action,
						    NULL, html_cursor_get_position (e->cursor)),
			      dir);
}

static void
insert_table_row (HTMLEngine *e, gboolean after, HTMLTableCell **row_cells, HTMLUndoDirection dir)
{
	HTMLTable *t;
	HTMLTableCell *cell;
	gint r, c, ntr, row, delta = 0, first_col = -1;

	t = HTML_TABLE (html_object_nth_parent (e->cursor->object, 3));
	if (!t)
		return;

	html_engine_freeze (e);

	cell = HTML_TABLE_CELL (html_object_nth_parent (e->cursor->object, 2));
	row  = cell->row + (after ? 1 : 0);
	ntr  = t->totalRows + 1;

	html_table_alloc_cell (t, t->totalRows, 0);

	for (c = 0; c < t->totalCols; c ++) {
		for (r = ntr - 1; r > row; r --) {
			HTMLTableCell *cell = t->cells [r - 1][c];

			if (cell && cell->row >= row) {
				if (cell->row == r - 1 && cell->col == c)
					html_table_cell_set_position (cell, r, c);
				t->cells [r][c]     = cell;
				t->cells [r - 1][c] = NULL;
			}
		}
		if (!t->cells [row][c]) {
			guint len;

			cell = row_cells
				? HTML_TABLE_CELL (html_object_op_copy (HTML_OBJECT (row_cells [c]), e, NULL, NULL, &len))
				:  new_cell (e, t);
			html_table_set_cell (t, row, c, cell);
			html_table_cell_set_position (t->cells [row][c], row, c);
			delta += html_object_get_recursive_length (HTML_OBJECT (cell)) + 1;
			if (first_col == -1)
				first_col = c;
		}
	}

	if (!after)
		e->cursor->position += delta;

	/* now we have right position, let move to the first cell of this new row */
	if (delta) {
		HTMLTableCell *cell;
		gboolean end;
		do {
			cell = HTML_TABLE_CELL (html_object_nth_parent (e->cursor->object, 2));
			if (cell->col == first_col && cell->row == row)
				break;
			if (after)
				end = html_cursor_forward (e->cursor, e);
			else
				end = html_cursor_backward (e->cursor, e);
		} while (end);
	} else
		g_warning ("no new cells added\n");

	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	insert_row_setup_undo (e, dir);
	html_engine_thaw (e);
}

/**
 * html_engine_insert_table_row:
 * @e: An HTML engine.
 * @after: If TRUE then inserts new row after current one, defined by current cursor position.
 *         If FALSE then inserts before current one.
 *
 * Inserts new row into table after/before current row.
 **/

void
html_engine_insert_table_row (HTMLEngine *e, gboolean after)
{
	g_print ("html_engine_insert_table_row\n");
	insert_table_row (e, after, NULL, HTML_UNDO_UNDO);
}

/*
 * Delete row
 */

static void
delete_row_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	DeleteCellsUndo *data = (DeleteCellsUndo *) undo_data;

	g_assert (data->size == HTML_TABLE (html_object_nth_parent (e->cursor->object, 3))->totalCols);
	insert_table_row (e, data->after, data->cells, html_undo_direction_reverse (dir));
}

static void
delete_row_setup_undo (HTMLEngine *e, HTMLTableCell **row_cells, gint size, gboolean after, HTMLUndoDirection dir)
{
	DeleteCellsUndo *undo_data;

	undo_data = delete_cells_undo_new (row_cells, size, after);

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Delete table row", delete_row_undo_action,
						    HTML_UNDO_DATA (undo_data), html_cursor_get_position (e->cursor)), dir);
}

static void
delete_table_row (HTMLEngine *e, HTMLUndoDirection dir)
{
	HTMLTable *t;
	HTMLTableCell *cell;
	HTMLTableCell **row_cells;
	HTMLObject *co;
	gint r, c, row, delta = 0;

	t = HTML_TABLE (html_object_nth_parent (e->cursor->object, 3));

	/* this command is valid only in table and when this table has > 1 row */
	if (!t || !HTML_IS_TABLE (HTML_OBJECT (t)) || t->totalRows < 2)
		return;

	html_engine_freeze (e);

	cell      = HTML_TABLE_CELL (html_object_nth_parent (e->cursor->object, 2));
	row       = cell->row;
	row_cells = g_new0 (HTMLTableCell *, t->totalCols);

	/* move cursor after/before this row (always keep it in table!) */
	do {
		if (row != t->totalRows - 1)
			html_cursor_forward (e->cursor, e);
		else
			html_cursor_backward (e->cursor, e);
		co = html_object_nth_parent (e->cursor->object, 2);
	} while (co && co->parent == HTML_OBJECT (t)
		 && HTML_OBJECT_TYPE (co) == HTML_TYPE_TABLECELL && HTML_TABLE_CELL (co)->row == row);

	for (c = 0; c < t->totalCols; c ++) {
		cell = t->cells [row][c];

		/* remove & keep old one */
		if (cell && cell->row == row) {
			HTML_OBJECT (cell)->parent = NULL;
			row_cells [c] = cell;
			t->cells [row][c] = NULL;
			delta += html_object_get_recursive_length (HTML_OBJECT (cell)) + 1;
		}

		for (r = row + 1; r < t->totalRows; r ++) {
			cell = t->cells [r][c];
			if (cell && cell->row != row) {
				if (cell->row == r && cell->col == c)
					html_table_cell_set_position (cell, r - 1, c);
				t->cells [r - 1][c] = cell;
				t->cells [r][c]     = NULL;
			}
		}
	}

	if (row != t->totalRows - 1)
		e->cursor->position -= delta;
	t->totalRows --;

	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	delete_row_setup_undo (e, row_cells, t->totalCols, row != t->totalRows - 1, dir);
	html_engine_thaw (e);
}

/**
 * html_engine_delete_table_row:
 * @e: An HTML engine.
 *
 * Deletes current table row.
 **/

void
html_engine_delete_table_row (HTMLEngine *e)
{
	delete_table_row (e, HTML_UNDO_UNDO);
}

typedef enum {
	HTML_TABLE_BORDER,
	HTML_TABLE_PADDING,
	HTML_TABLE_SPACING,
	HTML_TABLE_WIDTH,
	HTML_TABLE_BGCOLOR,
	HTML_TABLE_BGPIXMAP,
	HTML_TABLE_ALIGN,
} HTMLTableAttrType;

union _HTMLTableUndoAttr {
	gint border;
	gint spacing;
	gint padding;

	gchar *pixmap;

	struct {
		gint width;
		gboolean percent;
	} width;

	struct {
		GdkColor color;
		gboolean has_bg_color;
	} color;

	HTMLHAlignType align;
};
typedef union _HTMLTableUndoAttr HTMLTableUndoAttr;

struct _HTMLTableSetAttrUndo {
	HTMLUndoData data;

	HTMLTableUndoAttr attr;
	HTMLTableAttrType type;
};
typedef struct _HTMLTableSetAttrUndo HTMLTableSetAttrUndo;

static void
attr_destroy (HTMLUndoData *undo_data)
{
	HTMLTableSetAttrUndo *data = (HTMLTableSetAttrUndo *) undo_data;

	switch (data->type) {
	case HTML_TABLE_BGPIXMAP:
		g_free (data->attr.pixmap);
		break;
	default:
		;
	}
}

static HTMLTableSetAttrUndo *
attr_undo_new (HTMLTableAttrType type)
{
	HTMLTableSetAttrUndo *undo = g_new (HTMLTableSetAttrUndo, 1);

	html_undo_data_init (HTML_UNDO_DATA (undo));
	undo->data.destroy = attr_destroy;
	undo->type         = type;

	return undo;
}

/*
 * Border width
 */

static void table_set_border_width (HTMLEngine *e, HTMLTable *t, gint border_width, gboolean relative, HTMLUndoDirection dir);

static void
table_set_border_width_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	table_set_border_width (e, html_engine_get_table (e), ((HTMLTableSetAttrUndo *) undo_data)->attr.border, FALSE,
				html_undo_direction_reverse (dir));
}

static void
table_set_border_width (HTMLEngine *e, HTMLTable *t, gint border_width, gboolean relative, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *undo;
	gint new_border;

	if (!t || !HTML_IS_TABLE (t))
		return;

	if (relative)
		new_border = t->border + border_width;
	else
		new_border = border_width;
	if (new_border < 0)
		new_border = 0;
	if (new_border == t->border)
		return;

	undo = attr_undo_new (HTML_TABLE_BORDER);
	undo->attr.border = t->border;

	html_engine_freeze (e);
	t->border = new_border;

	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	html_engine_thaw (e);

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Set table border width", table_set_border_width_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)), dir);
}

void
html_engine_table_set_border_width (HTMLEngine *e, HTMLTable *t, gint border_width, gboolean relative)
{
	table_set_border_width (e, t, border_width, relative, HTML_UNDO_UNDO);
}

/*
 * bg color
 *
 */

static void table_set_bg_color (HTMLEngine *e, HTMLTable *t, GdkColor *c, HTMLUndoDirection dir);

static void
table_set_bg_color_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *data = (HTMLTableSetAttrUndo *) undo_data;

	table_set_bg_color (e, html_engine_get_table (e), data->attr.color.has_bg_color
			    ? &data->attr.color.color : NULL, html_undo_direction_reverse (dir));
}

static void
table_set_bg_color (HTMLEngine *e, HTMLTable *t, GdkColor *c, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *undo;

	undo = attr_undo_new (HTML_TABLE_BGCOLOR);
	if (t->bgColor) {
		undo->attr.color.color        = *t->bgColor;
		undo->attr.color.has_bg_color = TRUE;
	} else
		undo->attr.color.has_bg_color = FALSE;
	html_undo_add_action (e->undo,
			      html_undo_action_new ("Set table background color", table_set_bg_color_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)), dir);
	if (c) {
		if (!t->bgColor)
			t->bgColor = gdk_color_copy (c);
		*t->bgColor = *c;
	} else {
		if (t->bgColor)
			gdk_color_free (t->bgColor);
		t->bgColor = NULL;
	}
	html_engine_queue_draw (e, HTML_OBJECT (t));
}

void
html_engine_table_set_bg_color (HTMLEngine *e, HTMLTable *t, GdkColor *c)
{
	table_set_bg_color (e, t, c, HTML_UNDO_UNDO);
}

/*
 * bg pixmap
 *
 */

void table_set_bg_pixmap (HTMLEngine *e, HTMLTable *t, gchar *url, HTMLUndoDirection dir);

static void
table_set_bg_pixmap_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *data = (HTMLTableSetAttrUndo *) undo_data;

	table_set_bg_pixmap (e, html_engine_get_table (e), data->attr.pixmap, html_undo_direction_reverse (dir));
}

void
table_set_bg_pixmap (HTMLEngine *e, HTMLTable *t, gchar *url, HTMLUndoDirection dir)
{
	HTMLImagePointer *iptr;
	HTMLTableSetAttrUndo *undo;

	undo = attr_undo_new (HTML_TABLE_BGPIXMAP);
	undo->attr.pixmap = t->bgPixmap ? g_strdup (t->bgPixmap->url) : NULL;
	html_undo_add_action (e->undo,
			      html_undo_action_new ("Set table background pixmap", table_set_bg_pixmap_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)), dir);

	iptr = t->bgPixmap;
	t->bgPixmap = url ? html_image_factory_register (e->image_factory, NULL, url, TRUE) : NULL;
	if (iptr)
		html_image_factory_unregister (e->image_factory, iptr, NULL);
	html_engine_queue_draw (e, HTML_OBJECT (t));
}

void
html_engine_table_set_bg_pixmap (HTMLEngine *e, HTMLTable *t, gchar *url)
{
	table_set_bg_pixmap (e, t, url, HTML_UNDO_UNDO);
}

/*
 * spacing
 *
 */

static void table_set_spacing (HTMLEngine *e, HTMLTable *t, gint spacing, gboolean relative, HTMLUndoDirection dir);

static void
table_set_spacing_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *data = (HTMLTableSetAttrUndo *) undo_data;

	table_set_spacing (e, html_engine_get_table (e), data->attr.spacing, FALSE, html_undo_direction_reverse (dir));
}

static void
table_set_spacing (HTMLEngine *e, HTMLTable *t, gint spacing, gboolean relative, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *undo;
	gint new_spacing;

	if (!t || !HTML_IS_TABLE (t))
		return;

	if (relative)
		new_spacing = t->spacing + spacing;
	else
		new_spacing = spacing;
	if (new_spacing < 0)
		new_spacing = 0;
	if (new_spacing == t->spacing)
		return;

	undo = attr_undo_new (HTML_TABLE_SPACING);
	undo->attr.spacing = t->spacing;
	html_undo_add_action (e->undo,
			      html_undo_action_new ("Set table spacing", table_set_spacing_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)), dir);
	t->spacing = new_spacing;
	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	html_engine_schedule_update (e);
}

void
html_engine_table_set_spacing (HTMLEngine *e, HTMLTable *t, gint spacing, gboolean relative)
{
	table_set_spacing (e, t, spacing, relative, HTML_UNDO_UNDO);
}

/*
 * padding
 *
 */

static void table_set_padding (HTMLEngine *e, HTMLTable *t, gint padding, gboolean relative, HTMLUndoDirection dir);

static void
table_set_padding_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *data = (HTMLTableSetAttrUndo *) undo_data;

	table_set_padding (e, html_engine_get_table (e), data->attr.padding, FALSE, html_undo_direction_reverse (dir));
}

static void
table_set_padding (HTMLEngine *e, HTMLTable *t, gint padding, gboolean relative, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *undo;
	gint r, c;
	gint new_padding;

	if (!t || !HTML_IS_TABLE (t))
		return;

	if (relative)
		new_padding = t->padding + padding;
	else
		new_padding = padding;
	if (new_padding < 0)
		new_padding = 0;
	if (new_padding == t->padding)
		return;

	undo = attr_undo_new (HTML_TABLE_PADDING);
	undo->attr.padding = t->padding;
	html_undo_add_action (e->undo,
			      html_undo_action_new ("Set table padding", table_set_padding_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)), dir);

	t->padding = new_padding;
	for (r = 0; r < t->totalRows; r ++)
		for (c = 0; c < t->totalCols; c ++)
			if (t->cells [r][c]->col == c && t->cells [r][c]->row == r) {
				HTML_CLUEV (t->cells [r][c])->padding = new_padding;
				HTML_OBJECT (t->cells [r][c])->change |= HTML_CHANGE_ALL_CALC;
			}
	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	html_engine_schedule_update (e);
}

void
html_engine_table_set_padding (HTMLEngine *e, HTMLTable *t, gint padding, gboolean relative)
{
	table_set_padding (e, t, padding, relative, HTML_UNDO_UNDO);
}

/*
 * align
 *
 */

static void table_set_align (HTMLEngine *e, HTMLTable *t, HTMLHAlignType align, HTMLUndoDirection dir);

static void
table_set_align_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *data = (HTMLTableSetAttrUndo *) undo_data;

	table_set_align (e, html_engine_get_table (e), data->attr.align, html_undo_direction_reverse (dir));
}

static void
table_set_align (HTMLEngine *e, HTMLTable *t, HTMLHAlignType align, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *undo;

	g_return_if_fail (HTML_OBJECT (t)->parent);

	undo = attr_undo_new (HTML_TABLE_ALIGN);
	undo->attr.align = HTML_CLUE (HTML_OBJECT (t)->parent)->halign;

	if (align == HTML_HALIGN_NONE || align == HTML_HALIGN_CENTER) {
		if (HTML_IS_CLUEALIGNED (HTML_OBJECT (t)->parent)) {
			HTMLObject *aclue = HTML_OBJECT (t)->parent;

			html_clue_remove (HTML_CLUE (aclue), HTML_OBJECT (t));
			html_clue_append_after (HTML_CLUE (aclue->parent), HTML_OBJECT (t), aclue);
			html_clue_remove (HTML_CLUE (aclue->parent), aclue);
			html_object_destroy (aclue);
		}
	} else if (align == HTML_HALIGN_LEFT || align == HTML_HALIGN_RIGHT) {
		if (HTML_IS_CLUEFLOW (HTML_OBJECT (t)->parent)) {
			HTMLObject *aclue, *flow = HTML_OBJECT (t)->parent;

			html_clue_remove (HTML_CLUE (flow), HTML_OBJECT (t));
			aclue = html_cluealigned_new (NULL, 0, 0, flow->max_width, 100);
			html_clue_append (HTML_CLUE (flow), aclue);
			html_clue_append (HTML_CLUE (aclue), HTML_OBJECT (t));
		}
	} else
		g_assert_not_reached ();

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Set table align", table_set_align_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)), dir);

	HTML_CLUE (HTML_OBJECT (t)->parent)->halign = align;
	html_object_change_set (HTML_OBJECT (t)->parent, HTML_CHANGE_ALL_CALC);
	html_engine_schedule_update (e);
}

void
html_engine_table_set_align (HTMLEngine *e, HTMLTable *t, HTMLHAlignType align)
{
	table_set_align (e, t, align, HTML_UNDO_UNDO);
}

/*
 * width
 *
 */

static void table_set_width (HTMLEngine *e, HTMLTable *t, gint width, gboolean percent, HTMLUndoDirection dir);

static void
table_set_width_undo_action (HTMLEngine *e, HTMLUndoData *undo_data, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *data = (HTMLTableSetAttrUndo *) undo_data;

	table_set_width (e, html_engine_get_table (e), data->attr.width.width, data->attr.width.percent,
			 html_undo_direction_reverse (dir));
}

static void
table_set_width (HTMLEngine *e, HTMLTable *t, gint width, gboolean percent, HTMLUndoDirection dir)
{
	HTMLTableSetAttrUndo *undo;

	undo = attr_undo_new (HTML_TABLE_WIDTH);
	undo->attr.width.width = HTML_OBJECT (t)->percent
		? HTML_OBJECT (t)->percent
		: (HTML_OBJECT (t)->flags & HTML_OBJECT_FLAG_FIXEDWIDTH
		   ? t->specified_width : 0);
	undo->attr.width.percent = HTML_OBJECT (t)->percent != 0;
	html_undo_add_action (e->undo,
			      html_undo_action_new ("Set table width", table_set_width_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)), dir);

	if (percent) {
		HTML_OBJECT (t)->percent = width;
		HTML_OBJECT (t)->flags  &= ~ HTML_OBJECT_FLAG_FIXEDWIDTH;
		t->specified_width       = 0;
	} else {
		HTML_OBJECT (t)->percent = 0;
		t->specified_width       = width;
		if (width)
			HTML_OBJECT (t)->flags |= HTML_OBJECT_FLAG_FIXEDWIDTH;
		else
			HTML_OBJECT (t)->flags &= ~ HTML_OBJECT_FLAG_FIXEDWIDTH;
	}
	html_object_change_set (HTML_OBJECT (t), HTML_CHANGE_ALL_CALC);
	html_engine_schedule_update (e);
}

void
html_engine_table_set_width (HTMLEngine *e, HTMLTable *t, gint width, gboolean percent)
{
	table_set_width (e, t, width, percent, HTML_UNDO_UNDO);
}

gboolean
html_engine_table_goto_0_0 (HTMLEngine *e)
{
	HTMLTableCell *cell;

	cell = html_engine_get_table_cell (e);
	while (cell && (cell->row != 0 || cell->col != 0)) {
		html_engine_prev_cell (e);
		cell = html_engine_get_table_cell (e);
	}

	return cell != NULL;
}

gboolean
html_engine_table_goto_col (HTMLEngine *e, gint col)
{
	HTMLTableCell *cell;

	if (html_engine_table_goto_0_0 (e)) {
		cell = html_engine_get_table_cell (e);
		while (cell && cell->col != col) {
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}

		return cell != NULL;
	}

	return FALSE;
}

gboolean
html_engine_table_goto_row (HTMLEngine *e, gint row)
{
	HTMLTableCell *cell;

	if (html_engine_table_goto_0_0 (e)) {
		cell = html_engine_get_table_cell (e);
		while (cell && cell->row != row) {
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}

		return cell != NULL;
	}

	return FALSE;
}


gboolean
html_engine_table_goto_pos (HTMLEngine *e, gint row, gint col)
{
	HTMLTableCell *cell;

	if (html_engine_table_goto_0_0 (e)) {
		cell = html_engine_get_table_cell (e);
		while (cell && cell->row != row && cell->col != col) {
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}

		return cell != NULL;
	}

	return FALSE;
}

/*
 * set number of columns in current table
 */

void
html_engine_table_set_cols (HTMLEngine *e, gint cols)
{
	HTMLTable *table = html_engine_get_table (e);

	if (!table)
		return;

	if (table->totalCols == cols)
		return;

	if (table->totalCols < cols) {
		html_engine_table_goto_col (e, table->totalCols - 1);
		while (table->totalCols < cols)
			html_engine_insert_table_column (e, TRUE);
	} else {
		html_engine_table_goto_col (e, table->totalCols - 1);
		while (table->totalCols > cols)
			html_engine_delete_table_column (e);
	}

	/*
	  FIXME: jump to some well defined position (like the same position as before
	  or at beginning of 1st new column or to last position in remaining cells
	*/
}

/*
 * set number of rows in current table
 */

void
html_engine_table_set_rows (HTMLEngine *e, gint rows)
{
	HTMLTable *table = html_engine_get_table (e);

	if (!table)
		return;

	if (table->totalRows == rows)
		return;

	if (table->totalRows < rows) {
		html_engine_table_goto_row (e, table->totalRows - 1);
		while (table->totalRows < rows)
			html_engine_insert_table_row (e, TRUE);
	} else {
		html_engine_table_goto_row (e, table->totalRows - 1);
		while (table->totalRows > rows)
			html_engine_delete_table_row (e);
	}

	/*
	  FIXME: like set_cols
	*/
}

void
html_engine_delete_table (HTMLEngine *e)
{
	HTMLTable *table;

	table = html_engine_get_table (e);

	if (!table)
		return;
	while (e->cursor->object != HTML_OBJECT (table) || e->cursor->offset)
		html_cursor_backward (e->cursor, e);
	html_engine_set_mark (e);
	html_cursor_end_of_line (e->cursor, e);
	html_engine_delete (e);
}
