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
 * callback.c: Implementation of sample callback.
 *
 */

#include <glib.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include "callback.h"



static GObjectClass *parent_class;



static void 
callback_notify (PortableServer_Servant servant,
		 const GNOME_Speech_speech_callback_type type,
		 const CORBA_long text_id,
		 const CORBA_long offset,
		 CORBA_Environment *ev)
{
	switch (type) 
	{
	    case GNOME_Speech_speech_callback_speech_started:
		fprintf (stderr, "Speech started.\n");
		break;
	    case GNOME_Speech_speech_callback_speech_progress:
		    fprintf (stderr, "Speech progress %d.\r", offset);
		break;
	    case GNOME_Speech_speech_callback_speech_ended:
		fprintf (stderr, "\r\nSpeech ended.\n");
		bonobo_main_quit ();
		break;
	    default:
		break;
	}
	return;
}



Callback *
callback_new (void)
{
	Callback *cb = g_object_new (CALLBACK_TYPE, NULL);
	return cb;
}



static void
callback_init (Callback *c)
{
}

static void
callback_finalize (GObject *object)
{
    fprintf (stderr,"\nFinalize!\n");
    parent_class->finalize (object);
}




static void
callback_class_init (CallbackClass *klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS(klass);
  POA_GNOME_Speech_SpeechCallback__epv *epv = &klass->epv;
  parent_class = g_type_class_peek_parent (klass);
  object_class->finalize = callback_finalize;
  /* Setup epv table */

  epv->notify = callback_notify;
}


BONOBO_TYPE_FUNC_FULL (Callback,
		       GNOME_Speech_SpeechCallback,
		       bonobo_object_get_type (),
		       callback);
