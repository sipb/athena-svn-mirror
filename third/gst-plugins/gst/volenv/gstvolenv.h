/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
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


#ifndef __GST_VOLENV_H__
#define __GST_VOLENV_H__


#include <gst/gst.h>
/* #include <gst/meta/audioraw.h> */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_VOLENV \
  (gst_volenv_get_type())
#define GST_VOLENV(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VOLENV,GstVolEnv))
#define GST_VOLENV_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ULAW,GstVolEnv))
#define GST_IS_VOLENV(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VOLENV))
#define GST_IS_VOLENV_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VOLENV))

typedef struct _GstVolEnv GstVolEnv;
typedef struct _GstVolEnvClass GstVolEnvClass;

struct _GstVolEnv {
  GstElement element;

  GstPad *sinkpad,*srcpad;
  double run_time;		/* runtime in seconds */
  double level;			/* current volume level 1.0 = 0dB */
  double arg_rise;      /* rise value passed in as object argument */
  const gchar *control_point; /* control point passed in as object argument */
  double rise;			/* rise in level per second */
  double increase;		/* increase in level per sample */
  gboolean envelope_active;	/* are we using an envelope or not ? */
  GList *envelope;
  GList *next_cp;
  double next_time;		/* what time is the next control point ? */
  double next_level;		/* what level is the next control point ? */

  /*MetaAudioRaw meta; */

};

struct _GstVolEnvClass {
  GstElementClass parent_class;
};

GType gst_volenv_get_type(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_STEREO_H__ */
