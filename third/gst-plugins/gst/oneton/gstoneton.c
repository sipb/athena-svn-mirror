/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@prettypeople.org>
 *
 *  Copyright 2002 Iain Holmes
 *  Based one stereosplit: Richard Boulton <richard@tartarus.org>
 *  Based on stereo2mono: Zaheer Merali <zaheer@bellworldwide.net>
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

#include <config.h>
#include <gst/gst.h>

#define GST_TYPE_ONETON (gst_oneton_get_type ())
#define GST_ONETON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_ONETON, GstOneToN))
#define GST_ONETON_CLASS(klass) (G_TYPE_CHECK_CLASS_CASE ((klass), GST_TYPE_ONTON, GstOneToNClass))
#define GST_IS_ONETON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_ONETON))
#define GST_IS_ONETON_CLASS (klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_ONETON))

#if 0
#define DEBUG {g_print ("In function %s\n", __FUNCTION__);}
#else
#define DEBUG
#endif

typedef struct _GstOneToN GstOneToN;
typedef struct _GstOneToNClass GstOneToNClass;

struct _GstOneToN {
	GstElement element;

	GstPad *sinkpad;

	int channels;
	int width;
	
	GList *srcpads;
};

struct _GstOneToNClass {
	GstElementClass parent_class;
};

static GstElementDetails oneton_details = {
	"N Channel splitter",
	"Filter/Audio/Conversion",
	"LGPL",
	"Converts N Channel PCM to N mono PCM streams",
	VERSION,
	"Iain <iain@prettypeople.org>",
	"Copyright (C) 2002",
};

/* Signals and args */
enum {
	LAST_SIGNAL
};

enum {
	ARG_0,
};

static GstPadTemplate *
oneton_sink_factory (void)
{
	return gst_pad_template_new (
		"sink",
		GST_PAD_SINK,
		GST_PAD_ALWAYS,
		gst_caps_new (
			"int_n_channel_sink",
			"audio/raw",
			gst_props_new (
				"format", GST_PROPS_STRING ("int"),
				"law", GST_PROPS_INT (0),
				"endianness", GST_PROPS_INT (G_BYTE_ORDER),
				"signed", GST_PROPS_BOOLEAN (TRUE),
				"width", GST_PROPS_INT (16),
				"depth", GST_PROPS_INT (16),
				"rate", GST_PROPS_INT_RANGE (4000, 96000),
				"channels", GST_PROPS_INT_RANGE (1, 4),
				NULL)),
		NULL);
}

static GstPadTemplate *
oneton_src_factory (void)
{
	return gst_pad_template_new (
		"src_%d",
		GST_PAD_SRC,
		GST_PAD_SOMETIMES,
		gst_caps_new (
			"int_n_channel_src",
			"audio/raw",
			gst_props_new (
				"format", GST_PROPS_STRING ("int"),
				"law", GST_PROPS_INT (0),
				"endianness", GST_PROPS_INT (G_BYTE_ORDER),
				"signed", GST_PROPS_BOOLEAN (TRUE),
				"width", GST_PROPS_INT (16),
				"depth", GST_PROPS_INT (16),
				"rate", GST_PROPS_INT_RANGE (4000, 96000),
				"channels", GST_PROPS_INT (1),
				NULL)),
		NULL);
}

GType gst_oneton_get_type (void);

static void gst_oneton_class_init (GstOneToNClass *klass);
static void gst_oneton_init (GstOneToN *oneton);
static void gst_oneton_chain (GstPad *pad,
			      GstBuffer *buf);
static void inline gst_oneton_fast_16bit_chain (gint16 *data,
						int channels,
						gint16 **mono_data,
						guint numbytes);
static void inline gst_oneton_fast_8bit_chain (gint8 *data,
					       int channels,
					       gint8 **mono_data,
					       guint numbytes);



static GstPadTemplate *sinktemplate, *srctemplate;
static GstElementClass *parent_class = NULL;

static GstPadLinkReturn
gst_oneton_connect (GstPad *pad,
		    GstCaps *caps)
{
	GstOneToN *oneton;

	oneton = GST_ONETON (gst_pad_get_parent (pad));

	DEBUG;
	g_return_val_if_fail (GST_IS_ONETON (oneton), GST_PAD_LINK_REFUSED);

	if (!GST_CAPS_IS_FIXED (caps)) {
		return GST_PAD_LINK_DELAYED;
	}

	if (pad == oneton->sinkpad) {
		int i;
		GstCaps *srccaps;
		GList *p;

		/* Get the number of channels coming in */
		gst_caps_get_int (caps, "channels", &oneton->channels);
		if (oneton->channels == 0) {
			return GST_PAD_LINK_DELAYED;
		}

		srccaps = gst_caps_copy (caps);
		gst_caps_set (srccaps, "channels", GST_PROPS_INT (1));
		gst_caps_get_int (caps, "width", &oneton->width);

		/* Disconnect any pads that already exist */
		for (p = oneton->srcpads; p; p = p->next) {
			GstPad *peer, *pad = p->data;

			peer = GST_PAD_PEER (pad);
			
			/* Check if they're connected */
			if (peer) {
				gst_pad_unlink (pad, peer);
			}

			gst_element_remove_pad (GST_ELEMENT (oneton), pad);
		}
		g_list_free (oneton->srcpads);
		oneton->srcpads = NULL;
		
		/* Create that number of src pads */
		for (i = 0; i < oneton->channels; i++) {
			GstPad *pad;
			char *pad_name;

			pad_name = g_strdup_printf ("src_%d", i);
			pad = gst_pad_new_from_template (srctemplate, pad_name);
			g_free (pad_name);

			if (!gst_pad_try_set_caps (pad, srccaps)) {
				return GST_PAD_LINK_REFUSED;
			}
			gst_element_add_pad (GST_ELEMENT (oneton), pad);

			oneton->srcpads = g_list_append (oneton->srcpads, pad);
		}
	}

	return GST_PAD_LINK_OK;
}

GType
gst_oneton_get_type (void) {
	static GType type = 0;

	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (GstOneToNClass), NULL, NULL,
			(GClassInitFunc) gst_oneton_class_init, NULL, NULL,
			sizeof (GstOneToN), 0, (GInstanceInitFunc) gst_oneton_init,
		};

		type = g_type_register_static (GST_TYPE_ELEMENT,
					       "GstOneToN", &info, 0);
	}

	return type;
}

static void
gst_oneton_class_init (GstOneToNClass *klass)
{
	GObjectClass *gobject_class;
	GstElementClass *gstelement_class;

	gobject_class = (GObjectClass *)klass;
	gstelement_class = (GstElementClass *)klass;

	parent_class = g_type_class_ref (GST_TYPE_ELEMENT);
}

static void
gst_oneton_init (GstOneToN *oneton)
{
	DEBUG;
	oneton->sinkpad = gst_pad_new_from_template (sinktemplate, "sink");
	gst_pad_set_chain_function (oneton->sinkpad, gst_oneton_chain);
	gst_pad_set_link_function (oneton->sinkpad, gst_oneton_connect);
	gst_element_add_pad (GST_ELEMENT (oneton), oneton->sinkpad);

	oneton->channels = 0;
	oneton->width = 0;
	oneton->srcpads = NULL;
}

static void
gst_oneton_chain (GstPad *pad,
		  GstBuffer *buf)
{
	GstOneToN *oneton;
	gint16 *data;
	gint16 **mono_data;
	GstBuffer **out_bufs;
	GList *p;
	int i;
	
	DEBUG;
	g_return_if_fail (GST_IS_PAD (pad));
	g_return_if_fail (buf != NULL);

	oneton = GST_ONETON (gst_pad_get_parent (pad));
	g_return_if_fail (GST_IS_ONETON (oneton));

	if (GST_IS_EVENT (buf)) {
		GList *p;

		for (p = oneton->srcpads; p; p = p->next) {
			GstPad *pad = p->data;
			
			gst_pad_event_default (pad, GST_EVENT (buf));
		}

		return;
	}

	if (oneton->channels == 1) {
		gst_pad_push (GST_PAD (oneton->srcpads->data), buf);
		return;
	}
	
	data = (gint16 *) GST_BUFFER_DATA (buf);

	out_bufs = g_new (GstBuffer *, oneton->channels);
	mono_data = g_new (gint16 *, oneton->channels);

	/* Create our buffers */
	for (i = 0; i < oneton->channels; i++) {
		out_bufs[i] = gst_buffer_new ();
		GST_BUFFER_DATA (out_bufs[i]) = (char *) g_new (gint16, GST_BUFFER_SIZE (buf) / (2 * oneton->channels));
		GST_BUFFER_SIZE (out_bufs[i]) = GST_BUFFER_SIZE (buf) / oneton->channels;
		GST_BUFFER_OFFSET (out_bufs[i]) = GST_BUFFER_OFFSET (buf);
		GST_BUFFER_TIMESTAMP (out_bufs[i]) = GST_BUFFER_TIMESTAMP (buf);

		mono_data[i] = (gint16 *) GST_BUFFER_DATA (out_bufs[i]);
	}

	switch (oneton->width) {
	case 16:
		gst_oneton_fast_16bit_chain (data, oneton->channels,
					     mono_data, GST_BUFFER_SIZE (buf));
		break;

	case 8:
		gst_oneton_fast_8bit_chain ((gint8 *) data, oneton->channels,
					    (gint8 **) mono_data,
					    GST_BUFFER_SIZE (buf));
		break;

	default:
		gst_element_error (GST_ELEMENT (oneton),
				   "oneton: capsnego was never performed, bailing...");
		return;
	}

	gst_buffer_unref (buf);

	for (i = 0, p = oneton->srcpads; p; p = p->next, i++) {
		GstPad *pad = p->data;

		gst_pad_push (pad, out_bufs[i]);
	}

	g_free (out_bufs);
	g_free (mono_data);
}

static void inline
gst_oneton_fast_16bit_chain (gint16 *data,
			     int channels,
			     gint16 **mono_data,
			     guint numbytes)
{
	guint i, j, k;

	DEBUG;
	for (i = 0, j = 0; i < numbytes / channels; i += channels, j++) {
		for (k = 0; k < channels; k++) {
			mono_data[k][j] = data[i + k];
		}
	}
}

static void inline
gst_oneton_fast_8bit_chain (gint8 *data,
			    int channels,
			    gint8 **mono_data,
			    guint numbytes)
{
	guint i, j, k;

	DEBUG;
	for (i = 0, j = 0; i < numbytes / channels; i += channels, j++) {
		for (k = 0; k < channels; k++) {
			mono_data[k][j] = data[i + k];
		}
	}
}

static gboolean
plugin_init (GModule *module,
	     GstPlugin *plugin)
{
	GstElementFactory *factory;
	
	DEBUG;
	factory = gst_element_factory_new ("oneton", GST_TYPE_ONETON,
					   &oneton_details);
	g_return_val_if_fail (factory != NULL, FALSE);

	srctemplate = oneton_src_factory ();
	gst_element_factory_add_pad_template (factory, srctemplate);

	sinktemplate = oneton_sink_factory ();
	gst_element_factory_add_pad_template (factory, sinktemplate);

	gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

	return TRUE;
}

GstPluginDesc plugin_desc = {
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	"oneton",
	plugin_init
};
