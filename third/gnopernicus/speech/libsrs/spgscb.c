/* srsgscb.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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
#include <glib.h>
#include <bonobo/Bonobo.h>
#include "spgscb.h"
#include "SRMessages.h"
#include <libbonobo.h>
#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>

#define CALLBACK_TYPE        (callback_get_type ())
#define CALLBACK(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), CALLBACK_TYPE, CALLBACK))
#define CALLBACK_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), CALLBACK_TYPE, CallbackClass))
#define IS_CALLBACK(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), CALLBACK_TYPE))
#define IS_CALLBACK_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), CALLBACK_TYPE))

typedef struct 
{
    BonoboObject parent;
}Callback;

typedef struct 
{
  BonoboObjectClass 			parent_class;
  POA_GNOME_Speech_SpeechCallback__epv	epv;
} CallbackClass;

GType callback_get_type (void);
/*FIXME*/

static GObjectClass *parent_class;

/* UGLY but I don't know bonobo so... */
static SRSGSMarkersCallback     cl_callback;


static void 
callback_notify (PortableServer_Servant 			servant,
		 const GNOME_Speech_speech_callback_type 	type,
		 const CORBA_long 				text_id,
		 const CORBA_long 				offset,
		 CORBA_Environment 				*ev)
{
    sru_assert (cl_callback);

    cl_callback(type, text_id, -1); /*-1 because for moment no speech-progress markers */
}

static void
callback_class_init (CallbackClass *klass)
{
    /* GObjectClass * object_class = G_OBJECT_CLASS(klass); */
    POA_GNOME_Speech_SpeechCallback__epv *epv = &klass->epv;
    parent_class = g_type_class_peek_parent (klass);

    /* Setup entry point vector table */
    epv->notify = callback_notify;
}


static void
callback_init (Callback	*c)
{
}
/*
static void
callback_finalize (GObject *object)
{
    fprintf (stderr,"\nFinalize");
    parent_class->finalize (object);
}
*/

Callback *
callback_new (SRSGSMarkersCallback callback)
{
    Callback *cb;

    sru_assert (callback);
    cl_callback	= callback;
    cb 		= g_object_new (CALLBACK_TYPE, NULL);
	
    return cb;
}


BONOBO_TYPE_FUNC_FULL (Callback,
		       GNOME_Speech_SpeechCallback,
		       bonobo_object_get_type (),
		       callback);


gboolean
srs_gs_cb_register_callback (GNOME_Speech_Speaker speaker,
			     SRSGSMarkersCallback func)
{
    BonoboObject *bo;
    Callback *callback;
    gboolean rv;
    CORBA_Environment ev;

    sru_assert (speaker && func);

    callback = callback_new (func);
    sru_assert (callback);
    bo = BONOBO_OBJECT (callback);

    CORBA_exception_init (&ev);
    rv = GNOME_Speech_Speaker_registerSpeechCallback (speaker,
				bonobo_object_corba_objref (bo),
				&ev);
    if (ev._major != CORBA_NO_EXCEPTION && BONOBO_EX (&ev))
    {
	gchar *error = bonobo_exception_get_text (&ev);
	/* sru_warning ("exception %s occured", error); */
	sru_warning ("Message : %s", "Unable to register the callback");
	bonobo_object_unref (bo);
	g_free (error);

	CORBA_exception_free (&ev);
	rv = FALSE; /* FIXME: in gnome-speech */
    }

    /* GNOME_Speech_SpeechCallback_unref (callback, srs_gs_ev ()); */
    /* GNOME_Speech_SpeechCallback_unref (bo, srs_gs_ev ()); */
    /* srs_gs_check_ev ("Unable to unref the callback"); */
    /* bonobo_object_unref (callback); */    
    return rv;
}
