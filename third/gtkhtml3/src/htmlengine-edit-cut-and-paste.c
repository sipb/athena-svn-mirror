/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000, 2001, 2002 Ximian, Inc.
    Authors:                       Radek Doulik (rodo@ximian.com)

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
#include "htmlcluealigned.h"
#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlcolorset.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-clueflowstyle.h"
#include "htmlengine-edit-cursor.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlimage.h"
#include "htmlinterval.h"
#include "htmlobject.h"
#include "htmlplainpainter.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmlselection.h"
#include "htmlsettings.h"
#include "htmltext.h"
#include "htmlundo.h"
#include "htmlundo-action.h"

static gint        delete_object (HTMLEngine *e, HTMLObject **ret_object, guint *ret_len, HTMLUndoDirection dir,
				  gboolean add_prop);
static void        insert_object_for_undo (HTMLEngine *e, HTMLObject *obj, guint len, guint position_after, gint level,
					   HTMLUndoDirection dir, gboolean check);
static void        append_object (HTMLEngine *e, HTMLObject *o, guint len, HTMLUndoDirection dir);
static void        insert_empty_paragraph (HTMLEngine *e, HTMLUndoDirection dir, gboolean add_undo);
static void        insert_setup_undo (HTMLEngine *e, guint len, guint position_before, HTMLUndoDirection dir,
				      gboolean delete_paragraph_before, gboolean delete_paragraph_after);

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
	ret_level     = html_object_get_parent_level (common_parent);
	/* printf ("common parent level: %d\n", ret_level); */

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
	/* HTMLObject *left_orig = left->data; */
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

		if (HTML_IS_CLUEALIGNED (lo) && !HTML_IS_CLUEALIGNED (ro) && html_object_is_text (HTML_CLUE (lo)->head)) {
			HTMLObject *nlo = lo->prev;

			if (e->cursor->object->parent && e->cursor->object->parent == lo) {
				e->cursor->object = ro;
				e->cursor->offset = 0;
			}
			if (c && c->object->parent && c->object->parent == lo) {
				c->object = ro;
				c->offset = 0;
			}

			html_object_remove_child (lo->parent, lo);
			html_object_destroy (lo);
			lo = nlo;
			if (!nlo)
				break;
		} else if (HTML_IS_CLUEALIGNED (ro) && !HTML_IS_CLUEALIGNED (lo) && html_object_is_text (HTML_CLUE (ro)->head)) {
			HTMLObject *nro = ro->next;

			if (e->cursor->object->parent && e->cursor->object->parent == ro) {
				e->cursor->object = lo;
				e->cursor->offset = html_object_get_length (lo);
			}
			html_object_remove_child (ro->parent, ro);
			html_object_destroy (ro);
			ro = nro;
			if (!nro)
				break;
		}

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
	/* printf ("-- finished\n");
	   gtk_html_debug_dump_tree_simple (left_orig, 0); */
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
	printf ("-- SPLIT finish\n");
	if (*left && (*left)->data) {
		printf ("left\n");
		gtk_html_debug_dump_tree_simple (HTML_OBJECT ((*left)->data), 0);
	}
	if (*right && (*right)->data) {
		printf ("right\n");
		gtk_html_debug_dump_tree_simple (HTML_OBJECT ((*right)->data), 0);
	}
	printf ("-- SPLIT end\n");
#endif
}

/* end of helper */

void
html_engine_copy_object (HTMLEngine *e, HTMLObject **o, guint *len)
{
	GList *from, *to;

	if (e->clue && HTML_CLUE (e->clue)->head && html_engine_is_selection_active (e)) {
		prepare_delete_bounds (e, &from, &to, NULL, NULL);
		*len = 0;
		*o    = html_object_op_copy (HTML_OBJECT (from->data), NULL, e,
					     from->next, to->next, len);
#ifdef OP_DEBUG
		printf ("copy len: %d (parent %p)\n", *len, (*o)->parent);
		gtk_html_debug_dump_tree_simple (*o, 0);
#endif
	} else {
		*len = 0;
		*o = NULL;
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
	buffer       = html_object_op_copy (undo->buffer, NULL, e, NULL, NULL, &len);
	insert_object_for_undo (e, buffer, undo->buffer_len, position_after, undo->level, html_undo_direction_reverse (dir), TRUE);
}

static void
delete_setup_undo (HTMLEngine *e, HTMLObject *buffer, guint len, guint position_after, gint level, HTMLUndoDirection dir)
{
	DeleteUndo *undo;

	undo = g_new (DeleteUndo, 1);

	/* printf ("cursor level: %d undo level: %d\n", html_object_get_parent_level (e->cursor->object), level); */
	html_undo_data_init (HTML_UNDO_DATA (undo));
	undo->data.destroy = delete_undo_destroy;
	undo->buffer       = buffer;
	undo->buffer_len   = len;
	undo->level        = level;

	/* printf ("delete undo len %d\n", len); */

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Delete object", delete_undo_action,
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

static gboolean
haligns_equal (HTMLHAlignType a1, HTMLHAlignType a2)
{
	return a1 == a2
		|| (a1 == HTML_HALIGN_LEFT && a2 == HTML_HALIGN_NONE)
		|| (a1 == HTML_HALIGN_NONE && a2 == HTML_HALIGN_LEFT);

}

static gboolean
levels_equal (HTMLClueFlow *me, HTMLClueFlow *you)
{
	if (!you)
		return FALSE;

	if (me->levels->len != you->levels->len)
		return FALSE;

	if (me->levels->len == 0)
		return TRUE;

	return !memcmp (me->levels->data, you->levels->data, you->levels->len);
}

static void
check_flows (HTMLEngine *e, HTMLUndoDirection dir)
{
	/* I assume here that cursor is before mark */
	HTMLClueFlow *flow1, *flow2;
	gint level1, level2;

	g_return_if_fail (e->cursor);
	g_return_if_fail (e->cursor->object);
	g_return_if_fail (e->cursor->object->parent);
	g_return_if_fail (e->mark);
	g_return_if_fail (e->mark->object);
	g_return_if_fail (e->mark->object->parent);
	g_return_if_fail (e->cursor->position <= e->mark->position);

	if (e->cursor->offset || e->cursor->object->parent == e->mark->object->parent
	    || !HTML_IS_CLUEFLOW (e->cursor->object->parent) || !HTML_IS_CLUEFLOW (e->mark->object->parent)
	    || e->cursor->object != HTML_CLUE (e->cursor->object->parent)->head)
		return;

	level1 = html_object_get_parent_level (e->cursor->object->parent);
	level2 = html_object_get_parent_level (e->mark->object->parent);

	flow1 = HTML_CLUEFLOW (e->cursor->object->parent);
	flow2 = HTML_CLUEFLOW (e->mark->object->parent);

	if (level1 == level2
	    && (flow1->style != flow2->style
		|| (flow1->style == HTML_CLUEFLOW_STYLE_LIST_ITEM && flow1->item_type != flow2->item_type)
		|| !levels_equal (flow1, flow2)
		|| !haligns_equal (HTML_CLUE (flow1)->halign, HTML_CLUE (flow2)->halign))) {
		HTMLCursor *dest, *source;

		dest = html_cursor_dup (e->cursor);
		source = html_cursor_dup (e->mark);

		html_engine_selection_push (e);
		html_engine_disable_selection (e);
		html_cursor_jump_to_position_no_spell (e->cursor, e, dest->position);
		html_engine_set_clueflow_style (e,
						HTML_CLUEFLOW (source->object->parent)->style,
						HTML_CLUEFLOW (source->object->parent)->item_type,
						HTML_CLUE     (source->object->parent)->halign,
						HTML_CLUEFLOW (source->object->parent)->levels->len,
						HTML_CLUEFLOW (source->object->parent)->levels->data,
						HTML_ENGINE_SET_CLUEFLOW_INDENTATION_ALL,
						dir, TRUE);
		html_engine_selection_pop (e);
		html_cursor_destroy (source);
		html_cursor_destroy (dest);
	}
}

static gint
delete_object_do (HTMLEngine *e, HTMLObject **object, guint *len, HTMLUndoDirection dir, gboolean add_undo)
{
	GList *from, *to, *left, *right;
	guint position;
	gint level;

	html_engine_freeze (e);
	level = prepare_delete_bounds (e, &from, &to, &left, &right);
	place_cursor_before_mark (e);
	if (add_undo)
		check_flows (e, dir);
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

	if (html_cursor_backward (tail, e) && (!HTML_IS_TABLE (tail->object) || tail->offset))
		html_cursor_forward (tail, e);
	while (tail->offset == 0 && HTML_IS_TABLE (tail->object) && e->mark->position != e->cursor->position)
		html_cursor_backward (tail, e);
}

static void
check_table_1 (HTMLEngine *e)
{
	HTMLCursor *head;

	head = e->mark->position > e->cursor->position ? e->cursor : e->mark;

	if (html_cursor_forward (head, e) && (!HTML_IS_TABLE (head->object) || head->offset == 0))
		html_cursor_backward (head, e);
	while (head->offset == 1 && HTML_IS_TABLE (head->object) && e->mark->position != e->cursor->position)
		html_cursor_forward (head, e);
}

static gboolean
validate_tables (HTMLEngine *e, HTMLUndoDirection dir, gboolean add_undo, gboolean *fix_para)
{
	HTMLObject *next = html_object_next_not_slave (e->cursor->object);

	*fix_para = FALSE;

	if (next && HTML_IS_TABLE (next)) {
		insert_empty_paragraph (e, dir, add_undo);
		*fix_para = FALSE;

		return TRUE;
	} else if (!next) {
		gint steps = 0;

		while (html_cursor_forward (e->cursor, e)) {
			steps ++;
			if (HTML_IS_TABLE (e->cursor->object)) {
				next = html_object_next_not_slave (e->cursor->object);
				if (next) {
					insert_empty_paragraph (e, dir, FALSE);
					*fix_para = TRUE;
					steps ++;
					break;
				}
			} else
				break;
		}

		if (steps)
			html_cursor_backward_n (e->cursor, e, steps);
	}

	return FALSE;
}

static inline gboolean
in_aligned (HTMLCursor *cursor)
{
	return cursor->object->parent && HTML_IS_CLUEALIGNED (cursor->object->parent);
}

struct _FixEmptyAlignedUndo {
	HTMLUndoData data;

	HTMLObject *ac;
};
typedef struct _FixEmptyAlignedUndo FixEmptyAlignedUndo;

static void
fix_empty_aligned_undo_destroy (HTMLUndoData *data)
{
	FixEmptyAlignedUndo *undo = (FixEmptyAlignedUndo *) data;

	if (undo->ac)
		html_object_destroy (undo->ac);
}

static void
fix_empty_aligned_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir, guint position_after)
{
	HTMLObject *ac, *flow;

	g_return_if_fail (html_object_is_text (e->cursor->object) && HTML_TEXT (e->cursor->object)->text_len == 0
			  && e->cursor->object->parent && HTML_IS_CLUEFLOW (e->cursor->object->parent));

	ac = ((FixEmptyAlignedUndo *) data)->ac;
	((FixEmptyAlignedUndo *) data)->ac = NULL;

	html_engine_freeze (e);
	flow = e->cursor->object->parent;
	html_clue_remove_text_slaves (HTML_CLUE (flow));
	html_clue_append_after (HTML_CLUE (flow), ac, e->cursor->object);
	html_object_remove_child (flow, e->cursor->object);
	html_clue_append (HTML_CLUE (ac), e->cursor->object);
	html_object_change_set_down (flow, HTML_CHANGE_ALL);
	html_engine_thaw (e);
}

static void
fix_empty_aligned_setup_undo (HTMLEngine *e, HTMLUndoDirection dir, HTMLObject *ac)
{
	FixEmptyAlignedUndo *undo;

	undo = g_new (FixEmptyAlignedUndo, 1);

	html_undo_data_init (HTML_UNDO_DATA (undo));
	undo->data.destroy = fix_empty_aligned_undo_destroy;
	undo->ac       = ac;

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Remove empty aligned", fix_empty_aligned_undo_action,
						    HTML_UNDO_DATA (undo), html_cursor_get_position (e->cursor),
						    html_cursor_get_position (e->cursor)),
			      dir);
}

static void
fix_empty_aligned (HTMLEngine *e, HTMLUndoDirection dir, gboolean add_undo)
{
	if (html_object_is_text (e->cursor->object) && e->cursor->object->parent && HTML_IS_CLUEALIGNED (e->cursor->object->parent)) {
		HTMLObject *ac = e->cursor->object->parent;

		if (ac->parent && HTML_IS_CLUEFLOW (ac->parent)) {
			html_engine_freeze (e);
			html_clue_remove_text_slaves (HTML_CLUE (ac));
			html_object_remove_child (ac, e->cursor->object);
			html_clue_append_after (HTML_CLUE (ac->parent), e->cursor->object, ac);
			html_object_change_set_down (ac->parent, HTML_CHANGE_ALL);
			html_object_remove_child (ac->parent, ac);
			if (add_undo)
				fix_empty_aligned_setup_undo (e, dir, ac);
			html_engine_thaw (e);
		}
	}
}

static gint
delete_object (HTMLEngine *e, HTMLObject **ret_object, guint *ret_len, HTMLUndoDirection dir, gboolean add_undo)
{
	html_engine_edit_selection_updater_update_now (e->selection_updater);
	if (html_engine_is_selection_active (e)) {
		HTMLObject *object;
		guint len, position_before, saved_position, end_position;
		gint level;
		gboolean backward;
		gboolean fix_para;

		end_position = MIN (e->cursor->position, e->mark->position);
		if (HTML_IS_TABLE (e->cursor->object)
		    || (e->cursor->object->parent && e->cursor->object->parent->parent && HTML_IS_TABLE_CELL (e->cursor->object->parent->parent))
		    || HTML_IS_TABLE (e->mark->object)
		    || (e->mark->object->parent && e->mark->object->parent->parent && HTML_IS_TABLE_CELL (e->mark->object->parent->parent))) {
			check_table_0 (e);
			check_table_1 (e);
			html_engine_edit_selection_updater_update_now (e->selection_updater);
		}
		if (!html_engine_is_selection_active (e) || e->cursor->position == e->mark->position) {
			html_engine_disable_selection (e);
			html_cursor_jump_to_position (e->cursor, e, end_position);
			return 0;
		}

		position_before = MAX (e->cursor->position, e->mark->position);
		level = delete_object_do (e, &object, &len, dir, add_undo);
		if (ret_object && ret_len) {
			*ret_object = html_object_op_copy (object, NULL, e, NULL, NULL, ret_len);
			*ret_len    = len;
		}
		backward = validate_tables (e, dir, add_undo, &fix_para);
		if (fix_para) {
			saved_position = e->cursor->position;
			e->cursor->position = position_before + 1;
			insert_setup_undo (e, 1, position_before, dir, FALSE, FALSE);
			e->cursor->position = saved_position;
		}
		level = html_object_get_parent_level (e->cursor->object) - level + 1;
		if (add_undo) {
			delete_setup_undo (e, object, len, position_before + (backward ? 1 : 0), level, dir);
		} else
			html_object_destroy (object);

		if (backward)
			html_cursor_backward (e->cursor, e);
		gtk_html_editor_event (e->widget, GTK_HTML_EDITOR_EVENT_DELETE, NULL);
		fix_empty_aligned (e, dir, add_undo);

		return level;
	}

	return 0;
}

gint
html_engine_cut (HTMLEngine *e)
{
	gint rv;

	html_engine_clipboard_clear (e);
	html_undo_level_begin (e->undo, "Cut", "Uncut");
	rv = delete_object (e, &e->clipboard, &e->clipboard_len, HTML_UNDO_UNDO, TRUE);
	html_undo_level_end (e->undo);

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

	html_cursor_copy (e->cursor, orig);
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

static gboolean fix_aligned_position (HTMLEngine *e, guint *position_after, HTMLUndoDirection dir);

static void
fix_aligned_redo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir, guint position_after)
{
	guint pa;

	fix_aligned_position (e, &pa, html_undo_direction_reverse (dir));
}

static void
fix_aligned_undo_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir, guint position_after)
{
	HTMLObject *cf = e->cursor->object->parent;
	HTMLUndoData *undo;
	guint position_before = e->cursor->position;

	undo = g_new (HTMLUndoData, 1);

	if (!html_cursor_forward (e->cursor, e))
		g_assert (html_cursor_backward (e->cursor, e));
	else
		e->cursor->position --;

	html_clue_remove (HTML_CLUE (cf->parent), cf);
	html_object_destroy (cf);

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Fix aligned", fix_aligned_redo_action,
						    undo, html_cursor_get_position (e->cursor),
						    position_before),
			      html_undo_direction_reverse (dir));
}

static void
fix_align_setup_undo (HTMLEngine *e, guint position_before, HTMLUndoDirection dir)
{
	HTMLUndoData *undo;

	undo = g_new (HTMLUndoData, 1);

	html_undo_data_init (HTML_UNDO_DATA (undo));
	/* printf ("insert undo len %d\n", len); */

	html_undo_add_action (e->undo,
			      html_undo_action_new ("Undo aligned fix", fix_aligned_undo_action,
						    undo, html_cursor_get_position (e->cursor),
						    position_before),
			      dir);
}

static gboolean
fix_aligned_position (HTMLEngine *e, guint *position_after, HTMLUndoDirection dir)
{
	gboolean rv = FALSE;
	if (in_aligned (e->cursor)) {
		/* printf ("in aligned\n"); */
		if (e->cursor->offset) {
			if (html_cursor_forward (e->cursor, e))
				(*position_after) ++;
			if (in_aligned (e->cursor)) {
				HTMLObject *cf;
				HTMLObject *cluev;
				HTMLObject *flow;

				/* printf ("aligned: needs fixing\n"); */
				html_engine_freeze (e);
				cf = html_clueflow_new_from_flow (HTML_CLUEFLOW (e->cursor->object->parent->parent));
				flow = e->cursor->object->parent->parent;
				cluev = flow->parent;
				e->cursor->object = html_engine_new_text_empty (e);
				html_clue_append (HTML_CLUE (cf), e->cursor->object);
				html_clue_append_after (HTML_CLUE (cluev), cf, flow);
				e->cursor->offset = 0;
				e->cursor->position ++;
				(*position_after) ++;
#ifdef OP_DEBUG
				gtk_html_debug_dump_tree_simple (e->clue, 0);
#endif
				fix_align_setup_undo (e, e->cursor->position, dir);
				html_engine_thaw (e);
				rv = TRUE;
				if (e->cursor->object->parent && HTML_IS_CLUEALIGNED (e->cursor->object->parent))
					html_cursor_forward (e->cursor, e);

			}
		} else {
			if (html_cursor_backward (e->cursor, e))
				(*position_after) --;
			if (in_aligned (e->cursor)) {
				HTMLObject *cf;
				HTMLObject *cluev;
				HTMLObject *flow;

				/* printf ("aligned: needs fixing\n"); */
				html_engine_freeze (e);
				cf = html_clueflow_new_from_flow (HTML_CLUEFLOW (e->cursor->object->parent->parent));
				flow = e->cursor->object->parent->parent;
				cluev = flow->parent;
				e->cursor->object = html_engine_new_text_empty (e);
				html_clue_append (HTML_CLUE (cf), e->cursor->object);
				if (flow->prev)
					html_clue_append_after (HTML_CLUE (cluev), cf, flow->prev);
				else
					html_clue_prepend (HTML_CLUE (cluev), cf);
				e->cursor->offset = 0;
#ifdef OP_DEBUG
				gtk_html_debug_dump_tree_simple (e->clue, 0);
#endif
				fix_align_setup_undo (e, e->cursor->position, dir);
				html_engine_thaw (e);
				rv = TRUE;
			}
		}
	}

	return rv;
}

static void
insert_object_for_undo (HTMLEngine *e, HTMLObject *obj, guint len, guint position_after, gint level,
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

static void
insert_object (HTMLEngine *e, HTMLObject *obj, guint len, guint position_after, gint level,
	       HTMLUndoDirection dir, gboolean check)
{
	fix_aligned_position (e, &position_after, dir);
	insert_object_for_undo (e, obj, len, position_after, level, dir, check);
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

		copy = html_object_op_copy (e->clipboard, NULL, e, NULL, NULL, &len);
		html_engine_paste_object (e, copy, e->clipboard_len);
	}
}

static void
check_magic_link (HTMLEngine *e, const gchar *text, guint len)
{
	if (HTML_IS_TEXT (e->cursor->object)
	    && gtk_html_get_magic_links (e->widget) 
	    && len == 1
	    && (*text == ' ' || text [0] == '\n' || text [0] == '>' || text [0] == ')'))
		html_text_magic_link (HTML_TEXT (e->cursor->object), e, html_object_get_length (e->cursor->object));
}

static void
insert_empty_paragraph (HTMLEngine *e, HTMLUndoDirection dir, gboolean add_undo)
{
	GList *left=NULL, *right=NULL;
	HTMLCursor *orig;
	guint position_before;
	guint position_after;

	if (dir == HTML_UNDO_UNDO)
		if (fix_aligned_position (e, &position_after, dir))
			return;

	html_engine_freeze (e);

	position_before = e->cursor->position;
	orig = html_cursor_dup (e->cursor);
	split_and_add_empty_texts (e, 2, &left, &right);
	remove_empty_and_merge (e, FALSE, left, right, orig);

	/* replace empty link in empty flow by text with the same style */
	/* FIXME-link if (HTML_IS_LINK_TEXT (e->cursor->object) && html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
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
		} */

	html_cursor_forward (e->cursor, e);

	/* replace empty text in new empty flow by text with current style */
	if (html_clueflow_is_empty (HTML_CLUEFLOW (e->cursor->object->parent))) {
		HTMLObject *flow = e->cursor->object->parent;

		html_clue_remove (HTML_CLUE (flow), e->cursor->object);
		html_object_destroy (e->cursor->object);
		e->cursor->object = html_engine_new_text_empty (e);
		html_clue_append (HTML_CLUE (flow), e->cursor->object);
	}

	if (add_undo) {
		html_undo_level_begin (e->undo, "Insert paragraph", "Delete paragraph");
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
	if (add_undo)
		html_undo_level_end (e->undo);

	html_engine_thaw (e);
}

void
html_engine_insert_empty_paragraph (HTMLEngine *e)
{
	insert_empty_paragraph (e, HTML_UNDO_UNDO, TRUE);
}

static char *picto_chars = "DO)(|/PQ\0:-\0:\0:-\0:\0:;=-\0:;\0:-~\0:\0:\0:-\0:\0:-\0:\0:-\0:\0:-\0:\0";
static gint picto_states [] = { 9, 14, 19, 27, 35, 40, 45, 50, 0, -1, 12, 0, -1, 0, -2, 17, 0, -2, 0, -3, -4, -5, 24, 0, -3, -4, 0, -6, 31, 33, 0, -6, 0, -11, 0, -8, 38, 0, -8, 0, -9, 43, 0, -9, 0, -10, 48, 0, -10, 0, -12, 53, 0, -12, 0};
static gchar *picto_images [] = {
	"smiley-1.png",
	"smiley-2.png",
	"smiley-3.png",
	"smiley-4.png",
	"smiley-5.png",
	"smiley-6.png",
	"smiley-7.png",
	"smiley-8.png",
	"smiley-9.png",
	"smiley-10.png",
	"smiley-11.png",
	"smiley-12.png",
};

static void
use_pictograms (HTMLEngine *e)
{
	gint pos;
	gint state;
	gint relative;
	gunichar uc;

	if (!html_object_is_text (e->cursor->object) || !gtk_html_get_magic_smileys (e->widget))
		return;

	pos = e->cursor->offset - 1;
	state = 0;
	while (pos >= 0) {
		uc = html_text_get_char (HTML_TEXT (e->cursor->object), pos);
		relative = 0;
		while (picto_chars [state + relative]) {
			if (picto_chars [state + relative] == uc)
				break;
			relative ++;
		}
		state = picto_states [state + relative];
		/* 0 .. not found, -n .. found n-th */
		if (state <= 0)
			break;
		pos --;
	}

	if (state < 0) {
		HTMLObject *picto;
		gchar *filename;
		gchar *alt;
		gint len;

		if (pos > 0) {
			uc = html_text_get_char (HTML_TEXT (e->cursor->object), pos - 1);
			if (uc != ' ' && uc != '\t')
				return;
		}
		/* printf ("found %d\n", -state); */
		len = e->cursor->offset  - pos;
		alt = g_strndup (html_text_get_text (HTML_TEXT (e->cursor->object), pos), len);
		html_cursor_backward_n (e->cursor, e, len);
		html_engine_set_mark (e);
		html_cursor_forward_n (e->cursor, e, len);

		filename = g_strconcat ("file://" ICONDIR "/", picto_images [-state - 1], NULL);
		picto = html_image_new (html_engine_get_image_factory (e), filename, NULL, NULL, -1, -1, FALSE, FALSE, 0, NULL,
					HTML_VALIGN_MIDDLE, FALSE);
		html_image_set_alt (HTML_IMAGE (picto), alt);
		html_object_set_data (HTML_OBJECT (picto), "picto", alt);
		g_free (alt);
		html_engine_paste_object (e, picto, html_object_get_length (picto));
	}
}

void
html_engine_insert_text_with_extra_attributes (HTMLEngine *e, const gchar *text, guint len, PangoAttrList *attrs)
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
		nl   = g_utf8_strchr (text, -1, '\n');
		alen = nl ? g_utf8_pointer_to_offset (text, nl) : len;
		if (alen) {
			HTMLObject *o;
			gboolean check = FALSE;

			check_magic_link (e, text, alen);

			/* stop inserting links after space */
			if (*text == ' ')
				html_engine_set_insertion_link (e, NULL, NULL);

			o = html_engine_new_text (e, text, alen);
			if (attrs)
				HTML_TEXT (o)->extra_attr_list = pango_attr_list_copy (attrs);
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
			if (alen == 1 && !HTML_IS_PLAIN_PAINTER (e->painter))
				use_pictograms (e);
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
html_engine_insert_text (HTMLEngine *e, const gchar *text, guint len)
{
	html_engine_insert_text_with_extra_attributes (e, text, len, NULL);
}

void
html_engine_paste_text_with_extra_attributes (HTMLEngine *e, const gchar *text, guint len, PangoAttrList *attrs)
{
	gchar *undo_name = g_strdup_printf ("Paste text: '%s'", text);
	gchar *redo_name = g_strdup_printf ("Unpaste text: '%s'", text);

	html_undo_level_begin (e->undo, undo_name, redo_name);
	g_free (undo_name);
	g_free (redo_name);
	html_engine_delete (e);
	html_engine_insert_text_with_extra_attributes (e, text, len, attrs);
	html_undo_level_end (e->undo);
}

void
html_engine_paste_text (HTMLEngine *e, const gchar *text, guint len)
{
	html_engine_paste_text_with_extra_attributes (e, text, len, NULL);
}

void
html_engine_paste_link (HTMLEngine *e, const char *text, int len, const char *complete_url)
{
	char *url, *target;

	if (len == -1)
		len = g_utf8_strlen (text, -1);

	url = g_strdup (complete_url);
	target = strrchr (url, '#');
	if (target) {
		*target = 0;
		target ++;
	}

	html_engine_paste_text (e, text, len);
	html_text_add_link (HTML_TEXT (e->cursor->object), e, url, target, e->cursor->offset - len, e->cursor->offset);

	g_free (url);
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
		/* Remove magic smiley */
		if (!forward && len == 1 && gtk_html_get_magic_smileys (e->widget)) {
			HTMLObject *object = html_object_get_tail_leaf (e->cursor->object);

			if (HTML_IS_IMAGE (object) && html_object_get_data (object, "picto") != NULL) {
				gchar *picto = g_strdup (html_object_get_data (object, "picto"));
				html_undo_level_begin (e->undo, "Remove Magic Smiley", "Undo Remove Magic Smiley");
				html_cursor_backward (e->cursor, e);
				html_engine_delete (e);
				html_engine_insert_text (e, picto, -1);
				html_undo_level_end (e->undo);
				g_free (picto);

				html_engine_unblock_selection (e);
				html_engine_thaw (e);
				return;
			}
		}
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
html_engine_edit_set_link (HTMLEngine *e, const gchar *url, const gchar *target)
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
replace_objects_in_clue_from_another (HTMLClue *dest, HTMLClue *src)
{
	HTMLObject *cur, *next;

	for (cur = dest->head; cur; cur = next) {
		next = cur->next;
		html_object_remove_child (cur->parent, cur);
		html_object_destroy (cur);
	}

	for (cur = src->head; cur; cur = next) {
		next = cur->next;
		html_object_remove_child (cur->parent, cur);
		html_clue_append (dest, cur);
	}
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
	html_object_change_set (o, HTML_CHANGE_ALL_CALC);

	e->cursor->object = html_object_get_head_leaf (o);
	e->cursor->offset = 0;

	/* be sure we have valid cursor position (like when there is a focusable container) */
	position = e->cursor->position;
	while (html_cursor_backward (e->cursor, e))
		;
	e->cursor->position = position;

	/* we move objects between flows to preserve attributes as indentation, ... */
	if (HTML_IS_CLUEFLOW (o)) {
		replace_objects_in_clue_from_another (HTML_CLUE (where), HTML_CLUE (o));
		html_object_destroy (o);
	} else {
		html_clue_append_after (HTML_CLUE (where->parent), o, where);
		html_object_remove_child (where->parent, where);
		html_object_destroy (where);
	}

	html_cursor_forward_n (e->cursor, e, len);
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

static void
delete_upto (HTMLEngine *e, HTMLCursor **start, HTMLCursor **end, HTMLObject *object, guint offset)
{
	guint position;

	if (e->mark)
		html_cursor_destroy (e->mark);
	e->mark = *start;
	html_cursor_jump_to (e->cursor, e, object, offset);
	position = e->cursor->position;
	delete_object (e, NULL, NULL, HTML_UNDO_UNDO, TRUE);
	*start = html_cursor_dup (e->cursor);
	html_cursor_forward (*start, e);
	(*end)->position -= position - e->cursor->position;
}

void
html_engine_delete (HTMLEngine *e)
{
	html_undo_level_begin (e->undo, "Delete", "Undelete");
	html_engine_edit_selection_updater_update_now (e->selection_updater);
	if (html_engine_is_selection_active (e)) {
		HTMLCursor *start = html_cursor_dup (e->mark->position < e->cursor->position ? e->mark : e->cursor);
		HTMLCursor *end = html_cursor_dup (e->mark->position < e->cursor->position ? e->cursor : e->mark);
		gint start_position = start->position;

		while (start->position < end->position) {
			if (start->object->parent->parent == end->object->parent->parent) {
				if (e->mark)
					html_cursor_destroy (e->mark);
				html_cursor_destroy (e->cursor);
				e->mark = start;
				e->cursor = end;
				start = end = NULL;
				delete_object (e, NULL, NULL, HTML_UNDO_UNDO, TRUE);
				break;
			} else {
				HTMLObject *prev = NULL, *cur = start->object;

				/* go thru current cluev */
				do {
				        /* go thru current flow */
					while (cur) {
				                /* lets look if container is whole contained in the selection */
						if (html_object_is_container (cur)) {
							html_cursor_jump_to (e->cursor, e, cur, html_object_get_length (cur));
							if (e->cursor->position > end->position) {
								/* it's not => delete upto this container */

								delete_upto (e, &start, &end, cur, 0);
								prev = NULL;
								break;
							}
						}
						prev = cur;
						cur = html_object_next_not_slave (cur);
					}
				} while (prev && prev->parent->next && (cur = html_object_head (prev->parent->next)));

				if (prev)
				        /* cluev end is in the selection */
					delete_upto (e, &start, &end, prev, html_object_get_length (prev));
			}
		}

		if (start)
			html_cursor_destroy (start);
		if (end)
			html_cursor_destroy (end);
		html_cursor_jump_to_position (e->cursor, e, start_position);
	}
	html_undo_level_end (e->undo);
}
