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
#include "cd-selection.h"
#include "preferences.h"
#include "cdrom.h"
#include "eggtrayicon.h"

#define NUMBER_OF_DISPLAY_LINES 5

/* Stock icons */
#define GNOME_CD_PLAY "media-play"
#define GNOME_CD_PAUSE "media-pause"
#define GNOME_CD_STOP "media-stop"
#define GNOME_CD_PREVIOUS "media-prev"
#define GNOME_CD_NEXT "media-next"
#define GNOME_CD_FFWD "media-ffwd"
#define GNOME_CD_REWIND "media-rewind"
#define GNOME_CD_EJECT "media-eject"

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
} GCDTheme;

struct _GnomeCD {
	GtkWidget *tray;
	GtkWidget *tray_icon;
	GtkTooltips *tray_tips;

	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *display;
	GtkObject *position_adj;
	GtkWidget *position_slider;
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
	CDSelection *cd_selection;

	/* Set if if --device was given on the command line */
	char *device_override;

	char *discid;	/* used to track which one we're looking up */
};

void skip_to_track (GtkWidget *item,
		    GnomeCD *gcd);
void gnome_cd_set_window_title (GnomeCD *gcd,
				const char *artist,
				const char *track);
void gnome_cd_build_track_list_menu (GnomeCD *gcd);

void gcd_warning (const char *message, GError *error);
void gcd_debug (const gchar *format, ...) G_GNUC_PRINTF (1, 2);

/* theme.c */
GCDTheme *theme_load (GnomeCD    *gcd,
		      const char *theme_name);
void theme_change_widgets (GnomeCD *gcd);
void theme_free (GCDTheme *theme);

#endif
