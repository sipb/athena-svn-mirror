/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Unit tests for PDF loading
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "aconf.h"
#include "gpdf-g-switch.h"
#  include <bonobo.h>
#  include <libgnomevfs/gnome-vfs.h>
#include "gpdf-g-switch.h"
#include "gpdf-persist-file.h"
#include "gpdf-persist-stream.h"

#define TEST_INITIALIZATION		\
	bonobo_init (&argc, argv);	\
	gnome_vfs_init ();
#include <unit-test.h>

#define TEST_STR(expected, got)						\
	TESTING_MESSAGE ("string value of " #got);			\
	if (strcmp (expected, got) != 0) {				\
		g_print ("\n Expected \"%s\", got \"%s\"", expected, got); \
		FAIL_TEST ();						\
	}								\
	g_print ("Okay!\n");

static CORBA_Environment ev;

static void
setup ()
{
	CORBA_exception_init (&ev);
}

static void
tear_down ()
{
	CORBA_exception_free (&ev);
}

static void
loading_finished_cb (GPdfPersistStream *persist, char **log)
{
	if (*log == NULL) {
		*log = g_strdup ("loading_finished");
	} else {
		char *old_log = *log;

		*log = g_strconcat (old_log, " ", "loading_finished", NULL);
		g_free (old_log);
	}
}

TEST_BEGIN (GPdfPDFLoading, setup, tear_down)

TEST_NEW (load_via_bonobo_stream)
{
	Bonobo_Stream stream;
	GPdfPersistStream *gpdf_persist_stream;
	Bonobo_PersistStream persist_stream;
	char *log;
	PDFDoc *pdf_doc;

	stream = bonobo_get_object ("file:" TESTFILES_DIR "/simple-links.pdf",
				    "IDL:Bonobo/Stream:1.0", &ev);
	TEST (!BONOBO_EX (&ev));

	gpdf_persist_stream =
		gpdf_persist_stream_new ("OAFIID:GNOME_PDF_Control");
	TEST (gpdf_persist_stream != NULL);

	log = NULL;
	g_signal_connect (G_OBJECT (gpdf_persist_stream), "loading_finished",
			  G_CALLBACK (loading_finished_cb), &log);

	persist_stream = BONOBO_OBJREF (gpdf_persist_stream);
	TEST (persist_stream != CORBA_OBJECT_NIL);

	Bonobo_PersistStream_load (persist_stream, stream, "application/pdf",
				   &ev);
	TEST (!BONOBO_EX (&ev));
	TEST_STR ("loading_finished", log);

	pdf_doc = gpdf_persist_stream_get_pdf_doc (gpdf_persist_stream);
	TEST (dynamic_cast <PDFDoc *> (pdf_doc) != NULL);
	TEST (pdf_doc->isOk ());

	bonobo_object_unref (gpdf_persist_stream);
	bonobo_object_release_unref (stream, &ev);
}

TEST_NEW (persist_stream_content_types)
{
	GPdfPersistStream *gpdf_persist_stream;
	Bonobo_Persist persist;
	Bonobo_Persist_ContentTypeList *content_types;

	gpdf_persist_stream =
		gpdf_persist_stream_new ("OAFIID:GNOME_PDF_Control");
	TEST (gpdf_persist_stream != NULL);
	persist = BONOBO_OBJREF (gpdf_persist_stream);
	TEST (persist != CORBA_OBJECT_NIL);

	content_types = Bonobo_Persist_getContentTypes (persist, &ev);
	TEST (content_types != CORBA_OBJECT_NIL);
	TEST (content_types->_length == 1);
	TEST_STR ("application/pdf", content_types->_buffer [0]);
	CORBA_free (content_types);

	bonobo_object_unref (gpdf_persist_stream);
}

TEST_NEW (persist_stream_no_save)
{
	GPdfPersistStream *gpdf_persist_stream;
	Bonobo_PersistStream persist_stream;

	gpdf_persist_stream =
		gpdf_persist_stream_new ("OAFIID:GNOME_PDF_Control");
	TEST (gpdf_persist_stream != NULL);
	persist_stream = BONOBO_OBJREF (gpdf_persist_stream);
	TEST (persist_stream != CORBA_OBJECT_NIL);

	Bonobo_PersistStream_save (persist_stream, CORBA_OBJECT_NIL,
				   "application/pdf", &ev);
	TEST (BONOBO_EX (&ev));
	TEST_STR ("IDL:Bonobo/NotSupported:1.0", ev._id);

	bonobo_object_unref (gpdf_persist_stream);
}

TEST_NEW (load_via_persist_file)
{
	char *current_dir;
	char *uri;
	GPdfPersistFile *gpdf_persist_file;
	Bonobo_PersistFile persist_file;
	char *log;
	PDFDoc *pdf_doc;

	current_dir = g_get_current_dir ();
	uri = g_strconcat ("file://",
			   current_dir, "/",
			   TESTFILES_DIR "/simple-links.pdf", NULL);
	g_free (current_dir);

	gpdf_persist_file =
		gpdf_persist_file_new ("OAFIID:GNOME_PDF_Control");
	TEST (gpdf_persist_file != NULL);

	log = NULL;
	g_signal_connect (G_OBJECT (gpdf_persist_file), "loading_finished",
			  G_CALLBACK (loading_finished_cb), &log);

	persist_file = BONOBO_OBJREF (gpdf_persist_file);
	TEST (persist_file != CORBA_OBJECT_NIL);

	Bonobo_PersistFile_load (persist_file, uri, &ev);
	g_free (uri);
	TEST (!BONOBO_EX (&ev));
	TEST_STR ("loading_finished", log);

	pdf_doc = gpdf_persist_file_get_pdf_doc (gpdf_persist_file);
	TEST (dynamic_cast <PDFDoc *> (pdf_doc) != NULL);

	bonobo_object_unref (gpdf_persist_file);
}

TEST_NEW (persist_file_content_types)
{
	GPdfPersistFile *gpdf_persist_file;
	Bonobo_Persist persist;
	Bonobo_Persist_ContentTypeList *content_types;

	gpdf_persist_file =
		gpdf_persist_file_new ("OAFIID:GNOME_PDF_Control");
	TEST (gpdf_persist_file != NULL);
	persist = BONOBO_OBJREF (gpdf_persist_file);
	TEST (persist != CORBA_OBJECT_NIL);

	content_types = Bonobo_Persist_getContentTypes (persist, &ev);
	TEST (content_types != CORBA_OBJECT_NIL);
	TEST (content_types->_length == 1);
	TEST_STR ("application/pdf", content_types->_buffer [0]);
	CORBA_free (content_types);

	bonobo_object_unref (gpdf_persist_file);
}

TEST_NEW (persist_file_no_save)
{
	GPdfPersistFile *gpdf_persist_file;
	Bonobo_PersistStream persist_file;

	gpdf_persist_file =
		gpdf_persist_file_new ("OAFIID:GNOME_PDF_Control");
	TEST (gpdf_persist_file != NULL);
	persist_file = BONOBO_OBJREF (gpdf_persist_file);
	TEST (persist_file != CORBA_OBJECT_NIL);

	Bonobo_PersistFile_save (persist_file, "file:///tmp/foo", &ev);
	TEST (BONOBO_EX (&ev));
	TEST_STR ("IDL:Bonobo/NotSupported:1.0", ev._id);

	bonobo_object_unref (gpdf_persist_file);
}

TEST_NEW (persist_file_getCurrentFile)
{
	char *current_dir;
	char *uri;
	GPdfPersistFile *gpdf_persist_file;
	Bonobo_PersistFile persist_file;
	CORBA_char *current_file;

	current_dir = g_get_current_dir ();
	uri = g_strconcat ("file://",
			   current_dir, "/",
			   TESTFILES_DIR "/simple-links.pdf", NULL);
	g_free (current_dir);

	gpdf_persist_file =
		gpdf_persist_file_new ("OAFIID:GNOME_PDF_Control");
	persist_file = BONOBO_OBJREF (gpdf_persist_file);
	Bonobo_PersistFile_load (persist_file, uri, &ev);
	TEST (!BONOBO_EX (&ev));

	current_file = Bonobo_PersistFile_getCurrentFile (persist_file, &ev);
	TEST (!BONOBO_EX (&ev));

	TEST_STR (uri, current_file);

	CORBA_free (current_file);
	g_free (uri);

	bonobo_object_unref (gpdf_persist_file);
}

/* TEST_NEW (load_via_file_uri) */
/* { */
/* 	FILE *file; */
/* 	int fresult; */
/* 	Object obj; */
/* 	BaseStream *base_stream; */
/* 	PDFDoc *pdf_doc; */

/* 	file = fopen (TESTFILES_DIR "/simple-links.pdf", "rb"); */
/* 	TEST (file != NULL); */

/* 	obj.initNull (); */
/* 	base_stream = new FileStream (file, 0, gFalse, 0, &obj); */
/* 	pdf_doc = new PDFDoc (base_stream); */

/* 	TEST (pdf_doc->isOk ()); */
/* 	TEST (pdf_doc->getCatalog () != NULL); */

/* 	delete pdf_doc; */

/* 	fresult = fclose (file); */
/* 	TEST (fresult == 0); */
/* } */

TEST_END ()
