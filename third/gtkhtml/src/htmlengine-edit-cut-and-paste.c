/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000, 2001 Helix Code, Inc.
    Authors:                 Radek Doulik (rodo@helixcode.com)

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
#include <stdio.h>
#include <string.h>

#include "gtkhtmldebug.h"
#include "gtkhtml-private.h"
#include "gtkhtml-properties.h"

#include "htmlclue.h"
#include "htmlcursor.h"
#include "htmlcolorset.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cursor.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlinterval.h"
#include "htmllinktext.h"
#include "htmlobject.h"
#include "htmltable.h"
#include "htmlselection.h"
#include "htmlsettings.h"
#include "htmltext.h"
#include "htmlundo.h"
#include "htmlundo-action.h"

static gint        delete_object (HTMLEngine *e, HTMLObject **ret_object, guint *ret_len, HTMLUndoDirection dir,
				  gboolean add_prop);
static void        insert_object (HTMLEngine *e, HTMLObject *obj, guint len, guint position_after, gint level,
				  HTMLUndoDirection dir, gboolean check);
static void        append_object (HTMLEngine *e, HTMLObject *o, guint len, HTMLUndoDirection dir);
static void        insert_empty_paragraph (HTMLEngine *e, HTMLUndoDirection dir, gboolean add_undo);

/* helper functions -- need refactor */

/* #define OP_DEBUG */

static void
html_cursor_get_left (HTMLCursor *cursor, HTMLObject **obj, gint *off)
{
	if (cursor->offset == 0) {
		*obj = html_object_prev_not_slave (cursor->object);
		if (*obj) {
			*off = html_object_get_length (*obj);
			return;
		}
	}
	*obj = cursor->object;
	*off = cursor->offset;
}

static void
html_point_get_left (HTMLPoint *source, HTMLPoint *dest)
{
	if (source->offset == 0) {
		dest->object = html_object_prev_not_slave (source->object);
		if (dest->object) {
			dest->offset = html_object_get_length (dest->object);
			return;
		}
	}

	*dest = *source;
}

static void
html_point_get_right (HTMLPoint *source, HTMLPoint *dest)
{
	if (source->offset >= html_object_get_length (source->object)) {
		dest->object = html_object_next_not_slave (source->object);
		if (dest->object) {
			dest->offset = 0;
			return;
		}
	}

	*dest = *source;
}

static void
object_get_parent_list (HTMLObject *o, gint level, GList **list)
{
	while (level > 0 && o) {
		*list = g_list_prepend (*list, o);
		o = o->parent;
		level--;
	}
}

static GList *
point_get_parent_list (HTMLPoint *point, gint level, gboolean include_offset)
{
	GList *list;
	HTMLObject *o;

	list = include_offset ? g_list_prepend (NULL, GINT_TO_POINTER (point->offset)) : NULL;
	o    = point->object;

	object_get_parent_list (point->object, level, &list);

	return list;
}

static gint
get_parent_depth (HTMLObject *o, HTMLObject *parent)
{
	gint level = 1;

	while (o && parent && o != parent) {
		o = o->parent;
		level ++;
	}

	return level;
}

static gboolean
is_parent (HTMLObject *o,HTMLObject *parent)
{
	while (o) {
		if (o == parent)
			return TRUE;
		o = o->parent;
	}

	return FALSE;
}

static HTMLObject *
try_find_common_parent_of (HTMLObject *child, HTMLObject *parent)
{
	while (parent) {
		if (is_parent (child, parent))
			return parent;
		parent = parent->parent;
	}

	return NULL;
}

static HTMLObject *
get_common_parent (HTMLObject *from, HTMLObject *to)
{
	HTMLObject *parent;

	parent = try_find_common_parent_of (from, to);

	return parent ? parent : try_find_common_parent_of (to, from);
}

static gint
prepare_delete_bounds (HTMLEngine *e, GList **from_list, GList **to_list,
		       GList **bound_left, GList **bound_right)
{
	HTMLPoint b_left, b_right, begin, end;
	HTMLObject *common_parent;
	gint ret_level;

	g_assert (e->selection);

	html_point_get_right (&e->selection->from, &begin);
	html_point_get_left  (&e->selection->to,   &end);

	common_parent = get_common_parent (begin.object, end.object);
	ret_level     = get_parent_depth (begin.object, common_parent);

	*from_list = point_get_parent_list (&begin, get_parent_depth (begin.object, common_parent), TRUE);
	*to_list   = point_get_parent_list (&end,   get_parent_depth (end.object, common_parent),   TRUE);

	if (bound_left && bound_right) {
		gint level;

		html_point_get_left  (&e->selection->from, &b_left);
		html_point_get_right (&e->selection->to,   &b_right);

		common_parent = get_common_parent (b_left.object, b_right.object);

		level = get_parent_depth (b_left.object, common_parent);
		*bound_left  = b_left.object  ? point_get_parent_list (&b_left, level - 1, FALSE) : NULL;
		if (level > 1 && *bound_left)
			*bound_left  = g_list_prepend (*bound_left, NULL);

		level = get_parent_depth (b_right.object, common_parent);
		*bound_right = b_right.object ? point_get_parent_list (&b_right, level - 1, FALSE) : NULL;
		if (level > 1 && *bound_right)
			*bound_right = g_list_prepend (*bound_right, NULL);
	}

	return ret_level;
}

static void
remove_empty_and_merge (HTMLEngine *e, gboolean merge, GList *left, GList *right, HTMLCursor *c)
{
	HTMLObject *lo, *ro, *prev;

#ifdef OP_DEBUG
	printf ("before merge\n");
	gtk_html_debug_dump_tree_simple (e->clue, 0);
	if (left && left->data) {
		printf ("left\n");
		gtk_html_debug_dump_tree_simple (left->data, 0);
	}

	if (right && right->data) {
		printf ("right\n");
		gtk_html_debug_dump_tree_simple (right->data, 0);
	}
#endif
	while (left && left->data && right && right->data) {

		lo  = HTML_OBJECT (left->data);
		ro  = HTML_OBJECT (right->data);

		left  = left->next;
		right = right->next;

		if (html_object_is_text (lo) && !*HTML_TEXT (lo)->text && (html_object_prev_not_slave (lo) || merge)) {
			HTMLObject *nlo = html_object_prev_not_slave (lo);

			if (e->cursor->object == lo) {
				e->cursor->object = ro;
				e->cursor->offset = 0;
			}
			if (c && c->object == lo) {
				c->object = ro;
				c->offset = 0;
			}

			html_object_remove_child (lo->parent, lo);
			html_object_destroy (lo);
			lo = nlo;
		} else if (html_object_is_text (ro) && !*HTML_TEXT (ro)->text && (html_object_next_not_slave (ro) || merge)) {
			HTMLObject *nro = html_object_next_not_slave (ro);

			if (e->cursor->object == ro) {
				e->cursor->object = lo;
				e->cursor->offset = html_object_get_length (lo);
			}

			html_object_remove_child (ro->parent, ro);
			html_object_destroy (ro);
			ro = nro;
		}

		if (merge && lo && ro) {
			if (!html_object_merge (lo, ro, e, &left, &right, c))
				break;
			if (ro == e->cursor->object) {
				e->cursor->object  = lo;
				e->cursor->offset += html_object_get_length (lo);;
			}
		}
	}

	prev = html_object_prev_not_slave (e->cursor->object);
	if (prev && e->cursor->offset == 0) {
		e->cursor->object = prev;
		e->cursor->offset = html_object_get_length (e->cursor->object);
	}
#ifdef OP_DEBUG
	printf ("-- after\n");
	gtk_html_debug_dump_tree_simple (e->clue, 0);
	printf ("-- END merge\n");
#endif
}

static void
split_and_add_empty_texts (HTMLEngine *e, gint level, GList **left, GList **right)
{
#ifdef OP_DEBUG
	printf ("-- SPLIT begin\n");
	gtk_html_debug_dump_tree_simple (e->clue, 0);
	printf ("-- SPLIT middle\n");
#endif
	html_object_split (e->cursor->object, e, *right ? HTML_OBJECT ((*right)->data) : NULL,
			   e->cursor->offset, level, left, right);
#ifdef OP_DEBUG
	printf ("-- SPLIT middle\n");
	gtk_html_debug_dump_tree_simple (e->clue, 0);
	printf ("-- SPLIT end\n");
#endif
}

/* end of helper */

void
html_engine_copy_object (HTMLEngine *e, HTMLObject **o, guint *len)
{
	GList *from, *to;

	if (html_engine_is_selection_active (e)) {
		prepare_delete_bounds (e, &from, &to, NULL, NULL);
		*len = 0;
		*o    = html_object_op_copy (HTML_OBJECT (from->data), e,
					     from->next, to->next, len);
#ifdef OP_DEBUG
		printf ("copy len: %d (parent %p)\n", *len, (*o)->parent);
		gtk_html_debug_dump_tree_simple (*o, 0);
#endif
	}
}

void
html_engine_copy (HTMLEngine *e)
{
	html_engine_copy_object(e, &e->clipboard, &e->clipboard_len);
}

struct _DeleteUndo {
	HTMLUndoData data;

	HTMLObject *buffer;
	guint       buffer_len;
	gint        level;
};
typedef struct _DeleteUndo DeleteUndo;

static void
delete_undo_destroy (HTMLUndoData *data)
{
	DeleteUndo *undo = (DeleteUndo *) data;

	if (undo->buffer)
		html_object_destroy (undo->buffer);
}

static void
delete_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir, guint position_after)
{
	DeleteUndo *undo;
	HTMLObject *buffer;
	guint       len = 0;

	undo         = (DeleteUndo *) data;
	buffer       = html_object_op_copy (undo->buffer, e, NULL, NULL, &len);
	insert_object (e, buffer, undo->buffer_len, position_after, undo->level, html_undo_direction_reverse (dir), TRUE);
}

static void
delete_setup_undo (HTMLEngine *e, HTMLObject *buffer, guint len, guint position_after, gint level, HTMLUndoDirection dir)
{
	DeleteUndo *undo;

	undo = g_new (DeleteUndo, 1);

	html_undo_data_init (HTML_UNDO_DATA (undo));
	undo->data.destroy = delete_undo_destroy;
	undo->buffer       = buffer;
	undo->buffer_len   = len;
	undo->level        = level;

	/* printf ("delete undo len %d\n", len); */

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Delete", delete_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor),
						    position_after),
			      dir);
}

static void
move_cursor_before_delete (HTMLEngine *e)
{
	if (e->cursor->offset == 0) {
		if (html_object_prev_not_slave (e->cursor->object)) {
			HTMLObject *obj;
			gint off;

			html_cursor_get_left (e->cursor, &obj, &off);
			if (obj) {
				e->cursor->object = obj;
				e->cursor->offset = off;
			}
		}
	}
}

static void
place_cursor_before_mark (HTMLEngine *e)
{
	if (e->mark->position < e->cursor->position) {
		HTMLCursor *tmp;

		tmp = e->cursor;
		e->cursor = e->mark;
		e->mark = tmp;
	}
}

static gint
delete_object_do (HTMLEngine *e, HTMLObject **object, guint *len)
{
	GList *from, *to, *left, *right;
	guint position;
	gint level;

	html_engine_freeze (e);
	level = prepare_delete_bounds (e, &from, &to, &left, &right);
	place_cursor_before_mark (e);
	move_cursor_before_delete (e);
	html_engine_disable_selection (e);
	*len     = 0;
	*object  = html_object_op_cut  (HTML_OBJECT (from->data), e, from->next, to->next, left, right, len);
	position = e->cursor->position;
	remove_empty_and_merge (e, TRUE, left ? left->next : NULL, right ? right->next : NULL, NULL);
	e->cursor->position = position;
	html_engine_spell_check_range (e, e->cursor, e->cursor);
	html_engine_thaw (e);

	return level;
}

static void
check_table_0 (HTMLEngine *e)
{
	HTMLCursor *tail;

	tail = e->mark->position < e->cursor->position ? e->cursor : e->mark;

	while (tail->offset == 0 && HTML_IS_TABLE (tail->object) && e->mark->position != e->cursor->position)
		html_cursor_backward (tail, e);
}

static void
check_table_1 (HTMLEngine *e)
{
	HTMLCursor *head;

	head = e->mark->position > e->cursor->position ? e->cursor : e->mark;

	while (head->offset == 1 && HTML_IS_TABLE (head->object) && e->mark->position != e->cursor->position)
		html_cursor_forward (head, e);
}

static gint
delete_object (HTMLEngine *e, HTMLObject **ret_object, guint *ret_len, HTMLUndoDirection dir, gboolean add_undo)
{
	html_engine_edit_selection_updater_update_now (e->selection_updater);
	if (html_engine_is_selection_active (e)) {
		HTMLObject *object;
		guint len, position_before;
		gint level;

		if (!html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))
		    && !html_clueflow_is_empty (HTML_CLUEFLOW (e->mark->object->parent))) {
			check_table_0 (e);
			check_table_1 (e);
		}
		if (e->cursor->position == e->mark->position) {
			html_engine_disable_selection (e);
			return 0;
		}
		position_before = MAX (e->cursor->position, e->mark->position);
		level = delete_object_do (e, &object, &len);
		if (ret_object && ret_len) {
			*ret_object = html_object_op_copy (object, e, NULL, NULL, ret_len);
			*ret_len    = len;
		}
		if (add_undo) {
			delete_setup_undo (e, object, len, position_before, level, dir);
		} else
			html_object_destroy (object);
		gtk_html_editor_event (e->widget, GTK_HTML_EDITOR_EVENT_DELETE, NULL);

		return level;
	}

	return 0;
}

void
html_engine_delete (HTMLEngine *e)
{
	delete_object (e, NULL, NULL, HTML_UNDO_UNDO, TRUE);
}

gint
html_engine_cut (HTMLEngine *e)
{
	gint rv;

	html_engine_clipboard_clear (e);
	rv = delete_object (e, &e->clipboard, &e->clipboard_len, HTML_UNDO_UNDO, TRUE);

#ifdef OP_DEBUG
	printf ("cut  len: %d\n", e->clipboard_len);
	gtk_html_debug_dump_tree_simple (e->clipboard, 0);
#endif

	return rv;
}

/*
 * PASTE/INSERT
 */

static void
set_cursor_at_end_of_object (HTMLEngine *e, HTMLObject *o, guint len)
{
	guint save_position;
	gboolean need_spell_check;

	save_position       = e->cursor->position;
	e->cursor->object   = html_object_get_tail_leaf (o);
	need_spell_check = e->need_spell_check;
	e->need_spell_check = FALSE;
	while (html_cursor_forward (e->cursor, e))
		;
	e->need_spell_check = need_spell_check;
	e->cursor->position = save_position + len;
	e->cursor->offset   = html_object_get_length (e->cursor->object);
}

static inline void
isolate_tables (HTMLEngine *e, HTMLUndoDirection dir, guint position_before, guint position_after,
		  gboolean *delete_paragraph_before, gboolean *delete_paragraph_after)
{
	HTMLObject *next;

	*delete_paragraph_after  = FALSE;
	*delete_paragraph_before = FALSE;

	html_cursor_jump_to_position_no_spell (e->cursor, e, position_after);
	next = html_object_next_not_slave (e->cursor->object);
	if (next && e->cursor->offset == html_object_get_length (e->cursor->object)
	    && (HTML_IS_TABLE (e->cursor->object) || HTML_IS_TABLE (next))) {
		insert_empty_paragraph (e, dir, FALSE);
		*delete_paragraph_after = TRUE;
	}

	html_cursor_jump_to_position_no_spell (e->cursor, e, position_before);
	next = html_object_next_not_slave (e->cursor->object);
	if (next && e->cursor->offset == html_object_get_length (e->cursor->object)
	    && (HTML_IS_TABLE (e->cursor->object) || HTML_IS_TABLE (next))) {
		insert_empty_paragraph (e, dir, FALSE);
		*delete_paragraph_before = TRUE;
	}
}

static inline void
insert_object_do (HTMLEngine *e, HTMLObject *obj, guint *len, gint level, guint position_after,
		  gboolean check, HTMLUndoDirection dir)
{
	HTMLCursor *orig;
	GList *left = NULL, *right = NULL;
	GList *first = NULL, *last = NULL;
	guint position_before;

	html_engine_freeze (e);
	position_before = e->cursor->position;
	html_object_change_set_down (obj, HTML_CHANGE_ALL);
	split_and_add_empty_texts (e, level, &left, &right);
	orig = html_cursor_dup (e->cursor);
	orig->position = position_before;
	first = html_object_heads_list (obj);
	last  = html_object_tails_list (obj);
	set_cursor_at_end_of_object (e, obj, *len);

	if ((left && left->data) || (right && (right->data))) {
		HTMLObject *parent, *where;
		if (left && left->data) {
			where  = HTML_OBJECT (left->data);
			parent = where->parent;
		} else {
			where  = NULL;
			parent = HTML_OBJECT (right->data)->parent;
		}
		if (parent && html_object_is_clue (parent))
			html_clue_append_after (HTML_CLUE (parent), obj, where);
	}

#ifdef OP_DEBUG
	printf ("position before merge %d\n", e->cursor->position);
#endif
	remove_empty_and_merge (e, TRUE, last, right, orig);
	remove_empty_and_merge (e, TRUE, left, first, orig);
#ifdef OP_DEBUG
	printf ("position after merge %d\n", e->cursor->position);
#endif

	html_cursor_destroy (e->cursor);
	e->cursor = html_cursor_dup (orig);
	html_cursor_jump_to_position_no_spell (e->cursor, e, position_after);

	if (check)
		html_engine_spell_check_range (e, orig, e->cursor);
	html_cursor_destroy (orig);
	html_engine_thaw (e);
}

struct _InsertUndo {
	HTMLUndoData data;

	guint len;
	gboolean delete_paragraph_before;
	gboolean delete_paragraph_after;
};
typedef struct _InsertUndo InsertUndo;

static void
insert_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir, guint position_after)
{
	InsertUndo *undo;

	undo = (InsertUndo *) data;

	html_engine_set_mark (e);
	html_cursor_jump_to_position (e->cursor, e, position_after);
	delete_object (e, NULL, NULL, html_undo_direction_reverse (dir), TRUE);

	if (undo->delete_paragraph_after || undo->delete_paragraph_before) {
		html_cursor_jump_to_position (e->cursor, e, position_after);
		if (undo->delete_paragraph_before) {
			html_cursor_backward (e->cursor, e);
		}
		html_engine_set_mark (e);
		if (undo->delete_paragraph_before) {
			html_cursor_forward (e->cursor, e);
		}
		if (undo->delete_paragraph_after) {
			html_cursor_forward (e->cursor, e);
		}
		delete_object (e, NULL, NULL, HTML_UNDO_UNDO, FALSE);
	}
}

static void
insert_setup_undo (HTMLEngine *e, guint len, guint position_before, HTMLUndoDirection dir,
		   gboolean delete_paragraph_before, gboolean delete_paragraph_after)
{
	InsertUndo *undo;

	undo = g_new (InsertUndo, 1);

	html_undo_data_init (HTML_UNDO_DATA (undo));
	undo->len = len;
	undo->delete_paragraph_before = delete_paragraph_before;
	undo->delete_paragraph_after  = delete_paragraph_after;

	/* printf ("insert undo len %d\n", len); */

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Insert", insert_undo_action,
						    HTML_UNDO_DATA (undo),
						    html_cursor_get_position (e->cursor),
						    position_before),
			      dir);
}

static void
insert_object (HTMLEngine *e, HTMLObject *obj, guint len, guint position_after, gint level,
	       HTMLUndoDirection dir, gboolean check)
{
	gboolean delete_paragraph_before = FALSE;
	gboolean delete_paragraph_after = FALSE;
	guint position_before;

	position_before = e->cursor->position;
	insert_object_do (e, obj, &len, level, position_after, check, dir);
	isolate_tables (e, dir, position_before, position_after, &delete_paragraph_before, &delete_paragraph_after);
	html_cursor_jump_to_position_no_spell (e->cursor, e, position_after + (delete_paragraph_before ? 1 : 0));
	insert_setup_undo (e, len, position_before + (delete_paragraph_before ? 1 : 0),
			   dir, delete_paragraph_before, delete_paragraph_after);
}

void
html_engine_insert_object (HTMLEngine *e, HTMLObject *o, guint len, gint level)
{
	insert_object (e, o, len, e->cursor->position + len, level, HTML_UNDO_UNDO, TRUE);
}

void
html_engine_paste_object (HTMLEngine *e, HTMLObject *o, guint len)
{
	html_undo_level_begin (e->undo, "Paste", "Paste");
	html_engine_delete (e);
	html_engine_insert_object (e, o, len, html_object_get_insert_level (o));
	html_undo_level_end (e->undo);
}

void
html_engine_paste (HTMLEngine *e)
{
	if (e->clipboard) {
		HTMLObject *copy;
		guint len = 0;

		copy = html_object_op_copy (e->clipboard, e, NULL, NULL, &len);
		html_engine_paste_object (e, copy, e->clipboard_len);
	}
}

static void
check_magic_link (HTMLEngine *e, const gchar *text, guint len)
{
	if (HTML_IS_TEXT (e->cursor->object)
	    && GTK_HTML_PROPERTY (e->widget, magic_links) && len == 1
	    && (*text == ' ' || text [0] == '\n' || text [0] == '>' || text [0] == ')'))
		html_text_magic_link (HTML_TEXT (e->cursor->object), e, html_object_get_length (e->cursor->object));
}

static void
insert_empty_paragraph (HTMLEngine *e, HTMLUndoDirection dir, gboolean add_undo)
{
	GList *left=NULL, *right=NULL;
	HTMLCursor *orig;
	guint position_before;

	html_engine_freeze (e);
	position_before = e->cursor->position;
	orig = html_cursor_dup (e->cursor);
	split_and_add_empty_texts (e, 2, &left, &right);
	remove_empty_and_merge (e, FALSE, left, right, orig);

	/* replace empty link in empty flow by text with the same style */
	if (HTML_IS_LINK_TEXT (e->cursor->object) && html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
		HTMLObject *flow = e->cursor->object->parent;
		HTMLObject *new_text;

		new_text = html_link_text_to_text (HTML_LINK_TEXT (e->cursor->object), e);
		html_clue_remove (HTML_CLUE (flow), e->cursor->object);
		html_object_destroy (e->cursor->object);
		if (orig->object == e->cursor->object) {
			orig->object = NULL;
		}
		e->cursor->object = new_text;
		if (!orig->object) {
			orig->object = e->cursor->object;
		}
		html_clue_append (HTML_CLUE (flow), e->cursor->object);
	}

	html_cursor_forward (e->cursor, e);

	/* replace empty text in new empty flow by text with current style */
	if (html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
		HTMLObject *flow = e->cursor->object->parent;

		html_clue_remove (HTML_CLUE (flow), e->cursor->object);
		html_object_destroy (e->cursor->object);
		e->cursor->object = html_engine_new_text_empty (e);
		html_clue_append (HTML_CLUE (flow), e->cursor->object);
	}

	html_undo_level_begin (e->undo, "Insert paragraph", "Delete paragraph");
	if (add_undo) {
		insert_setup_undo (e, 1, position_before, dir, FALSE, FALSE);
	}
	g_list_free (left);
	g_list_free (right);
	html_engine_spell_check_range (e, orig, e->cursor);
	html_cursor_destroy (orig);

	html_cursor_backward (e->cursor, e);
	check_magic_link (e, "\n", 1);
	html_cursor_forward (e->cursor, e);
	
	gtk_html_editor_event_command (e->widget, GTK_HTML_COMMAND_INSERT_PARAGRAPH, FALSE);
	html_undo_level_end (e->undo);

	html_engine_thaw (e);
}

void
html_engine_insert_empty_paragraph (HTMLEngine *e)
{
	insert_empty_paragraph (e, HTML_UNDO_UNDO, TRUE);
}

void
html_engine_insert_text (HTMLEngine *e, const gchar *text, guint len)
{
	gchar *nl;
	gint alen;

	if (len == -1)
		len = g_utf8_strlen (text, -1);
	if (!len)
		return;

	html_undo_level_begin (e->undo, "Insert text", "Delete text");
	/* FIXME add insert text event */
	gtk_html_editor_event_command (e->widget, GTK_HTML_COMMAND_INSERT_PARAGRAPH, TRUE);

	do {
		nl   = g_utf8_strchr (text, '\n');
		alen = nl ? g_utf8_pointer_to_offset (text, nl) : len;
		if (alen) {
			HTMLObject *o;
			gboolean check = FALSE;

			check_magic_link (e, text, alen);

			/* stop inserting links after space */
			if (*text == ' ')
				html_engine_set_insertion_link (e, NULL, NULL);

			o = html_engine_new_text (e, text, alen);
			html_text_convert_nbsp (HTML_TEXT (o), TRUE);

			if (alen == 1 && html_is_in_word (html_text_get_char (HTML_TEXT (o), 0))
			    && !html_is_in_word (html_cursor_get_current_char (e->cursor))) {
				/* printf ("need_spell_check\n"); */
				e->need_spell_check = TRUE;
			} else {
				check = TRUE;
			}
			insert_object (e, o, html_object_get_length (o), e->cursor->position + html_object_get_length (o),
				       1, HTML_UNDO_UNDO, check);
		}
		if (nl) {
			html_engine_insert_empty_paragraph (e);
			len -= g_utf8_pointer_to_offset (text, nl) + 1;
			text = nl + 1;
		}
	} while (nl);
	html_undo_level_end (e->undo);
}

void
html_engine_paste_text (HTMLEngine *e, const gchar *text, guint len)
{
	gchar *undo_name = g_strdup_printf ("Paste text: '%s'", text);
	gchar *redo_name = g_strdup_printf ("Unpaste text: '%s'", text);

	html_undo_level_begin (e->undo, undo_name, redo_name);
	g_free (undo_name);
	g_free (redo_name);
	html_engine_delete (e);
	html_engine_insert_text (e, text, len);
	html_undo_level_end (e->undo);
}

void
html_engine_delete_container (HTMLEngine *e)
{
	g_assert (HTML_IS_ENGINE (e));
	g_assert (e->cursor->object);
	g_assert (html_object_is_container (e->cursor->object));

	html_engine_set_mark (e);
	html_engine_update_selection_if_necessary (e);
	html_engine_freeze (e);
	if (e->cursor->offset)
		html_cursor_beginning_of_line (e->cursor, e);
	else
		html_cursor_end_of_line (e->cursor, e);
	html_engine_delete (e);
	html_engine_thaw (e);
}

void
html_engine_delete_n (HTMLEngine *e, guint len, gboolean forward)
{
	if (html_engine_is_selection_active (e))
		html_engine_delete (e);
	else {
		html_engine_block_selection (e);
		html_engine_set_mark (e);
		html_engine_update_selection_if_necessary (e);
		html_engine_freeze (e);
		while (len != 0) {
			if (forward)
				html_cursor_forward (e->cursor, e);
			else
				html_cursor_backward (e->cursor, e);
			len --;
		}
		html_engine_delete (e);
		html_engine_unblock_selection (e);
		html_engine_thaw (e);	
	}
}

void
html_engine_cut_line (HTMLEngine *e)
{
	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	html_undo_level_begin (e->undo, "Cut Line", "Undo Cut Line");
	html_engine_set_mark (e);
	html_engine_end_of_line (e);

	if (e->cursor->position == e->mark->position)
		html_cursor_forward (e->cursor, e);

	html_engine_cut (e);
	html_undo_level_end (e->undo);
}

typedef struct {
	HTMLColor   *color;
	const gchar *url;
	const gchar *target;
} HTMLEngineLinkInsertData;

static void
change_link (HTMLObject *o, HTMLEngine *e, gpointer data)
{
	HTMLObject *changed;
	HTMLEngineLinkInsertData *d = (HTMLEngineLinkInsertData *) data;

	changed = d->url ? html_object_set_link (o, d->color, d->url, d->target) : html_object_remove_link (o, d->color);
	if (changed) {
		if (o->parent) {
			HTMLObject *prev;

			prev = o->prev;
			g_assert (HTML_OBJECT_TYPE (o->parent) == HTML_TYPE_CLUEFLOW);

			html_clue_append_after (HTML_CLUE (o->parent), changed, o);
			html_clue_remove (HTML_CLUE (o->parent), o);
			html_object_destroy (o);
			if (changed->prev)
				html_object_merge (changed->prev, changed, e, NULL, NULL, NULL);
		} else {
			html_object_destroy (e->clipboard);
			e->clipboard     = changed;
			e->clipboard_len = html_object_get_length (changed);
		}
	}
}

void
html_engine_set_insertion_link (HTMLEngine *e, const gchar *url, const gchar *target)
{
	html_engine_set_url    (e, url);
	html_engine_set_target (e, target);
	if (!url && e->insertion_color == html_colorset_get_color (e->settings->color_set, HTMLLinkColor))
		html_engine_set_color (e, html_colorset_get_color (e->settings->color_set, HTMLTextColor));
	else if (url)
		html_engine_set_color (e, html_colorset_get_color (e->settings->color_set, HTMLLinkColor));
}

void
html_engine_insert_link (HTMLEngine *e, const gchar *url, const gchar *target)
{
	if (html_engine_is_selection_active (e)) {
		HTMLEngineLinkInsertData data;

		data.url    = url;
		data.target = target;
		data.color  = url
			? html_colorset_get_color (e->settings->color_set, HTMLLinkColor)
			: html_colorset_get_color (e->settings->color_set, HTMLTextColor);
		html_engine_cut_and_paste (e,
					   url ? "Insert link" : "Remove link",
					   url ? "Remove link" : "Insert link",
					   change_link, &data);
	} else
		html_engine_set_insertion_link (e, url, target);
}

static void
prepare_empty_flow (HTMLEngine *e, HTMLUndoDirection dir)
{
	if (!html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
		insert_empty_paragraph (e, dir, TRUE);
		if (e->cursor->object->parent->prev
		    && html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent->prev))) {
			html_cursor_backward (e->cursor, e);
		} else if (!html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
			insert_empty_paragraph (e, dir, TRUE);
			html_cursor_backward (e->cursor, e);
		}
	}
}

static void
append_object (HTMLEngine *e, HTMLObject *o, guint len, HTMLUndoDirection dir)
{
	HTMLObject *c, *cn;
	HTMLClue *clue;
	guint position_before;

	html_engine_freeze (e);
	prepare_empty_flow (e, dir);
	position_before = e->cursor->position;

	g_return_if_fail (html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent)));

	clue = HTML_CLUE (e->cursor->object->parent);
	for (c = clue->head; c; c = cn) {
		cn = c->next;
		html_object_destroy (c);
	}
	clue->head = clue->tail = o;
	e->cursor->object = o;
	e->cursor->offset = 0;
	o->parent = HTML_OBJECT (clue);

	html_cursor_forward_n (e->cursor, e, len);
	html_object_change_set (o, HTML_CHANGE_ALL_CALC);
	html_engine_thaw (e);

	insert_setup_undo (e, len, position_before, dir, FALSE, FALSE);

	return;
}

void
html_engine_append_object (HTMLEngine *e, HTMLObject *o, guint len)
{
	html_undo_level_begin (e->undo, "Append object", "Remove appended object");
	append_object (e, o, len, HTML_UNDO_UNDO);
	html_undo_level_end (e->undo);
}

static void
append_flow (HTMLEngine *e, HTMLObject *o, guint len, HTMLUndoDirection dir)
{
	HTMLObject *where;
	guint position, position_before;

	html_engine_freeze (e);

	position_before = e->cursor->position;
	prepare_empty_flow (e, dir);

	g_return_if_fail (html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent)));

	where = e->cursor->object->parent;

	e->cursor->object = html_object_get_head_leaf (o);
	e->cursor->offset = 0;
	position = e->cursor->position;
	while (html_cursor_backward (e->cursor, e))
		;
	e->cursor->position = position;
	html_clue_append_after (HTML_CLUE (where->parent), o, where);
	html_object_remove_child (where->parent, where);
	html_object_destroy (where);

	html_cursor_forward_n (e->cursor, e, len);
	html_object_change_set (o, HTML_CHANGE_ALL_CALC);
	html_engine_thaw (e);

	insert_setup_undo (e, len, position_before, dir, FALSE, FALSE);

	return;
}

void
html_engine_append_flow (HTMLEngine *e, HTMLObject *o, guint len)
{
	html_undo_level_begin (e->undo, "Append flow", "Remove appended flow");
	append_flow (e, o, len, HTML_UNDO_UNDO);
	html_undo_level_end (e->undo);
}

void
html_engine_cut_and_paste_begin (HTMLEngine *e, const gchar *undo_op_name, const gchar *redo_op_name)
{
	guint position;
	gint level;

	html_engine_hide_cursor (e);
	html_engine_selection_push (e);
	html_engine_clipboard_push (e);
	html_undo_level_begin (e->undo, undo_op_name, redo_op_name);
	position = e->mark ? MAX (e->cursor->position, e->mark->position) : e->cursor->position;
	level = html_engine_cut (e);

	e->cut_and_paste_stack = g_list_prepend (e->cut_and_paste_stack, GINT_TO_POINTER (level));
	e->cut_and_paste_stack = g_list_prepend (e->cut_and_paste_stack, GUINT_TO_POINTER (position));
}

void
html_engine_cut_and_paste_end (HTMLEngine *e)
{
	guint position;
	gint level;

	position = GPOINTER_TO_UINT (e->cut_and_paste_stack->data);
	e->cut_and_paste_stack = g_list_remove (e->cut_and_paste_stack, e->cut_and_paste_stack->data);
	level    = GPOINTER_TO_INT (e->cut_and_paste_stack->data);
	e->cut_and_paste_stack = g_list_remove (e->cut_and_paste_stack, e->cut_and_paste_stack->data);

	if (e->clipboard) {
		insert_object (e, e->clipboard, e->clipboard_len, position, level, HTML_UNDO_UNDO, TRUE);
		e->clipboard = NULL;
	}
	html_undo_level_end (e->undo);
	html_engine_clipboard_pop (e);
	html_engine_selection_pop (e);
	html_engine_show_cursor (e);
}

void
html_engine_cut_and_paste (HTMLEngine *e, const gchar *undo_op_name, const gchar *redo_op_name,
			   HTMLObjectForallFunc iterator, gpointer data)
{
	html_engine_edit_selection_updater_update_now (e->selection_updater);
	html_engine_cut_and_paste_begin (e, undo_op_name, redo_op_name);
	if (e->clipboard)
		html_object_forall (e->clipboard, e, iterator, data);
	html_engine_cut_and_paste_end (e);
}
