/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc. 2001, 2002 Ximian Inc.

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
            Radek Doulik <rodo@ximian.com>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnome/gnome-i18n.h>
#include <string.h>
#include <gnome.h>
#include <bonobo.h>
#include <stdio.h>
#include <glib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtkscrolledwindow.h>

#include <glade/glade.h>

#include "Editor.h"

#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlimage.h"
#include "htmlinterval.h"
#include "htmlselection.h"
#include "htmlfontmanager.h"
#include "htmlsettings.h"
#include "htmlpainter.h"
#include "htmlplainpainter.h"

#include "engine.h"
#include "menubar.h"
#include "persist-file.h"
#include "persist-stream.h"
#include "popup.h"
#include "toolbar.h"
#include "properties.h"
#include "text.h"
#include "paragraph.h"
#include "body.h"
#include "spell.h"
#include "html-stream-mem.h"

#include "gtkhtmldebug.h"
#include "editor-control-factory.h"

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

static void
activate_cb (BonoboControl      *control,
	     gboolean            active,
	     GtkHTMLControlData *cd)
{
	Bonobo_UIContainer remote_ui_container;
	BonoboUIComponent *ui_component;

	printf ("ACTIVATE\n");

	if (active) {
		remote_ui_container = bonobo_control_get_remote_ui_container (control, NULL);
		cd->uic = ui_component = bonobo_control_get_ui_component (control);
		bonobo_ui_component_set_container (ui_component, remote_ui_container, NULL);
		bonobo_object_release_unref (remote_ui_container, NULL);

		menubar_setup (ui_component, cd);
	}
}

static void
set_frame_cb (BonoboControl *control,
	      gpointer data)
{
	Bonobo_UIContainer remote_ui_container;
	BonoboUIComponent *ui_component;
	GtkHTMLControlData *control_data;
	GtkWidget *toolbar;
	GtkWidget *scrolled_window;
	Bonobo_ControlFrame frame;

	control_data = (GtkHTMLControlData *) data;

	frame = bonobo_control_get_control_frame (control, NULL);
	if (frame == CORBA_OBJECT_NIL)
		return;

	CORBA_Object_release (frame, NULL);

	remote_ui_container = bonobo_control_get_remote_ui_container (control, NULL);
	control_data->uic = ui_component = bonobo_control_get_ui_component (control);
	bonobo_ui_component_set_container (ui_component, remote_ui_container, NULL);

	/* Setup the tool bar.  */

	toolbar = toolbar_style (control_data);
	gtk_box_pack_start (GTK_BOX (control_data->vbox), toolbar, FALSE, FALSE, 0);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (control_data->html));
	gtk_widget_show_all (scrolled_window);

	gtk_box_pack_start (GTK_BOX (control_data->vbox), scrolled_window, TRUE, TRUE, 0);

	/* Setup the menu bar.  */

	menubar_setup (ui_component, control_data);

	if (!spell_has_control ()) {
		control_data->has_spell_control = FALSE;
		bonobo_ui_component_set_prop (ui_component, "/commands/EditSpellCheck", "sensitive", "0", NULL);
	} else
		control_data->has_spell_control = TRUE;

	gtk_html_set_editor_api (GTK_HTML (control_data->html), editor_api, control_data);

	bonobo_object_release_unref (remote_ui_container, NULL);
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
	g_signal_handler_disconnect (widget, cd->releaseId);

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

        if (strncmp (url, "file:", 5) == 0)
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

static gboolean
html_show_popup(GtkWidget *html, GtkHTMLControlData *cd)
{
	popup_show_at_cursor (cd);

	return TRUE;
}

static gint
html_button_pressed (GtkWidget *html, GdkEventButton *event, GtkHTMLControlData *cd)
{
	HTMLEngine *engine = cd->html->engine;
	guint offset;

	cd->obj = html_engine_get_object_at (engine, event->x, event->y, &offset, FALSE);
	switch (event->button) {
	case 1:
		if (event->type == GDK_2BUTTON_PRESS && cd->obj && event->state & GDK_CONTROL_MASK) {
			cd->releaseId = g_signal_connect (html, "button_release_event",
							  G_CALLBACK (release), cd);
			return TRUE;
		}
		break;
	case 2:
		/* pass this for pasting */
		return FALSE;
	case 3:
		if (!html_engine_is_selection_active (engine) || !html_engine_point_in_selection (engine, cd->obj, offset)) {
			html_engine_disable_selection (engine);
			html_engine_jump_at (engine, event->x, event->y);
			gtk_html_update_styles (cd->html);
		}

		if (popup_show (cd, event)) {
			g_signal_stop_emission_by_name (html, "button_press_event");
			return TRUE;
		}
		break;
	default:
		;
	}

	return FALSE;
}

static gint
html_button_pressed_after (GtkWidget *html, GdkEventButton *event, GtkHTMLControlData *cd)
{
	HTMLEngine *e = cd->html->engine;
	HTMLObject *obj = e->cursor->object;

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS && obj && obj->parent && !html_engine_is_selection_active (e)) {
		if (html_object_is_text (obj) && html_object_get_data (obj->parent, "template_text")) {
			html_object_set_data_full (obj->parent, "template_text", NULL, NULL);
			html_cursor_jump_to_position (e->cursor, e, e->cursor->position - e->cursor->offset);
			html_engine_set_mark (e);
			html_cursor_jump_to_position (e->cursor, e, e->cursor->position + html_object_get_length (obj));
			html_engine_select_interval (e, html_interval_new_from_cursor (e->mark, e->cursor));
			/* printf ("delete template text\n"); */
			html_engine_delete (cd->html->engine);
		} else if (HTML_IS_IMAGE (obj) && html_object_get_data (obj->parent, "template_image")) {
			property_dialog_show (cd);
			html_object_set_data_full (obj->parent, "template_image", NULL, NULL);
		}
	}

	return FALSE;
}

static void
editor_init_painters (GtkHTMLControlData *cd)
{	
	GtkHTML *html;

	g_return_if_fail (cd != NULL);


	html = cd->html;
	
	gtk_widget_ensure_style (GTK_WIDGET (html));
	
	if (!cd->plain_painter) {
		cd->gdk_painter = HTML_GDK_PAINTER (html->engine->painter);
		cd->plain_painter = HTML_GDK_PAINTER (html_plain_painter_new (GTK_WIDGET (html), TRUE));

		html_colorset_add_slave (html->engine->settings->color_set, 
					 HTML_PAINTER (cd->plain_painter)->color_set);

		/* the plain painter starts with a ref */
		g_object_ref (G_OBJECT (cd->gdk_painter));
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

	/* printf ("set format %d\n", format_html); */

	toolbar_update_format (cd);
	menubar_update_format (cd);

	if (html->engine->painter != (HTMLPainter *)p) {
		html_gdk_painter_unrealize (old_p);
		if (html->engine->window)
			html_gdk_painter_realize (p, html->engine->window);

		html_font_manager_set_default (&HTML_PAINTER (p)->font_manager,
					       HTML_PAINTER (old_p)->font_manager.variable.face,
					       HTML_PAINTER (old_p)->font_manager.fixed.face,
					       HTML_PAINTER (old_p)->font_manager.var_size,
					       HTML_PAINTER (old_p)->font_manager.var_points,
					       HTML_PAINTER (old_p)->font_manager.fix_size,
					       HTML_PAINTER (old_p)->font_manager.fix_points);

		html_engine_set_painter (html->engine, HTML_PAINTER (p));
		html_engine_schedule_redraw (html->engine);
	}
		
}

static enum {
	PROP_EDIT_HTML,
	PROP_HTML_TITLE,
	PROP_INLINE_SPELLING,
	PROP_MAGIC_LINKS,
	PROP_MAGIC_SMILEYS
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
	case PROP_INLINE_SPELLING:
		BONOBO_ARG_SET_BOOLEAN (arg, gtk_html_get_inline_spelling (cd->html));
		break;
	case PROP_MAGIC_LINKS:
		BONOBO_ARG_SET_BOOLEAN (arg, gtk_html_get_magic_links (cd->html));
		break;
	case PROP_MAGIC_SMILEYS:
		BONOBO_ARG_SET_BOOLEAN (arg, gtk_html_get_magic_smileys (cd->html));
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
	case PROP_INLINE_SPELLING:
		gtk_html_set_inline_spelling (cd->html, BONOBO_ARG_GET_BOOLEAN (arg));
		break;
	case PROP_MAGIC_LINKS:
		gtk_html_set_magic_links (cd->html, BONOBO_ARG_GET_BOOLEAN (arg));
		break;
	case PROP_MAGIC_SMILEYS:
		gtk_html_set_magic_smileys (cd->html, BONOBO_ARG_GET_BOOLEAN (arg));
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static void
control_destroy (BonoboObject *o, GtkHTMLControlData *cd)
{
	gtk_html_control_data_destroy (cd);
}

static void
editor_control_construct (BonoboControl *control, GtkWidget *vbox)
{
	GtkHTMLControlData *cd;
	GtkWidget *html_widget;
	BonoboPropertyBag  *pb;
	BonoboArg          *def;

	/* GtkHTML widget */
	html_widget = gtk_html_new ();
	gtk_html_load_empty (GTK_HTML (html_widget));
	gtk_html_set_editable (GTK_HTML (html_widget), TRUE);
	gtk_html_set_animate (GTK_HTML (html_widget), FALSE);

	cd = gtk_html_control_data_new (GTK_HTML (html_widget), vbox);
	g_signal_connect (control, "destroy", G_CALLBACK (control_destroy), cd);

	/* HTMLEditor::Engine */
	cd->editor_bonobo_engine = editor_engine_new (cd);
	bonobo_object_add_interface (BONOBO_OBJECT (control), 
				     BONOBO_OBJECT (cd->editor_bonobo_engine));

	/* Bonobo::PersistStream */
	cd->persist_stream = gtk_html_persist_stream_new (GTK_HTML (html_widget));
	bonobo_object_add_interface (BONOBO_OBJECT (control), cd->persist_stream);

	/* Bonobo::PersistFile */
	cd->persist_file = gtk_html_persist_file_new (GTK_HTML (html_widget));
	bonobo_object_add_interface (BONOBO_OBJECT (control), cd->persist_file);

	/* PropertyBag */
	pb = bonobo_property_bag_new (editor_get_prop, editor_set_prop, cd);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);

	bonobo_property_bag_add (pb, "FormatHTML", PROP_EDIT_HTML,
				 BONOBO_ARG_BOOLEAN, def,
				 "Whether or not to edit in HTML mode", 
				 0);

	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, gtk_html_get_inline_spelling (GTK_HTML (html_widget)));

	bonobo_property_bag_add (pb, "InlineSpelling", PROP_INLINE_SPELLING,
				 BONOBO_ARG_BOOLEAN, def,
				 "Include spelling errors inline", 
				 0);

	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, gtk_html_get_magic_links (GTK_HTML (html_widget)));

	bonobo_property_bag_add (pb, "MagicLinks", PROP_MAGIC_LINKS,
				 BONOBO_ARG_BOOLEAN, def,
				 "Recognize links in text and replace them", 
				 0);

	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, gtk_html_get_magic_smileys (GTK_HTML (html_widget)));

	bonobo_property_bag_add (pb, "MagicSmileys", PROP_MAGIC_SMILEYS,
				 BONOBO_ARG_BOOLEAN, def,
				 "Recognize smileys in text and replace them", 
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

	bonobo_property_bag_add (pb, "SpellLanguage", PROP_CURRENT_LANGUAGE,
				 BONOBO_ARG_STRING, def,
				 "The title of the html document", 
				 0);
	CORBA_free (def);
	
	def = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (def, "");

	bonobo_property_bag_add (pb, "OnURL", PROP_EDIT_HTML,
				 BONOBO_ARG_STRING, def,
				 "The URL of the link the mouse is currently over", 
				 0);
	*/

	bonobo_control_set_properties (control, BONOBO_OBJREF (pb), NULL);
	bonobo_object_unref (BONOBO_OBJECT (pb));



	/* Part of the initialization must be done after the control is
	   embedded in its control frame.  We use the "set_frame" signal to
	   handle that.  */

	g_signal_connect (control, "activate", G_CALLBACK (activate_cb), cd);
	g_signal_connect (control, "set_frame", G_CALLBACK (set_frame_cb), cd);
	g_signal_connect (html_widget, "url_requested", G_CALLBACK (url_requested_cb), cd);
	g_signal_connect (html_widget, "button_press_event", G_CALLBACK (html_button_pressed), cd);
	g_signal_connect_after (html_widget, "button_press_event", G_CALLBACK (html_button_pressed_after), cd);
	g_signal_connect (html_widget, "popup_menu", G_CALLBACK(html_show_popup), cd);

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

static GValue *
send_event_str (GNOME_GtkHTML_Editor_Engine engine, GNOME_GtkHTML_Editor_Listener listener, gchar *name, GValue *arg)
{
	CORBA_Environment ev;
	GValue *gvalue_retval = NULL;
	BonoboArg *bonobo_arg = bonobo_arg_new (bonobo_arg_type_from_gtype (G_VALUE_TYPE (arg)));
	BonoboArg *bonobo_retval;

	bonobo_arg_from_gvalue (bonobo_arg, arg);

	/* printf ("sending to listener\n"); */
	CORBA_exception_init (&ev);
	bonobo_retval = GNOME_GtkHTML_Editor_Listener_event (listener, name, bonobo_arg, &ev);
	bonobo_arg_release (bonobo_arg);

	if (ev._major == CORBA_NO_EXCEPTION) {
		if (!bonobo_arg_type_is_equal (bonobo_retval->_type, TC_null, &ev)
		    && !bonobo_arg_type_is_equal (bonobo_retval->_type, TC_void, &ev)) {
			gvalue_retval = g_new0 (GValue, 1);
			gvalue_retval = g_value_init (gvalue_retval, bonobo_arg_type_to_gtype (bonobo_retval->_type));
			bonobo_arg_to_gvalue (gvalue_retval, bonobo_retval);
		}
		CORBA_free (bonobo_retval);
	}
	CORBA_exception_free (&ev);

	return gvalue_retval;
}

static void
send_event_void (GNOME_GtkHTML_Editor_Engine engine, GNOME_GtkHTML_Editor_Listener listener, gchar *name)
{
	CORBA_Environment ev;
	CORBA_any *any;
	CORBA_any *retval;

	any = CORBA_any_alloc ();
	any->_type = TC_null;
	CORBA_exception_init (&ev);
	retval = GNOME_GtkHTML_Editor_Listener_event (listener, name, any, &ev);
	if (!BONOBO_EX (&ev))
	    CORBA_free (retval);

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
	CORBA_any *retval;
	GNOME_GtkHTML_Editor_URLRequestEvent e;
	CORBA_Environment ev;
	BonoboObject *bstream;

	any = CORBA_any__alloc ();
	any->_type = TC_GNOME_GtkHTML_Editor_URLRequestEvent;
	any->_value = &e;

	e.url = (char *)url;
	bstream = html_stream_mem_create (stream);
	e.stream = BONOBO_OBJREF (bstream);
	
	CORBA_exception_init (&ev);
	retval = GNOME_GtkHTML_Editor_Listener_event (listener, name, any, &ev);
	if (!BONOBO_EX (&ev))
	    CORBA_free (retval);

	bonobo_object_unref (BONOBO_OBJECT (bstream));
	CORBA_exception_free (&ev);
	CORBA_free (any);
}
		  
static GValue *
editor_api_event (GtkHTML *html, GtkHTMLEditorEventType event_type, GValue *args, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	GValue *retval = NULL;

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
				retval = send_event_str (engine, listener, "command_before", args);
				break;
			case GTK_HTML_EDITOR_EVENT_COMMAND_AFTER:
				retval = send_event_str (engine, listener, "command_after", args);
				break;
			case GTK_HTML_EDITOR_EVENT_IMAGE_URL:
				retval = send_event_str (engine, listener, "image_url", args);
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
	return retval;
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
		glade_init ();
	}
}

BonoboObject *
editor_control_factory (BonoboGenericFactory *factory, const gchar *component_id, gpointer closure)
{
	BonoboControl *control;
	GtkWidget *vbox;

	/* printf ("factory: %s\n", component_id); */

	editor_control_init ();

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	/* g_warning ("Creating a new GtkHTML editor control."); */
	control = bonobo_control_new (vbox);
	/* printf ("%s\n", bonobo_ui_component_get_name (bonobo_control_get_ui_component (control))); */

	if (control){
		editor_control_construct (control, vbox);
		return BONOBO_OBJECT (control);
	} else {
		gtk_widget_unref (vbox);
		return NULL;
	}
}
