/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Jonas Borgström <jonas_b@bitsmart.com>.
    Copyright (C) 2000, 2001, 2002 Ximian, Inc.

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

/*
  FIXME
  we need to call deprecated GtkList, which is ised in non deprecated GtkCombo
  remove following preprocessor hackery once gtk+ is fixed
 */

#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
#include <gtk/gtklist.h>
#define GTK_DISABLE_DEPRECATED 1
#else
#include <gtk/gtklist.h>
#endif

#include <gtk/gtkcombo.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkcellrenderertext.h>
#include "htmlselect.h"
#include <string.h>

HTMLSelectClass html_select_class;
static HTMLEmbeddedClass *parent_class = NULL;

static void
free_strings (gpointer o, gpointer data)
{
	g_free (o);
}

static void
destroy (HTMLObject *o)
{
	HTMLSelect *select;

	select = HTML_SELECT (o);

	if (select->default_selection)
		g_list_free (select->default_selection);

	if (select->values) {

		g_list_foreach (select->values, free_strings, NULL);
		g_list_free (select->values);
	}

	if (select->strings) {

		g_list_foreach (select->strings, free_strings, NULL);
		g_list_free (select->strings);
	}

	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	/* FIXME TODO */
	HTMLSelect *s = HTML_SELECT (self);
	HTMLSelect *d = HTML_SELECT (dest);

	(* HTML_OBJECT_CLASS (parent_class)->copy) (self,dest);

	
	/* FIXME g_warning ("HTMLSelect::copy() is not complete."); */
	d->size =    s->size;
	d->multi =   s->multi;
	
	d->values = NULL;
	d->strings = NULL;
	d->default_selection = NULL;

	d->view = NULL;
}

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLSelect *select = HTML_SELECT (o);

	if (select->needs_update) {
		if (GTK_IS_COMBO (HTML_EMBEDDED (select)->widget)) {
			gtk_combo_set_popdown_strings (GTK_COMBO (HTML_EMBEDDED (o)->widget), select->strings);
			gtk_list_select_item (GTK_LIST (GTK_COMBO (HTML_EMBEDDED (o)->widget)->list), select->default_selected);
		}
	}

	select->needs_update = FALSE;	

	(* HTML_OBJECT_CLASS (parent_class)->draw) (o, p, x, y, width, height, tx, ty);
}

static void
select_row (GtkTreeSelection *selection, GtkTreeModel *model, gint r)
{
	GtkTreeIter iter;
	gchar *row;

	row = g_strdup_printf ("%d", r);

	if (gtk_tree_model_get_iter_from_string (model, &iter, row))
		gtk_tree_selection_select_iter (selection, &iter);
	g_free (row);
}

static void
reset (HTMLEmbedded *e)
{
	HTMLSelect *s = HTML_SELECT(e);
	GList *i = s->default_selection;
	gint row = 0;

	if (s->multi) {
		GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (s->view));

		gtk_tree_selection_unselect_all (selection);
		
		while (i) {
			if (i->data)
				select_row (selection, GTK_TREE_MODEL (s->store), row);

			i = i->next;
			row++;
		}		
	} else if (s->size > 1) {
		GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (s->view));

		select_row (selection, GTK_TREE_MODEL (s->store), s->default_selected);
	} else {
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (e->widget)->entry),
				    (gchar *) g_list_nth(s->strings, s->default_selected)->data);
	}
}

struct EmbeddedSelectionInfo {
	HTMLEmbedded *e;
	GString *str;
};

static void
add_selected (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	struct EmbeddedSelectionInfo *info = data;
	gchar *value, *ptr;

	gtk_tree_model_get (model, iter, 0, &value, -1);

	if (info->str->len)
		info->str = g_string_append_c (info->str, '&');

	ptr = html_embedded_encode_string (info->e->name);
	info->str = g_string_append (info->str, ptr);
	g_free (ptr);
					
	info->str = g_string_append_c (info->str, '=');
					
	ptr = html_embedded_encode_string (value);
	info->str = g_string_append (info->str, ptr);
	g_free (ptr);
}

static gchar *
encode (HTMLEmbedded *e)
{
	HTMLSelect *s = HTML_SELECT(e);
	GList *i;
	GString *encoding = g_string_new ("");
	const gchar *txt;
	gchar *ptr;

	if(strlen (e->name)) {
		if (s->size > 1) {
			struct EmbeddedSelectionInfo info;

			info.e = e;
			info.str = encoding;
			gtk_tree_selection_selected_foreach (gtk_tree_view_get_selection (GTK_TREE_VIEW (s->view)),
							     add_selected, &info);
			encoding = info.str;
		} else {
			gint item;

			ptr = html_embedded_encode_string (e->name);
			encoding = g_string_assign (encoding, ptr);
			g_free (ptr);
			encoding = g_string_append_c (encoding, '=');

			txt = gtk_entry_get_text (GTK_ENTRY(GTK_COMBO(e->widget)->entry));
			i = s->strings;
			item = 0;

			while (i) {

				if (strcmp(txt, (gchar *)i->data) == 0) {

					ptr = html_embedded_encode_string ((gchar *)g_list_nth (s->values, item)->data);
					encoding = g_string_append (encoding, ptr);
					g_free (ptr);
					
					break;
				}
				i = i->next;
				item++;
			}

		}
	}
	ptr = encoding->str;
	g_string_free(encoding, FALSE);
	
	return ptr;
}

void
html_select_type_init (void)
{
	html_select_class_init (&html_select_class, HTML_TYPE_SELECT, sizeof (HTMLSelect));
}

void
html_select_class_init (HTMLSelectClass *klass,
			HTMLType type,
			guint object_size)
{
	HTMLEmbeddedClass *element_class;
	HTMLObjectClass *object_class;

	element_class = HTML_EMBEDDED_CLASS (klass);
	object_class = HTML_OBJECT_CLASS (klass);

	html_embedded_class_init (element_class, type, object_size);

	/* HTMLEmbedded methods.   */
	element_class->reset = reset;
	element_class->encode = encode;

	/* HTMLObject methods.   */
	object_class->destroy = destroy;
	object_class->copy = copy;
	object_class->draw = draw;

	parent_class = &html_embedded_class;
}

void
html_select_init (HTMLSelect *select,
		      HTMLSelectClass *klass,
		      GtkWidget *parent,
		      gchar *name,
		      gint size,
		      gboolean multi)
{

	HTMLEmbedded *element;
	HTMLObject *object;
	GtkWidget *widget;

	element = HTML_EMBEDDED (select);
	object = HTML_OBJECT (select);

	html_embedded_init (element, HTML_EMBEDDED_CLASS (klass),
			   parent, name, NULL);

	if (size > 1 || multi) {
		GtkRequisition req;
		GtkTreeIter iter;

		select->store = gtk_list_store_new (1, G_TYPE_STRING);
		select->view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (select->store));

		gtk_tree_view_append_column (GTK_TREE_VIEW (select->view),
					     gtk_tree_view_column_new_with_attributes ("Labels",
										       gtk_cell_renderer_text_new (),
										       "text", 0, NULL));

		if (multi)
			gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (select->view)),
						     GTK_SELECTION_MULTIPLE);

		widget = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_IN);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

		gtk_container_add (GTK_CONTAINER (widget), select->view);
		gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (select->view), FALSE);
		gtk_widget_show_all (widget);

		gtk_list_store_append (select->store, &iter);
		gtk_list_store_set (select->store, &iter, 0, "height", -1);
		gtk_widget_size_request (select->view, &req);
		gtk_widget_set_size_request (select->view, 120, req.height * size);
		gtk_list_store_remove (select->store, &iter);
	} else {
		widget = gtk_combo_new ();
		gtk_editable_set_editable (GTK_EDITABLE (GTK_COMBO(widget)->entry), FALSE);
		gtk_widget_set_size_request ( GTK_WIDGET (widget), 120, -1);
	}
	html_embedded_set_widget (element, widget);

	select->size = size;
	select->multi = multi;
	select->default_selected = 0;
	select->values = NULL;
	select->strings = NULL;
	select->default_selection = NULL;
	select->needs_update = TRUE;
}

HTMLObject *
html_select_new (GtkWidget *parent,
		     gchar *name,
		     gint size,
		     gboolean multi)
{
	HTMLSelect *ti;

	ti = g_new0 (HTMLSelect, 1);
	html_select_init (ti, &html_select_class,
			      parent, name, size, multi);

	return HTML_OBJECT (ti);
}

void
html_select_add_option (HTMLSelect *select, gchar *value, gboolean selected)
{
	if(select->size > 1 || select->multi) {
		GtkTreeIter iter;

		gtk_list_store_append (select->store, &iter);
		gtk_list_store_set (select->store, &iter, 0, value, -1);

		if(selected) {

			select->default_selected = g_list_length (select->values) - 1;
			gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (select->view)), &iter);
		}
	} else {
		select->strings = g_list_append (select->strings, g_strdup (""));

		select->needs_update = TRUE;
		if (selected || g_list_length (select->strings) == 1)
			select->default_selected = g_list_length (select->strings) - 1;
	}

	select->values = g_list_append (select->values, g_strdup (value));

	if(select->multi)
		select->default_selection = g_list_append (select->default_selection, GINT_TO_POINTER(selected));
}

static char *
longest_string (HTMLSelect *s)
{
	GList *i = s->strings;
	gint max = 0;
	gchar *str = NULL;

	while (i) {
		if (strlen(i->data) > max) {
			max = strlen (i->data);
			str = i->data;
		}
		i = i->next;
	}
	return str;
}

void 
html_select_set_text (HTMLSelect *select, gchar *text) 
{
	GtkWidget *w = GTK_WIDGET (HTML_EMBEDDED (select)->widget);
	gint item;

	if (select->size > 1 || select->multi) {
		GtkRequisition req;
		GtkTreeIter iter;
		gchar *row;
		item = g_list_length (select->values) - 1;

		row = g_strdup_printf ("%d", item);
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (select->store), &iter, row);
		gtk_list_store_set (select->store, &iter, 0, text, -1);
		g_free (row);

		gtk_widget_size_request (GTK_WIDGET (select->view), &req);
		HTML_OBJECT (select)->width = req.width;

		/* Add width of scrollbar */

		if ((item + 1) > select->size && GTK_SCROLLED_WINDOW(w)->vscrollbar) {
			GtkRequisition req;

			gtk_widget_size_request (GTK_SCROLLED_WINDOW(w)->vscrollbar, &req);
			HTML_OBJECT (select)->width += req.width + 8;
		}

		gtk_widget_set_size_request (w, HTML_OBJECT(select)->width, -1);
	} else {
		w = HTML_EMBEDDED (select)->widget;
		item = g_list_length (select->strings) - 1;

		if (select->strings) {
			char *longest;
			GList *last = g_list_last (select->strings);
			
			g_free (last->data);
			last->data = g_strdup (text);

			select->needs_update = TRUE;
			gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(w)->entry), 
					    g_list_nth(select->strings, select->default_selected)->data);

			longest = longest_string (select);
			if (longest)
				gtk_entry_set_width_chars (GTK_ENTRY(GTK_COMBO(w)->entry), 
							   strlen (longest));
		}

		gtk_widget_set_size_request (w, -1, -1);
	}

	if (item >= 0 && g_list_nth (select->values, item)->data == NULL)
		g_list_nth (select->values, item)->data = g_strdup(text);
}


