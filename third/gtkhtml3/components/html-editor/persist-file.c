/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2002 Ximian, Inc.
   Authors:           Radek Doulik (rodo@ximian.com)
                      Larry Ewing  (lewing@ximian.com)
		      Ettore Perazzoli (ettore@ximian.com)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHcANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <libgnome/gnome-i18n.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "gtkhtml.h"
#include "persist-file.h"
#include "editor-control-factory.h"

static BonoboObjectClass *gtk_html_persist_file_parent_class;

static void impl_save (PortableServer_Servant servant, const CORBA_char *path, CORBA_Environment * ev);
static void impl_load (PortableServer_Servant servant, const CORBA_char *path, CORBA_Environment * ev);

static void
finalize (GObject *object)
{
	GtkHTMLPersistFile *file = GTK_HTML_PERSIST_FILE (object);

	if (file->html) {
		g_object_unref (file->html);
		file->html = NULL;
	}

	G_OBJECT_CLASS (gtk_html_persist_file_parent_class)->finalize (object);
}

static Bonobo_Persist_ContentTypeList *
get_content_types (BonoboPersist *persist, CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (2, "text/html", "text/plain");
}

static void
gtk_html_persist_file_class_init (GtkHTMLPersistFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	BonoboPersistClass *persist_class = BONOBO_PERSIST_CLASS (klass);
	POA_Bonobo_PersistFile__epv *epv = &klass->epv;
	
	gtk_html_persist_file_parent_class = g_type_class_peek_parent (klass);

	epv->load = impl_load;
	epv->save = impl_save;

	object_class->finalize = finalize;
	persist_class->get_content_types = get_content_types;
}

GType
gtk_html_persist_file_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (GtkHTMLPersistFileClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gtk_html_persist_file_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (GtkHTMLPersistFile),
			0, /* n_preallocs */
			(GInstanceInitFunc) NULL
		};

		type = bonobo_type_unique (
			BONOBO_TYPE_PERSIST,
			POA_Bonobo_PersistFile__init, POA_Bonobo_PersistFile__fini,
			G_STRUCT_OFFSET (GtkHTMLPersistFileClass, epv),
			&info, "GtkHTMLPersistFile");
	}

	return type;
}

BonoboObject *
gtk_html_persist_file_new (GtkHTML *html)
{
	BonoboObject *file;

	file = g_object_new (gtk_html_persist_file_get_type (), NULL);
	bonobo_persist_construct (BONOBO_PERSIST (file), CONTROL_FACTORY_ID);

	g_object_ref (html);
	GTK_HTML_PERSIST_FILE (file)->html = html;

	return file;
}

static void
impl_load (PortableServer_Servant servant, const CORBA_char *path, CORBA_Environment * ev)
{
	GtkHTMLPersistFile *file = GTK_HTML_PERSIST_FILE (bonobo_object_from_servant (servant));
	GtkHTMLStream *stream;
#define BUFFER_SIZE 4096
	char buffer[BUFFER_SIZE];
	ssize_t count;
	gboolean was_editable;
	int fd;

	fd = open (path, O_RDONLY);
	if (fd == -1)
		return;

	was_editable = gtk_html_get_editable (file->html);
	if (was_editable)
		gtk_html_set_editable (file->html, FALSE);

	stream = gtk_html_begin (file->html);
	if (stream == NULL) {
		close (fd);
		if (was_editable)
			gtk_html_set_editable (file->html, TRUE);
		return;
	}

	while (1) {
		count = read (fd, buffer, BUFFER_SIZE);
		if (count > 0)
			gtk_html_write (file->html, stream, buffer, count);
		else
			break;
	}

	close (fd);

	if (count == 0) {
		gtk_html_end (file->html, stream, GTK_HTML_STREAM_OK);
		if (was_editable)
			gtk_html_set_editable (file->html, TRUE);
	} else {
		gtk_html_end (file->html, stream, GTK_HTML_STREAM_ERROR);
		if (was_editable)
			gtk_html_set_editable (file->html, TRUE);
	}
}

static gboolean
save_receiver  (const HTMLEngine *engine, const char *data, unsigned int len, void *user_data)
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

static void
impl_save (PortableServer_Servant servant, const CORBA_char *path, CORBA_Environment * ev)
{
	GtkHTMLPersistFile *file = GTK_HTML_PERSIST_FILE (bonobo_object_from_servant (servant));
	int fd;

	fd = open (path, O_WRONLY | O_CREAT | O_TRUNC, 0600);

	if (fd == -1)
		return;

	gtk_html_save (file->html, (GtkHTMLSaveReceiverFn) save_receiver, GINT_TO_POINTER (fd));

	close (fd);
}
