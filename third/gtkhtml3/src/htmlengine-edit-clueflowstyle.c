/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

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
#include "htmlengine-edit-clueflowstyle.h"
#include "htmlundo.h"
#include "htmltype.h"
#include "htmlcursor.h"
#include "htmlselection.h"


/* Properties of a paragraph.  */
struct _ClueFlowProps {
	HTMLClueFlowStyle style;
	HTMLListType item_type;
	HTMLHAlignType alignment;
	GByteArray *levels;
};
typedef struct _ClueFlowProps ClueFlowProps;

/* Data for the redo/undo operation.  */
struct _ClueFlowStyleOperation {
	HTMLUndoData data;
	
	/* Whether we should go backward or forward when re-setting
           the style.  */
	gboolean forward;

	/* List of properties for the paragraphs (ClueFlowProps).  */
	GList *prop_list;
};
typedef struct _ClueFlowStyleOperation ClueFlowStyleOperation;

static void
free_prop (ClueFlowProps *props)
{
	g_byte_array_free (props->levels, TRUE);
	g_free (props);
}

static void
free_prop_list (GList *list)
{
	GList *p;

	for (p = list; p != NULL; p = p->next) {
		ClueFlowProps *props;

		props = (ClueFlowProps *) p->data;
		free_prop (props);
	}

	g_list_free (list);
}

static void
style_operation_destroy (HTMLUndoData *data)
{
	free_prop_list (((ClueFlowStyleOperation *) data)->prop_list);
}

static ClueFlowStyleOperation *
style_operation_new (GList *prop_list, gboolean forward)
{
	ClueFlowStyleOperation *op;

	op = g_new (ClueFlowStyleOperation, 1);

	html_undo_data_init (HTML_UNDO_DATA (op));

	op->data.destroy = style_operation_destroy;
	op->prop_list    = prop_list;
	op->forward      = forward;

	return op;
}

static ClueFlowProps *
get_props (HTMLClueFlow *clueflow)
{
	ClueFlowProps *props;

	props = g_new (ClueFlowProps, 1);

	props->levels      = html_clueflow_dup_levels (clueflow);
	props->alignment   = html_clueflow_get_halignment (clueflow);
	props->style       = html_clueflow_get_style (clueflow);
	props->item_type   = html_clueflow_get_item_type (clueflow);

	return props;
}


static void
set_props (HTMLEngine *engine,
	   HTMLClueFlow *clueflow,
	   HTMLClueFlowStyle style,
	   HTMLListType item_type,
	   HTMLHAlignType alignment,
	   gint indentation_delta,
	   guint8 *indentation_levels,
	   HTMLEngineSetClueFlowStyleMask mask)
{
	if (mask & HTML_ENGINE_SET_CLUEFLOW_INDENTATION)
		html_clueflow_set_indentation (clueflow, engine, indentation_delta, indentation_levels);
	
	if (mask & HTML_ENGINE_SET_CLUEFLOW_INDENTATION_DELTA)
		html_clueflow_modify_indentation_by_delta (clueflow, engine, indentation_delta, indentation_levels);
	
	/* FIXME levels mostly work now */
	if (mask & HTML_ENGINE_SET_CLUEFLOW_STYLE) {
		if (style == HTML_CLUEFLOW_STYLE_LIST_ITEM && clueflow->style != HTML_CLUEFLOW_STYLE_LIST_ITEM
		    && clueflow->levels->len == 0
		    && !(mask & (HTML_ENGINE_SET_CLUEFLOW_INDENTATION | HTML_ENGINE_SET_CLUEFLOW_INDENTATION_DELTA))) {
			guint8 tmp = item_type;
			html_clueflow_set_indentation (clueflow, engine, 1, &tmp);
		} else if (clueflow->style == HTML_CLUEFLOW_STYLE_LIST_ITEM && style != HTML_CLUEFLOW_STYLE_LIST_ITEM
			   && clueflow->levels->len == 1 
			   && !(mask & (HTML_ENGINE_SET_CLUEFLOW_INDENTATION | HTML_ENGINE_SET_CLUEFLOW_INDENTATION_DELTA))) {
			html_clueflow_set_indentation (clueflow, engine, 0, NULL);
		}
		html_clueflow_set_style (clueflow, engine, style);
		html_clueflow_set_item_type (clueflow, engine, item_type);
		html_object_change_set_down (HTML_OBJECT (clueflow), HTML_CHANGE_ALL);
	}
	if (mask & HTML_ENGINE_SET_CLUEFLOW_ALIGNMENT)
		html_clueflow_set_halignment (clueflow, engine, alignment);
	
}


/* Undo/redo operations.  */

static void add_undo (HTMLEngine *engine, ClueFlowStyleOperation *op, HTMLUndoDirection dir);

static void
undo_or_redo (HTMLEngine *engine, HTMLUndoData *data, HTMLUndoDirection dir, guint position_after)
{
	ClueFlowStyleOperation *op, *new_op;
	ClueFlowProps *props, *orig_props;
	HTMLObject *obj;
	HTMLClueFlow *clueflow;
	GList *prop_list;
	GList *p;

	op = (ClueFlowStyleOperation *) data;
	g_assert (op != NULL);
	g_assert (op->prop_list != NULL);

	obj = engine->cursor->object;
	g_assert (obj != NULL);

	prop_list = NULL;

	p = op->prop_list;

	while (p != NULL) {
		if (HTML_OBJECT_TYPE (obj->parent) != HTML_TYPE_CLUEFLOW) {
			g_warning ("(%s:%s)  Eeeek!  Unknown parent type `%s'.",
				   __FILE__, G_GNUC_FUNCTION,
				   html_type_name (HTML_OBJECT_TYPE (obj->parent)));
			break;
		}

		clueflow = HTML_CLUEFLOW (obj->parent);

		orig_props = get_props (clueflow);
		prop_list = g_list_prepend (prop_list, orig_props);

		props = (ClueFlowProps *) p->data;

		html_clueflow_set_levels (clueflow, engine, props->levels);
		html_clueflow_set_style (clueflow, engine, props->style);
		html_clueflow_set_item_type (clueflow, engine, props->item_type);
		html_clueflow_set_levels (clueflow, engine, props->levels);
		html_clueflow_set_halignment (clueflow, engine, props->alignment);

		p = p->next;
		if (p == NULL)
			break;

		/* Go forward object by object, until we find one
                   whose parent (i.e. paragraph) is different.  */
		do {
			if (op->forward)
				obj = html_object_next_leaf (obj);
			else
				obj = html_object_prev_leaf (obj);

			if (obj == NULL) {
				/* This should not happen.  */
				g_warning ("(%s:%s)  There were not enough paragraphs for "
					   "setting the paragraph style.",
					   __FILE__, G_GNUC_FUNCTION);
				break;
			}
		} while (obj != NULL && HTML_CLUEFLOW (obj->parent) == clueflow);
	}

	if (prop_list == NULL) {
		/* This should not happen.  */
		g_warning ("%s:%s Eeek!  Nothing done?", __FILE__, G_GNUC_FUNCTION);
		return;
	}

	prop_list = g_list_reverse (prop_list);

	new_op = style_operation_new (prop_list, op->forward);

	add_undo (engine, new_op, html_undo_direction_reverse (dir));
}

static HTMLUndoAction *
undo_action_from_op (HTMLEngine *engine,
		     ClueFlowStyleOperation *op)
{
	return html_undo_action_new ("Paragraph style change",
				     undo_or_redo, HTML_UNDO_DATA (op),
				     html_cursor_get_position (engine->cursor),
				     html_cursor_get_position (engine->cursor));
}

static void
add_undo (HTMLEngine *engine,
	  ClueFlowStyleOperation *op, HTMLUndoDirection dir)
{
	html_undo_add_action (engine->undo, undo_action_from_op (engine, op), dir);
}


/* "Do" operations.  */

static void
set_clueflow_style_in_region (HTMLEngine *engine,
			      HTMLClueFlowStyle style,
			      HTMLListType item_type,
			      HTMLHAlignType alignment,
			      gint indentation_delta,
			      guint8 *indentation_levels,
			      HTMLEngineSetClueFlowStyleMask mask,
			      HTMLUndoDirection dir,
			      gboolean do_undo)
{
	HTMLClueFlow *clueflow;
	HTMLObject *start, *end, *p;
	GList *prop_list;
	gboolean undo_forward;

	if (html_cursor_precedes (engine->cursor, engine->mark)) {
		start = engine->cursor->object;
		end = engine->mark->object;
		undo_forward = TRUE;
	} else {
		start = engine->mark->object;
		end = engine->cursor->object;
		undo_forward = FALSE;
	}

	prop_list = NULL;

	p = start;
	while (p != NULL) {
		if (HTML_OBJECT_TYPE (p->parent) != HTML_TYPE_CLUEFLOW) {
			g_warning ("(%s:%s)  Eeeek!  Unknown parent type `%s'.",
				   __FILE__, G_GNUC_FUNCTION,
				   html_type_name (HTML_OBJECT_TYPE (p->parent)));
			break;
		}

		clueflow = HTML_CLUEFLOW (p->parent);

		if (do_undo)
			prop_list = g_list_prepend (prop_list, get_props (clueflow));

		set_props (engine, clueflow,
			   style, item_type, alignment, indentation_delta, indentation_levels,
			   mask);
		
		if (p == end)
			break;

		do {
			p = html_object_next_leaf (p);
		} while (p != NULL && p != end && HTML_CLUEFLOW (p->parent) == clueflow);

		if (p == end && HTML_CLUEFLOW (p->parent) == clueflow)
			break;
	}

	if (! do_undo)
		return;

	add_undo (engine, style_operation_new (undo_forward ? g_list_reverse (prop_list) : prop_list, undo_forward), dir);
}

static void
set_clueflow_style_at_cursor (HTMLEngine *engine,
			      HTMLClueFlowStyle style,
			      HTMLListType item_type,
			      HTMLHAlignType alignment,
			      gint indentation_delta,
			      guint8 *indentation_levels,
			      HTMLEngineSetClueFlowStyleMask mask,
			      HTMLUndoDirection dir, gboolean do_undo)
{
	HTMLClueFlow *clueflow;
	HTMLObject *curr;

	curr = engine->cursor->object;

	g_return_if_fail (curr != NULL);
	g_return_if_fail (curr->parent != NULL);
	g_return_if_fail (HTML_OBJECT_TYPE (curr->parent) == HTML_TYPE_CLUEFLOW);

	clueflow = HTML_CLUEFLOW (curr->parent);

	if (do_undo)
		add_undo (engine, style_operation_new (g_list_append (NULL, get_props (clueflow)), TRUE), dir);

	set_props (engine, clueflow,
		   style, item_type, alignment, 
		   indentation_delta,
		   indentation_levels,
		   mask);

}


gboolean
html_engine_set_clueflow_style (HTMLEngine *engine,
				HTMLClueFlowStyle style,
				HTMLListType item_type,
				HTMLHAlignType alignment,
				gint indentation_delta,
				guint8 *indentation_levels,
				HTMLEngineSetClueFlowStyleMask mask,
				HTMLUndoDirection dir, gboolean do_undo)
{
	g_return_val_if_fail (engine != NULL, FALSE);
	g_return_val_if_fail (HTML_IS_ENGINE (engine), FALSE);

	html_engine_freeze (engine);
	if (html_engine_is_selection_active (engine))
		set_clueflow_style_in_region (engine,
					      style, item_type, alignment, 
					      indentation_delta, indentation_levels,
					      mask,
					      dir, do_undo);
	else
		set_clueflow_style_at_cursor (engine,
					      style, item_type, alignment, 
					      indentation_delta, indentation_levels,
					      mask,
					      dir, do_undo);
	html_engine_thaw (engine);

	/* This operation can never fail.  */
	return TRUE;
}


/* The following functions are used to report the current indentation
   as it should be shown e.g in a toolbar.  */

static HTMLClueFlow *
get_current_para (HTMLEngine *engine)
{
	HTMLObject *current;
	HTMLObject *parent;

	current = engine->cursor->object;
	if (current == NULL)
		return NULL;

	parent = current->parent;
	if (parent == NULL)
		return NULL;

	if (HTML_OBJECT_TYPE (parent) != HTML_TYPE_CLUEFLOW)
		return NULL;

	return HTML_CLUEFLOW (parent);
}

void
html_engine_get_current_clueflow_style (HTMLEngine *engine, HTMLClueFlowStyle *style, HTMLListType *item_type)
{
	HTMLClueFlow *para;

	/* FIXME TODO region */

	*style = HTML_CLUEFLOW_STYLE_NORMAL;
	*item_type = HTML_LIST_TYPE_UNORDERED;

	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	para = get_current_para (engine);
	if (para) {
		*style = para->style;
		*item_type = para->item_type;
	}
}
 
guint
html_engine_get_current_clueflow_indentation (HTMLEngine *engine)
{
	HTMLClueFlow *para;

	/* FIXME TODO region */

	g_return_val_if_fail (engine != NULL, 0);
	g_return_val_if_fail (HTML_IS_ENGINE (engine), 0);

	para = get_current_para (engine);
	if (para == NULL)
		return 0;
	
	/* FIXME levels TODO levels */
	return para->levels->len;
}

HTMLHAlignType
html_engine_get_current_clueflow_alignment (HTMLEngine *engine)
{
	HTMLClueFlow *para;

	/* FIXME TODO region */

	g_return_val_if_fail (engine != NULL, HTML_HALIGN_LEFT);
	g_return_val_if_fail (HTML_IS_ENGINE (engine), HTML_HALIGN_LEFT);

	para = get_current_para (engine);
	if (para == NULL)
		return 0;

	return html_clueflow_get_halignment (para);
}
