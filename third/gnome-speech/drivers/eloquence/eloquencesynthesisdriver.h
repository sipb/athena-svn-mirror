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
 * eloquencesynthesisdriver.h: Definition of  the EloquenceSynthesisDriver
 *                           object-- a GNOME Speech driver for SpeechWorks's
 *                           Eloquence TTS SDK (implementation in
 *                           eloquencesynthesisdriver.c)
 *
 */


#ifndef __ELOQUENCE_SYNTHESIS_DRIVER_H_
#define __ELOQUENCE_SYNTHESIS_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>

#define ELOQUENCE_SYNTHESIS_DRIVER_TYPE        (eloquence_synthesis_driver_get_type ())
#define ELOQUENCE_SYNTHESIS_DRIVER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), ELOQUENCE_SYNTHESIS_DRIVER_TYPE, EloquenceSynthesisDriver))
#define ELOQUENCE_SYNTHESIS_DRIVER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), eloquence_SYNTHESIS_driver_TYPE, EloquenceSynthesisDriverClass))
#define ELOQUENCE_SYNTHESIS_DRIVER_GET_CLASS(obj)                 (G_TYPE_INSTANCE_GET_CLASS ((obj), ELOQUENCE_SYNTHESIS_DRIVER_TYPE, EloquenceSynthesisDriverClass))
#define IS_ELOQUENCE_SYNTHESIS_DRIVER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), ELOQUENCE_SYNTHESIS_DRIVER_TYPE))
#define IS_ELOQUENCE_SYNTHESIS_DRIVER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), ELOQUENCE_SYNTHESIS_DRIVER_TYPE))

typedef struct {
  BonoboObject parent;
} EloquenceSynthesisDriver;

typedef struct {
  BonoboObjectClass parent_class;

  POA_GNOME_Speech_SynthesisDriver__epv epv;
} EloquenceSynthesisDriverClass;

GType
eloquence_synthesis_driver_get_type   (void);

EloquenceSynthesisDriver *
eloquence_synthesis_driver_new (void);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ELOQUENCE_SYNTHESIS_DRIVER_H_ */
