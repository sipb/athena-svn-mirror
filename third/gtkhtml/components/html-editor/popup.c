/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.
    Authors:           Radek Doulik (rodo@helixcode.com)

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gal/widgets/e-unicode.h>

#include "gtkhtml.h"

#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmllinktext.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-table.h"
#include "htmlengine-edit-tablecell.h"
#include "htmlimage.h"
#include "htmlselection.h"
#include "htmltable.h"
#include "htmltablecell.h"

#include "body.h"
#include "cell.h"
#include "image.h"
#include "menubar.h"
#include "popup.h"
#include "properties.h"
#include "paragraph.h"
#include "spell.h"
#include "table.h"
#include "text.h"

/* #define DEBUG */
#ifdef DEBUG
#include "gtkhtmldebug.h"
#endif

static void
undo (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_undo (cd->html);
}

static void
redo (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_redo (cd->html);
}

static void
copy (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_copy (cd->html);
}

static void
cut (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_cut (cd->html);
}

static void
paste (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_paste (cd->html, FALSE);
}

static void
paste_cite (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_paste (cd->html, TRUE);
}

static void
insert_link (GtkWidget *mi, GtkHTMLControlData *cd)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"), ICONDIR "/insert-link-24.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_LINK, _("Link"),
						   link_insert,
						   link_insert_cb,
						   link_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, GTK_HTML_EDIT_PROPERTY_LINK);
}

static void
remove_link (GtkWidget *mi, GtkHTMLControlData *cd)
{
	html_engine_selection_push (cd->html->engine);
	if (!html_engine_is_selection_active (cd->html->engine))
		html_engine_select_word_editable (cd->html->engine);
	html_engine_edit_set_link (cd->html->engine, NULL, NULL);
	html_engine_selection_pop (cd->html->engine);
}

static void
insert_table_cb (GtkWidget *mi, GtkHTMLControlData *cd)
{
	insert_table (cd);
}

static void
insert_row_above (GtkWidget *mi, GtkHTMLControlData *cd)
{
	html_engine_insert_table_row (cd->html->engine, FALSE);
}

static void
insert_row_below (GtkWidget *mi, GtkHTMLControlData *cd)
{
	html_engine_insert_table_row (cd->html->engine, TRUE);
}

static void
insert_column_before (GtkWidget *mi, GtkHTMLControlData *cd)
{
	html_engine_insert_table_column (cd->html->engine, FALSE);
}

static void
insert_column_after (GtkWidget *mi, GtkHTMLControlData *cd)
{
	html_engine_insert_table_column (cd->html->engine, TRUE);
}

static void
delete_table (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_command (cd->html, "delete-table");
}

static void
delete_row (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_command (cd->html, "delete-table-row");
}

static void
delete_column (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_command (cd->html, "delete-table-column");
}

static void
delete_cell_contents (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_command (cd->html, "delete-cell-contents");
}

static void
show_prop_dialog (GtkHTMLControlData *cd, GtkHTMLEditPropertyType start)
{
	GtkHTMLEditPropertyType t;
	GList *cur;

	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);
	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, FALSE, _("Properties"), ICONDIR "/properties-16.png");

	cur = cd->properties_types;
	while (cur) {
		t = GPOINTER_TO_INT (cur->data);
		switch (t) {
		case GTK_HTML_EDIT_PROPERTY_TEXT:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Text"),
								   text_properties,
								   text_apply_cb,
								   text_close_cb);
			break;
		case GTK_HTML_EDIT_PROPERTY_LINK:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Link"),
								   link_properties,
								   link_apply_cb,
								   link_close_cb);
			break;
		case GTK_HTML_EDIT_PROPERTY_IMAGE:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Image"),
								   image_properties,
								   image_apply_cb,
								   image_close_cb);
								   break;
		case GTK_HTML_EDIT_PROPERTY_PARAGRAPH:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Paragraph"),
								   paragraph_properties,
								   paragraph_apply_cb,
								   paragraph_close_cb);
			break;
		case GTK_HTML_EDIT_PROPERTY_BODY:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Page"),
								   body_properties,
								   body_apply_cb,
								   body_close_cb);
			break;
		case GTK_HTML_EDIT_PROPERTY_RULE:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Rule"),
								   rule_properties,
								   rule_apply_cb,
								   rule_close_cb);
			break;
		case GTK_HTML_EDIT_PROPERTY_TABLE:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Table"),
								   table_properties,
								   table_apply_cb,
								   table_close_cb);
			break;
		case GTK_HTML_EDIT_PROPERTY_CELL:
			gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
								   t, _("Cell"),
								   cell_properties,
								   cell_apply_cb,
								   cell_close_cb);
			break;
		default:
			;
		}
		cur = cur->next;
	}

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	if (start > GTK_HTML_EDIT_PROPERTY_NONE)
		gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, start);
}

static void
prop_dialog (GtkWidget *mi, GtkHTMLControlData *cd)
{
	show_prop_dialog (cd, GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (mi), "type")));
}

static void
link_prop_dialog (GtkWidget *mi, GtkHTMLControlData *cd)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, FALSE, _("Properties"), ICONDIR "/insert-link-24.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_LINK, _("Link"),
						   link_properties,
						   link_apply_cb,
						   link_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, GTK_HTML_EDIT_PROPERTY_LINK);
}

static void
spell_suggest (GtkWidget *mi, GtkHTMLControlData *cd)
{
	HTMLEngine *e = cd->html->engine;

	/* gtk_signal_emit_by_name (GTK_OBJECT (cd->html), "spell_suggestion_request",
	   e->spell_checker, html_engine_get_word (e)); */
	spell_suggestion_request (cd->html, html_engine_get_spell_word (e), cd);
}

static void
spell_check_cb (GtkWidget *mi, GtkHTMLControlData *cd)
{
	spell_check_dialog (cd, FALSE);
}

static void
spell_add (GtkWidget *mi, GtkHTMLControlData *cd)
{
	HTMLEngine *e = cd->html->engine;
	gchar *word;

	word = html_engine_get_spell_word (e);
	if (word) {
		spell_add_to_personal (cd->html, word, cd);
		g_free (word);
	}
	html_engine_spell_check (e);
}

static void
spell_ignore (GtkWidget *mi, GtkHTMLControlData *cd)
{
	HTMLEngine *e = cd->html->engine;
	gchar *word;

	word = html_engine_get_spell_word (e);
	if (word) {
		spell_add_to_session (cd->html, word, cd);
		g_free (word);
	}
	html_engine_spell_check (e);
}

#ifdef DEBUG
static void
dump_tree_simple (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_debug_dump_tree_simple (cd->html->engine->clue, 0);
}

static void
dump_tree (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_debug_dump_tree (cd->html->engine->clue, 0);
}

static void
insert_html (GtkWidget *mi, GtkHTMLControlData *cd)
{
	gtk_html_insert_html (cd->html, "<BR>Hello dude!<BR><PRE>--\nrodo\n</PRE>");
}
#endif

#define ADD_ITEM_BASE(f,t) \
                gtk_object_set_data (GTK_OBJECT (menuitem), "type", GINT_TO_POINTER (GTK_HTML_EDIT_PROPERTY_ ## t)); \
		gtk_menu_append (GTK_MENU (menu), menuitem); \
		gtk_widget_show (menuitem); \
		gtk_signal_connect (GTK_OBJECT (menuitem), "activate", GTK_SIGNAL_FUNC (f), cd); \
                (*items)++; items_sep++

#define ADD_ITEM(l,f,t) \
		menuitem = gtk_menu_item_new_with_label (l); \
                ADD_ITEM_BASE (f,t)

#define ADD_ITEM_SENSITIVE(l,f,t,s) \
		menuitem = gtk_menu_item_new_with_label (l); \
                ADD_ITEM_BASE (f,t); \
                gtk_widget_set_sensitive (menuitem, s);

#define ADD_ITEM_UTF8(l,f,t) \
                menuitem = e_utf8_gtk_menu_item_new_with_label (GTK_MENU (menu), l); \
                ADD_ITEM_BASE (f,t)

#define ADD_SEP \
        if (items_sep) { \
                menuitem = gtk_menu_item_new (); \
                gtk_menu_append (GTK_MENU (menu), menuitem); \
                gtk_widget_show (menuitem); \
		items_sep = 0; \
        }

#define ADD_PROP(x) \
        cd->properties_types = g_list_append (cd->properties_types, GINT_TO_POINTER (GTK_HTML_EDIT_PROPERTY_ ## x))

#define SUBMENU(l) \
		        menuitem = gtk_menu_item_new_with_label (_(l)); \
			gtk_menu_append (GTK_MENU (menu), menuitem); \
			gtk_widget_show (menuitem); \
			(*items)++; items_sep++; \
			submenu = gtk_menu_new (); \
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu); \
			menuparent = menu; \
			menu = submenu

#define END_SUBMENU \
			gtk_widget_show (menu); \
			menu = menuparent

static GtkWidget *
prepare_properties_and_menu (GtkHTMLControlData *cd, guint *items)
{
	HTMLEngine *e = cd->html->engine;
	HTMLObject *obj;
	GtkWidget *menu;
	GtkWidget *submenu, *menuparent;
	GtkWidget *menuitem;
	guint items_sep = 0;
	gboolean active = FALSE;

	obj  = cd->html->engine->cursor->object;
	menu = gtk_menu_new ();

	if (cd->properties_types) {
		g_list_free (cd->properties_types);
		cd->properties_types = NULL;
	}

#ifdef DEBUG
	ADD_ITEM ("Dump tree (simple)", dump_tree_simple, NONE);
	ADD_ITEM ("Dump tree", dump_tree, NONE);
	ADD_ITEM ("Insert HTML", insert_html, NONE);
	ADD_SEP;
#endif
	active = html_engine_is_selection_active (e);
	ADD_ITEM (_("Undo"), undo, NONE); 
	ADD_ITEM (_("Redo"), redo, NONE); 

	ADD_SEP;
	ADD_ITEM_SENSITIVE (_("Copy"), copy, NONE, active);
	ADD_ITEM_SENSITIVE (_("Cut"),  cut, NONE, active);
	ADD_ITEM (_("Paste"),  paste, NONE);
	ADD_ITEM (_("Paste Quotation"),  paste_cite, NONE);

	ADD_SEP;
	ADD_ITEM (_("Insert link"), insert_link, NONE);
	if (cd->format_html
	    && ((active && html_engine_selection_contains_link (e))
		|| (obj
		    && (HTML_OBJECT_TYPE (obj) == HTML_TYPE_LINKTEXT
			|| (HTML_OBJECT_TYPE (obj) == HTML_TYPE_IMAGE
			    && (HTML_IMAGE (obj)->url
				|| HTML_IMAGE (obj)->target)))))) {
		ADD_ITEM (_("Remove link"), remove_link, NONE);
	}

	if (!active && obj && html_object_is_text (obj)
	    && !html_engine_spell_word_is_valid (e)) {
		gchar *spell, *word, *check_utf8, *add_utf8, *ignore_utf8, *ignore, *add;

		word   = html_engine_get_spell_word (e);
		check_utf8 = e_utf8_from_locale_string (_("Check '%s' spelling..."));
		spell  = g_strdup_printf (check_utf8, word);
		g_free (check_utf8);
		add_utf8 = e_utf8_from_locale_string (_("Add '%s' to dictionary"));
		add    = g_strdup_printf (add_utf8, word);
		g_free (add_utf8);
		ignore_utf8 = e_utf8_from_locale_string (_("Ignore '%s'"));
		ignore = g_strdup_printf (ignore_utf8, word);
		g_free (ignore_utf8);
		ADD_SEP;
		SUBMENU (N_("Spell checker"));
		if (cd->has_spell_control) {
			ADD_ITEM_UTF8 (spell, spell_check_cb, NONE);
		} else {
			ADD_ITEM (_("Suggest word"), spell_suggest, NONE);
		}
		ADD_ITEM_UTF8 (add, spell_add, NONE);
		ADD_ITEM_UTF8 (ignore, spell_ignore, NONE);
		END_SUBMENU;

		g_free (spell);
		g_free (add);
		g_free (ignore);
		g_free (word);
	}

	if (cd->format_html && obj) {
		switch (HTML_OBJECT_TYPE (obj)) {
		case HTML_TYPE_TEXT:
			ADD_SEP;
			ADD_ITEM (_("Text..."), prop_dialog, TEXT);
			ADD_PROP (TEXT);
			ADD_ITEM (_("Paragraph..."), prop_dialog, PARAGRAPH);
			ADD_PROP (PARAGRAPH);
			break;
		case HTML_TYPE_LINKTEXT:
			ADD_SEP;
			ADD_ITEM (_("Link..."), link_prop_dialog, LINK);
			ADD_PROP (LINK);
			ADD_ITEM (_("Paragraph..."), prop_dialog, PARAGRAPH);
			ADD_PROP (PARAGRAPH);
			break;
		case HTML_TYPE_RULE:
			ADD_SEP;
			ADD_ITEM (_("Rule..."), prop_dialog, RULE);
			ADD_PROP (RULE);
			break;
		case HTML_TYPE_IMAGE:
			ADD_SEP;
			ADD_ITEM (_("Image..."), prop_dialog, IMAGE);
			ADD_PROP (IMAGE);
			ADD_ITEM (_("Paragraph..."), prop_dialog, PARAGRAPH);
			ADD_PROP (PARAGRAPH);
			break;
		case HTML_TYPE_TABLE:
			ADD_SEP;
			ADD_PROP (TABLE);
			ADD_ITEM (_("Table..."), prop_dialog, TABLE);
		default:
		}
		if (obj->parent && obj->parent->parent && HTML_IS_TABLE_CELL (obj->parent->parent)) {
			if (cd->format_html) {
				ADD_SEP;
				ADD_PROP (CELL);
				ADD_ITEM (_("Cell..."), prop_dialog, CELL);
				if (obj->parent->parent->parent && HTML_IS_TABLE (obj->parent->parent->parent)) {
					ADD_PROP (TABLE);
					ADD_ITEM (_("Table..."), prop_dialog, TABLE);
				}
			}
			SUBMENU (N_("Table insert"));
			ADD_ITEM (_("Table"), insert_table_cb, NONE);
			ADD_SEP;
			ADD_ITEM (_("Row above"), insert_row_above, NONE);
			ADD_ITEM (_("Row below"), insert_row_below, NONE);
			ADD_SEP;
			ADD_ITEM (_("Column before"), insert_column_before, NONE);
			ADD_ITEM (_("Column after"), insert_column_after, NONE);
			END_SUBMENU;
			SUBMENU (N_("Table delete"));
			ADD_ITEM (_("Table"), delete_table, NONE);
			ADD_ITEM (_("Row"), delete_row, NONE);
			ADD_ITEM (_("Column"), delete_column, NONE);
			ADD_ITEM (_("Cell contents"), delete_cell_contents, NONE);
			END_SUBMENU;
		}
	}

	if (cd->format_html) {
		ADD_SEP;
		ADD_PROP (BODY);
		ADD_ITEM (_("Page..."), prop_dialog, BODY);
	}

	gtk_widget_show (menu);

	return menu;
}

gint
popup_show (GtkHTMLControlData *cd, GdkEventButton *event)
{
	GtkWidget *menu;
	guint items = 0;

	menu = prepare_properties_and_menu (cd, &items);
	if (items)
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
				event ? event->button : 0, event ? event->time : 0);
	gtk_widget_unref (menu);

	return (items > 0);
}

static void
set_position (GtkMenu *menu, gint *x, gint *y, gpointer data)
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *) data;
	HTMLEngine *e = cd->html->engine;
	gint xw, yw;

	gdk_window_get_origin (GTK_WIDGET (cd->html)->window, &xw, &yw);
	html_object_get_cursor_base (e->cursor->object, e->painter, e->cursor->offset, x, y);
	*x += xw + e->leftBorder;
	*y += yw + e->topBorder;
}

gint
popup_show_at_cursor (GtkHTMLControlData *cd)
{
	GtkWidget *menu;
	guint items = 0;

	menu = prepare_properties_and_menu (cd, &items);
	gtk_widget_show (menu);
	if (items)
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, set_position, cd, 0, 0);
	gtk_widget_unref (menu);

	return (items > 0);
}

void
property_dialog_show (GtkHTMLControlData *cd)
{
	guint items = 0;

	gtk_widget_unref (prepare_properties_and_menu (cd, &items));
	if (items)
		show_prop_dialog (cd, GTK_HTML_EDIT_PROPERTY_NONE);
}
