/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2000 Helix Code, Inc.
   Authors:           Radek Doulik (rodo@helixcode.com)

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
#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <gal/widgets/e-scroll-frame.h>

#include "gtkhtml.h"
#include "gtkhtml-properties.h"

#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cursor.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlobject.h"
#include "htmlselection.h"

#include "menubar.h"
#include "spell.h"

#define DICTIONARY_IID "OAFIID:GNOME_Spell_Dictionary:0.2"
#define CONTROL_IID "OAFIID:GNOME_Spell_Control:0.2"

struct _SpellPopup {
	GtkHTMLControlData *cd;

	GtkWidget *window;
	GtkWidget *clist;

	gchar *misspeled_word;
	gboolean replace;
};
typedef struct _SpellPopup SpellPopup;

static void
destroy (GtkWidget *w, SpellPopup *sp)
{
	/* printf ("destroy popup\n"); */

	if (sp->replace) {
		gchar *replacement;

		if (GTK_CLIST (sp->clist)->selection) {
			CORBA_Environment ev;

			CORBA_exception_init (&ev);
			GNOME_Spell_Dictionary_setCorrection (sp->cd->dict, sp->misspeled_word, replacement, &ev);
			CORBA_exception_free (&ev);

			gtk_clist_get_text (GTK_CLIST (sp->clist),
					    GPOINTER_TO_INT (GTK_CLIST (sp->clist)->selection->data), 0, &replacement);
			html_engine_replace_spell_word_with (sp->cd->html->engine, replacement);

			/* printf ("replace: %s with: %s\n", sp->misspeled_word, replacement); */
		}
	}

	gtk_grab_remove (sp->window);
	if (sp->misspeled_word)
		g_free (sp->misspeled_word);
	sp->misspeled_word = NULL;
}

static void
key_press_event (GtkWidget *widget, GdkEventKey *event, SpellPopup *sp)
{
	switch (event->keyval) {
	case GDK_space:
	case GDK_Return:
	case GDK_KP_Enter:
		sp->replace = TRUE;
	case GDK_Escape:
	case GDK_Left:
	case GDK_Right:
		gtk_widget_destroy (sp->window);
		break;
	}
}

static void
select_row (GtkCList *clist, gint row, gint column, GdkEvent *event, SpellPopup *sp)
{
	if (event == NULL)
		return;

	if (event->type == GDK_BUTTON_PRESS) {
		sp->replace = TRUE;
		gtk_widget_destroy (sp->window);
	}
}

static void
fill_suggestion_clist (GtkWidget *clist, const gchar *word, GtkHTMLControlData *cd)
{
	GNOME_Spell_StringSeq *seq;
	CORBA_Environment      ev;
	const char * suggested_word [1];
	gint i;

	CORBA_exception_init (&ev);
	seq = GNOME_Spell_Dictionary_getSuggestions (cd->dict, word, &ev );

	if (ev._major != CORBA_SYSTEM_EXCEPTION) {
		for (i=0; i < seq->_length; i++) {
			suggested_word [0] = seq->_buffer [i];
			gtk_clist_append (GTK_CLIST (clist), (gchar **) suggested_word);
		}
		CORBA_free (seq);
	}
	CORBA_exception_free (&ev);
}

void
spell_suggestion_request (GtkHTML *html, const gchar *word, gpointer data)
{
	GtkHTMLControlData *cd;
	SpellPopup *sp;
	HTMLEngine *e = html->engine;
	GtkWidget *scroll_frame;
	GtkWidget *frame;
	gint x, y, xw, yw;

	/* printf ("spell_suggestion_request_cb %s\n", word); */

	cd = (GtkHTMLControlData *) data;
	sp = g_new (SpellPopup, 1);
	sp->cd = cd;
	sp->replace = FALSE;
	sp->misspeled_word = g_strdup (word);

	sp->window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_policy (GTK_WINDOW (sp->window), FALSE, FALSE, FALSE);
	gtk_widget_set_name (sp->window, "html-editor-spell-suggestions");
	scroll_frame = e_scroll_frame_new (NULL, NULL);
	e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (scroll_frame), GTK_SHADOW_IN);
	e_scroll_frame_set_policy (E_SCROLL_FRAME (scroll_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	sp->clist  = gtk_clist_new (1);
	gtk_clist_set_selection_mode (GTK_CLIST (sp->clist), GTK_SELECTION_BROWSE);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

	fill_suggestion_clist (sp->clist, word, cd);

	gtk_widget_set_usize (sp->window, gtk_clist_columns_autosize (GTK_CLIST (sp->clist)) + 40, 200);
	gtk_container_add (GTK_CONTAINER (scroll_frame), sp->clist);
	gtk_container_add (GTK_CONTAINER (frame), scroll_frame);
	gtk_container_add (GTK_CONTAINER (sp->window), frame);
	gtk_widget_show_all (frame);

	gdk_window_get_origin (GTK_WIDGET (html)->window, &xw, &yw);
	html_object_get_cursor_base (e->cursor->object, e->painter, e->cursor->offset, &x, &y);
	/* printf ("x: %d y: %d\n", x, y); */
	x += xw + e->leftBorder + 5;
	y += yw + e->cursor->object->ascent + e->cursor->object->descent + e->topBorder + 4;
	/* printf ("x: %d y: %d\n", x, y); */

	gtk_signal_connect (GTK_OBJECT (sp->window),   "key_press_event", key_press_event, sp);
	gtk_signal_connect (GTK_OBJECT (sp->window),   "destroy",         destroy, sp);
	gtk_signal_connect (GTK_OBJECT (sp->clist),    "select_row",      select_row, sp);

	gtk_widget_popup (sp->window, x, y);
	gtk_grab_add (sp->window);
	gtk_window_set_focus (GTK_WINDOW (sp->window), sp->clist);
}

BonoboObjectClient *
spell_new_dictionary (void)
{
	BonoboObjectClient *dictionary_client = bonobo_object_activate (DICTIONARY_IID, 0);

	if (!dictionary_client) {
		g_warning ("Cannot activate spell dictionary (iid:%s)", DICTIONARY_IID);
		return NULL;
	}

	return dictionary_client;
}

gboolean
spell_check_word (GtkHTML *html, const gchar *word, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	CORBA_Environment   ev;
	gboolean rv;

	if (!cd->dict)
		return TRUE;

	/* printf ("check word: %s\n", word); */
	CORBA_exception_init (&ev);
	rv = GNOME_Spell_Dictionary_checkWord (cd->dict, word, &ev);
	if (ev._major == CORBA_SYSTEM_EXCEPTION)
		rv = TRUE;
	CORBA_exception_free (&ev);

	return rv;
}

void
spell_add_to_session (GtkHTML *html, const gchar *word, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	CORBA_Environment   ev;

	g_return_if_fail (word);

	if (!cd->dict)
		return;

	CORBA_exception_init (&ev);
	GNOME_Spell_Dictionary_addWordToSession (cd->dict, word, &ev);
	CORBA_exception_free (&ev);
}

void
spell_add_to_personal (GtkHTML *html, const gchar *word, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	CORBA_Environment   ev;

	g_return_if_fail (word);

	if (!cd->dict)
		return;

	CORBA_exception_init (&ev);
	GNOME_Spell_Dictionary_addWordToPersonal (cd->dict, word, &ev);
	CORBA_exception_free (&ev);
}

void
spell_set_language (GtkHTML *html, const gchar *language, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	CORBA_Environment   ev;

	if (!cd->dict)
		return;

	/* printf ("spell_set_language %s\n", language); */

	CORBA_exception_init (&ev);
	GNOME_Spell_Dictionary_setLanguage (cd->dict, language, &ev);
	CORBA_exception_free (&ev);

	menubar_set_languages (cd, language);
}

void
spell_init (GtkHTML *html, GtkHTMLControlData *cd)
{
}

static void
set_word (GtkHTMLControlData *cd)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	html_engine_select_spell_word_editable (cd->html->engine);
	bonobo_property_bag_client_set_value_string (cd->spell_control_pb, "word",
						     html_engine_get_spell_word (cd->html->engine), &ev);
	CORBA_exception_free (&ev);
}

static gboolean
next_word (GtkHTMLControlData *cd)
{
	gboolean rv = TRUE;
	while (html_engine_forward_word (cd->html->engine)
	       && (rv = html_engine_spell_word_is_valid (cd->html->engine)))
		;

	return rv;
}

static void
check_next_word (GtkHTMLControlData *cd, gboolean update)
{
	HTMLEngine *e = cd->html->engine;

	html_engine_disable_selection (e);
	if (update)
		html_engine_spell_check (e);

	if (!cd->spell_check_next || next_word (cd)) {
		gnome_dialog_close (GNOME_DIALOG (cd->spell_dialog));
	} else {
		set_word (cd);
	}
}

static void 
replace_cb (BonoboListener    *listener,
	    char              *event_name, 
	    CORBA_any         *arg,
	    CORBA_Environment *ev,
	    gpointer           user_data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) user_data;

	/* printf ("replace '%s'\n", BONOBO_ARG_GET_STRING (arg)); */

	html_engine_replace_spell_word_with (cd->html->engine, BONOBO_ARG_GET_STRING (arg));
	check_next_word (cd, FALSE);
}

static void 
skip_cb (BonoboListener    *listener,
	    char              *event_name, 
	    CORBA_any         *arg,
	    CORBA_Environment *ev,
	    gpointer           user_data)
{
	check_next_word ((GtkHTMLControlData *) user_data, FALSE);
}

static void 
add_cb (BonoboListener    *listener,
	char              *event_name, 
	CORBA_any         *arg,
	CORBA_Environment *ev,
	gpointer           user_data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) user_data;
	gchar *word;

	word = html_engine_get_spell_word (cd->html->engine);
	g_return_if_fail (word);

	GNOME_Spell_Dictionary_addWordToPersonal (cd->dict, word, ev);
	g_free (word);
	check_next_word ((GtkHTMLControlData *) user_data, TRUE);
}

static void 
ignore_cb (BonoboListener    *listener,
	   char              *event_name, 
	   CORBA_any         *arg,
	   CORBA_Environment *ev,
	   gpointer           user_data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) user_data;
	gchar *word;

	word = html_engine_get_spell_word (cd->html->engine);
	g_return_if_fail (word);

	GNOME_Spell_Dictionary_addWordToSession (cd->dict, word, ev);
	g_free (word);
	check_next_word ((GtkHTMLControlData *) user_data, TRUE);
}

gboolean
spell_has_control ()
{
	GtkWidget *control;
	gboolean rv;

	control = bonobo_widget_new_control (CONTROL_IID, CORBA_OBJECT_NIL);
	rv = control != NULL;

	if (control)
		gtk_widget_unref (control);

	return rv;
}

void
spell_check_dialog (GtkHTMLControlData *cd, gboolean whole_document)
{
	GtkWidget *control;
	GtkWidget *dialog;
	guint position;

	position = cd->html->engine->cursor->position;
	cd->spell_check_next = whole_document;
	if (whole_document) {
		html_engine_disable_selection (cd->html->engine);
		html_engine_beginning_of_document (cd->html->engine);
	}

	if (html_engine_spell_word_is_valid (cd->html->engine))
		if (next_word (cd)) {
			html_engine_hide_cursor (cd->html->engine);
			html_cursor_jump_to_position (cd->html->engine->cursor, cd->html->engine, position);
			html_engine_show_cursor (cd->html->engine);

			gnome_ok_dialog (_("No misspelled word found"));

			return;
		}

	dialog  = gnome_dialog_new (_("Spell checker"), GNOME_STOCK_BUTTON_CLOSE, NULL);
	control = bonobo_widget_new_control (CONTROL_IID, CORBA_OBJECT_NIL);

	if (!control) {
		g_warning ("Cannot create spell control");
		gtk_widget_unref (dialog);
		return;
	}

	cd->spell_dialog = dialog;
        cd->spell_control_pb = bonobo_control_frame_get_control_property_bag
		(bonobo_widget_get_control_frame (BONOBO_WIDGET (control)), NULL);
	bonobo_property_bag_client_set_value_string (cd->spell_control_pb, "language",
						     GTK_HTML_CLASS (GTK_OBJECT (cd->html)->klass)->properties->language,
						     NULL);
	bonobo_event_source_client_add_listener (cd->spell_control_pb, replace_cb, "Bonobo/Property:change:replace", NULL, cd);
	bonobo_event_source_client_add_listener (cd->spell_control_pb, add_cb, "Bonobo/Property:change:add", NULL, cd);
	bonobo_event_source_client_add_listener (cd->spell_control_pb, ignore_cb, "Bonobo/Property:change:ignore",
						 NULL, cd);
	bonobo_event_source_client_add_listener (cd->spell_control_pb, skip_cb, "Bonobo/Property:change:skip", NULL, cd);
	set_word (cd);

	gtk_widget_show (control);
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (dialog)->vbox), control);
	gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
}

static void
language_cb (BonoboUIComponent *uic, const char *path, Bonobo_UIComponent_EventType type,
	     const char *state, gpointer user_data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) user_data;
	GString *str, *lang;
	gchar *val;
	gint i;

	if (cd->block_language_changes || !cd->languages)
		return;

	/* printf ("language callback %s\n", path); */

	str = g_string_new (NULL);
	lang = g_string_new (NULL);
	for (i = 0; i < cd->languages->_length; i ++) {
		g_string_sprintf (lang, "/commands/SpellLanguage%d", i + 1);
		val = bonobo_ui_component_get_prop (cd->uic, lang->str, "state", NULL);
		if (val && *val == '1') {
			g_string_append (str, cd->languages->_buffer [i].abrev);
			g_string_append_c (str, ' ');
		}
	}
	html_engine_set_language (cd->html->engine, str->str);
	g_string_free (str, TRUE);
	g_string_free (lang, TRUE);
}

void
spell_create_language_menu (GtkHTMLControlData *cd)
{
	CORBA_sequence_GNOME_Spell_Language *seq;
	CORBA_Environment ev;

	if (cd->dict == CORBA_OBJECT_NIL)
		return;

	CORBA_exception_init (&ev);
	cd->languages = seq = GNOME_Spell_Dictionary_getLanguages (cd->dict, &ev);

	if (ev._major == CORBA_NO_EXCEPTION) {
		if (seq && seq->_length > 0) {
			GString *str;
			gchar *line;
			gint i;

			str = g_string_new ("<submenu name=\"EditSpellLanguagesSubmenu\" _label=\"");
			g_string_append (str, _("Current _Languages"));
			g_string_append (str, "\">\n");
			for (i = 0; i < seq->_length; i ++) {
				line = g_strdup_printf ("<menuitem name=\"SpellLanguage%d\" _label=\"%s\""
							" verb=\"SpellLanguage%d\" type=\"toggle\"/>\n",
							i + 1, seq->_buffer [i].name, i + 1);
				g_string_append (str, line);
				g_free (line);
			}
			g_string_append (str, "</submenu>\n");

			bonobo_ui_component_set_translate (cd->uic, "/menu/Edit/EditMisc/EditSpellLanguages/",
							   str->str, NULL);

			for (i = 0; i < seq->_length; i ++) {
				g_string_sprintf (str, "SpellLanguage%d", i + 1);
				bonobo_ui_component_add_listener (cd->uic, str->str, language_cb, cd);
			}
			g_string_free (str, TRUE);
		}
	} else {
		g_warning ("CORBA exception: %s\n", bonobo_exception_get_text (&ev));
		cd->languages = NULL;
	}
	CORBA_exception_free (&ev);
}
