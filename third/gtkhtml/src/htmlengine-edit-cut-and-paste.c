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

static void        delete_object (HTMLEngine *e, HTMLObject **ret_object, guint *ret_len, HTMLUndoDirection dir);
static void        insert_object (HTMLEngine *e, HTMLObject *obj, guint len, HTMLUndoDirection dir, gboolean check);
static void        append_object (HTMLEngine *e, HTMLObject *o, guint len, HTMLUndoDirection dir);
static void        insert_empty_paragraph (HTMLEngine *e, HTMLUndoDirection dir);

/* helper functions -- need refactor */

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
html_cursor_get_right (HTMLCursor *cursor, HTMLObject **obj, gint *off)
{
	if (cursor->offset >= html_object_get_length (cursor->object)) {
		*obj = html_object_next_not_slave (cursor->object);
		if (*obj) {
			*off = 0;
		}
		return;
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

static void
prepare_delete_bounds (HTMLEngine *e, GList **from_list, GList **to_list,
		       GList **bound_left, GList **bound_right)
{
	HTMLPoint b_left, b_right, begin, end;
	HTMLObject *common_parent;

	g_assert (e->selection);

	html_point_get_right (&e->selection->from, &begin);
	html_point_get_left  (&e->selection->to,   &end);

	common_parent = get_common_parent (begin.object, end.object);

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
}

static void
remove_empty_and_merge (HTMLEngine *e, gboolean merge, GList *left, GList *right, HTMLCursor *c)
{
	HTMLObject *lo, *ro, *prev;
	gint len;

	/* printf ("before merge\n");
	gtk_html_debug_dump_tree_simple (e->clue, 0);
	if (left && left->data) {
		printf ("left\n");
		gtk_html_debug_dump_tree_simple (left->data, 0);
	}

	if (right && right->data) {
		printf ("right\n");
		gtk_html_debug_dump_tree_simple (right->data, 0);
		} */

	while (left && left->data && right && right->data) {

		lo  = HTML_OBJECT (left->data);
		ro  = HTML_OBJECT (right->data);
		len = html_object_get_length (lo);

		left  = left->next;
		right = right->next;

		if (html_object_is_text (lo) && !*HTML_TEXT (lo)->text && (html_object_prev_not_slave (lo) || merge)) {
			HTMLObject *nlo = html_object_prev_not_slave (lo);

			if (e->cursor->object == lo)
				e->cursor->object = ro;
			if (c && c->object == lo)
				c->object = ro;

			html_object_remove_child (lo->parent, lo);
			html_object_destroy (lo);
			lo = nlo;
		} else if (html_object_is_text (ro) && !*HTML_TEXT (ro)->text && (html_object_next_not_slave (ro) || merge)) {
			HTMLObject *nro = html_object_next_not_slave (ro);

			if (e->cursor->object == ro)
				e->cursor->object = lo;

			html_object_remove_child (ro->parent, ro);
			html_object_destroy (ro);
			ro = nro;
		}

		if (merge && lo && ro) {
			if (!html_object_merge (lo, ro, e, left, right))
				break;
			if (ro == e->cursor->object) {
				e->cursor->object  = lo;
				e->cursor->offset += len;
			}
		}
	}

	prev = html_object_prev_not_slave (e->cursor->object);
	if (prev && e->cursor->offset == 0) {
		e->cursor->object = prev;
		e->cursor->offset = html_object_get_length (e->cursor->object);
	}
	/* printf ("-- after\n");
	gtk_html_debug_dump_tree_simple (e->clue, 0);
	printf ("-- END merge\n"); */
}

static gboolean
look_for_non_appendable (GList *list)
{
	while (list) {
		HTMLObject *o = HTML_OBJECT (list->data);

		if (o->parent && HTML_OBJECT_TYPE (o->parent) == HTML_TYPE_TABLE)
			return TRUE;
		list = list->next;
	}

	return FALSE;
}

static HTMLObject *
split_between_objects (HTMLObject *object, HTMLObject *other,
		       GList **object_parents, GList **other_parents, gint *object_level)
{
	HTMLObject *common_parent;
	gint d_object, d_other;

	common_parent = get_common_parent (object, other);

	if (common_parent) {
		d_object = get_parent_depth (object, common_parent);
		d_other  = get_parent_depth (other,  common_parent);

		if (d_object <= *object_level) {
			object_get_parent_list (object, d_object - 1, object_parents);
			object_get_parent_list (other, d_other - 1, other_parents);

#ifdef GTKHTML_DEBUG_TABLE
			if (*object_parents) {
				printf ("split object\n");
				gtk_html_debug_dump_tree_simple ((*object_parents)->data, 0);
			}

			if (*other_parents) {
				printf ("split other\n");
				gtk_html_debug_dump_tree_simple ((*other_parents)->data, 0);
			}
#endif
			if (!look_for_non_appendable (*object_parents) && !look_for_non_appendable (*object_parents)) {
				g_list_free (*object_parents);
				g_list_free (*other_parents);

				printf ("drop lists, do simple split\n");
				*object_parents = NULL;
				*other_parents = NULL;

				return NULL;
			}

			*object_level -= d_object - 1;

			return common_parent;
		}
	}

	return NULL;
}

static void
split_and_add_empty_texts (HTMLEngine *e, gint level, GList **left, GList **right)
{
	HTMLObject *object = NULL;

	if ((e->cursor->offset == 0 || (html_object_get_length (e->cursor->object) == e->cursor->offset))) {
		HTMLObject *leaf;
		gboolean prev;

		prev = html_object_get_length (e->cursor->object) != e->cursor->offset;

		leaf = prev
		? html_object_prev_leaf_not_type (e->cursor->object, HTML_TYPE_TEXTSLAVE)
		: html_object_next_leaf_not_type (e->cursor->object, HTML_TYPE_TEXTSLAVE);
		if (leaf) {
			if (prev)
				object = split_between_objects (e->cursor->object, leaf, right, left, &level);
			else
				object = split_between_objects (e->cursor->object, leaf, left, right, &level);
		}
	}

	if (!object)
		object = e->cursor->object;

	html_object_split (object, e, *right ? HTML_OBJECT ((*right)->data) : NULL, e->cursor->offset, level, left, right);
}

/* end of helper */

void
html_engine_copy_object (HTMLEngine *e, HTMLObject **o, guint *len)
{
	GList *from, *to;

	if (html_engine_is_selection_active (e)) {
		html_engine_freeze (e);
		prepare_delete_bounds (e, &from, &to, NULL, NULL);
		*len = 0;
		*o    = html_object_op_copy (HTML_OBJECT (from->data), e,
				from->next, to->next, len);
#ifdef GTKHTML_DEBUG_TABLE
		printf ("copy len: %d (parent %p)\n", *len, (*o)->parent);
		gtk_html_debug_dump_tree_simple (*o, 0);
#endif
		html_engine_thaw (e);
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
delete_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir)
{
	DeleteUndo *undo;
	HTMLObject *buffer;
	guint       len = 0;

	undo         = (DeleteUndo *) data;
	buffer       = html_object_op_copy (undo->buffer, e, NULL, NULL, &len);
	insert_object (e, buffer, undo->buffer_len, html_undo_direction_reverse (dir), TRUE);
}

static void
delete_setup_undo (HTMLEngine *e, HTMLObject *buffer, guint len, HTMLUndoDirection dir)
{
	DeleteUndo *undo;

	undo = g_new (DeleteUndo, 1);

	html_undo_data_init (HTML_UNDO_DATA (undo));
	undo->data.destroy = delete_undo_destroy;
	undo->buffer       = buffer;
	undo->buffer_len   = len;

	/* printf ("delete undo len %d\n", len); */

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Delete", delete_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)),
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
		} else {
			HTMLObject *obj;
			gint off;

			html_cursor_get_right (e->mark, &obj, &off);
			if (obj) {
				e->cursor->object = obj;
				e->cursor->offset = 0;
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

static void
delete_object_do (HTMLEngine *e, HTMLObject **object, guint *len)
{
	GList *from, *to, *left, *right;

	html_engine_freeze (e);
	prepare_delete_bounds (e, &from, &to, &left, &right);
	place_cursor_before_mark (e);
	move_cursor_before_delete (e);
	html_engine_disable_selection (e);
	*len     = 0;
	*object  = html_object_op_cut  (HTML_OBJECT (from->data), e, from->next, to->next, left, right, len);
	remove_empty_and_merge (e, TRUE, left ? left->next : NULL, right ? right->next : NULL, NULL);
	html_engine_spell_check_range (e, e->cursor, e->cursor);
	html_engine_thaw (e);
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

static void
delete_object (HTMLEngine *e, HTMLObject **ret_object, guint *ret_len, HTMLUndoDirection dir)
{
	html_engine_edit_selection_updater_update_now (e->selection_updater);
	if (html_engine_is_selection_active (e)) {
		HTMLObject *object;
		guint len;

		if (!html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))
		    && !html_clueflow_is_empty (HTML_CLUEFLOW (e->mark->object->parent))) {
			check_table_0 (e);
			check_table_1 (e);
		}
		if (e->cursor->position == e->mark->position) {
			html_engine_disable_selection (e);
			return;
		}
		delete_object_do (e, &object, &len);
		if (ret_object && ret_len) {
			*ret_object = html_object_op_copy (object, e, NULL, NULL, ret_len);
			*ret_len    = len;
		}
		delete_setup_undo (e, object, len, dir);
		gtk_html_editor_event (e->widget, GTK_HTML_EDITOR_EVENT_DELETE, NULL);
	}
}

void
html_engine_delete (HTMLEngine *e)
{
	delete_object (e, NULL, NULL, HTML_UNDO_UNDO);
}

void
html_engine_cut (HTMLEngine *e)
{
	html_engine_clipboard_clear (e);
	delete_object (e, &e->clipboard, &e->clipboard_len, HTML_UNDO_UNDO);

#ifdef GTKHTML_DEBUG_TABLE
	printf ("cut  len: %d\n", e->clipboard_len);
	gtk_html_debug_dump_tree_simple (e->clipboard, 0);
#endif
}

/*
 * PASTE/INSERT
 */

static void
set_cursor_at_end_of_object (HTMLEngine *e, HTMLObject *o, guint len)
{
	guint save_position;

	save_position       = e->cursor->position;
	e->cursor->object   = html_object_get_tail_leaf (o);
	while (html_cursor_forward (e->cursor, e))
		;
	e->cursor->position = save_position + len;
	e->cursor->offset   = html_object_get_length (e->cursor->object);
}

static inline void
insert_object_do (HTMLEngine *e, HTMLObject *obj, guint len, gboolean check, HTMLUndoDirection dir)
{
	HTMLObject *cur;
	HTMLCursor *orig;
	GList *left = NULL, *right = NULL;
	GList *first = NULL, *last = NULL;
	gint level;

	html_engine_freeze (e);

	if (HTML_IS_TABLE (e->cursor->object)) {
		if (e->cursor->offset) {
			HTMLObject *head = html_object_get_head_leaf (obj);

			if (!head->parent || (HTML_IS_CLUEFLOW (head->parent)
					      && !html_clueflow_is_empty (HTML_CLUEFLOW (head->parent))))
				insert_empty_paragraph (e, dir);
		} else {
			HTMLObject *tail = html_object_get_tail_leaf (obj);

			if (!tail->parent || (HTML_IS_CLUEFLOW (tail->parent)
					      && !html_clueflow_is_empty (HTML_CLUEFLOW (tail->parent)))) {
				insert_empty_paragraph (e, dir);
				html_cursor_backward (e->cursor, e);
			}
		}
	}

	level = 0;
	cur   = html_object_get_head_leaf (obj);
	while (cur) {
		level++;
		cur = cur->parent;
	}
	orig = html_cursor_dup (e->cursor);

	html_object_change_set_down (obj, HTML_CHANGE_ALL);
	split_and_add_empty_texts (e, MIN (3, level), &left, &right);
	first = html_object_heads_list (obj);
	last  = html_object_tails_list (obj);
	set_cursor_at_end_of_object (e, obj, len);

	if ((left && left->data) || (right && (right->data))) {
		HTMLObject *parent, *where;
		if (left && left->data) {
			where  = HTML_OBJECT (left->data);
			parent = where->parent;
		} else {
			where  = NULL;
			parent = HTML_OBJECT (right->data)->parent;
		}
		if (parent)
			html_clue_append_after (HTML_CLUE (parent), obj, where);
	}

	remove_empty_and_merge (e, TRUE, last, right, orig);
	remove_empty_and_merge (e, TRUE, left, first, orig);

	if (check)
		html_engine_spell_check_range (e, orig, e->cursor);
	html_cursor_destroy (orig);
	html_engine_thaw (e);
}

struct _InsertUndo {
	HTMLUndoData data;

	guint len;
};
typedef struct _InsertUndo InsertUndo;

static void
insert_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir)
{
	InsertUndo *undo;

	undo = (InsertUndo *) data;

	html_engine_set_mark (e);
	html_engine_move_cursor (e, HTML_ENGINE_CURSOR_LEFT, undo->len);
	delete_object (e, NULL, NULL, html_undo_direction_reverse (dir));
}

static void
insert_setup_undo (HTMLEngine *e, guint len, HTMLUndoDirection dir)
{
	InsertUndo *undo;

	undo = g_new (InsertUndo, 1);

	html_undo_data_init (HTML_UNDO_DATA (undo));
	undo->len = len;

	/* printf ("insert undo len %d\n", len); */

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Insert", insert_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor)),
			      dir);
}

static void
insert_object (HTMLEngine *e, HTMLObject *obj, guint len, HTMLUndoDirection dir, gboolean check)
{
	/* FIXME for tables */
	if (HTML_IS_TABLE (obj))
		append_object (e, obj, len, dir);
	else if (len > 0) {
		insert_object_do (e, obj, len, check, dir);
		insert_setup_undo (e, len, dir);
	}
}

void
html_engine_insert_object (HTMLEngine *e, HTMLObject *o, guint len)
{
	insert_object (e, o, len, HTML_UNDO_UNDO, TRUE);
}

void
html_engine_paste_object (HTMLEngine *e, HTMLObject *o, guint len)
{
	html_undo_level_begin (e->undo, "Paste", "Paste");
	html_engine_delete (e);
	html_engine_insert_object (e, o, len);
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
	if (HTML_OBJECT_TYPE (e->cursor->object) == HTML_TYPE_TEXT
	    && GTK_HTML_PROPERTY (e->widget, magic_links) && len == 1
	    && (*text == ' ' || text [0] == '\n' || text [0] == '>' || text [0] == ')'))
		html_text_magic_link (HTML_TEXT (e->cursor->object), e, html_object_get_length (e->cursor->object));
}

static void
insert_empty_paragraph (HTMLEngine *e, HTMLUndoDirection dir)
{
	GList *left=NULL, *right=NULL;
	HTMLCursor *orig;

	html_engine_freeze (e);
	orig = html_cursor_dup (e->cursor);
	split_and_add_empty_texts (e, 2, &left, &right);
	remove_empty_and_merge (e, FALSE, left, right, orig);
	html_cursor_forward (e->cursor, e);

	/* replace empty text in new empty flow by text with current style */
	if (html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
		HTMLObject *flow = e->cursor->object->parent;

		html_clue_remove (HTML_CLUE (flow), e->cursor->object);
		html_object_destroy (e->cursor->object);
		e->cursor->object = html_engine_new_text_empty (e);
		html_clue_append (HTML_CLUE (flow), e->cursor->object);
	}

	insert_setup_undo (e, 1, dir);
	g_list_free (left);
	g_list_free (right);
	html_engine_spell_check_range (e, orig, e->cursor);
	html_cursor_destroy (orig);

	html_cursor_backward (e->cursor, e);
	check_magic_link (e, "\n", 1);
	html_cursor_forward (e->cursor, e);

	html_engine_thaw (e);

	gtk_html_editor_event_command (e->widget, GTK_HTML_COMMAND_INSERT_PARAGRAPH);

	/* break links in new paragraph */
	html_engine_insert_link (e, NULL, NULL);
}

void
html_engine_insert_empty_paragraph (HTMLEngine *e)
{
	insert_empty_paragraph (e, HTML_UNDO_UNDO);
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
			    && !html_is_in_word (html_cursor_get_current_char (e->cursor)))
					e->need_spell_check = TRUE;
			else
				check = TRUE;
			insert_object (e, o, html_object_get_length (o), HTML_UNDO_UNDO, check);
		}
		if (nl) {
			html_engine_insert_empty_paragraph (e);
			len -= g_utf8_pointer_to_offset (text, nl) + 1;
			text = nl + 1;
		}
	} while (nl);
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

	html_engine_freeze (e);
	html_engine_set_mark (e);
	if (e->cursor->offset)
		html_cursor_beginning_of_line (e->cursor, e);
	else
		html_cursor_end_of_line (e->cursor, e);
	html_engine_update_selection_if_necessary (e);
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
		while (len != 0) {
			if (forward)
				html_cursor_forward (e->cursor, e);
			else
				html_cursor_backward (e->cursor, e);
			len --;
		}
		html_engine_delete (e);
		html_engine_unblock_selection (e);
	}
}

void
html_engine_cut_line (HTMLEngine *e)
{
	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	html_engine_set_mark (e);
	html_engine_end_of_line (e);

	if (e->cursor->position == e->mark->position)
		html_cursor_forward (e->cursor, e);

	html_engine_cut (e);
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
				html_object_merge (changed->prev, changed, e, NULL, NULL);
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
append_object (HTMLEngine *e, HTMLObject *o, guint len, HTMLUndoDirection dir)
{
	GList *left = NULL, *right = NULL;
	HTMLObject *where;
	gint back = 0;

	html_engine_freeze (e);
	if (html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
		HTMLObject *c, *cn;
		HTMLClue *clue = HTML_CLUE (e->cursor->object->parent);
		for (c = clue->head; c; c = cn) {
			cn = c->next;
			html_object_destroy (c);
		}
		clue->head = clue->tail = o;
		e->cursor->object = o;
		e->cursor->offset = 0;
		o->parent = HTML_OBJECT (clue);
	} else {
		HTMLObject *flow;

		flow  = html_clueflow_new (HTML_CLUEFLOW_STYLE_NORMAL, 0, HTML_LIST_TYPE_UNORDERED, 1);
		html_clue_append (HTML_CLUE (flow), o);

		html_object_split (e->cursor->object, e, NULL, e->cursor->offset, 2, &left, &right);
		len += 2;
		back = 1;

		where = HTML_OBJECT (left->data);
		html_clue_append_after (HTML_CLUE (where->parent), flow, where);

		if (html_clueflow_is_empty (HTML_CLUEFLOW (where))) {
			html_cursor_forward (e->cursor, e);
			html_clue_remove (HTML_CLUE (where->parent), where);
			html_object_destroy (where);
			len --;
			e->cursor->position --;
		}
		if (html_clueflow_is_empty (HTML_CLUEFLOW (HTML_OBJECT (flow)->next))) {
			HTMLObject *empty;

			empty = HTML_OBJECT (flow)->next;
			html_clue_remove (HTML_CLUE (empty->parent), empty);
			html_object_destroy (empty);
			len --;
			back = 0;
		}
	}

	html_cursor_forward_n (e->cursor, e, len);
	html_object_change_set (o, HTML_CHANGE_ALL_CALC);
	html_engine_thaw (e);

	insert_setup_undo (e, len, dir);
	html_cursor_backward_n (e->cursor, e, back);
}

void
html_engine_append_object (HTMLEngine *e, HTMLObject *o, guint len)
{
	append_object (e, o, len, HTML_UNDO_UNDO);
}
