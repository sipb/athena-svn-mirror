/* 
 * Copyright (C) 2003 Red Hat, Inc.
 * Copyright (C) 2003 Novell, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>

#include <eel/eel-stock-dialogs.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libnautilus-extension/nautilus-menu-provider.h>

#define NAUTILUS_TYPE_BURN  (nautilus_burn_get_type ())
#define NAUTILUS_BURN(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), NAUTILUS_TYPE_BURN, NautilusBurn))

typedef struct 
{
	GObject parent_slot;
} NautilusBurn;

typedef struct 
{
	GObjectClass parent_slot;
} NautilusBurnClass;

static GType nautilus_burn_get_type      (void);
static void  nautilus_burn_register_type (GTypeModule *module);

static GObjectClass *parent_class;

static void
launch_process (char **argv, GtkWindow *parent)
{
	GError *error;

	error = NULL;
	if (!g_spawn_async (NULL,
			    argv, NULL,
			    0,
			    NULL, NULL,
			    NULL,
			    &error)) {

		eel_show_error_dialog (_("Unable to launch the cd burner application"),
				       error->message,
				       _("Can't launch cd burner"),
				       GTK_WINDOW (parent));
		g_error_free (error);
	}
}

static void
write_activate_cb (NautilusMenuItem *item,
		   gpointer user_data)
{
	char *argv[] = {BINDIR "/nautilus-cd-burner", NULL};
	
	launch_process (argv, GTK_WINDOW (user_data));
}

static void
write_iso_activate_cb (NautilusMenuItem *item,
		       gpointer user_data)
{
	NautilusFileInfo *file;
	GtkWidget *window;
	char *argv[3];
	char *uri;
	char *image_name;

	file = g_object_get_data (G_OBJECT (item), "file");
	window = g_object_get_data (G_OBJECT (item), "window");

	uri = nautilus_file_info_get_uri (file);
	image_name = gnome_vfs_get_local_path_from_uri (uri);
	if (!image_name) {
		g_warning ("Can not get local path for URI %s", uri);
		g_free (uri);
		return;
	}
	
	g_free (uri);

	argv[0] = BINDIR "/nautilus-cd-burner";
	argv[1] = image_name;
	argv[2] = NULL;

	launch_process (argv, GTK_WINDOW (user_data));
	
	g_free (image_name);
	
}

static GList *
nautilus_burn_get_file_items (NautilusMenuProvider *provider,
			      GtkWidget *window,
			      GList *selection)
{
	NautilusMenuItem *item;
	NautilusFileInfo *file;
	char *mime_type;
	char *scheme;
	
	if (!selection || selection->next != NULL) {
		return NULL;
	}
	
	file = NAUTILUS_FILE_INFO (selection->data);

	scheme = nautilus_file_info_get_uri_scheme (file); 
	if (strcmp (scheme, "file") != 0) {
		g_free (scheme);
		return NULL;
	}
	g_free (scheme);
	
	mime_type = nautilus_file_info_get_mime_type (file);
	if (strcmp (mime_type, "application/x-iso-image") != 0 &&
	    strcmp (mime_type, "application/x-cd-image") != 0) {
		g_free (mime_type);
		return NULL;
	}
	g_free (mime_type);
	
	item = nautilus_menu_item_new ("NautilusBurn::write_iso",
				       _("_Write to Disc..."),
				       _("Write disc image to a CD or DVD disc"),
				       "gnome-dev-cdrom");
	g_object_set_data (G_OBJECT (item), "file", file);
	g_object_set_data (G_OBJECT (item), "window", window);
	g_signal_connect (item, "activate", 
			  G_CALLBACK (write_iso_activate_cb), NULL);

	return g_list_append (NULL, item);
}

static GList *
nautilus_burn_get_background_items (NautilusMenuProvider *provider,
				    GtkWidget *window,
				    NautilusFileInfo *current_folder)
{
	GList *items;
	char *scheme;
	
	items = NULL;

	scheme = nautilus_file_info_get_uri_scheme (current_folder);

	if (!strcmp (scheme, "burn")) {
		NautilusMenuItem *item;
		
		item = nautilus_menu_item_new ("NautilusBurn::write_menu",
					       _("_Write to Disc..."),
					       _("Write contents to a CD or DVD disc"),
					       "gnome-dev-cdrom");
		g_signal_connect (item, "activate", 
				  G_CALLBACK (write_activate_cb), window);
		items = g_list_append (items, item);
	}

	g_free (scheme);

	return items;
}

static GList *
nautilus_burn_get_toolbar_items (NautilusMenuProvider *provider,
				 GtkWidget *window,
				 NautilusFileInfo *current_folder)
{
	GList *items;
	char *scheme;

	items = NULL;

	scheme = nautilus_file_info_get_uri_scheme (current_folder);

	if (!strcmp (scheme, "burn")) {
		NautilusMenuItem *item;
		
		item = nautilus_menu_item_new ("NautilusBurn::write_toolbar",
					       _("Write to Disc"),
					       _("Write contents to a CD or DVD disc"),
					       "gnome-dev-cdrom");
		g_object_set (item, "priority", TRUE, NULL);
		g_signal_connect (item, "activate", 
				  G_CALLBACK (write_activate_cb), window);
		
		items = g_list_append (items, item);
	}

	g_free (scheme);

	return items;
}

static void
nautilus_burn_menu_provider_iface_init (NautilusMenuProviderIface *iface)
{
	iface->get_file_items = nautilus_burn_get_file_items;
	iface->get_background_items = nautilus_burn_get_background_items;
	iface->get_toolbar_items = nautilus_burn_get_toolbar_items;
}

static void 
nautilus_burn_instance_init (NautilusBurn *cvs)
{
}

static void
nautilus_burn_class_init (NautilusBurnClass *class)
{
	parent_class = g_type_class_peek_parent (class);
}

static GType burn_type = 0;

static GType 
nautilus_burn_get_type (void)
{
	return burn_type;
}

static void
nautilus_burn_register_type (GTypeModule *module)
{
	static const GTypeInfo info = {
		sizeof (NautilusBurnClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) nautilus_burn_class_init,
		NULL, 
		NULL,
		sizeof (NautilusBurn),
		0,
		(GInstanceInitFunc) nautilus_burn_instance_init,
	};

	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) nautilus_burn_menu_provider_iface_init,
		NULL,
		NULL
	};

	burn_type = g_type_module_register_type (module,
						 G_TYPE_OBJECT,
						 "NautilusBurn",
						 &info, 0);
	
	g_type_module_add_interface (module,
				     burn_type,
				     NAUTILUS_TYPE_MENU_PROVIDER,
				     &menu_provider_iface_info);
}

void
nautilus_module_initialize (GTypeModule *module)
{
	nautilus_burn_register_type (module);
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

void
nautilus_module_shutdown (void)
{
}

void
nautilus_module_list_types (const GType **types,
			    int *num_types)
{
	static GType type_list[1];
	
	type_list[0] = NAUTILUS_TYPE_BURN;
	
	*types = type_list;
	*num_types = 1;
}

