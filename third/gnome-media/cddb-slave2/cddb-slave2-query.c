/*
 * cddb-slave2-query.c: perform a CDDB query using the CDDB-Slave2 component
 *
 * Copyright (C) <2004> Thomas Vander Stichele <thomas at apestaart dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>                           /* strtol */

#include <glib.h>
#include <bonobo/bonobo-main.h>

#include "cddb-slave-private.h"
#include <cddb-slave-client.h>
#include <GNOME_Media_CDDBSlave2.h>

static gboolean is_setup = FALSE; /* set from setup_cb */
static gboolean query_ok = FALSE; /* set to TRUE from cb if query went ok */
static gboolean have_result = FALSE;

/* In a real application we would put these in a structure
 * and pass the structure around.
 */
static CDDBSlaveClient *slave_client = NULL;
gchar *discid = NULL;
gchar *offsets = NULL;
gint ntracks;
gint nsecs;

static void listener_event_cb (BonoboListener *listener,
                               const char *name,
                               const BonoboArg *arg,
                               CORBA_Environment *ev,
	                       gpointer data);

/* display the result */
static void
display_result (CDDBSlaveClient *slave_client, const char *discid)
{
	gchar *artist = NULL;
	gchar *title = NULL;
	gint ntracks;
	gint i;
	CDDBSlaveClientTrackInfo **track_info = NULL;

	artist = cddb_slave_client_get_artist (slave_client, discid);
	title = cddb_slave_client_get_disc_title (slave_client, discid);
	track_info = cddb_slave_client_get_tracks (slave_client, discid);
	ntracks = cddb_slave_client_get_ntrks (slave_client, discid);
	track_info = cddb_slave_client_get_tracks (slave_client, discid);

	g_print ("Results of CDDBSlave2 lookup:\n");
	g_print ("Artist:     %s\n", artist);
	g_print ("Disc Title: %s\n", title);
	g_print ("Tracks:     %d\n", ntracks);
	g_print ("\n");

	for (i = 1; i <= ntracks; ++i)
	{
		gint length;

		length = track_info[i - 1]->length;
		g_print ("Track %2d:   %s (%d:%d)\n", i,
                         track_info[i - 1]->name,
                         length / 60, length % 60);
	}
	g_print ("\n");
}


/* set up our slave client */
static gboolean
setup_cb (void)
{
	BonoboListener *listener = NULL;

	/* create a cddb slave client */
	slave_client = cddb_slave_client_new ();
	cs_debug ("created slave client %p", slave_client);

	/* create a bonobo listener */
	listener = bonobo_listener_new (NULL, NULL);
	cs_debug ("created bonobo listener %p", listener);

	/* connect a callback to the listener */
	g_signal_connect (G_OBJECT (listener), "event-notify",
			  G_CALLBACK (listener_event_cb), NULL);

	/* add the listener to the slave client */
	cddb_slave_client_add_listener (slave_client, listener);

	is_setup = TRUE;
	/* remove ourselves from idle loop */
	return FALSE;
}


static gboolean
query_cb (gpointer data)
{
	if (!is_setup)
		/* call us back later */
		return TRUE;

	/* send the query */
	cs_debug ("query: sending query");
	if (! cddb_slave_client_query (slave_client, discid, ntracks, offsets, nsecs,
                                       "cddb-slave2-query", "0"))
		g_warning ("Could not query");

	cs_debug ("query: sent query");
	return FALSE;
}

/* teardown the connection; will also quit from bonobo main loop */
static gboolean
teardown_cb (gpointer data)
{
	if (!have_result) return TRUE;

	/* unref our slave object
         * this also triggers notify removal */
	cs_debug ("teardown_cb: unreffing slave client");
        g_object_unref (G_OBJECT (slave_client));

	cs_debug ("teardown_cb: quitting bonobo main loop");
	bonobo_main_quit ();

	return FALSE;
}

static void
listener_event_cb (BonoboListener *listener,
                   const char *name,
                   const BonoboArg *arg,
                   CORBA_Environment *ev,
	           gpointer data)
{
        GNOME_Media_CDDBSlave2_QueryResult *qr;

	cs_debug ("listener event callback on slave client %p", slave_client);

        qr = arg->_value;

        switch (qr->result) {
        case GNOME_Media_CDDBSlave2_OK:
		cs_debug ("callback: lookup ok");
		query_ok = TRUE;
                break;
	default:
		g_warning ("Lookup through slave failed due to an error: %d",
			qr->result);
		break;
	}
	have_result = TRUE;
	if (query_ok)
		display_result (slave_client, discid);

}

int
main (int argc,
      char *argv[])
{
	char *endptr = NULL;
	gchar *string = NULL;
	gint i;

	/* initialize bonobo so we can use cddb_slave_client_new */
	/* watch out ! bonobo_init takes **argv, not ***argv like others */
	bonobo_init (&argc, argv);

	if (argc < 2)
	{
		g_print ("Please specify a disc id to look up.\n");
		return 1;
	}
	discid = g_strdup (argv[1]);

	if (argc < 3)
	{
		g_print ("Please specify the number of tracks.\n");
		return 2;
	}
	ntracks = strtol (argv[2], &endptr, 0);
	if (*endptr != '\0')
	{
		g_print ("Please specify an integer as the number of tracks.\n");
		return 3;
	}

	if (argc < 3 + ntracks)
	{
		g_print ("Please specify as many frame start offsets as tracks.\n");
		return 4;
	}

	/* take the first offset */
	offsets = g_strdup (argv[3]);

	/* and now add the others */
	for (i = 1; i < ntracks; ++i)
	{
		string = g_strdup_printf ("%s %s", offsets, argv[3 + i]);
		g_free (offsets);
		offsets = string;
	}
	if (argc < 3 + ntracks + 1)
	{
		g_print ("Please specify the total length in seconds.\n");
		return 5;
	}
	nsecs = strtol (argv[3 + ntracks], &endptr, 0);
	if (*endptr != '\0')
	{
		g_print ("Please specify an integer as the total length in seconds.\n");
		return 6;
	}

	/* output parsed info */
	g_print ("Looking up CDDBSlave2 entry with:\n");
	g_print ("discid:           %s\n", discid);
	g_print ("number of tracks: %d\n", ntracks);
	g_print ("offsets:          %s\n", offsets);
	g_print ("total seconds:    %d\n", nsecs);
	g_print ("\n");

	/* check for debug */
	if (g_getenv ("CDDB_SLAVE_DEBUG"))
		cs_set_debug (TRUE);
	g_timeout_add (10, (GSourceFunc) setup_cb, NULL);
	g_timeout_add (10, (GSourceFunc) query_cb, NULL);
	g_timeout_add (10, (GSourceFunc) teardown_cb, NULL);

	/* enter bonobo main loop */
	bonobo_main ();

	cs_debug ("query: returned from bonobo_main");

	/* remove the listener from the slave */
	//cddb_slave_client_remove_listener (slave, listener);


	return 0;
}
