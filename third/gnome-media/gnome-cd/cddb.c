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
static CDDBSlaveClient *slave_client = NULL;
void freekey(gpointer key, gpointer value,gpointer userdata);
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
	
        gcd_debug ("get_disc_info for discid %s", discid);
	info = g_hash_table_lookup (cddb_cache, discid);
	if (info == NULL) {
		g_warning ("No cache for %s", discid);
		return;
	} else {
		gcd->disc_info = info;
	}

	info->artist = cddb_slave_client_get_artist (slave_client, discid);
	info->title = cddb_slave_client_get_disc_title (slave_client, discid);
	info->track_info = cddb_slave_client_get_tracks (slave_client, discid);
        gcd_debug ("get_disc_info: artist %s, title %s, track_info %p",
		info->artist, info->title, info->track_info);

	if (count_tracks (info->track_info) != info->ntracks) {
		/* Duff info */
		cddb_slave_client_free_track_info (info->track_info);
		gcd_debug ("Track count doesn't match, bad entry ?");
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

	gcd_debug ("Got a result for discid %s on listener %p", qr->discid, listener);
	if (gcd->discid == NULL) {
		/* We weren't looking up, so just return */
		gcd_debug ("We're not looking up, so ignoring");
		return;
	}
	if (strcmp (gcd->discid, qr->discid) != 0) {
		gcd_debug ("Not our discid, so ignoring");
		return;
	}
	g_free (gcd->discid);
	gcd->discid = NULL;
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
	if (slave_client != NULL)
		g_object_unref (G_OBJECT (slave_client));
}

// when the CD is ejected we need to destroy the cddb_cache hashTable
void destroy_cache_hashTable()
{
	g_hash_table_foreach(cddb_cache,freekey,NULL); //first free the key
	g_hash_table_destroy(cddb_cache); //destroy the table
	cddb_cache = NULL;
}

//this will only free the keys in hashtable, the values are already freed.
void freekey(gpointer key, gpointer value,gpointer userdata)
{
		if(key != NULL)
			g_free(key);
		key = NULL;
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
		char *tmp, *tmp2;

		tmp = g_strdup_printf ("%u ", data->offsets[i]);
		if (offsets == NULL) {
			offsets = tmp;
		} else {
			tmp2 = g_strconcat (offsets, tmp, NULL);
			g_free (tmp);
			offsets = g_strdup (tmp2);
			g_free (tmp2);
		}
	}

	info = g_hash_table_lookup (cddb_cache, discid);

	if (info != NULL) {
		gcd->disc_info = info;

		gnome_cd_build_track_list_menu (gcd);
		return;
	} else {
		info = cddb_make_disc_info (data);
		// g_strdup is added so that the key will persist
		g_hash_table_insert (cddb_cache, g_strdup(info->discid), info);
		gcd->disc_info = info;
		gnome_cd_build_track_list_menu (gcd);
	}

	/* Remove the last space */
	offsets[strlen (offsets) - 1] = 0;

	/* Create a new slave client if we don't have one yet */
	if (slave_client == NULL) {
		slave_client = cddb_slave_client_new ();
		listener = bonobo_listener_new (NULL, NULL);
		gcd_debug ("Created a new bonobo listener %p", listener);
		g_signal_connect (G_OBJECT (listener), "event-notify",
				  G_CALLBACK (cddb_listener_event_cb), gcd);

		cddb_slave_client_add_listener (slave_client, listener);
	}

	gcd_debug ("Performing lookup for:");
	gcd_debug ("%s %d %s %d", discid, data->ntrks, offsets, data->nsecs);
	gcd->discid = discid;
	cddb_slave_client_query (slave_client, discid, data->ntrks, offsets, 
				 data->nsecs, "GnomeCD", VERSION);
	
	gnome_cdrom_free_cddb_data (data);
	g_free (offsets);
}

int
cddb_sum (int n)
{
	char buf[12], *p = NULL;
	int ret = 0;
	
	/* This is what I get for copying TCD code */
	sprintf (buf, "%u", n);
	for (p = buf; *p != '\0'; p++) {
		ret += (*p - '0');
	}

	return ret;
}
