/*
 * Copyright (C) 2003 Sun Microsystems, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.

 * Authors:
 *      Mark McLoughlin <mark@skynet.ie>
 */

#include <config.h>

#include "netstatus-applet.h"

#include <string.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <libgnomeui/gnome-about.h>
#include <gconf/gconf-client.h>
#include <gdk/gdkkeysyms.h>
#include <libgnomeui/gnome-help.h>

#include "netstatus-icon.h"
#include "netstatus-iface.h"
#include "netstatus-dialog.h"

struct _NetstatusAppletPrivate
{
  NetstatusIface *iface;

  GtkWidget      *icon;
  
  GtkWidget      *status_dialog;
  GtkWidget      *about_dialog;
  
  GConfClient    *client;
  guint           notify_id;
};

static void     netstatus_applet_instance_init             (NetstatusApplet      *applet,
							    NetstatusAppletClass *klass);
static void     netstatus_applet_class_init                (NetstatusAppletClass *klass);
static void     netstatus_applet_finalize                  (GObject              *object);
static gboolean netstatus_applet_key_press_event           (GtkWidget            *widget,
							    GdkEventKey          *event);
static void     netstatus_applet_orientation_changed       (NetstatusApplet      *applet,
							    PanelAppletOrient     orient);
static void     netstatus_applet_display_help              (BonoboUIComponent    *uic,
							    NetstatusApplet      *applet);
static void     netstatus_applet_display_about_dialog      (BonoboUIComponent    *uic,
							    NetstatusApplet      *applet);
static void     netstatus_applet_display_properties_dialog (BonoboUIComponent    *uic,
							    NetstatusApplet      *applet);
static void     netstatus_applet_display_status_dialog     (NetstatusApplet      *applet);
static void     netstatus_applet_iface_name_changed        (NetstatusApplet      *applet);

static GObjectClass *parent_class;

static const BonoboUIVerb netstatus_menu_verbs [] =
  {
    BONOBO_UI_UNSAFE_VERB ("NetstatusProperties", netstatus_applet_display_properties_dialog),
    BONOBO_UI_UNSAFE_VERB ("NetstatusHelp", netstatus_applet_display_help),
    BONOBO_UI_UNSAFE_VERB ("NetstatusAbout", netstatus_applet_display_about_dialog),
    
    BONOBO_UI_VERB_END
  };

GType
netstatus_applet_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo info =
      {
	sizeof (NetstatusAppletClass),
	NULL,
	NULL,
	(GClassInitFunc) netstatus_applet_class_init,
	NULL,
	NULL,
	sizeof (NetstatusApplet),
	0,
	(GInstanceInitFunc) netstatus_applet_instance_init,
	NULL
      };

      type = g_type_register_static (PANEL_TYPE_APPLET, "NetstatusApplet", &info, 0);

      netstatus_setup_debug_flags ();
    }

  return type;
}

static void
netstatus_applet_instance_init (NetstatusApplet      *applet,
				NetstatusAppletClass *klass)
{
  applet->priv = g_new0 (NetstatusAppletPrivate, 1);

  applet->priv->client = gconf_client_get_default ();

  applet->priv->iface = netstatus_iface_new (NULL);
  applet->priv->icon  = netstatus_icon_new (applet->priv->iface);

  g_signal_connect_swapped (applet->priv->iface, "notify::name",
			    G_CALLBACK (netstatus_applet_iface_name_changed),
			    applet);

  g_signal_connect_swapped (applet->priv->icon, "invoked",
			    G_CALLBACK (netstatus_applet_display_status_dialog),
			    applet);

  gtk_container_add (GTK_CONTAINER (applet), applet->priv->icon);
  gtk_widget_show (applet->priv->icon);

  gtk_widget_show (GTK_WIDGET (applet));

  g_signal_connect (applet, "change-orient",
		    G_CALLBACK (netstatus_applet_orientation_changed), NULL);
}

static void
netstatus_applet_class_init (NetstatusAppletClass *klass)
{
  GObjectClass *gobject_class  = (GObjectClass *) klass;
  GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = netstatus_applet_finalize;

  widget_class->key_press_event = netstatus_applet_key_press_event;
}

static void
netstatus_applet_finalize (GObject *object)
{
  NetstatusApplet *applet = (NetstatusApplet *) object;

  if (applet->priv->notify_id)
    gconf_client_notify_remove (applet->priv->client, applet->priv->notify_id);
  applet->priv->notify_id = 0;

  if (applet->priv->client)
    g_object_unref (applet->priv->client);
  applet->priv->client = NULL;

  if (applet->priv->status_dialog)
    gtk_widget_destroy (applet->priv->status_dialog);
  applet->priv->status_dialog = NULL;

  if (applet->priv->about_dialog)
    gtk_widget_destroy (applet->priv->about_dialog);
  applet->priv->about_dialog = NULL;

  if (applet->priv->iface)
    g_object_unref (applet->priv->iface);
  applet->priv->iface = NULL;

  g_free (applet->priv);
  applet->priv = NULL;

  parent_class->finalize (object);
}

static gboolean
netstatus_applet_key_press_event (GtkWidget   *widget,
				  GdkEventKey *event)
{
  NetstatusApplet *applet = (NetstatusApplet *) widget;

  switch (event->keyval)
    {
    case GDK_space:
    case GDK_KP_Space:
    case GDK_Return:
    case GDK_KP_Enter:
    case GDK_ISO_Enter:
    case GDK_3270_Enter:
      netstatus_icon_invoke (NETSTATUS_ICON (applet->priv->icon));
      return TRUE;
    default:
      return FALSE;
    }
}

static void
netstatus_applet_display_properties_dialog (BonoboUIComponent *uic,
					    NetstatusApplet   *applet)
{
  netstatus_applet_display_status_dialog (applet);
}

static void
netstatus_applet_display_status_dialog (NetstatusApplet *applet)
{
  if (applet->priv->status_dialog)
    {
      gtk_window_set_screen (GTK_WINDOW (applet->priv->status_dialog),
			     gtk_widget_get_screen (GTK_WIDGET (applet)));
      gtk_window_present (GTK_WINDOW (applet->priv->status_dialog));
      return;
    }

  applet->priv->status_dialog = netstatus_dialog_new (applet->priv->iface);

  gtk_window_set_screen (GTK_WINDOW (applet->priv->status_dialog),
			 gtk_widget_get_screen (GTK_WIDGET (applet)));

  g_signal_connect (applet->priv->status_dialog,
		    "destroy",
		    G_CALLBACK (gtk_widget_destroyed),
		    &applet->priv->status_dialog);

  gtk_widget_show (GTK_WIDGET (applet->priv->status_dialog));
}

static void
netstatus_applet_display_help (BonoboUIComponent *uic,
			       NetstatusApplet   *applet)
{
  GError *error = NULL;
  
  gnome_help_display_on_screen ("gnome-netstatus", NULL,
				gtk_widget_get_screen (GTK_WIDGET (applet)),
				&error);
  if (error)
    {
      GtkWidget *message_dialog;
      message_dialog = gtk_message_dialog_new (NULL,
					       0,
					       GTK_MESSAGE_ERROR,
					       GTK_BUTTONS_OK,
					       _("There was an error displaying help: %s"),
					       error->message);
      g_error_free (error);
      gtk_window_set_screen (GTK_WINDOW (message_dialog),
			     gtk_widget_get_screen (GTK_WIDGET (applet)));
      gtk_dialog_run (GTK_DIALOG (message_dialog));
      gtk_widget_destroy (message_dialog);
    }
}

static void
netstatus_applet_display_about_dialog (BonoboUIComponent *uic,
				       NetstatusApplet   *applet)
{
  static const char *authors [] =
    {
      "Mark McLoughlin <mark@skynet.ie>",
      "Erwann Chenede <erwann.chenede@sun.com>",
      "Calum Benson <calum.benson@sun.com>",
      NULL
    };
  const char *documenters [] = { NULL };
  const char *translator_credits = _("translator_credits");

  GdkPixbuf *pixbuf = NULL;
  char      *file;

  if (applet->priv->about_dialog)
    {
      gtk_window_set_screen (GTK_WINDOW (applet->priv->about_dialog),
			     gtk_widget_get_screen (GTK_WIDGET (applet)));
      gtk_window_present (GTK_WINDOW (applet->priv->about_dialog));
      return;
    }
  
  file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
				    "gnome-netstatus-tx.png", TRUE, NULL);
  pixbuf = gdk_pixbuf_new_from_file (file, NULL);
  g_free (file);

  applet->priv->about_dialog =
    gnome_about_new (_("Network Monitor"), VERSION,
		     "Copyright \xc2\xa9 2003 Sun Microsystems, Inc.\n",
		     _("The Network Monitor displays the status of a network device."),
		     authors,
		     documenters,
		     strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
		     pixbuf);

  gtk_window_set_wmclass (GTK_WINDOW (applet->priv->about_dialog), "netstatus", "Netstatus");
  gtk_window_set_screen (GTK_WINDOW (applet->priv->about_dialog),
			 gtk_widget_get_screen (GTK_WIDGET (applet)));

  if (pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (applet->priv->about_dialog), pixbuf);
      g_object_unref (pixbuf);
    }

  g_signal_connect (applet->priv->about_dialog,
		    "destroy",
		    G_CALLBACK (gtk_widget_destroyed),
		    &applet->priv->about_dialog);
  gtk_widget_show (applet->priv->about_dialog);
}

static void
netstatus_applet_orientation_changed (NetstatusApplet   *applet,
				      PanelAppletOrient  orient)
{
  GtkOrientation orientation;

  if (orient == PANEL_APPLET_ORIENT_UP ||
      orient == PANEL_APPLET_ORIENT_DOWN)
    orientation = GTK_ORIENTATION_HORIZONTAL;
  else
    orientation = GTK_ORIENTATION_VERTICAL;

  netstatus_icon_set_orientation (NETSTATUS_ICON (applet->priv->icon),
				  orientation);
}

static void
netstatus_applet_iface_name_changed (NetstatusApplet *applet)
{
  const char *iface_name;

  iface_name = netstatus_iface_get_name (applet->priv->iface);
  if (iface_name)
    {
      panel_applet_gconf_set_string (PANEL_APPLET (applet),
				     "interface",
				     iface_name,
				     NULL);
    }
}

static void
netstatus_applet_iface_pref_changed (GConfClient     *client,
				     guint            cnxn_id,
				     GConfEntry      *entry,
				     NetstatusApplet *applet)
{
  if (entry->value && entry->value->type == GCONF_VALUE_STRING)
    {
      const char *iface_name;

      iface_name = gconf_value_get_string (entry->value);
      netstatus_iface_set_name (applet->priv->iface, iface_name);
    }
}

static gboolean
netstatus_applet_create (NetstatusApplet *applet,
			 const char      *iid)
{
  char *iface_name;
  char *key;

  if (strcmp (iid, "OAFIID:GNOME_NetstatusApplet") != 0)
    return FALSE;

  panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);

  panel_applet_setup_menu_from_file (PANEL_APPLET (applet), NULL,
				     "GNOME_NetstatusApplet.xml",
				     NULL, netstatus_menu_verbs, applet);

  panel_applet_add_preferences (PANEL_APPLET (applet),
				"/schemas/apps/netstatus_applet/prefs",
				NULL);

  iface_name = panel_applet_gconf_get_string (PANEL_APPLET (applet),
					      "interface",
					      NULL);
  if (!iface_name || !iface_name [0])
    {
      GError *error = NULL;
      GList  *iface_names;

      g_free (iface_name);
      iface_name = NULL;
      
      iface_names = netstatus_list_interface_names (&error);
      if (iface_names)
	{
	  GList *l;

	  /* FIXME: instead of picking the first one we should
	   *        figure out what interfaces aren't currently
	   *        displayed in an applet.
	   */
	  iface_name = iface_names->data;

	  for (l = iface_names->next; l; l = l->next)
	    g_free (l->data);
	  g_list_free (iface_names);
	}
      else
	{
	  g_assert (error != NULL);
	  g_object_set (G_OBJECT (applet->priv->iface),
			"error", error,
			NULL);
	  g_error_free (error);
	}
    }

  netstatus_iface_set_name (applet->priv->iface, iface_name);
  g_free (iface_name);

  key = panel_applet_gconf_get_full_key (PANEL_APPLET (applet), "interface");
  applet->priv->notify_id =
    gconf_client_notify_add (applet->priv->client,
			     key,
			     (GConfClientNotifyFunc) netstatus_applet_iface_pref_changed,
			     applet,
			     NULL,
			     NULL);
  g_free (key);

  return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_NetstatusApplet_Factory",
			     NETSTATUS_TYPE_APPLET,
			     "gnome-netstatus",
			     VERSION,
			     (PanelAppletFactoryCallback) netstatus_applet_create,
			     NULL)
