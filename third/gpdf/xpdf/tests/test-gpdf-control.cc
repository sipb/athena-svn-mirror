/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Unit tests for GPdf Bonobo Control
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
#include "gpdf-control.h"
#include "gpdf-persist-stream.h"
#include "gpdf-persist-file.h"

#define TEST_INITIALIZATION				\
	bonobo_ui_init ("test", VERSION, &argc, argv);	\
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

TEST_BEGIN (GPdfControl, setup, tear_down)

TEST_NEW (control_implements_persist_stream_and_file)
{
	GPdfPersistStream *gpdf_persist_stream;
	GPdfPersistFile *gpdf_persist_file;
	GPdfControl *gpdf_control;
	Bonobo_Control control;
	Bonobo_PersistStream persist_stream;
	Bonobo_PersistFile persist_file;

	gpdf_persist_stream =
		gpdf_persist_stream_new ("OAFIID:GNOME_PDF_Control");
	gpdf_persist_file =
		gpdf_persist_file_new ("OAFIID:GNOME_PDF_Control");

	gpdf_control = GPDF_CONTROL (
		g_object_new (GPDF_TYPE_CONTROL,
			      "persist_stream", gpdf_persist_stream,
			      "persist_file", gpdf_persist_file, NULL));
	bonobo_object_unref (gpdf_persist_stream);
	bonobo_object_unref (gpdf_persist_file);

	TEST (GPDF_IS_CONTROL (gpdf_control));

	control = BONOBO_OBJREF (gpdf_control);
	persist_stream = Bonobo_Unknown_queryInterface (
		control, "IDL:Bonobo/PersistStream:1.0", &ev);
	TEST (!BONOBO_EX (&ev));
	TEST (persist_stream != CORBA_OBJECT_NIL);

	persist_file = Bonobo_Unknown_queryInterface (
		control, "IDL:Bonobo/PersistFile:1.0", &ev);
	TEST (!BONOBO_EX (&ev));
	TEST (persist_file != CORBA_OBJECT_NIL);

	bonobo_object_release_unref (persist_file, &ev);
	TEST (!BONOBO_EX (&ev));
	bonobo_object_release_unref (persist_stream, &ev);
	TEST (!BONOBO_EX (&ev));

	bonobo_object_unref (gpdf_control);
}

TEST_NEW (control_property_title)
{
	GPdfPersistFile *gpdf_persist_file;
	GPdfPersistStream *gpdf_persist_stream;
	GPdfControl *gpdf_control;
	Bonobo_PropertyBag property_bag;
	Bonobo_PersistFile persist_file;
	char *current_dir;
	char *uri;
	GnomeVFSURI *vfs_uri;
	const char *expected_title;
	char *title;

	gpdf_persist_file =
		gpdf_persist_file_new ("OAFIID:GNOME_PDF_Control");
	gpdf_persist_stream =
		gpdf_persist_stream_new ("OAFIID:GNOME_PDF_Control");
	gpdf_control = GPDF_CONTROL (
		g_object_new (GPDF_TYPE_CONTROL,
			      "persist_file", gpdf_persist_file,
			      "persist_stream", gpdf_persist_stream, NULL));
	bonobo_object_unref (gpdf_persist_file);

	TEST (GPDF_IS_CONTROL (gpdf_control));

	property_bag = Bonobo_Control_getProperties (
		BONOBO_OBJREF (gpdf_control), &ev);
	TEST (property_bag != CORBA_OBJECT_NIL);

	current_dir = g_get_current_dir ();
	uri = g_strconcat ("file://",
			   current_dir, "/",
			   TESTFILES_DIR "/simple-links.pdf", NULL);
	g_free (current_dir);

	persist_file = Bonobo_Unknown_queryInterface (
		BONOBO_OBJREF (gpdf_control), "IDL:Bonobo/PersistFile:1.0", &ev);
	Bonobo_PersistFile_load (persist_file, uri, &ev);
	TEST (!BONOBO_EX (&ev));

	
	vfs_uri = gnome_vfs_uri_new (uri);
	expected_title = gnome_vfs_uri_get_path (vfs_uri);
	title = bonobo_pbclient_get_string_with_default (property_bag, "title",
							 "no title returned",
							 NULL);
	TEST_STR (expected_title, title);
	g_free (title);
	gnome_vfs_uri_unref (vfs_uri);
	g_free (uri);

	bonobo_object_release_unref (property_bag, &ev);
	bonobo_object_release_unref (persist_file, &ev);
	bonobo_object_unref (gpdf_control);
}

TEST_NEW (loading_through_stream_unloads_file)
{
	/* FIXME writeme */
}

TEST_END ()
