/* $Id: gdict-app.c,v 1.1.1.3 2003-01-04 21:14:24 ghudson Exp $ */
/* -*- mode: c; style: k&r; c-basic-offset: 4 -*- */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict main window
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define GTK_ENABLE_BROKEN
#include <gnome.h>

#include "dict.h"
#include "gdict-about.h"
#include "gdict-pref.h"

#include "gdict-app.h"
#include "gdict-defbox.h"
#include "gdict-speller.h"

#ifdef HAVE_GNOME_PRINT
#  include <libgnomeprint/gnome-printer-dialog.h>
#endif /* HAVE_GNOME_PRINT */

#define APPNAME "gnome-dictionary"

#define GDICT_RESPONSE_FIND 100

GtkWidget *gdict_app;
GtkWidget *gdict_appbar;
GtkWidget *gnome_word_entry;
GtkWidget *word_entry;
GtkWidget *find_entry;
GDictDefbox *defbox;
GDictSpeller *speller;
GtkWidget *pref_dialog;
GtkWidget *socket_error_dialog;
gboolean  gail_loaded = FALSE;

static gchar *find_text = NULL;
static gboolean search_from_beginning = TRUE;

#ifdef HAVE_GNOME_PRINT
GnomePrinter *gdict_printer = NULL;
#endif /* HAVE_GNOME_PRINT */

dict_context_t *context;

static void gdict_edit_menu_set_sensitivity (gboolean flag);

static gint
socket_dialog_close_cb (GtkWidget *widget, gpointer data) 
{
    socket_error_dialog = NULL;
    return FALSE;
}

static void
socket_error_cb (GtkWidget *widget, gchar *message, gpointer data) 
{
    socket_error_dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                  		  GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                  		  "%s", message, NULL); 
    gtk_dialog_run (GTK_DIALOG (socket_error_dialog));
    gtk_widget_destroy (socket_error_dialog);
    
}


/* gdict_init_context
 *
 * Initialises the context object with information on the current server
 *
 * Retrurns 0 on success, -1 if the server could not be found
 */

gint
gdict_init_context (void) 
{
    if (context) 
      dict_context_destroy (context);
    
    context = dict_context_new (gdict_pref.server, gdict_pref.port);
    context->command = dict_disconnect_command_new ();
    
    defbox->context = context;

    if (context->hostinfo)
	return 0;
    else
	return -1;
}

void
gdict_app_clear (void) 
{
    gchar *word;
    
    gdict_defbox_clear (defbox);

    /* Update entry */
    if ((word = gdict_defbox_get_word (defbox))) {
        gtk_entry_set_text (GTK_ENTRY(word_entry), word);
        gtk_editable_select_region (GTK_EDITABLE(word_entry), 0, strlen(word));
    }
}

static void
spell_lookup_start_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_clear_stack(GNOME_APPBAR(gdict_appbar));
    gnome_appbar_push(GNOME_APPBAR(gdict_appbar), _("Spell-checking..."));
    gnome_appbar_refresh(GNOME_APPBAR(gdict_appbar));
}

static void
spell_lookup_done_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("Spell check done"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static void
spell_not_found_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("No matches found"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

void
gdict_open_speller (void) 
{
    if (!speller) {
        speller = GDICT_SPELLER (gdict_speller_new (context));
        
        if (!speller) return;

        gtk_signal_connect (GTK_OBJECT (speller), "word_lookup_start",
                            GTK_SIGNAL_FUNC (spell_lookup_start_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (speller), "word_lookup_done",
                            GTK_SIGNAL_FUNC (spell_lookup_done_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (speller), "word_not_found",
                            GTK_SIGNAL_FUNC (spell_not_found_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (speller), "socket_error",
                            GTK_SIGNAL_FUNC (socket_error_cb), NULL);
        gtk_widget_show_all (GTK_WIDGET (speller));
    }
    else
    	gtk_window_present (GTK_WINDOW (speller));
}


gint
gdict_spell (gchar *text, gboolean pattern) 
{
    g_return_val_if_fail(text != NULL, 0);

    gdict_open_speller ();

    if (!speller) return -1;

    if (pattern) 
	speller->strat = "re";
    else
	speller->strat = gdict_pref.dfl_strat;

    if (gdict_speller_lookup (speller, text) == -1) return -1;

    return 0;
}

static gboolean
is_pattern (gchar *text) 
{
    if (strpbrk (text, "*|{}()[]"))
	return TRUE;
    else
	return FALSE;
}

void
gdict_app_do_lookup (gchar *text) 
{
    gint retval;
    gchar *word_entry_text;
    
    g_return_if_fail (text != NULL);
    
    /* If needed update the the word_entry */
    word_entry_text = gtk_editable_get_chars (GTK_EDITABLE (word_entry), 0, -1);
    if ((word_entry_text == NULL) || (*word_entry_text == 0) || (strcmp (word_entry_text, text) != 0))
    {
        g_strdown (text);
        gnome_entry_prepend_history (GNOME_ENTRY (gnome_word_entry), 1, text);
        gtk_entry_set_text (GTK_ENTRY (word_entry), text);
    }

    g_free (word_entry_text);

    if (gdict_pref.smart && is_pattern (text)) {
	retval = gdict_spell (text, TRUE);
    }
    else {
	retval = gdict_defbox_lookup (defbox, text);
	if (!retval) gtk_widget_show (gdict_app);
    }

    if (retval) {
	gdict_not_online ();
    }
}

static void
lookup_entry (void) 
{
    gchar *text = gtk_editable_get_chars (GTK_EDITABLE (word_entry), 0, -1);
    g_strdown(text);
    gdict_app_do_lookup (text);
    if (text)
        g_free (text);
}

static void
lookup_defbox (void) 
{
    gchar *text = NULL;
    gtk_signal_emit_by_name (GTK_OBJECT (defbox), "copy_clipboard");
    gtk_entry_set_text (GTK_ENTRY (word_entry), "");
    gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "paste_clipboard");
    text = gtk_editable_get_chars (GTK_EDITABLE (word_entry), 0, -1);
    g_strdown (text);
    gnome_entry_prepend_history (GNOME_ENTRY (gnome_word_entry), 1, text);
    gdict_app_do_lookup (text);
    if (text)
        g_free (text);
}

static void
lookup_any (void) 
{
    /*if (GTK_WIDGET_HAS_FOCUS (word_entry))
	lookup_entry ();*/
    if (GTK_WIDGET_HAS_FOCUS (defbox))
	lookup_defbox ();
    else
        lookup_entry ();
}

static void
def_lookup_start_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_clear_stack(GNOME_APPBAR(gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("Looking up word..."));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static void
def_lookup_done_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("Lookup done"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
    gdict_edit_menu_set_sensitivity (TRUE);
}

static void
def_not_found_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("No matches found"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
    gdict_edit_menu_set_sensitivity (FALSE);
 
    if (gdict_pref.smart) {
        gdict_spell (gdict_defbox_get_word (defbox), FALSE);
    }
}

static void
def_substr_not_found_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("String not found"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static void
lookup_cb (GtkWidget *menuitem, gpointer user_data) 
{
    lookup_any();
}

static void
lookup_button_cb (GtkButton *button, gpointer data)
{
    lookup_any();
}

static void
lookup_button_drag_cb (GtkWidget *widget, GdkDragContext *context, gint x, gint y,
		       GtkSelectionData *sd, guint info, guint t, gpointer data)
{
	gchar *text;
	
	text = gtk_selection_data_get_text (sd);
	
	if (text) {
		g_strdown (text);
		gtk_entry_set_text (GTK_ENTRY (word_entry), text);
    		gnome_entry_prepend_history (GNOME_ENTRY (gnome_word_entry), 1, text);
    		gdict_app_do_lookup (text);
		g_free (text);
	}
}

static void
spell_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gchar *text;

    text = gtk_editable_get_chars (GTK_EDITABLE (word_entry), 0, -1);
    if (gdict_spell (text, FALSE) < 0)
	gdict_not_online ();
    if (text)
        g_free (text);
}

#ifdef HAVE_GNOME_PRINT

static void
print_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gdict_defbox_print (defbox);
}

static void
print_setup_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gdict_printer = gnome_printer_dialog_new_modal();
}

#endif /* HAVE_GNOME_PRINT */

static void
close_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gtk_widget_hide(gdict_app);
}

static void
exit_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    if (find_text)
    	g_free (find_text);
    gtk_main_quit();
}

static void
cut_cb (GtkWidget *button, gpointer user_data) 
{
    gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "cut_clipboard");
}

static void
copy_cb (GtkWidget *menuitem, gpointer user_data) 
{
    if (GTK_WIDGET_HAS_FOCUS (defbox))
        gtk_signal_emit_by_name (GTK_OBJECT (defbox), "copy_clipboard");
    else if (GTK_WIDGET_HAS_FOCUS (word_entry))
        gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "copy_clipboard");
}

static void
paste_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "paste_clipboard");
}

static void
clear_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gdict_edit_menu_set_sensitivity (FALSE);  
    gdict_defbox_clear (defbox);
}

static void
select_all_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    GtkTextBuffer* buffer = NULL;
    GtkTextIter start_iter, end_iter;
    
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (defbox));
    
    gtk_text_buffer_get_bounds (buffer, &start_iter, &end_iter);
    
    gtk_text_buffer_place_cursor (buffer, &end_iter);
    gtk_text_buffer_move_mark (buffer,
                               gtk_text_buffer_get_mark (buffer, "selection_bound"),
                               &start_iter);

}


static void
find_word_dialog_cb (GtkDialog *dialog, gint id, gpointer data)
{
     GtkEntry *entry;
     const gchar *str;

     entry = GTK_ENTRY (data);
    
     gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));

     switch (id) {
        case GTK_RESPONSE_DELETE_EVENT:
        case GTK_RESPONSE_CLOSE:
		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	case GDICT_RESPONSE_FIND:
		str = gtk_entry_get_text (entry);

		if (str == NULL)
			return;
			
		if (gdict_defbox_find (defbox, str, search_from_beginning))
		{
			search_from_beginning = FALSE;
			
			if (find_text != NULL)
				g_free (find_text);
			
			find_text = g_strdup (str);
		}
		else
			search_from_beginning = TRUE;
		
		break;
     }
}

static void
find_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    static GtkWidget *dialog = NULL;
    static GtkWidget *entry = NULL;
    GtkWidget *hbox;
    GtkWidget *label;
    
    if (dialog == NULL)
    {
    	dialog = gtk_dialog_new_with_buttons (_("Find"),
					  GTK_WINDOW (gdict_app),
					  GTK_DIALOG_DESTROY_WITH_PARENT,
					  GTK_STOCK_CLOSE,
					  GTK_RESPONSE_CLOSE,
					  NULL);
    	hbox = gtk_hbox_new (FALSE, 8);
	
    	label = gtk_label_new_with_mnemonic ("_Search for:");
    
    	entry = gtk_entry_new ();
   	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
    
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    	gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

    	gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
    
    	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox);
    	gdict_dialog_add_button (GTK_DIALOG (dialog), _("_Find"), 
		    	GTK_STOCK_FIND, GDICT_RESPONSE_FIND);
    
    	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GDICT_RESPONSE_FIND);
		    
   	g_signal_connect (G_OBJECT (dialog), "response", 
		      	  G_CALLBACK (find_word_dialog_cb), entry);

	g_signal_connect(G_OBJECT (dialog), "destroy",
			 G_CALLBACK (gtk_widget_destroyed), &dialog);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	gtk_widget_show_all (GTK_WIDGET (dialog));

	search_from_beginning = TRUE;
    }
    else
	gtk_window_present (GTK_WINDOW (dialog));

    if (find_text != NULL)
	    gtk_entry_set_text (GTK_ENTRY (entry), find_text);

    gtk_entry_select_region (GTK_ENTRY (entry), 0, -1);
    gtk_widget_grab_focus (entry);
}

static void
find_again_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    if (find_text)
        search_from_beginning = !gdict_defbox_find (defbox, find_text, search_from_beginning);
    else
	find_cb (NULL, NULL);
}

static void
pref_dialog_apply_cb (GtkWidget *widget, gpointer user_data) 
{
    gdict_defbox_reset (defbox, context);
    if (speller) 
	gdict_speller_reset (speller, context);
}

void
gdict_not_online () 
{
    GtkWidget *w;
    gchar *s;
    
    s = g_strdup_printf (_("Unable to perform requested operation. \n"
    		           "Either the server you are using is not available \n"
    		           "or you are not connected to the Internet."));
    w = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                     "%s", s, NULL); 
    gtk_dialog_run (GTK_DIALOG (w));
    gtk_widget_destroy (w);
    g_free (s);
}

void
gdict_app_show_preferences (void) 
{
	
    if (!pref_dialog)
        pref_dialog = gdict_pref_dialog_new (context);

    gtk_widget_show (pref_dialog);
}

static void
preferences_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gdict_app_show_preferences ();
}

static void
about_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gdict_about(GTK_WINDOW (user_data));
}


/*
 * add_atk_namedesc
 * @widget    : The Gtk Widget for which @name and @desc are added.
 * @name      : Accessible Name
 * @desc      : Accessible Description
 * Description: This function adds accessible name and description to a
 *              Gtk widget.
 */

void
add_atk_namedesc(GtkWidget *widget, const gchar *name, const gchar *desc)
{
    AtkObject *atk_widget;

    g_return_if_fail (GTK_IS_WIDGET(widget));

    atk_widget = gtk_widget_get_accessible(widget);
    
    if( name != NULL )
    atk_object_set_name(atk_widget, name);
    if( desc !=NULL )
    atk_object_set_description(atk_widget, desc);
}


/*
 * add_atk_relation
 * @obj1      : The first widget in the relation @rel_type
 * @obj2      : The second widget in the relation @rel_type.
 * @rel_type  : Relation type which relates @obj1 and @obj2
 * Description: This function establishes Atk Relation between two given
 *              objects.
 */

void
add_atk_relation(GtkWidget *obj1, GtkWidget *obj2, AtkRelationType rel_type)
{

    AtkObject *atk_obj1, *atk_obj2;
    AtkRelationSet *relation_set;
    AtkRelation *relation;

    g_return_if_fail (GTK_IS_WIDGET(obj1));
    g_return_if_fail (GTK_IS_WIDGET(obj2));

    atk_obj1 = gtk_widget_get_accessible(obj1);

    atk_obj2 = gtk_widget_get_accessible(obj2);

    relation_set = atk_object_ref_relation_set (atk_obj1);
    relation = atk_relation_new(&atk_obj2, 1, rel_type);
    atk_relation_set_add(relation_set, relation);
    g_object_unref(G_OBJECT (relation));

}

GtkWidget*
gdict_dialog_add_button (GtkDialog *dialog, const gchar* text, const gchar* stock_id, 
                         gint response_id)
{
        GtkWidget *button;

        g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);
        g_return_val_if_fail (text != NULL, NULL);
        g_return_val_if_fail (stock_id != NULL, NULL);

        button = gdict_button_new_with_stock_image (text, stock_id);
        g_return_val_if_fail (button != NULL, NULL);

        GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

        gtk_widget_show (button);

        gtk_dialog_add_action_widget (dialog, button, response_id);

        return button;
}

GtkWidget* 
gdict_button_new_with_stock_image (const gchar* text, const gchar* stock_id)
{
	GtkWidget *button;
	GtkStockItem item;
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *hbox;
	GtkWidget *align;

	button = gtk_button_new ();

 	if (GTK_BIN (button)->child)
    		gtk_container_remove (GTK_CONTAINER (button),
				      GTK_BIN (button)->child);

  	if (gtk_stock_lookup (stock_id, &item))
    	{
      		label = gtk_label_new_with_mnemonic (text);

		gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (button));
      
		image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
      		hbox = gtk_hbox_new (FALSE, 2);

      		align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
      
      		gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
      		gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
      
      		gtk_container_add (GTK_CONTAINER (button), align);
      		gtk_container_add (GTK_CONTAINER (align), hbox);
      		gtk_widget_show_all (align);

      		return button;
    	}

      	label = gtk_label_new_with_mnemonic (text);
      	gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (button));
  
  	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

  	gtk_widget_show (label);
  	gtk_container_add (GTK_CONTAINER (button), label);

	return button;
}

static GnomeUIInfo dictionary_menu_uiinfo[] = {
    GNOMEUIINFO_ITEM_STOCK (N_("_Look Up Word"),
			    N_("Lookup word in dictionary"),
			    lookup_cb, GNOME_STOCK_MENU_SEARCH),
    GNOMEUIINFO_ITEM_STOCK (N_("Check _Spelling"), 
			    N_("Check word spelling"), 
			    spell_cb, GNOME_STOCK_MENU_SPELLCHECK),
#ifdef HAVE_GNOME_PRINT
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_PRINT_ITEM (print_cb, NULL),
    GNOMEUIINFO_MENU_PRINT_SETUP_ITEM (print_setup_cb, NULL),
#endif /* HAVE_GNOME_PRINT */
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_QUIT_ITEM(exit_cb, NULL),
    GNOMEUIINFO_END
};

#ifdef HAVE_GNOME_PRINT
#define EXIT_FILE_MENU_ITEM 6
#else
#define EXIT_FILE_MENU_ITEM 3
#endif

static GnomeUIInfo applet_close_item = 
  GNOMEUIINFO_MENU_CLOSE_ITEM (close_cb, NULL);

static GnomeUIInfo edit_menu_uiinfo[] = {
    GNOMEUIINFO_MENU_CUT_ITEM (cut_cb, NULL),
    GNOMEUIINFO_MENU_COPY_ITEM (copy_cb, NULL),
    GNOMEUIINFO_MENU_PASTE_ITEM (paste_cb, NULL),
    GNOMEUIINFO_MENU_CLEAR_ITEM (clear_cb, NULL),
    GNOMEUIINFO_MENU_SELECT_ALL_ITEM (select_all_cb, NULL),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_FIND_ITEM (find_cb, NULL),
    GNOMEUIINFO_MENU_FIND_AGAIN_ITEM (find_again_cb, NULL),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_PREFERENCES_ITEM (preferences_cb, NULL),
    GNOMEUIINFO_END
};



static GnomeUIInfo help_menu_uiinfo[] = {
    GNOMEUIINFO_HELP(APPNAME),
    GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo menubar_uiinfo[] = {
    { GNOME_APP_UI_SUBTREE, N_("_Dictionary"), NULL, 
      dictionary_menu_uiinfo, NULL, NULL, (GnomeUIPixmapType) 0, 
      NULL, 0, (GdkModifierType) 0, NULL },
    GNOMEUIINFO_MENU_EDIT_TREE (edit_menu_uiinfo),
    GNOMEUIINFO_MENU_HELP_TREE (help_menu_uiinfo),
    GNOMEUIINFO_END
};

enum
{
  TARGET_STRING,
  TARGET_TEXT,
  TARGET_COMPOUND_TEXT,
  TARGET_UTF8_STRING,
  TARGET_TEXT_BUFFER_CONTENTS
};

GtkWidget *gdict_app_create (gboolean applet) {
    GtkWidget *dock;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *button;
    GtkWidget *label, *word_label;
    GtkWidget *scrolled;
    GtkWidget *defbox_vscrollbar;
    GtkTooltips *tooltips;
    static GtkTargetEntry drop_targets [] = {
    	{ "UTF8_STRING", 0, 0 },
  	{ "COMPOUND_TEXT", 0, 0 },
  	{ "TEXT", 0, 0 },
  	{ "text/plain", 0, 0 },
  	{ "STRING",     0, 0 }
    };
  
    tooltips = gtk_tooltips_new ();
  
    gdict_app = gnome_app_new ("gnome-dictionary", _("Dictionary"));

    gtk_window_set_default_size (GTK_WINDOW (gdict_app), 450, 330);
    
    dock = GNOME_APP (gdict_app)->dock;
    gtk_widget_show (dock);
    
    if (applet)  /* quick hack */
        memcpy (&dictionary_menu_uiinfo[EXIT_FILE_MENU_ITEM],
		&applet_close_item, sizeof(GnomeUIInfo));

    gnome_app_create_menus_with_data (GNOME_APP (gdict_app), menubar_uiinfo, GTK_WIDGET(gdict_app)); 
    gdict_edit_menu_set_sensitivity (FALSE);  
    
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gnome_app_set_contents (GNOME_APP (gdict_app), vbox);
    
    hbox = gtk_hbox_new (FALSE, 6);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    word_label = gtk_label_new_with_mnemonic (_("_Word: "));
    gtk_widget_show ( GTK_WIDGET(word_label) );
    gtk_box_pack_start (GTK_BOX (hbox), word_label, FALSE, FALSE, 0);
    
    button = gdict_button_new_with_stock_image (_("_Look Up Word"), GTK_STOCK_FIND);
    gtk_widget_show (button);
    gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_drag_dest_set (button, GTK_DEST_DEFAULT_ALL, drop_targets, 
    		       G_N_ELEMENTS (drop_targets), GDK_ACTION_COPY | GDK_ACTION_MOVE);
    g_signal_connect (G_OBJECT (button), "clicked",
    		      G_CALLBACK (lookup_button_cb), defbox);
    g_signal_connect (G_OBJECT (button), "drag_data_received",
    		      G_CALLBACK (lookup_button_drag_cb), defbox);

    gnome_word_entry = gnome_entry_new ("Wordentry");
    gnome_entry_set_max_saved (GNOME_ENTRY (gnome_word_entry), 10);
    gtk_widget_show (gnome_word_entry);
    gtk_box_pack_start (GTK_BOX (hbox), gnome_word_entry, TRUE, TRUE, 0);
    gnome_entry_set_max_saved (GNOME_ENTRY (gnome_word_entry), 50);
    word_entry = gnome_entry_gtk_entry(GNOME_ENTRY (gnome_word_entry));
   
    gtk_label_set_mnemonic_widget (GTK_LABEL (word_label), word_entry);
    GTK_WIDGET_SET_FLAGS (word_entry, GTK_CAN_FOCUS);
    gtk_widget_grab_focus (word_entry);

    if ( GTK_IS_ACCESSIBLE (gtk_widget_get_accessible(gnome_word_entry)))
    {
        gail_loaded = TRUE;
        add_atk_namedesc(GTK_WIDGET(word_label), _("Word"), _("Word"));
        add_atk_namedesc(gnome_word_entry, _("Word Entry"), _("Enter a Word or select one from the list below"));
        add_atk_namedesc(word_entry, _("Word Entry"), NULL);
        add_atk_relation(gnome_word_entry, GTK_WIDGET(word_label), ATK_RELATION_LABELLED_BY);
        add_atk_namedesc(GTK_WIDGET(button), NULL, _("Look Up for a Word"));
    }

    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled), GNOME_PAD_SMALL);    
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                        GTK_POLICY_AUTOMATIC,             
	       				GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_ETCHED_IN);

    gtk_widget_show (scrolled);
  
    defbox = GDICT_DEFBOX (gdict_defbox_new ());
    defbox_setup_tags (defbox);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (defbox), GNOME_PAD_SMALL);
    gtk_signal_connect (GTK_OBJECT (defbox), "word_lookup_start",
                        GTK_SIGNAL_FUNC (def_lookup_start_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "word_lookup_done",
                        GTK_SIGNAL_FUNC (def_lookup_done_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "word_not_found",
                        GTK_SIGNAL_FUNC (def_not_found_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "substr_not_found",
                        GTK_SIGNAL_FUNC (def_substr_not_found_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "socket_error",
                        GTK_SIGNAL_FUNC (socket_error_cb), defbox);
  
    gtk_widget_show (GTK_WIDGET (defbox));
    gtk_text_view_set_editable (GTK_TEXT_VIEW (defbox), FALSE);
    gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (defbox));

    gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 0);

    gdict_appbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_NEVER);
    gnome_app_set_statusbar (GNOME_APP (gdict_app), gdict_appbar);
    gnome_app_install_menu_hints (GNOME_APP (gdict_app), menubar_uiinfo);

    gtk_widget_show (gdict_appbar);

    if (!applet)
      gtk_signal_connect (GTK_OBJECT (gdict_app), "delete_event",
                          GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    else
      gtk_signal_connect (GTK_OBJECT (gdict_app), "delete_event",
                          GTK_SIGNAL_FUNC (gtk_widget_hide), NULL);
    
    gtk_signal_connect (GTK_OBJECT (word_entry), "activate",
                        GTK_SIGNAL_FUNC(lookup_cb), NULL);
    return gdict_app;
}

static void
gdict_edit_menu_set_sensitivity (gboolean flag)
{
  static gboolean sensitivity = TRUE;
  gint i, items[4] = {3, 4, 6, 7};

  if (! (flag ^ sensitivity))
    return; 
  for (i=0; i < 4; i++) {
    sensitivity = flag;
    gtk_widget_set_sensitive (GTK_WIDGET (edit_menu_uiinfo[items[i]].widget), flag);
  }
}
