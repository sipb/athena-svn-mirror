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

    Authors: Ettore Perazzoli <ettore@helixcode.com>
*/

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>
#include <gnome.h>
#include <bonobo.h>

#include <Editor.h>

#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "htmlsourceview.h"


#ifdef USING_OAF
#define HTML_EDITOR_CONTROL_ID "OAFIID:GNOME_GtkHTML_Editor:" EDITOR_API_VERSION
#else
#define HTML_EDITOR_CONTROL_ID "control:html-editor"
#endif

GtkWidget *control;
gint formatHTML = 1;


/* Saving/loading through PersistStream.  */

static void
load_through_persist_stream (const gchar *filename,
			     Bonobo_PersistStream pstream)
{
	BonoboStream *stream;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	stream = bonobo_stream_open (BONOBO_IO_DRIVER_FS, filename,
				     Bonobo_Storage_READ, 0);

	if (stream == NULL) {
		g_warning ("Couldn't load `%s'\n", filename);
	} else {
		BonoboObject *stream_object;
		Bonobo_Stream corba_stream;

		stream_object = BONOBO_OBJECT (stream);
		corba_stream = bonobo_object_corba_objref (stream_object);
		Bonobo_PersistStream_load (pstream, corba_stream,
					   "text/html", &ev);
	}

	Bonobo_Unknown_unref (pstream, &ev);
	CORBA_Object_release (pstream, &ev);

	CORBA_exception_free (&ev);
}

static void
save_through_persist_stream (const gchar *filename,
			     Bonobo_PersistStream pstream)
{
	BonoboStream *stream;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	stream = bonobo_stream_open (BONOBO_IO_DRIVER_FS, filename,
				     Bonobo_Storage_WRITE |
				     Bonobo_Storage_CREATE,
				     S_IRUSR | S_IWUSR);

	if (stream == NULL) {
		g_warning ("Couldn't create `%s'\n", filename);
	} else {
		BonoboObject *stream_object;
		Bonobo_Stream corba_stream;

		stream_object = BONOBO_OBJECT (stream);
		corba_stream = bonobo_object_corba_objref (stream_object);
		Bonobo_Stream_truncate (corba_stream, 0, &ev);
		Bonobo_PersistStream_save (pstream, corba_stream,
					   "text/html", &ev);
	}

	Bonobo_Unknown_unref (pstream, &ev);
	CORBA_Object_release (pstream, &ev);

	CORBA_exception_free (&ev);
}

static void
save_through_plain_persist_stream (const gchar *filename,
			     Bonobo_PersistStream pstream)
{
	BonoboStream *stream;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	stream = bonobo_stream_open (BONOBO_IO_DRIVER_FS, filename,
				     Bonobo_Storage_READ | 
				     Bonobo_Storage_WRITE |
				     Bonobo_Storage_CREATE, S_IRUSR | S_IWUSR);

	if (stream == NULL) {
		g_warning ("Couldn't create `%s'\n", filename);
	} else {
		BonoboObject *stream_object;
		Bonobo_Stream corba_stream;

		stream_object = BONOBO_OBJECT (stream);
		corba_stream = bonobo_object_corba_objref (stream_object);
		Bonobo_Stream_truncate (corba_stream, 0, &ev);
		Bonobo_PersistStream_save (pstream, corba_stream,
					   "text/plain", &ev);
	}

	Bonobo_Unknown_unref (pstream, &ev);
	CORBA_Object_release (pstream, &ev);

	CORBA_exception_free (&ev);
}


/* Loading/saving through PersistFile.  */

static void
load_through_persist_file (const gchar *filename,
			   Bonobo_PersistFile pfile)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	Bonobo_PersistFile_load (pfile, filename, &ev);

	if (ev._major != CORBA_NO_EXCEPTION)
		g_warning ("Cannot load.");

	CORBA_exception_free (&ev);
}

static void
save_through_persist_file (const gchar *filename,
			   Bonobo_PersistFile pfile)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	Bonobo_PersistFile_save (pfile, filename, &ev);

	if (ev._major != CORBA_NO_EXCEPTION)
		g_warning ("Cannot save.");

	CORBA_exception_free (&ev);
}


/* Common file selection widget.  We make sure there is only one open at a
   given time.  */

enum _FileSelectionOperation {
	OP_NONE,
	OP_SAVE_THROUGH_PERSIST_STREAM,
	OP_SAVE_THROUGH_PLAIN_PERSIST_STREAM,
	OP_LOAD_THROUGH_PERSIST_STREAM,
	OP_SAVE_THROUGH_PERSIST_FILE,
	OP_LOAD_THROUGH_PERSIST_FILE
};
typedef enum _FileSelectionOperation FileSelectionOperation;

struct _FileSelectionInfo {
	BonoboWidget *control;
	GtkWidget *widget;

	FileSelectionOperation operation;
};
typedef struct _FileSelectionInfo FileSelectionInfo;

static FileSelectionInfo file_selection_info = {
	NULL,
	NULL,
	OP_NONE
};

static void
file_selection_destroy_cb (GtkWidget *widget,
			   gpointer data)
{
	file_selection_info.widget = NULL;
}

static void
view_source_dialog (BonoboWindow *app, char *type, gboolean as_html)
{
	BonoboWidget *control;
	GtkWidget *window;
	GtkWidget *view;

	control = BONOBO_WIDGET (bonobo_window_get_contents (app));

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	view = html_source_view_new ();

	gtk_container_add (GTK_CONTAINER (window), view);
	html_source_view_set_source (HTML_SOURCE_VIEW (view), control, type);
	html_source_view_set_mode (HTML_SOURCE_VIEW (view), as_html);
	gtk_widget_show_all (window);

	gtk_signal_connect_object (GTK_OBJECT (control),
				   "destroy", GTK_SIGNAL_FUNC (gtk_object_destroy),
				   GTK_OBJECT (window));
}    

static void
view_html_source_cb (GtkWidget *widget,
		     gpointer data)
{
	view_source_dialog (data, "text/html", FALSE);
}

static void
view_plain_source_cb (GtkWidget *widget,
		      gpointer data)
{
	view_source_dialog (data, "text/plain", FALSE);
}

static void
view_html_source_html_cb (GtkWidget *widget,
			  gpointer data)
{
	view_source_dialog (data, "text/html", TRUE);
}

static void
file_selection_ok_cb (GtkWidget *widget,
		      gpointer data)
{
	CORBA_Object interface;
	BonoboObjectClient *object_client;
	const gchar *interface_name;

	object_client = bonobo_widget_get_server (file_selection_info.control);

	if (file_selection_info.operation == OP_SAVE_THROUGH_PERSIST_FILE
	    || file_selection_info.operation == OP_LOAD_THROUGH_PERSIST_FILE)
		interface_name = "IDL:Bonobo/PersistFile:1.0";
	else
		interface_name = "IDL:Bonobo/PersistStream:1.0";

	interface = bonobo_object_client_query_interface (object_client, interface_name,
							  NULL);

	if (interface == CORBA_OBJECT_NIL) {
		g_warning ("The Control does not seem to support `%s'.", interface_name);
	} else 	 {
		const gchar *fname;
	
		fname = gtk_file_selection_get_filename
			(GTK_FILE_SELECTION (file_selection_info.widget));

		switch (file_selection_info.operation) {
		case OP_LOAD_THROUGH_PERSIST_STREAM:
			load_through_persist_stream (fname, interface);
			break;
		case OP_SAVE_THROUGH_PERSIST_STREAM:
			save_through_persist_stream (fname, interface);
			break;
		case OP_SAVE_THROUGH_PLAIN_PERSIST_STREAM:
			save_through_plain_persist_stream (fname, interface);
			break;
		case OP_LOAD_THROUGH_PERSIST_FILE:
			load_through_persist_file (fname, interface);
			break;
		case OP_SAVE_THROUGH_PERSIST_FILE:
			save_through_persist_file (fname, interface);
			break;
		default:
			g_assert_not_reached ();
		}
	} 

	gtk_widget_destroy (file_selection_info.widget);
}


static void
open_or_save_as_dialog (BonoboWindow *app,
			FileSelectionOperation op)
{
	GtkWidget    *widget;
	BonoboWidget *control;

	control = BONOBO_WIDGET (bonobo_window_get_contents (app));

	if (file_selection_info.widget != NULL) {
		gdk_window_show (GTK_WIDGET (file_selection_info.widget)->window);
		return;
	}

	if (op == OP_LOAD_THROUGH_PERSIST_FILE || op == OP_LOAD_THROUGH_PERSIST_STREAM)
		widget = gtk_file_selection_new (_("Open file..."));
	else
		widget = gtk_file_selection_new (_("Save file as..."));

	gtk_window_set_transient_for (GTK_WINDOW (widget),
				      GTK_WINDOW (app));

	file_selection_info.widget = widget;
	file_selection_info.control = control;
	file_selection_info.operation = op;

	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (widget)->cancel_button),
				   "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (widget));

	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (widget)->ok_button),
			    "clicked", GTK_SIGNAL_FUNC (file_selection_ok_cb),
			    NULL);

	gtk_signal_connect (GTK_OBJECT (file_selection_info.widget), "destroy",
			    GTK_SIGNAL_FUNC (file_selection_destroy_cb),
			    NULL);

	gtk_widget_show (file_selection_info.widget);
}

/* "Open through persist stream" dialog.  */
static void
open_through_persist_stream_cb (GtkWidget *widget,
				gpointer data)
{
	open_or_save_as_dialog (BONOBO_WINDOW (data), OP_LOAD_THROUGH_PERSIST_STREAM);
}

/* "Save through persist stream" dialog.  */
static void
save_through_persist_stream_cb (GtkWidget *widget,
				gpointer data)
{
	open_or_save_as_dialog (BONOBO_WINDOW (data), OP_SAVE_THROUGH_PERSIST_STREAM);
}

/* "Save through persist stream" dialog.  */
static void
save_through_plain_persist_stream_cb (GtkWidget *widget,
				gpointer data)
{
	open_or_save_as_dialog (BONOBO_WINDOW (data), OP_SAVE_THROUGH_PLAIN_PERSIST_STREAM);
}

/* "Open through persist file" dialog.  */
static void
open_through_persist_file_cb (GtkWidget *widget,
			      gpointer data)
{
	open_or_save_as_dialog (BONOBO_WINDOW (data), OP_LOAD_THROUGH_PERSIST_FILE);
}

/* "Save through persist file" dialog.  */
static void
save_through_persist_file_cb (GtkWidget *widget,
			      gpointer data)
{
	open_or_save_as_dialog (BONOBO_WINDOW (data), OP_SAVE_THROUGH_PERSIST_FILE);
}


static void
exit_cb (GtkWidget *widget,
	 gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (data));
}



static BonoboUIVerb verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("OpenFile",   open_through_persist_file_cb),
	BONOBO_UI_UNSAFE_VERB ("SaveFile",   save_through_persist_file_cb),
	BONOBO_UI_UNSAFE_VERB ("OpenStream", open_through_persist_stream_cb),
	BONOBO_UI_UNSAFE_VERB ("SaveStream", save_through_persist_stream_cb),
	BONOBO_UI_UNSAFE_VERB ("SavePlainStream", save_through_plain_persist_stream_cb),
	BONOBO_UI_UNSAFE_VERB ("ViewHTMLSource", view_html_source_cb),
	BONOBO_UI_UNSAFE_VERB ("ViewHTMLSourceHTML", view_html_source_html_cb),
	BONOBO_UI_UNSAFE_VERB ("ViewPlainSource", view_plain_source_cb),
	BONOBO_UI_UNSAFE_VERB ("FileExit", exit_cb),

	BONOBO_UI_VERB_END
};

static void
menu_format_html_cb (BonoboUIComponent           *component,
		     const char                  *path,
		     Bonobo_UIComponent_EventType type,
		     const char                  *state,
		     gpointer                     user_data)

{
	if (type != Bonobo_UIComponent_STATE_CHANGED)
		return;
	formatHTML = *state == '0' ? 0 : 1;
	bonobo_widget_set_property (BONOBO_WIDGET (user_data), "FormatHTML", formatHTML, NULL);
	bonobo_widget_set_property (BONOBO_WIDGET (user_data), "HTMLTitle", "testing", NULL);
}

/* A dirty, non-translatable hack */
static char ui [] = 
"<Root>"
"	<commands>"
"	        <cmd name=\"FileExit\" _label=\"Exit\" _tip=\"Exit the program\""
"	         pixtype=\"stock\" pixname=\"Exit\" accel=\"*Control*q\"/>"
"	        <cmd name=\"FormatHTML\" _label=\"HTML mode\" type=\"toggle\" _tip=\"HTML Format switch\"/>"
"	</commands>"
"	<menu>"
"		<submenu name=\"File\" _label=\"_File\">"
"			<menuitem name=\"OpenFile\" verb=\"\" _label=\"Open (PersistFile)\" _tip=\"Open using the PersistFile interface\""
"			pixtype=\"stock\" pixname=\"Open\"/>"
"			<menuitem name=\"SaveFile\" verb=\"\" _label=\"Save (PersistFile)\" _tip=\"Save using the PersistFile interface\""
"			pixtype=\"stock\" pixname=\"Save\"/>"
"			<separator/>"
"			<menuitem name=\"OpenStream\" verb=\"\" _label=\"_Open (PersistStream)\" _tip=\"Open using the PersistStream interface\""
"			pixtype=\"stock\" pixname=\"Open\"/>"
"			<menuitem name=\"SaveStream\" verb=\"\" _label=\"_Save (PersistStream)\" _tip=\"Save using the PersistStream interface\""
"			pixtype=\"stock\" pixname=\"Save\"/>"
"			<menuitem name=\"SavePlainStream\" verb=\"\" _label=\"Save _plain(PersistStream)\" _tip=\"Save using the PersistStream interface\""
"			pixtype=\"stock\" pixname=\"Save\"/>"
"			<separator/>"
"                       <menuitem name=\"ViewHTMLSource\" verb=\"\" _label=\"View HTML Source\" _tip=\"View the html source of the current document\"/>"
"                       <menuitem name=\"ViewHTMLSourceHTML\" verb=\"\" _label=\"View HTML Output\" _tip=\"View the html source of the current document as html\"/>"
"                       <menuitem name=\"ViewPlainSource\" verb=\"\" _label=\"View PLAIN Source\" _tip=\"View the plain text source of the current document\"/>"
"			<separator/>"
"			<menuitem name=\"FileExit\" verb=\"\" _label=\"E_xit\"/>"
"		</submenu>"
"		<placeholder name=\"Component\"/>"
"		<submenu name=\"Format\" _label=\"For_mat\">"
"			<menuitem name=\"FormatHTML\" verb=\"\"/>"
"                       <separator/>"
"                       <placeholder name=\"FormatParagraph\"/>"
"               </submenu>"
"	</menu>"
"	<dockitem name=\"Toolbar\" behavior=\"exclusive\">"
"	</dockitem>"
"</Root>";


static void
app_destroy_cb (GtkWidget *app, BonoboUIContainer *uic)
{
	bonobo_object_unref (BONOBO_OBJECT (uic));

	gtk_main_quit ();
}

static int
app_delete_cb (GtkWidget *widget, GdkEvent *event, gpointer dummy)
{
	gtk_widget_destroy (GTK_WIDGET (widget));

	return FALSE;
}

static guint
container_create (void)
{
	GtkWidget *win;
	GtkWindow *window;
	BonoboUIComponent *component;
	BonoboUIContainer *container;
	CORBA_Environment ev;
	GNOME_GtkHTML_Editor_Engine engine;

	win = bonobo_window_new ("test-editor",
				 "HTML Editor Control Test");
	window = GTK_WINDOW (win);

	container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (container, BONOBO_WINDOW (win));

	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (app_delete_cb), NULL);

	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			    GTK_SIGNAL_FUNC (app_destroy_cb), container);

	gtk_window_set_default_size (window, 600, 440);
	gtk_window_set_policy (window, TRUE, TRUE, FALSE);

	component = bonobo_ui_component_new ("test-editor");

	bonobo_ui_component_set_container (
		component,
		bonobo_object_corba_objref (BONOBO_OBJECT (container)));

	bonobo_ui_component_add_verb_list_with_data (component, verbs, win);

	bonobo_ui_component_set_translate (component, "/", ui, NULL);

	control = bonobo_widget_new_control (
		HTML_EDITOR_CONTROL_ID,
		bonobo_object_corba_objref (BONOBO_OBJECT (container)));

	if (control == NULL)
		g_error ("Cannot get `%s'.", HTML_EDITOR_CONTROL_ID);

	bonobo_widget_set_property (BONOBO_WIDGET (control), "FormatHTML", formatHTML, NULL);
	bonobo_ui_component_set_prop (component, "/commands/FormatHTML", "state", formatHTML ? "1" : "0", NULL);
	bonobo_ui_component_add_listener (component, "FormatHTML", menu_format_html_cb, control);

	bonobo_window_set_contents (BONOBO_WINDOW (win), control);

	gtk_widget_show_all (GTK_WIDGET (window));

	CORBA_exception_init (&ev);
	engine = (GNOME_GtkHTML_Editor_Engine) bonobo_object_client_query_interface
		(bonobo_widget_get_server (BONOBO_WIDGET (control)), "IDL:GNOME/GtkHTML/Editor/Engine:1.0", &ev);
	GNOME_GtkHTML_Editor_Engine_runCommand (engine, "grab-focus", &ev);
	Bonobo_Unknown_unref (engine, &ev);
	CORBA_Object_release (engine, &ev);
	CORBA_exception_free (&ev);

	return FALSE;
}

#ifdef USING_OAF

#include <liboaf/liboaf.h>

static CORBA_ORB
init_corba (int *argc, char **argv)
{
	gnome_init_with_popt_table ("test-gnome-gtkhtml-editor", "1.0", *argc, argv,
				    NULL, 0, NULL);
	return oaf_init (*argc, argv);
}

#else  /* USING_OAF */

#include <libgnorba/gnorba.h>

static CORBA_ORB
init_corba (int *argc, char **argv)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	gnome_CORBA_init ("test-gnome-gtkhtml-editor", EDITOR_API_VERSION, argc, argv, 0, &ev);

	CORBA_exception_free (&ev);

	return CORBA_OBJECT_NIL;
}

#endif /* USING_OAF */

static gint
load_file (const gchar *fname)
{
	CORBA_Object interface;

	printf ("loading: %s\n", fname);
	interface = bonobo_object_client_query_interface (bonobo_widget_get_server (BONOBO_WIDGET (control)),
							  "IDL:Bonobo/PersistFile:1.0", NULL);
	load_through_persist_file (fname, interface);

	return FALSE;
}

int
main (int argc, char **argv)
{
#ifdef GTKHTML_HAVE_GCONF
	GError  *gconf_error  = NULL;
#endif

	bindtextdomain(GTKHTML_RELEASE_STRING, GNOMELOCALEDIR);
	textdomain(GTKHTML_RELEASE_STRING);

	if (bonobo_init (init_corba (&argc, argv), CORBA_OBJECT_NIL, CORBA_OBJECT_NIL) == FALSE)
		g_error ("Could not initialize Bonobo\n");

#ifdef GTKHTML_HAVE_GCONF
	if (!gconf_init (argc, argv, &gconf_error)) {
		g_assert (gconf_error != NULL);
		g_error ("GConf init failed:\n  %s", gconf_error->message);
		return 1;
	}
#endif

	/* We can't make any CORBA calls unless we're in the main loop.  So we
	   delay creating the container here. */
	gtk_idle_add ((GtkFunction) container_create, NULL);
	if (argc > 1 && *argv [argc - 1] != '-')
		gtk_idle_add ((GtkFunction) load_file, argv [argc - 1]);

	bonobo_main ();

	return 0;
}
