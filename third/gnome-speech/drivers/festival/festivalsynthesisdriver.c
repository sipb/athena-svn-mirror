/*
 *
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
 * festivalsynthesisdriver.c: Implementation of the FestivalSynthesisDriver
 *                            object-- a GNOME Speech driver for the Festival
 *                            Speech Synthesis System
 *
 */

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <libbonobo.h>
#include <netdb.h>
#include "festivalsynthesisdriver.h"
#include "festivalspeaker.h"


#undef FESTIVAL_DEBUG_QUEUE
#undef FESTIVAL_DEBUG_MARKERS
#undef FESTIVAL_DEBUG_CONTROL
#undef FESTIVAL_DEBUG_SEND
#undef FESTIVAL_DEBUG_TEXT

static gint 		text_id   = 0;
static GObjectClass 	*parent_class;
static gboolean 	festival_server_exists = FALSE;
static GSList		*driver_list = NULL;
static GSList		*markers_list = NULL;

typedef struct
{
    gint text_id;
    GSList *callbacks;
    GNOME_Speech_speech_callback_type type;
} FestivalTextMarker;

typedef struct 
{
    FestivalSynthesisDriver *driver;
    FestivalSpeaker 	    *speaker;
    gchar 	*text;
    gint 	text_id;
} FestivalTextOut;

static int festival_socket (const char *host, 
			    int port);
static gboolean festival_response_sock (GIOChannel *source, 
					GIOCondition condition, 
					gpointer data);
static gboolean festival_response_pipe (GIOChannel *source, 
					GIOCondition condition, 
					gpointer data);
static void festival_synthesis_driver_read_raw_line_sock (FestivalSynthesisDriver *d, 
			               		          gchar **ack);
static void festival_synthesis_driver_read_raw_line_pipe (FestivalSynthesisDriver *d, 
							  gchar **ack);

static gboolean festival_querying_queue (gpointer data);

static FestivalTextOut* festival_text_out_new (void);
static void 		festival_text_out_terminate (FestivalTextOut *text_out);

static void 	festival_add_driver	    	(FestivalSynthesisDriver *d);
static void 	festival_remove_driver 		(FestivalSynthesisDriver *d);
static gboolean festival_driver_is_alive 	(FestivalSynthesisDriver *d);
static gboolean festival_is_any_driver_alive 	(void);
static gboolean festival_synthesis_driver_say_ 	(FestivalSynthesisDriver *d,
			    			 FestivalSpeaker *s,
			    			 gchar *text);
static gboolean festival_synthesis_driver_process_list_idle 	(gpointer data);
static void     festival_synthesis_driver_process_list 		(FestivalSynthesisDriver *driver);
static void 	festival_process_text_out 			(FestivalTextOut *text_out);
static void 	festival_free_list 				(FestivalSynthesisDriver *d);


static FestivalTextMarker *
festival_text_marker_new ()
{
	return g_new0 (FestivalTextMarker, 1);
} 


static void
festival_text_marker_terminate (FestivalTextMarker *text_marker)
{
	g_assert (text_marker);

	clb_list_free (text_marker->callbacks);
	g_free (text_marker);
}


FestivalTextOut *
festival_text_out_new (void)
{
	return  g_new0 (FestivalTextOut, 1);
}


void
festival_text_out_terminate (FestivalTextOut *text_out)
{
        g_assert (text_out);
	
	g_free (text_out->text);
        bonobo_object_unref (BONOBO_OBJECT (text_out->driver));
        bonobo_object_unref (BONOBO_OBJECT (text_out->speaker));
    
	g_free (text_out);
}


static gchar *
festival_get_version (void)
{
	gchar *version;
	char buf[81];
	gchar *start, *end;
	FILE *festival;

	festival = popen ("festival --version", "r");
	if (!festival)
		return NULL;
	fgets (buf, 80, festival);
	pclose (festival);
	start = strstr (buf, "System: ");
	if (!start)
		return NULL;
	end = strstr (start+8, ":");
	version = g_strndup (start+8, end-start-8);
	return version;
}


static gboolean
festival_synthesis_driver_process_list_idle (gpointer data)
{
        FestivalSynthesisDriver *driver = data;    
    
        g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (driver));

        if (driver->list && !driver->is_shutting_up  && !driver->is_speaking)
	{
    	    FestivalTextOut *text_out;
	    GSList *tmp;

	    text_out = driver->list->data;
	    tmp = driver->list;
	    driver->list = driver->list->next;
#ifdef FESTIVAL_DEBUG_QUEUE
	    {
		FestivalTextOut *text_out = tmp->data;
		fprintf (stderr, "\n PROCESS QUEUE ELEMENT: %d---\"%s\"",text_out->text_id, text_out->text);
	    }
#endif
	    festival_process_text_out (tmp->data);
	    festival_text_out_terminate(tmp->data);
	    g_slist_free_1 (tmp);
	}
    
	bonobo_object_unref (BONOBO_OBJECT (driver));
	return FALSE;
}


static void
festival_synthesis_driver_process_list (FestivalSynthesisDriver *driver)
{
	g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (driver));

        if (driver->list)
	{
    	    bonobo_object_ref (BONOBO_OBJECT (driver));
	    g_idle_add (festival_synthesis_driver_process_list_idle, driver);
	}
}


static gboolean
festival_generate_callback (FestivalTextMarker *text_marker)
{
	GSList *tmp = NULL;
	CORBA_Environment  ev;
#ifdef FESTIVAL_DEBUG_MARKERS	
	static gint old_text_id = -1;
	static GNOME_Speech_speech_callback_type old_type = GNOME_Speech_speech_callback_speech_started;
#endif
    
	g_assert (text_marker && text_marker->callbacks);

#ifdef FESTIVAL_DEBUG_MARKERS
	g_assert (old_text_id == -1 || old_text_id <= text_marker->text_id);
	g_assert (old_text_id == -1 || old_text_id != text_marker->text_id ||
		    (old_text_id == text_marker->text_id &&
		     old_type == GNOME_Speech_speech_callback_speech_started &&
		     text_marker->type == GNOME_Speech_speech_callback_speech_ended));
	g_assert (old_text_id == -1 || old_text_id == text_marker->text_id ||
		    (old_text_id != text_marker->text_id &&
		     text_marker->type == GNOME_Speech_speech_callback_speech_started));    
		     
	old_text_id = text_marker->text_id;
	old_type = text_marker->type;
#endif

#ifdef FESTIVAL_DEBUG_MARKERS
	fprintf (stderr, "\n MARKER %d ---%s", text_marker->text_id, 
	    	    text_marker->type == GNOME_Speech_speech_callback_speech_started ? "started" :
		    text_marker->type == GNOME_Speech_speech_callback_speech_ended ? "ended" : "unknown");
#endif    

	CORBA_exception_init (&ev);
	for (tmp = text_marker->callbacks; tmp; tmp = tmp->next)
		GNOME_Speech_SpeechCallback_notify (tmp->data,
	    					    			text_marker->type,  
						    				text_marker->text_id, 
						    				-1, 
						    				&ev);
	 
	CORBA_exception_free (&ev);

	return FALSE;
} 

static void
generate_callbacks ()
{
    static gint busy = FALSE;

    if (busy)
	return;
    busy  = TRUE;

    while (markers_list)
    {
	FestivalTextMarker *marker = markers_list->data;
	markers_list = g_slist_remove_link (markers_list, markers_list);
	festival_generate_callback (marker);
        festival_text_marker_terminate (marker);
    }

    busy = FALSE;
}

static void 
add_callback (GSList *callbacks, 
	      gint text_id,
	      GNOME_Speech_speech_callback_type type)
{
	FestivalTextMarker *text_marker;

	g_assert (callbacks);
    
	text_marker = festival_text_marker_new ();

	text_marker->callbacks = clb_list_duplicate (callbacks); 
	text_marker->text_id = text_id;
	text_marker->type = type;

	markers_list = g_slist_append (markers_list, text_marker);
}

static gboolean
festival_response_sock (GIOChannel   *source, 
			GIOCondition  condition,
			gpointer      data)
{
	gchar *ack = NULL;
	FestivalSynthesisDriver *driver = data;

        g_return_val_if_fail (festival_driver_is_alive (driver), FALSE);
	g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (driver));

        festival_synthesis_driver_read_raw_line_sock (driver, &ack);
    
	if (ack && strncmp (ack, "shutup", 6) == 0)
	{
#ifdef FESTIVAL_DEBUG_CONTROL
    	    fprintf (stderr, "\nRECEIVE SHUTUP");
#endif
	    driver->is_shutting_up = FALSE;
	    driver->is_speaking = FALSE;
	    festival_synthesis_driver_process_list (driver);
	}
        else if (ack && strncmp (ack, "query", 5) == 0)
	{
#ifdef FESTIVAL_DEBUG_CONTROL
    	    fprintf (stderr, "\nRECEIVE QUERY");
#endif
	    driver->is_querying = FALSE;
        }
    
	g_free (ack);

	return TRUE;
}

gboolean
festival_response_pipe (GIOChannel   *source,
			GIOCondition  condition,
			gpointer      data)
{
        gchar *ack = NULL;
        FestivalSynthesisDriver *driver = data;

        g_return_val_if_fail (festival_driver_is_alive (driver), FALSE);
        g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (driver));

	festival_synthesis_driver_read_raw_line_pipe (driver, &ack);

        if (ack && strncmp(ack,"Command_queue:",14) == 0)
	{
    	    gint cnt;
	    
	    cnt = atoi (ack+14);
	    g_assert (cnt >=0 || cnt <= 2);
	    if (driver->queue_length != 0 && cnt == 0)
	    {
		if (driver->is_speaking && driver->crt_clbs)
	    	    add_callback (driver->crt_clbs, driver->crt_id, GNOME_Speech_speech_callback_speech_ended);
		driver->is_speaking = FALSE;
		festival_synthesis_driver_process_list (driver);
		generate_callbacks ();
	    }
	    driver->queue_length = cnt;

	}
	g_free (ack);

	return TRUE;
}


int
festival_socket (const char *host, 
		 int port)
{
        struct sockaddr_in	serv_addr;
	struct hostent 		*serverhost;
	gint fd;

	serverhost = gethostbyname (host);
	if (!serverhost)
	{
    	    fprintf (stderr,"\n gethostbyname failed");
    	    return -1;
	}

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd<0)
        {
	    fprintf (stderr,"\n socket failed");
    	    return -1;
	}

	memset (&serv_addr, 0, sizeof (serv_addr));
	memmove (&serv_addr.sin_addr, serverhost->h_addr, serverhost->h_length);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons (port);
	
        if (connect (fd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) != 0)
        {
            fprintf (stderr,"\n connect failed");
            close (fd);
	    return -1;
        }
	return fd;
}

static gboolean
festival_start (FestivalSynthesisDriver *d)
{
        gchar *s 	= NULL;
	gchar *args[]	= {"festival", "--server", NULL};
    
	if ((s = g_find_program_in_path ("festival")) != NULL)
	{
	    g_free (s);
	    festival_server_exists = TRUE;
        }
	else
	{
	    festival_server_exists = FALSE;
	    return FALSE;	
	}
    
        g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (d));

	if (!g_spawn_async_with_pipes (	NULL,		args,
					NULL,		G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
					NULL,		NULL,
					&d->pid,	NULL,
					NULL,		&d->pipe,
					NULL))
	{
		d->pid = -1;
		d->pipe = -1;
	}
	
	if (d->pipe < 0)
		return FALSE;
        usleep(2000000);
	d->sock = festival_socket ("localhost",1314);
	
        if (d->sock < 0)
	    return FALSE;
	
        d->channel_pipe = g_io_channel_unix_new(d->pipe);
        g_io_add_watch (d->channel_pipe, G_IO_IN , festival_response_pipe, d);
	
	d->channel_sock = g_io_channel_unix_new(d->sock);
        g_io_add_watch (d->channel_sock, G_IO_IN, festival_response_sock, d);

	if (festival_server_exists)
    	    festival_synthesis_driver_say_raw (d, "(audio_mode 'async)\n");
	
        g_timeout_add (50,festival_querying_queue, d);

        return festival_server_exists;
}


static void
festival_stop (FestivalSynthesisDriver *d)
{
        g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (d));

	if (d->pid > 0)
        {
	    kill (d->pid, SIGTERM);
	    waitpid (d->pid, NULL, 0);
        }
	d->pid = -1;

        g_free (d->version);
        d->version = NULL;

	d->initialized = FALSE;

        d->last_speaker = NULL;
    

	if (d->channel_sock)
	{
	    g_io_channel_shutdown (d->channel_sock, FALSE, NULL);
	    g_io_channel_unref (d->channel_sock);
        }
	d->channel_sock = NULL;

	if (d->channel_pipe)
	{
	    g_io_channel_shutdown (d->channel_pipe, FALSE, NULL);
	    g_io_channel_unref (d->channel_pipe);
	}
	d->channel_pipe = NULL;

	if (d->sock > 0)
    	    close(d->sock);
	d->sock = -1;

        if (d->pipe > 0)
	    close (d->pipe);
        d->pipe = -1;

	festival_free_list (d);

	clb_list_free (d->crt_clbs);
	d->crt_clbs = NULL;

}


static void 
festival_add_driver (FestivalSynthesisDriver *d)
{
	driver_list = g_slist_append (driver_list, d);
}


static void 
festival_remove_driver (FestivalSynthesisDriver *d)
{
	driver_list = g_slist_remove (driver_list, d);
}


static gboolean 
festival_driver_is_alive (FestivalSynthesisDriver *d)
{
	return g_slist_find (driver_list, d) != NULL;    
}


static gboolean 
festival_is_any_driver_alive ()
{
	return driver_list != NULL;    
}


static FestivalSynthesisDriver *
festival_synthesis_driver_from_servant (PortableServer_Servant *servant)
{
	return FESTIVAL_SYNTHESIS_DRIVER(bonobo_object_from_servant(servant));
}


static CORBA_string
festival__get_driverName (PortableServer_Servant servant,
			  CORBA_Environment * ev)
{
	return CORBA_string_dup ("Festival GNOME Speech Driver");  
}




static CORBA_string
festival__get_synthesizerName (PortableServer_Servant servant,
			       CORBA_Environment * ev)
{
	return CORBA_string_dup ("Festival Speech Synthesis System");
}



static CORBA_string
festival__get_driverVersion (PortableServer_Servant aservant,
			     CORBA_Environment * ev)
{
	return CORBA_string_dup ("0.3");
}



static CORBA_string
festival__get_synthesizerVersion (PortableServer_Servant servant,
				  CORBA_Environment * ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);
	return CORBA_string_dup (d->version);
}



static GSList *
get_voice_list (void)
{
	GSList *l = NULL;
	GNOME_Speech_VoiceInfo *info;	

	if (festival_server_exists)
	{
	    info = GNOME_Speech_VoiceInfo__alloc ();
	    info->language = CORBA_string_dup("en_US");
	    info->name = CORBA_string_dup ("Kevin");
	    info->gender = GNOME_Speech_gender_male;
	    l = g_slist_prepend (l, info);
	    info = GNOME_Speech_VoiceInfo__alloc ();
	    info->language = CORBA_string_dup("en_US");
	    info->name = CORBA_string_dup ("Kal");
	    info->gender = GNOME_Speech_gender_male;
	    l = g_slist_prepend (l, info);
	    l = g_slist_reverse (l);
	}
	return l;
}


static void
voice_list_free (GSList *l)
{
	g_assert (l);
	GSList *tmp = l;
	
	while (tmp)
	{
    	    CORBA_free (tmp->data);
	    tmp = tmp->next;
	}
	g_slist_free (l);
}



static GSList *
prune_voice_list (GSList *l,
		  const GNOME_Speech_VoiceInfo *info)
{
	GSList *cur, *next;
	GNOME_Speech_VoiceInfo *i;

	cur = l;
	while (cur) {
		i = (GNOME_Speech_VoiceInfo *) cur->data;
		next = cur->next;
		if (strlen(info->name))
			if (strcmp (i->name, info->name)) {
				CORBA_free (i);
				l = g_slist_remove_link (l, cur);
				cur = next;
				continue;
			}
		if (strlen(info->language))
			if (strcmp (i->language, info->language)) {
				CORBA_free (i);
				l = g_slist_remove_link (l, cur);
				cur = next;
				continue;
			}
		cur = next;
	}
	return l;
}


static GNOME_Speech_VoiceInfoList *
voice_info_list_from_voice_list (GSList *l)
{
	int i = 0;
	GNOME_Speech_VoiceInfoList *rv = GNOME_Speech_VoiceInfoList__alloc ();
  
	if (!l) {
		rv->_length = rv->_maximum = 0;
		return rv ;
	}

	rv->_length = rv->_maximum = g_slist_length (l);
	rv->_buffer = GNOME_Speech_VoiceInfoList_allocbuf (rv->_length);

	while (l) {
		GNOME_Speech_VoiceInfo *info =
			(GNOME_Speech_VoiceInfo *) l->data;
		rv->_buffer[i].name = CORBA_string_dup (info->name);
		rv->_buffer[i].gender = info->gender;
		rv->_buffer[i].language = CORBA_string_dup(info->language);
		i++;
		l = l->next;
	}
	return rv;
}



static GNOME_Speech_VoiceInfoList *
festival_getVoices (PortableServer_Servant servant,
		    const GNOME_Speech_VoiceInfo *voice_spec,
		    CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list ();
	l = prune_voice_list (l, voice_spec);
	rv = voice_info_list_from_voice_list (l);
	voice_list_free (l);
	return rv;
}



static GNOME_Speech_VoiceInfoList *
festival_getAllVoices (PortableServer_Servant servant,
		       CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list ();
	rv = voice_info_list_from_voice_list (l);
	voice_list_free (l);
	return rv;
}



static GNOME_Speech_Speaker
festival_createSpeaker (PortableServer_Servant servant,
			const GNOME_Speech_VoiceInfo *voice_spec,
			CORBA_Environment *ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);
	FestivalSpeaker *s;
	GNOME_Speech_VoiceInfo *info;
	GSList *l;
	
	l = get_voice_list ();
	l = prune_voice_list (l, voice_spec);
	
	if (l)
		info = l->data;
	else
		info = NULL;

	s = festival_speaker_new (G_OBJECT(d), info);
	return CORBA_Object_duplicate(bonobo_object_corba_objref (BONOBO_OBJECT(s)), ev);
}



static CORBA_boolean
festival_driverInit (PortableServer_Servant servant,
		     CORBA_Environment *ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);

	if (d->initialized)
		return TRUE;

	d->version = festival_get_version ();
	if (!d->version)
		return FALSE;
	else
	if (!festival_start (d)) {
		festival_stop (d);
		return FALSE;
	}

	d->initialized = TRUE;
	return TRUE;
}


static CORBA_boolean
festival_isInitialized (PortableServer_Servant servant,
			CORBA_Environment *ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);

	return d->initialized;
}


static void
festival_synthesis_driver_init (FestivalSynthesisDriver *d)
{
        d->version = NULL;
        d->last_speaker = NULL;
        d->initialized = FALSE;
        d->sock = -1;
	d->pipe = -1;
        d->channel_sock = NULL;
        d->channel_pipe = NULL;	
        d->crt_clbs = NULL;
        d->is_shutting_up = FALSE;
	d->is_speaking = FALSE;
        d->is_querying = FALSE;
        d->list = NULL;
        d->crt_id = 0;
        d->queue_length = 0;	
}


static void
festival_synthesis_driver_finalize (GObject *obj)
{
        FestivalSynthesisDriver *d = FESTIVAL_SYNTHESIS_DRIVER (obj);
    
#ifdef FESTIVAL_DEBUG_CONTROL
	fprintf (stderr, "\nSEND EXIT");
#endif
	festival_synthesis_driver_say_raw (d, "(exit)\n");

	festival_stop (d);
	if (parent_class->finalize)
    	    parent_class->finalize (obj);
		
        printf ("Festival driver finalized.\n");
	festival_remove_driver (d);
	
        if (!festival_is_any_driver_alive ())
	    bonobo_main_quit ();
}


static void
festival_synthesis_driver_class_init (FestivalSynthesisDriverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = festival_synthesis_driver_finalize;
	
	/* Initialize epv table */

	klass->epv._get_driverName = festival__get_driverName;
	klass->epv._get_driverVersion = festival__get_driverVersion;
	klass->epv._get_synthesizerName = festival__get_synthesizerName;
	klass->epv._get_synthesizerVersion = festival__get_synthesizerVersion;
	klass->epv.getVoices = festival_getVoices;
	klass->epv.getAllVoices = festival_getAllVoices;
	klass->epv.createSpeaker = festival_createSpeaker;
	klass->epv.driverInit = festival_driverInit;
	klass->epv.isInitialized = festival_isInitialized;
}


void
festival_synthesis_driver_say_raw (FestivalSynthesisDriver *d,
			    	   gchar 		   *text)
{
        int l, written;

	g_assert (text && IS_FESTIVAL_SYNTHESIS_DRIVER (d) && d->channel_sock);
#ifdef FESTIVAL_DEBUG_SEND
        fprintf (stderr, "\nRAW SEND :\"%s\"", text);
#endif
	l = strlen (text);
        g_io_channel_write_chars (d->channel_sock, text, l, &written, NULL);
        g_io_channel_flush (d->channel_sock, NULL);
}


void
festival_synthesis_driver_read_raw_line_sock (FestivalSynthesisDriver 	*d,
			    		      gchar 			**ack)
{
        g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (d) && ack && d->channel_sock);

	g_io_channel_read_line (d->channel_sock, ack, NULL, NULL, NULL);
}


void
festival_synthesis_driver_read_raw_line_pipe (FestivalSynthesisDriver 	*d,
			    		      gchar 			**ack)
{
        g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (d) && ack && d->channel_pipe);

	g_io_channel_read_line (d->channel_pipe, ack, NULL, NULL, NULL);
}


static gboolean
festival_querying_queue (gpointer data)
{
        FestivalSynthesisDriver *driver = data;

	g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (driver));

        if (!driver->is_querying && !driver->is_shutting_up && driver->is_speaking)
	{	
#ifdef FESTIVAL_DEBUG_CONTROL
	    fprintf (stderr, "\nSENT QUERY");
#endif
	    festival_synthesis_driver_say_raw (driver, "(audio_mode 'query)\n");
	    driver->is_querying = TRUE;
	}
    
        return TRUE;
}


static gboolean
festival_synthesis_driver_say_ (FestivalSynthesisDriver *d,
			        FestivalSpeaker 	*s,
			        gchar 			*text)
{
        gchar *escaped_string;
	gchar *ptr1, *ptr2;

	g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (d) && IS_FESTIVALSPEAKER (s) && text);
    
    
        escaped_string = g_malloc (strlen (text)*2+1);
        ptr1 = text;
        ptr2 = escaped_string;
        while (ptr1 && *ptr1)
        {
        	if (*ptr1 == '\"')
		    *ptr2++ = '\\';
		*ptr2++ = *ptr1++;
        }
	*ptr2 = 0;

        /* Refresh if needded */ 
        if (d->last_speaker != s || speaker_needs_parameter_refresh (SPEAKER(s)))
	{
	    /* if (!d->last_speaker || strcmp (d->last_speaker->voice, s->voice))*/
	    festival_synthesis_driver_say_raw (d, s->voice);
	    speaker_refresh_parameters (SPEAKER(s));
	    d->last_speaker = s;
	}

	clb_list_free (d->crt_clbs);
	d->crt_clbs = speaker_get_clb_list (SPEAKER (s));

#ifdef FESTIVAL_DEBUG_TEXT
	fprintf (stderr, "\nSENT:\"%s\" from \"%s\"", escaped_string, text);
#endif
	d->is_speaking = TRUE;
	d->queue_length = 1;
	festival_synthesis_driver_say_raw (d, "(SayText \"");
	festival_synthesis_driver_say_raw (d, escaped_string);
	festival_synthesis_driver_say_raw (d, "\")\r\n");

	festival_synthesis_driver_say_raw (d, "(SayText \"\")\r\n");

        if (escaped_string)
		g_free (escaped_string);
    
	return TRUE;
}


static void
festival_process_text_out (FestivalTextOut *text_out)
{
	g_assert (text_out);

#ifdef FESTIVAL_DEBUG_TEXT
        fprintf (stderr, "\nPROCESS: %d ---\"%s\"", text_out->text_id, text_out->text);
#endif
	festival_synthesis_driver_say_ (text_out->driver, text_out->speaker, text_out->text);
	     
        text_out->driver->crt_id = text_out->text_id;
	if (text_out->driver->crt_clbs)
	{
	    add_callback (text_out->driver->crt_clbs, text_out->driver->crt_id, GNOME_Speech_speech_callback_speech_started);
	    generate_callbacks ();
	}
}


static void
festival_free_text_out_list (GSList *list)
{

        GSList *crt;

#ifdef FESTIVAL_DEBUG_QUEUE
	fprintf (stderr, "\nQUEUE DISCARDING");
#endif
	for (crt = list; crt; crt = crt->next)
	{
#if defined (FESTIVAL_DEBUG_QUEUE) || defined (FESTIVAL_DEBUG_TEXT)
	    FestivalTextOut *text_out = crt->data;
	    fprintf (stderr, "\n DISCARD QUEUE ELEMENT: %d---\"%s\"",text_out->text_id, text_out->text);
#endif
	    festival_text_out_terminate(crt->data);
	}	
	
	g_slist_free (list);
	list = NULL;
}


static void 
festival_free_list (FestivalSynthesisDriver *driver)
{
        GSList *tmp;
    
	g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (driver));
    
	tmp = driver->list;
	driver->list = NULL;
	festival_free_text_out_list (tmp);
}


gint
festival_synthesis_driver_say (FestivalSynthesisDriver  *d,
			       FestivalSpeaker 		*s,
			       gchar 			*text)
{
        FestivalTextOut *text_out;

        g_assert (IS_FESTIVAL_SYNTHESIS_DRIVER (d) && IS_FESTIVALSPEAKER (s) && text);

	text_out = festival_text_out_new ();
        text_out->text = g_strdup (text);           
        text_out->driver = bonobo_object_ref (BONOBO_OBJECT (d)); 
        text_out->speaker = bonobo_object_ref (BONOBO_OBJECT (s));
        text_out->text_id = text_id++;
    
#if defined (FESTIVAL_DEBUG_QUEUE) || defined (FESTIVAL_DEBUG_TEXT)
	fprintf (stderr, "\nQUEUE ADD %d---\"%s\"",text_out->text_id, text_out->text);
#endif    
        d->list = g_slist_append (d->list, text_out);  

	festival_synthesis_driver_process_list (d);
    
        return text_out->text_id;
}


gboolean
festival_synthesis_driver_is_speaking (FestivalSynthesisDriver *d)
{
	g_assert (d);
	
	return d->is_speaking;
}

gboolean
festival_synthesis_driver_stop (FestivalSynthesisDriver *d)
{
        g_assert (d);

	d->queue_length = 0;
        festival_free_list (d);
        if (!d->is_shutting_up)
	{
#ifdef FESTIVAL_DEBUG_CONTROL
	    fprintf (stderr, "\nSEND SHUTUP");
#endif
	    festival_synthesis_driver_say_raw (d, "(audio_mode 'shutup)\n");
	    d->is_shutting_up = TRUE;
	}
    
	return TRUE;
}


BONOBO_TYPE_FUNC_FULL (FestivalSynthesisDriver,
		       GNOME_Speech_SynthesisDriver,
		       bonobo_object_get_type (),
		       festival_synthesis_driver);


FestivalSynthesisDriver * 
festival_synthesis_driver_new (void)
{
        FestivalSynthesisDriver *driver;
	
	driver = g_object_new (FESTIVAL_SYNTHESIS_DRIVER_TYPE, NULL);
	festival_add_driver (driver);
    
	return driver;
}


int
main (int  argc,
      char **argv)
{
        FestivalSynthesisDriver *driver;
	char 	*obj_id;
	int 	ret;

        driver_list = NULL;
	markers_list = NULL;

	if (!bonobo_init (&argc, argv))
        {
    	    g_error ("Could not initialize Bonobo Activation / Bonobo");
	}

        obj_id = "OAFIID:GNOME_Speech_SynthesisDriver_Festival:proto0.3";

	driver = festival_synthesis_driver_new ();
	
	if (!driver)
    	    g_error ("Error creating speech synthesis driver object.\n");

        ret = bonobo_activation_active_server_register (obj_id,
					                bonobo_object_corba_objref (bonobo_object (driver)));

        if (ret != Bonobo_ACTIVATION_REG_SUCCESS)
	    g_error ("Error registering speech synthesis driver.\n");
	else
	    bonobo_main ();
	
        g_assert (driver_list == NULL);
	g_assert (markers_list == NULL);
	bonobo_debug_shutdown ();	

        return 0;
}
