/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2000 Helix Code, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHcANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlundo.h"

struct _HTMLUndoStack {
	GList *stack;
	guint  size;
};
typedef struct _HTMLUndoStack HTMLUndoStack;

struct _HTMLUndo {
	HTMLUndoStack undo;
	HTMLUndoStack redo;
	HTMLUndoStack undo_used;

	/* these lists are stacks containing other
	   levels undo/redo after calling html_undo_level_start */
	GSList   *undo_levels;
	GSList   *redo_levels;
	guint     level;
	guint     in_redo;
};

#ifdef UNDO_DEBUG
static void html_undo_debug (HTMLUndo *undo);
#define DEBUG(x) html_undo_debug (x)
#else
#define DEBUG(x)
#endif

static void add_used_and_redo_to_undo (HTMLUndo *undo);

inline static void
stack_copy (HTMLUndoStack *src, HTMLUndoStack *dst)
{
	dst->stack = src->stack;
	dst->size  = src->size;
}

inline static void
stack_dup (HTMLUndoStack *src, HTMLUndoStack *dst)
{
	dst->stack = g_list_copy (src->stack);
	dst->size  = src->size;
}

static void
destroy_action_list (GList *lp)
{
	GList *p;

	for (p = lp; p != NULL; p = p->next)
		html_undo_action_destroy ((HTMLUndoAction *) p->data);
}


HTMLUndo *
html_undo_new (void)
{
	HTMLUndo *new_undo;

	new_undo = g_new0 (HTMLUndo, 1);

	return new_undo;
}

void
html_undo_destroy  (HTMLUndo *undo)
{
	g_return_if_fail (undo != NULL);

	destroy_action_list (undo->undo.stack);
	destroy_action_list (undo->redo.stack);

	g_free (undo);
}


static void
action_do_and_destroy_redo (HTMLEngine *engine, HTMLUndo *undo, GList **stack, HTMLUndoDirection dir)
{
	HTMLUndoAction *action;
	GList *first;

	first  = *stack;
	action = HTML_UNDO_ACTION (first->data);

	html_cursor_jump_to_position (engine->cursor, engine, action->position);
	(* action->function) (engine, action->data, dir);

	*stack = g_list_remove (first, first->data);
	if (undo->level == 0) {
		html_undo_action_destroy (action);

		first = undo->undo_used.stack;
		html_undo_action_destroy (HTML_UNDO_ACTION (first->data));
		undo->undo_used.stack = g_list_remove (first, first->data);
	}
}

static void
action_do_and_destroy_undo (HTMLEngine *engine, HTMLUndo *undo, HTMLUndoDirection dir)
{
	HTMLUndoAction *action;
	GList *first;

	first  = undo->undo.stack;
	action = HTML_UNDO_ACTION (first->data);

	html_cursor_jump_to_position (engine->cursor, engine, action->position);
	(* action->function) (engine, action->data, dir);

	undo->undo.stack = g_list_remove (first, first->data);
	if (undo->level == 0)
		undo->undo_used.stack = g_list_prepend (undo->undo_used.stack, action);
}

void
html_undo_do_undo (HTMLUndo *undo,
		   HTMLEngine *engine)
{
	g_return_if_fail (undo != NULL);
	g_return_if_fail (engine != NULL);

	if (undo->undo.size > 0) {
#ifdef UNDO_DEBUG
		if (!undo->level) {
			printf ("UNDO begin\n");
			DEBUG (undo);
		}
#endif
		engine->block_events ++;
		action_do_and_destroy_undo (engine, undo, HTML_UNDO_UNDO);
		undo->undo.size--;
		engine->block_events --;
#ifdef UNDO_DEBUG
		if (!undo->level) {
			DEBUG (undo);
			printf ("UNDO end\n");
		}
#endif
	}
}

void
html_undo_do_redo (HTMLUndo *undo,
		   HTMLEngine *engine)
{
	g_return_if_fail (undo != NULL);
	g_return_if_fail (engine != NULL);

	if (undo->redo.size > 0) {
#ifdef UNDO_DEBUG
		if (!undo->level) {
			printf ("REDO begin\n");
			DEBUG (undo);
		}
#endif
		undo->in_redo ++;
		engine->block_events ++;
		action_do_and_destroy_redo (engine, undo, &undo->redo.stack, HTML_UNDO_REDO);
		undo->redo.size--;
		engine->block_events --;
		undo->in_redo --;

#ifdef UNDO_DEBUG
		if (!undo->level) {
			DEBUG (undo);
			printf ("REDO end\n");
		}
#endif
	}
}


void
html_undo_discard_redo (HTMLUndo *undo)
{
	g_return_if_fail (undo != NULL);

	if (undo->redo.stack == NULL)
		return;

	destroy_action_list (undo->redo.stack);

	undo->redo.stack = NULL;
	undo->redo.size = 0;
}

void
html_undo_add_undo_action  (HTMLUndo *undo, HTMLUndoAction *action)
{
	g_return_if_fail (undo != NULL);
	g_return_if_fail (action != NULL);

	if (undo->level == 0 && undo->in_redo == 0 && undo->redo.size > 0)
		add_used_and_redo_to_undo (undo);

	if (!undo->level && undo->undo.size >= HTML_UNDO_LIMIT) {
		HTMLUndoAction *last_action;
		GList *last;

		last = g_list_last (undo->undo.stack);
		last_action = (HTMLUndoAction *) last->data;

		undo->undo.stack = g_list_remove_link (undo->undo.stack, last);
		g_list_free (last);

		html_undo_action_destroy (last_action);

		undo->undo.size--;
	}

	undo->undo.stack = g_list_prepend (undo->undo.stack, action);
	undo->undo.size ++;

#ifdef UNDO_DEBUG
	if (!undo->level) {
		printf ("ADD UNDO\n");
		DEBUG (undo);
	}
#endif
}

void
html_undo_add_redo_action  (HTMLUndo *undo,
			    HTMLUndoAction *action)
{
	g_return_if_fail (undo != NULL);
	g_return_if_fail (action != NULL);

	undo->redo.stack = g_list_prepend (undo->redo.stack, action);
	undo->redo.size++;
}

void
html_undo_add_action  (HTMLUndo *undo, HTMLUndoAction *action, HTMLUndoDirection dir)
{
	if (dir == HTML_UNDO_UNDO)
		html_undo_add_undo_action (undo, action);
	else
		html_undo_add_redo_action (undo, action);
}

/*
  undo levels

  * IDEA: it closes number of undo steps into one
  * examples: paste
               - it first cuts active selection and then inserts objects
                 from cut_buffer on actual cursor position
               - if you don't use udo levels, it will generate two undo steps/actions
              replace
               - replace uses paste operation, so when it replaces N occurences,
                 it generates 2*N steps (without using undo levels in paste and replace)

  * usage is simple - just call html_undo_level_begin before using functions with undo
    and html_undo_level_end after them

*/

#define HTML_UNDO_LEVEL(x) ((HTMLUndoLevel *) x)
struct _HTMLUndoLevel {
	HTMLUndoData data;

	HTMLUndo      *parent_undo;
	HTMLUndoStack  stack;

	gchar    *description [HTML_UNDO_END];
};
typedef struct _HTMLUndoLevel HTMLUndoLevel;

static void undo_step_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir);
static void redo_level_begin (HTMLUndo *undo, const gchar *redo_desription, const gchar *undo_desription);
static void redo_level_end   (HTMLUndo *undo);

static void
level_destroy (HTMLUndoData *data)
{
	HTMLUndoLevel *level;

	g_assert (data);

	level = HTML_UNDO_LEVEL (data);

	for (; level->stack.stack; level->stack.stack = level->stack.stack->next)
		html_undo_action_destroy (HTML_UNDO_ACTION (level->stack.stack->data));

	g_free (level->description [HTML_UNDO_UNDO]);
	g_free (level->description [HTML_UNDO_REDO]);
}

static HTMLUndoLevel *
level_new (HTMLUndo *undo, HTMLUndoStack *stack, const gchar *undo_description, const gchar *redo_description)
{
	HTMLUndoLevel *nl = g_new (HTMLUndoLevel, 1);

	html_undo_data_init (HTML_UNDO_DATA (nl));

	stack_copy (stack, &nl->stack);

	nl->data.destroy                 = level_destroy;
	nl->parent_undo                  = undo;
	nl->description [HTML_UNDO_UNDO] = g_strdup (undo_description);
	nl->description [HTML_UNDO_REDO] = g_strdup (redo_description);

	return nl;
}

void
html_undo_level_begin (HTMLUndo *undo, const gchar *undo_desription, const gchar *redo_desription)
{
	undo->undo_levels = g_slist_prepend (undo->undo_levels, level_new (undo, &undo->undo,
									   undo_desription, redo_desription));
	undo->undo.stack  = NULL;
	undo->undo.size   = 0;

	undo->level ++;
}

static void
redo_level_begin (HTMLUndo *undo, const gchar *redo_desription, const gchar *undo_desription)
{
	undo->redo_levels = g_slist_prepend (undo->redo_levels, level_new (undo, &undo->redo,
									   undo_desription, redo_desription));
	undo->redo.stack  = NULL;
	undo->redo.size   = 0;

	undo->level ++;
}

static void
redo_level_end (HTMLUndo *undo)
{
	HTMLUndoLevel *level;
	HTMLUndoStack  save_redo;
	GSList *head;

	g_assert (undo->redo_levels);

	undo->level --;

	/* preserve current redo stack */
	stack_copy (&undo->redo, &save_redo);

	/* restore last level from levels stack */
	level = HTML_UNDO_LEVEL (undo->redo_levels->data);
	stack_copy (&level->stack, &undo->redo);

	/* fill level with current redo step */
	stack_copy (&save_redo, &level->stack);

	/* add redo step redo action */
	if (save_redo.size) {
		HTMLUndoAction *action;

		/* we use position from last redo action on the stack */
		action = (HTMLUndoAction *) save_redo.stack->data;
		html_undo_add_redo_action (undo, action = html_undo_action_new (level->description [HTML_UNDO_REDO],
										undo_step_action,
										HTML_UNDO_DATA (level), action->position));
#ifdef UNDO_DEBUG
		action->is_level = TRUE;
#endif
	} else
		html_undo_data_unref (HTML_UNDO_DATA (level));

	head = undo->redo_levels;
	undo->redo_levels = g_slist_remove_link (undo->redo_levels, head);
	g_slist_free (head);
}

void
html_undo_level_end (HTMLUndo *undo)
{
	HTMLUndoLevel *level;
	HTMLUndoStack  save_undo;
	GSList *head;

	g_assert (undo->undo_levels);
	g_assert (undo->level);

	undo->level--;

	/* preserve current redo stack */
	stack_copy (&undo->undo, &save_undo);

	/* restore last level from levels stack */
	level = HTML_UNDO_LEVEL (undo->undo_levels->data);
	stack_copy (&level->stack, &undo->undo);

	/* fill level with current undo step */
	stack_copy (&save_undo, &level->stack);

	/* add undo step undo action */
	if (save_undo.size) {
		HTMLUndoAction *action;


		/* we use position from last undo action on the stack */
		action = html_undo_action_new (level->description [HTML_UNDO_UNDO],
					       undo_step_action,
					       HTML_UNDO_DATA (level),
					       HTML_UNDO_ACTION (save_undo.stack->data)->position);
#ifdef UNDO_DEBUG
		action->is_level = TRUE;
#endif
		html_undo_add_undo_action (undo, action);
	} else
		html_undo_data_unref (HTML_UNDO_DATA (level));

	head = undo->undo_levels;
	undo->undo_levels = g_slist_remove_link (undo->undo_levels, head);
	g_slist_free (head);
}

static void
undo_step_action (HTMLEngine *e, HTMLUndoData *data, HTMLUndoDirection dir)
{
	HTMLUndo      *undo;
	HTMLUndoLevel *level;
	HTMLUndoStack  save;
	HTMLUndoStack *stack;

	level = HTML_UNDO_LEVEL (data);
	undo  = level->parent_undo;
	stack = dir == HTML_UNDO_UNDO ? &undo->undo : &undo->redo;

	/* prepare undo/redo step */
	if (dir == HTML_UNDO_UNDO)
		redo_level_begin (undo, level->description [HTML_UNDO_REDO], level->description [HTML_UNDO_UNDO]);
	else
		html_undo_level_begin (undo, level->description [HTML_UNDO_UNDO], level->description [HTML_UNDO_REDO]);

	/* preserve current undo/redo stack */
	stack_copy (stack, &save);

	/* set this level */
	stack_dup (&level->stack, stack);

	undo->level ++;
	if (dir == HTML_UNDO_UNDO)
		while (undo->undo.size)
			html_undo_do_undo (undo, e);
	else
		while (undo->redo.size)
			html_undo_do_redo (undo, e);
	undo->level --;

	/* restore current undo/redo stack */
	stack_copy (&save, stack);

	/* end redo/undo step */
	if (dir == HTML_UNDO_UNDO)
		redo_level_end (undo);
	else
		html_undo_level_end (undo);
}

void
html_undo_data_init (HTMLUndoData   *data)
{
	data->ref_count = 1;
	data->destroy   = NULL;
}

void
html_undo_data_ref (HTMLUndoData *data)
{
	g_assert (data);

	data->ref_count ++;
}

void
html_undo_data_unref (HTMLUndoData *data)
{
	g_assert (data);
	g_assert (data->ref_count > 0);

	data->ref_count --;

	if (data->ref_count == 0) {
		if (data->destroy)
			(*data->destroy) (data);
		g_free (data);
	}
}

HTMLUndoDirection
html_undo_direction_reverse (HTMLUndoDirection dir)
{
	return dir == HTML_UNDO_UNDO ? HTML_UNDO_REDO : HTML_UNDO_UNDO;
}

static void
add_used_and_redo_to_undo (HTMLUndo *undo)
{
	GList *stack;
	GList *cur;

	stack            = g_list_reverse (undo->redo.stack);
	undo->redo.stack = NULL;
	undo->redo.size  = 0;

	/* add undo_used */
	for (cur = undo->undo_used.stack; cur; cur = cur->next)
		html_undo_add_undo_action (undo, HTML_UNDO_ACTION (cur->data));
	g_list_free (undo->undo_used.stack);
	undo->undo_used.stack = NULL;

	for (cur = stack; cur; cur = cur->next)
		html_undo_add_undo_action (undo, HTML_UNDO_ACTION (cur->data));
	g_list_free (stack);

#ifdef UNDO_DEBUG
	printf ("REVERSED\n");
	DEBUG (undo);
#endif
}

#ifdef UNDO_DEBUG

static void
print_stack (GList *stack, guint size, gint l)
{
	gint i;

	for (; stack; stack = stack->next) {
		HTMLUndoAction *action;
		for (i = 0; i < l; i++) printf ("  ");
		action = HTML_UNDO_ACTION (stack->data);
		printf ("%s\n", action->description);
		if (action->is_level) {
			HTMLUndoLevel *level;

			level = (HTMLUndoLevel *) action->data;

			print_stack (level->stack.stack, level->stack.size, l + 1);
		}
	}
}

static void
html_undo_debug (HTMLUndo *undo)
{
	printf ("Undo stack (%d):\n", undo->undo.size);
	print_stack (undo->undo.stack, undo->undo.size, 1);
	printf ("Redo stack (%d):\n", undo->redo.size);
	print_stack (undo->redo.stack, undo->redo.size, 1);
	printf ("Used stack (%d):\n", undo->undo_used.size);
	print_stack (undo->undo_used.stack, undo->undo_used.size, 1);
	printf ("--\n");
}

#endif
