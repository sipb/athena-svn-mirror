/*
 * GNOME Speech - Speech services for the GNOME desktop
 *
 * Copyright 2002 Sun Microsystems Inc.
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
 * festivalsynthesisdriver.h: Definition of the FestivalSynthesisDriver
 *                            object-- a GNOME Speech driver for the Festival
 *                            Speech Synthesis System (implementation in
 *                            festivalsynthesisdriver.c)
 *
 */

#ifndef __FESTIVAL_SYNTHESIS_DRIVER_H_
#define __FESTIVAL_SYNTHESIS_DRIVER_H_

#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>
#include "festivalspeaker.h"

#define FESTIVAL_SYNTHESIS_DRIVER_TYPE        (festival_synthesis_driver_get_type ())
#define FESTIVAL_SYNTHESIS_DRIVER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), FESTIVAL_SYNTHESIS_DRIVER_TYPE, FestivalSynthesisDriver))
#define FESTIVAL_SYNTHESIS_DRIVER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), FESTIVAL_SYNTHESIS_DRIVER_TYPE, FestivalSynthesisDriverClass))
#define IS_FESTIVAL_SYNTHESIS_DRIVER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), FESTIVAL_SYNTHESIS_DRIVER_TYPE))
#define IS_FESTIVAL_SYNTHESIS_DRIVER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), FESTIVAL_SYNTHESIS_DRIVER_TYPE))

typedef struct {
	BonoboObject parent;

	pid_t pid;
	gchar *version;
	gboolean initialized;
	FestivalSpeaker *last_speaker;
	int sock;
	int pipe;
	GIOChannel *channel_sock;
	GIOChannel *channel_pipe;
	gboolean is_shutting_up; 
	gboolean is_speaking;
	GSList *list;
	gboolean is_querying; 
	gint crt_id;
	GSList *crt_clbs;
	int queue_length;
} FestivalSynthesisDriver;

typedef struct {
	BonoboObjectClass parent_class;
	POA_GNOME_Speech_SynthesisDriver__epv epv;
} FestivalSynthesisDriverClass;

GType
festival_synthesis_driver_get_type   (void);

FestivalSynthesisDriver *
festival_synthesis_driver_new (void);
gint
festival_synthesis_driver_say (FestivalSynthesisDriver *d,
			       FestivalSpeaker *s,
			       gchar *text);
gboolean
festival_synthesis_driver_stop (FestivalSynthesisDriver *d);
gboolean
festival_synthesis_driver_is_speaking (FestivalSynthesisDriver *d);
void
festival_synthesis_driver_say_raw (FestivalSynthesisDriver *d,
				   gchar *text);


  
#endif /* __FESTIVAL_SYNTHESIS_DRIVER_H_ */
