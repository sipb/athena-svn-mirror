/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@ximian.com>
 *
 *  Copyright 2002 Iain Holmes 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <config.h>

typedef enum _GnomeCDPreferencesStart {
	GNOME_CD_PREFERENCES_START_NOTHING,
	GNOME_CD_PREFERENCES_START_START,
	GNOME_CD_PREFERENCES_START_STOP
} GnomeCDPreferencesStart;

typedef enum _GnomeCDPreferencesStop {
	GNOME_CD_PREFERENCES_STOP_NOTHING,
	GNOME_CD_PREFERENCES_STOP_STOP,
	GNOME_CD_PREFERENCES_STOP_OPEN,
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	GNOME_CD_PREFERENCES_STOP_CLOSE
#endif
} GnomeCDPreferencesStop;

typedef struct _GnomeCDPreferences {
	GnomeCD *gcd;
	char *device;
	char *theme_name;
	
	GnomeCDPreferencesStart start;
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	gboolean start_close;
#endif

	GnomeCDPreferencesStop stop;

	/* GConf IDs */
	guint device_id;
	guint start_id;
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	guint close_id;
#endif
	guint stop_id;
	guint theme_id;
} GnomeCDPreferences;

GnomeCDPreferences *preferences_new         (GnomeCD *gcd);
GtkWidget          *preferences_dialog_show (GnomeCD *gcd,
					     gboolean only_device);

#endif
