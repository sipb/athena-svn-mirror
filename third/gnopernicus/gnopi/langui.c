/* langui.c
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
#include "langui.h"
#include "coreconf.h"
#include "SRMessages.h"
#include <libgnomeui/libgnomeui.h>
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"


enum {
    LANGUAGE_COUNTRY,
    LANGUAGE_ID,
    LANGUAGE_ACTIVE,
    LANGUAGE_SUPPORTED,
    LANGUAGE_ACTIVATABLE,
    LANGUAGE_NO_COLUMN
};

LanguageType languages[] =
{
    {"aa",	N_("Afar"),	FALSE},
    {"ab",	N_("Abkhazian"),FALSE},
    {"ae",	N_("Avestan"),	FALSE},
    {"af",	N_("Afrikaans"),FALSE},
    {"am",	N_("Amharic"),	FALSE},
    {"ar",	N_("Arabic"),	FALSE},
    {"as",	N_("Assamese"),	FALSE},
    {"ay",	N_("Aymara"),	FALSE},
    {"az",	N_("Azerbaijani"),FALSE},
    {"ba",	N_("Bashkir"),	FALSE},
    {"be",	N_("Byelorussian; Belarusian"),FALSE},
    {"bg",	N_("Bulgarian"),FALSE},
    {"bh",	N_("Bihari"),	FALSE},
    {"bi",	N_("Bislama"),	FALSE},
    {"bn",	N_("Bengali; Bangla"),FALSE},
    {"bo",	N_("Tibetan"),	FALSE},
    {"br",	N_("Breton"),	FALSE},
    {"bs",	N_("Bosnian"),	FALSE},
    {"ca",	N_("Catalan"),	FALSE},
    {"ce",	N_("Chechen"),	FALSE},
    {"ch",	N_("Chamorro"),	FALSE},
    {"co",	N_("Corsican"),	FALSE},
    {"cs",	N_("Czech"),	FALSE},
    {"cu",	N_("Church Slavic"),FALSE},
    {"cv",	N_("Chuvash"),	FALSE},
    {"cy",	N_("Welsh"),	FALSE},
    {"da",	N_("Danish"),	FALSE},
    {"de",	N_("German"),	FALSE},
    {"dz",	N_("Dzongkha; Bhutani"),FALSE},
    {"el",	N_("Greek"),	FALSE},
    {"en",	N_("English"),	TRUE},
    {"eo",	N_("Esperanto"),FALSE},
    {"es",	N_("Spanish"),	FALSE},
    {"et",	N_("Estonian"),	FALSE},
    {"eu",	N_("Basque"),	FALSE},
    {"fa",	N_("Persian"),	FALSE},
    {"fi",	N_("Finnish"),	FALSE},
    {"fj",	N_("Fijian; Fiji"),FALSE},
    {"fo",	N_("Faroese"),	FALSE},
    {"fr",	N_("French"),	FALSE},
    {"fy",	N_("Frisian"),	FALSE},
    {"ga",	N_("Irish"),	FALSE},
    {"gd",	N_("Scots; Gaelic"),FALSE},
    {"gl",	N_("Gallegan; Galician"),FALSE},
    {"gn",	N_("Guarani"),	FALSE},
    {"gu",	N_("Gujarati"),	FALSE},
    {"gv",	N_("Manx"),	FALSE},
    {"ha",	N_("Hausa (?)"),FALSE},
    {"he",	N_("Hebrew (formerly iw)"),FALSE},
    {"hi",	N_("Hindi"),	FALSE},
    {"ho",	N_("Hiri Motu"),FALSE},
    {"hr",	N_("Croatian"),	FALSE},
    {"hu",	N_("Hungarian"),FALSE},
    {"hy",	N_("Armenian"),	FALSE},
    {"hz",	N_("Herero"),	FALSE},
    {"ia",	N_("Interlingua"),FALSE},
    {"id",	N_("Indonesian (formerly in)"),	FALSE},
    {"ie",	N_("Interlingue"),FALSE},
    {"ik",	N_("Inupiak"),	FALSE},
    {"io",	N_("Ido"),	FALSE},
    {"is",	N_("Icelandic"),FALSE},
    {"it",	N_("Italian"),	FALSE},
    {"iu",	N_("Inuktitut"),FALSE},
    {"ja",	N_("Japanese"),	FALSE},
    {"jv",	N_("Javanese"),	FALSE},
    {"ka",	N_("Georgian"),	FALSE},
    {"ki",	N_("Kikuyu"),	FALSE},
    {"kj",	N_("Kuanyama"),	FALSE},
    {"kk",	N_("Kazakh"),	FALSE},
    {"kl",	N_("Kalaallisut; Greenlandic"),	FALSE},
    {"km",	N_("Khmer; Cambodian"),	FALSE},
    {"kn",	N_("Kannada"),	FALSE},
    {"ko",	N_("Korean"),	FALSE},
    {"ks",	N_("Kashmiri"),	FALSE},
    {"ku",	N_("Kurdish"),	FALSE},
    {"kv",	N_("Komi"),	FALSE},
    {"kw",	N_("Cornish"),	FALSE},
    {"ky",	N_("Kirghiz"),	FALSE},
    {"la",	N_("Latin"),	FALSE},
    {"lb",	N_("Letzeburgesch"),FALSE},
    {"ln",	N_("Lingala"),	FALSE},
    {"lo",	N_("Lao; Laotian"),FALSE},
    {"lt",	N_("Lithuanian"),FALSE},
    {"lv",	N_("Latvian; Lettish"),FALSE},
    {"mg",	N_("Malagasy"),	FALSE},
    {"mh",	N_("Marshall"),	FALSE},
    {"mi",	N_("Maori"),	FALSE},
    {"mk",	N_("Macedonian"),FALSE},
    {"ml",	N_("Malayalam"),FALSE},
    {"mn",	N_("Mongolian"),FALSE},
    {"mo",	N_("Moldavian"),FALSE},
    {"mr",	N_("Marathi"),	FALSE},
    {"ms",	N_("Malay"),	FALSE},
    {"mt",	N_("Maltese"),	FALSE},
    {"my",	N_("Burmese"),	FALSE},
    {"na",	N_("Nauru"),	FALSE},
    {"nb",	N_("Norwegian Bokmaal"),FALSE},
    {"nd",	N_("Ndebele, North"),FALSE},
    {"ne",	N_("Nepali"),	FALSE},
    {"ng",	N_("Ndonga"),	FALSE},
    {"nl",	N_("Dutch"),	FALSE},
    {"nn",	N_("Norwegian Nynorsk"),FALSE},
    {"no",	N_("Norwegian"),FALSE},
    {"nr",	N_("Ndebele, South"),FALSE},
    {"nv",	N_("Navajo"),	FALSE},
    {"ny",	N_("Chichewa; Nyanja"),	FALSE},
    {"oc",	N_("Occitan; Provenc,al"),FALSE},
    {"om",	N_("(Afan) Oromo"),FALSE},
    {"or",	N_("Oriya"),	FALSE},
    {"os",	N_("Ossetian; Ossetic"),FALSE},
    {"pa",	N_("Panjabi; Punjabi"),FALSE},
    {"pi",	N_("Pali"),	FALSE},
    {"pl",	N_("Polish"),	FALSE},
    {"ps",	N_("Pashto, Pushto"),FALSE},
    {"pt",	N_("Portuguese"),FALSE},
    {"qu",	N_("Quechua"),	FALSE},
    {"rm",	N_("Rhaeto-Romance"),FALSE},
    {"rn",	N_("Rundi; Kirundi"),FALSE},
    {"ro",	N_("Romanian"),	TRUE},
    {"ru",	N_("Russian"),	FALSE},
    {"rw",	N_("Kinyarwanda"),FALSE},
    {"sa",	N_("Sanskrit"),	FALSE},
    {"sc",	N_("Sardinian"),FALSE},
    {"sd",	N_("Sindhi"),	FALSE},
    {"se",	N_("Northern Sami"),FALSE},
    {"sg",	N_("Sango; Sangro"),FALSE},
    {"si",	N_("Sinhalese"),FALSE},
    {"sk",	N_("Slovak"),	FALSE},
    {"sl",	N_("Slovenian"),FALSE},
    {"sm",	N_("Samoan"),	FALSE},
    {"sn",	N_("Shona"),	FALSE},
    {"so",	N_("Somali"),	FALSE},
    {"sq",	N_("Albanian"),	FALSE},
    {"sr",	N_("Serbian"),	FALSE},
    {"ss",	N_("Swati; Siswati"),FALSE},
    {"st",	N_("Sesotho; Sotho, Southern"),FALSE},
    {"su",	N_("Sundanese"),FALSE},
    {"sv",	N_("Swedish"),	FALSE},
    {"sw",	N_("Swahili"),	FALSE},
    {"ta",	N_("Tamil"),	FALSE},
    {"te",	N_("Telugu"),	FALSE},
    {"tg",	N_("Tajik"),	FALSE},
    {"th",	N_("Thai"),	FALSE},
    {"ti",	N_("Tigrinya"),	FALSE},
    {"tk",	N_("Turkmen"),	FALSE},
    {"tl",	N_("Tagalog"),	FALSE},
    {"tn",	N_("Tswana; Setswana"),	FALSE},
    {"to",	N_("Tonga (?)"),FALSE},
    {"tr",	N_("Turkish"),	FALSE},
    {"ts",	N_("Tsonga"),	FALSE},
    {"tt",	N_("Tatar"),	FALSE},
    {"tw",	N_("Twi"),	FALSE},
    {"ty",	N_("Tahitian"),	FALSE},
    {"ug",	N_("Uighur"),	FALSE},
    {"uk",	N_("Ukrainian"),FALSE},
    {"ur",	N_("Urdu"),	FALSE},
    {"uz",	N_("Uzbek"),	FALSE},
    {"vi",	N_("Vietnamese"),FALSE},
    {"vo",	N_("Volapuk; Volapuk"),	FALSE},
    {"wa",	N_("Walloon"),	FALSE},
    {"wo",	N_("Wolof"),	FALSE},
    {"xh",	N_("Xhosa"),	FALSE},
    {"yi",	N_("Yiddish (formerly ji)"),FALSE},
    {"yo",	N_("Yoruba"),	FALSE},
    {"za",	N_("Zhuang"),	FALSE},
    {"zh",	N_("Chinese"),	FALSE},
    {"zu",	N_("Zulu"),	FALSE},
};

static GtkWidget *dl_language = NULL;
static GtkWidget *tv_language = NULL;
static GtkWidget *ck_sensitive_all;

gint
lng_set_envirom (gchar *id)
{
    gint rv = 0;
/*    extern int _nl_msg_cat_cntr;
    gchar *env;
    
    env = g_strdup_printf ("LANGUAGE=%s",id);
    putenv   (env); 
    _nl_msg_cat_cntr ++;
    g_free (env);*/
    return rv;
}

static  void
lng_get_selected_language ()
{
    GtkTreeIter 	iter;
    GtkTreeModel 	*model; 	
    gboolean		valid;
    
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv_language));
        
    valid = gtk_tree_model_get_iter_first (model, &iter);
    
    while (valid)
    {
	if (GTK_IS_LIST_STORE (model))
	{
	    gboolean active = FALSE;
	    gchar *curr_id  = NULL;
	    gchar *id  	    = NULL;
	    
	    gtk_tree_model_get (model, 		&iter, 
				LANGUAGE_ID,	&id,
				LANGUAGE_ACTIVE,&active,
				-1);
	    curr_id = getenv ("LANGUAGE");
	    if (active && curr_id)
	    {
		if (strcmp (curr_id, id))
		    srcore_language_set (id);
	    }
	}	    
	valid = gtk_tree_model_iter_next (model, &iter);	    
    }
}

static void
lng_lang_cancel_clicked (GtkWidget *widget,
			 gpointer user_data)
{
    gtk_widget_hide (dl_language);
}

static void
lng_lang_ok_clicked (GtkWidget *widget,
		     gpointer user_data)
{
    lng_get_selected_language ();
    gtk_widget_hide (dl_language);
}

static void
lng_lang_remove (GtkWidget *widget,
		gpointer user_data)
{
    gtk_widget_hide (dl_language);
    dl_language = NULL;
}

static void
lng_selectable_all_toogled (GtkWidget *widget,
			    gpointer user_data)
{
    GtkTreeModel     *model;
    GtkTreeIter      iter;
    gboolean 	     valid;
    
    gboolean ck_active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tv_language));
    
    valid = gtk_tree_model_get_iter_first ( model, &iter );
    
    while (valid)
    {
	if (GTK_IS_LIST_STORE (model))
	{
	    gboolean supported;
	    gboolean act;
	    gtk_tree_model_get (model, 			&iter,
				LANGUAGE_SUPPORTED,	&supported,
				LANGUAGE_ACTIVE,	&act,
            			-1);

	    if (!supported && act && !ck_active) 
	    {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_sensitive_all),
						    TRUE);
		break;
	    }
	    if (!supported)
	    {
		gtk_list_store_set ( GTK_LIST_STORE (model), &iter,
    				    LANGUAGE_ACTIVATABLE,    ck_active,
                    		    -1);
	    }
	}
	valid = gtk_tree_model_iter_next (model, &iter);	    
    }
    
}

static GtkTreeModel*
lng_create_model (void)
{
    GtkListStore *store;
    GtkTreeIter iter;
    gint i;
      
    store = gtk_list_store_new (LANGUAGE_NO_COLUMN, 
				G_TYPE_STRING,
				G_TYPE_STRING,
				G_TYPE_BOOLEAN,
				G_TYPE_BOOLEAN,
				G_TYPE_BOOLEAN);
    
    for (i = 0 ; i < G_N_ELEMENTS (languages) ; i++)
    {
	gtk_list_store_append ( GTK_LIST_STORE (store), &iter);
	gtk_list_store_set ( GTK_LIST_STORE (store), &iter, 
			    LANGUAGE_COUNTRY,	_((gchar*)languages[i].country),
			    LANGUAGE_ID,	_((gchar*)languages[i].id),
			    LANGUAGE_ACTIVE, 	FALSE,
			    LANGUAGE_SUPPORTED, languages[i].supported,
			    LANGUAGE_ACTIVATABLE,languages[i].supported,
		    	    -1);
    }    
    return GTK_TREE_MODEL (store) ;
}

void 
lng_selected_language_show ()
{
    GtkTreeModel     *model;
    GtkTreeIter      iter;
    gboolean 	     valid;
    gchar 	     *current_lang = NULL;
    
    if (!dl_language) return;
    
    current_lang = srcore_language_get ();
    
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tv_language));
    
    valid = gtk_tree_model_get_iter_first (model, &iter);
    
    while (valid)
    {
	gchar *id = NULL;
	gboolean supported;
	
	if (GTK_IS_LIST_STORE (model))
	{
	    gtk_tree_model_get (model, 		&iter,
				LANGUAGE_ID,	&id,
				LANGUAGE_SUPPORTED,	&supported,
            			-1);
	}

	if (id && current_lang)
	{
	    if ( !strcmp (current_lang, id))
	    {		
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				    LANGUAGE_ACTIVE, TRUE,
				    -1);
		if (!supported)
		{
		    	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_sensitive_all),
						      TRUE);
		}
	    }
	    else
	    {
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				    LANGUAGE_ACTIVE, FALSE,
				    -1);
	    }
	}
	
	valid = gtk_tree_model_iter_next (model, &iter);
	    
	g_free (id);
    }
    
    g_free (current_lang);
    current_lang = NULL;
}

static void
lng_cell_toggled (GtkCellRendererToggle *cell,
		  gchar 		*str_path,
		  gpointer 		data)
{
    GtkTreeModel *model = (GtkTreeModel*)data;
    GtkTreePath *path = gtk_tree_path_new_from_string (str_path);
    GtkTreeIter iter;
    GtkTreeIter iter_current;
    gint *column;
    gchar *id, *id_current;
    
    gboolean toggle_item;
    gboolean valid;
    
    column = g_object_get_data (G_OBJECT (cell), "column");
    
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, 
			column, &toggle_item, 
			LANGUAGE_ID,	&id,
			-1);
			
    toggle_item = TRUE;
    
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			column, toggle_item,
			-1);
			
    gtk_tree_path_free (path);


    valid = gtk_tree_model_get_iter_first (model, &iter_current);
    
    while (valid)
    {
	if (GTK_IS_LIST_STORE (model))
	{
	    gboolean supported = FALSE;
	    gtk_tree_model_get (model, 		&iter_current, 
				LANGUAGE_ID,	&id_current,
				-1);

	    if (strcmp (id, id_current))
	    {
	        gtk_list_store_set (GTK_LIST_STORE (model), &iter_current,
				    column, 		    supported,
				    -1);
	    }
	}	    
	valid = gtk_tree_model_iter_next (model, &iter_current);	    
    }

}


static void 
lng_set_handler (GladeXML *xml)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;

    glade_xml_signal_connect (xml, "on_bt_lang_cancel_clicked",
			GTK_SIGNAL_FUNC (lng_lang_cancel_clicked));
    glade_xml_signal_connect (xml, "on_bt_lang_ok_clicked",
			GTK_SIGNAL_FUNC (lng_lang_ok_clicked));
    glade_xml_signal_connect (xml, "on_dl_language_remove",
			GTK_SIGNAL_FUNC (lng_lang_remove));
    glade_xml_signal_connect (xml, "on_ck_selectable_all_toggled",
			GTK_SIGNAL_FUNC (lng_selectable_all_toogled));

    tv_language = glade_xml_get_widget (xml, "tv_language");
    
    ck_sensitive_all = glade_xml_get_widget (xml, "ck_selectable_all");
    
    model = lng_create_model ();
                
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_language), model);

    
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE (model), 
					    LANGUAGE_COUNTRY, 
					    GTK_SORT_ASCENDING);
					    
    cell_renderer = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes   (_(" "),
    							cell_renderer,
							"active", 	LANGUAGE_ACTIVE,
							"activatable",  LANGUAGE_ACTIVATABLE,
							NULL);	
							
    g_signal_connect (cell_renderer, "toggled", G_CALLBACK (lng_cell_toggled), model);
    g_object_set_data (G_OBJECT (cell_renderer),"column", (gint *)LANGUAGE_ACTIVE);
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_language), column);

    
    cell_renderer = gtk_cell_renderer_text_new ();
    
    column = gtk_tree_view_column_new_with_attributes   (_("Country"),
    							cell_renderer,
							"text", LANGUAGE_COUNTRY,
							NULL);	
    gtk_tree_view_column_set_sort_column_id (column, LANGUAGE_COUNTRY);
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_language), column);
    
    column = gtk_tree_view_column_new_with_attributes   (_("ID"),
    							cell_renderer,
							"text", LANGUAGE_ID,
							NULL);	
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_language), column);
    
    g_object_unref (G_OBJECT (model));
    
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_language));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);    
}

void
lng_load_language (GtkWidget *parent_window)
{
    if (!dl_language)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Language/language.glade2", "dl_language");
	if (xml)
	{
	    dl_language = glade_xml_get_widget (xml, "dl_language");
	    lng_set_handler (xml);	
	    g_object_unref (G_OBJECT (xml));
	    gtk_window_set_transient_for (  GTK_WINDOW (dl_language),
					    GTK_WINDOW (parent_window));
	    gtk_window_set_destroy_with_parent ( GTK_WINDOW (dl_language), 
					        TRUE);	
	}
	else
	    return;
    }
    else
	gtk_widget_show (dl_language);

    lng_selected_language_show	();
}
