/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of gnome-spell bonobo component

    Copyright (C) 2001 Ximian, Inc.
    Authors:      Radek Doulik <rodo@ximian.com>

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
*/

#include <config.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkmain.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkwindow.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-entry.h>
#include <glade/glade.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-property-bag-client.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-property-bag.h>

#include "Spell.h"
#include "control.h"

typedef struct {
	BonoboControl *control;
	GNOME_Spell_Dictionary dict;
	BonoboPropertyBag *pb;

	gchar *add_language;
	gchar *language;
	gchar *word;

	GtkWidget *label_word;
	GtkWidget *list_suggestions;
	GtkListStore *store_suggestions;
	GtkTreeIter iter_suggestions;

	GtkWidget *button_replace;
	GtkWidget *button_add;
	GtkWidget *button_ignore;
	GtkWidget *button_skip;
	GtkWidget *button_back;

	GtkWidget *combo_add;
	GtkWidget *entry_add;
	GList *abbrevs;
	GList *langs;
} SpellControlData;

enum {
	PROP_SPELL_WORD,
	PROP_SPELL_REPLACE,
	PROP_SPELL_ADD,
	PROP_SPELL_IGNORE,
	PROP_SPELL_SKIP,
	PROP_SPELL_BACK,
	PROP_SPELL_LANGUAGE,
	PROP_SPELL_SINGLE,
} SpellControlProps;

static void
control_get_prop (BonoboPropertyBag *bag,
		  BonoboArg         *arg,
		  guint              arg_id,
		  CORBA_Environment *ev,
		  gpointer           user_data)
{
	SpellControlData *cd = (SpellControlData *) user_data;

	switch (arg_id) {
	case PROP_SPELL_LANGUAGE:
		BONOBO_ARG_SET_STRING (arg, cd->language);
		printf ("get language %s\n", cd->language);
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static void
clicked_replace (GtkWidget *w, SpellControlData *cd)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *replacement, *language;

        if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (cd->list_suggestions)),
					     &model, &iter)) {
		CORBA_Environment ev;
                gtk_tree_model_get (model, &iter, 0, &replacement, 1, &language, -1);

		CORBA_exception_init (&ev);
		GNOME_Spell_Dictionary_setCorrection (cd->dict, cd->word, replacement, language, &ev);
		CORBA_exception_free (&ev);

		bonobo_pbclient_set_string (BONOBO_OBJREF (cd->pb), "replace", replacement, NULL);
		g_free (replacement);
	}
}

static void
abbrevs_langs_clear (SpellControlData *cd)
{
	if (cd->abbrevs) {
		g_list_foreach (cd->abbrevs, (GFunc) g_free, NULL);
		g_list_free (cd->abbrevs);
		cd->abbrevs = NULL;
	}
	if (cd->langs) {
		g_list_foreach (cd->langs, (GFunc) g_free, NULL);
		g_list_free (cd->langs);
		cd->langs = NULL;
	}
}

static void
set_entry_add (SpellControlData *cd)
{
	
	CORBA_sequence_GNOME_Spell_Language *language_seq;
	CORBA_Environment   ev;
	
	CORBA_exception_init (&ev);
	language_seq = GNOME_Spell_Dictionary_getLanguages (cd->dict, &ev);
	if (ev._major == CORBA_NO_EXCEPTION && language_seq) {
		gint i, n;

		abbrevs_langs_clear (cd);
		for (i = 0, n = 1; i < language_seq->_length; i++) {
			if (strstr (cd->language, language_seq->_buffer[i].abbreviation)) {
				cd->langs = g_list_append (cd->langs, g_strdup (_(language_seq->_buffer[i].name)));
				cd->abbrevs = g_list_append (cd->abbrevs, g_strdup (language_seq->_buffer[i].abbreviation));
			}
		}
		gtk_combo_set_popdown_strings (GTK_COMBO (cd->combo_add), cd->langs);
	}
	CORBA_exception_free (&ev);
}

static gchar *
get_abbrev (SpellControlData *cd)
{
	GList *l, *a;
	const gchar *text;

	text = gtk_entry_get_text (GTK_ENTRY (cd->entry_add));

	if (text)
		for (l = cd->langs, a = cd->abbrevs; l && a; l = l->next, a = a->next)
			if (!strcmp (text, l->data))
				return (gchar *) a->data;

	return NULL;
}

static void
clicked_add (GtkWidget *w, SpellControlData *cd)
{
	gchar *abbrev;

	/* printf ("set add\n"); */
	abbrev = get_abbrev (cd);
	if (abbrev) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		GNOME_Spell_Dictionary_addWordToPersonal (cd->dict, cd->word, abbrev, &ev);
		CORBA_exception_free (&ev);
		bonobo_pbclient_set_string (BONOBO_OBJREF (cd->pb), "add", abbrev, NULL);
	}
}

static void
clicked_ignore (GtkWidget *w, SpellControlData *cd)
{
	CORBA_Environment ev;

	/* printf ("set ignore\n"); */
	CORBA_exception_init (&ev);
	GNOME_Spell_Dictionary_addWordToSession (cd->dict, cd->word, &ev);
	CORBA_exception_free (&ev);

	bonobo_pbclient_set_boolean (BONOBO_OBJREF (cd->pb), "ignore", TRUE, NULL);
}

static void
clicked_skip (GtkWidget *w, SpellControlData *cd)
{
	bonobo_pbclient_set_boolean (BONOBO_OBJREF (cd->pb), "skip", TRUE, NULL);
}

static void
clicked_back (GtkWidget *w, SpellControlData *cd)
{
	bonobo_pbclient_set_boolean (BONOBO_OBJREF (cd->pb), "back", TRUE, NULL);
}

static void
set_word (SpellControlData *cd, gchar *word)
{
	GNOME_Spell_StringSeq *seq;
	CORBA_Environment ev;
	gchar *str;
	gint i;

	str = g_strdup_printf (_("Suggestions for '%s'"), word);
	gtk_label_set_text (GTK_LABEL (cd->label_word), str);
	g_free (str);

	g_free (cd->word);
	cd->word = g_strdup (word);

	/* printf ("set_word %s\n", word); */
	CORBA_exception_init (&ev);
	seq = GNOME_Spell_Dictionary_getSuggestions (cd->dict, word, &ev);
	CORBA_exception_free (&ev);
	if (seq) {
		gtk_list_store_clear (cd->store_suggestions);
		for (i = 0; i < seq->_length; i += 2) {

			gtk_list_store_append (cd->store_suggestions, &cd->iter_suggestions);
			gtk_list_store_set (cd->store_suggestions, &cd->iter_suggestions, 0, seq->_buffer [i], 1, seq->_buffer [i+1], -1);
		}
		gtk_widget_grab_focus (cd->list_suggestions);
		gtk_widget_set_sensitive (GTK_WIDGET (cd->button_replace), seq->_length != 0);

		CORBA_free (seq);
	}
}

static void
set_language (SpellControlData *cd, gchar *language)
{
	CORBA_Environment ev;

	g_free (cd->language);
	cd->language = g_strdup (language);

	printf ("set language %s\n", cd->language);

	CORBA_exception_init (&ev);
	GNOME_Spell_Dictionary_setLanguage (cd->dict, cd->language, &ev);
	CORBA_exception_free (&ev);

	set_entry_add (cd);
}

static void
control_set_prop (BonoboPropertyBag *bag,
		  const BonoboArg   *arg,
		  guint              arg_id,
		  CORBA_Environment *ev,
		  gpointer           user_data)
{
	SpellControlData *cd = user_data;
	
	switch (arg_id) {
	case PROP_SPELL_WORD:
		set_word (cd, BONOBO_ARG_GET_STRING (arg));
		break;
	case PROP_SPELL_LANGUAGE:
		set_language (cd, BONOBO_ARG_GET_STRING (arg));
		break;
	case PROP_SPELL_SINGLE:
		if (BONOBO_ARG_GET_BOOLEAN (arg)) {
			gtk_widget_hide (cd->button_skip);
			gtk_widget_hide (cd->button_back);
		}
		break;
	case PROP_SPELL_REPLACE:
	case PROP_SPELL_ADD:
	case PROP_SPELL_SKIP:
	case PROP_SPELL_BACK:
	case PROP_SPELL_IGNORE:
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static void
control_destroy (GObject *control, gpointer data)
{
	SpellControlData *cd;

	cd = (SpellControlData *) data;
	/* printf ("release spell control dict\n"); */
	bonobo_object_release_unref (cd->dict, NULL);

	g_free (cd);
}

static void
spell_control_construct (BonoboControl *control, GtkWidget *table, GladeXML *xml)
{
	BonoboArg *def;
	SpellControlData *cd;

	cd = g_new0 (SpellControlData, 1);
	cd->control = control;

	cd->label_word = glade_xml_get_widget (xml, "label_word");
	cd->list_suggestions = glade_xml_get_widget (xml, "list_suggestions");
	cd->store_suggestions = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (cd->list_suggestions), GTK_TREE_MODEL (cd->store_suggestions));
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (cd->list_suggestions),
				     gtk_tree_view_column_new_with_attributes (_("Suggestions"),
									       gtk_cell_renderer_text_new (),
									       "text", 0, NULL));
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (cd->list_suggestions)),
				     GTK_SELECTION_SINGLE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (cd->list_suggestions), FALSE);

	cd->button_replace = glade_xml_get_widget (xml, "button_replace");
	cd->button_add     = glade_xml_get_widget (xml, "button_add");
	cd->button_ignore  = glade_xml_get_widget (xml, "button_ignore");
	cd->button_skip    = glade_xml_get_widget (xml, "button_skip");
	cd->button_back    = glade_xml_get_widget (xml, "button_back");
	cd->combo_add      = glade_xml_get_widget (xml, "combo_add");
	cd->entry_add      = glade_xml_get_widget (xml, "entry_add");

	g_signal_connect (cd->button_replace, "clicked", G_CALLBACK (clicked_replace), cd);
	g_signal_connect (cd->button_add, "clicked", G_CALLBACK (clicked_add), cd);
	g_signal_connect (cd->button_ignore, "clicked", G_CALLBACK (clicked_ignore), cd);
	g_signal_connect (cd->button_skip, "clicked", G_CALLBACK (clicked_skip), cd);
	g_signal_connect (cd->button_back, "clicked", G_CALLBACK (clicked_back), cd);

	g_signal_connect (control, "destroy", G_CALLBACK (control_destroy), cd);

	/* PropertyBag */
	cd->pb = bonobo_property_bag_new (control_get_prop, control_set_prop, cd);
	bonobo_control_set_properties (control, BONOBO_OBJREF (cd->pb), NULL);
	bonobo_object_unref (BONOBO_OBJECT (cd->pb));

	bonobo_property_bag_add (cd->pb, "word", PROP_SPELL_WORD, BONOBO_ARG_STRING, NULL,
				 "checked word",  BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add (cd->pb, "language", PROP_SPELL_LANGUAGE, BONOBO_ARG_STRING, NULL,
				 "dictionary language", BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add (cd->pb, "single", PROP_SPELL_SINGLE, BONOBO_ARG_BOOLEAN, NULL,
				 "check single word", BONOBO_PROPERTY_WRITEABLE);

	def = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (def, "replace default value");
	bonobo_property_bag_add (cd->pb, "replace", PROP_SPELL_REPLACE, BONOBO_ARG_STRING, def,
				 "replacement to replace word",  BONOBO_PROPERTY_READABLE);
	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (def, "");
	bonobo_property_bag_add (cd->pb, "add", PROP_SPELL_REPLACE, BONOBO_ARG_STRING, def,
				 "add word to dictionary",  BONOBO_PROPERTY_READABLE);
	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);
	bonobo_property_bag_add (cd->pb, "ignore", PROP_SPELL_REPLACE, BONOBO_ARG_BOOLEAN, def,
				 "add word to session dictionary",  BONOBO_PROPERTY_READABLE);
	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);
	bonobo_property_bag_add (cd->pb, "skip", PROP_SPELL_REPLACE, BONOBO_ARG_BOOLEAN, def,
				 "skip this word",  BONOBO_PROPERTY_READABLE);
	CORBA_free (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);
	bonobo_property_bag_add (cd->pb, "back", PROP_SPELL_REPLACE, BONOBO_ARG_BOOLEAN, def,
				 "back to prev incorrect word",  BONOBO_PROPERTY_READABLE);
	CORBA_free (def);

	cd->dict = bonobo_get_object ("OAFIID:GNOME_Spell_Dictionary:" API_VERSION, "GNOME/Spell/Dictionary", NULL);

	/* set_language (cd, "en_us"); */
}

BonoboObject *
gnome_spell_control_new ()
{
	BonoboControl *control;
	GtkWidget *table;
	GladeXML *xml;

	xml = glade_xml_new (GLADE_DATADIR "/spell-checker.glade", "simple_control", NULL);
	if (!xml)
		g_error (_("Could not load glade file."));
	table = glade_xml_get_widget (xml, "simple_control");

	control = bonobo_control_new (table);

	if (control) {
		spell_control_construct (control, table, xml);
		return BONOBO_OBJECT (control);
	} else {
		gtk_widget_unref (table);
		return NULL;
	}
}
