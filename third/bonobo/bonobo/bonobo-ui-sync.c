/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * bonobo-ui-sync.h: An abstract base for Bonobo XML / widget sync sync'ing.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include <config.h>
#include <stdlib.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-sync.h>

#define PARENT_TYPE gtk_object_get_type ()
#define CLASS(o) BONOBO_UI_SYNC_CLASS (GTK_OBJECT (o)->klass)

static void
impl_sync_state_placeholder (BonoboUISync     *sync,
			     BonoboUINode     *node,
			     BonoboUINode     *cmd_node,
			     GtkWidget        *widget,
			     GtkWidget        *parent)
{
	gboolean show = FALSE;
	char    *txt;
		
	if ((txt = bonobo_ui_engine_get_attr (
		node, cmd_node, "delimit"))) {

		show = !strcmp (txt, "top");
		bonobo_ui_node_free_string (txt);
	}
		
	if (show)
		gtk_widget_show (widget);	
	else
		gtk_widget_hide (widget);
}

static void
class_init (BonoboUISyncClass *sync_class)
{
	sync_class->sync_state_placeholder = impl_sync_state_placeholder;
}

GtkType
bonobo_ui_sync_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUISync",
			sizeof (BonoboUISync),
			sizeof (BonoboUISyncClass),
			(GtkClassInitFunc)  class_init,
			(GtkObjectInitFunc) NULL,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

BonoboUISync *
bonobo_ui_sync_construct (BonoboUISync   *sync,
			  BonoboUIEngine *engine,
			  gboolean        is_recursive,
			  gboolean        has_widgets)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	sync->engine = engine;
	sync->is_recursive = is_recursive;
	sync->has_widgets  = has_widgets;

	return sync;
}

gboolean
bonobo_ui_sync_is_recursive (BonoboUISync *sync)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), FALSE);

	return sync->is_recursive;
}

gboolean
bonobo_ui_sync_has_widgets (BonoboUISync *sync)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), FALSE);

	return sync->has_widgets;
}

void
bonobo_ui_sync_state (BonoboUISync     *sync,
		      BonoboUINode     *node,
		      BonoboUINode     *cmd_node,
		      GtkWidget        *widget,
		      GtkWidget        *parent)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	return CLASS (sync)->sync_state (
		sync, node, cmd_node, widget, parent);
}

void
bonobo_ui_sync_state_placeholder (BonoboUISync     *sync,
				  BonoboUINode     *node,
				  BonoboUINode     *cmd_node,
				  GtkWidget        *widget,
				  GtkWidget        *parent)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	return CLASS (sync)->sync_state_placeholder (
		sync, node, cmd_node, widget, parent);
}

GtkWidget *
bonobo_ui_sync_build (BonoboUISync     *sync,
		      BonoboUINode     *node,
		      BonoboUINode     *cmd_node,
		      int              *pos,
		      GtkWidget        *parent)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	return CLASS (sync)->build (sync, node, cmd_node, pos, parent);
}

GtkWidget *
bonobo_ui_sync_build_placeholder (BonoboUISync     *sync,
				  BonoboUINode     *node,
				  BonoboUINode     *cmd_node,
				  int              *pos,
				  GtkWidget        *parent)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	return CLASS (sync)->build_placeholder (
		sync, node, cmd_node, pos, parent);
}

GList *
bonobo_ui_sync_get_widgets (BonoboUISync *sync,
			    BonoboUINode *node)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	if (CLASS (sync)->get_widgets)
		return CLASS (sync)->get_widgets (sync, node);
	else
		return NULL;
}

void
bonobo_ui_sync_state_update (BonoboUISync     *sync,
			     GtkWidget        *widget,
			     const char       *new_state)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	CLASS (sync)->state_update (sync, widget, new_state);
}

void
bonobo_ui_sync_remove_root (BonoboUISync *sync,
			    BonoboUINode *root)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	if (CLASS (sync)->remove_root)
		CLASS (sync)->remove_root (sync, root);
}

void
bonobo_ui_sync_update_root (BonoboUISync *sync,
			    BonoboUINode *root)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	if (CLASS (sync)->update_root)
		CLASS (sync)->update_root (sync, root);
}

gboolean
bonobo_ui_sync_ignore_widget (BonoboUISync *sync,
			      GtkWidget    *widget)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), FALSE);

	if (CLASS (sync)->ignore_widget)
		return CLASS (sync)->ignore_widget (sync, widget);
	else
		return FALSE;
}

void
bonobo_ui_sync_stamp_root (BonoboUISync *sync)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	if (CLASS (sync)->stamp_root)
		CLASS (sync)->stamp_root (sync);
}

gboolean
bonobo_ui_sync_can_handle (BonoboUISync *sync,
			   BonoboUINode *node)
{
	if (CLASS (sync)->can_handle)
		return CLASS (sync)->can_handle (sync, node);
	else
		return FALSE;
}

/*
 *   For some widgets such as menus, the submenu widget
 * is attached to the actual container in a strange way
 * this works around only having single inheritance.
 */
GtkWidget *
bonobo_ui_sync_get_attached (BonoboUISync *sync,
			     GtkWidget    *widget,
			     BonoboUINode *node)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	if (CLASS (sync)->get_attached)
		return CLASS (sync)->get_attached (sync, widget, node);
	else
		return NULL;
}

gboolean
bonobo_ui_sync_do_show_hide (BonoboUISync *sync,
			     BonoboUINode *node,
			     BonoboUINode *cmd_node,
			     GtkWidget    *widget)
{
	char      *txt;
	gboolean   changed;
	GtkWidget *attached;

	if (sync && 
	    (attached = bonobo_ui_sync_get_attached (
		    sync, widget, node)))

		widget = attached;

	if (!widget)
		return FALSE;

	if ((txt = bonobo_ui_engine_get_attr (node, cmd_node, "hidden"))) {
		if (atoi (txt)) {
			changed = GTK_WIDGET_VISIBLE (widget);
			gtk_widget_hide (widget);
		} else {
			changed = !GTK_WIDGET_VISIBLE (widget);
			gtk_widget_show (widget);
		}
		bonobo_ui_node_free_string (txt);
	} else {
		changed = !GTK_WIDGET_VISIBLE (widget);
		gtk_widget_show (widget);
	}

	return changed;
}
