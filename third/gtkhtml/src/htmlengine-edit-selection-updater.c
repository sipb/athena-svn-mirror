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

/* WARNING / FIXME : All of this code assumes that the only kind of
   selectable object that does not accept the cursor is
   `HTMLClueFlow'.  */


#include <config.h>
#include <gtk/gtkmain.h>
#include "htmlengine-edit-selection-updater.h"
#include "htmlinterval.h"
#include "htmlselection.h"

struct _HTMLEngineEditSelectionUpdater {
	HTMLEngine *engine;
	gint idle_id;
};


/**
 * html_engine_edit_selection_updater_new:
 * @engine: A GtkHTML engine object.
 * 
 * Create a new updater associated with @engine.
 * 
 * Return value: A pointer to the new updater object.
 **/
HTMLEngineEditSelectionUpdater *
html_engine_edit_selection_updater_new (HTMLEngine *engine)
{
	HTMLEngineEditSelectionUpdater *new;

	g_return_val_if_fail (engine != NULL, NULL);
	g_return_val_if_fail (HTML_IS_ENGINE (engine), NULL);

	new = g_new (HTMLEngineEditSelectionUpdater, 1);

	new->engine  = engine;
	new->idle_id = 0;

	return new;
}

/**
 * html_engine_edit_selection_updater_destroy:
 * @updater: An HTMLEngineEditSelectionUpdater object.
 * 
 * Destroy @updater.
 **/
void
html_engine_edit_selection_updater_destroy (HTMLEngineEditSelectionUpdater *updater)
{
	g_return_if_fail (updater != NULL);

	if (updater->idle_id != 0)
		gtk_idle_remove (updater->idle_id);

	g_free (updater);
}


static gint
updater_idle_callback (gpointer data)
{
	HTMLEngineEditSelectionUpdater *updater;
	HTMLEngine *engine;

	updater = (HTMLEngineEditSelectionUpdater *) data;
	engine = updater->engine;

	if (engine->mark && html_cursor_get_position (engine->mark) != html_cursor_get_position (engine->cursor))
		html_engine_select_interval (engine, html_interval_new_from_cursor (engine->mark, engine->cursor));
	else {
		gboolean selection_mode = engine->selection_mode;

		html_engine_disable_selection (engine);
		engine->selection_mode = selection_mode;
	}

	updater->idle_id = 0;
	return FALSE;
}

/**
 * html_engine_edit_selection_updater_schedule:
 * @updater: An HTMLEngineEditSelectionUpdater object.
 * 
 * Schedule an update for the keyboard-selected region on @updater.
 **/
void
html_engine_edit_selection_updater_schedule (HTMLEngineEditSelectionUpdater *updater)
{
	g_return_if_fail (updater != NULL);

	if (updater->idle_id != 0)
		return;

	updater->idle_id = gtk_idle_add (updater_idle_callback, updater);
}

/**
 * html_engine_edit_selection_updater_reset:
 * @updater: An HTMLEngineEditSelectionUpdater object.
 * 
 * Reset @updater so after no selection is active anymore.
 **/
void
html_engine_edit_selection_updater_reset (HTMLEngineEditSelectionUpdater *updater)
{
	g_return_if_fail (updater != NULL);

	if (updater->idle_id != 0) {
		gtk_idle_remove (updater->idle_id);
		updater->idle_id = 0;
	}
}

/**
 * html_engine_edit_selection_updater_update_now:
 * @updater: An HTMLEngineEditSelectionUpdater object.
 * 
 * Remove @updater idle callback and run's update callback immediately.
 **/
void
html_engine_edit_selection_updater_update_now (HTMLEngineEditSelectionUpdater *updater)
{
	/* remove scheduled idle cb */
	if (updater->idle_id != 0) {
		gtk_idle_remove (updater->idle_id);
		updater->idle_id = 0;
	}

	/* run it now */
	updater_idle_callback (updater);
}

void
html_engine_edit_selection_updater_do_idle (HTMLEngineEditSelectionUpdater *updater)
{
	/* remove scheduled idle cb */
	if (updater->idle_id != 0) {
		gtk_idle_remove (updater->idle_id);
		updater->idle_id = 0;

		/* run it now */
		updater_idle_callback (updater);
	}
}
