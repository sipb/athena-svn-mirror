/* ACME
 * Copyright (C) 2001 Bastien Nocera <hadess@hadess.net>
 *
 * acme.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 * USA.
 */

#include <config.h>

/* system headers */
#include <sys/file.h>
#include <sys/stat.h>
/* X11 headers */
#include <X11/X.h>

#ifdef HAVE_XFREE
#include <X11/XF86keysym.h>
#endif
#ifdef HAVE_XSUN
#include <X11/Sunkeysym.h>
#endif

/* Gnome headers */
#include <gdk/gdkx.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <libwnck/libwnck.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include "eggtrayicon.h"

#include "acme.h"
#include "g-volume.h"
#ifdef USE_FBLEVEL
#include "fb-level.h"
#endif

typedef struct {
	GVolume *volobj;
#ifdef USE_FBLEVEL
	FBLevel *levobj;
#endif
	GladeXML *xml;
	GtkWidget *dialog;
	int NumLockMask, CapsLockMask, ScrollLockMask;
	GConfClient *conf_client;
	guint dialog_timeout;
	WnckScreen *screen;

	/* Multihead stuff */
	GdkDisplay *display;
	GdkScreen *current_screen;
	GList *screens;

	/* Tray icon */
	EggTrayIcon *tray_icon;
	GtkTooltips *tray_icon_tooltip;
	GtkWidget *popup_menu;
} Acme;

enum {
	ICON_MUTED,
	ICON_LOUD,
	ICON_BRIGHT,
	ICON_EJECT,
};

static void init_tray (Acme *acme);

static void
selection_get_func (GtkClipboard *clipboard, GtkSelectionData *selection_data,
		guint info, gpointer user_data_or_owner)
{
}

static void
selection_clear_func (GtkClipboard *clipboard, gpointer user_data_or_owner)
{       
	return;
}

#define SELECTION_NAME "_ACME_SELECTION"

static gboolean
acme_get_lock (Acme *acme)
{
	gboolean result = FALSE;
	GtkClipboard *clipboard;
	Atom clipboard_atom = gdk_x11_get_xatom_by_name (SELECTION_NAME);
	static const GtkTargetEntry targets[] = {
		{ SELECTION_NAME, 0, 0 }
	};

	XGrabServer (GDK_DISPLAY());

	if (XGetSelectionOwner (GDK_DISPLAY(), clipboard_atom) != None)
		goto out;

	clipboard = gtk_clipboard_get (gdk_atom_intern (SELECTION_NAME, FALSE));

	if (!gtk_clipboard_set_with_data  (clipboard, targets,
				G_N_ELEMENTS (targets),
				selection_get_func,
				selection_clear_func, NULL))
		goto out;

	result = TRUE;

out:
	XUngrabServer (GDK_DISPLAY());
	gdk_flush();

	return result;
}

static void
acme_exit (Acme *acme)
{
	exit (0);
}

static void
acme_error (char * msg)
{
	GtkWidget *error_dialog;

	error_dialog =
	    gtk_message_dialog_new (NULL,
			    GTK_DIALOG_MODAL,
			    GTK_MESSAGE_ERROR,
			    GTK_BUTTONS_OK,
			    "%s", msg);
	gtk_dialog_set_default_response (GTK_DIALOG (error_dialog),
			GTK_RESPONSE_OK);
	gtk_widget_show (error_dialog);
	gtk_dialog_run (GTK_DIALOG (error_dialog));
	gtk_widget_destroy (error_dialog);
}

static void
execute (char *cmd, gboolean sync)
{
	gboolean retval;

	if (sync == TRUE)
		retval = g_spawn_command_line_sync
			(cmd, NULL, NULL, NULL, NULL);
	else
		retval = g_spawn_command_line_async (cmd, NULL);

	if (retval == FALSE)
	{
		char *msg;

		msg = g_strdup_printf
			(_("Couldn't execute command: %s\n"
			   "Verify that this command exists."),
			 cmd);

		acme_error (msg);
		g_free (msg);
	}
}

static void
execute_this_or_that (char *cmd1, char *cmd2)
{
	if (g_spawn_command_line_async (cmd1, NULL) == FALSE)
	{
		if (g_spawn_command_line_async (cmd2, NULL) == FALSE)
		{
			char *msg;

			msg = g_strdup_printf
				(_("Couldn't execute either command: %s\n"
				   "or command: %s\n"
				   "Verify that at least one of these commands"
				   " exist."),
				 cmd1, cmd2);

			acme_error (msg);
			g_free (msg);
		}
	}
}

static void
acme_play_sound (Acme *acme)
{
	char *soundfile, *command;

	soundfile = gconf_client_get_string (acme->conf_client,
			"/apps/acme/soundfile_name", NULL);
	if ((soundfile == NULL) || (strcmp (soundfile, "") == 0)) 
		return;

	if (g_file_test ("/usr/bin/esdplay",
			(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)))
	{
		command = g_strdup_printf ("/usr/bin/esdplay %s",
				soundfile);
	} else if (g_file_test ("/usr/bin/play",
			(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)))
	{
		command = g_strdup_printf ("/usr/bin/play %s",
				soundfile);
	} else {
		return;
	}

	execute (command, FALSE);
	g_free (command);
}

static char*
permission_problem_string (const char *files)
{
	return g_strdup_printf (_("Permissions on the file %s are broken\n"
				"Please check ACME's documentation, correct "
				"the problem and restart ACME."), files);
}

static gboolean
vol_problem_cb (GVolume *fb, gpointer data)
{
	char *msg;

	msg = permission_problem_string ("/dev/mixer");
	acme_error (msg);
	g_free (msg);

	return TRUE;
}

#ifdef USE_FBLEVEL
static void
fb_problem_cb (void)
{
	char *msg;

	if (fb_level_is_powerbook () == FALSE)
		return;

	msg = permission_problem_string ("/dev/pmu");
	acme_error (msg);
	g_free (msg);

	return;
}
#endif

static void
update_use_pcm_cb (GConfClient *client, guint id, GConfEntry *entry,
		gpointer data)
{
	Acme *acme = (Acme *)data;
	gboolean use_pcm = FALSE;

	use_pcm = gconf_client_get_bool (acme->conf_client,
			"/apps/acme/use_pcm",
			NULL);
	g_volume_set_use_pcm (acme->volobj, use_pcm);
}

static void
acme_image_set (Acme *acme, int icon)
{
	GtkWidget *image;

	image = glade_xml_get_widget (acme->xml, "image1");
	g_return_if_fail (image != NULL);

	switch (icon) {
	case ICON_LOUD:
		gtk_image_set_from_file (GTK_IMAGE(image),
				ACME_DATA "gnome-speakernotes.png");
		break;
	case ICON_MUTED:
		gtk_image_set_from_file (GTK_IMAGE(image),
				ACME_DATA "gnome-speakernotes-muted.png");
		break;
	case ICON_BRIGHT:
		gtk_image_set_from_file (GTK_IMAGE(image),
				ACME_DATA "acme-brightness.png");
		break;
	case ICON_EJECT:
		gtk_image_set_from_file (GTK_IMAGE(image),
				ACME_DATA "acme-eject.png");
		break;
	default:
		g_assert_not_reached ();
	}
}

static void
ungrab_key_real (int key_code, GdkWindow *root)
{
	gdk_error_trap_push ();
	XUngrabKey (GDK_DISPLAY (), key_code, AnyModifier,
			GDK_WINDOW_XID (root));

	gdk_flush ();
	if (gdk_error_trap_pop ()) {
		char *error;

		error = g_strdup_printf
			(_("There was an error removing access to the "
			   "multimedia keys.\nKey %d couldn't be unbound."),
			 key_code);
		acme_error (error);
		g_free (error);
		exit (1);
	}
}

static void
ungrab_key (Acme *acme, int key_code)
{
	GList *l;

	for (l = acme->screens; l != NULL; l = l->next)
	{
		GdkScreen *screen;

		screen = (GdkScreen *) l->data;
		ungrab_key_real (key_code, gdk_screen_get_root_window (screen));
	}
}


static void
grab_key_real (int key_code, GdkWindow *root)
{
	gdk_error_trap_push ();

	XGrabKey (GDK_DISPLAY (), key_code,
			0,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey (GDK_DISPLAY (), key_code,
			Mod2Mask,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey (GDK_DISPLAY (), key_code,
			Mod5Mask,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey (GDK_DISPLAY (), key_code,
			LockMask,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey (GDK_DISPLAY (), key_code,
			Mod2Mask | LockMask,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey (GDK_DISPLAY (), key_code,
			Mod5Mask | LockMask,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey (GDK_DISPLAY (), key_code,
			Mod2Mask | Mod5Mask,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey (GDK_DISPLAY (), key_code,
			Mod2Mask | Mod5Mask | LockMask,
			GDK_WINDOW_XID (root), True,
			GrabModeAsync, GrabModeAsync);

	gdk_flush ();
	if (gdk_error_trap_pop ())
	{
		char *error;

		error = g_strdup_printf
			(_("It seems that another application already has"
			   " access to the multimedia keys.\n"
			   "Key %d couldn't be bound.\n"
			   "Is another daemon already running ?"),
			 key_code);
		acme_error (error);
		g_free (error);
	}
}

static void
grab_key (Acme *acme, int key_code)
{
	GList *l;

	for (l = acme->screens; l != NULL; l = l->next)
	{
		GdkScreen *screen;

		screen = (GdkScreen *) l->data;
		grab_key_real (key_code, gdk_screen_get_root_window (screen));
	}
}

static void
unhookup_keysym (int keycode)
{
	char *command;

	if (keycode <= 0)
		return;

	command = g_strdup_printf ("xmodmap -e \"keycode %d = \"", keycode);

	g_spawn_command_line_sync (command, NULL, NULL, NULL, NULL);
	g_free (command);
}

static gboolean
hookup_keysym (int keycode, const char *keysym)
{
	char *command;

	if (keycode <= 0)
		return TRUE;

	command = g_strdup_printf ("xmodmap -e \"keycode %d = %s\"",
			keycode, keysym);

	g_spawn_command_line_sync (command, NULL, NULL, NULL, NULL);
	g_free (command);

	return FALSE;
}

static void
update_kbd_cb (GConfClient *client, guint id, GConfEntry *entry, gpointer data)
{
	Acme *acme = (Acme *) data;
	int i;
	char *keysym = NULL;

	g_return_if_fail (entry->key != NULL);

	/* Find the key that was modified */
	for (i = 0; i < HANDLED_KEYS; i++)
	{
		if (strcmp (entry->key, keys[i].key_config) == 0)
		{
			int key_code;
			gboolean grab = TRUE;

			ungrab_key (acme, keys[i].key_code);
			switch (keys[i].key_type) {
			case PLAY_KEY:
			case PAUSE_KEY:
			case STOP_KEY:
			case PREVIOUS_KEY:
			case NEXT_KEY:
			case REFRESH_KEY:
				unhookup_keysym (keys[i].key_code);
				break;
			}

			keys[i].key_code = -1;

			key_code = gconf_client_get_int (acme->conf_client,
					keys[i].key_config, NULL);

			switch (keys[i].key_type) {
			case PLAY_KEY:
				grab = hookup_keysym (key_code,
						"XF86AudioPlay");
				break;
			case PAUSE_KEY:
				grab = hookup_keysym (key_code,
						"XF86AudioPause");
				break;
			case STOP_KEY:
				grab = hookup_keysym (key_code,
						"XF86AudioStop");
				break;
			case PREVIOUS_KEY:
				grab = hookup_keysym (key_code,
						"XF86AudioPrev");
				break;
			case NEXT_KEY:
				grab = hookup_keysym (key_code,
						"XF86AudioNext");
				break;
			case REFRESH_KEY:
				grab = hookup_keysym (key_code,
						"XF86Refresh");
				break;
			}

			if (key_code > 0 && grab == TRUE)
			{
				grab_key (acme, key_code);
				keys[i].key_code = key_code;
			}
		}
	}
}

static gboolean
init_kbd (Acme *acme)
{
	int i;
	gboolean retval = FALSE;

	acme->display = gdk_display_get_default ();
	acme->screens = NULL;

	if (gdk_display_get_n_screens (acme->display) == 1)
	{
		acme->screens = g_list_append (acme->screens,
				gdk_screen_get_default ());
	} else {
		for (i = 0; i < gdk_display_get_n_screens (acme->display); i++)
		{
			GdkScreen *screen;

			screen = gdk_display_get_screen (acme->display, i);

			if (screen != NULL)
				acme->screens = g_list_append (acme->screens,
						screen);
		}
	}

	for (i = 0; i < HANDLED_KEYS; i++)
	{
		gboolean grab = TRUE;
		int tmp;

		tmp = gconf_client_get_int (acme->conf_client,
				keys[i].key_config,
				NULL);

		keys[i].key_code = tmp;

		switch (keys[i].key_type) {
		case PLAY_KEY:
			grab = hookup_keysym (keys[i].key_code,
					"XF86AudioPlay");
			break;
		case PAUSE_KEY:
			grab = hookup_keysym (keys[i].key_code,
					"XF86AudioPause");
			break;
		case STOP_KEY:
			grab = hookup_keysym (keys[i].key_code,
					"XF86AudioStop");
			break;
		case PREVIOUS_KEY:
			grab = hookup_keysym (keys[i].key_code,
					"XF86AudioPrev");
			break;
		case NEXT_KEY:
			grab = hookup_keysym (keys[i].key_code,
					"XF86AudioNext");
			break;
		case REFRESH_KEY:
			grab = hookup_keysym (keys[i].key_code,
					"XF86Refresh");
			break;
		}

		if (tmp > 0 && grab == TRUE)
		{
#ifdef DEBUG
			g_print ("grabbed key %d for gconf key %s\n",
					keys [i].key_code,
					keys[i].key_config);
#endif
			grab_key (acme, keys [i].key_code);
			retval = TRUE;
		}
	}

	for (i = 0; i < HANDLED_KEYS; i++)
	{
		gconf_client_notify_add (acme->conf_client,
				keys[i].key_config,
				update_kbd_cb,
				acme, NULL, NULL);
	}

	return retval;
}

static void
prefs_activated (GtkMenuItem *menuitem, gpointer user_data)
{
	execute ("acme-properties", FALSE);
}

static void
about_activated (GtkMenuItem *menuitem, gpointer user_data)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf = NULL;
	const gchar *authors[] = { "Bastien Nocera <hadess@hadess.net>", NULL };
	const gchar *documenters[] = { NULL };
	const gchar *translator_credits = _("translator_credits");

	if (about != NULL)
	{
		gdk_window_raise (about->window);
		gdk_window_show (about->window);
		return;
	}

	pixbuf = gdk_pixbuf_new_from_file (ACME_DATA "acme-48.png", NULL);

	about = gnome_about_new(_("Acme"), VERSION,
			"Copyright \xc2\xa9 2001-2002 Bastien Nocera",
			_("Multimedia keys daemon"),
			(const char **)authors,
			(const char **)documenters,
			strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			pixbuf);
	
	if (pixbuf != NULL)
		gdk_pixbuf_unref (pixbuf);
	
	g_signal_connect (G_OBJECT (about), "destroy",
			G_CALLBACK (gtk_widget_destroyed), &about);
	g_object_add_weak_pointer (G_OBJECT (about), (void**)&(about));

	gtk_widget_show(about);
}

static gboolean
tray_icon_release (GtkWidget *widget, GdkEventButton *event, Acme *acme)
{
	if (event->button == 3)
	{
		gtk_menu_popdown (GTK_MENU (acme->popup_menu));
		return FALSE;
	}

	return TRUE;
}


static gboolean
tray_icon_press (GtkWidget *widget, GdkEventButton *event, Acme *acme)
{
	if (event->button == 3)
	{
		gtk_menu_popup (GTK_MENU (acme->popup_menu), NULL, NULL, NULL,
				NULL, event->button, event->time);
		return TRUE;
	}

	return FALSE;
}

static gboolean
tray_destroyed (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	init_tray ((Acme *) user_data);
	return TRUE;
}

static void
init_tray (Acme *acme)
{
	GtkWidget *image, *evbox, *item;

	acme->tray_icon = egg_tray_icon_new ("Multimedia Keys daemon");
	image = gtk_image_new_from_file (ACME_DATA "acme-16.png");

	acme->tray_icon_tooltip = gtk_tooltips_new ();
	gtk_tooltips_set_tip (acme->tray_icon_tooltip,
			GTK_WIDGET (acme->tray_icon),
			_("Multimedia Keys daemon active"),
			NULL);

	/* Event box */
	evbox = gtk_event_box_new ();
	g_signal_connect (G_OBJECT (evbox), "button_press_event",
			G_CALLBACK (tray_icon_press), (gpointer) acme);
	g_signal_connect (G_OBJECT (evbox), "button_release_event",
			G_CALLBACK (tray_icon_release), (gpointer) acme);

	/* Popup menu */
	acme->popup_menu = gtk_menu_new ();
	item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES,
			NULL);
	g_signal_connect (G_OBJECT (item), "activate",
			G_CALLBACK (prefs_activated), (gpointer) acme);
	gtk_widget_show (item);
	gtk_menu_shell_append (GTK_MENU_SHELL (acme->popup_menu), item);

	item = gtk_image_menu_item_new_from_stock (GNOME_STOCK_ABOUT,
			NULL);
	g_signal_connect (G_OBJECT (item), "activate",
			G_CALLBACK (about_activated), (gpointer) acme);
	gtk_widget_show (item);
	gtk_menu_shell_append (GTK_MENU_SHELL (acme->popup_menu), item);

	gtk_container_add (GTK_CONTAINER (evbox), image);
	gtk_container_add (GTK_CONTAINER (acme->tray_icon), evbox);
	gtk_widget_show_all (GTK_WIDGET (acme->tray_icon));

	g_signal_connect (G_OBJECT (acme->tray_icon), "destroy-event",
			G_CALLBACK (tray_destroyed), (gpointer) acme);
}

static void
init_sm (Acme *acme)
{
	GnomeClient *master;
	GnomeClientFlags flags;

	master = gnome_master_client ();
	flags = gnome_client_get_flags (master);
	if (flags & GNOME_CLIENT_IS_CONNECTED) {
#ifdef DEBUG
		gnome_client_set_restart_style (master,
				GNOME_RESTART_NEVER);
#else
		gnome_client_set_restart_style (master,
				GNOME_RESTART_ANYWAY);
#endif
		gnome_client_flush (master);
	}

	g_signal_connect (GTK_OBJECT (master), "die",
			G_CALLBACK (acme_exit), acme);
}

static gboolean
dialog_hide (Acme *acme)
{
	gtk_widget_hide (acme->dialog);
	acme->dialog_timeout = 0;
	return FALSE;
}

static void
dialog_show (Acme *acme)
{
	int orig_x, orig_y, orig_w, orig_h, orig_d;
	int screen_w, screen_h;
	int x, y;

	gtk_window_set_screen (GTK_WINDOW (acme->dialog), acme->current_screen);
	gtk_widget_realize (GTK_WIDGET (acme->dialog));

	gdk_window_get_geometry (GTK_WIDGET (acme->dialog)->window,
				 &orig_x, &orig_y,
				 &orig_w, &orig_h, &orig_d);

	screen_w = gdk_screen_get_width (acme->current_screen);
	screen_h = gdk_screen_get_height (acme->current_screen);

	x = (screen_w - orig_w) / 2;
	y = screen_h / 2 + (screen_h / 2 - orig_h) / 2;

	gdk_window_move (GTK_WIDGET (acme->dialog)->window, x, y);

	gtk_widget_show (acme->dialog);

	/* this makes sure the dialog is actually shown */
	while (gtk_events_pending())
		gtk_main_iteration();

	acme->dialog_timeout = gtk_timeout_add (DIALOG_TIMEOUT,
			(GtkFunction) dialog_hide, acme);
}

static void
do_close_window_action (Acme *acme)
{
	GList *windows, *item;
	WnckWindow *focused;

	focused = NULL;
	windows = wnck_screen_get_windows (acme->screen);

	if (windows == NULL)
		return;

	for (item = windows; item != NULL; item = item->next)
	{
		WnckWindow *window = (WnckWindow *) (item->data);
		if (wnck_window_is_active (window) == TRUE)
		{
			focused = window;
			break;
		}
	}

	if (focused != NULL)
		wnck_window_close (focused);
}

static void
do_shade_window_action (Acme *acme)
{
	GList *windows, *item;
	WnckWindow *focused;

	focused = NULL;
	windows = wnck_screen_get_windows (acme->screen);

	if (windows == NULL)
		return;

	for (item = windows; item != NULL; item = item->next)
	{
		WnckWindow *window = (WnckWindow *) (item->data);
		if (wnck_window_is_active (window) == TRUE)
		{
			focused = window;
			break;
		}
	}

	if (focused != NULL)
	{
		if (wnck_window_is_shaded (focused) == TRUE)
			wnck_window_unshade (focused);
		else
			wnck_window_shade (focused);
	}
}

static void
do_www_action (Acme *acme, const char *url)
{
	char *string, *command;

	string = gconf_client_get_string (acme->conf_client,
		"/desktop/gnome/url-handlers/unknown/command",
		 NULL);

	if (string == NULL && strcmp (string, "") == 0)
		return;

	if (url == NULL)
		command = g_strdup_printf (string, "about:blank");
	else
		command = g_strdup_printf (string, url);

	execute (command, FALSE);

	g_free (command);
	g_free (string);
}

static void
do_exit_action (Acme *acme)
{
	GnomeClient *master;

	master = gnome_master_client();
	g_return_if_fail(master != NULL);

	gnome_client_request_save(master,
			GNOME_SAVE_BOTH,
			TRUE,
			GNOME_INTERACT_ANY,
			FALSE,
			TRUE);
}

static void
do_eject_action (Acme *acme)
{
	GtkWidget *progress;
	char *command;

	if (acme->dialog_timeout != 0)
	{
		gtk_timeout_remove (acme->dialog_timeout);
		acme->dialog_timeout = 0;
	}

	progress = glade_xml_get_widget (acme->xml, "progressbar");
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress),
			(double) 0);
	gtk_widget_set_sensitive (progress, FALSE);

	acme_image_set (acme, ICON_EJECT);
	dialog_show (acme);

	command = gconf_client_get_string (acme->conf_client,
			"/apps/acme/eject_command", NULL);
	if ((command != NULL) && (strcmp (command, "") != 0))
		execute (command, TRUE);
	else
		execute ("eject", TRUE);

	gtk_widget_set_sensitive (progress, TRUE);
}

static void
do_media_action (Acme *acme)
{
	GnomeVFSMimeApplication *app;

	app = gnome_vfs_mime_get_default_application ("audio/x-mp3");
	if (app->requires_terminal == TRUE || app->command == NULL)
		return;

	execute (app->command, FALSE);
}

#ifdef USE_FBLEVEL
static void
do_brightness_action (Acme *acme, int type)
{
	GtkWidget *progress;
	int level;

	if (acme->dialog_timeout != 0)
	{
		gtk_timeout_remove (acme->dialog_timeout);
		acme->dialog_timeout = 0;
	}

	level = fb_level_get_level (acme->levobj);
	acme_image_set (acme, ICON_BRIGHT);

	switch (type) {
	case BRIGHT_DOWN_KEY:
		fb_level_set_level (acme->levobj, level - 1);
		break;
	case BRIGHT_UP_KEY:
		fb_level_set_level (acme->levobj, level + 1);
		break;
	}

	level = fb_level_get_level (acme->levobj);
	progress = glade_xml_get_widget (acme->xml, "progressbar");
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress),
			(double) level / 15);

	dialog_show (acme);
}
#endif

static void
do_sound_action (Acme *acme, int type)
{
	GtkWidget *progress;
	gboolean muted;
	int vol;

	if (acme->dialog_timeout != 0)
	{
		gtk_timeout_remove (acme->dialog_timeout);
		acme->dialog_timeout = 0;
	}

	vol = g_volume_get_volume (acme->volobj);
	muted = g_volume_get_mute (acme->volobj);

	switch (type) {
	case MUTE_KEY:
		g_volume_mute_toggle(acme->volobj);
		break;
	case VOLUME_DOWN_KEY:
		if (muted)
		{
			g_volume_mute_toggle(acme->volobj);
		} else {
			g_volume_set_volume (acme->volobj, vol - VOLUME_STEP);
		}
		break;
	case VOLUME_UP_KEY:
		if (muted)
		{
			g_volume_mute_toggle(acme->volobj);
		} else {
			g_volume_set_volume (acme->volobj, vol + VOLUME_STEP);
		}
		break;
	}

	muted = g_volume_get_mute(acme->volobj);
	acme_image_set (acme, muted ? ICON_MUTED : ICON_LOUD);

	vol = g_volume_get_volume (acme->volobj);
	progress = glade_xml_get_widget (acme->xml, "progressbar");
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress),
			(double) vol / 100);

	dialog_show (acme);

	/* No need to play any sound if we're muted, right ? */
	if (muted == FALSE)
		acme_play_sound (acme);
}

static void
do_action (int type, Acme *acme)
{
#ifdef DEBUG
	g_print ("do_action, type is: %d\n", type);
#endif
	switch (type) {
	case MUTE_KEY:
	case VOLUME_DOWN_KEY:
	case VOLUME_UP_KEY:
		if (acme->volobj != NULL)
			do_sound_action (acme, type);
		break;
	case POWER_KEY:
		do_exit_action (acme);
		break;
	case EJECT_KEY:
		do_eject_action (acme);
		break;
	case MEDIA_KEY:
		do_media_action (acme);
		break;
	/* For GCC to be happy */
	case PLAY_KEY:
	case PAUSE_KEY:
	case STOP_KEY:
	case PREVIOUS_KEY:
	case NEXT_KEY:
	case REFRESH_KEY:
		break;
	case HOME_KEY:
		execute ("nautilus", FALSE);
		break;
	case SEARCH_KEY:
		execute ("gnome-search-tool", FALSE);
		break;
	case EMAIL_KEY:
		execute_this_or_that ("evolution mailto:", "balsa");
		break;
	case SLEEP_KEY:
		execute_this_or_that ("apm", "xset dpms force off");
		break;
	case SCREENSAVER_KEY:
		execute ("xscreensaver-command -lock", FALSE);
		break;
	case FINANCE_KEY:
		execute ("gnucash", FALSE);
		break;
	case HELP_KEY:
		execute ("yelp", FALSE);
		break;
	case WWW_KEY:
		do_www_action (acme, NULL);
		break;
	case GROUPS_KEY:
		do_www_action (acme, "http://www.gnomedesktop.org");
		break;
	case CALCULATOR_KEY:
		execute ("gnome-calculator", FALSE);
		break;
	case RECORD_KEY:
		execute ("gnome-sound-recorder", FALSE);
		break;
	case CLOSE_WINDOW_KEY:
		do_close_window_action (acme);
		break;
	case SHADE_WINDOW_KEY:
		do_shade_window_action (acme);
		break;
#ifdef USE_FBLEVEL
	case BRIGHT_DOWN_KEY:
	case BRIGHT_UP_KEY:
		if (acme->levobj != NULL)
			do_brightness_action (acme, type);
		break;
#endif
	default:
		g_assert_not_reached ();
	}
}

static GdkFilterReturn
acme_filter_events (GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
	Acme *acme = (Acme *) data;
	XEvent *xev = (XEvent *) xevent;
	XAnyEvent *xanyev = (XAnyEvent *) xevent;
	XKeyEvent *key = (XKeyEvent *) xevent;
	GdkScreen *event_screen = NULL;
	GList *l;
	int i;

	if (xev->type != KeyRelease)
		return GDK_FILTER_CONTINUE;

	/* Look for which screen we're receiving events */
	for (l = acme->screens; (l != NULL) && (event_screen == NULL);
			l = l->next)
	{
		GdkWindow *window;
		GdkScreen *screen;

		screen = (GdkScreen *) l->data;
		window = gdk_screen_get_root_window (screen);

		if (GDK_WINDOW_XID (window) == xanyev->window)
		{
			event_screen = screen;
			break;
		}
	}

	key = (XKeyEvent *) xevent;

	for (i = 0; i < HANDLED_KEYS; i++)
	{
#ifdef DEBUG
		g_print ("comparing %d and %d\n", keys[i].key_code,
				key->keycode);
#endif

		if (keys[i].key_code == key->keycode)
		{
			switch (keys[i].key_type) {
			case PLAY_KEY:
			case PAUSE_KEY:
			case STOP_KEY:
			case PREVIOUS_KEY:
			case NEXT_KEY:
			case REFRESH_KEY:
				return GDK_FILTER_CONTINUE;
			}

			acme->current_screen = event_screen;

			do_action (keys[i].key_type, acme);
			return GDK_FILTER_REMOVE;
		}
	}

	return GDK_FILTER_CONTINUE;
}

int
main (int argc, char *argv[])
{
	Acme *acme;
	GList *l;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init ("acme", VERSION,
			LIBGNOMEUI_MODULE,
			argc, argv,
			NULL);

	acme = g_new0 (Acme, 1);
	acme->conf_client = NULL;

	if (acme_get_lock (acme) == FALSE)
	{
		g_print ("Daemon already running, exiting...\n");
		acme_exit (acme);
	}

	glade_gnome_init ();
	acme->xml = glade_xml_new (ACME_DATA "acme.glade", NULL, NULL);

	if (acme->xml == NULL) {
		acme_error (_("Couldn't load the Glade file.\n"
				"Make sure that this daemon is properly"
				" installed."));
		exit (1);
	}

	acme->dialog = glade_xml_get_widget (acme->xml, "dialog");
	acme_image_set (acme, ICON_LOUD);

	acme->conf_client = gconf_client_get_default ();
	gconf_client_add_dir (acme->conf_client,
			"/apps/acme",
			GCONF_CLIENT_PRELOAD_ONELEVEL,
			NULL);

	if (init_kbd (acme) == FALSE)
	{
		execute ("acme-properties", FALSE);
		exit (0);
	}
	init_sm (acme);
	init_tray (acme);
	acme->current_screen = gdk_screen_get_default ();
	acme->screen = wnck_screen_get_default ();
	gtk_widget_realize (acme->dialog);
	acme->dialog_timeout = 0;

	/* initialise Volume handler */
	acme->volobj = g_volume_new();
	if (acme->volobj != NULL)
	{
		g_signal_connect(G_OBJECT(acme->volobj),
				"fd_problem",
				G_CALLBACK (vol_problem_cb), NULL);

		g_volume_set_use_pcm (acme->volobj,
				gconf_client_get_bool (acme->conf_client,
					"/apps/acme/use_pcm", NULL));
		gconf_client_notify_add (acme->conf_client,
				"/apps/acme/use_pcm",
				update_use_pcm_cb,
				acme, NULL, NULL);
	}

#ifdef USE_FBLEVEL
	/* initialise Frame Buffer level handler */
	acme->levobj = fb_level_new();
	if (acme->levobj == NULL)
		fb_problem_cb ();
#endif

	/* Start filtering the events */
	for (l = acme->screens; l != NULL; l = l->next)
	{
		GdkScreen *screen;
		GdkWindow *window;

		screen = (GdkScreen *) l->data;
		window = gdk_screen_get_root_window (screen);
		gdk_window_add_filter (window,
				acme_filter_events,
				(gpointer) acme);
	}

	gtk_main ();

	return 0;
}

