/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * cddb.c
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes <iain@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <bonobo/bonobo-listener.h>

#include <cddb-slave-client.h>
#include <GNOME_Media_CDDBSlave2.h>

#include <libgnome/gnome-util.h>

#include "gnome-cd.h"
#include "cdrom.h"
#include "cddb.h"

static GHashTable *cddb_cache;
static CDDBSlaveClient *slave = NULL;

static int
count_tracks (CDDBSlaveClientTrackInfo **info)
{
	int i;

	for (i = 0; info[i] != NULL; i++) {
		;
	}

	return i;
}

static void
get_disc_info (GnomeCD *gcd,
	       const char *discid)
{
	GnomeCDDiscInfo *info;
	
	info = g_hash_table_lookup (cddb_cache, discid);
	if (info == NULL) {
		g_warning ("No cache for %s", discid);
		return;
	} else {
		gcd->disc_info = info;
	}

	info->title = cddb_slave_client_get_disc_title (slave, discid);
	info->artist = cddb_slave_client_get_artist (slave, discid);
	info->track_info = cddb_slave_client_get_tracks (slave, discid);

	if (count_tracks (info->track_info) != info->ntracks) {
		/* Duff info */
		cddb_slave_client_free_track_info (info->track_info);
		info->track_info = NULL;
		gcd->disc_info = NULL;
	}

	gnome_cd_build_track_list_menu (gcd);
}

static void
cddb_listener_event_cb (BonoboListener *listener,
			const char *name,
			const BonoboArg *arg,
			CORBA_Environment *ev,
			GnomeCD *gcd)
{
	GNOME_Media_CDDBSlave2_QueryResult *qr;

	qr = arg->_value;

	switch (qr->result) {
	case GNOME_Media_CDDBSlave2_OK:
		get_disc_info (gcd, qr->discid);
		break;

	case GNOME_Media_CDDBSlave2_REQUEST_PENDING:
		/* Do nothing really */
		gcd->disc_info = NULL;
		break;

	case GNOME_Media_CDDBSlave2_ERROR_CONTACTING_SERVER:
		g_warning ("Could not contact CDDB server");
		gcd->disc_info = NULL;
		break;

	case GNOME_Media_CDDBSlave2_ERROR_RETRIEVING_DATA:
		g_warning ("Error downloading data");
		gcd->disc_info = NULL;
		break;

	case GNOME_Media_CDDBSlave2_MALFORMED_DATA:
		g_warning ("Malformed data");
		gcd->disc_info = NULL;
		break;

	case GNOME_Media_CDDBSlave2_IO_ERROR:
		g_warning ("Generic IO error");
		gcd->disc_info = NULL;
		break;

	default:
		break;
	}
}

void
cddb_free_disc_info (GnomeCDDiscInfo *info)
{
	g_free (info->discid);
	g_free (info->title);
	g_free (info->artist);

	if (info->track_info)
		cddb_slave_client_free_track_info (info->track_info);
	g_free (info);
}

static GnomeCDDiscInfo *
cddb_make_disc_info (GnomeCDRomCDDBData *data)
{
	GnomeCDDiscInfo *discinfo;

	discinfo = g_new0 (GnomeCDDiscInfo, 1);
	discinfo->discid = g_strdup_printf ("%08lx", (gulong) data->discid);
	discinfo->title = NULL;
	discinfo->artist = NULL;
	discinfo->ntracks = data->ntrks;
	discinfo->track_info = NULL;

	return discinfo;
}

void
cddb_close_client (void)
{
	if (slave != NULL)
		g_object_unref (G_OBJECT (slave));
}

void
cddb_get_query (GnomeCD *gcd)
{
	GnomeCDRomCDDBData *data;
	GnomeCDDiscInfo *info;
	BonoboListener *listener;
	char *discid;
	char *offsets = NULL;
	int i;

	if (cddb_cache == NULL) {
		cddb_cache = g_hash_table_new (g_str_hash, g_str_equal);
	}

	if (gnome_cdrom_get_cddb_data (gcd->cdrom, &data, NULL) == FALSE) {
		g_print ("gnome_cdrom_get_cddb_data returned FALSE");
		return;
	}

	discid = g_strdup_printf ("%08lx", (gulong) data->discid);
	for (i = 0; i < data->ntrks; i++) {
		char *tmp;

		tmp = g_strdup_printf ("%u ", data->offsets[i]);
		if (offsets == NULL) {
			offsets = tmp;
		} else {
			offsets = g_strconcat (offsets, tmp, NULL);
			g_free (tmp);
		}
	}

	info = g_hash_table_lookup (cddb_cache, discid);
	if (info != NULL) {
		gcd->disc_info = info;

		gnome_cd_build_track_list_menu (gcd);
		return;
	} else {
		info = cddb_make_disc_info (data);
		g_hash_table_insert (cddb_cache, info->discid, info);
	}

	/* Remove the last space */
	offsets[strlen (offsets) - 1] = 0;

	if (slave == NULL) {
		slave = cddb_slave_client_new ();
		listener = bonobo_listener_new (NULL, NULL);
		g_signal_connect (G_OBJECT (listener), "event-notify",
				  G_CALLBACK (cddb_listener_event_cb), gcd);

		cddb_slave_client_add_listener (slave, listener);
	}

	cddb_slave_client_query (slave, discid, data->ntrks, offsets, 
				 data->nsecs, "GnomeCD", VERSION);
	
	gnome_cdrom_free_cddb_data (data);
}

int
cddb_sum (int n)
{
	char buf[12], *p;
	int ret = 0;
	
	/* This is what I get for copying TCD code */
	sprintf (buf, "%u", n);
	for (p = buf; *p != '\0'; p++) {
		ret += (*p - '0');
	}

	return ret;
}
