/*
 * GNOME Speech - Speech services for the GNOME desktop
 *
 * Copyright 2003 Sun Microsystems Inc.
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
 *
 * dectalksynthesisdriver.h: Definition of the DectalkSynthesisDriver
 *                           object-- a GNOME Speech driver for Fonix's
 *                           DECtalk TTS SDK (implementation in
 *                           dectalksynthesisdriver.c)
 *
 */

#ifndef __DECTALK_SYNTHESIS_DRIVER_H_
#define __DECTALK_SYNTHESIS_DRIVER_H_

#include <glib/gthread.h>
#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>
#include <dtk/ttsapi.h>
#include "dectalkspeaker.h"

#define DECTALK_SYNTHESIS_DRIVER_TYPE        (dectalk_synthesis_driver_get_type ())
#define DECTALK_SYNTHESIS_DRIVER(o)          (G_TYPE_CHECK_INSTANCE_CAST((o), DECTALK_SYNTHESIS_DRIVER_TYPE, DectalkSynthesisDriver))
#define DECTALK_SYNTHESIS_DRIVER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), dectalk_SYNTHESIS_driver_TYPE, DectalkSynthesisDriverClass))
#define DECTALK_SYNTHESIS_DRIVER_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS((obj), DECTALK_SYNTHESIS_DRIVER_TYPE, DectalkSynthesisDriverClass))
#define IS_DECTALK_SYNTHESIS_DRIVER(o)       (G_TYPE_CHECK_INSTANCE_TYPE((o), DECTALK_SYNTHESIS_DRIVER_TYPE))
#define IS_DECTALK_SYNTHESIS_DRIVER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), DECTALK_SYNTHESIS_DRIVER_TYPE))

typedef struct {
	BonoboObject parent;

	/* Protect index queue */

	GMutex *mutex;
	
	gboolean initialized;

	LPTTS_HANDLE_T handle;

	gchar *text;
	guint text_idle;

        /* Callback stuff */

	guint timeout_id;
	GSList *index_queue;
	GSList *post_queue;

	/* Last speaker which spoke */

	DectalkSpeaker *last_speaker;
} DectalkSynthesisDriver;

typedef struct {
	BonoboObjectClass parent_class;

	POA_GNOME_Speech_SynthesisDriver__epv epv;
} DectalkSynthesisDriverClass;

#include "dectalkspeaker.h"

GType
dectalk_synthesis_driver_get_type   (void);

DectalkSynthesisDriver *
dectalk_synthesis_driver_new (void);
gint
dectalk_synthesis_driver_say (DectalkSynthesisDriver *driver,
			      DectalkSpeaker *s,
			      gchar *text);
void
dectalk_synthesis_driver_speak_raw (DectalkSynthesisDriver *driver,
				    gchar *text);
gboolean
dectalk_synthesis_driver_stop (DectalkSynthesisDriver *driver);
void
dectalk_synthesis_driver_wait (DectalkSynthesisDriver *d);
gboolean
dectalk_synthesis_driver_is_speaking (DectalkSynthesisDriver *d);

#endif /* __DECTALK_SYNTHESIS_DRIVER_H_ */
