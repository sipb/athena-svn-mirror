/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2000, 2001, 2002 Ximian Inc.
   Authors:           Larry Ewing <lewing@ximian.com>
                      Radek Doulik <rodo@ximian.com>

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
#include <gnome.h>


#include "gtkhtml-propmanager.h"
#include "gtkhtml-properties.h"

#define d(x) x;

static GtkObject *parent_class;

enum {
	CHANGED,
	LAST_SIGNAL
};

enum {
	KEYMAP_EMACS,
	KEYMAP_XEMACS,
	KEYMAP_MS,
	KEYMAP_LAST
};

static char *keymap_names[KEYMAP_LAST + 1] = {"emacs", "xemacs", "ms", NULL};

static guint signals [LAST_SIGNAL] = { 0 };

struct _GtkHTMLPropmanagerPrivate {
	GladeXML *xml;

	GtkWidget *variable;
	GtkWidget *variable_print;
	GtkWidget *fixed;
	GtkWidget *fixed_print;
	GtkWidget *anim_check;
	GtkWidget *live_spell_check;
	GtkWidget *live_spell_options;
	GtkWidget *magic_links_check;
	GtkWidget *magic_smileys_check;
	GtkWidget *keymap;
	
	GtkHTMLClassProperties *saved_prop;
	GtkHTMLClassProperties *orig_prop;
	GtkHTMLClassProperties *actual_prop;

	GConfClient *client;
	
	guint notify_id;
	gboolean active;

	GHashTable *nametable;
};

static char *
keymap_option_get (GtkWidget *option)
{
	GtkWidget *active;
	char *name;
        active = gtk_menu_get_active (GTK_MENU (gtk_option_menu_get_menu (GTK_OPTION_MENU (option))));
	
	name = gtk_object_get_data (GTK_OBJECT (active), "GtkHTMLPropKeymap");

	return name ? name : "ms";
}

static void			       
keymap_option_set (GtkWidget *option, char *name)
{
	int i = 0;
	
	while (i < KEYMAP_LAST) {
		if (!strcmp (name, keymap_names[i])) {
			gtk_option_menu_set_history (GTK_OPTION_MENU (option), i);
		}
		i++;
	}
}


static void
gtk_html_propmanager_sync_gui (GtkHTMLPropmanager *pman)
{
	GtkHTMLPropmanagerPrivate *priv;

	g_return_if_fail (pman != NULL);
	priv = pman->priv;

	if (priv->anim_check)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->anim_check),
					      priv->actual_prop->animations);

	if (priv->magic_links_check)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->magic_links_check),
					      priv->actual_prop->magic_links);

	if (priv->magic_smileys_check)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->magic_smileys_check),
					      priv->actual_prop->magic_smileys);

	if (priv->live_spell_check)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->live_spell_check),
					      priv->actual_prop->live_spell_check);

	if (priv->live_spell_options) {	
		gtk_widget_set_sensitive (GTK_WIDGET (priv->live_spell_options),
					  priv->actual_prop->live_spell_check);
	}

	if (priv->keymap) {
		keymap_option_set (priv->keymap, priv->actual_prop->keybindings_theme);
	}

#define SET_FONT(f,w) \
        if (w) gnome_font_picker_set_font_name (GNOME_FONT_PICKER (w), priv->actual_prop-> f);

	SET_FONT (font_var,       priv->variable);
	SET_FONT (font_fix,       priv->fixed);
	SET_FONT (font_var_print, priv->variable_print);
	SET_FONT (font_fix_print, priv->fixed_print);
}
	
static void
propmanager_client_notify (GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer data)
{
	GtkHTMLPropmanager *pman = data;

	if (!pman->priv->active) {
		gtk_html_class_properties_load (pman->priv->actual_prop, client);
		gtk_html_propmanager_sync_gui (pman);
	} 
}

static void
propmanager_toggle_changed (GtkWidget *widget, GtkHTMLPropmanager *pman)
{
	g_return_if_fail (GTK_IS_TOGGLE_BUTTON (widget));

	gtk_signal_emit (GTK_OBJECT (pman), signals[CHANGED]);
}

static void
propmanager_keymap_changed (GtkWidget *menu, GtkHTMLPropmanager *pman)
{
	g_return_if_fail (GTK_IS_HTML_PROPMANAGER (pman));

	gtk_signal_emit (GTK_OBJECT (pman), signals[CHANGED]);	
}

static void
propmanager_font_changed (GtkWidget *picker, char *font_name, GtkHTMLPropmanager *pman)
{
	g_return_if_fail (GTK_IS_HTML_PROPMANAGER (pman));

	gtk_signal_emit (GTK_OBJECT (pman), signals[CHANGED]);
}

#define SELECTOR(x) GTK_FONT_SELECTION_DIALOG (GNOME_FONT_PICKER (x)->font_dialog)

static void
propmanager_picker_clicked (GtkWidget *w, gpointer proportional)
{
	gchar *mono_spaced [] = { "c", "m", NULL };

	if (!GPOINTER_TO_INT (proportional))
		gtk_font_selection_dialog_set_filter (SELECTOR (w),
						      GTK_FONT_FILTER_BASE, GTK_FONT_ALL,
						      NULL, NULL, NULL, NULL,
						      mono_spaced, NULL);
}

static void
propmanager_child_destroyed (GtkWidget *w, GtkHTMLPropmanager *pman)
{
	GtkHTMLPropmanagerPrivate *priv;

	g_return_if_fail (GTK_IS_HTML_PROPMANAGER (pman));
	priv = pman->priv;

	/* this is ugly but I am lazy */
#define MAYBE_CLEAR(x) \
        if (w == priv->x) priv->x = NULL;

	MAYBE_CLEAR (variable);
	MAYBE_CLEAR (variable_print);
	MAYBE_CLEAR (fixed_print);
	MAYBE_CLEAR (fixed);
	MAYBE_CLEAR (anim_check);
	MAYBE_CLEAR (live_spell_check);
	MAYBE_CLEAR (magic_links_check);
	MAYBE_CLEAR (magic_smileys_check);
	MAYBE_CLEAR (live_spell_options);
	MAYBE_CLEAR (keymap);

	gtk_object_unref (GTK_OBJECT (pman));
}

static GtkWidget *
propmanager_get_widget (GtkHTMLPropmanager *pman, char *name)
{
	char *xml_name = NULL;
	GtkWidget *widget;

	if (pman->priv->nametable)
		xml_name = g_hash_table_lookup (pman->priv->nametable, name);

	if (!xml_name)
		xml_name = name;

	widget = glade_xml_get_widget (pman->priv->xml, xml_name);

	if (widget) {
		gtk_object_ref (GTK_OBJECT (pman));

		d(g_warning ("found_widget: %s", name));

		gtk_signal_connect (GTK_OBJECT (widget), "destroy", 
				    propmanager_child_destroyed, pman);
	}

	return glade_xml_get_widget (pman->priv->xml, xml_name);
}

static GtkWidget *
propmanager_add_toggle (GtkHTMLPropmanager *pman,
			char *name,
			gboolean *found)
{
	GtkWidget *toggle;

	toggle = propmanager_get_widget (pman, name);

	if (toggle) {
		if (!GTK_IS_TOGGLE_BUTTON (toggle))
			return NULL;

		gtk_signal_connect (GTK_OBJECT (toggle), "toggled", propmanager_toggle_changed,
				    pman);

		*found = TRUE;
	}

	return toggle;
}

static GtkWidget *
propmanager_add_picker (GtkHTMLPropmanager *pman,
			char *name,
			gboolean proportional, 
			gboolean *found)
{
	GtkWidget *picker;

	picker = propmanager_get_widget (pman, name);

	if (picker) {
		if (!GNOME_IS_FONT_PICKER (picker))
			return NULL;

		gtk_signal_connect (GTK_OBJECT (picker), "font_set", propmanager_font_changed,
				    pman);
		gtk_signal_connect (GTK_OBJECT (picker), "clicked", propmanager_picker_clicked,
				    GINT_TO_POINTER (proportional));
		*found = TRUE;
	}
	return picker;
}

static GtkWidget *
propmanager_add_keymap (GtkHTMLPropmanager *pman, char *name, gboolean *found)
{
	GtkWidget *option;
	GtkWidget *menu;
	GList *items;
	gint i;

	option = propmanager_get_widget (pman, name);

	if (option) {
		if (!GTK_IS_OPTION_MENU (option)) 
			return NULL;

		menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (option));
		
		i = 0;
		items = GTK_MENU_SHELL (menu)->children;
		while (items && (i < KEYMAP_LAST)) {
			gtk_object_set_data (GTK_OBJECT (items->data), "GtkHTMLPropKeymap", keymap_names[i]);
			items = items->next;
			i++;
		}
		
		gtk_signal_connect (GTK_OBJECT (menu), "selection-done", propmanager_keymap_changed, pman);	
		
		*found = TRUE;
	}

	return option;
}

void
gtk_html_propmanager_set_names (GtkHTMLPropmanager *pman, char *names[][2])
{
	GHashTable *ht;
	int i;

	g_return_if_fail (pman != NULL);
	g_return_if_fail (names != NULL);

	ht = g_hash_table_new (g_str_hash, g_str_equal);

	i = 0;
	while (names[i][0] != NULL) {
		g_hash_table_insert (ht, names[i][0], names[i][1]);
		i++;
	}

	gtk_html_propmanager_set_nametable (pman, ht);
}

void
gtk_html_propmanager_set_nametable (GtkHTMLPropmanager *pman, GHashTable *ht)
{
	if (pman->priv->nametable)
		g_hash_table_destroy (pman->priv->nametable);

	pman->priv->nametable = ht;
}

gboolean
gtk_html_propmanager_set_gui (GtkHTMLPropmanager *pman, GladeXML *xml, GHashTable *nametable)
{
	GtkHTMLPropmanagerPrivate *priv;
	GError      *gconf_error  = NULL;
	gboolean found_widget = FALSE;

	g_return_val_if_fail (pman != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HTML_PROPMANAGER (pman), FALSE);

	if (nametable)
		gtk_html_propmanager_set_nametable (pman, nametable);

	priv = pman->priv;
	
	gtk_object_ref (GTK_OBJECT (xml));
	priv->xml = xml;

	gconf_client_add_dir (priv->client, GTK_HTML_GCONF_DIR, GCONF_CLIENT_PRELOAD_NONE, NULL);

	priv->orig_prop = gtk_html_class_properties_new ();
	priv->saved_prop = gtk_html_class_properties_new ();
	priv->actual_prop = gtk_html_class_properties_new ();

	gtk_html_class_properties_load (priv->actual_prop, priv->client);
	gtk_html_class_properties_copy (priv->saved_prop, priv->actual_prop);
	gtk_html_class_properties_copy (priv->orig_prop, priv->actual_prop);

	/* Toggle Buttons */
	priv->anim_check = propmanager_add_toggle (pman, "anim_check", &found_widget);
	priv->magic_links_check = propmanager_add_toggle (pman, "magic_links_check", &found_widget);
	priv->magic_smileys_check = propmanager_add_toggle (pman, "magic_smileys_check", &found_widget);
	priv->live_spell_check = propmanager_add_toggle (pman, "live_spell_check", &found_widget);

	if ((priv->live_spell_options = propmanager_get_widget (pman, "button_configure_spell_checking"))) {
		found_widget = TRUE;
	}

	/* KEYMAP */
	priv->keymap = propmanager_add_keymap (pman, "gtk_html_prop_keymap_option", &found_widget);

	/* Font Pickers */
	priv->variable = propmanager_add_picker (pman, "screen_variable", TRUE, &found_widget);
	priv->variable_print = propmanager_add_picker (pman, "print_variable", TRUE, &found_widget);
	priv->fixed = propmanager_add_picker (pman, "screen_fixed", FALSE, &found_widget);
	priv->fixed_print = propmanager_add_picker (pman, "print_fixed", FALSE, &found_widget);

	priv->notify_id = gconf_client_notify_add (priv->client, GTK_HTML_GCONF_DIR, 
						   propmanager_client_notify, 
						   pman, NULL, &gconf_error);
	if (gconf_error)
		g_warning ("gconf error: %s\n", gconf_error->message);
		

	/* only hold a ref while we retrieve the widgets */
	gtk_object_unref (GTK_OBJECT (priv->xml));
	priv->xml = NULL;

	gtk_html_propmanager_sync_gui (pman);
	return found_widget;
}

static gchar *
get_attr (gchar *font_name, gint n)
{
    gchar *s, *end;

    /* Search paramether */
    for (s=font_name; n; n--,s++)
	    s = strchr (s,'-');

    if (s && *s != 0) {
	    end = strchr (s, '-');
	    if (end)
		    return g_strndup (s, end - s);
	    else
		    return g_strdup (s);
    } else
	    return g_strdup ("Unknown");
}

void
gtk_html_propmanager_apply (GtkHTMLPropmanager *pman)
{
	GtkHTMLPropmanagerPrivate *priv;
	gchar *size_str;

	g_return_if_fail (pman != NULL);

	priv = pman->priv;
	
	if (priv->anim_check)
		priv->actual_prop->animations = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->anim_check));

	if (priv->magic_links_check)
		priv->actual_prop->magic_links = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->magic_links_check));

	if (priv->magic_smileys_check)
		priv->actual_prop->magic_smileys = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->magic_smileys_check));

	if (priv->live_spell_check)
		priv->actual_prop->live_spell_check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->live_spell_check));

	if (priv->keymap) {
		g_free (priv->actual_prop->keybindings_theme);
		priv->actual_prop->keybindings_theme = g_strdup (keymap_option_get (priv->keymap));
	}

#define APPLY(f,s,w) \
        if (w) { \
	        g_free (priv->actual_prop-> f); \
	        priv->actual_prop-> f = g_strdup (gnome_font_picker_get_font_name (GNOME_FONT_PICKER (w))); \
	        size_str = get_attr (gnome_font_picker_get_font_name (GNOME_FONT_PICKER (w)), 7); \
                if (!strcmp (size_str, "*")) { \
                          g_free (size_str); \
	                  size_str = get_attr (gnome_font_picker_get_font_name (GNOME_FONT_PICKER (w)), 8); \
                          priv->actual_prop-> f ## _points = TRUE; \
                } else { \
                          priv->actual_prop-> f ## _points = FALSE; \
                } \
	        priv->actual_prop-> s = atoi (size_str); \
	        g_free (size_str); \
	}

	APPLY (font_var,       font_var_size,       priv->variable);
	APPLY (font_fix,       font_fix_size,       priv->fixed);
	APPLY (font_var_print, font_var_size_print, priv->variable_print);
	APPLY (font_fix_print, font_fix_size_print, priv->fixed_print);

	priv->active = TRUE;
	gtk_html_class_properties_update (priv->actual_prop, priv->client,
					  priv->saved_prop);
	priv->active = FALSE;
	gtk_html_class_properties_copy (priv->saved_prop, priv->actual_prop);
}

void
gtk_html_propmanager_reset (GtkHTMLPropmanager *pman)
{
	GtkHTMLPropmanagerPrivate *priv;

	g_return_if_fail (GTK_IS_HTML_PROPMANAGER (pman));
	priv = pman->priv;
	
	gtk_html_class_properties_copy (priv->actual_prop, priv->orig_prop);
	gtk_html_class_properties_update (priv->actual_prop, priv->client,
					  priv->saved_prop);
	gtk_html_class_properties_copy (priv->saved_prop, priv->orig_prop);
	gtk_html_propmanager_sync_gui (pman);
}

static void
gtk_html_propmanager_real_changed (GtkHTMLPropmanager *pman)
{
	GtkHTMLPropmanagerPrivate *priv = pman->priv;

	if (priv->live_spell_options) {	
		gboolean sensitive;

		d (g_warning ("spell sensitivity changed = %d", priv->actual_prop->live_spell_check));

		sensitive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->live_spell_check));
		gtk_widget_set_sensitive (GTK_WIDGET (priv->live_spell_options), sensitive);
	}
}

static void
gtk_html_propmanager_init (GtkHTMLPropmanager *pman)
{
	GtkHTMLPropmanagerPrivate *priv;

	priv = g_new0 (GtkHTMLPropmanagerPrivate, 1);
	
	pman->priv = priv;

	gtk_object_ref (GTK_OBJECT (pman));
	gtk_object_sink (GTK_OBJECT (pman));
}

GtkObject *
gtk_html_propmanager_new (GConfClient *client)
{
	GtkHTMLPropmanager *pman;
	
	pman = GTK_HTML_PROPMANAGER (gtk_type_new (GTK_TYPE_HTML_PROPMANAGER));
	
	if (client) {
		pman->priv->client = client;
		gtk_object_ref (GTK_OBJECT (client));
	} else {
		pman->priv->client = gconf_client_get_default ();
	}

	return (GtkObject *)pman;
}

static void
gtk_html_propmanager_finalize (GtkObject *object)
{
	GtkHTMLPropmanagerPrivate *priv = GTK_HTML_PROPMANAGER (object)->priv;

	if (priv->notify_id)
		gconf_client_notify_remove (GTK_HTML_PROPMANAGER (object)->priv->client, priv->notify_id);

	if (priv->orig_prop) {
		gtk_html_class_properties_destroy (priv->orig_prop);
		gtk_html_class_properties_destroy (priv->actual_prop);
		gtk_html_class_properties_destroy (priv->saved_prop);
	}

	gtk_object_unref (GTK_OBJECT (priv->client));

	g_free (priv);

	if (GTK_OBJECT_CLASS (parent_class)->finalize)
		(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);		
}

static void
gtk_html_propmanager_class_init (GtkHTMLPropmanagerClass *klass)
{
	GtkObjectClass *object_class;
	object_class = (GtkObjectClass *)klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	signals [CHANGED] = 
		gtk_signal_new ("changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLPropmanagerClass, changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	object_class->finalize = gtk_html_propmanager_finalize;
	klass->changed = gtk_html_propmanager_real_changed;
}

	
GtkType
gtk_html_propmanager_get_type (void)
{
	static GtkType propmanager_type = 0;

	if (!propmanager_type) {
		GtkTypeInfo propmanager_type_info = {
			"GtkHTMLPropmanager",
			sizeof (GtkHTMLPropmanager),
			sizeof (GtkHTMLPropmanagerClass),
			(GtkClassInitFunc) gtk_html_propmanager_class_init,
			(GtkObjectInitFunc) gtk_html_propmanager_init,
			NULL,
			NULL
		};
		
		propmanager_type = gtk_type_unique (gtk_object_get_type (), &propmanager_type_info);
	}

	return propmanager_type;
}
