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
 * callback.h: Implementation of test callback.
 *
 */

#ifndef _CALLBACK_H_
#define _CALLBACK_H_


#include <bonobo/bonobo-object.h>
#include <gnome-speech.h>


#define CALLBACK_TYPE        (callback_get_type ())
#define CALLBACK(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), CALLBACK_TYPE, CALLBACK))
#define CALLBACK_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), CALLBACK_TYPE, CallbackClass))
#define IS_CALLBACK(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), CALLBACK_TYPE))
#define IS_CALLBACK_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), CALLBACK_TYPE))

typedef struct {
  BonoboObject parent;
} Callback;

typedef struct {
  BonoboObjectClass parent_class;
  POA_GNOME_Speech_SpeechCallback__epv epv;
} CallbackClass;

GType
callback_get_type   (void);
Callback *
callback_new (void);


#endif /* _SPEAKER_H_ */
