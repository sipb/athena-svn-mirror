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

#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include <stdio.h>
#include <glib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glade/glade.h>
#include <gal/widgets/e-scroll-frame.h>

#ifdef GNOME_GTKHTML_EDITOR_SHLIB
#include <bonobo/bonobo-shlib-factory.h>
#endif

#include "Editor.h"

#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlselection.h"
#include "htmlfontmanager.h"
#include "htmlsettings.h"
#include "htmlpainter.h"
#include "htmlplainpainter.h"

#include "engine.h"
#include "menubar.h"
#include "persist-file-impl.h"
#include "persist-stream-impl.h"
#include "popup.h"
#include "toolbar.h"
#include "properties.h"
#include "text.h"
#include "paragraph.h"
#include "body.h"
#include "spell.h"
#include "html-stream-mem.h"

#include "gtkhtmldebug.h"

#ifdef USING_OAF
#define CONTROL_FACTORY_ID "OAFIID:GNOME_GtkHTML_Editor_Factory:" EDITOR_API_VERSION
#else
#define CONTROL_FACTORY_ID "control-factory:html-editor"
#endif

#ifndef GNOME_GTKHTML_EDITOR_SHLIB
static BonoboGenericFactory *factory;
static gint active_controls = 0;

#endif
static void send_event_stream (GNOME_GtkHTML_Editor_Engine engine, 
			       GNOME_GtkHTML_Editor_Listener listener,
			       const char *name,
			       const char *url,
			       GtkHTMLStream *stream); 


/* This is the initialization that can only be performed after the
   control has been embedded (signal "set_frame").  */

struct _SetFrameData {
	GtkWidget *html;
	GtkWidget *vbox;
};
typedef struct _SetFrameData SetFrameData;

static GtkHTMLEditorAPI *editor_api;

static gint
gtk_toolbar_focus (GtkContainer     *container,
			  GtkDirectionType  direction)
{
	return FALSE;
}

static void
set_frame_cb (BonoboControl *control,
	      gpointer data)
{
	Bonobo_UIContainer remote_ui_container;
	BonoboUIComponent *ui_component;
	GtkHTMLControlData *control_data;
	GtkWidget *toolbar;
	GtkWidget *scroll_frame;

	control_data = (GtkHTMLControlData *) data;

	if (bonobo_control_get_control_frame (control) == CORBA_OBJECT_NIL)
		return;

	remote_ui_container = bonobo_control_get_remote_ui_container (control);
	control_data->uic = ui_component = bonobo_control_get_ui_component (control);
	bonobo_ui_component_set_container (ui_component, remote_ui_container);

	/* Setup the tool bar.  */

	toolbar = toolbar_style (control_data);

	/* hack to fix gtk toolbar focus bug, should be removed once gtk+ is fixed */
	if (GTK_CONTAINER_CLASS (gtk_type_class (gtk_toolbar_get_type ()))->focus == NULL)
		GTK_CONTAINER_CLASS (gtk_type_class (gtk_toolbar_get_type ()))->focus = gtk_toolbar_focus;
	gtk_box_pack_start (GTK_BOX (control_data->vbox), toolbar, FALSE, FALSE, 0);

	scroll_frame = e_scroll_frame_new (NULL, NULL);
	e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (scroll_frame), GTK_SHADOW_IN);
	e_scroll_frame_set_policy (E_SCROLL_FRAME (scroll_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_container_add (GTK_CONTAINER (scroll_frame), GTK_WIDGET (control_data->html));
	gtk_widget_show_all (scroll_frame);

	gtk_box_pack_start (GTK_BOX (control_data->vbox), scroll_frame, TRUE, TRUE, 0);

	/* Setup the menu bar.  */

	menubar_setup (ui_component, control_data);

	if (!spell_has_control ()) {
		control_data->has_spell_control = FALSE;
		bonobo_ui_component_set_prop (ui_component, "/commands/EditSpellCheck", "sensitive", "0", NULL);
	} else
		control_data->has_spell_control = TRUE;

	gtk_html_set_editor_api (GTK_HTML (control_data->html), editor_api, control_data);
}

static gint
release (GtkWidget *widget, GdkEventButton *event, GtkHTMLControlData *cd)
{
	HTMLEngine *e = cd->html->engine;
	GtkHTMLEditPropertyType start = GTK_HTML_EDIT_PROPERTY_BODY;
	gboolean run_dialog = FALSE;

	if (cd->obj) {
		switch (HTML_OBJECT_TYPE (cd->obj)) {
		case HTML_TYPE_IMAGE:
		case HTML_TYPE_LINKTEXT:
		case HTML_TYPE_TEXT:
		case HTML_TYPE_RULE:
			run_dialog = TRUE;
			break;
		default:
			;
		}
		if (run_dialog) {
			cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, FALSE, _("Properties"), 
										     ICONDIR "/properties-16.png");
			html_cursor_jump_to (e->cursor, e, cd->obj, 0);
			html_engine_disable_selection (e);
			html_engine_set_mark (e);
			html_cursor_jump_to (e->cursor, e, cd->obj, html_object_get_length (cd->obj));
			html_engine_edit_selection_updater_update_now (e->selection_updater);

			switch (HTML_OBJECT_TYPE (cd->obj)) {
			case HTML_TYPE_IMAGE:
				gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
									   GTK_HTML_EDIT_PROPERTY_IMAGE, _("Image"),
									   image_properties,
									   image_apply_cb,
									   image_close_cb);
				start = GTK_HTML_EDIT_PROPERTY_IMAGE;
				break;
			case HTML_TYPE_LINKTEXT:
			case HTML_TYPE_TEXT:
				gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
									   GTK_HTML_EDIT_PROPERTY_TEXT, _("Text"),
									   text_properties,
									   text_apply_cb,
									   text_close_cb);
				start = (HTML_OBJECT_TYPE (cd->obj) == HTML_TYPE_TEXT)
					? GTK_HTML_EDIT_PROPERTY_TEXT
					: GTK_HTML_EDIT_PROPERTY_LINK;
						
				break;
			case HTML_TYPE_RULE:
				gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
									   GTK_HTML_EDIT_PROPERTY_RULE, _("Rule"),
									   rule_properties,
									   rule_apply_cb,
									   rule_close_cb);
				start = GTK_HTML_EDIT_PROPERTY_RULE;
				break;
			default:
				;
			}
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   GTK_HTML_EDIT_PROPERTY_PARAGRAPH, _("Paragraph"),
								   paragraph_properties,
								   paragraph_apply_cb,
								   paragraph_close_cb);
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   GTK_HTML_EDIT_PROPERTY_BODY, _("Page"),
								   body_properties,
								   body_apply_cb,
								   body_close_cb);
			gtk_html_edit_properties_dialog_show (cd->properties_dialog);
			gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, start);
		}
	}
	gtk_signal_disconnect (GTK_OBJECT (widget), cd->releaseId);

	return FALSE;
}

static int
load_from_file (GtkHTML *html,
		const char *url,
		GtkHTMLStream *handle)
{
	unsigned char buffer[4096];
	int len;
	int fd;
        const char *path;

        if (strncmp (url, "file:", 5) != 0) {
		return FALSE;
	} 
	path = url + 5; 

	if ((fd = open (path, O_RDONLY)) == -1) {
		g_warning ("%s", g_strerror (errno));
		return FALSE;
	}
      
       	while ((len = read (fd, buffer, 4096)) > 0) {
		gtk_html_write (html, handle, buffer, len);
	}

	if (len < 0) {
		/* check to see if we stopped because of an error */
		gtk_html_end (html, handle, GTK_HTML_STREAM_ERROR);
		g_warning ("%s", g_strerror (errno));
		return TRUE;
	}	
	/* done with no errors */
	gtk_html_end (html, handle, GTK_HTML_STREAM_OK);
	close (fd);
	return TRUE;
}

static void
url_requested_cb (GtkHTML *html, const char *url, GtkHTMLStream *handle, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *)data;

	g_return_if_fail (data != NULL);
	g_return_if_fail (url != NULL);
	g_return_if_fail (handle != NULL);

	if (load_from_file (html, url, handle)) {
		/* g_warning ("valid local reponse"); */
		
	} else if (cd->editor_bonobo_engine) {
		GNOME_GtkHTML_Editor_Engine engine;
		GNOME_GtkHTML_Editor_Listener listener;
		CORBA_Environment ev;
		
		CORBA_exception_init (&ev);
		engine = bonobo_object_corba_objref (BONOBO_OBJECT (cd->editor_bonobo_engine));

		if (engine != CORBA_OBJECT_NIL
		    && (listener = GNOME_GtkHTML_Editor_Engine__get_listener (engine, &ev)) != CORBA_OBJECT_NIL) {
			send_event_stream (engine, listener, "url_requested", url, handle);
		}	
		CORBA_exception_free (&ev);
	} else {
		g_warning ("unable to resolve url: %s", url);
	}
}

static gint
html_button_pressed (GtkWidget *html, GdkEventButton *event, GtkHTMLControlData *cd)
{
	HTMLEngine *engine = cd->html->engine;
	guint offset;

	cd->obj = html_engine_get_object_at (engine,
					     event->x + engine->x_offset,
					     event->y + engine->y_offset,
					     &offset, FALSE);
	switch (event->button) {
	case 1:
		if (event->type == GDK_2BUTTON_PRESS && cd->obj && event->state & GDK_CONTROL_MASK)
			cd->releaseId = gtk_signal_connect (GTK_OBJECT (html), "button_release_event",
							    GTK_SIGNAL_FUNC (release), cd);
		else
			return TRUE;
		break;
	case 2:
		/* pass this for pasting */
		return TRUE;
	case 3:
		if (!html_engine_is_selection_active (engine) || !html_engine_point_in_selection (engine, cd->obj, offset)) {
			html_engine_disable_selection (engine);
			html_engine_jump_at (engine,
					     event->x + engine->x_offset,
					     event->y + engine->y_offset);
			gtk_html_update_styles (cd->html);
		}

		if (popup_show (cd, event))
			gtk_signal_emit_stop_by_name (GTK_OBJECT (html), "button_press_event");
		break;
	default:
		;
	}

	return FALSE;
}

static void
destroy_control_data_cb (GtkObject *control, GtkHTMLControlData *cd)
{
	gtk_html_control_data_destroy (cd);

#ifndef GNOME_GTKHTML_EDITOR_SHLIB
	active_controls --;

	if (active_controls)
		return;

	bonobo_object_unref (BONOBO_OBJECT (factory));
	gtk_main_quit ();
#endif
}

static void
editor_init_painters (GtkHTMLControlData *cd)
{	
	GtkHTMLClassProperties *prop;
	GtkHTML *html;

	g_return_if_fail (cd != NULL);

	html = cd->html;
	prop = GTK_HTML_CLASS (GTK_OBJECT (html)->klass)->properties;
	
	if (!cd->plain_painter) {
		cd->plain_painter = HTML_GDK_PAINTER (html_plain_painter_new (TRUE));
		html_font_manager_set_default (html_engine_font_manager_with_painter (html->engine,
										      HTML_PAINTER (cd->plain_painter)),
					       prop->font_var,      prop->font_fix,
					       prop->font_var_size, prop->font_var_points,
					       prop->font_fix_size, prop->font_fix_points);
		
		html_colorset_add_slave (html->engine->settings->color_set, 
					 HTML_PAINTER (cd->plain_painter)->color_set);

		cd->gdk_painter = HTML_GDK_PAINTER (html->engine->painter);

		/* the plain painter starts with a ref */
		gtk_object_ref (GTK_OBJECT (cd->gdk_painter));
	}
}

static void
editor_set_format (GtkHTMLControlData *cd, gboolean format_html)
{
	HTMLGdkPainter *p, *old_p;
	GtkHTML *html;

	g_return_if_fail (cd != NULL);
	
	editor_init_painters (cd);
	
	html = cd->html;
	cd->format_html = format_html;

	if (format_html) {
		p = cd->gdk_painter;
		old_p = cd->plain_painter;
	} else {
		p = cd->plain_painter;
		old_p = cd->gdk_painter;
	}		

	toolbar_update_format (cd);
	menubar_update_format (cd);

	if (html->engine->painter != (HTMLPainter *)p) {
		html_gdk_painter_unrealize (old_p);
		if (html->engine->window)
			html_gdk_painter_realize (p, html->engine->window);

		html_engine_set_painter (html->engine, HTML_PAINTER (p));
		html_engine_schedule_redraw (html->engine);
	}
		
}

enum {
	PROP_EDIT_HTML,
	PROP_HTML_TITLE
} EditorControlProps;

static void
editor_get_prop (BonoboPropertyBag *bag,
		 BonoboArg         *arg,
		 guint              arg_id,
		 CORBA_Environment *ev,
		 gpointer           user_data)
{
	GtkHTMLControlData *cd = user_data;
	
	switch (arg_id) {
	case PROP_EDIT_HTML:
		BONOBO_ARG_SET_BOOLEAN (arg, cd->format_html);
		break;
	case PROP_HTML_TITLE:
		BONOBO_ARG_SET_STRING (arg, gtk_html_get_title (cd->html));
		break;
       	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static void
editor_set_prop (BonoboPropertyBag *bag,
		 const BonoboArg   *arg,
		 guint              arg_id,
		 CORBA_Environment *ev,
		 gpointer           user_data)
{
	GtkHTMLControlData *cd = user_data;
	
	/* g_warning ("set_prop"); */
	switch (arg_id) {
	case PROP_EDIT_HTML:
		editor_set_format (cd, BONOBO_ARG_GET_BOOLEAN (arg));
		break;
	case PROP_HTML_TITLE:
		gtk_html_set_title (cd->html, BONOBO_ARG_GET_STRING (arg));
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static void
editor_control_construct (BonoboControl *control, GtkWidget *vbox)
{
	GtkHTMLControlData *cd;
	GtkWidget *html_widget;
	BonoboPropertyBag  *pb;
	BonoboArg          *def;

#ifndef GNOME_GTKHTML_EDITOR_SHLIB
	active_controls++;
#endif
	/* GtkHTML widget */
	html_widget = gtk_html_new ();
	gtk_html_load_empty (GTK_HTML (html_widget));
	gtk_html_set_editable (GTK_HTML (html_widget), TRUE);

	cd = gtk_html_control_data_new (GTK_HTML (html_widget), vbox);

	/* HTMLEditor::Engine */
	cd->editor_bonobo_engine = editor_engine_new (cd);
	bonobo_object_add_interface (BONOBO_OBJECT (control), 
				     BONOBO_OBJECT (cd->editor_bonobo_engine));

	/* Bonobo::PersistStream */
	cd->persist_stream = persist_stream_impl_new (GTK_HTML (html_widget));
	bonobo_object_add_interface (BONOBO_OBJECT (control), BONOBO_OBJECT (cd->persist_stream));

	/* Bonobo::PersistFile */
	cd->persist_file = persist_file_impl_new (GTK_HTML (html_widget));
	bonobo_object_add_interface (BONOBO_OBJECT (control), BONOBO_OBJECT (cd->persist_file));

	/* PropertyBag */
	pb = bonobo_property_bag_new (editor_get_prop, editor_set_prop, cd);
	bonobo_control_set_properties (control, pb);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);

	bonobo_property_bag_add (pb, "FormatHTML", PROP_EDIT_HTML,
				 BONOBO_ARG_BOOLEAN, def,
				 "Whether or not to edit in HTML mode", 
				 0);

	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (def, "");

	bonobo_property_bag_add (pb, "HTMLTitle", PROP_HTML_TITLE,
				 BONOBO_ARG_STRING, def,
				 "The title of the html document", 
				 0);
	CORBA_free (def);
	/*
	def = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (def, "");

	bonobo_property_bag_add (pb, "OnURL", PROP_EDIT_HTML,
				 BONOBO_ARG_STRING, def,
				 "The URL of the link the mouse is currently over", 
				 0);
	*/

	bonobo_object_unref (BONOBO_OBJECT (pb));

	/* Part of the initialization must be done after the control is
	   embedded in its control frame.  We use the "set_frame" signal to
	   handle that.  */

	gtk_signal_connect (GTK_OBJECT (control), "set_frame",
			    GTK_SIGNAL_FUNC (set_frame_cb), cd);

	gtk_signal_connect (GTK_OBJECT (control), "destroy",
			    GTK_SIGNAL_FUNC (destroy_control_data_cb), cd);

	gtk_signal_connect (GTK_OBJECT (html_widget), "url_requested",
			    GTK_SIGNAL_FUNC (url_requested_cb), cd);

	gtk_signal_connect (GTK_OBJECT (html_widget), "button_press_event",
			    GTK_SIGNAL_FUNC (html_button_pressed), cd);

	cd->control = control;
}

static gboolean
editor_api_command (GtkHTML *html, GtkHTMLCommandType com_type, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	gboolean rv = TRUE;

	switch (com_type) {
	case GTK_HTML_COMMAND_POPUP_MENU:
		popup_show_at_cursor (cd);
		break;
	case GTK_HTML_COMMAND_PROPERTIES_DIALOG:
		property_dialog_show (cd);
		break;
	case GTK_HTML_COMMAND_TEXT_COLOR_APPLY:
		toolbar_apply_color (cd);
		break;
	default:
		rv = FALSE;
	}

	return rv;
}

static GtkType
wait_for_bonobo_patch__bonobo_arg_type_to_gtk (BonoboArgType id)
{
	CORBA_Environment ev;
	GtkType gtk_type = GTK_TYPE_NONE;

	CORBA_exception_init (&ev);

	if (bonobo_arg_type_is_equal (TC_char, id, &ev))         gtk_type = GTK_TYPE_CHAR;
	else if (bonobo_arg_type_is_equal (TC_boolean, id, &ev)) gtk_type = GTK_TYPE_BOOL;
	else if (bonobo_arg_type_is_equal (TC_short,   id, &ev)) gtk_type = GTK_TYPE_INT;
	else if (bonobo_arg_type_is_equal (TC_ushort,  id, &ev)) gtk_type = GTK_TYPE_UINT;
	else if (bonobo_arg_type_is_equal (TC_long,    id, &ev)) gtk_type = GTK_TYPE_LONG;
	else if (bonobo_arg_type_is_equal (TC_ulong,   id, &ev)) gtk_type = GTK_TYPE_ULONG;
	else if (bonobo_arg_type_is_equal (TC_float,   id, &ev)) gtk_type = GTK_TYPE_FLOAT;
	else if (bonobo_arg_type_is_equal (TC_double,  id, &ev)) gtk_type = GTK_TYPE_DOUBLE;
	else if (bonobo_arg_type_is_equal (TC_string,  id, &ev)) gtk_type = GTK_TYPE_STRING;
	else
		g_warning ("Unmapped bonobo arg type");

	CORBA_exception_free (&ev);

	return gtk_type;
}

static GtkArg *
send_event_str (GNOME_GtkHTML_Editor_Engine engine, GNOME_GtkHTML_Editor_Listener listener, gchar *name, GtkArg *arg)
{
	CORBA_Environment ev;
	GtkArg *gtk_retval = NULL;
	BonoboArg *bonobo_arg = bonobo_arg_new (bonobo_arg_type_from_gtk (arg->type));
	BonoboArg *bonobo_retval;

	bonobo_arg_from_gtk (bonobo_arg, arg);

	/* printf ("sending to listener\n"); */
	CORBA_exception_init (&ev);
	bonobo_retval = GNOME_GtkHTML_Editor_Listener_event (listener, name, bonobo_arg, &ev);
	CORBA_free (bonobo_arg);

	if (ev._major == CORBA_NO_EXCEPTION) {
		if (!bonobo_arg_type_is_equal (bonobo_retval->_type, TC_null, &ev)
		    && !bonobo_arg_type_is_equal (bonobo_retval->_type, TC_void, &ev)) {
			gtk_retval = gtk_arg_new (wait_for_bonobo_patch__bonobo_arg_type_to_gtk (bonobo_retval->_type));
			bonobo_arg_to_gtk (gtk_retval, bonobo_retval);
		}
		CORBA_free (bonobo_retval);
	}
	CORBA_exception_free (&ev);

	return gtk_retval;
}

static void
send_event_void (GNOME_GtkHTML_Editor_Engine engine, GNOME_GtkHTML_Editor_Listener listener, gchar *name)
{
	CORBA_Environment ev;
	CORBA_any *any;

	any = CORBA_any_alloc ();
	any->_type = TC_null;
	CORBA_exception_init (&ev);
	GNOME_GtkHTML_Editor_Listener_event (listener, name, any, &ev);
	CORBA_exception_free (&ev);
	CORBA_free (any);
}

static void
send_event_stream (GNOME_GtkHTML_Editor_Engine engine, 
		   GNOME_GtkHTML_Editor_Listener listener,
		   const char *name,
		   const char *url,
		   GtkHTMLStream *stream) 
{
	CORBA_any *any;
	GNOME_GtkHTML_Editor_URLRequestEvent e;
	CORBA_Environment ev;
	BonoboStream *bstream;

	any = CORBA_any__alloc ();
	any->_type = TC_GNOME_GtkHTML_Editor_URLRequestEvent;
	any->_value = &e;

	e.url = (char *)url;
	bstream = html_stream_mem_create (stream);
	e.stream = BONOBO_OBJREF (bstream);
	
	CORBA_exception_init (&ev);
	GNOME_GtkHTML_Editor_Listener_event (listener, name, any, &ev);

	bonobo_object_unref (BONOBO_OBJECT (bstream));
	CORBA_exception_free (&ev);
	CORBA_free (any);
}
		  
static GtkArg *
editor_api_event (GtkHTML *html, GtkHTMLEditorEventType event_type, GtkArg **args, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	GtkArg *gtk_retval = NULL;

	/* printf ("editor_api_event\n"); */

	if (cd->editor_bonobo_engine) {
		GNOME_GtkHTML_Editor_Engine engine;
		GNOME_GtkHTML_Editor_Listener listener;
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		engine = bonobo_object_corba_objref (BONOBO_OBJECT (cd->editor_bonobo_engine));

		if (engine != CORBA_OBJECT_NIL
		    && (listener = GNOME_GtkHTML_Editor_Engine__get_listener (engine, &ev)) != CORBA_OBJECT_NIL) {

			switch (event_type) {
			case GTK_HTML_EDITOR_EVENT_COMMAND_BEFORE:
				gtk_retval = send_event_str (engine, listener, "command_before", args [0]);
				break;
			case GTK_HTML_EDITOR_EVENT_COMMAND_AFTER:
				gtk_retval = send_event_str (engine, listener, "command_after", args [0]);
				break;
			case GTK_HTML_EDITOR_EVENT_IMAGE_URL:
				gtk_retval = send_event_str (engine, listener, "image_url", args [0]);
				break;
			case GTK_HTML_EDITOR_EVENT_DELETE:
				send_event_void (engine, listener, "delete");
				break;
			default:
				g_warning ("Unsupported event.\n");
			}
			CORBA_exception_free (&ev);
		}
	}
	return gtk_retval;
}

static GtkWidget *
editor_api_create_input_line (GtkHTML *html, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	GtkWidget *entry;

	entry = gtk_entry_new ();
	gtk_box_pack_end (GTK_BOX (cd->vbox), entry, FALSE, FALSE, 0);
	gtk_widget_show (entry);

	return entry;
}

static void
new_editor_api ()
{
	editor_api = g_new (GtkHTMLEditorAPI, 1);

	editor_api->check_word         = spell_check_word;
	editor_api->suggestion_request = spell_suggestion_request;
	editor_api->add_to_personal    = spell_add_to_personal;
	editor_api->add_to_session     = spell_add_to_session;
	editor_api->set_language       = spell_set_language;
	editor_api->command            = editor_api_command;
	editor_api->event              = editor_api_event;
	editor_api->create_input_line  = editor_api_create_input_line;
}

static void
editor_control_init (void)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;

		new_editor_api ();
		gdk_rgb_init ();
		glade_gnome_init ();
	}
}

static BonoboObject *
editor_control_factory (BonoboGenericFactory *factory, gpointer closure)
{
	BonoboControl *control;
	GtkWidget *vbox;

	editor_control_init ();

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	/* g_warning ("Creating a new GtkHTML editor control."); */
	control = bonobo_control_new (vbox);

	if (control){
		editor_control_construct (control, vbox);
		return BONOBO_OBJECT (control);
	} else {
		gtk_widget_unref (vbox);
		return NULL;
	}
}

#ifdef GNOME_GTKHTML_EDITOR_SHLIB

BONOBO_OAF_SHLIB_FACTORY (CONTROL_FACTORY_ID, "GNOME HTML Editor factory", editor_control_factory, NULL);

#else

#ifdef USING_OAF

#include <liboaf/liboaf.h>

static CORBA_ORB
init_corba (int *argc, char **argv)
{
	gnome_init_with_popt_table ("gnome-gtkhtml-editor", VERSION,
				    *argc, argv, oaf_popt_options, 0, NULL);

	return oaf_init (*argc, argv);
}

#else

#include <libgnorba/gnorba.h>

static CORBA_ORB
init_corba (int *argc, char **argv)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	gnome_CORBA_init_with_popt_table ("html-editor-factory", EDITOR_API_VERSION,
					  argc, argv,
					  NULL, 0, NULL,
					  GNORBA_INIT_SERVER_FUNC,
					  &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		g_error (_("Could not initialize GNORBA"));

	CORBA_exception_free (&ev);

	return CORBA_OBJECT_NIL;
}

#endif

static void
init_bonobo (int *argc, char **argv)
{
	if (bonobo_init (init_corba (argc, argv), CORBA_OBJECT_NIL, CORBA_OBJECT_NIL) == FALSE)
		g_error (_("Could not initialize Bonobo"));
}

int
main (int argc, char **argv)
{
#ifdef GTKHTML_HAVE_GCONF
	GError  *gconf_error  = NULL;
#endif

	/* Initialize the i18n support */
	bindtextdomain(GTKHTML_RELEASE_STRING, GNOMELOCALEDIR);
	textdomain(GTKHTML_RELEASE_STRING);

	init_bonobo (&argc, argv);
#ifdef GTKHTML_HAVE_GCONF
	if (!gconf_init (argc, argv, &gconf_error)) {
		g_assert (gconf_error != NULL);
		g_error ("GConf init failed:\n  %s", gconf_error->message);
		return 1;
	}
#endif

	factory = bonobo_generic_factory_new (CONTROL_FACTORY_ID,
					      editor_control_factory,
					      NULL);
	if (factory == NULL)
		g_error ("I could not register the GNOME_GtkHTML_Editor factory.");

	bonobo_main ();

	return 0;
}
#endif /* GNOME_GTKHTML_EDITOR_SHLIB */
