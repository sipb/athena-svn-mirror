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

typedef struct _GnomeCDPreferences {
	GnomeCD *gcd;
	char *device;
	char *theme_name;
	
	gboolean start_play;
	gboolean stop_eject;
	
	/* GConf IDs */
	guint device_id;
	guint start_id;
	guint stop_id;
	guint theme_id;
} GnomeCDPreferences;

GnomeCDPreferences *preferences_new         (GnomeCD *gcd);
GtkWidget          *preferences_dialog_show (GnomeCD *gcd,
					     gboolean only_device);

#endif
