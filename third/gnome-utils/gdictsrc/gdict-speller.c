/* $Id: gdict-speller.c,v 1.1.1.3 2003-01-04 21:13:23 ghudson Exp $ */

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

#include <gnome.h>
#ifdef HAVE_GNOME_PRINT
#  include <libgnomeprint/gnome-print.h>
#  include <libgnomeprint/gnome-printer-dialog.h>
#  include <math.h>
#endif

#include "gdict-speller.h"
#include "gdict-pref.h"
#include "gdict-app.h"

enum {
    WORD_LOOKUP_START_SIGNAL,
    WORD_LOOKUP_DONE_SIGNAL,
    WORD_NOT_FOUND_SIGNAL,
    SOCKET_ERROR_SIGNAL,
    LAST_SIGNAL
};

#define GDICT_SPELL 100

GtkWidget *ss_label;

static gint gdict_speller_signals[LAST_SIGNAL] = { 0, 0, 0 };

static GnomeDialogClass *parent_class;

static void gdict_speller_init (GDictSpeller *speller);
static void gdict_speller_class_init (GDictSpellerClass *class);

static void speller_add_word  (GDictSpeller *speller, gchar *word);
static void speller_add_strat (GDictSpeller *speller, gchar *strat, gchar *desc);

static void spell_error_cb        (dict_command_t *command, DictStatusCode code, 
                                   gchar *message, gpointer data);
static void spell_word_status_cb  (dict_command_t *command, DictStatusCode code, 
                                   int num_found, gpointer data);
static void spell_strat_status_cb (dict_command_t *command, DictStatusCode code, 
                                   int num_found, gpointer data);
static void speller_set_strat_cb  (GtkWidget *widget, gpointer data);
static void spell_word_data_cb    (dict_command_t *command, dict_res_t *res,
                                   gpointer data);
static void spell_strat_data_cb   (dict_command_t *command, dict_res_t *res,
                                   gpointer data);
static void spell_response_cb	  (GtkDialog *dialog, gint id, gpointer data);
static void spell_button_pressed  (GtkButton *button, gpointer data);
static void row_selected_cb       (GtkTreeSelection *selection, gpointer data);
static void row_activated_cb      (GtkTreeView *tree, GtkTreePath *path, 
				   GtkTreeViewColumn *column, gpointer data);
static void spell_entry_cb	  (GtkEntry *entry, gpointer data);
static void speller_lookup        (GDictSpeller *speller);


/* gdict_speller_get_type
 *
 * Register the GDictSpeller type with Gtk's type system if necessary and
 * return the type identifier code
 */

GType
gdict_speller_get_type (void) {
    static GType gdict_speller_type = 0;
    
    if (!gdict_speller_type) {
        static const GTypeInfo gdict_speller_info = {
            sizeof (GDictSpellerClass),
            NULL,
            NULL,
            (GClassInitFunc) gdict_speller_class_init,
	    NULL,
	    NULL,
 	    sizeof (GDictSpeller),
	    0,
            (GInstanceInitFunc) gdict_speller_init
        };
        
        gdict_speller_type = 
            g_type_register_static (GTK_TYPE_DIALOG, "GDictSpeller", &gdict_speller_info, 0);
    }
    
    return gdict_speller_type;
}

/* gdict_speller_init
 *
 * Initialises an instance of a GDictSpeller object
 */

static void 
gdict_speller_init (GDictSpeller *speller) {
    GtkWidget *label, *button, *scrolled_win;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkListStore *model;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;
    GtkCellRenderer *cell;
    
    speller->context = NULL;
    speller->get_strat_cmd = NULL;
    speller->spell_cmd = NULL;
    speller->strat = gdict_pref.dfl_strat;
    
    vbox = gtk_vbox_new (FALSE, 8);
    g_object_set (G_OBJECT (vbox), "border_width", 6, NULL);
    
    hbox = gtk_hbox_new (FALSE, 8);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    label = gtk_label_new_with_mnemonic (_("_Word:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    
    speller->word_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (speller->word_entry));

    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (speller->word_entry), TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (speller->word_entry), "activate",
    	  	      G_CALLBACK (spell_entry_cb), speller);

    if (gail_loaded)
    {
        add_atk_namedesc (label, _("Word"), _("Word"));
        add_atk_namedesc (GTK_WIDGET(speller->word_entry), _("Word Entry"), _("Enter a word to know the spelling"));
        add_atk_relation (GTK_WIDGET(speller->word_entry), label, ATK_RELATION_LABELLED_BY);
    }
  
    if (gail_loaded)
        add_atk_namedesc (GTK_WIDGET (button), NULL, _("Click to do the spell check"));
    
    speller->hbox = gtk_hbox_new (FALSE, 8);
    gtk_box_pack_start (GTK_BOX (vbox), speller->hbox, FALSE, FALSE, 0);
    
    ss_label = gtk_label_new_with_mnemonic (_("Search S_trategy:"));
    gtk_box_pack_start (GTK_BOX (speller->hbox), ss_label, FALSE, FALSE, 0);
        
    
    hbox = gtk_hbutton_box_new ();
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_END);
		    
    button = gdict_button_new_with_stock_image (_("Check _Spelling"), GTK_STOCK_SPELL_CHECK);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (spell_button_pressed),
    		      speller);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    frame = gtk_frame_new (NULL);
    
    label = gtk_label_new_with_mnemonic (_("Search _Results"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    g_object_set (G_OBJECT (label), "xalign", 0.0, NULL);
   
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
    
    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    g_object_set (G_OBJECT (scrolled_win), "border_width", 6, NULL);

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_win),
					 GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_size_request (scrolled_win, 400, 250);
    
    model = gtk_list_store_new (1, G_TYPE_STRING);
    
    speller->word_list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (speller->word_list), FALSE);
    g_object_unref (G_OBJECT (model));
    
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (speller->word_list));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (speller->word_list));
    g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (row_selected_cb),
    		      speller);
    
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("words", 
    						       cell,
    						       "text", 0,
    						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, 0);
    
    /*gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                          0,GTK_SORT_ASCENDING); */
                                          
    gtk_tree_view_append_column (GTK_TREE_VIEW (speller->word_list), column);

    gtk_container_add (GTK_CONTAINER (scrolled_win), speller->word_list);
    gtk_container_add (GTK_CONTAINER (frame), scrolled_win);
   
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (speller)->vbox), vbox);
		    
    gtk_dialog_add_button (GTK_DIALOG (speller), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
    gdict_dialog_add_button (GTK_DIALOG (speller), _("_Look Up Word"), GTK_STOCK_FIND, GDICT_SPELL);
	   
    gtk_dialog_set_default_response (GTK_DIALOG (speller), GDICT_SPELL);
    g_signal_connect (G_OBJECT (speller->word_list), "row_activated",
    		      G_CALLBACK (row_activated_cb), speller);
    		      
    g_signal_connect (G_OBJECT (speller), "response", 
		      G_CALLBACK (spell_response_cb),
    		      speller);

    /* To prevent dialog destruction when pressing ESC */
    g_signal_connect (G_OBJECT (speller), "delete_event", 
		      G_CALLBACK (gtk_true), NULL);


    gtk_window_set_title (GTK_WINDOW (speller), _("Check Spelling"));
                           
    gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (speller)->vbox));
}

/* gdict_speller_class_init
 *
 * Initialises a structure describing the GDictSpeller class; sets up signals
 * for speller events in the Gtk signal management system
 */

static void 
gdict_speller_class_init (GDictSpellerClass *class) {
    GtkObjectClass *object_class;
    
    object_class = GTK_OBJECT_CLASS (class);

    gdict_speller_signals[WORD_LOOKUP_START_SIGNAL] =
        gtk_signal_new ("word_lookup_start", GTK_RUN_FIRST, GTK_CLASS_TYPE (object_class),
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, word_lookup_start),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_speller_signals[WORD_LOOKUP_DONE_SIGNAL] =
        gtk_signal_new ("word_lookup_done", GTK_RUN_FIRST, GTK_CLASS_TYPE (object_class),
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, word_lookup_done),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_speller_signals[WORD_NOT_FOUND_SIGNAL] =
        gtk_signal_new ("word_not_found", GTK_RUN_FIRST, GTK_CLASS_TYPE (object_class),
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, word_not_found),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_speller_signals[SOCKET_ERROR_SIGNAL] =
        gtk_signal_new ("socket_error", GTK_RUN_FIRST, GTK_CLASS_TYPE (object_class),
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, socket_error),
                        gtk_marshal_VOID__STRING, GTK_TYPE_NONE, 1,
                        GTK_TYPE_STRING);
    
    parent_class = g_type_class_peek_parent (class);
    class->word_lookup_done = NULL;
    class->word_not_found = NULL;

    object_class->destroy = (void (*) (GtkObject *)) gdict_speller_destroy;
}

/* gdict_speller_new
 *
 * Creates a new GDictSpeller object
 */

GtkWidget *
gdict_speller_new (dict_context_t *context) {
    GDictSpeller *speller;
    
    g_return_val_if_fail (context != NULL, NULL);
    
    speller = GDICT_SPELLER (gtk_type_new (gdict_speller_get_type ()));
    speller->context = context;
    
    gdict_speller_reset_strat (speller);
    
    return GTK_WIDGET (speller);
}

/* gdict_speller_destroy
 *
 * Destroys a speller dialog
 */

void
gdict_speller_destroy (GDictSpeller *speller) {

    g_free (speller->database);
    dict_command_destroy (speller->get_strat_cmd);
    dict_command_destroy (speller->spell_cmd);
    
}

/* gdict_speller_lookup
 *
 * Sends the command to the server to commence looking up matches for a word
 * of a word and sets the callbacks so that the definition will be displayed
 * in this speller
 *
 * Returns 0 on success and -1 on command invocation error
 */

gint 
gdict_speller_lookup (GDictSpeller *speller, gchar *text) {
    g_return_val_if_fail (speller != NULL, -1);
    g_return_val_if_fail (IS_GDICT_SPELLER (speller), -1);
    g_return_val_if_fail (text != NULL, -1);
    
    while (isspace (*text)) text++;
    
    if (*text == '\0')
        return 0;
    
    gtk_signal_emit (GTK_OBJECT (speller), 
                     gdict_speller_signals[WORD_LOOKUP_START_SIGNAL]);
    
    gdict_speller_clear (speller);
    
    gtk_entry_set_text (speller->word_entry, text);
    gtk_editable_select_region (GTK_EDITABLE (speller->word_entry), 
                                0, strlen (text));
    
    if (speller->database) g_free (speller->database);
    
    speller->database = g_strdup (gdict_pref.database);
    
    speller->spell_cmd = 
        dict_match_command_new (speller->database, speller->strat, text);
    speller->spell_cmd->error_notify_cb = spell_error_cb;
    speller->spell_cmd->status_notify_cb = spell_word_status_cb;
    speller->spell_cmd->data_notify_cb = spell_word_data_cb;
    speller->spell_cmd->user_data = speller;
    
    if (dict_command_invoke (speller->spell_cmd, speller->context) < 0)
      return -1;
    
    return 0;
}

/* gdict_speller_clear
 *
 * Clears the text in a speller and eliminates the current command structure
 */

void 
gdict_speller_clear (GDictSpeller *speller) {
    GtkTreeModel *model;
    g_return_if_fail (speller != NULL);
    g_return_if_fail (IS_GDICT_SPELLER (speller));
    
    gtk_entry_set_text (speller->word_entry, "");
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (speller->word_list));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    speller->current_word = NULL;
    
    if (speller->spell_cmd) {
        dict_command_destroy (speller->spell_cmd);
        speller->spell_cmd = NULL;
    }
}

/* gdict_speller_reset
 *
 * Reinvokes the search with a new context and preferences
 */

void
gdict_speller_reset (GDictSpeller *speller, dict_context_t *context) {
    gchar *word;
    
    /* If we have a new context, we cannot trust that the search strategy
     * we were using will still be present
     */
    
    if (context != speller->context) {
        speller->context = context;
        speller->strat = gdict_pref.dfl_strat;
        gdict_speller_reset_strat (speller);
    }
    
    /* Re-invoke current query only if there is a new database specified */
    
    if (context != speller->context ||
        strcmp (speller->database, gdict_pref.database))
    {
        if (speller->spell_cmd) {
            word = g_strdup (speller->spell_cmd->search_term);
            dict_command_destroy (speller->spell_cmd);
            speller->spell_cmd = NULL;
            gdict_speller_lookup (speller, word);
            g_free (word);
        }
    }
}

/* gdict_speller_get_word
 *
 * Returns the word defined in the speller, if any
 */

gchar *
gdict_speller_get_word (GDictSpeller *speller) {
    g_return_val_if_fail (speller != NULL, NULL);
    g_return_val_if_fail (IS_GDICT_SPELLER (speller), NULL);
    
    return speller->spell_cmd ? speller->spell_cmd->search_term : NULL;
}

/* gdict_speller_reset_strat
 *
 * Resets the list of strategies
 */

void
gdict_speller_reset_strat (GDictSpeller *speller) {
    GtkWidget *error_label, *alignment;

    if (speller->get_strat_cmd)
	dict_command_destroy (speller->get_strat_cmd);
    
    if (speller->strat_sel) {
        gtk_option_menu_remove_menu (speller->strat_sel);
        gtk_widget_destroy (GTK_WIDGET (speller->strat_sel));
        gtk_widget_show_all (GTK_WIDGET (speller->hbox));
        speller->strat_sel = NULL;
    }
    
    speller->strat_list = GTK_MENU (gtk_menu_new ());
    
    speller->get_strat_cmd = dict_show_strat_command_new ();
    speller->get_strat_cmd->error_notify_cb = spell_error_cb;
    speller->get_strat_cmd->data_notify_cb = spell_strat_data_cb;
    speller->get_strat_cmd->status_notify_cb = spell_strat_status_cb;
    speller->get_strat_cmd->user_data = speller;
    speller->strat_idx = 0;
    
    if (dict_command_invoke (speller->get_strat_cmd,
			     speller->context) == -1) 
    {
	/* Could not look up search strategies, so just display a
	 * label; FIXME: Memory leak
	 */
	error_label = gtk_label_new (_("Cannot connect to server"));
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (alignment), 
                           GTK_WIDGET (error_label));
        gtk_box_pack_start (GTK_BOX (speller->hbox), alignment, TRUE, TRUE, 0);
        gtk_widget_show_all (GTK_WIDGET (speller->hbox));
    }
}

/* spell_error_cb
 *
 * Callback invoked when there was an error in the last query
 */

static void
spell_error_cb (dict_command_t *command, DictStatusCode code,
                gchar *message, gpointer data)
{
    GtkWindow *speller;
    gchar *string;
    
    speller = GTK_WINDOW (data);
    
    if (code != DICT_SOCKET_ERROR) {
        GtkWidget *dialog;
        string = g_strdup_printf (_("Error invoking query: %s"), message);
        dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                  	 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                  	 "%s", string, NULL); 
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
        g_free (string);
        if (command->cmd == C_MATCH)
          gtk_signal_emit (GTK_OBJECT (speller), 
                           gdict_speller_signals[WORD_LOOKUP_DONE_SIGNAL]);
    }
    else {
        gtk_signal_emit (GTK_OBJECT (speller),
                         gdict_speller_signals[SOCKET_ERROR_SIGNAL], message);
    }
}

/* speller_add_word
 *
 * Adds a word to the word list
 */

static void 
speller_add_word (GDictSpeller *speller, gchar *word) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (speller->word_list));
    
    gtk_list_store_insert (GTK_LIST_STORE (model), &iter, 0);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, word, -1);
    
    if (!speller->current_word) speller->current_word = word;
}

/* speller_set_strat_cb
 *
 * Sets the current search strategy to the one indicated
 */

static void
speller_set_strat_cb (GtkWidget *widget, gpointer data) {
    GDictSpeller *speller;
    gchar *text;
    gchar *strat;
    
    speller = GDICT_SPELLER (data);
    strat = gtk_object_get_data (GTK_OBJECT (widget), "strat_name");
    speller->strat = strat;

    text = gtk_editable_get_chars (GTK_EDITABLE (speller->word_entry), 0, -1);
    gtk_editable_select_region (GTK_EDITABLE (speller->word_entry), 0,
                                strlen (text));
    gdict_speller_lookup (speller, text);
    if (text)
    	g_free (text);
}

/* speller_add_strat
 *
 * Adds a search strategy to the search strategy list
 */

static void
speller_add_strat (GDictSpeller *speller, gchar *strat, gchar *desc) {
    GtkWidget *menu_item;
    GtkWidget *label;
    
    label = gtk_label_new (desc);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    
    menu_item = gtk_menu_item_new ();
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", 
                        GTK_SIGNAL_FUNC (speller_set_strat_cb), speller);
    gtk_object_set_data (GTK_OBJECT (menu_item), "strat_name", strat);
    gtk_container_add (GTK_CONTAINER (menu_item), label);
    gtk_widget_show_all (menu_item);
    gtk_menu_append (speller->strat_list, menu_item);
    
    if (!strcmp (speller->strat, strat))
      gtk_menu_set_active (speller->strat_list, speller->strat_idx);
    speller->strat_idx++;
}

/* spell_word_data_cb
 *
 * Callback used when a new word has arrived over the link
 */

static void 
spell_word_data_cb (dict_command_t *command, dict_res_t *res, gpointer data) {
    GDictSpeller *speller;
    
    speller = GDICT_SPELLER (data);
    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;
    speller_add_word (speller, res->desc);
}

/* spell_strat_data_cb
 *
 * Callback used when a new strategy definition has arrived over the link
 */

static void
spell_strat_data_cb (dict_command_t *command, dict_res_t *res, gpointer data) {
    GDictSpeller *speller;
    
    speller = GDICT_SPELLER (data);
    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;
    speller_add_strat (speller, res->name, res->desc);
}

/* spell_word_status_cb
 *
 * Callback used when a status code has arrived over the link
 */

static void 
spell_word_status_cb (dict_command_t *command, DictStatusCode code, 
                      int num_found, gpointer data)
{
    GDictSpeller *speller;
    
    speller = GDICT_SPELLER (data);

    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;
    
    if (code == DICT_STATUS_OK)
      gtk_signal_emit (GTK_OBJECT (speller), 
                       gdict_speller_signals[WORD_LOOKUP_DONE_SIGNAL]);
    else if (code == DICT_STATUS_NO_MATCH)
      gtk_signal_emit (GTK_OBJECT (speller), 
                       gdict_speller_signals[WORD_NOT_FOUND_SIGNAL]);
}

/* spell_strat_status_cb
 *
 * Callback used when a status code has arrived over the link
 */

static void 
spell_strat_status_cb (dict_command_t *command, DictStatusCode code, 
                       int num_found, gpointer data)
{
    GDictSpeller *speller;
    GtkWidget *alignment;
    
    speller = GDICT_SPELLER (data);
    
    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;

    if (code == DICT_STATUS_OK) {
        speller->strat_sel = GTK_OPTION_MENU (gtk_option_menu_new ());
        gtk_option_menu_set_menu (speller->strat_sel, 
                                  GTK_WIDGET (speller->strat_list));
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (alignment), 
                           GTK_WIDGET (speller->strat_sel));
        gtk_box_pack_start (GTK_BOX (speller->hbox), alignment, TRUE, TRUE, 0);
        gtk_widget_show_all (GTK_WIDGET (speller->hbox));

        gtk_label_set_mnemonic_widget (GTK_LABEL (ss_label), GTK_WIDGET (speller->strat_sel));        

        if ( gail_loaded )
        {
            add_atk_namedesc(ss_label, _("Search Strategy"), _("Search Strategy"));
            add_atk_relation( GTK_WIDGET (speller->strat_sel), ss_label, ATK_RELATION_LABELLED_BY);
        }
 
    }
}

static void
spell_response_cb (GtkDialog *dialog, gint id, gpointer data) {
	GDictSpeller *speller;
        
	g_return_if_fail (data);
	speller = GDICT_SPELLER (data);
	
	switch (id) {
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CLOSE:
		gtk_widget_hide (GTK_WIDGET (dialog));
		break;
	case GDICT_SPELL:
		speller_lookup (speller);
		break;
	default:
		break;
	}
	
}

static void
spell_button_pressed (GtkButton *button, gpointer data) {
    GDictSpeller *speller;
    gchar *text;
    
    g_return_if_fail (data != NULL);
    
    speller = GDICT_SPELLER (data);
    text = gtk_editable_get_chars (GTK_EDITABLE (speller->word_entry), 0, -1);
    gtk_editable_select_region (GTK_EDITABLE (speller->word_entry), 0,
                                strlen (text));
    gdict_speller_lookup (speller, text);
    if (text)
    	g_free (text);

}

static void
row_selected_cb (GtkTreeSelection *selection, gpointer data) {
    GDictSpeller *speller;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *word;
    g_return_if_fail (data != NULL);
    
    speller = GDICT_SPELLER (data);
    
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    	return;
    
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (speller->word_list));
    gtk_tree_model_get (model, &iter, 0, &word, -1);
    speller->current_word = g_strdup (word);

}    

static void
row_activated_cb (GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column,
		  gpointer data) {
    GDictSpeller *speller;
    
    g_return_if_fail (data != NULL);
    speller = GDICT_SPELLER (data);
    
    speller_lookup (speller);
    
}

static void
spell_entry_cb (GtkEntry *entry, gpointer data) {
        
    g_return_if_fail (data != NULL);
    
    spell_button_pressed (NULL, data);
    
}

/* speller_lookup_cb
 *
 * Looks up the currently selected word
 */

static void
speller_lookup (GDictSpeller *speller) {
    
    if (!speller->current_word) return;
    
    gtk_entry_set_text (GTK_ENTRY (word_entry), speller->current_word);
    gtk_editable_select_region (GTK_EDITABLE (word_entry), 0,
                                strlen (speller->current_word));
    gdict_app_do_lookup (speller->current_word);
}

