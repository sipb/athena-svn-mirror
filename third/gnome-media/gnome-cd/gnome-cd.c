/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * gnome-cd.c: GNOME-CD player.
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include <gnome.h>
#include <bonobo.h>

#include "gnome-cd.h"
#include "cddb.h"
#include "cdrom.h"
#include "display.h"
#include "callbacks.h"
#include "preferences.h"
#include "access/factory.h"

#define DEFAULT_THEME "lcd"

/* Debugging? */
gboolean debug_mode = FALSE;
static char *cd_option_device = NULL;

static gboolean cd_option_unique = FALSE;
static gboolean cd_option_play = FALSE;

void
gcd_warning (const char *message,
	     GError *error)
{
	if (debug_mode == FALSE) {
		return;
	}
	
	g_warning (message, error ? error->message : "(None)");
}

void
gnome_cd_set_window_title (GnomeCD *gcd,
			   const char *artist,
			   const char *track)
{
	char *title;

	if (artist == NULL ||
	    track == NULL) {
		title = g_strdup (_("CD Player"));
	} else {
		title = g_strconcat (track, " - ", artist, NULL);
	}

	gtk_window_set_title (GTK_WINDOW (gcd->window), title);
	g_free (title);
}

/* Can't be static because it ends up being referenced from another file */
void
skip_to_track (GtkWidget *item,
	       GnomeCD *gcd)
{
	int track, end_track;
	GnomeCDRomStatus *status = NULL;
	GnomeCDRomMSF msf, *endmsf;
	GError *error;
	
	track = gtk_option_menu_get_history (GTK_OPTION_MENU (gcd->tracks));

	if (gnome_cdrom_get_status (GNOME_CDROM (gcd->cdrom), &status, NULL) == FALSE) {
		g_free (status);
		return;
	}

	if (status->track - 1 == track && status->audio == GNOME_CDROM_AUDIO_PLAY) {
		g_free (status);
		return;
	}

	g_free (status);
	msf.minute = 0;
	msf.second = 0;
	msf.frame = 0;

	if (gcd->cdrom->playmode == GNOME_CDROM_SINGLE_TRACK) {
		end_track = track + 2;
		endmsf = &msf;
	} else {
		end_track = -1;
		endmsf = NULL;
	}
	
	if (gnome_cdrom_play (GNOME_CDROM (gcd->cdrom), track + 1, &msf, end_track, endmsf, &error) == FALSE) {
		gcd_warning ("Error skipping %s", error);
		g_error_free (error);
	}
}

void
gnome_cd_build_track_list_menu (GnomeCD *gcd)
{
	GtkMenu *menu;
	GtkWidget *item;

	/* Block the changed signal */
	g_signal_handlers_block_matched (G_OBJECT (gcd->tracks), G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
					 G_CALLBACK (skip_to_track), gcd);
	if (gcd->menu != NULL) {
		menu = GTK_MENU (gcd->menu);
		gtk_option_menu_remove_menu (GTK_OPTION_MENU (gcd->tracks));
	}

	menu = GTK_MENU(gtk_menu_new ());
	if (gcd->disc_info != NULL &&
	    gcd->disc_info->track_info) {
		int i;
		for (i = 0; i < gcd->disc_info->ntracks; i++) {
			char *title;
			CDDBSlaveClientTrackInfo *info;

			info = gcd->disc_info->track_info[i];
			if (info == NULL) {
				title = g_strdup_printf ("%d - %s", i + 1,
							 _("Unknown track"));
			} else {
				title = g_strdup_printf ("%d - %s", i + 1,
							 info->name);
			}
			
			item = gtk_menu_item_new_with_label (title);
			g_free (title);
			gtk_widget_show (item);

			gtk_menu_shell_append (GTK_MENU_SHELL(menu), item);
		}
	} else {
		GnomeCDRomCDDBData *data;
		GnomeCDRomStatus *status;

		if (gnome_cdrom_get_status (gcd->cdrom, &status, NULL) != FALSE) {

			if (status->cd == GNOME_CDROM_STATUS_OK) {
				if (gnome_cdrom_get_cddb_data (gcd->cdrom, &data, NULL) != FALSE) {
					int i;

					if (data != NULL) {
						for (i = 0; i < data->ntrks; i++) {
							char *label;
							
							label = g_strdup_printf (_("%d - Unknown"), i + 1);
							item = gtk_menu_item_new_with_label (label);
							g_free (label);
							
							gtk_widget_show (item);
							
							gtk_menu_shell_append ((GtkMenuShell*)(menu), item);
						}
						
						g_free (data);
					}
				}
			}

		}
		g_free (status);
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (gcd->tracks), GTK_WIDGET (menu));
	gcd->menu = GTK_WIDGET(menu);

	g_signal_handlers_unblock_matched (G_OBJECT (gcd->tracks), G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
					   G_CALLBACK (skip_to_track), gcd);
}

static GtkWidget *
make_button_from_widget (GnomeCD *gcd,
			 GtkWidget *widget,
			 GCallback func,
			 const char *tooltip,
			 const char *shortname)
{
	GtkWidget *button;
	AtkObject *aob;

	button = gtk_button_new ();
	gtk_container_add (GTK_CONTAINER (button), widget);
/*  	gtk_container_set_border_width (GTK_CONTAINER (button), 2); */
	
	if (func) {
		g_signal_connect (G_OBJECT (button), "clicked",
				  G_CALLBACK (func), gcd);
	}
	gtk_tooltips_set_tip (gcd->tooltips, button, tooltip, "");

	aob = gtk_widget_get_accessible (button);
	atk_object_set_name (aob, shortname);
	return button;
}

static GtkWidget *
make_button_from_pixbuf (GnomeCD *gcd,
			 GdkPixbuf *pixbuf,
			 GCallback func,
			 const char *tooltip,
			 const char *shortname)
{
	GtkWidget *pixmap;

	g_return_val_if_fail (gcd != NULL, NULL);
	g_return_val_if_fail (pixbuf != NULL, NULL);
		
	pixmap = gtk_image_new_from_pixbuf (pixbuf);

	return make_button_from_widget (gcd, pixmap, func, tooltip, shortname);
}

static GtkWidget *
make_button_from_stock (GnomeCD *gcd,
			const char *stock,
			GCallback func,
			const char *tooltip, 
			const char *shortname)
{
	GtkWidget *pixmap;

	pixmap = gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);

	return make_button_from_widget (gcd, pixmap, func, tooltip, shortname);
}

static GdkPixbuf *
pixbuf_from_file (const char *filename)
{
	GdkPixbuf *pixbuf;
	char *fullname;

	fullname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, 
                   filename, TRUE, NULL);
	g_return_val_if_fail (fullname != NULL, NULL);

	pixbuf = gdk_pixbuf_new_from_file (fullname, NULL);
	g_free (fullname);

	return pixbuf;
}

static void
window_destroy_cb (GtkWidget *window,
		   gpointer data)
{
	GnomeCD *gcd = data;

	/* Before killing the cdrom object, do the shutdown on it */
	switch (gcd->preferences->stop) {
	case GNOME_CD_PREFERENCES_STOP_NOTHING:
		break;

	case GNOME_CD_PREFERENCES_STOP_STOP:
		gnome_cdrom_stop (gcd->cdrom, NULL);
		break;

	case GNOME_CD_PREFERENCES_STOP_OPEN:
		gnome_cdrom_eject (gcd->cdrom, NULL);
		break;
		
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	case GNOME_CD_PREFERENCES_STOP_CLOSE:
		gnome_cdrom_close_tray (gcd->cdrom, NULL);
		break;
#endif

	default:
		g_assert_not_reached ();
		break;
	}

	/* Unref the cddb slave */
	cddb_close_client ();

	/* And the track editor */
	destroy_track_editor ();
	
	g_object_unref (gcd->cdrom);
	bonobo_main_quit ();
}

struct _MenuItem {
	char *name;
	char *icon;
	GCallback callback;
};

struct _MenuItem menuitems[] = {
	{N_("P_revious track"), GNOME_CD_PREVIOUS, G_CALLBACK (back_cb)},
	{N_("_Stop"), GNOME_CD_STOP, G_CALLBACK (stop_cb)},
	{N_("_Play / Pause"), GNOME_CD_PLAY, G_CALLBACK (play_cb)},
	{N_("_Next track"), GNOME_CD_NEXT, G_CALLBACK (next_cb)},
	{N_("_Eject disc"), GNOME_CD_EJECT, G_CALLBACK (eject_cb)},
	{N_("_Help"), GTK_STOCK_HELP, G_CALLBACK (help_cb)},
	{N_("_About CD player"), GNOME_STOCK_ABOUT, G_CALLBACK (about_cb)},
	{NULL, NULL, NULL}
};

static GtkWidget *
make_popup_menu (GnomeCD *gcd)
{
	GtkWidget *menu;
	int i;

	menu = gtk_menu_new ();
	for (i = 0; menuitems[i].name != NULL; i++) {
		GtkWidget *item, *image;

		item = gtk_image_menu_item_new_with_mnemonic (_(menuitems[i].name));
		if (menuitems[i].icon != NULL) {
			char *ext = strrchr (menuitems[i].icon, '.');

			if (ext == NULL) {
				image = gtk_image_new_from_stock (menuitems[i].icon,
								  GTK_ICON_SIZE_MENU);
			} else {
				char *fullname;

				fullname = gnome_program_locate_file (NULL, 
				GNOME_FILE_DOMAIN_PIXMAP, menuitems[i].icon,
                                TRUE, NULL);
				if (fullname != NULL) {
					image = gtk_image_new_from_file (fullname);
				} else {
					image = NULL;
				}
				
				g_free (fullname);
			}

			if (image != NULL) {
				gtk_widget_show (image);
				gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(item), image);
			}
		}
				
		gtk_widget_show (item);

		gtk_menu_shell_append ((GtkMenuShell *)(menu), item);
		if (menuitems[i].callback != NULL) {
			g_signal_connect (G_OBJECT (item), "activate",
					  G_CALLBACK (menuitems[i].callback), gcd);
		}
	}

	return menu;
}

static void
show_error (GtkWidget	*dialog,
	    GError 	*error,
	    GError	*detailed_error,
	    GnomeCD 	*gcd)
{
	switch (gtk_dialog_run (GTK_DIALOG (dialog))) {
	case 1:
		gtk_label_set_text (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
				    detailed_error->message);

		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), 1, FALSE);
		show_error (dialog, error, detailed_error, gcd);

		break;

	case 2:
		gtk_widget_destroy (dialog);
		dialog = GTK_WIDGET(preferences_dialog_show (gcd, TRUE));

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		break;

	default:
		exit (0);
	}
}

static GnomeCD *
init_player (const char *device_override)
{
	GnomeCD *gcd;
	GnomeCDRomStatus *status;
	GtkWidget *display_box;
	GtkWidget *top_hbox, *button_hbox, *option_hbox;
	GtkWidget *button;
	GdkPixbuf *pixbuf;
	GtkWidget *box;
	GError *error = NULL;
	GError *detailed_error = NULL;

	gcd = g_new0 (GnomeCD, 1);

	gcd->not_ready = TRUE;
	gcd->preferences = (GnomeCDPreferences *)preferences_new (gcd);
	gcd->device_override = g_strdup (device_override);
	
	if (gcd->preferences->device == NULL || gcd->preferences->device[0] == 0) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_NONE,
						 _("There is no CD device set. This means that the CD player\n"
						   "will be unable to run. Click 'Set device' to go to a dialog\n"
						   "where you can set the device, or click 'Quit' to quit the CD player."));
		gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_QUIT, GTK_RESPONSE_CLOSE,
					_("Set device"), 1, NULL);
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), 1);
		gtk_window_set_title (GTK_WINDOW (dialog), _("No CD device"));
		
		switch (gtk_dialog_run (GTK_DIALOG (dialog))) {
		case 1:
			gtk_widget_destroy (dialog);
			dialog = GTK_WIDGET(preferences_dialog_show (gcd, TRUE));

			/* Don't care what it returns */
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);

			break;

		default:
			exit (0);
		}
	}

 nodevice:		
	gcd->cdrom = gnome_cdrom_new (gcd->device_override ? gcd->device_override : gcd->preferences->device, 
				      GNOME_CDROM_UPDATE_CONTINOUS, &error);

	if (gcd->cdrom == NULL) {

		if (error != NULL) {
			gcd_warning ("%s", error);
		}
		if (gcd->device_override) {
			g_free (gcd->device_override);
			gcd->device_override = NULL;

			g_error_free (error);
			error = NULL;

			goto nodevice;
		}
	} else {
		g_signal_connect (G_OBJECT (gcd->cdrom), "status-changed",
				  G_CALLBACK (cd_status_changed_cb), gcd);
	}

	if (error != NULL) {
		GtkWidget *dialog;

		detailed_error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_OPENED,
					      "The CD player is unable to run correctly.\n\n"
					      "%s\n"
					      "Press 'Set device' to go to a dialog"
					      " where you can set the device, or press 'Quit' to quit the CD player", error->message ? error->message : " ");

		
		dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_NONE,
						 _("The CD player is unable to run correctly.\n\n"
						 "Press 'Details' for more details on reasons for the failure.\n\n" 
						 "Press 'Set device' to go to a dialog"
						 " where you can set the device, or press 'Quit' to quit the CD player"));
		gtk_dialog_add_buttons (GTK_DIALOG (dialog), _("_Details"), 1, GTK_STOCK_QUIT, GTK_RESPONSE_CLOSE,
					_("_Set device"), 2, NULL);
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), 2);
		gtk_window_set_title (GTK_WINDOW (dialog), _("Invalid CD device"));
		show_error (dialog, error, detailed_error, gcd);

		g_error_free (error);
		g_error_free (detailed_error);
		error = NULL;
		detailed_error = NULL;

		goto nodevice;
	}
		
	gcd->cd_selection = cd_selection_start (gcd->device_override ?
						gcd->device_override : gcd->preferences->device);
	gcd->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (gcd->window), _("CD Player"));
	gtk_window_set_role (GTK_WINDOW (gcd->window), "gnome-cd-main-window");
	gtk_window_set_default_size (GTK_WINDOW (gcd->window), 350, 129);
	
	pixbuf = pixbuf_from_file ("gnome-cd/cd.png");
	if (pixbuf == NULL) {
		g_warning ("Error finding gnome-cd/cd.png");
	} else {
		gtk_window_set_icon (GTK_WINDOW (gcd->window), pixbuf);
		g_object_unref (G_OBJECT (pixbuf));
	}

	g_signal_connect (G_OBJECT (gcd->window), "destroy",
			  G_CALLBACK (window_destroy_cb), gcd);
	gcd->vbox = gtk_vbox_new (FALSE, 1);
	gcd->tooltips = gtk_tooltips_new ();
	top_hbox = gtk_hbox_new (FALSE, 1);

	/* Create the display */
	display_box = gtk_vbox_new (FALSE, 1);

	gcd->display = GTK_WIDGET (cd_display_new ());
	GTK_WIDGET_SET_FLAGS (gcd->display, GTK_CAN_FOCUS | GTK_CAN_DEFAULT);
	g_signal_connect (G_OBJECT (gcd->display), "loopmode-changed",
			  G_CALLBACK (loopmode_changed_cb), gcd);
	g_signal_connect (G_OBJECT (gcd->display), "playmode-changed",
			  G_CALLBACK (playmode_changed_cb), gcd);

	/* Theme needs to be loaded after the display is created */
	gcd->theme = (GCDTheme *)theme_load (gcd, gcd->preferences->theme_name);
	if (gcd->theme == NULL) {
		g_error ("Could not create theme");
	}
	
	gnome_popup_menu_attach (make_popup_menu (gcd), gcd->display, NULL);
	
	gtk_box_pack_start (GTK_BOX (display_box), gcd->display, TRUE, TRUE, 0);


	gtk_box_pack_start (GTK_BOX (top_hbox), display_box, TRUE, TRUE, 0);

	/* Volume slider */
	gcd->slider = gtk_vscale_new_with_range (-255.0, 0.0, 1.0);
	gtk_scale_set_draw_value (GTK_SCALE (gcd->slider), FALSE);
	gtk_box_pack_start (GTK_BOX (top_hbox), gcd->slider, FALSE, FALSE, 0);
	
	gtk_tooltips_set_tip (gcd->tooltips, GTK_WIDGET (gcd->slider),
			      _("Volume control"), NULL);

	gtk_box_pack_start (GTK_BOX (gcd->vbox), top_hbox, TRUE, TRUE, 0);

	option_hbox = gtk_hbox_new (FALSE, 2);

	/* Create app controls */
	button = make_button_from_stock (gcd, GTK_STOCK_INDEX,
					 G_CALLBACK(open_track_editor),
					 _("Open track editor"),
					 _("Track editor"));
	gtk_widget_set_sensitive (button, FALSE);
	gtk_box_pack_start (GTK_BOX (option_hbox), button, FALSE, FALSE, 0);
	gcd->trackeditor_b = button;

	button = make_button_from_stock (gcd, GTK_STOCK_PREFERENCES,
					 G_CALLBACK(open_preferences),
					 _("Open preferences"),
					 _("Preferences"));
	gtk_box_pack_start (GTK_BOX (option_hbox), button, FALSE, FALSE, 0);
	gcd->properties_b = button;

	gcd->tracks = gtk_option_menu_new ();
	g_signal_connect (G_OBJECT (gcd->tracks), "changed",
			  G_CALLBACK (skip_to_track), gcd);
	gtk_tooltips_set_tip (gcd->tooltips, GTK_WIDGET (gcd->tracks), 
			      _("Track List"), NULL );
	gnome_cd_build_track_list_menu (gcd);
	gtk_box_pack_start (GTK_BOX (option_hbox), gcd->tracks, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (gcd->vbox), option_hbox, FALSE, FALSE, 0);
	
	/* Get the initial volume */
	if (gnome_cdrom_get_status (gcd->cdrom, &status, NULL) == TRUE) {
		gtk_range_set_value (GTK_RANGE (gcd->slider),
				     (double) status->volume);
		g_free (status);
	} else {
		g_free (status);
		gcd_warning ("Error getting status: %s", NULL);
	}

	g_signal_connect (G_OBJECT (gcd->slider), "value-changed",
			  G_CALLBACK (volume_changed), gcd);
	
	button_hbox = gtk_hbox_new (TRUE, 2);
	
  	button = make_button_from_stock (gcd, GNOME_CD_PREVIOUS, G_CALLBACK (back_cb), _("Previous track"), _("Previous"));
	gtk_box_pack_start (GTK_BOX (button_hbox), button, TRUE, TRUE, 0);
	gcd->back_b = button;

	button = make_button_from_stock (gcd, GNOME_CD_REWIND, NULL, _("Rewind"), _("Rewind"));
	g_signal_connect (G_OBJECT (button), "button-press-event",
			  G_CALLBACK (rewind_press_cb), gcd);
	g_signal_connect (G_OBJECT (button), "button-release-event",
			  G_CALLBACK (rewind_release_cb), gcd);
	gtk_box_pack_start (GTK_BOX (button_hbox), button, TRUE, TRUE, 0);
	gcd->rewind_b = button;

	/* Create the play and pause images, and ref them so they never
	   get destroyed */
	gcd->play_image = gtk_image_new_from_stock (GNOME_CD_PLAY, GTK_ICON_SIZE_BUTTON);
	g_object_ref (gcd->play_image);

	gcd->pause_image = gtk_image_new_from_stock (GNOME_CD_PAUSE, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (gcd->pause_image);
	g_object_ref (gcd->pause_image);

	button = make_button_from_widget (gcd, gcd->play_image, G_CALLBACK (play_cb), _("Play / Pause"), _("Play"));
	gtk_box_pack_start (GTK_BOX (button_hbox), button, TRUE, TRUE, 0);
	gcd->play_b = button;
	gcd->current_image = gcd->play_image;
	
	button = make_button_from_stock (gcd, GNOME_CD_STOP, G_CALLBACK (stop_cb), _("Stop"), _("Stop"));
	gtk_box_pack_start (GTK_BOX (button_hbox), button, TRUE, TRUE, 0);
	gcd->stop_b = button;

	button = make_button_from_stock (gcd, GNOME_CD_FFWD, NULL, _("Fast forward"), _("Fast forward"));
	g_signal_connect (G_OBJECT (button), "button-press-event",
			  G_CALLBACK (ffwd_press_cb), gcd);
	g_signal_connect (G_OBJECT (button), "button-release-event",
			  G_CALLBACK (ffwd_release_cb), gcd);
	gtk_box_pack_start (GTK_BOX (button_hbox), button, TRUE, TRUE, 0);
	gcd->ffwd_b = button;

	button = make_button_from_stock (gcd, GNOME_CD_NEXT, G_CALLBACK (next_cb), _("Next track"), _("Next track"));
	gtk_box_pack_start (GTK_BOX (button_hbox), button, TRUE, TRUE, 0);
	gcd->next_b = button;
	
	button = make_button_from_stock (gcd, GNOME_CD_EJECT, G_CALLBACK (eject_cb), _("Eject CD"), _("Eject"));
	gtk_box_pack_start (GTK_BOX (button_hbox), button, TRUE, TRUE, 0);
	gcd->eject_b = button;

	gtk_box_pack_start (GTK_BOX (gcd->vbox), button_hbox, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (gcd->window), gcd->vbox);
	gtk_widget_show_all (gcd->vbox);

	gcd->not_ready = FALSE;

	/* Tray icon */
	gcd->tray = GTK_WIDGET (egg_tray_icon_new ("GnomeCD Tray Icon"));
	box = gtk_event_box_new ();
	g_signal_connect (G_OBJECT (box), "button_press_event",
			 	G_CALLBACK (tray_icon_clicked), gcd);
	gtk_container_add (GTK_CONTAINER (gcd->tray), box);
	pixbuf = gdk_pixbuf_scale_simple (pixbuf_from_file ("gnome-cd/cd.png"),
				16, 16, GDK_INTERP_BILINEAR);
	gcd->tray_icon = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (box), gcd->tray_icon);
	gcd->tray_tips = gtk_tooltips_new ();
	gtk_tooltips_set_tip (gcd->tray_tips, gcd->tray, _("CD Player"), NULL);
	gnome_popup_menu_attach (make_popup_menu (gcd), box, NULL);
	gtk_widget_show_all (gcd->tray);

	return gcd;
}

static int 
save_session(GnomeClient        *client,
             gint                phase,
             GnomeRestartStyle   save_style,
             gint                shutdown,
             GnomeInteractStyle  interact_style,
             gint                fast,
             gpointer            client_data) 
{

    gchar *argv[]= { NULL };

    argv[0] = (gchar*) client_data;
    gnome_client_set_clone_command (client, 1, argv);
    gnome_client_set_restart_command (client, 1, argv);

    return TRUE;
}

static gint client_die(GnomeClient *client,
		       GnomeCD *gcd)
{
	gtk_widget_destroy (gcd->window);
	gtk_main_quit ();
}

static const char *items [] =
{
	GNOME_CD_PLAY,
	GNOME_CD_STOP,
	GNOME_CD_PAUSE,
	GNOME_CD_PREVIOUS,
	GNOME_CD_NEXT,
	GNOME_CD_FFWD,
	GNOME_CD_REWIND,
	GNOME_CD_EJECT
};

static void
register_stock_icons (void)
{
	GtkIconFactory *factory;
	int i;

	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	for (i = 0; i < (int) G_N_ELEMENTS (items); i++) {
		GtkIconSet *icon_set;
		GdkPixbuf *pixbuf;
		char *filename, *fullname;

		filename = g_strconcat ("gnome-cd/", items[i], ".png", NULL);
		fullname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, filename, TRUE, NULL);
		g_free (filename);

		pixbuf = gdk_pixbuf_new_from_file (fullname, NULL);
		g_free (fullname);

		icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
		gtk_icon_factory_add (factory, items[i], icon_set);
		gtk_icon_set_unref (icon_set);

		g_object_unref (G_OBJECT (pixbuf));
	}

	g_object_unref (G_OBJECT (factory));
}

int 
main (int argc, char *argv[])
{
	static const struct poptOption cd_popt_options [] = {
		{ "device", '\0', POPT_ARG_STRING, &cd_option_device, 0,
		  N_("CD device to use"), NULL },
		{ "unique", '\0', POPT_ARG_NONE, &cd_option_unique, 0,
		  N_("Only start if there isn't already a CD player application running"), NULL },
		{ "play",   '\0', POPT_ARG_NONE, &cd_option_play, 0,
		  N_("Play the CD on startup"), NULL },
		{ NULL, '\0', 0, NULL, 0 },
	};

	GnomeCD *gcd;
	CDSelection *cd_selection;
	GnomeClient *client;

	free (malloc (8)); /* -lefence */

	if (g_getenv ("GNOME_CD_DEBUG")) {
		debug_mode = TRUE;
	}
	
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init ("gnome-cd", VERSION, LIBGNOMEUI_MODULE, 
			    argc, argv, 
			    GNOME_PARAM_POPT_TABLE, cd_popt_options,
			    GNOME_PARAM_APP_DATADIR, DATADIR, NULL);

	register_stock_icons ();
	client = gnome_master_client ();
    	g_signal_connect (client, "save_yourself",
                         G_CALLBACK (save_session), (gpointer) argv[0]);

	gcd = init_player (cd_option_device);
	if (gcd == NULL) {
		/* Stick a message box here? */
		g_error (_("Cannot create player"));
		exit (0);
	}

	gtk_widget_show_all (gcd->window);
	g_signal_connect (client, "die",
			  G_CALLBACK (client_die), gcd);

	/* Do the start up stuff */
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	if (gcd->preferences->start_close) {
		gnome_cdrom_close_tray (gcd->cdrom, NULL);
	}
#endif

	if (cd_option_play) {
		/* Just fake a click on the button */
		play_cb (NULL, gcd);
	} else {
	switch (gcd->preferences->start) {
	case GNOME_CD_PREFERENCES_START_NOTHING:
		break;

	case GNOME_CD_PREFERENCES_START_START:
		/* Just fake a click on the button */
		play_cb (NULL, gcd);
		break;
		
	case GNOME_CD_PREFERENCES_START_STOP:
		gnome_cdrom_stop (gcd->cdrom, NULL);
		break;

	default:
		break;
	}
	}
	
	if (cd_option_unique &&
	    !cd_selection_is_master (gcd->cd_selection))
		return 0;

	setup_a11y_factory ();

	bonobo_main ();
	return 0;
}
