/* -*- Mode: C; style: linux -*- */

/* capplet-widget.c
 * Copyright (C) 2000 Helix Code, Inc.
 * Copyright (C) 1998 Red Hat Software, Inc.
 *
 * Written by Bradford Hovinen (hovinen@helixcode.com),
 *            Jonathon Blandford (jrb@redhat.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include "capplet-widget.h"

static void capplet_widget_init       (CappletWidget *widget);
static void capplet_widget_class_init (CappletWidgetClass *klass);

static void capplet_widget_show       (GtkWidget *widget);

static void ok_cb                     (GtkWidget *button,
				       CappletWidget *widget);
static void cancel_cb                 (GtkWidget *menu_item,
				       CappletWidget *widget);

static void set_controls_sensitive    (CappletWidget *widget, 
				       gboolean s, 
				       gboolean set_undo);

enum {
        TRY_SIGNAL,
        REVERT_SIGNAL,
        OK_SIGNAL,
        CANCEL_SIGNAL,
        HELP_SIGNAL,
        NEW_MULTI_CAPPLET,
        PAGE_HIDDEN_SIGNAL,
        PAGE_SHOWN_SIGNAL,
        LAST_SIGNAL
};

static int capplet_widget_signals[LAST_SIGNAL] = {0,0,0,0,0,0,0,0};

static GtkFrameClass *parent_class;

static gchar* cc_ior = NULL;
static gint id = -1;
static guint xid = 0;
static gint capid = -1;
static gint cap_session_init = 0;
static gint cap_ignore = 0;
static gboolean do_get = FALSE, do_set = FALSE;

static struct poptOption cap_options[] = {
        {"id", '\0', POPT_ARG_INT, &id, 0, 
	 N_("id of the capplet -- assigned by the control-center"), N_("ID")},
        {"cap-id", '\0', POPT_ARG_INT, &capid, 0, 
	 N_("Multi-capplet id."), N_("CAPID")},
        {"xid", '\0', POPT_ARG_INT, &xid, 0, 
	 N_("X ID of the socket it's plugged into"), N_("XID")},
	{"ior", '\0', POPT_ARG_STRING, &cc_ior, 0, 
	 N_("IOR of the control-center"), N_("IOR")},
        {"init-session-settings", '\0', POPT_ARG_NONE, &cap_session_init, 0, 
	 N_("Initialize session settings"), NULL},
        {"ignore", '\0', POPT_ARG_NONE, &cap_ignore, 0, 
	 N_("Ignore default action.  Used for custom init-session cases"), 
	 NULL},
	{"get", 'g', POPT_ARG_NONE, &do_get, 0, 
	 N_("Get an XML description of the capplet's state"), N_("DO_GET")},
	{"set", 's', POPT_ARG_NONE, &do_set, 0, 
	 N_("Read an XML description of the capplet's state and apply it"), 
	 N_("DO_SET")},
        {NULL, '\0', 0, NULL, 0}
};

guint 
capplet_widget_get_type (void) 
{
	static guint capplet_widget_type = 0;

	if (!capplet_widget_type) {
		GtkTypeInfo capplet_widget_info = {
			"CappletWidget",
			sizeof (CappletWidget),
			sizeof (CappletWidgetClass),
			(GtkClassInitFunc) capplet_widget_class_init,
			(GtkObjectInitFunc) capplet_widget_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
                };

		capplet_widget_type = gtk_type_unique 
			(gtk_frame_get_type (), &capplet_widget_info);
	}
        
	return capplet_widget_type;
}

static void
capplet_widget_init (CappletWidget *widget) 
{
	gtk_frame_set_shadow_type (GTK_FRAME (widget), GTK_SHADOW_NONE);

	widget->dialog = GNOME_DIALOG 
		(gnome_dialog_new ("Capplet",
				   GNOME_STOCK_BUTTON_OK,
				   GNOME_STOCK_BUTTON_CANCEL,
				   NULL));

	gtk_window_set_policy (GTK_WINDOW (widget->dialog), TRUE, TRUE, TRUE);

	gnome_dialog_button_connect (widget->dialog, 0,
				     GTK_SIGNAL_FUNC (ok_cb), widget);
	gnome_dialog_button_connect (widget->dialog, 1,
				     GTK_SIGNAL_FUNC (cancel_cb), widget);

	gtk_signal_connect (GTK_OBJECT (widget->dialog), "destroy",
			    GTK_SIGNAL_FUNC (cancel_cb), widget);

	gtk_widget_show_all (widget->dialog->vbox);

	gtk_box_pack_start (GTK_BOX (widget->dialog->vbox), 
			    GTK_WIDGET (widget),
			    TRUE, TRUE, 0);

	set_controls_sensitive (widget, FALSE, TRUE);

	widget->capid = capid;
}

static void
capplet_widget_class_init (CappletWidgetClass *klass)
{
        GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

        object_class = GTK_OBJECT_CLASS (klass);
        widget_class = GTK_WIDGET_CLASS (klass);
        parent_class = gtk_type_class (gtk_frame_get_type ());

        capplet_widget_signals[TRY_SIGNAL] =
                gtk_signal_new("try",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 try),
                               gtk_marshal_NONE__NONE,
                               GTK_TYPE_NONE, 0);
        capplet_widget_signals[REVERT_SIGNAL] =
                gtk_signal_new("revert",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 revert),
                               gtk_marshal_NONE__NONE,
                               GTK_TYPE_NONE, 0);
        capplet_widget_signals[OK_SIGNAL] =
                gtk_signal_new("ok",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 ok),
                               gtk_marshal_NONE__NONE,
                               GTK_TYPE_NONE, 0);
        capplet_widget_signals[CANCEL_SIGNAL] =
                gtk_signal_new("cancel",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 cancel),
                               gtk_marshal_NONE__NONE,
                               GTK_TYPE_NONE, 0);
        capplet_widget_signals[HELP_SIGNAL] =
                gtk_signal_new("help",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 help),
                               gtk_marshal_NONE__NONE,
                               GTK_TYPE_NONE, 0);
        capplet_widget_signals[NEW_MULTI_CAPPLET] =
                gtk_signal_new("new_multi_capplet",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 new_multi_capplet),
                               gtk_marshal_NONE__POINTER,
                               GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
        capplet_widget_signals[PAGE_HIDDEN_SIGNAL] =
                gtk_signal_new("page_hidden",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 page_hidden),
                               gtk_marshal_NONE__NONE,
                               GTK_TYPE_NONE, 0);
        capplet_widget_signals[PAGE_SHOWN_SIGNAL] =
                gtk_signal_new("page_shown",
                               GTK_RUN_LAST,
                               object_class->type,
                               GTK_SIGNAL_OFFSET(CappletWidgetClass,
                                                 page_shown),
                               gtk_marshal_NONE__NONE,
                               GTK_TYPE_NONE, 0);

        gtk_object_class_add_signals (object_class, 
				      capplet_widget_signals, 
				      LAST_SIGNAL);

	object_class->destroy = capplet_widget_destroy;

        klass->try = NULL;
        klass->revert = NULL;
        klass->ok = NULL;
        klass->new_multi_capplet = NULL;

	widget_class->show = capplet_widget_show;
}

GtkWidget *
capplet_widget_new (void) 
{
	GtkWidget *widget;

	widget = gtk_type_new (capplet_widget_get_type ());

	/* Just display the whole thing here, because otherwise we'll
	 * have to play games with overriding the realize method
	 */
	gtk_widget_show_all (GTK_WIDGET (CAPPLET_WIDGET (widget)->dialog));

	return widget;
}

void
capplet_widget_destroy (GtkObject *object) 
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_CAPPLET_WIDGET (object));
	g_return_if_fail (CAPPLET_WIDGET (object)->dialog != NULL);
	g_return_if_fail (GNOME_IS_DIALOG (CAPPLET_WIDGET (object)->dialog));

	gtk_object_destroy (GTK_OBJECT (CAPPLET_WIDGET (object)->dialog));
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
capplet_widget_multi_new (gint capid) 
{
	GtkWidget *widget;

	widget = capplet_widget_new ();
	CAPPLET_WIDGET (widget)->capid = capid;
	return widget;
}

void
capplet_gtk_main (void) 
{
	gtk_main ();
}

/* returns 0 upon successful initialization.
   returns 1 if --init-session-settings was passed on the cmdline
   returns 2 if --ignore was passed on the cmdline
   returns 3 if --get was passed on the cmdline
   returns 4 if --set was passed on the cmdline
   returns -1 upon error
*/

gint 
gnome_capplet_init (const char *app_id, const char *app_version,
		    int argc, char **argv, struct poptOption *options,
		    unsigned int flags, poptContext *return_ctx) 
{
	gint retval;

        gnomelib_register_popt_table(cap_options, "capplet options");
	retval = gnome_init_with_popt_table (app_id, app_version, argc, argv,
					     options, flags, return_ctx);

	if (cap_session_init) return 1;
	else if (cap_ignore) return 2;
	else if (do_get) return 3;
	else if (do_set) return 4;
	else if (retval < 0) return -1;
	else return 0;
}

void
capplet_widget_state_changed (CappletWidget *cap, gboolean undoable) 
{
	g_return_if_fail (cap != NULL);
	g_return_if_fail (IS_CAPPLET_WIDGET (cap));

	set_controls_sensitive (cap, TRUE, undoable);
}

static void
capplet_widget_show (GtkWidget *widget) 
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_CAPPLET_WIDGET (widget));
	g_return_if_fail (CAPPLET_WIDGET (widget)->dialog != NULL);
	g_return_if_fail (GNOME_IS_DIALOG (CAPPLET_WIDGET (widget)->dialog));

	GTK_WIDGET_CLASS (parent_class)->show (widget);
	gtk_widget_show (GTK_WIDGET (CAPPLET_WIDGET (widget)->dialog));
}

static void
ok_cb (GtkWidget *button, CappletWidget *widget)
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_CAPPLET_WIDGET (widget));
	g_return_if_fail (CAPPLET_WIDGET (widget)->dialog != NULL);
	g_return_if_fail (GNOME_IS_DIALOG (CAPPLET_WIDGET (widget)->dialog));

	gtk_widget_hide (GTK_WIDGET (widget->dialog));
	gtk_signal_emit (GTK_OBJECT (widget),
			 capplet_widget_signals[OK_SIGNAL]);
	gtk_main_quit ();
}

static void
cancel_cb (GtkWidget *menu_item, CappletWidget *widget)
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_CAPPLET_WIDGET (widget));
	g_return_if_fail (CAPPLET_WIDGET (widget)->dialog != NULL);
	g_return_if_fail (GNOME_IS_DIALOG (CAPPLET_WIDGET (widget)->dialog));

	gtk_widget_hide (GTK_WIDGET (widget->dialog));
	gtk_signal_emit (GTK_OBJECT (widget),
			 capplet_widget_signals[CANCEL_SIGNAL]);
	gtk_main_quit ();
}

static void
set_controls_sensitive (CappletWidget *widget, gboolean s, gboolean set_undo) 
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_CAPPLET_WIDGET (widget));
	g_return_if_fail (CAPPLET_WIDGET (widget)->dialog != NULL);
	g_return_if_fail (GNOME_IS_DIALOG (CAPPLET_WIDGET (widget)->dialog));

	gnome_dialog_set_sensitive (widget->dialog, 0, s);
}

gint
capplet_widget_class_get_capid (void) 
{
	return capid;
}
