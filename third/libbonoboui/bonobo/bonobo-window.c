/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-win.c: The Bonobo Window implementation.
 *
 * Author:
 *	Michael Meeks (michael@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

#include <bonobo/bonobo-dock-item.h>
#include <bonobo/bonobo-dock.h>
#include <bonobo/bonobo-window.h>
#include <libbonobo.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-macros.h>

#include <bonobo/bonobo-ui-preferences.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-sync-menu.h>
#include <bonobo/bonobo-ui-sync-keys.h>
#include <bonobo/bonobo-ui-sync-status.h>
#include <bonobo/bonobo-ui-sync-toolbar.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

GNOME_CLASS_BOILERPLATE (BonoboWindow, bonobo_window,
			 GtkWindow, GTK_TYPE_WINDOW);

struct _BonoboWindowPrivate {
	BonoboUIEngine *engine;

	BonoboUISync   *sync_menu;
	BonoboUISync   *sync_keys;
	BonoboUISync   *sync_status;
	BonoboUISync   *sync_toolbar;
	
	BonoboDock     *dock;

	BonoboDockItem *menu_item;
	GtkMenuBar     *menu;

	GtkAccelGroup  *accel_group;

	char           *name;		/* Win name */
	char           *prefix;		/* Win prefix */

	GtkBox         *status;

	gboolean        allow_all_focus;
};

enum {
	PROP_0,
	PROP_WIN_NAME
};

/**
 * bonobo_window_remove_popup:
 * @win: the window
 * @path: the path
 * 
 * Remove the popup at @path
 **/
void
bonobo_window_remove_popup (BonoboWindow     *win,
			    const char    *path)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_sync_menu_remove_popup (
		BONOBO_UI_SYNC_MENU (win->priv->sync_menu), path);
}

/**
 * bonobo_window_add_popup:
 * @win: the window
 * @menu: the menu widget
 * @path: the path
 * 
 * Add a popup @menu at @path
 **/
void
bonobo_window_add_popup (BonoboWindow *win,
			 GtkMenu      *menu,
			 const char   *path)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_sync_menu_add_popup (
		BONOBO_UI_SYNC_MENU (win->priv->sync_menu), menu, path);
}

/**
 * bonobo_window_set_contents:
 * @win: the bonobo window
 * @contents: the new widget for it to contain.
 * 
 * Insert a widget into the main window contents.
 **/
void
bonobo_window_set_contents (BonoboWindow *win,
			    GtkWidget    *contents)
{
	g_return_if_fail (win != NULL);
	g_return_if_fail (win->priv != NULL);

	bonobo_dock_set_client_area (win->priv->dock, contents);
}

/**
 * bonobo_window_get_contents:
 * @win: the bonobo window
 * 
 * Return value: the contained widget
 **/
GtkWidget *
bonobo_window_get_contents (BonoboWindow *win)
{
	g_return_val_if_fail (win != NULL, NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);
	g_return_val_if_fail (win->priv->dock != NULL, NULL);

	return bonobo_dock_get_client_area (win->priv->dock);
}

static void
bonobo_window_dispose (GObject *object)
{
	BonoboWindow *win = (BonoboWindow *)object;
	
	if (win->priv->engine) {
		bonobo_ui_engine_dispose (win->priv->engine);
		g_object_unref (win->priv->engine);
		win->priv->engine = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
bonobo_window_finalize (GObject *object)
{
	BonoboWindow *win = (BonoboWindow *)object;
	
	g_free (win->priv->name);
	g_free (win->priv->prefix);
	g_free (win->priv);

	win->priv = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * bonobo_window_get_accel_group:
 * @win: the bonobo window
 * 
 * Return value: the associated accelerator group for this window
 **/
GtkAccelGroup *
bonobo_window_get_accel_group (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	return win->priv->accel_group;
}

static BonoboWindowPrivate *
construct_priv (BonoboWindow *win)
{
	GtkWidget *main_vbox;
	BonoboWindowPrivate *priv;
	BonoboDockItemBehavior behavior;

	priv = g_new0 (BonoboWindowPrivate, 1);

	priv->engine = bonobo_ui_engine_new (G_OBJECT (win));

	main_vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), main_vbox);

	priv->dock = BONOBO_DOCK (bonobo_dock_new ());
	gtk_box_pack_start (GTK_BOX (main_vbox),
			    GTK_WIDGET (priv->dock),
			    TRUE, TRUE, 0);

	behavior = (BONOBO_DOCK_ITEM_BEH_EXCLUSIVE
		    | BONOBO_DOCK_ITEM_BEH_NEVER_VERTICAL);
	if (!bonobo_ui_preferences_get_menubar_detachable ())
		behavior |= BONOBO_DOCK_ITEM_BEH_LOCKED;

	priv->menu_item = BONOBO_DOCK_ITEM (bonobo_dock_item_new (
		"menu", behavior));
	priv->menu      = GTK_MENU_BAR (gtk_menu_bar_new ());
	gtk_container_add (GTK_CONTAINER (priv->menu_item),
			   GTK_WIDGET    (priv->menu));
	bonobo_dock_add_item (priv->dock, priv->menu_item,
			     BONOBO_DOCK_TOP, 0, 0, 0, TRUE);

	priv->status = GTK_BOX (gtk_hbox_new (FALSE, 0));
	gtk_box_pack_end (GTK_BOX (main_vbox),
			  GTK_WIDGET (priv->status),
			  FALSE, FALSE, 0);

	priv->accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (win),
				    priv->accel_group);

	gtk_widget_show_all (GTK_WIDGET (main_vbox));
	gtk_widget_hide (GTK_WIDGET (priv->status));

	priv->sync_menu = bonobo_ui_sync_menu_new (
		priv->engine, priv->menu,
		GTK_WIDGET (priv->menu_item),
		priv->accel_group);

	bonobo_ui_engine_add_sync (priv->engine, priv->sync_menu);


	priv->sync_toolbar = bonobo_ui_sync_toolbar_new (
		priv->engine, BONOBO_DOCK (priv->dock));

	bonobo_ui_engine_add_sync (priv->engine, priv->sync_toolbar);

	/* Keybindings; the gtk_binding stuff is just too evil */
	priv->sync_keys = bonobo_ui_sync_keys_new (priv->engine);
	bonobo_ui_engine_add_sync (priv->engine, priv->sync_keys);

	priv->sync_status = bonobo_ui_sync_status_new (
		priv->engine, priv->status);
	bonobo_ui_engine_add_sync (priv->engine, priv->sync_status);

	return priv;
}

/*
 *   To kill bug reports of hiding not working
 * we want to stop show_all showing hidden menus etc.
 */
static void
bonobo_window_show_all (GtkWidget *widget)
{
	GtkWidget *client;
	BonoboWindow *win = BONOBO_WINDOW (widget);

	if (win->priv->dock &&
	    (client = bonobo_dock_get_client_area (win->priv->dock)))
		gtk_widget_show_all (client);

	gtk_widget_show (widget);
}

static gboolean
bonobo_window_key_press_event (GtkWidget *widget,
			       GdkEventKey *event)
{
	gboolean handled;
	BonoboUISyncKeys *sync;
	BonoboWindow *window = (BonoboWindow *) widget;

	if (event->state & GDK_CONTROL_MASK) {
		window->priv->allow_all_focus = TRUE;

		if (event->keyval == GDK_F10)
			bonobo_dock_focus_roll (window->priv->dock);
	}

	handled = GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event);
	if (handled)
		return TRUE;

	sync = BONOBO_UI_SYNC_KEYS (window->priv->sync_keys);
	if (sync)
		return bonobo_ui_sync_keys_binding_handle (widget, event, sync);

	return FALSE;
}

static gboolean
bonobo_window_key_release_event (GtkWidget *widget,
				 GdkEventKey *event)
{
	BonoboWindow *window = (BonoboWindow *) widget;

	if (event->keyval == GDK_Control_L ||
	    event->keyval == GDK_Control_R)
		window->priv->allow_all_focus = FALSE;

	return GTK_WIDGET_CLASS (parent_class)->key_release_event (widget, event);
}

static gboolean
bonobo_window_focus (GtkWidget        *widget,
		     GtkDirectionType  direction)
{
  GtkWindow *window;
  GtkContainer *container;
  GtkWidget *old_focus_child;
  GtkWidget *parent;
  GtkWidget *child;
  BonoboWindow *win = (BonoboWindow *) widget;

  if (win->priv->allow_all_focus)
	  return GTK_WIDGET_CLASS (parent_class)->focus (widget, direction);

  container = GTK_CONTAINER (widget);
  window = GTK_WINDOW (widget);

  old_focus_child = container->focus_child;
  child = win->priv->dock ? bonobo_dock_get_client_area (win->priv->dock) : NULL;
  
  /* We need a special implementation here to deal properly with wrapping
   * around in the tab chain without the danger of going into an
   * infinite loop.
   */
  if (old_focus_child)
    {
      if (gtk_widget_child_focus (old_focus_child, direction))
	return TRUE;
    }

  if (window->focus_widget)
    {
      /* Wrapped off the end, clear the focus setting for the toplpevel */
      parent = window->focus_widget->parent;
      while (parent)
	{
	  gtk_container_set_focus_child (GTK_CONTAINER (parent), NULL);
	  parent = GTK_WIDGET (parent)->parent;
	}
      
      gtk_window_set_focus (GTK_WINDOW (container), NULL);
    }

  /* Now try to focus the first widget in the window */
  if (child)
    {
      if (gtk_widget_child_focus (child, direction))
        return TRUE;
    }

  return FALSE;
}

static void
bonobo_window_set_property (GObject         *object,
			guint            prop_id,
			 const GValue    *value,
			GParamSpec      *pspec)
{
	BonoboWindow *window;

	window = BONOBO_WINDOW (object);

	switch (prop_id)
	{
	case PROP_WIN_NAME:
		bonobo_window_set_name (window, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
bonobo_window_get_property (GObject         *object,
			guint            prop_id,
			GValue          *value,
			GParamSpec      *pspec)
{
	BonoboWindow *window;

	window = BONOBO_WINDOW (object);

	switch (prop_id)
	{
	case PROP_WIN_NAME:
		g_value_set_string (value, window->priv->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
bonobo_window_class_init (BonoboWindowClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

	gobject_class->dispose  = bonobo_window_dispose;
	gobject_class->finalize = bonobo_window_finalize;
	gobject_class->set_property = bonobo_window_set_property;
	gobject_class->get_property = bonobo_window_get_property;

	widget_class->focus = bonobo_window_focus;
	widget_class->show_all = bonobo_window_show_all;
	widget_class->key_press_event = bonobo_window_key_press_event;
	widget_class->key_release_event = bonobo_window_key_release_event;

	/* Properties: */
	g_object_class_install_property (gobject_class,
		PROP_WIN_NAME,
		g_param_spec_string ("win_name",
		_("Name"),
		_("Name of the window - used for configuration serialization."),
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
bonobo_window_instance_init (BonoboWindow *win)
{
	BonoboUIContainer *ui_container = 0;
	
	win->priv = construct_priv (win);
	
	/* Create UIContainer: */
	ui_container = bonobo_ui_container_new ();
	bonobo_ui_container_set_engine (ui_container, win->priv->engine);
	bonobo_object_unref (BONOBO_OBJECT (ui_container));
}

/**
 * bonobo_window_set_name:
 * @win: the bonobo window
 * @win_name: the window name
 * 
 * Set the name of the window - used for configuration
 * serialization.
 **/
void
bonobo_window_set_name (BonoboWindow  *win,
			const char *win_name)
{
	BonoboWindowPrivate *priv;

	g_return_if_fail (BONOBO_IS_WINDOW (win));

	priv = win->priv;

	g_free (priv->name);
	g_free (priv->prefix);

	if (win_name) {
		priv->name = g_strdup (win_name);
		priv->prefix = g_strconcat ("/", win_name, "/", NULL);
	} else {
		priv->name = NULL;
		priv->prefix = g_strdup ("/");
	}
}

/**
 * bonobo_window_get_name:
 * @win: the bonobo window
 * 
 * Return value: the name of the window
 **/
char *
bonobo_window_get_name (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);

	if (win->priv->name)
		return g_strdup (win->priv->name);
	else
		return NULL;
}

/**
 * bonobo_window_get_ui_engine:
 * @win: the bonobo window
 * 
 * Return value: the #BonoboUIEngine
 **/
BonoboUIEngine *
bonobo_window_get_ui_engine (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);

	return win->priv->engine;
}

/**
 * bonobo_window_get_ui_container:
 * @win: the bonobo window
 * 
 * Return value: the #BonoboUIContainer
 **/
BonoboUIContainer *
bonobo_window_get_ui_container (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);

	return bonobo_ui_engine_get_ui_container (win->priv->engine);
}

/**
 * bonobo_window_construct:
 * @win: the window to construct
 * @iu_container: the UI container
 * @win_name: the window name
 * @title: the window's title for the title bar
 *
 * Don't use this ever - use construct time properties instead.
 * TODO: Remove this when we are allowed API changes.
 *
 * Return value: a constructed window
 **/
GtkWidget *
bonobo_window_construct (BonoboWindow      *win,
			 BonoboUIContainer *ui_container,
			 const char        *win_name,
			 const char        *title)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
	g_return_val_if_fail (BONOBO_IS_UI_CONTAINER (ui_container), NULL);

	bonobo_window_set_name (win, win_name);

	bonobo_ui_container_set_engine (ui_container, win->priv->engine);

	bonobo_object_unref (BONOBO_OBJECT (ui_container));

	if (title)
		gtk_window_set_title (GTK_WINDOW (win), title);

	return GTK_WIDGET (win);
}

/**
 * bonobo_window_new:
 * @win_name: the window name
 * @title: the window's title for the title bar
 * 
 * Return value: a new BonoboWindow
 **/
GtkWidget *
bonobo_window_new (const char *win_name,
		   const char *title)
{
	BonoboWindow *win = g_object_new (BONOBO_TYPE_WINDOW, "win_name", win_name, "title", title, NULL);

	return GTK_WIDGET (win);
}


