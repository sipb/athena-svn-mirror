/* srs-gs-wrap.c
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
#include "srs-gs-wrap.h"
#include <libbonobo.h>
#include <gnome-speech/gnome-speech.h>
#include <string.h>
#include "SRMessages.h"
#include <spgscb.h>

typedef struct
{
    GNOME_Speech_SynthesisDriver  driver;
    gchar			 *name;
    GNOME_Speech_VoiceInfoList 	 *voices;
}SRSGSWrapDriver;


static SRSGSWrapCallback  srs_gs_wrap_callback = NULL;
static GPtrArray	 *srs_gs_wrap_drivers  = NULL;
static CORBA_Environment  srs_gs_wrap_ev       = {0};

#define srs_gs_wrap_return_if_ev(err)			\
		    if (!srs_gs_wrap_check_ev (err))	\
			return;

#define srs_gs_wrap_return_val_if_ev(err, val)		\
		if (!srs_gs_wrap_check_ev (err))	\
			return (val);

static CORBA_Environment*
srs_gs_wrap_get_ev (void)
{
    CORBA_exception_init (&srs_gs_wrap_ev);
    return &srs_gs_wrap_ev;
}

static gboolean
srs_gs_wrap_check_ev (gchar *message)
{
    CORBA_Environment *ev = &srs_gs_wrap_ev;
    
    if (ev && ev->_major != CORBA_NO_EXCEPTION && BONOBO_EX (ev))
    {
	gchar *error;
	
	error = bonobo_exception_get_text (ev);

	sru_warning ("Exception %s occured.\n", error);
	sru_warning ("Message : %s", message);
	g_free (error);
	
	CORBA_exception_free (ev);
	
	return FALSE;
    }
    return TRUE;
}

static void
srs_gs_wrap_gsdriver_unref (GNOME_Speech_SynthesisDriver driver)
{
    sru_assert (driver);

    GNOME_Speech_SynthesisDriver_unref (driver, srs_gs_wrap_get_ev ());
    srs_gs_wrap_check_ev ("Unable to unref the driver");
}

static void
srs_gs_wrap_gsvoiceslist_free (GNOME_Speech_VoiceInfoList *voices)
{
    sru_assert (voices);

    CORBA_free (voices); /* more free calls here ?!? */
}

static void
srs_gs_wrap_gsserverlist_free (Bonobo_ServerInfoList *servers)
{
    sru_assert (servers);
    CORBA_free (servers); /* more free calls here ?!? */
}

static void
srs_gs_wrap_gsparameterlist_free (GNOME_Speech_ParameterList *parameters)
{
    sru_assert (parameters);
    CORBA_free (parameters); /* more free calls here ?!? */
}

static gboolean
srs_gs_wrap_bonobo_init ()
{
    if (!bonobo_init (NULL, NULL))
    {
	sru_warning ("Bonobo initialization failed.");
	return FALSE;
    }

    return TRUE;
}

static void
srs_gs_wrap_bonobo_terminate ()
{
    bonobo_debug_shutdown ();
}

static GNOME_Speech_SynthesisDriver
srs_gs_wrap_get_activated_server_from_server_info (Bonobo_ServerInfo *info)
{
    CORBA_Object obj = CORBA_OBJECT_NIL;

    sru_assert (info);

    obj = bonobo_activation_activate_from_id (info->iid, 
		  0, NULL, srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("Unable to activate server", NULL);

    if (!GNOME_Speech_SynthesisDriver_driverInit (obj, srs_gs_wrap_get_ev ()) ||
	!srs_gs_wrap_check_ev ("Server activation  failed."))
    {
    	srs_gs_wrap_gsdriver_unref (obj);
	obj = NULL;
    }

    return obj;
}
    

static GNOME_Speech_VoiceInfoList*
srs_gs_wrap_get_all_voices_from_driver (GNOME_Speech_SynthesisDriver driver)
{
    GNOME_Speech_VoiceInfoList *voices;
    gint i;
    
    sru_assert (driver);
    
    voices = GNOME_Speech_SynthesisDriver_getAllVoices (driver, srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("Unable to get voices for driver", NULL);

    for (i = 0; i < voices->_length; i++)
    {	
	if (voices->_buffer[i].name && voices->_buffer[i].name[0])
	    return voices;
    }
    srs_gs_wrap_gsvoiceslist_free (voices);

    return NULL; 
}

static gchar*
srs_gs_wrap_driver_get_name (GNOME_Speech_SynthesisDriver driver)
{
    CORBA_string name;
    gchar *rv = NULL;

    sru_assert (driver);
    name = GNOME_Speech_SynthesisDriver__get_driverName (driver,
				srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("Unable to get driver name", NULL);

    rv = g_strdup (name);
    CORBA_free (name);
    
    return rv;
}

static SRSGSWrapDriver* srs_gs_wrap_driver_new ();

static GPtrArray*
srs_gs_wrap_get_drivers_from_servers (Bonobo_ServerInfoList *servers)
{
    gint	i;
    GPtrArray 	*drivers;
    
    drivers = g_ptr_array_new ();
    
    for (i = 0; i < servers->_length; i++)
    {
	GNOME_Speech_SynthesisDriver 	gsd = NULL;
	GNOME_Speech_VoiceInfoList	*voices = NULL;
	gchar *name = NULL;
	gboolean ok = FALSE;

        gsd = srs_gs_wrap_get_activated_server_from_server_info (&servers->_buffer[i]); 
	if (gsd)
	    voices = srs_gs_wrap_get_all_voices_from_driver (gsd);
	if (voices)
	    name = srs_gs_wrap_driver_get_name (gsd);
	if (name)
	{
	    SRSGSWrapDriver	*driver;
	    driver = srs_gs_wrap_driver_new ();
	    driver->driver = gsd;
	    driver->name = name;
	    driver->voices = voices;
	    g_ptr_array_add (drivers, driver);
	    ok = TRUE;
	}
	if (!ok)
	{
	    if (gsd)
		srs_gs_wrap_gsdriver_unref (gsd);
	    if (voices)
		srs_gs_wrap_gsvoiceslist_free (voices);
	    g_free (name);
	}
    }

    if (drivers->len > 0)
	return drivers;
    g_ptr_array_free (drivers, TRUE);
    return NULL;
}

static Bonobo_ServerInfoList*
srs_gs_wrap_get_gsserverslist ()
{
    Bonobo_ServerInfoList *servers = CORBA_OBJECT_NIL;
    servers = bonobo_activation_query (
			"repo_ids.has ('IDL:GNOME/Speech/SynthesisDriver:0.3')",
			NULL, srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("Activation Error!", NULL);

    return servers;
}



static SRSGSWrapDriver*
srs_gs_wrap_driver_new ()
{
    return g_new0 (SRSGSWrapDriver, 1);
}

static void
srs_gs_wrap_driver_terminate(SRSGSWrapDriver *driver)
{
    sru_assert (driver);

    if (driver->driver)
	srs_gs_wrap_gsdriver_unref (driver->driver);
    g_free (driver->name);
    srs_gs_wrap_gsvoiceslist_free (driver->voices);
    g_free (driver);
}

gboolean 
srs_gs_wrap_init (SRSGSWrapCallback callback)
{
    gboolean rv = FALSE;
    sru_assert (callback);

    srs_gs_wrap_callback = callback;
    srs_gs_wrap_drivers  = NULL;
    CORBA_exception_init (&srs_gs_wrap_ev);
    if (srs_gs_wrap_bonobo_init ())
    {
	Bonobo_ServerInfoList* servers;

	servers = srs_gs_wrap_get_gsserverslist ();
	if (servers)
	{
	    srs_gs_wrap_drivers = srs_gs_wrap_get_drivers_from_servers (servers);
	    rv = srs_gs_wrap_drivers != NULL;
    	    srs_gs_wrap_gsserverlist_free (servers);
	}
	if (!rv)
	    srs_gs_wrap_bonobo_terminate ();
    }
    return rv;
}

void
srs_gs_wrap_terminate ()
{
    gint i;

    sru_assert (srs_gs_wrap_drivers);
    for (i = 0; i < srs_gs_wrap_drivers->len; i++)
	srs_gs_wrap_driver_terminate (g_ptr_array_index (srs_gs_wrap_drivers, i));
    g_ptr_array_free (srs_gs_wrap_drivers, TRUE);
    CORBA_exception_free (&srs_gs_wrap_ev);
    srs_gs_wrap_bonobo_terminate ();
}

gchar**
srs_gs_wrap_get_drivers ()
{
    GPtrArray *drvs;
    gint i;
    
    sru_assert (srs_gs_wrap_drivers && srs_gs_wrap_drivers->len > 0);
    
    drvs = g_ptr_array_new (); 
    for (i = 0; i < srs_gs_wrap_drivers->len; i++)
    {
	SRSGSWrapDriver *drv = g_ptr_array_index (srs_gs_wrap_drivers, i);
	sru_assert (drv);
	g_ptr_array_add (drvs, g_strdup (drv->name));
    }

    g_ptr_array_add (drvs, NULL);
    return (gchar**) g_ptr_array_free(drvs, FALSE);
}

static SRSGSWrapDriver*
srs_gs_wrap_get_driver (gchar *driver)
{
    gint i;

    sru_assert (srs_gs_wrap_drivers && srs_gs_wrap_drivers->len > 0);
    sru_assert (driver);

    for (i = 0; i < srs_gs_wrap_drivers->len; i++)
    {
	SRSGSWrapDriver *drv = g_ptr_array_index (srs_gs_wrap_drivers, i);
	sru_assert (drv && drv->name);
    	if (strcmp (drv->name, driver) == 0)
	    return drv;
    }
    return NULL;
}

static gint
srs_gs_wrap_get_voice_index (SRSGSWrapDriver *driver,
			     gchar *voice)
{
    gint i;

    sru_assert (driver && voice && driver->voices);

    for (i = 0; i < driver->voices->_length; i++)
    {
	sru_assert (driver->voices->_buffer[i].name);
	if (strcmp (driver->voices->_buffer[i].name, voice) == 0)
	    return i;
    }
    return -1;
}

gchar**
srs_gs_wrap_get_driver_voices (gchar *driver)
{
    GPtrArray *vcs;
    gint i;
    SRSGSWrapDriver *drv;

    sru_assert (srs_gs_wrap_drivers && srs_gs_wrap_drivers->len > 0);
    sru_assert (driver);

    vcs = g_ptr_array_new ();
    drv = srs_gs_wrap_get_driver (driver);
    sru_assert (drv);

    for (i = 0; i < drv->voices->_length; i++)
	if (drv->voices->_buffer[i].name && drv->voices->_buffer[i].name[0])
    	    g_ptr_array_add (vcs, g_strdup (drv->voices->_buffer[i].name));
    
    g_ptr_array_add (vcs, NULL);
    sru_assert (vcs->len > 1);

    return (gchar**) g_ptr_array_free (vcs, FALSE);
}

static void
srs_gs_wrap_clb (GNOME_Speech_speech_callback_type type,
		 CORBA_long text_id,
		 CORBA_long offset)
{
    sru_assert (srs_gs_wrap_callback);

    srs_gs_wrap_callback (text_id, type, offset);
}

SRSGSWrapSpeaker
srs_gs_wrap_speaker_new (gchar *driver,
			 gchar *voice
#ifdef SRS_NO_MARKERS_SUPPORTED
			 , gboolean *has_callback
#endif
			)
{
    GNOME_Speech_Speaker sp;
    SRSGSWrapDriver *drv;
    gint i;
    gboolean rv;

#ifdef SRS_NO_MARKERS_SUPPORTED
    sru_assert (has_callback);
#endif

    drv = srs_gs_wrap_get_driver (driver);
    sru_assert (drv);
    i = srs_gs_wrap_get_voice_index (drv, voice);
    sru_assert (0 <= i && i < drv->voices->_length);

    sp = GNOME_Speech_SynthesisDriver_createSpeaker (drv->driver,
				&drv->voices->_buffer[i], srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("Cannot create speaker !", NULL);

#ifdef SRS_NO_MARKERS_SUPPORTED
    *has_callback = 
#endif
    rv = srs_gs_cb_register_callback (sp, srs_gs_wrap_clb);
    if (!rv)
#ifdef SRS_NO_MARKERS_SUPPORTED
	sru_warning ("Unable to register the callback");
#else
    	sru_error ("Unable to register the callback");
#endif

    return sp;
}

void
srs_gs_wrap_speaker_terminate (SRSGSWrapSpeaker speaker)
{
    sru_assert (speaker);

    GNOME_Speech_Speaker_unref (speaker, srs_gs_wrap_get_ev ());
    srs_gs_wrap_check_ev ("Unable to unref the speaker");
}

static gboolean
srs_gs_wrap_speaker_set_parameter (SRSGSWrapSpeaker speaker,
				     gchar *name,
				     gint val)
{
    GNOME_Speech_ParameterList *parameters;
    GNOME_Speech_Parameter *parameter;
    gint i;

    sru_assert (speaker && name);

    parameters = GNOME_Speech_Speaker_getSupportedParameters (speaker,
					      srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("No parameters suported", FALSE);
    
    parameter = NULL;
    for (i = 0; i < parameters->_length; i++)
    {
	sru_assert (parameters->_buffer[i].name);
	if (strcmp (parameters->_buffer[i].name, name) == 0)
	     parameter = &(parameters->_buffer[i]);
    }

    if (!parameter)
	sru_warning ("Unable to find parameter");

    if (parameter)
    {
	val = MAX (parameter->min, val);
	val = MIN (parameter->max, val);
	GNOME_Speech_Speaker_setParameterValue (speaker, name,
						val, srs_gs_wrap_get_ev ());
	srs_gs_wrap_return_val_if_ev ("Unable to set parameter", FALSE);
    }

    srs_gs_wrap_gsparameterlist_free (parameters);

    return TRUE;
}

gboolean
srs_gs_wrap_speaker_set_pitch (SRSGSWrapSpeaker speaker,
			       gint pitch)
{
    return srs_gs_wrap_speaker_set_parameter (speaker, "pitch", pitch);
}

gboolean
srs_gs_wrap_speaker_set_rate (SRSGSWrapSpeaker speaker,
			      gint rate)
{
    return srs_gs_wrap_speaker_set_parameter (speaker, "rate", rate);
}

gboolean
srs_gs_wrap_speaker_set_volume (SRSGSWrapSpeaker speaker,
				gint volume)
{
    return srs_gs_wrap_speaker_set_parameter (speaker, "volume", volume);
}

gboolean
srs_gs_wrap_speaker_shutup (SRSGSWrapSpeaker speaker)
{
    sru_assert (speaker);
    
    GNOME_Speech_Speaker_stop (speaker, srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("Cannot shutup current voice", FALSE);

    return TRUE;
}

SRSGSWrapLong
srs_gs_wrap_speaker_say (SRSGSWrapSpeaker speaker,
			 gchar *text)
{
    CORBA_long id;

    sru_assert (speaker && text);

    id = GNOME_Speech_Speaker_say (speaker, text, srs_gs_wrap_get_ev ());
    srs_gs_wrap_return_val_if_ev ("Cannot speak with the current voice", -1);

    return id;
}
