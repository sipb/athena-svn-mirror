/* brlui.c
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
#include "brlui.h"
#include "gnopiui.h"
#include "SRMessages.h"
#include "libsrconf.h"
#include <glade/glade.h>
#include "cmdmapconf.h"
#include "srintl.h"

#define NO_ITEM_IN_CURSOR 		3
#define NO_ITEM_IN_ATTRIBUTE 		4
#define NO_ITEM_IN_OPTICAL_SENSORS 	13
#define NO_ITEM_IN_POSITION_SENSORS	13
#define NO_ITEM_IN_STATUS_CELL 		2


static struct {
    KeyItem 		*current_keyitem;
    GtkTreeIter 	current_iter;
    gboolean		current_is_new;
} brl_current_item;

enum
{
    DOT_OFF = 0,
    DOT_6   = 6,
    DOT_7   = 7,
    DOT_8   = 8,
    DOT_78  = 78
};

const gchar *cursor_style[] =
{
    "block",
    "underline",
    "user"
};

const gchar *braille_style[] =
{
    "8",
    "6"
};

typedef enum
{
    TABLE_COLUMN,
    NO_OF_TABLE_COLUMNS
} TransTableListStruct;


const struct
{
    gchar *state;
    gchar *file_name;
} trans_table_list [] = 
{
    {N_("American English"),	"en_US"},
    {N_("German"),		"de"},
    {N_("Spanish"),		"es"},
    {N_("Swedish"),		"sv"},
    {NULL,		NULL}
};

const gint attribute_value [] = 
{
    7,8,78,0
};

const gchar *status_cell_value [] = 
{
    "CursorPos",
    "FontStyle"
};

#define BRLTTY "BRLTTY's BrlAPI"
/**
 * Braille Settings widgets 					
**/
/* Window  */
GtkWidget		*w_braille_settings = NULL;


/**
 * Braille device widgets 					
**/
GtkWidget		*w_braille_device;
static GtkWidget 	*cb_braille_device;
static GtkWidget 	*sp_port;
static GtkWidget	*lb_port;

/**
 * Translation Table widgets 					
**/
static GtkWidget	*w_translation_table;
static GtkWidget	*tv_translation_table;


/**
 * Braille Style 						
**/
static GtkWidget	*w_braille_style;
static GtkWidget 	*rb_braille_style[2];


/**
 * Cursor Settings 						
**/
static GtkWidget	*w_cursor_settings;
static GtkWidget 	*rb_cursor_settings [ NO_ITEM_IN_CURSOR ];


/**
 * Attribute Settings 						
**/
static GtkWidget	*w_attribute_settings;
static GtkWidget 	*rb_attribute_settings [ NO_ITEM_IN_ATTRIBUTE ];	


/**
 * Braille Fill Char 						
**/
static GtkWidget	*w_braille_fill_char;
static GtkWidget	*et_fill_char;


/**
 * Status Cell 							
**/
static GtkWidget	*w_status_cell;
static GtkWidget 	*rb_status_cell [ NO_ITEM_IN_STATUS_CELL ];
		

/**
 * Braille Mapping					
**/
static GtkWidget	*w_braille_key_mapping;
static GtkWidget 	*w_braille_mapping_add_modify;
static GtkWidget 	*rb_position_switches [ NO_ITEM_IN_POSITION_SENSORS ];
static GtkWidget 	*rb_optical_sensor [ NO_ITEM_IN_OPTICAL_SENSORS ];
static GtkWidget	*tv_braille_keys;
static GtkWidget	*tv_cmd_list;
static GtkWidget	*cb_braille_key;
static GtkWidget	*bt_braille_key_remove;
static GtkWidget	*bt_braille_key_modify;

extern GList 	*brldev_id_list;
extern GList	*brldev_description_list;	
       GList 	*braille_key_combination_list = NULL;
       GSList 	*braille_keys_list;
       GSList 	*removed_items_list;
extern Braille 	*braille_setting;
extern CmdMapFunctionsList cmd_function[]; 


static void 
brlui_braille_device_clicked 	(GtkButton       *button,
                            	gpointer         user_data)
{
    brlui_load_braille_device ((Braille*) user_data);
}

static void 
brlui_translation_table_clicked	(GtkButton       *button,
                            	gpointer         user_data)
{
    brlui_load_translation_table ((Braille*) user_data);
}

static void 
brlui_braille_style_clicked (GtkButton       *button,
                             gpointer         user_data)
{
    brlui_load_braille_style ((Braille*) user_data);
}

static void 
brlui_cursor_settings_clicked (GtkButton       *button,
                               gpointer         user_data)
{
    brlui_load_cursor_settings ((Braille*) user_data);
}

static void	
brlui_attribute_settings_clicked (GtkButton       *button,
                            	  gpointer         user_data)
{
    brlui_load_attribute_settings ((Braille*) user_data);
}

static void	
brlui_braille_fill_char_clicked (GtkButton       *button,
                            	 gpointer         user_data)
{
    brlui_load_braille_fill_char ((Braille*) user_data);
}

static void	
brlui_status_cells_clicked (GtkButton       *button,
                            gpointer         user_data)
{
    brlui_load_status_cell ((Braille*) user_data);
}

static void	
brlui_braille_mapping_clicked (GtkButton       *button,
                               gpointer         user_data)
{
    brlui_load_braille_key_mapping ((Braille*) user_data);
}

static void 
brlui_braille_settings_close_clicked (GtkButton       *button,
                                      gpointer         user_data)
{
    gtk_widget_hide (w_braille_settings);
}

static void 
brlui_braille_settings_remove (GtkButton       *button,
                               gpointer         user_data)
{
    gtk_widget_hide (w_braille_settings);
    w_braille_settings = NULL;
}

void 
brlui_set_handlers_braille_settings (GladeXML 	*xml,
				     Braille 	*braille_setting)
{
    w_braille_settings = glade_xml_get_widget (xml, "w_braille_settings");

    glade_xml_signal_connect_data (xml,"on_w_braille_settings_remove",	
				GTK_SIGNAL_FUNC (brlui_braille_settings_remove), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_braille_device_clicked",	
				GTK_SIGNAL_FUNC (brlui_braille_device_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_translation_table_clicked", 
				GTK_SIGNAL_FUNC (brlui_translation_table_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_braille_style_clicked",	
				GTK_SIGNAL_FUNC (brlui_braille_style_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_cursor_settings_clicked",	
				GTK_SIGNAL_FUNC (brlui_cursor_settings_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_attribute_settings_clicked", 
				GTK_SIGNAL_FUNC (brlui_attribute_settings_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_braille_fill_char_clicked", 
				GTK_SIGNAL_FUNC (brlui_braille_fill_char_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_status_cells_clicked",	
				GTK_SIGNAL_FUNC (brlui_status_cells_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_braille_mapping_clicked", 
				GTK_SIGNAL_FUNC (brlui_braille_mapping_clicked), 
				(gpointer)braille_setting);
    glade_xml_signal_connect_data (xml,"on_bt_braille_settings_close_clicked", 
				GTK_SIGNAL_FUNC (brlui_braille_settings_close_clicked), 
				(gpointer)braille_setting);
}

gboolean 
brlui_load_braille_settings (GtkWidget *parent_window)
{
    if (!w_braille_settings)
    {
	GladeXML *xml;	
	xml = gn_load_interface ("Braille_Settings/braille_settings.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_braille_settings (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_braille_settings),
	    			      GTK_WINDOW (parent_window));
	gtk_window_set_destroy_with_parent ( GTK_WINDOW (w_braille_settings), 
						TRUE);
    }
    else
	gtk_widget_show (w_braille_settings);
    	
    return TRUE;
}


static void 
brlui_braille_device_changeing (Braille *braille_setting)
{
    gchar *tmp = NULL;
    gchar *id = NULL;
    
    sru_return_if_fail (braille_setting);    
    
    tmp = (gchar*)gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cb_braille_device)->entry));

    id  = (gchar*)brlconf_device_id_return (tmp);	 
    if (id)
	brlconf_device_set (id);
    braille_setting->port_no = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_port));
    brlconf_port_no_set (braille_setting->port_no);
}

static void
brlui_braille_device_combo_changed (GtkWidget *entry,
				    gpointer  user_data)
{
#if 0 /* remove when it need to select the tty number */
    gchar   *port_text = NULL;
    gdouble min_val, max_val;
#endif 
    gboolean 	sensitivity;
    sru_return_if_fail (braille_setting);    
    
    if (!strcmp (BRLTTY, gtk_entry_get_text (GTK_ENTRY (entry))))
	sensitivity = FALSE;
    else
	sensitivity = TRUE;
	
    gtk_widget_set_sensitive (GTK_WIDGET (lb_port), sensitivity);
    gtk_widget_set_sensitive (GTK_WIDGET (sp_port), sensitivity);
    
#if 0    /* remove when it need to select the tty number */
    if (!strcmp (BRLTTY, gtk_entry_get_text (GTK_ENTRY (entry))))
    {
	min_val   = (gdouble)MIN_TTY_CONSOL_NO;
	max_val   = (gdouble)MAX_TTY_CONSOL_NO;
	/*To translators: X = X Window System server */
	port_text = _("X display's _terminal number:");
    }
    else
    {
    	min_val   = (gdouble)MIN_SERIAL_PORT_NO;
	max_val   = (gdouble)MAX_SERIAL_PORT_NO;
    	port_text = _("_Port:");
    }

    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_port), min_val, max_val);
    gtk_label_set_text (GTK_LABEL (lb_port), port_text);
    gtk_label_set_use_underline (GTK_LABEL (lb_port), TRUE);
#endif
}

static void
brlui_settings_response_braille_device (GtkDialog *dialog,
					gint       response_id,
					gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    if (response_id == GTK_RESPONSE_OK)
         brlui_braille_device_changeing ( (Braille*) user_data);

    brlconf_device_list_free ();
    gtk_widget_hide ((GtkWidget*)dialog);
}


static gint
brlui_delete_emit_response_cancel (GtkDialog *dialog,
				   GdkEventAny *event,
				   gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}


void 
brlui_set_handlers_braille_device (GladeXML *xml,
				   Braille *braille_setting)
{
    w_braille_device  = glade_xml_get_widget (xml, "w_braille_device");
    cb_braille_device = glade_xml_get_widget (xml, "cb_braille_device");
    sp_port 	      = glade_xml_get_widget (xml, "sp_port");
    lb_port 	      = glade_xml_get_widget (xml, "lb_port");

    g_signal_connect (w_braille_device, "response",
		      G_CALLBACK (brlui_settings_response_braille_device), 
		      (gpointer)braille_setting);
    g_signal_connect (w_braille_device, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);        

    glade_xml_signal_connect (xml,"on_combo-entry1_changed",		
			    GTK_SIGNAL_FUNC (brlui_braille_device_combo_changed));

    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (cb_braille_device)->entry), FALSE);
}


gboolean 
brlui_load_braille_device (Braille *braille_setting)
{
    if (!w_braille_device)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/braille_device.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
    	brlui_set_handlers_braille_device (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_braille_device),
				      GTK_WINDOW (w_braille_settings));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_braille_device), 
						TRUE);
    }
    else
	gtk_widget_show (w_braille_device);
    
    brlui_braille_device_value_add_to_widgets (braille_setting);
    
    return TRUE;
}

static void
brlui_braille_device_set_to_combo (GList *brldev_list)
{
    sru_return_if_fail (brldev_list);    
    
    gtk_combo_set_popdown_strings ( GTK_COMBO (cb_braille_device),
				    brldev_list);
}


gboolean 
brlui_braille_device_value_add_to_widgets (Braille *braille_setting)
{    
    sru_return_val_if_fail (braille_setting, FALSE);    
    
    if (!w_braille_device)
	return FALSE;
	
    brlconf_active_device_list_get ();
    
    brlui_braille_device_set_to_combo (brldev_description_list);

    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (cb_braille_device)->entry), 
			brlconf_return_active_device ());
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (sp_port), 
			       (gdouble)braille_setting->port_no);
    return TRUE;
}

gboolean 
brlui_translation_table_value_add_to_widgets (Braille *braille_setting)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    GtkTreeIter      iter;
    gboolean 	     valid;
    gint 	     l_iter;
    

    sru_return_val_if_fail (w_translation_table, FALSE);
    sru_return_val_if_fail (braille_setting, FALSE);
    
    g_free (braille_setting->translation_table);
    braille_setting->translation_table = 
	    brlconf_translation_table_get ();
    
    model 	  = gtk_tree_view_get_model ( GTK_TREE_VIEW (tv_translation_table));
    selection     = gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_translation_table));
    
    valid = gtk_tree_model_get_iter_first ( model, &iter );

        
    for (l_iter = 0 ; trans_table_list [ l_iter ].state && valid ; l_iter++)
    {
	if (!strcmp (braille_setting->translation_table, trans_table_list [ l_iter ].file_name))
	    gtk_tree_selection_select_iter (selection, &iter);
	else
	    gtk_tree_selection_unselect_iter (selection, &iter);
	    
	valid = gtk_tree_model_iter_next (model, &iter);
    }

    return TRUE;
}


void 
brlui_translation_table_changeing (Braille *braille_setting)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    GtkTreeIter      iter;
    gint 	     l_iter;
    gchar 	    *key;

    sru_return_if_fail (braille_setting);
    
    model 	  = 
	gtk_tree_view_get_model ( GTK_TREE_VIEW (tv_translation_table));
    selection     = 
	gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_translation_table));

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
	return;
    
    gtk_tree_model_get (model, &iter,
        		TABLE_COLUMN, &key,
        		-1);

    for (l_iter = 0 ; trans_table_list [ l_iter ].state ; l_iter++)
    {
	if (!strcmp (key, _(trans_table_list [ l_iter ].state)))
	{
	    g_free (braille_setting->translation_table);
	    braille_setting->translation_table = g_strdup (trans_table_list [ l_iter ].file_name);
	    brlconf_translation_table_set (braille_setting->translation_table);
	}
    }
    g_free (key);
}

static gboolean
brlui_search_equal_func (GtkTreeModel  *model,
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


static GtkTreeModel*
brlui_create_model (void)
{
    GtkListStore *store;
    GtkTreeIter iter;
    gint i = 0;
      
    store = gtk_list_store_new (NO_OF_TABLE_COLUMNS, 
				G_TYPE_STRING);
    
    while (trans_table_list[i].state)
    {
	gtk_list_store_append ( GTK_LIST_STORE (store), &iter);
	gtk_list_store_set ( GTK_LIST_STORE (store), &iter, 
		    	    TABLE_COLUMN, _(trans_table_list[i].state),
		    	    -1);
	i++;
    }
    
    return GTK_TREE_MODEL (store);
}

static void
brlui_settings_response_translation_table ( GtkDialog *dialog,
					    gint       response_id,
					    gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_OK)
         brlui_translation_table_changeing ((Braille*) user_data);

    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    gtk_widget_hide ((GtkWidget*)dialog);
}


void 
brlui_set_handlers_translation_table (GladeXML *xml, Braille *braille_setting)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;

    w_translation_table  = glade_xml_get_widget (xml, "w_translation_table");
    tv_translation_table = glade_xml_get_widget (xml, "tv_translation_table");

    g_signal_connect (w_translation_table, "response",
		      G_CALLBACK (brlui_settings_response_translation_table), 
		      (gpointer)braille_setting);
    g_signal_connect (w_translation_table, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);

    model = brlui_create_model ();
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_translation_table), model);
	    
    g_object_unref (G_OBJECT (model));

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes   (_("Language"),
    							cell_renderer,
							"text", TABLE_COLUMN,
							NULL);	
    gtk_tree_view_column_set_sort_column_id (column, TABLE_COLUMN);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_translation_table), column);
        
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_translation_table));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);    
    
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (tv_translation_table), 
				    TABLE_COLUMN);    
    gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (tv_translation_table),
					brlui_search_equal_func,
					NULL, NULL);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv_translation_table), TRUE);

}

gboolean 
brlui_load_translation_table (Braille *braille_setting)
{
    if (!w_translation_table)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/translation_table.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_translation_table (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_translation_table),
				      GTK_WINDOW (w_braille_settings));
    	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_translation_table), TRUE);
    }
    else
        gtk_widget_show (w_translation_table);	
    
    brlui_translation_table_value_add_to_widgets (braille_setting);
    
    return TRUE;
}

void 
brlui_braille_style_changeing (Braille *braille_setting)
{
    
    g_free (braille_setting->braille_style);
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_braille_style[0])))
	braille_setting->braille_style = g_strdup_printf ("%d", DOT_8);
    else
    	braille_setting->braille_style = g_strdup_printf ("%d", DOT_6);

    brlconf_style_set (braille_setting->braille_style);
}

static void
brlui_settings_response_braille_style ( GtkDialog *dialog,
					gint       response_id,
					gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    if (response_id == GTK_RESPONSE_OK)
        brlui_braille_style_changeing ((Braille*) user_data);

    gtk_widget_hide ((GtkWidget*)dialog);
}


void 
brlui_set_handlers_braille_style (GladeXML *xml, Braille *braille_setting)
{
    w_braille_style    = glade_xml_get_widget (xml, "w_braille_style");
    rb_braille_style[0] = glade_xml_get_widget (xml, "rb_braille_style_1");
    rb_braille_style[1] = glade_xml_get_widget (xml, "rb_braille_style_2");

    g_signal_connect (w_braille_style, "response",
		      G_CALLBACK (brlui_settings_response_braille_style), 
		      (gpointer)braille_setting);
    g_signal_connect (w_braille_style, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);
}

gboolean 
brlui_load_braille_style (Braille *braille_setting)
{
    if (!w_braille_style)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/braille_style.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_braille_style (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_braille_style),
				      GTK_WINDOW (w_braille_settings));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_braille_style), 
					        TRUE);
    }
    else
	gtk_widget_show (w_braille_style);
    
    brlui_braille_style_value_add_to_widgets (braille_setting);

    return TRUE;
}

gboolean 
brlui_braille_style_value_add_to_widgets (Braille *braille_setting)
{
    gint iter;
    sru_return_val_if_fail (braille_setting, FALSE);
    
    if (!w_braille_style)
	return FALSE;

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_braille_style[0]), TRUE);
    
    for (iter = 0 ; iter < G_N_ELEMENTS (braille_style) ; iter++)
    {
	if (!strcmp (braille_setting->braille_style, braille_style[iter]))
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_braille_style[iter]), TRUE);
    }

    return TRUE;
}

static void 
brlui_cursor_setting_changeing (Braille *braille_setting)
{
    gint i;
    for (i = 0 ; i < G_N_ELEMENTS (cursor_style) ; i++)
    {
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_cursor_settings[i])))
	{
	    g_free (braille_setting->cursor_style);
	    braille_setting->cursor_style = g_strdup (cursor_style [i]);
	    brlconf_cursor_style_set (braille_setting->cursor_style);
	    break;
	}
    }
}

static void
brlui_settings_response_cursor_settings (GtkDialog *dialog,
					 gint       response_id,
					 gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    if (response_id == GTK_RESPONSE_OK)
	    brlui_cursor_setting_changeing ((Braille*) user_data);

    gtk_widget_hide ((GtkWidget*)dialog);
}


void 
brlui_set_handlers_cursor_settings (GladeXML *xml, Braille *braille_setting)
{
    w_cursor_settings     = glade_xml_get_widget (xml, "w_cursor_settings");
    rb_cursor_settings[0] = glade_xml_get_widget (xml, "rb_cursor_settings_1");
    rb_cursor_settings[1] = glade_xml_get_widget (xml, "rb_cursor_settings_2");
    rb_cursor_settings[2] = glade_xml_get_widget (xml, "rb_cursor_settings_3");

    g_signal_connect (w_cursor_settings, "response",
		      G_CALLBACK (brlui_settings_response_cursor_settings), 
		      (gpointer)braille_setting);
    g_signal_connect (w_cursor_settings, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);
}



gboolean 
brlui_load_cursor_settings (Braille *braille_setting)
{
    if (!w_cursor_settings)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/cursor_settings.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_cursor_settings (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_cursor_settings),
				      GTK_WINDOW (w_braille_settings));
	gtk_window_set_destroy_with_parent ( GTK_WINDOW (w_cursor_settings), 
					        TRUE);
    }
    else
	gtk_widget_show (w_cursor_settings);

    brlui_cursor_setting_value_add_to_widgets (braille_setting);

    return TRUE;
}

gboolean 
brlui_cursor_setting_value_add_to_widgets (Braille *braille_setting)
{
    gint i;
    
    sru_return_val_if_fail (braille_setting, FALSE);
    
    if (!w_cursor_settings)
	return FALSE;
    
    for (i = 0; i < G_N_ELEMENTS (cursor_style); i++)
    {
	if (!strcmp (braille_setting->cursor_style, cursor_style[i]))
	{
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_cursor_settings[i]),
					TRUE);
	    break;
	}
    }
    return FALSE;
}


void 
brlui_attribute_setting_changeing (Braille *braille_setting)
{
    gint i;
    
    for (i = 0;i < G_N_ELEMENTS (attribute_value) ;i++)
    {
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_attribute_settings[i])))
	{
	    if (braille_setting->attributes != attribute_value[i])
	    {
	        braille_setting->attributes = attribute_value[i];
	        brlconf_attributes_set (braille_setting->attributes);
	    }
	    break;
	}
    }
}

static void
brlui_settings_response_attribute_settings (GtkDialog *dialog,
					    gint       response_id,
					    gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    if (response_id == GTK_RESPONSE_OK)
         brlui_attribute_setting_changeing ((Braille*) user_data);

    gtk_widget_hide ((GtkWidget*)dialog);
}


void 
brlui_set_handlers_attribute_settings (GladeXML *xml,
				       Braille *braille_setting)
{
    w_attribute_settings     = glade_xml_get_widget (xml, "w_attribute_settings");
    rb_attribute_settings[0] = glade_xml_get_widget (xml, "rb_attribute_settings_1");
    rb_attribute_settings[1] = glade_xml_get_widget (xml, "rb_attribute_settings_2");
    rb_attribute_settings[2] = glade_xml_get_widget (xml, "rb_attribute_settings_3");
    rb_attribute_settings[3] = glade_xml_get_widget (xml, "rb_attribute_settings_4");

    g_signal_connect (w_attribute_settings, "response",
		      G_CALLBACK (brlui_settings_response_attribute_settings), 
		      (gpointer)braille_setting);
    g_signal_connect (w_attribute_settings, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);

}



gboolean 
brlui_load_attribute_settings (Braille *braille_setting)
{
    if (!w_attribute_settings)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/attribute_settings.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_attribute_settings (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_attribute_settings),
				      GTK_WINDOW (w_braille_settings));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_attribute_settings), 
						TRUE);
    }
    else
	gtk_widget_show (w_attribute_settings);

    brlui_attribute_setting_value_add_to_widgets (braille_setting);    
    
    return TRUE;
}

gboolean 
brlui_attribute_setting_value_add_to_widgets (Braille *braille_setting)
{
    sru_return_val_if_fail (braille_setting, FALSE);
    
    if (!w_attribute_settings)
	return FALSE;
    
    switch (braille_setting->attributes)
    {
        case DOT_8:  
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_attribute_settings[0]), TRUE);
	    break;
        case DOT_7:  
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_attribute_settings[1]), TRUE);
	    break;
        case DOT_78: 
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_attribute_settings[2]), TRUE);
	    break;
        case DOT_OFF:
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_attribute_settings[3]), TRUE);
	    break;
        default:     
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_attribute_settings[3]), TRUE);
	    break;
    }
    return TRUE;
}




gboolean 
brlui_braille_fill_char_value_add_to_widgets (Braille *braille_setting)
{
    sru_return_val_if_fail (braille_setting, FALSE);
    sru_return_val_if_fail (braille_setting->fill_char, FALSE);
    
    if (!w_braille_fill_char)
	return FALSE;
    
    gtk_entry_set_text (GTK_ENTRY (et_fill_char), 
			braille_setting->fill_char);
    return TRUE;
}

void 
brlui_braille_fill_char_changeing (Braille *braille_setting)
{
    gchar *tmp;
    
    sru_return_if_fail (braille_setting);
    
    tmp = (gchar*)gtk_entry_get_text (GTK_ENTRY (et_fill_char));
    
    if (tmp != NULL)
    {
	if (strcmp (braille_setting->fill_char,tmp))
	{
	    g_free (braille_setting->fill_char);
	    braille_setting->fill_char = g_strdup (tmp);
	    brlconf_fill_char_set (braille_setting->fill_char);
	}
    }
    else
    {
	g_free (braille_setting->fill_char);
        braille_setting->fill_char = g_strdup (DEFAULT_BRAILLE_FILL_CHAR);
        brlconf_fill_char_set (braille_setting->fill_char);
    }
}


static void
brlui_settings_response_braille_fill_char ( GtkDialog *dialog,
					    gint       response_id,
					    gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    if (response_id == GTK_RESPONSE_OK)
        brlui_braille_fill_char_changeing ((Braille*) user_data);

    gtk_widget_hide ((GtkWidget*)dialog);
}


void 
brlui_set_handlers_braille_fill_char (GladeXML *xml,
				      Braille *braille_setting)
{
    w_braille_fill_char = glade_xml_get_widget (xml, "w_braille_fill_char");
    et_fill_char = glade_xml_get_widget (xml, "et_fill_char");

    g_signal_connect (w_braille_fill_char, "response",
		      G_CALLBACK (brlui_settings_response_braille_fill_char), 
		      (gpointer)braille_setting);
    g_signal_connect (w_braille_fill_char, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);
        
}

gboolean 
brlui_load_braille_fill_char (Braille *braille_setting)
{
    if (!w_braille_fill_char)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/braille_fill_char.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_braille_fill_char (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_braille_fill_char),
				      GTK_WINDOW (w_braille_settings));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_braille_fill_char), 
					    TRUE);
    }
    else
	gtk_widget_show (w_braille_fill_char);

    brlui_braille_fill_char_value_add_to_widgets (braille_setting);    

    return TRUE;
}


gboolean 
brlui_status_cell_value_add_to_widgets (Braille *braille_setting)
{
    gint i;
    sru_return_val_if_fail (braille_setting, FALSE);
    sru_return_val_if_fail (braille_setting->status_cell, FALSE);
    
    if (!w_status_cell)
	return FALSE;
    
    if (braille_setting->status_cell)
    {
	for (i = 0; i < G_N_ELEMENTS (status_cell_value); i++)
	{
	    if (!strcmp (braille_setting->status_cell, status_cell_value[i]))
	    {
	        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
					     (rb_status_cell[i]), TRUE);
		break;
	    }
	}
    }
    return TRUE;
}


void 
brlui_status_cell_changeing (Braille *braille_setting)
{
    gint i;
    
    sru_return_if_fail (braille_setting);
    
    for (i = 0; i < G_N_ELEMENTS (status_cell_value); i++)
    {
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_status_cell[i])))
	{
	    if (strcmp (braille_setting->status_cell, status_cell_value[i]))
	    {
		g_free (braille_setting->status_cell);
	        braille_setting->status_cell = g_strdup (status_cell_value[i]);
		brlconf_status_cell_set (braille_setting->status_cell);
	    }
	    break;
	}
    }
}

static void
brlui_settings_response_status_cell (GtkDialog *dialog,
				    gint       response_id,
				    gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    if (response_id == GTK_RESPONSE_OK)
        brlui_status_cell_changeing ((Braille*)user_data);

    gtk_widget_hide ((GtkWidget*)dialog);
}


void 
brlui_set_handlers_status_cell (GladeXML *xml, Braille *braille_setting)
{
    w_status_cell 	= glade_xml_get_widget (xml, "w_status_cells");
    rb_status_cell[0] 	= glade_xml_get_widget (xml, "rb_status_cells_1");
    rb_status_cell[1] 	= glade_xml_get_widget (xml, "rb_status_cells_2");

    g_signal_connect (w_status_cell, "response",
		      G_CALLBACK (brlui_settings_response_status_cell), 
		      (gpointer)braille_setting);
    g_signal_connect (w_status_cell, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);

}


gboolean 
brlui_load_status_cell (Braille *braille_setting)
{
    if (!w_status_cell)
    {
	GladeXML *xml;
    	xml = gn_load_interface ("Braille_Settings/status_cell.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_status_cell (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_status_cell),
				      GTK_WINDOW (w_braille_settings));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_status_cell), 
						TRUE);
    }
    else
        gtk_widget_show (w_status_cell);	

    brlui_status_cell_value_add_to_widgets (braille_setting);    

    return TRUE;
}


/******************************************************************************/	    
static void
brlui_selection_add_func (GtkTreeModel 	*model,
		    	  GtkTreePath	*path,
			  GtkTreeIter	*iter,
			  gpointer	data)
{       
    if (GTK_IS_LIST_STORE (model))
    {
	gint pos;
	gchar *text = NULL; 
	gtk_tree_model_get (model, 	 iter,
                    	    CMD_COLUMN, &text,
                    	    -1);
			    
	for (pos = 0 ; cmd_function[pos].descr ; pos++)
	    if (!strcmp (_(cmd_function [pos].descr), text)) break;
	    
	if (cmd_function[pos].descr)
	{
	    (brl_current_item.current_keyitem)->command_list = 
		g_slist_prepend ((brl_current_item.current_keyitem)->command_list, 
				g_strdup (cmd_function [pos].name));
	}
	
	g_free (text);
    }
}


static gboolean
brlui_add_modify_ok (void)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    GtkTreeIter	     iter;

    gchar *txt;
    gchar *key;
    
    sru_return_val_if_fail (tv_cmd_list, FALSE);
    sru_return_val_if_fail (tv_braille_keys, FALSE);
    sru_return_val_if_fail (cb_braille_key, FALSE);
    
    key 	  = (gchar*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cb_braille_key)->entry));
    selection     = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv_cmd_list));
    model	  = gtk_tree_view_get_model 	(GTK_TREE_VIEW (tv_braille_keys));

    if (!key || strlen (key) == 0)
	return FALSE;

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
	gn_show_message (_("No selected command!"));
	return FALSE;
    }
    
    if (brl_current_item.current_is_new)
    {
	if (!cmdconf_check_if_item_exist (braille_keys_list, key))
	{
	    KeyItem * new_item = cmdconf_new_keyitem ();
	    
	    sru_return_val_if_fail (new_item, FALSE);
		
	    new_item->key_code = g_strdup (key);
	    braille_keys_list = 
		g_slist_prepend (braille_keys_list , new_item);		    
		
	    brl_current_item.current_keyitem = new_item;
	    (brl_current_item.current_keyitem)->command_list = NULL;
	    (brl_current_item.current_keyitem)->commands = NULL;
    	    gtk_list_store_append (GTK_LIST_STORE (model), 
				    &brl_current_item.current_iter);
	}
	else
	{
	    gn_show_message (_("Invalid device key!"));
	    return FALSE;
	}
    }
        
    (brl_current_item.current_keyitem)->command_list =
	cmdconf_free_list_and_data ((brl_current_item.current_keyitem)->command_list);

    gtk_tree_selection_selected_foreach (selection, 
    					brlui_selection_add_func, 
					NULL);
					
    (brl_current_item.current_keyitem)->command_list = 
        cmdconf_check_integrity ( (brl_current_item.current_keyitem)->command_list);

    txt = cmdconf_create_view_string ((brl_current_item.current_keyitem)->command_list);
        
    if (!txt)
	return FALSE;
	
    (brl_current_item.current_keyitem)->commands = txt;
    gtk_list_store_set (GTK_LIST_STORE (model), &brl_current_item.current_iter, 
			KEYS_COLUMN, 	(brl_current_item.current_keyitem)->key_code,
		    	FUNC_COLUMN, 	_((brl_current_item.current_keyitem)->commands),
		    	-1);
    return TRUE;
}

static void 
brlui_treeviews_set_current_value (void)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;


    if (!w_braille_mapping_add_modify)
	 return;

    model 	  = gtk_tree_view_get_model 	( GTK_TREE_VIEW (tv_cmd_list));
    selection     = gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_cmd_list));
    
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    
    gtk_tree_selection_unselect_all (selection);	
            
    if (!brl_current_item.current_is_new)
    {
        if (!(brl_current_item.current_keyitem)->key_code) 
	return;

	if (strcmp ((brl_current_item.current_keyitem)->key_code, BLANK))
	{	
	GSList *elem = NULL;
	for (elem = (brl_current_item.current_keyitem)->command_list ; elem ; elem = elem->next)
	{
	    GtkTreeIter iter;
	    gboolean valid;
	    gint pos;
	    
	    for (pos = 0 ; cmd_function[pos].name ; pos++)
		    if (!strcmp (cmd_function[pos].name, (gchar*)elem->data)) break;
		    	    
	    valid = gtk_tree_model_get_iter_first (model, &iter);
	    
	    while (valid)
	    {
		gchar *key = NULL;
		if (GTK_IS_LIST_STORE (model))
		{
		    gtk_tree_model_get (model, 	     &iter,
            				CMD_COLUMN,  &key,
            				-1);
		}
		if (cmd_function[pos].name && !strcmp (_(cmd_function[pos].descr), key))
		{
		    gtk_tree_selection_select_iter (selection, &iter);
		    break;
		}
		g_free (key);
		valid = gtk_tree_model_iter_next (model, &iter);	    
	    }
	}
	}
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (cb_braille_key)->entry), 
			    (brl_current_item.current_keyitem)->key_code);
    }
    else
        gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (cb_braille_key)->entry),  "");
	
    gtk_widget_set_sensitive (cb_braille_key, brl_current_item.current_is_new);
}

static void
brlui_response_add_modify_event (GtkDialog *dialog,
		    		gint       response_id,
		    		gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }

    if (response_id == GTK_RESPONSE_OK)
	brlui_add_modify_ok ();

    gtk_widget_hide ((GtkWidget*)dialog);	
}


static void
brlui_cmd_list_activated (GtkTreeView       *tree_view,
                	  GtkTreePath       *path,
			  GtkTreeViewColumn *column)
{
    brlui_response_add_modify_event (GTK_DIALOG (w_braille_mapping_add_modify ),
				    GTK_RESPONSE_OK,
				    NULL);
}

#define VARIO_DISPLAY_KEY_NO 6
static void
brlui_load_braille_key_combination_list (void)
{
    gint i,j;
/* NOTE:
    This values are hardcoded because, the UI does not get any information
    from the braille device about the buttons keynames.
*/
    for (i = 0 ; i < VARIO_DISPLAY_KEY_NO ; i++)
    {
    	braille_key_combination_list = 
		    g_list_append (braille_key_combination_list,
				    g_strdup_printf ("DK%02d",i));
	for (j = 0 ; j < VARIO_DISPLAY_KEY_NO ; j++)
	{
	    if (i != j)
		braille_key_combination_list = 
		    g_list_append (braille_key_combination_list,
				    g_strdup_printf ("DK%02dDK%02d",i,j));
	}
    }
}

static void
brlui_free_braille_key_combination_list (void)
{
    GList *elem = NULL;
    
    for (elem = braille_key_combination_list ; elem ; elem = elem->next)
	g_free ((gchar*)elem->data);

    g_list_free (braille_key_combination_list);
    braille_key_combination_list = NULL;
}

static void
brlui_braille_mapping_add_combo_list (void)
{
    sru_return_if_fail (cb_braille_key);
    sru_return_if_fail (braille_key_combination_list);
    
    gtk_combo_set_popdown_strings ( GTK_COMBO (cb_braille_key),
				    braille_key_combination_list);

}

static GtkListStore*
brlui_populate_cmd_list_store_with_keys (GtkListStore *store)
{
    GtkTreeIter iter;
    gint pos;
    
    for (pos = 0 ; cmd_function[pos].descr ; pos++)
    {
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 
		    	    CMD_COLUMN, _(cmd_function[pos].descr), 
		    	    -1);
    }
    
    return store;
}

static GtkTreeModel*
brlui_create_model_add_modify (void)
{
    GtkListStore *store;
  
    store = gtk_list_store_new (NO_OF_CMD_COLUMNS, 
				G_TYPE_STRING);

    brlui_populate_cmd_list_store_with_keys (store);
  
    return GTK_TREE_MODEL (store);
}

static void
brlui_set_handlers_braille_mapping_add_modify (GladeXML *xml)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;
    
    w_braille_mapping_add_modify 	
	= glade_xml_get_widget (xml, "w_braille_mapping_add_modify");
    tv_cmd_list		
	= glade_xml_get_widget (xml, "tv_cmd_list");
    cb_braille_key    
	= glade_xml_get_widget (xml, "cb_braille_key");
	
    g_signal_connect (w_braille_mapping_add_modify 	 , "response",
		      G_CALLBACK (brlui_response_add_modify_event), NULL);
    g_signal_connect (w_braille_mapping_add_modify 	 , "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);

    brlui_braille_mapping_add_combo_list ();

    model = brlui_create_model_add_modify ();
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_cmd_list), model);
    
    gtk_tree_sortable_set_sort_column_id (  GTK_TREE_SORTABLE (model), 
					    CMD_COLUMN, 
					    GTK_SORT_ASCENDING);

    g_signal_connect (tv_cmd_list, "row_activated", 
		      G_CALLBACK (brlui_cmd_list_activated), 
		      model);

	    
    g_object_unref (G_OBJECT (model));

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes   (_("Commands"),
    							cell_renderer,
							"text", CMD_COLUMN,
							NULL);							
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_cmd_list), column);
    gtk_tree_view_column_set_sort_column_id (column, CMD_COLUMN);
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv_cmd_list));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE); 
    
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (tv_cmd_list), 
				    CMD_COLUMN);    
    gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (tv_cmd_list),
					brlui_search_equal_func,
					NULL, NULL);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv_cmd_list), TRUE);

}

static gboolean
brlui_load_braille_mapping_add_modify (void)
{
    if (!w_braille_mapping_add_modify)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/braille_mapping.glade2", "w_braille_mapping_add_modify");
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_braille_mapping_add_modify (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_braille_mapping_add_modify),
				      GTK_WINDOW (w_braille_key_mapping));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_braille_mapping_add_modify), 
						TRUE);
    }
    else
        gtk_widget_show (w_braille_mapping_add_modify);
	
    
    brlui_treeviews_set_current_value ();
    
    return TRUE;
}

static GSList*
brlui_removed_item_list_free (GSList *list)
{
    if (!list)
	return NULL;
    
    return
	cmdconf_free_list_and_data (list);
}

static void 
brlui_braille_key_mapping_changeing (Braille *braille_setting)
{
    gint i;
    
    for (i = 0 ; i < NO_ITEM_IN_POSITION_SENSORS ; i++)
    {
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_position_switches[i])))
	{
	    braille_setting->position = i;
	    brlconf_position_sensor_set (braille_setting->position);
	    break;
	}
    }
    for (i = 0 ; i < NO_ITEM_IN_OPTICAL_SENSORS ; i++)
    {
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_optical_sensor[i])))
	{
	    braille_setting->optical = i;
	    brlconf_optical_sensor_set (braille_setting->optical);
	    break;
	}
    }


    removed_items_list =
	cmdconf_remove_items_from_gconf_list (removed_items_list);
	
    removed_items_list = 
	brlui_removed_item_list_free (removed_items_list);

    cmdconf_braille_key_set  (braille_keys_list);
    cmdconf_key_cmd_list_set (braille_keys_list);
    
    cmdconf_changes_end_event ();
}

static void
brlui_clear_treeview (void)
{
    GtkTreeModel *model = 
        gtk_tree_view_get_model (GTK_TREE_VIEW (tv_braille_keys));
	
    gtk_list_store_clear (GTK_LIST_STORE (model));
}

static void
brlui_settings_response_braille_key_mapping (GtkDialog *dialog,
					    gint       response_id,
					    gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_OK)
    {
        brlui_braille_key_mapping_changeing ((Braille*) user_data);
    }
    
    if (response_id == GTK_RESPONSE_APPLY)
    {
        brlui_braille_key_mapping_changeing ((Braille*) user_data);
	return;
    }
    
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-braille-prefs");
	return;
    }
    
    removed_items_list = 
	    brlui_removed_item_list_free (removed_items_list);
    
    braille_keys_list = 
	    cmdconf_free_keyitem_list_item (braille_keys_list);

    brlui_clear_treeview ();
    
    gtk_widget_hide ((GtkWidget*)dialog);
    
    brlui_free_braille_key_combination_list ();
}

static void
brlui_braille_keys_add_clicked(GtkWidget *button,
	        	       gpointer user_data)
{
    brl_current_item.current_is_new = TRUE;
    brlui_load_braille_mapping_add_modify ();
}

static void
brlui_braille_keys_modify (void)
{
    KeyItem 	     *new_code  = NULL;
    GtkTreeIter      iter;
    GSList 	     *elem = NULL;
    gchar            *key = NULL;
    GtkTreeModel     *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv_braille_keys));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv_braille_keys));

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gn_show_message (_("No selected item to modify!"));
	return;
    }
    
    sru_return_if_fail (GTK_IS_LIST_STORE (model));
    
    gtk_tree_model_get (model, 	     &iter,
    			KEYS_COLUMN, &key,
            		-1);
	
    if (!key || strlen (key) == 0)
    {
	sru_message (_("Invalid selected item!"));	    
	g_free (key);
	return;
    }
	
    for (elem = braille_keys_list; elem ; elem = elem->next)
    {
	if (!strcmp (((KeyItem*)elem->data)->key_code, key))
	{
	    new_code = elem->data;
	    break;
	}
    }
		
    g_free (key);

    sru_return_if_fail (new_code);
    
    brl_current_item.current_keyitem = new_code;
    brl_current_item.current_iter = iter;
    brl_current_item.current_is_new = FALSE;
    brlui_load_braille_mapping_add_modify ();
}

static void
brlui_braille_keys_modify_clicked (GtkWidget *button,
	        	    	   gpointer user_data)
{
    brlui_braille_keys_modify ();
}

static void
brlui_braille_keys_row_activated_cb (GtkTreeView       *tree_view,
                		    GtkTreePath       *path,
				    GtkTreeViewColumn *column)
{
    brlui_braille_keys_modify ();
}

static GSList*
brlui_removed_list_add (GSList *list,
			const gchar *str)
{
    GSList *elem = NULL;

    sru_return_val_if_fail (str, list);    
	
    for (elem = list ; elem ; elem = elem->next)
     if (!strcmp ((gchar*)elem->data, str))
        return list;

    list = g_slist_append (list, g_strdup (str));
    
    return list;
}


static GSList*
brlui_remove_keyitem_from_list (GSList 	*list,
				GtkTreeView *tree_view)
{
    gchar 	*key = NULL;
    GtkTreeIter  iter;
    GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);	    
    
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gn_show_message (_("No selected item to remove!"));
	return list;
    }	    
    
    sru_return_val_if_fail (GTK_IS_LIST_STORE (model), list);
    
    gtk_tree_model_get (model, &iter,
			KEYS_COLUMN, &key,
            		-1);
	
    if (!key || strlen (key) == 0)
    {
        sru_message (_("Invalid selected item!"));
        g_free (key);
        return list;
    }
	
    if (strcmp (key, BLANK))
    {
	GSList *elem = NULL;
			
	for (elem = list; elem ; elem = elem->next)
	    if (!strcmp (key, ((KeyItem*)elem->data)->key_code)) break;
		
	g_free (key);	
	
	sru_return_val_if_fail (elem, list);	    
	
	list = g_slist_remove_link (list, elem);
		
	removed_items_list =
		    brlui_removed_list_add (removed_items_list,
					   ((KeyItem*)elem->data)->key_code);
					   
	elem = cmdconf_free_keyitem_list_item (elem);
	
	gtk_list_store_remove ( GTK_LIST_STORE (model), &iter);
    }		
    
    return list;
}

static void
brlui_braille_keys_remove_clicked (GtkWidget *button,
	        		   gpointer user_data)
{
    braille_keys_list = 
	brlui_remove_keyitem_from_list (braille_keys_list,
				    GTK_TREE_VIEW (tv_braille_keys));

}

static void
brlui_braille_keys_selection_changed (GtkTreeSelection *selection,
				      gpointer  	user_data)
{
    gboolean 	sensitive;
    
    sensitive = gtk_tree_selection_get_selected (selection, NULL, NULL);
    
    gtk_widget_set_sensitive (GTK_WIDGET (bt_braille_key_remove), sensitive);
    gtk_widget_set_sensitive (GTK_WIDGET (bt_braille_key_modify), sensitive);
}


static GtkListStore*
brlui_populate_list_store_with_keys (GtkListStore *store, 
				     GSList *list)
{
    GtkTreeIter iter;
    GSList *elem = NULL;
    
    if (!list) 
	return store;
    
    for (elem = list; elem ; elem = elem->next)
    {
	gtk_list_store_append ( GTK_LIST_STORE (store), &iter);
	gtk_list_store_set ( GTK_LIST_STORE (store), &iter, 
		    	    KEYS_COLUMN, ((KeyItem*)elem->data)->key_code,
		    	    FUNC_COLUMN, _(((KeyItem*)elem->data)->commands),
		    	    -1);
    }
    
    return store;
}


static void
brlui_update_list_stores (void)
{
    GtkTreeModel *model = 
	gtk_tree_view_get_model (GTK_TREE_VIEW (tv_braille_keys));
        
    if (braille_keys_list && GTK_IS_LIST_STORE (model))
	brlui_populate_list_store_with_keys (GTK_LIST_STORE (model), 
					    braille_keys_list);
					        
}

static GtkTreeModel*
brlui_create_model_braille_key (void)
{
    GtkListStore *store;
      
    store = gtk_list_store_new (NO_OF_KEYS_COLUMNS, 
				G_TYPE_STRING, 
				G_TYPE_STRING);
    brlui_populate_list_store_with_keys (store, braille_keys_list);
    return GTK_TREE_MODEL (store);
}



static void 
brlui_set_handlers_braille_key_mapping	(GladeXML *xml,
					Braille *braille_setting)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;

    gint i;
    gchar *name;
    w_braille_key_mapping = glade_xml_get_widget (xml, "w_braille_mapping");
    bt_braille_key_modify = glade_xml_get_widget (xml, "bt_braille_key_modify");
    bt_braille_key_remove = glade_xml_get_widget (xml, "bt_braille_key_remove");
    tv_braille_keys	  = glade_xml_get_widget (xml, "tv_braille_keys");
    
    for (i = 0 ; i < NO_ITEM_IN_POSITION_SENSORS ; i++)
    {
	name = g_strdup_printf ("rb_position_switches_%d", i);
	rb_position_switches[i] = glade_xml_get_widget (xml, name);
	g_free (name);
	name = NULL;
    }
    
    for (i = 0 ; i < NO_ITEM_IN_OPTICAL_SENSORS ; i++)
    {
	name = g_strdup_printf ("rb_optical_sensor_%d",i);
	rb_optical_sensor[i] = glade_xml_get_widget (xml, name);
	g_free (name);
	name = NULL;
    }

    glade_xml_signal_connect (xml,"on_bt_braille_key_remove_clicked",  
		GTK_SIGNAL_FUNC (brlui_braille_keys_remove_clicked));

    glade_xml_signal_connect (xml,"on_bt_braille_key_add_clicked",  	
		GTK_SIGNAL_FUNC (brlui_braille_keys_add_clicked));
    glade_xml_signal_connect (xml,"on_bt_braille_key_modify_clicked",	
		GTK_SIGNAL_FUNC (brlui_braille_keys_modify_clicked));
    
    g_signal_connect (w_braille_key_mapping, "response",
		      G_CALLBACK (brlui_settings_response_braille_key_mapping), 
		      (gpointer)braille_setting);
    g_signal_connect (w_braille_key_mapping, "delete_event",
                      G_CALLBACK (brlui_delete_emit_response_cancel), NULL);    

    model = brlui_create_model_braille_key ();
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_braille_keys), model);
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv_braille_keys));
    
    g_signal_connect (selection, "changed",
		      G_CALLBACK (brlui_braille_keys_selection_changed), NULL);
    
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE (model), 
					    KEYS_COLUMN, 
					    GTK_SORT_ASCENDING);


    g_signal_connect (tv_braille_keys, "row_activated", 
		      G_CALLBACK (brlui_braille_keys_row_activated_cb), 
		      model);

	    
    g_object_unref (G_OBJECT (model));
    
    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes   (_("Braille Keys"),
    							cell_renderer,
							"text", KEYS_COLUMN,
							NULL);
    gtk_tree_view_column_set_sort_column_id (column, KEYS_COLUMN);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_braille_keys),column);
    column = gtk_tree_view_column_new_with_attributes   (_("Commands"),
    							cell_renderer,
							"text", FUNC_COLUMN,
							NULL);    
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_braille_keys), column);
        
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    
    brlui_braille_keys_selection_changed (selection, NULL);
    
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (tv_braille_keys), 
				    KEYS_COLUMN);    
    gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (tv_braille_keys),
					brlui_search_equal_func,
					NULL, NULL);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv_braille_keys), TRUE);
}



gboolean 
brlui_braille_key_mapping_value_add_to_widgets (Braille *braille_setting)
{
    sru_return_val_if_fail (braille_setting, FALSE);

    if (!w_braille_key_mapping)
	return FALSE;
	    
    if (braille_setting->position != -1 && 
	braille_setting->position < NO_ITEM_IN_POSITION_SENSORS)
    {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
				     (rb_position_switches [braille_setting->position]), 
				    TRUE);
    }
    
    if (braille_setting->optical != -1 && 
	braille_setting->optical < NO_ITEM_IN_OPTICAL_SENSORS)
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON 
				     (rb_optical_sensor [braille_setting->optical]), 
				     TRUE);
    }

    return TRUE;
}

gboolean 
brlui_load_braille_key_mapping	(Braille *braille_setting)
{
    brlui_load_braille_key_combination_list ();
    
    braille_keys_list = cmdconf_braille_key_get (braille_keys_list);
    
    if (!w_braille_key_mapping)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Braille_Settings/braille_mapping.glade2", "w_braille_mapping");
	sru_return_val_if_fail (xml, FALSE);
	brlui_set_handlers_braille_key_mapping (xml, braille_setting);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_braille_key_mapping),
				      GTK_WINDOW (w_braille_settings));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_braille_key_mapping), 
						TRUE);
    }
    else
    {
	brlui_update_list_stores ();
	gtk_widget_show (w_braille_key_mapping);	
    }
    
    brlui_braille_key_mapping_value_add_to_widgets (braille_setting);    

    return TRUE;
}

