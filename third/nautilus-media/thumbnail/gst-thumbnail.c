/* gst-thumbnail
 * Copyright (C) <2002> Keith Conger <keithconger@earthlink.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gst/gst.h>

#define FRAME	200		/* which frame to snapshot */
#define TIMEOUT	3000		/* how long before we give up, msec */

gboolean finished = FALSE;
gboolean can_finish = FALSE;

void end_of_snap (GstElement *pipeline)
{
	g_print ("Snapped.\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);
	while (!can_finish) ;
	gst_main_quit ();
	finished = TRUE;
}

/* timeout after a given amount of time */
gboolean timeout (GstPipeline *pipeline)
{
	/* setting the state NULL will make iterate return false */
	gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);

	gst_main_quit ();
	return FALSE;
}

gboolean iterator (GstPipeline *pipeline)
{
	/* setting the state NULL will make iterate return false */
	return gst_bin_iterate (GST_BIN (pipeline));
}


static void
gst_thumbnail_pngenc_get (const char *media, const char *thumbnail)
{
	GstElement *pipeline;
	GstElement *gnomevfssrc;
	GstElement *snapshot;
	GstElement *sink;
	GstPad *pad;
	GstEvent *event;
	gboolean res;
	GError *error = NULL;
	int i;

	/* FIXME: we might want to use a static autoplugger
	 *        or implement a kill callback */
	pipeline = gst_parse_launch ("gnomevfssrc name=gnomevfssrc ! spider ! "
			             "colorspace ! pngenc name=snapshot",
				     &error);
	if (!GST_IS_PIPELINE (pipeline))
	{
		g_print ("Parse error: %s\n", error->message);
		exit (1);
	}
	gnomevfssrc = gst_bin_get_by_name (GST_BIN (pipeline), "gnomevfssrc");
	snapshot = gst_bin_get_by_name (GST_BIN (pipeline), "snapshot");
	g_assert (GST_IS_ELEMENT (snapshot));
	g_assert (GST_IS_ELEMENT (gnomevfssrc));
	g_object_set (G_OBJECT (gnomevfssrc), "location", media, NULL);

	gst_element_set_state (pipeline, GST_STATE_PLAYING);
	/* FIXME: do an actual seek here instead */
	for (i = 0; i < FRAME; ++i)
		gst_bin_iterate (GST_BIN (pipeline));
	gst_element_set_state (pipeline, GST_STATE_PAUSED);

	sink = gst_element_factory_make ("filesink", "sink");
	g_assert (GST_IS_ELEMENT (sink));
	g_object_set (G_OBJECT (sink), "location", thumbnail, NULL);
	gst_bin_add (GST_BIN (pipeline), sink);
	gst_element_link (snapshot, sink);
	g_signal_connect (G_OBJECT (sink), "handoff",
			  G_CALLBACK (end_of_snap), pipeline);

	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	/* commit suicide in due time if necessary */
	g_timeout_add (TIMEOUT, (GSourceFunc) timeout, pipeline);
	g_idle_add ((GSourceFunc) iterator, pipeline);

	can_finish = TRUE;
	gst_main ();
}
int
main (int argc, char *argv[])
{
	GstElement *snapshot;
	GstElement *pngenc;
	gboolean use_pngenc = FALSE;

	gst_init (&argc, &argv);


	if (argc != 3)
	{
		g_print ("gst-thumbnail: thumbnailer for Nautilus\n");
		g_print ("usage: %s <input-filename> <output-filename>\n",
			 argv[0]);
		return -1;
	}

	pngenc = gst_element_factory_make ("pngenc", "pngenc");
	if (pngenc == NULL)
	{
		g_error ("Couldn't find pngenc plug-in.");
	}
	gst_thumbnail_pngenc_get (argv[1], argv[2]);
	/* FIXME: how about creating a "dummy" png when we couldn't make it ? */
	if (finished) return 0; else return -2;
}
