/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Copyright (C) 2002 Thomas Vander Stichele
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Thomas Vander Stichele <thomas at apestaart dot org>
 */

/* audio-play.c - play files using AudioPlay GObject
 */

#include "config.h"

#include <gst/gst.h>
#include "audio-play.h"

/* private object properties */
struct AudioPlayPriv {
	GstElement *pipeline;		/* main playback pipeline */
	GstElement *src;
	GstElement *typefind;
	GstElement *decoder;
	GstElement *sink;

	GstCaps *caps;			/* FIXME: why ? */

	gboolean have_type;
	gboolean have_eos;		/* dispatcher booleans from idler */
	gboolean have_cache_empty;
	gboolean have_caps;

	gchar *location;

	guint64 length_nanoseconds;	/* total length */
	gint seconds;			/* current playing time */
	guint64 nanoseconds;

	AudioPlayErrorHandler error_handler;
	gpointer error_data;
	guint idler_id;
	guint iterator_id;
	guint tick_timeout_id;
};

/* signal enum */
enum {
	EOS_SIGNAL,
	TICK_SIGNAL,
	LENGTH_SIGNAL,
	LAST_SIGNAL
};

/* GError enum */
enum {
	AUDIO_PLAY_ERROR_SRC,
	AUDIO_PLAY_ERROR_TYPEFIND,
	AUDIO_PLAY_ERROR_AUTOPLUG
};

static guint audio_play_signals [LAST_SIGNAL] = { 0 };


/* GError quark stuff */
static GQuark
audio_play_error_quark (void)
{
	static GQuark quark = 0;
	if (quark == 0)
		quark = g_quark_from_static_string ("audio-play-error-quark");
	return quark;
}

/* forward declarations */
static void	audio_play_class_init	(AudioPlayClass *klass);
static void	audio_play_init		(AudioPlay *play);

static gboolean	audio_play_playing_to_ready (AudioPlay *play);
static void	audio_play_dispose	(GObject *object);
static void	audio_play_finalize	(GObject *object);
static void	callback_sink_eos 	(GstElement *element, AudioPlay *play);

/* GObject type stuff */
static GObject *parent_class = NULL;

GType
audio_play_get_type (void)
{
	static GType play_type = 0;

	if (!play_type)
	{
		static const GTypeInfo play_info = {
			sizeof (AudioPlayClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) audio_play_class_init,
			NULL, NULL, sizeof (AudioPlay),
			0, (GInstanceInitFunc) audio_play_init,
			NULL
		};
		play_type = g_type_register_static (G_TYPE_OBJECT, "AudioPlay",
						    &play_info, 0);
										        }
	return play_type;
}


/* private functions */
/* the lazy man's version of debugging */
static void
audio_play_debug (AudioPlay *play)
{
	NM_DEBUG("audio_play %p\n", play);
	NM_DEBUG("pipeline %p\n", play->priv->pipeline);
	NM_DEBUG("typefind %p\n", play->priv->typefind);
	NM_DEBUG("decoder %p\n", play->priv->decoder);
	NM_DEBUG("time %d\n", play->priv->seconds);
}

/* error function; calls the handler if it's there */

static void
audio_play_error (AudioPlay *play, const gchar *message)
{
	if (play == NULL)
	{
		g_warning ("No play object passed.");
		return;
	}
	if (play->priv->error_handler)
		play->priv->error_handler (message, play->priv->error_data);
	else
		g_print ("audio-play error: %s\n", message);
}
/* handler functions dispatched from iterator */

static void
handle_have_type (AudioPlay *play)
{
	AudioPlayPriv *priv;
	GstCaps *caps;
	GstStructure *str;
	GstElement *decoder = NULL;
	gchar *mime;

	priv = play->priv;
	caps = priv->caps;
	str = gst_caps_get_structure (caps, 0);
	mime = g_strdup_printf (gst_structure_get_name (str));
	gst_caps_free (priv->caps);

#ifdef DEBUG
	fprintf (stderr, "handle_have_type\n");
	fprintf (stderr, "have caps, mime type is %s\n", mime);
#endif

	g_assert (GST_IS_ELEMENT (priv->pipeline));
	priv->have_type = FALSE;

	gst_element_set_state (priv->pipeline, GST_STATE_READY);

	/* disconnect the typefind from the pipeline and remove it,
	 * since we now know what the type is */
	NM_DEBUG("disconnecting typefind\n");
	gst_element_disconnect (priv->src, priv->typefind);
	/* ref it so it doesn't go away */
	g_object_ref (priv->typefind);
	NM_DEBUG("removing typefind\n");
	gst_bin_remove (GST_BIN (priv->pipeline), priv->typefind);

#ifdef STATICDECODER
	/* now based on the mime type set up the pipeline properly */
	NM_DEBUG("deciding on decoder\n");
	if ((strcmp (mime, "application/x-ogg") == 0) ||
            (strcmp (mime, "application/ogg") == 0))
		decoder = gst_element_factory_make ("vorbisfile", "decoder");
	else if ((strcmp (mime, "audio/mpeg") == 0) ||
		 (strcmp (mime, "audio/x-mp3") == 0) ||
		 (strcmp (mime, "audio/mp3") == 0) ||
		 (strcmp (mime, "application/x-id3") == 0) ||
		 (strcmp (mime, "audio/x-id3") == 0))
		decoder = gst_element_factory_make ("mad", "decoder");
	else if (strcmp (mime, "application/x-flac") == 0)
		decoder = gst_element_factory_make ("flacdec", "decoder");
	else if (strcmp (mime, "audio/x-wav") == 0)
		decoder = gst_element_factory_make ("wavparse", "decoder");
	else if (strcmp (mime, "audio/x-mod") == 0 ||
		 strcmp (mime, "audio/x-s3m") == 0 ||
		 strcmp (mime, "audio/x-xm") == 0 ||
		 strcmp (mime, "audio/x-it") == 0)
		decoder = gst_element_factory_make ("modplug", "decoder");
	else
	{
		g_print ("mime type %s not handled in this program.\n", mime);
		g_free (mime);
		return;
	}
	g_free (mime);
#else
	decoder = gst_element_factory_make ("spider", "spider");
#endif
	priv->decoder = decoder;

	/* set it to null */
	gst_element_set_state (priv->pipeline, GST_STATE_NULL);

	/* now put playback pipe in place of where typefind was  */
	NM_DEBUG("putting in decoder\n");
	gst_bin_add (GST_BIN (priv->pipeline), priv->decoder);
	NM_DEBUG("putting in sink\n");
	gst_bin_add (GST_BIN (priv->pipeline), priv->sink);
	/* FIXME: this shouldn't go here */
	g_signal_connect (G_OBJECT (play->priv->sink), "eos",
			  G_CALLBACK (callback_sink_eos), play);
	if (!gst_element_connect (priv->src, priv->decoder))
		g_error ("warning: could not connect src and decoder !\n");
	if (!gst_element_connect (priv->decoder, priv->sink))
		g_print ("warning: could not connect decoder and sink !\n");
	if (gst_element_set_state (priv->pipeline, GST_STATE_PAUSED)
			== GST_STATE_FAILURE)
		g_print ("pipelien could not be paused !\n");
	NM_DEBUG("handle_have_type: DONE\n");
}

/* callbacks & idlers */
/* iterator idler func; iterates pipeline */
static gboolean
iterator (AudioPlay *play)
{
#ifdef DEBUG
	g_print ("+");
#endif
	if (gst_bin_iterate (GST_BIN (play->priv->pipeline)))
	{
		return TRUE;
	}
	else
	{
		NM_DEBUG("iterator: couldn't iterate\n");
		/* do an eos */
		play->priv->have_eos = TRUE;
		return FALSE;
	}
}

/* main idler func, handles requests and signal actions asynchronously */
static gboolean
idler (AudioPlay *play)
{
#ifdef DEBUG
	g_print ("-");
#endif
	if (play->priv->have_type)
		handle_have_type (play);
	if (play->priv->have_eos)
	{
		NM_DEBUG("signalling eos\n");
		play->priv->have_eos = FALSE;
		audio_play_playing_to_ready (play);
		/* gst_main_quit (); */
		g_signal_emit (G_OBJECT (play),
			       audio_play_signals [EOS_SIGNAL],
			       0);
		return FALSE;
	}
	if (audio_play_get_state (play) == GST_STATE_PLAYING)
		return TRUE;
	else
	{
		NM_DEBUG("idler returning false, not playing\n");
		return FALSE;
	}
}

/* timeout func for emitting the tick signal */
static gboolean
tick_timeout (AudioPlay *play)
{
	gint secs;
	guint64 nanosecs;
	GstElement *sink = play->priv->sink;
	GstFormat format = GST_FORMAT_TIME;
	gboolean q = FALSE;
	AudioPlayPriv *priv = play->priv;

	q = gst_element_query (sink, GST_QUERY_POSITION, &format, &nanosecs);
	if (!q) return;

	secs = (gint) (nanosecs / GST_SECOND);
	if (secs != priv->seconds)
	{
		priv->seconds = secs;
		priv->nanoseconds = nanosecs;
		g_signal_emit (G_OBJECT (play),
			       audio_play_signals [TICK_SIGNAL],
			       0, priv->nanoseconds);
	}
	/* remove ourselves when we stop playing */
	return (gst_element_get_state (priv->pipeline) == GST_STATE_PLAYING);
}


static void
callback_sink_eos (GstElement *element, AudioPlay *play)
{
	AudioPlayPriv *priv = play->priv;

	NM_DEBUG("sink_eos triggered\n");
	play->priv->have_eos = TRUE;
	/* gst_main_quit (); */
}

/* FIXME: would you please sanitify naming of callbacks and so on ? */
/* callback used for errors  coming from GStreamer */
static void
error_callback (GObject *object, GstObject *origin, gchar *error,
		gpointer data)
{
	AudioPlay *play = data;
	gchar *message;

	g_assert (IS_AUDIO_PLAY (play));

	message = g_strdup_printf ("GStreamer error: %s\n", error);
	audio_play_error (play, message);
	g_free (message);
}

/* callback used by query of length of track to know when caps are there */
/* FIXME: we moved this */
static void
deep_notify (GObject *object, GstObject *origin,
	     GParamSpec *pspec, AudioPlay *play)
{
	GValue value = { 0 };
	gint rate = 0;

	NM_DEBUG("deep_notify: of %s\n", pspec->name);
	if (strcmp (pspec->name, "caps") == 0)
	{
		GstElement *element;

		element = GST_ELEMENT (gst_object_get_parent (origin));
		/* FIXME: we should check if the caps are sufficient
		 * for length calculation */
		g_value_init (&value, pspec->value_type);
		g_object_get_property (G_OBJECT (origin), pspec->name, &value);
		play->priv->caps = g_value_peek_pointer (&value);
		NM_DEBUG("deep_notify: got caps %" GST_PTR_FORMAT " from %s\n",
			play->priv->caps, gst_element_get_name (element));
		NM_DEBUG("deep_notify: element %p, sink %p\n", element, play->priv->sink);
		NM_DEBUG("deep_notify: sink %s\n", gst_element_get_name (play->priv->sink));
		if (element == play->priv->sink)
		{
			NM_DEBUG("deep_notify: caps coming from sink\n");
			play->priv->have_caps = TRUE;
		}
	}
	if (strcmp (pspec->name, "channels") == 0)
	{
		/* good enough to have_caps on */
		play->priv->have_caps = TRUE;
	}
}
/* callback used by query of length of track to know when caps are present
   on the audio sink bin */
static void
deep_notify_sink (GObject *object, GstObject *origin,
	          GParamSpec *pspec, AudioPlay *play)
{
	GValue value = { 0 };
	gint rate = 0;

	NM_DEBUG("deep_notify_sink: of %s\n", pspec->name);
	if (strcmp (pspec->name, "caps") == 0)
	{
		GstElement *element;

		element = GST_ELEMENT (gst_object_get_parent (origin));
		/* FIXME: we should check if the caps are sufficient
		 * for length calculation */
		g_value_init (&value, pspec->value_type);
		g_object_get_property (G_OBJECT (origin), pspec->name, &value);
		play->priv->caps = g_value_peek_pointer (&value);
		play->priv->have_caps = TRUE;
		NM_DEBUG("deep_notify: got caps %" GST_PTR_FORMAT " from %s\n",
			play->priv->caps, gst_element_get_name (element));
	}
}

/* callback for when we have the type of the file
 * set the boolean and handle it in the iterator
 * we set up a decoder based on the mime type
 */
static void
have_type_callback (GstElement *element, guint probability, GstCaps *caps, AudioPlay *play)
{
	AudioPlayPriv *priv = play->priv;

	g_assert (priv);
#ifdef DEBUG
	NM_DEBUG("have_type callback called, caps %" GST_PTR_FORMAT ".\n", caps);
#endif

	g_assert (GST_IS_ELEMENT (priv->pipeline));
	priv->have_type = TRUE;
	priv->caps = gst_caps_copy (caps);
}

/* object functions */

static void
audio_play_class_init (AudioPlayClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        parent_class = g_type_class_ref (G_TYPE_OBJECT);

        klass->eos = NULL;

        gobject_class->dispose = audio_play_dispose;
        gobject_class->finalize = audio_play_finalize;

        audio_play_signals [EOS_SIGNAL] =
		g_signal_new ("eos",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
                              G_STRUCT_OFFSET (AudioPlayClass, eos),
                              NULL, NULL,
                              gst_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
        audio_play_signals [LENGTH_SIGNAL] =
		g_signal_new ("length",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
                              G_STRUCT_OFFSET (AudioPlayClass, length),
                              NULL, NULL,
                              gst_marshal_VOID__INT64,
                              G_TYPE_NONE, 1,
			      G_TYPE_INT64);
        audio_play_signals [TICK_SIGNAL] =
		g_signal_new ("tick",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
                              G_STRUCT_OFFSET (AudioPlayClass, tick),
                              NULL, NULL,
                              gst_marshal_VOID__INT64,
                              G_TYPE_NONE, 1,
			      G_TYPE_INT64);
}

static void
audio_play_init (AudioPlay *play)
{
	AudioPlayPriv *priv = g_malloc (sizeof (AudioPlayPriv));

	priv->pipeline = NULL;
	priv->src = NULL;
	priv->typefind = NULL;
	priv->decoder = NULL;
	priv->sink = NULL;

	priv->caps = NULL;
	priv->seconds = 0;
	priv->nanoseconds = 0;
	priv->length_nanoseconds = 0;

	priv->location = NULL;

	priv->idler_id = 0;
	priv->iterator_id = 0;

	priv->have_caps = FALSE;

	play->priv = priv;
}

/* create audio play object */
/* makes a pipeline with gnomevfsssrc in it
 * and pre-initializes typefind
 */

AudioPlay *
audio_play_new (GError **error)
{
	AudioPlay *play = g_object_new (AUDIO_PLAY_TYPE, NULL);
	AudioPlayPriv *priv = play->priv;

	g_assert (priv);
	priv->pipeline = gst_pipeline_new ("pipeline");
	g_assert (priv->pipeline);

	/* set up all the elements we're going to use */

	priv->src = gst_element_factory_make ("gnomevfssrc", "src");
	if (priv->src == NULL)
	{
		if (!error) return NULL;
		/* FIXME: I18N !!! */
		*error = g_error_new (AUDIO_PLAY_ERROR,
				      AUDIO_PLAY_ERROR_SRC,
				     "GStreamer: Could not create %s element",
				     "gnomevfssrc");
		return NULL;
	}

	priv->typefind = gst_element_factory_make ("typefind", "typefind");
	if (priv->typefind == NULL)
	{
		gst_object_unref (GST_OBJECT (priv->src));
		priv->src = NULL;
		if (!error) return NULL;
		*error = g_error_new (AUDIO_PLAY_ERROR,
				      AUDIO_PLAY_ERROR_TYPEFIND,
				     "GStreamer: Could not create %s element",
				     "typefind");
		return NULL;
	}
	gst_bin_add (GST_BIN (priv->pipeline), priv->src);

	/* execute the callback when we find the type */
	g_signal_connect (priv->typefind, "have_type",
		          G_CALLBACK (have_type_callback), play);

	/* catch deep notifies */
/*
	g_signal_connect (priv->pipeline, "deep_notify",
		          G_CALLBACK (deep_notify), play);
*/
	/* catch errors, they're fun ! */
	g_signal_connect (priv->pipeline, "error",
			  G_CALLBACK (error_callback), play);

	priv->have_type = FALSE;
	priv->have_eos = FALSE;
	priv->error_handler = NULL;
	priv->error_data = NULL;
	play->priv = priv;

	NM_DEBUG("audio_play_new done\n");
	audio_play_debug (play);
	return play;
}

void
audio_play_dispose (GObject *obj)
{
	AudioPlay *play;
	AudioPlayPriv *priv;

	play = AUDIO_PLAY (obj);

	NM_DEBUG("audio_play_dispose\n");
	g_assert (play != NULL);
	g_return_if_fail (IS_AUDIO_PLAY (play));
	if (gst_element_get_state (play->priv->pipeline) == GST_STATE_PLAYING)
		audio_play_playing_to_ready (play);

	NM_DEBUG("dispose: getting priv\n");
	priv = play->priv;

	NM_DEBUG("dispose: setting pipeline to NULL\n");
	gst_element_set_state (priv->pipeline, GST_STATE_NULL);

	if (priv->tick_timeout_id) {
		g_source_remove (priv->tick_timeout_id);
		priv->tick_timeout_id = 0;
	}

	if (priv->iterator_id != 0)
	{
		g_source_remove (priv->iterator_id);
		priv->iterator_id = 0;
	}
	if (priv->idler_id != 0)
	{
		g_source_remove (priv->idler_id);
		priv->idler_id = 0;
	}
	NM_DEBUG("dispose: removing decoder\n");
	if (priv->decoder)
	{
		gst_bin_remove (GST_BIN (priv->pipeline), priv->decoder);
		g_object_unref (priv->decoder);
		priv->decoder = NULL;
	}

	NM_DEBUG("dispose: unreffing typefind\n");
	if (priv->typefind)
	{
		g_object_unref (priv->typefind);
		priv->typefind = NULL;
	}

	NM_DEBUG("dispose: unreffing src\n");
	if (priv->src)
	{
		g_object_unref (priv->src);
		priv->src = NULL;
	}

	NM_DEBUG("dispose: unreffing sink\n");
	if (priv->sink)
	{
		g_object_unref (priv->sink);
		priv->sink = NULL;
	}

	/* FIXME: for some reason this one is complaining heavily
	NM_DEBUG("dispose: unreffing pipeline\n");
	if (priv->pipeline)
	{
		g_object_unref (priv->pipeline);
		priv->pipeline = NULL;
	}
	*/
	NM_DEBUG("dispose: done unreffing\n");
}

/* Free all the resources held by the object */
void
audio_play_finalize (GObject *obj)
{
	AudioPlay *play;
	AudioPlayPriv *priv;

	play = AUDIO_PLAY (obj);

	priv = play->priv;

	g_free (priv->location);
	g_free (priv);
}

/* prerequisites:
 * - no decoder in pipeline
 * - typefind exists but not yet in pipeline
 */
static gboolean
audio_play_autoplug (AudioPlay *play, GError **error)
{
	AudioPlayPriv *priv = play->priv;
#ifdef DEBUG
	NM_DEBUG("starting autoplug\n");
#endif
	g_assert (GST_IS_BIN (priv->pipeline));

	if (gst_element_get_state (priv->pipeline) != GST_STATE_READY)
	{
		g_print ("Can't autoplug, not in READY state\n");
		return FALSE;
	}
	g_assert (play->priv->decoder == NULL);

	/* put in typefind */
	NM_DEBUG ("putting in typefind\n");
	gst_bin_add (GST_BIN (priv->pipeline), priv->typefind);
	gst_element_connect (priv->src, priv->typefind);

	/* iterate pipe until type is found through callback */
	gst_element_set_state ((play->priv->pipeline), GST_STATE_PLAYING);
	/* gst_scheduler_show (GST_ELEMENT_SCHED (play->priv->pipeline)); */
	audio_play_debug (play);
	NM_DEBUG("starting iterating for autoplug\n");
	g_assert (gst_element_get_state (play->priv->pipeline) == GST_STATE_PLAYING);
	while (play->priv->have_type == FALSE)
	{
		NM_DEBUG ("*");
		if (!gst_bin_iterate (GST_BIN (play->priv->pipeline)))
		{
		       /* clean up typefind */
			gst_element_disconnect (priv->src, priv->typefind);
			g_object_ref (priv->typefind);
  			gst_bin_remove (GST_BIN (priv->pipeline), 
					priv->typefind);
		       if (!error) return FALSE;
		       *error = g_error_new (AUDIO_PLAY_ERROR,
				             AUDIO_PLAY_ERROR_AUTOPLUG,
					     "Could not find a codec for %s\n",
					     play->priv->location);
		       return FALSE;
	       }
	}
	/* we have the type, handle it */
	handle_have_type (play);
#ifdef DEBUG
	NM_DEBUG("done autoplug\n");
#endif
	return TRUE;
}

/* load the location into the play object */
gboolean
audio_play_set_location (AudioPlay *play, const char *uri, GError **error)
{
#ifdef DEBUG
	NM_DEBUG("loading location %s\n", uri);
#endif
	if (gst_element_get_state (play->priv->pipeline) == GST_STATE_PLAYING)
	{
		audio_play_playing_to_ready (play);
	}
	gst_element_set_state ((play->priv->pipeline), GST_STATE_READY);
	if (play->priv->location) g_free (play->priv->location);
	play->priv->location = g_strdup (uri);
	g_object_set (G_OBJECT (play->priv->src), "location", uri, NULL);

	/* clear values that need clearing */
	play->priv->seconds = -1; /* this ensures we get a tick at 0 too */
	play->priv->nanoseconds = 0;
	play->priv->length_nanoseconds = 0;
	play->priv->have_caps = FALSE;

	/* do a typefind */
	audio_play_autoplug (play, error);
}


/* (re)query length of track */
static void
audio_play_update_length (AudioPlay *play)
{
	gboolean res;
	GstFormat format = GST_FORMAT_TIME;
	GstPad *pad;
	gint64 value;
	GstCaps *caps;
	const GstFormatDefinition *definition = gst_format_get_details (format);

	g_assert (IS_AUDIO_PLAY (play));
	NM_DEBUG("audio_play_update_length: START\n");

	while (gst_bin_iterate (GST_BIN (play->priv->pipeline)) &&
	       play->priv->have_caps == FALSE)
		NM_DEBUG ("?");
	g_print ("\n");

	/* since the decoder is spider, we don't know the srcpad name.
	   so instead we get the sink's ghost pad, and get that one's peer */
	pad = gst_element_get_pad (play->priv->sink, "sink");
	pad = gst_pad_get_peer (pad);
	if (pad == NULL)
	{
		g_print ("WARNING: could not get sink pad of decoder !\n");
		return;
	}
	caps = gst_pad_get_caps (pad);
	if (caps == NULL) g_print ("WARNING: caps are NULL\n");

	res = gst_pad_query (pad, GST_QUERY_TOTAL, &format, &value);
	if (!res) 
	{
		g_print ("WARNING: pad query failed,can't get length !\n");
		play->priv->length_nanoseconds = FALSE;
		return;
	}
	g_assert (res == TRUE);
	g_assert (format == GST_FORMAT_TIME);
	NM_DEBUG("length in %s: %lld (nanosecs)\n", definition->nick, 
		 value);
	play->priv->length_nanoseconds = value;
	g_signal_emit (G_OBJECT (play), 
	               audio_play_signals [LENGTH_SIGNAL], 0,
		       play->priv->length_nanoseconds);
}

/* seek to given time (in nanoseconds) */
void
audio_play_seek_to_time (AudioPlay *play, guint64 nanosecs)
{
	GstEvent *event;
	gint64 seek_time;

	if (nanosecs < 0LL)
		seek_time = 0LL;
	else
		seek_time = (gint64) nanosecs;
	NM_DEBUG("audio_play_seek_to_time: seeking to %lld\n", seek_time);
	gst_element_set_state (play->priv->pipeline, GST_STATE_PAUSED);
	event = gst_event_new_seek (GST_FORMAT_TIME |
			            GST_SEEK_METHOD_SET |
				    GST_SEEK_FLAG_FLUSH, seek_time);
	/* FIXME: do we need to ref before sending ? */
	gst_event_ref (event);
	if (gst_element_send_event (play->priv->sink, event))
	{
		play->priv->nanoseconds = nanosecs;
		g_signal_emit (G_OBJECT (play),
			       audio_play_signals [TICK_SIGNAL], 0,
			       play->priv->nanoseconds);
	}
	else
	{
		g_error ("DEBUG: gst_element_send_event: FAILED\n");
	}
	gst_element_set_state (play->priv->pipeline, GST_STATE_PLAYING);
}

/* seek to given position (0.0 - 1.0) */
void
audio_play_seek_to_pos (AudioPlay *play, double value)
{
	guint64 seek_to = (guint64) ((double) play->priv->length_nanoseconds * value);
	NM_DEBUG("audio_play_seek_to_pos: seeking to %lld\n", seek_to);
	NM_DEBUG("value was %f\n", value);
	audio_play_seek_to_time (play, seek_to);
}

/* state change functions */
/* splitting these specific ones up is a good idea because it's cleaner
 * and less indented */

static gboolean
audio_play_set_state_playing (AudioPlay *play, GstElementState state,
		              GError **error)
{
	NM_DEBUG("audio_play_set_state_playing: START\n");
	if (play->priv->decoder == NULL)
	{
		audio_play_error (play, "no decoder found yet");
		return FALSE;
	}
	if (gst_element_set_state (play->priv->pipeline, GST_STATE_PLAYING) ==
				   GST_STATE_FAILURE)
	{
		/*
		 * FIXME: this error is due to a core error, so somehow
		 * find out if the core already signaled an error and if it
		 * did,  we don't need to show this one */
		/*
		audio_play_error (play, "could not set play pipeline");
		*/
		return FALSE;
	}
	audio_play_update_length (play);

	/* add idlers */
	/* handle signals asyncly */
	play->priv->idler_id =
		g_idle_add ((GSourceFunc) idler, play);
	g_assert (play->priv->idler_id > 0);
	/* set handler for playback; it will unregister
	 * when iterate fails */
	NM_DEBUG("adding iterator\n");
	play->priv->iterator_id =
		g_idle_add_full (G_PRIORITY_DEFAULT_IDLE - 10,
				 (GSourceFunc) iterator,
				 play, NULL);
	g_assert (play->priv->iterator_id > 0);
	/* set tick timeout */
	play->priv->tick_timeout_id = g_timeout_add (200, (GSourceFunc) tick_timeout, play);
	NM_DEBUG("audio_play_set_state_playing: DONE\n");
}

gboolean
audio_play_set_state (AudioPlay *play, GstElementState state, GError **error)
{
	switch (state)
	{
		case GST_STATE_PLAYING:
			return audio_play_set_state_playing (play, state, error);
			break;
		case GST_STATE_PAUSED:
			NM_DEBUG("pausing audio play\n");
			if (gst_element_set_state (play->priv->pipeline,
					           GST_STATE_PAUSED) ==
					           GST_STATE_FAILURE)
			{
				audio_play_error (play, "Could not move to "
						        "'paused' state.");
				return FALSE;
			};
			break;
		case GST_STATE_READY:
			NM_DEBUG("going to ready");
			if (play->priv->decoder)
			{
				if (!audio_play_playing_to_ready (play))
				{
					audio_play_error (play,
							  "couldn't go from playing to ready");
					return FALSE;
				}
			}
			break;
		default:
			g_print ("unhandled state\n");
			break;
	}
}

GstElementState
audio_play_get_state (AudioPlay *play)
{
	return gst_element_get_state (play->priv->pipeline);
}

guint64
audio_play_get_length (AudioPlay *play)
{
	return play->priv->length_nanoseconds;
}

/* set an audio sink on the play object to use.  It must be initialized */
void
audio_play_set_audio_sink (AudioPlay *play, GstElement *element)
{
	g_assert (GST_IS_ELEMENT (element));
	if (play->priv->sink != NULL)
	{
		g_warning ("Already have a sink !\n");
		return;
	}
	play->priv->sink = element;
	g_signal_connect (play->priv->sink, "deep_notify",
		          G_CALLBACK (deep_notify_sink), play);
	g_signal_connect (G_OBJECT (play->priv->sink), "eos",
			  G_CALLBACK (callback_sink_eos), play);
}
/* set an error handler on the play object to call on errors. */
void
audio_play_set_error_handler (AudioPlay *play,
		              AudioPlayErrorHandler handler, gpointer data)
{
	g_assert (IS_AUDIO_PLAY (play));
	play->priv->error_handler = handler;
	play->priv->error_data = data;
}

/* move from playing to ready */
static gboolean
audio_play_playing_to_ready (AudioPlay *play)
{
	AudioPlayPriv *priv = play->priv;
	/* stop the pipepline */
	gst_element_set_state (priv->pipeline, GST_STATE_READY);

	/* disconnect src ! decoder ! sink */
	gst_element_disconnect (priv->src, priv->decoder);
	gst_element_disconnect (priv->decoder, priv->sink);
	/* remove sink and decoder from the bin */
	gst_bin_remove (GST_BIN (priv->pipeline), priv->decoder);
	g_object_ref (priv->sink);
	gst_bin_remove (GST_BIN (priv->pipeline), priv->sink);

	/* null out decoder */
	priv->decoder = NULL;

	/* if idle functions are still going, remove them */
	if (priv->iterator_id != 0)
	{
		g_source_remove (priv->iterator_id);
		priv->iterator_id = 0;
	}
	if (priv->idler_id != 0)
	{
		g_source_remove (priv->idler_id);
		priv->idler_id = 0;
	}


	/* reconnect typefind to source and readd to bin */
	/*
	NM_DEBUG("reconnecting typefind\n");
	gst_element_connect (priv->src, priv->typefind);
	gst_bin_add (GST_BIN (priv->pipeline), priv->typefind);
	*/
	NM_DEBUG("play to ready done\n");
	return TRUE;
}
