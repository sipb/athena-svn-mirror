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

    Author: Ettore Perazzoli <ettore@helixcode.com>
*/

/* This file implements the Bonobo::PersistFile interface for the HTML editor
   control.  */

#include <config.h>

#include <gnome.h>
#include <bonobo.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "gtkhtml.h"

#include "persist-file-impl.h"


#define BUFFER_SIZE 4096


/* Loading.  */

static int
pf_impl_load (BonoboPersistFile *pf,
	      const CORBA_char *filename,
	      CORBA_Environment *ev,
	      void *closure)
{
	GtkHTML *html;
	GtkHTMLStream *stream;
	char buffer[BUFFER_SIZE];
	ssize_t count;
	gboolean was_editable;
	int fd;

	html = GTK_HTML (closure);

	fd = open (filename, O_RDONLY);
	if (fd == -1)
		return -1;

	was_editable = gtk_html_get_editable (html);
	if (was_editable)
		gtk_html_set_editable (html, FALSE);

	stream = gtk_html_begin (html);
	if (stream == NULL) {
		close (fd);
		if (was_editable)
			gtk_html_set_editable (html, TRUE);
		return -1;
	}

	while (1) {
		count = read (fd, buffer, BUFFER_SIZE);
		if (count > 0)
			gtk_html_write (html, stream, buffer, count);
		else
			break;
	}

	close (fd);

	if (count == 0) {
		gtk_html_end (html, stream, GTK_HTML_STREAM_OK);
		if (was_editable)
			gtk_html_set_editable (html, TRUE);
		return 0;
	} else {
		gtk_html_end (html, stream, GTK_HTML_STREAM_ERROR);
		if (was_editable)
			gtk_html_set_editable (html, TRUE);
		return -1;
	}
}


/* Saving.  */

static gboolean
save_receiver  (const HTMLEngine *engine,
		const char *data,
		unsigned int len,
		void *user_data)
{
	int fd;

	fd = GPOINTER_TO_INT (user_data);

	while (len > 0) {
		ssize_t count;

		count = write (fd, data, len);
		if (count < 0)
			return FALSE;

		len -= count;
		data += count;
	}

	return TRUE;
}

static int
pf_impl_save (BonoboPersistFile *pf,
	   const CORBA_char *filename,
	   CORBA_Environment *ev,
	   void *closure)
{
	GtkHTML *html;
	int retval;
	int fd;

	html = GTK_HTML (closure);

	fd = open (filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);

	if (fd == -1)
		return -1;

	if (!gtk_html_save (html, (GtkHTMLSaveReceiverFn)save_receiver, GINT_TO_POINTER (fd)))
		retval = -1;
	else
		retval = 0;

	close (fd);

	return retval;
}

static void
pf_destroy (BonoboPersistFile *pf, GtkHTML *html)
{
	g_return_if_fail (GTK_IS_HTML (html));
	
	gtk_object_unref (GTK_OBJECT (html));
}


BonoboPersistFile *
persist_file_impl_new (GtkHTML *html)
{
	BonoboPersistFile *pf;
	
	gtk_object_ref (GTK_OBJECT (html));

	pf = bonobo_persist_file_new (pf_impl_load, pf_impl_save, html);

	gtk_signal_connect (GTK_OBJECT (pf), "destroy",
			    pf_destroy, html);

	return pf;
}




