/* spdictui.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "spdictui.h"
#include "SRMessages.h"
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"
#include "spconf.h"


enum
{
    SPEECH_DICTIONARY_WORD,
    SPEECH_DICTIONARY_REPLACER,
    
    SPEECH_DICTIONARY_NO_COLUMN
};

struct {
    GtkWidget	*w_dictionary;
    GtkWidget	*w_dict_add_modify;
    GtkWidget	*et_word;
    GtkWidget	*et_replace;
    GtkWidget	*tv_dictionary;
    GtkWidget	*bt_dict_modify;
    GtkWidget	*bt_dict_remove;
} spui_speech_dictionary;

static gboolean				new_item;
static GtkTreeIter			curr_iter;		
extern DictionaryListType		*dictionary_list;

#define STR(X) (X == NULL? "" : X)

static gint
spui_delete_emit_response_cancel (GtkDialog *dialog,
				  GdkEventAny *event,
				  gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}



static gboolean
spui_dictionary_add_modify_set_value (void)
{
    GtkTreeModel     *model;
    gchar *word    = (gchar*) gtk_entry_get_text (GTK_ENTRY (spui_speech_dictionary.et_word));
    gchar *replace = (gchar*) gtk_entry_get_text (GTK_ENTRY (spui_speech_dictionary.et_replace));

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
    
    if (!word    || strlen (word)    == 0 ||
        !replace || strlen (replace) == 0)
    {
	gn_show_message (_("Invalid word or replacer!"));
	return FALSE;
    }

    if (spconf_dictionary_find_word (dictionary_list, word))
    {
	gn_show_message (_("Word exists in dictionary!"));
	return FALSE;
    }

    if (!GTK_IS_LIST_STORE (model))
	return FALSE;
	

    if (new_item)
    {
    	dictionary_list =	
    	    spconf_dictionary_add_word (dictionary_list,
					word, replace);
    }
    else
    {
	dictionary_list =	
	    spconf_dictionary_modify_word (dictionary_list,
			    		    word, replace);
    }			
    
    gtk_list_store_set (GTK_LIST_STORE (model), 	&curr_iter, 
    			SPEECH_DICTIONARY_WORD,	 	(gchar*)word,
			SPEECH_DICTIONARY_REPLACER,  	(gchar*)replace,
		    	-1);
    return TRUE;
}

static void
spui_dictionary_add_modify_restore_state (void)
{
    GtkTreeModel     *model;
    
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
    
    if (!GTK_IS_LIST_STORE (model))
	return;

    if (new_item)
	gtk_list_store_remove (GTK_LIST_STORE (model), &curr_iter);
}

static void
spui_dictionary_add_modify_response (GtkDialog *dialog,
				    gint       response_id,
				    gpointer   user_data)
{
    switch (response_id)
    { 
	case GTK_RESPONSE_OK:
	    if (!spui_dictionary_add_modify_set_value ())
		spui_dictionary_add_modify_restore_state ();
		
	break;
	case GTK_RESPONSE_CANCEL:
	    spui_dictionary_add_modify_restore_state ();
	break;
	case GTK_RESPONSE_HELP:
    	    gn_load_help ("gnopernicus-speech-prefs");
	    return;
	break;
	default:
	break;
    }
    gtk_widget_hide ((GtkWidget*)dialog);
}

static void
spui_dictionary_add_modify_value_add_to_widgets (void)
{
    GtkTreeModel     *model;
    gchar *word = NULL;
    gchar *replacer = NULL;
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
    
    if (!GTK_IS_LIST_STORE (model))
	return;
    
    gtk_tree_model_get (model, 	&curr_iter,
			SPEECH_DICTIONARY_WORD, &word,
			SPEECH_DICTIONARY_REPLACER, &replacer,
                    	-1);

    gtk_entry_set_text (GTK_ENTRY (spui_speech_dictionary.et_word), STR(word));
    gtk_entry_set_text (GTK_ENTRY (spui_speech_dictionary.et_replace), STR(replacer));

    g_free (word);
    g_free (replacer);
}


static void
spui_dictionary_add_modify_handlers_set (GladeXML *xml)
{
    spui_speech_dictionary.w_dict_add_modify = glade_xml_get_widget (xml, "w_dictionary_add_modify");
    spui_speech_dictionary.et_word = glade_xml_get_widget (xml, "et_word"); 
    spui_speech_dictionary.et_replace = glade_xml_get_widget (xml, "et_replace"); 
        
    g_signal_connect (spui_speech_dictionary.w_dict_add_modify, "response",
		      G_CALLBACK (spui_dictionary_add_modify_response), NULL);
    g_signal_connect (spui_speech_dictionary.w_dict_add_modify, "delete_event",
                      G_CALLBACK (spui_delete_emit_response_cancel), NULL);

}

static gboolean 
spui_dictionary_add_modify_load (GtkWidget *parent_window)
{
    if (!spui_speech_dictionary.w_dict_add_modify)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Speech_Settings/speech_settings.glade2", "w_dictionary_add_modify");
	sru_return_val_if_fail (xml, FALSE);
	spui_dictionary_add_modify_handlers_set (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (spui_speech_dictionary.w_dict_add_modify),
				           GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (spui_speech_dictionary.w_dict_add_modify), 
					         TRUE);
    }
    else
	gtk_widget_show (spui_speech_dictionary.w_dict_add_modify);
    
    spui_dictionary_add_modify_value_add_to_widgets ();
    
    return TRUE;
}


static void
spui_dictionary_response (GtkDialog *dialog,
			  gint       response_id,
			  gpointer   user_data)
{
    switch (response_id)
    {
	case GTK_RESPONSE_OK:
    	    spconf_dictionary_save (dictionary_list);
	break;
	case GTK_RESPONSE_CANCEL:
	    dictionary_list =	
		spconf_dictionary_free (dictionary_list);
	break;
	case GTK_RESPONSE_HELP:
	    gn_load_help ("gnopernicus-speech-prefs");
	    return;
	break;
	default:
	break;
    }
    
    gtk_widget_hide ((GtkWidget*)dialog);    
}

static void
spui_dictionary_button_add_clicked (GtkWidget *widget,
		    	    	    gpointer data)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;    
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
    model     = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));

    if (!GTK_IS_LIST_STORE (model))
	return;
        
    new_item  = TRUE;
    
    gtk_list_store_append (GTK_LIST_STORE (model), &curr_iter);
    gtk_tree_selection_select_iter (selection, &curr_iter);
    
    spui_dictionary_add_modify_load (spui_speech_dictionary.w_dictionary);
}

static void
spui_dictionary_button_modify_clicked (GtkWidget *widget,
				       gpointer data)
{
    GtkTreeIter	     iter;
    GtkTreeSelection *selection;    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
    
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
    	gn_show_message (_("No selected word to modify!"));
	return;
    }	
    
    curr_iter = iter;
    new_item  = FALSE;
    
    spui_dictionary_add_modify_load (spui_speech_dictionary.w_dictionary);
}

static void
spui_dictionary_button_remove_clicked (GtkWidget *widget,
				       gpointer data)
{
    GtkTreeModel     *model = NULL;
    GtkTreeIter	     iter;
    GtkTreeSelection *selection = NULL;    
    gchar 	     *word = NULL;
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
    model     = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
    
    if (!GTK_IS_LIST_STORE (model))
	return;
    
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
    	gn_show_message (_("No selected word to remove!"));
	return;
    }
	
    gtk_tree_model_get (model, &iter,
			SPEECH_DICTIONARY_WORD, &word,
                    	-1);
    dictionary_list =	
	    spconf_dictionary_remove_word (dictionary_list, word);

    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

    g_free (word);
}

static void
spui_dictionary_row_activated_cb (GtkTreeView       *tree_view,
            	    		  GtkTreePath       *path,
		    		  GtkTreeViewColumn *column)
{
    GtkTreeIter	     iter;
    GtkTreeSelection *selection;    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
    	gn_show_message (_("No selected word to modify!"));
	return;
    }	
    curr_iter = iter;
    new_item  = FALSE;
    
    spui_dictionary_add_modify_load (spui_speech_dictionary.w_dictionary);
}

static void
spui_dictionary_selection_changed (GtkTreeSelection *selection,
				   gpointer  user_data)
{
    gboolean 	     sensitive;
            
    sensitive = gtk_tree_selection_get_selected (selection, NULL, NULL);
    
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_dictionary.bt_dict_modify), sensitive);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_dictionary.bt_dict_remove), sensitive);
}

static gboolean
spui_dictionary_search_equal_func (GtkTreeModel  *model,
				    gint 		column,
				    const gchar	*key,
				    GtkTreeIter	*iter,
				    gpointer	search_data)
{
    gchar *list_key = NULL;
    
    gtk_tree_model_get (model, 	iter,
    			column, &list_key,
                    	-1);

    return g_strcasecmp (key, list_key) > 0; 
}


static void
spui_dictionary_fill_table (GtkListStore *store)
{
    DictionaryListType* elem = NULL;
    GtkTreeIter  iter;
    
    for (elem = dictionary_list; elem ; elem = elem->next)
    {
	gchar *word = NULL;
	gchar *replace = NULL;
	spconf_dictionary_split_entry ((gchar*)elem->data, &word, &replace);

	gtk_list_store_append (GTK_LIST_STORE (store), &iter);
	gtk_list_store_set (GTK_LIST_STORE (store), &iter, 
			    SPEECH_DICTIONARY_WORD,	(gchar*)word,
			    SPEECH_DICTIONARY_REPLACER,  (gchar*)replace,
		    	    -1);
	g_free (word);
	g_free (replace);
    }
}

static GtkTreeModel*
spui_dictionary_create_model (void)
{
    GtkListStore *store;
    store = gtk_list_store_new (SPEECH_DICTIONARY_NO_COLUMN, 
				G_TYPE_STRING,
				G_TYPE_STRING);

    spui_dictionary_fill_table (store);
    return GTK_TREE_MODEL (store) ;
}


static void
spui_dictionary_handlers_set (GladeXML *xml)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;

    spui_speech_dictionary.w_dictionary = glade_xml_get_widget (xml, "w_dictionary");
    spui_speech_dictionary.tv_dictionary = glade_xml_get_widget (xml, "tv_dictionary");
    spui_speech_dictionary.bt_dict_modify = glade_xml_get_widget (xml, "bt_dict_modify");
    spui_speech_dictionary.bt_dict_remove = glade_xml_get_widget (xml, "bt_dict_remove");
    
    g_signal_connect (spui_speech_dictionary.w_dictionary, "response",
		      G_CALLBACK (spui_dictionary_response), NULL);
    g_signal_connect (spui_speech_dictionary.w_dictionary, "delete_event",
                      G_CALLBACK (spui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml,"on_bt_dict_add_clicked",			
			     GTK_SIGNAL_FUNC (spui_dictionary_button_add_clicked));

    glade_xml_signal_connect (xml,"on_bt_dict_modify_clicked",			
			     GTK_SIGNAL_FUNC (spui_dictionary_button_modify_clicked));

    glade_xml_signal_connect (xml,"on_bt_dict_remove_clicked",			
			     GTK_SIGNAL_FUNC (spui_dictionary_button_remove_clicked));

    glade_xml_signal_connect (xml,"on_et_word_selected_activate",			
			     GTK_SIGNAL_FUNC (spui_dictionary_button_modify_clicked));


    model = spui_dictionary_create_model ();
                
    gtk_tree_view_set_model (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary), model);
    
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));

    g_signal_connect (selection, "changed",
                      G_CALLBACK (spui_dictionary_selection_changed), NULL);

    
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE (model), 
					    SPEECH_DICTIONARY_WORD, 
					    GTK_SORT_ASCENDING);

    g_signal_connect (spui_speech_dictionary.tv_dictionary, "row_activated", 
		      G_CALLBACK (spui_dictionary_row_activated_cb), model);					        

    cell_renderer = gtk_cell_renderer_text_new ();
    
    column = gtk_tree_view_column_new_with_attributes   (_("Word"),
    							cell_renderer,
							"text", SPEECH_DICTIONARY_WORD,
							NULL);	
    gtk_tree_view_column_set_sort_column_id (column, SPEECH_DICTIONARY_WORD);
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary), column);
    
    column = gtk_tree_view_column_new_with_attributes   (_("Replacer"),
    							cell_renderer,
							"text", SPEECH_DICTIONARY_REPLACER,
							NULL);	
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary), column);

    g_object_unref (G_OBJECT (model));
    
    gtk_tree_view_set_search_column 
	(GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary), 
	SPEECH_DICTIONARY_WORD);    
    gtk_tree_view_set_search_equal_func 
	(GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary),
	spui_dictionary_search_equal_func,
	NULL, NULL);
    gtk_tree_view_set_enable_search 
	(GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary), 
	TRUE);

    
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);    
    
    spui_dictionary_selection_changed (selection, NULL);
}

gboolean 
spui_dictionary_load (GtkWidget *parent_window)
{
    if (!dictionary_list)
    {
	dictionary_list =
	    spconf_dictionary_load ();
    }
    
    if (!spui_speech_dictionary.w_dictionary)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Speech_Settings/speech_settings.glade2", "w_dictionary");
	sru_return_val_if_fail (xml, FALSE);
	spui_dictionary_handlers_set (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (spui_speech_dictionary.w_dictionary),
				      GTK_WINDOW (parent_window));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (spui_speech_dictionary.w_dictionary), 
					    TRUE);
    }
    else
    {
	GtkTreeModel *model;
	model = 
	    gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_dictionary.tv_dictionary));
	gtk_list_store_clear (GTK_LIST_STORE (model));
	spui_dictionary_fill_table (GTK_LIST_STORE (model));
	gtk_widget_show (spui_speech_dictionary.w_dictionary);
    }
    
    return TRUE;
}
