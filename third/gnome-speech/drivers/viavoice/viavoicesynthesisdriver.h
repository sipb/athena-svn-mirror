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
 * viavoicesynthesisdriver.h: Definition of  the ViavoiceSynthesisDriver
 *                            object-- a GNOME Speech driver for IBM's
 *                            Viavoice TTS RTK (implementation in
 *                            viavoicesynthesisdriver.c)
 *
 */


#ifndef __VIAVOICE_SYNTHESIS_DRIVER_H_
#define __VIAVOICE_SYNTHESIS_DRIVER_H_

#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>
#include "viavoicespeaker.h"

#define VIAVOICE_SYNTHESIS_DRIVER_TYPE        (viavoice_synthesis_driver_get_type ())
#define VIAVOICE_SYNTHESIS_DRIVER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), VIAVOICE_SYNTHESIS_DRIVER_TYPE, ViavoiceSynthesisDriver))
#define VIAVOICE_SYNTHESIS_DRIVER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), viavoice_SYNTHESIS_driver_TYPE, ViavoiceSynthesisDriverClass))
#define VIAVOICE_SYNTHESIS_DRIVER_GET_CLASS(obj)                 (G_TYPE_INSTANCE_GET_CLASS ((obj), VIAVOICE_SYNTHESIS_DRIVER_TYPE, ViavoiceSynthesisDriverClass))
#define IS_VIAVOICE_SYNTHESIS_DRIVER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), VIAVOICE_SYNTHESIS_DRIVER_TYPE))
#define IS_VIAVOICE_SYNTHESIS_DRIVER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), VIAVOICE_SYNTHESIS_DRIVER_TYPE))

typedef struct {
	BonoboObject parent;

	ECIHand handle;

	ViavoiceSpeaker *last_speaker;
	guint timeout_id;
	GSList *index_queue;
	gboolean initialized;
} ViavoiceSynthesisDriver;

typedef struct {
	BonoboObjectClass parent_class;

	POA_GNOME_Speech_SynthesisDriver__epv epv;
} ViavoiceSynthesisDriverClass;

GType
viavoice_synthesis_driver_get_type   (void);

ViavoiceSynthesisDriver *
viavoice_synthesis_driver_new (void);
gint
viavoice_synthesis_driver_say (ViavoiceSynthesisDriver *d,
			       ViavoiceSpeaker *s,
			       gchar *text);
gboolean
viavoice_synthesis_driver_stop (ViavoiceSynthesisDriver *d);
void
viavoice_synthesis_driver_wait (ViavoiceSynthesisDriver *d);
gboolean
viavoice_synthesis_driver_is_speaking (ViavoiceSynthesisDriver *d);
void
viavoice_synthesis_driver_set_voice_param (const ViavoiceSynthesisDriver *d,
					   ECIVoiceParam param,
					   gint new_value);
gboolean
viavoice_synthesis_driver_set_voice (const ViavoiceSynthesisDriver *d,
				     gint v);

					   
#endif /* __VIAVOICE_SYNTHESIS_DRIVER_H_ */
