#define _ELOADER_FILE_C_

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

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "eloader-file.h"

#ifdef __GNUC__
#define EL_DEBUG(str,section) if (FALSE) g_print ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str);
#else
#define EL_DEBUG(str,section) if (FALSE) g_print ("%s:%d (%s)\n", __FILE__, __LINE__, str);
#endif
#define EL_MAX_BUF 1024

static void eloader_file_class_init (GtkObjectClass * klass);
static void eloader_file_init (GtkObject * object);
static void eloader_file_destroy (GtkObject * object);

static gint eloader_file_idle (gpointer data);

static ELoaderClass * parent_class;

GtkType
eloader_file_get_type (void)
{
	static GtkType loader_type = 0;
	if (!loader_type) {
		GtkTypeInfo loader_info = {
			"ELoaderFILE",
			sizeof (ELoaderFILE),
			sizeof (ELoaderFILEClass),
			(GtkClassInitFunc) eloader_file_class_init,
			(GtkObjectInitFunc) eloader_file_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL
		};
		loader_type = gtk_type_unique (eloader_get_type (), &loader_info);
	}
	return loader_type;
}

static void
eloader_file_class_init (GtkObjectClass * klass)
{
	parent_class = gtk_type_class (eloader_get_type ());

	klass->destroy = eloader_file_destroy;
}

static void
eloader_file_init (GtkObject * object)
{
	ELoaderFILE * el;

	el = ELOADER_FILE (object);

	el->fh = -1;
	el->length = 0;
	el->buf = NULL;

	el->iid = 0;
}

static void
eloader_file_destroy (GtkObject * object)
{
	ELoaderFILE * el;

	el = ELOADER_FILE (object);

	if (el->iid) {
		gtk_idle_remove (el->iid);
		el->iid = 0;
	}

	if (el->fh >= 0) {
		close (el->fh);
		el->fh = -1;
		el->length = 0;
	}

	if (el->buf) {
		g_free (el->buf);
		el->buf = NULL;
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

ELoader *
eloader_file_new (EBrowser * ebr, const gchar * path, GtkHTMLStream * stream)
{
	ELoaderFILE * elf;
	struct stat st;
	gint fh;

	if (stat (path, &st)) {
		EL_DEBUG ("Bad PATH", LOADER);
		gtk_html_stream_close (stream, GTK_HTML_STREAM_ERROR);
		return NULL;
	}

	if (st.st_size < 1) {
		EL_DEBUG ("Zero sized file", LOADER);
		gtk_html_stream_close (stream, GTK_HTML_STREAM_ERROR);
		return NULL;
	}

	fh = open (path, O_NONBLOCK);
	if (fh < 0) {
		EL_DEBUG ("Cannot open", LOADER);
		gtk_html_stream_close (stream, GTK_HTML_STREAM_ERROR);
		return NULL;
	}

	elf = gtk_type_new (ELOADER_FILE_TYPE);

	eloader_construct (ELOADER (elf), ebr, stream);

	eloader_connect (ELOADER (elf), path, "text/html");

	elf->fh = fh;
	elf->length = st.st_size;
	elf->buf = g_new (gchar, MIN (elf->length, EL_MAX_BUF));

	elf->iid = gtk_idle_add (eloader_file_idle, elf);

	return ELOADER (elf);
}

static gint
eloader_file_idle (gpointer data)
{
	ELoaderFILE * elf;
	gint rsize;
	gint rcount;

	elf = ELOADER_FILE (data);

	rsize = MIN (elf->length, EL_MAX_BUF);

	rcount = read (elf->fh, elf->buf, rsize);

	if (rcount < 0) {
		EL_DEBUG ("Error reading", LOADER);
		elf->iid = 0;
		eloader_done (ELOADER (elf), ELOADER_ERROR);
#if 0
		gtk_object_unref (GTK_OBJECT (elf));
#endif
		return FALSE;
	}

	if (rcount > 0) {
		gtk_html_stream_write (elf->loader.stream, elf->buf, rcount);
		elf->length -= rcount;
	}

	if (elf->length <= 0) {
		EL_DEBUG ("Done", LOADER);
		elf->iid = 0;
		eloader_done (ELOADER (elf), ELOADER_OK);
#if 0
		gtk_object_unref (GTK_OBJECT (elf));
#endif
		return FALSE;
	} else {
		EL_DEBUG ("Partial", LOADER);
		return TRUE;
	}
}


