/*
 * gnome-cd.h
 *
 * Copyright (C) 2001, 2002 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifndef __GNOME_CD_H__
#define __GNOME_CD_H__

#include <gtk/gtkwidget.h>
#include <gtk/gtktooltips.h>

#include <pango/pango.h>

#include <cddb-slave-client.h>

#include "gnome-cd-type.h"
#include "preferences.h"
#include "cdrom.h"

#define NUMBER_OF_DISPLAY_LINES 5

typedef struct _GnomeCDDiscInfo {
	char *discid;
	char *title;
	char *artist;
	int ntracks;
	CDDBSlaveClientTrackInfo **track_info;
} GnomeCDDiscInfo;

typedef struct _GnomeCDText {
	char *text;
	int length;
	int height;
	PangoLayout *layout;
	GdkColor *foreground;
	GdkColor *background;
} GnomeCDText;

typedef struct _GCDTheme {
	char *name;
	
	GdkPixbuf *previous, *previous_menu;
	GdkPixbuf *rewind;
	GdkPixbuf *play, *play_menu;
	GdkPixbuf *pause;
	GdkPixbuf *stop, *stop_menu;
	GdkPixbuf *forward;
	GdkPixbuf *next, *next_menu;
	GdkPixbuf *eject, *eject_menu;
} GCDTheme;

struct _GnomeCD {
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *display;
	GtkWidget *tracks;
	GtkWidget *menu;
	GtkWidget *slider;
	GtkTooltips *tooltips;

	GtkWidget *trackeditor_b, *properties_b;

	gboolean not_ready;
	/* FIXME: Make this a control */
	GtkWidget *back_b, *rewind_b;
	GtkWidget *play_b, *stop_b;
	GtkWidget *ffwd_b, *next_b;
	GtkWidget *eject_b;

	GtkWidget *play_image, *pause_image, *current_image;

	/* FIXME: Make this a control too */
	GtkWidget *mixer_b, *volume_b;

	GnomeCDRom *cdrom;

	GnomeCDRomStatus *last_status;

	guint32 timeout;
	guint32 display_timeout;

	int height, max_width;

	GnomeCDDiscInfo *disc_info;

	GCDTheme *theme;
	GnomeCDPreferences *preferences;
};

void skip_to_track (GtkWidget *item,
		    GnomeCD *gcd);
void gnome_cd_set_window_title (GnomeCD *gcd,
				const char *artist,
				const char *track);
void gnome_cd_build_track_list_menu (GnomeCD *gcd);

void gcd_warning (const char *message, GError *error);

/* theme.c */
GCDTheme *theme_load (GnomeCD    *gcd,
		      const char *theme_name);
void theme_change_widgets (GnomeCD *gcd);
void theme_free (GCDTheme *theme);

#endif
