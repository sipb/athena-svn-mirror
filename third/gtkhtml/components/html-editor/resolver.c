/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Larry Ewing <lewing@helixcode.com>

*/

#include <config.h>
#include <bonobo.h>
#include <stdio.h>
#include <glib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/*
 * This pulls the CORBA definitions for the Demo::Echo server
 */
#include "Editor.h"

/*
 * This pulls the definition for the BonoboObject (Gtk Type)
 */
#include "resolver.h"

static BonoboObjectClass *resolver_parent_class;
static POA_GNOME_GtkHTML_Editor_Resolver__vepv htmleditor_resolver_vepv;
GNOME_GtkHTML_Editor_Resolver *htmleditor_resolver_corba_object_create (BonoboObject *object);

#define CORBA_BLOCK_SIZE 4096

static int
resolver_load_from_file (const Bonobo_ProgressiveDataSink sink,
			 const char *url,
			 CORBA_Environment *ev)
{
	unsigned char buffer[CORBA_BLOCK_SIZE];
	int len;
	int fd;
        const char *path;
	Bonobo_Stream_iobuf *buf;
        
	if (strncmp (url, "file:", 5) != 0) {
		g_warning ("Unsupported image url: %s", url);
		return FALSE;
	} 
	path = url + 5; 

	if ((fd = open (path, O_RDONLY)) == -1) {
		g_warning ("%s", g_strerror (errno));
		return FALSE;
	}

	buf = Bonobo_Stream_iobuf__alloc ();
       	while ((len = read (fd, buffer, CORBA_BLOCK_SIZE)) > 0) {
		buf->_buffer = buffer;
		buf->_length = len;
		buf->_maximum = CORBA_BLOCK_SIZE;
		Bonobo_ProgressiveDataSink_addData (sink, buf, ev);
	}

	if (len < 0) {
		/* check to see if we stopped because of an error */
		Bonobo_ProgressiveDataSink_end (sink, ev);
		g_warning ("%s", g_strerror (errno));
		return FALSE;
	}	
	/* done with no errors */
	Bonobo_ProgressiveDataSink_end (sink, ev);
	close (fd);
	return TRUE;
}

static void
impl_GNOME_GtkHTML_Editor_Resolver_loadURL (PortableServer_Servant servant,
				  const Bonobo_ProgressiveDataSink sink,
				  const CORBA_char *url,
				  CORBA_Environment *ev)
{
	CORBA_Environment our_ev;
	gboolean result = FALSE;

	CORBA_exception_init (&our_ev);

	/* FIXME need real exceptions */
	if (!strncmp (url, "/dev/null", 6)) {
		g_warning ("how about a url: %s", url);
	} else {
		g_warning ("perhaps this sounds better: %s", url);
		if (sink != CORBA_OBJECT_NIL) {

			Bonobo_ProgressiveDataSink_start (sink, &our_ev);
			result = resolver_load_from_file (sink, url, &our_ev);
	
  		} 
	}

	if (!result || (our_ev._major != CORBA_NO_EXCEPTION)) {
		/* FIXME this is not passing correct information */
		CORBA_exception_set (ev,
				     CORBA_USER_EXCEPTION,
				     ex_GNOME_GtkHTML_Editor_Resolver_NotFound,
				     NULL);
	}			
	CORBA_exception_free (&our_ev);	
}

POA_GNOME_GtkHTML_Editor_Resolver__epv *
htmleditor_resolver_get_epv (void)
{
	POA_GNOME_GtkHTML_Editor_Resolver__epv *epv;

	epv = g_new0 (POA_GNOME_GtkHTML_Editor_Resolver__epv, 1);
		
	epv->loadURL = impl_GNOME_GtkHTML_Editor_Resolver_loadURL;

	return epv;
}


static void
init_htmleditor_resolver_corba_class (void)
{
	htmleditor_resolver_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	htmleditor_resolver_vepv.GNOME_GtkHTML_Editor_Resolver_epv = htmleditor_resolver_get_epv ();
}

static void
htmleditor_resolver_class_init (HTMLEditorResolverClass *resolver_class)
{
	resolver_parent_class = gtk_type_class (bonobo_object_get_type ());
	init_htmleditor_resolver_corba_class ();
}

GNOME_GtkHTML_Editor_Resolver *
htmleditor_resolver_corba_object_create (BonoboObject *object)
{
	POA_GNOME_GtkHTML_Editor_Resolver *servant;
	CORBA_Environment ev;

	servant = (POA_GNOME_GtkHTML_Editor_Resolver *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &htmleditor_resolver_vepv;

	CORBA_exception_init (&ev);
	POA_GNOME_GtkHTML_Editor_Resolver__init ((PortableServer_Servant) servant, &ev);
	ORBIT_OBJECT_KEY(servant->_private)->object = NULL;

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return (GNOME_GtkHTML_Editor_Resolver*) bonobo_object_activate_servant (object, servant);
}

GtkType
htmleditor_resolver_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"HTMLEditorResolver",
			sizeof (HTMLEditorResolver),
			sizeof (HTMLEditorResolverClass),
			(GtkClassInitFunc) htmleditor_resolver_class_init,
			(GtkObjectInitFunc) NULL,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		
		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}
	return type;
}

HTMLEditorResolver *
htmleditor_resolver_new (void)
{
	HTMLEditorResolver *resolver;
	GNOME_GtkHTML_Editor_Resolver *corba_resolver;

	resolver = gtk_type_new (htmleditor_resolver_get_type ());

	corba_resolver = htmleditor_resolver_corba_object_create (BONOBO_OBJECT (resolver));

	if (corba_resolver == CORBA_OBJECT_NIL) {
		gtk_object_destroy (GTK_OBJECT (resolver));
		return NULL;
	}

	return resolver;
}




