/* cmdmapui.c
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
#include "libsrconf.h"
#include "cmdmapui.h"
#include "SRMessages.h"
#include <glade/glade.h>
#include "gnopiui.h"
#include "srintl.h"

#define MODIFIER(K,C) (K == TRUE ? C : "")
#define MODIFIER_IS(STR,MODIFIER,MODIFIER_STRING) (g_strrstr(STR,MODIFIER)?MODIFIER_STRING:"")
#define LINE(X)       (X == TRUE ? "-" : "")
#define STR(X)	      (X != NULL ? X : "")
#define STR_UPPER(X,Y)    ((Y == TRUE && g_unichar_isalpha (X) == TRUE) ? g_utf8_strup (X, -1) : X)

#define ALT_KEY		"Alt"
#define CONTROL_KEY	"Ctrl"
#define SHIFT_KEY	"Shift"

#define KEY_SEPARATOR	"+"
#define KEY_NAME_SEPARATOR "-"

CmdMapFunctionsList cmd_function[] = 
    {
  	{"decrease y scale", 	N_("decrease y scale")},
	{"decrease x scale", 	N_("decrease x scale")},
	{"increase y scale", 	N_("increase y scale")},
	{"increase x scale", 	N_("increase x scale")},
	{"lock xy scale", 	N_("lock xy scale")},
	{"smoothing toggle", 	N_("smoothing toggle")},
	{"cursor toggle", 	N_("cursor toggle")},
	{"cursor on/off", 	N_("cursor on/off")},
	{"cursor mag on/off", 	N_("cursor magnification on/off")},
	{"decrease cursor size", N_("decrease cursor size")},
	{"increase cursor size", N_("increase cursor size")},
	{"invert on/off", 	N_("invert on/off")},
	{"mag default", 	N_("magnifier default")},
	{"crosswire on/off", 	N_("crosshair on/off")},
	{"crosswire clip on/off", N_("crosshair clip on/off")},
	{"decrease crosswire size", N_("decrease crosshair size")},
	{"increase crosswire size", N_("increase crosshair size")},
	{"automatic panning on/off", N_("automatic panning on/off")},
	{"mouse tracking toggle", N_("mouse tracking toggle")},
	{"load magnifier defaults", N_("load magnifier defaults")},

	{"decrease pitch", 	N_("decrease pitch")},
	{"increase pitch", 	N_("increase pitch")},
	{"default pitch", 	N_("default pitch")},
	{"decrease rate", 	N_("decrease rate")},
	{"increase rate", 	N_("increase rate")},
	{"default rate", 	N_("default rate")},
	{"decrease volume", 	N_("decrease volume")},
	{"increase volume", 	N_("increase volume")},
	{"default volume", 	N_("default volume")},
	{"speech default", 	N_("speech default")},
	
	{"goto parent", 	N_("goto parent")},
	{"goto child", 		N_("goto child")},
	{"goto previous", 	N_("goto previous")},
	{"goto next",		N_("goto next")},
	{"repeat last", 	N_("repeat last")},
	{"goto focus", 		N_("goto focus")},
	{"goto title", 		N_("goto title")},
	{"goto menu", 		N_("goto menu")},
	{"goto toolbar", 	N_("goto toolbar")},
	{"goto statusbar", 	N_("goto statusbar")},
	{"widget surroundings", N_("widget surroundings")},
	
	{"goto caret", 		N_("goto caret")},
	{"goto first", 		N_("goto first")},
	{"goto last", 		N_("goto last")},
	{"change navigation mode", N_("change navigation mode")},

	{"toggle tracking mode",N_("toggle flat review/focus tracking mode")},	
	{"flat review", 	N_("flat review")},
	{"window hierarchy", 	N_("window hierarchy")},
	{"read whole window", 	N_("read whole window")},
	{"detailed informations", N_("detailed informations")},
	{"do default action", 	N_("do default action")},
	{"window overview", 	N_("window overview")},
	{"find next",		N_("find next")},
	{"find set", 		N_("find set")},
	{"attributes at caret", N_("attributes at caret")},	
	{"watch current object", N_("watch current object")},	
	{"unwatch all objects", N_("unwatch all objects")},	

	{"mouse left press", 	N_("mouse left press")},
	{"mouse left click", 	N_("mouse left click")},
	{"mouse left release", 	N_("mouse left release")},
	{"mouse right press", 	N_("mouse right press")},
	{"mouse right click", 	N_("mouse right click")},
	{"mouse right release", N_("mouse right release")},
	{"mouse middle press", 	N_("mouse middle press")},
	{"mouse middle release", N_("mouse middle release")},
	{"mouse middle click", 	N_("mouse middle click")},
	{"mouse goto current", 	N_("mouse goto current")},
	
	{"shutup",		N_("shutup")},
	{"pause/resume",	N_("pause/resume")},
	
	{"char left", 		N_("char left")}, 
	{"char right", 		N_("char right")},
	{"display left", 	N_("display left")},
	{"display right", 	N_("display right")},

	{"braille on/off",	N_("braille on/off")},
	{"speech on/off",	N_("speech on/off")},
	{"magnifier on/off",	N_("magnifier on/off")},
	{"braille monitor on/off", N_("braille monitor on/off")},

	{NULL,			NULL}
	
    };

typedef struct
{
    gchar *key;
    gchar *key_trans;
}CmdMapKeyList;

CmdMapKeyList key_list[] =
  {
    {"A",	N_("A")},
    {"B", 	N_("B")},
    {"C",	N_("C")},
    {"D",	N_("D")},
    {"E",	N_("E")},
    {"F",	N_("F")},
    {"G",	N_("G")},
    {"H",	N_("H")},
    {"I",	N_("I")},
    {"J",	N_("J")},
    {"K",	N_("K")},
    {"L",	N_("L")},
    {"M",	N_("M")},
    {"N",	N_("N")},
    {"O",	N_("O")},
    {"P",	N_("P")},
    {"Q",	N_("Q")},
    {"R",	N_("R")},
    {"S",	N_("S")},
    {"T",	N_("T")},
    {"U",	N_("U")},
    {"V",	N_("V")},
    {"W",	N_("W")},
    {"X",	N_("X")},
    {"Y",	N_("Y")},
    {"Z",	N_("Z")},
    {"F1",	N_("F1")},
    {"F2",	N_("F2")},
    {"F3",	N_("F3")},
    {"F4",	N_("F4")},
    {"F5",	N_("F5")},
    {"F6",	N_("F6")},
    {"F7",	N_("F7")},
    {"F8",	N_("F8")},
    {"F9",	N_("F9")},
    {"F10",	N_("F10")},
    {"F11",	N_("F11")},
    {"F12",	N_("F12")},
    {"0",	N_("0")},
    {"1",	N_("1")},
    {"2",	N_("2")},
    {"3",	N_("3")},
    {"4",	N_("4")},
    {"5",	N_("5")},
    {"6",	N_("6")},
    {"7",	N_("7")},
    {"8",	N_("8")},
    {"9",	N_("9")},
    {"apostrophe",	N_("apostrophe")},
    {"backslash", 	N_("backslash")},
    {"bracketleft",	N_("bracket left")},
    {"bracketright",	N_("bracket right")},
    {"comma",		N_("comma")},
    {"equal",		N_("equal")},
    {"grave",		N_("grave")},
    {"minus",		N_("minus")},
    {"period",		N_("period")},
    {"quoteleft",	N_("quote left")},
    {"semicolon",	N_("semicolon")},
    {"slash",		N_("slash")}
};


typedef enum
{
    KEY_PAD,
    USER_DEFINED
}TypeOfUsingList;

static struct {
    KeyItem 		*current_keyitem;
    GtkTreeIter 	current_iter;
    GSList		*current_list;
    GtkTreeView		*current_tree_view;
    gboolean		current_is_new;
    TypeOfUsingList    	current_type;
} cmd_current_item;
    
static GtkWidget	*w_user_keys = NULL;
static GtkWidget	*w_layer_key = NULL;
static GtkWidget	*w_command_list = NULL;
static GtkWidget	*w_command_map = NULL;

static GtkWidget	*tv_cmd_list;
static GtkWidget	*tv_key_pad;
static GtkWidget	*tv_user_def;

static GtkWidget	*cb_key_list;
static GtkWidget	*ck_alt,*ck_ctrl,*ck_shift;

static GtkWidget	*sp_layer_no;
static GtkWidget	*sp_key_no;

static GtkWidget	*bt_key_pad_remove;
static GtkWidget	*bt_key_pad_modify;

static GtkWidget	*bt_user_remove;
static GtkWidget	*bt_user_modify;

GSList 		*user_def_list = NULL;
GSList 		*key_pad_list = NULL;

static GSList 		*removed_items_list = NULL;
static gboolean		cmdui_changes = FALSE;

static void
cmdui_changes_set (gboolean state)
{
    cmdui_changes = state;
}

static gboolean
cmdui_changes_get (void)
{
    return cmdui_changes;
}

#define SIZE_OF_LAYER_KEY 6
#define NO_REPR_DIGIT	  2
#define POS_DIGIT_0   1
#define POS_DIGIT_1   2
#define POS_DIGIT_2   4
#define POS_DIGIT_3   5

gchar*
cmdui_get_modifiers_from_code (const gchar *str)
{
    gchar *delimit;
    gchar *tmp = NULL;
    gchar *rv = NULL;
    
    delimit = g_strrstr (str, KEY_NAME_SEPARATOR);
    
    if (!delimit)
	return NULL;    
	
    for (tmp = (gchar*)str ; *tmp != '-' && tmp ; tmp++)
    {
	gchar *tmp2 = NULL;

	if (tmp != str && rv)
	{
	    tmp2 = g_strconcat (KEY_SEPARATOR, rv, NULL);
	    g_free (rv);
	    rv = tmp2;
	}
	
	switch (*tmp)
	{
	    case 'A': tmp2 = g_strconcat (ALT_KEY, rv , NULL); break;
	    case 'S': tmp2 = g_strconcat (SHIFT_KEY, rv, NULL); break;
	    case 'C': tmp2 = g_strconcat (CONTROL_KEY, rv, NULL); break;
	    default:
		break;
	}
	
	g_free (rv);
	rv = tmp2;
    }
    
    return rv;
}


gchar*
cmdui_get_modifier_from_text (const gchar *str)
{
    gchar *delimit;
    gchar *rv = NULL;
    
    delimit = g_strrstr (str, KEY_SEPARATOR);
    
    if (!delimit)
	return NULL;    
	
    rv = g_strdup_printf ("%s%s%s-", 	MODIFIER_IS(str, ALT_KEY,"A"),
					MODIFIER_IS(str, CONTROL_KEY,"C"),
				        MODIFIER_IS(str, SHIFT_KEY,"S"));
    return rv;
}

gchar*
cmdui_get_text_from_code (const gchar *str)
{
    gint iter;
    gchar *delimit;
    gchar *tmp;
    
    sru_return_val_if_fail (str, NULL);

    if (str[0] == 'L' &&
	str[3] == 'K' &&
	strlen (str) == SIZE_OF_LAYER_KEY)
	return g_strdup_printf ("Layer%c%cKey%c%c", 
				str[POS_DIGIT_0], 
				str[POS_DIGIT_1], 
				str[POS_DIGIT_2], 
				str[POS_DIGIT_3]);

    delimit = g_strrstr (str, KEY_NAME_SEPARATOR);
    (const gchar*)tmp = str;
    if (delimit)
	tmp = delimit + 1;

    for (iter = 0 ; iter < G_N_ELEMENTS (key_list) ; iter++)
    {
	if (!strcmp (tmp, key_list[iter].key))
	{
	    gchar *rv   = NULL;
	    if (delimit)
	    {
		gchar *tmp2 = NULL;
		tmp2 = cmdui_get_modifiers_from_code (str);
		if (tmp2)
		    rv = g_strconcat (tmp2, KEY_SEPARATOR, key_list[iter].key_trans, NULL);
		else
		    rv = g_strdup (key_list[iter].key_trans);
		g_free (tmp2);
	    }
	    else
		rv = g_strdup (key_list[iter].key_trans);
	    return rv;
	}
    }	
	
    return g_strdup (str);
}

gchar*
cmdui_get_code_from_text (const gchar *str)
{
    gchar *tmp = NULL;
    gchar *delimit;
    gchar *tmp2;
    gint iter;

    
    sru_return_val_if_fail (str, NULL);
    
    if (!g_ascii_strncasecmp (str,"Layer", strlen ("Layer")) &&
        !g_ascii_strncasecmp (str + strlen("Layer") + NO_REPR_DIGIT,"Key", strlen ("Key")))
    {
	tmp = g_strdup_printf ("L%c%cK%c%c", *(str + strlen("Layer")),
					     *(str + strlen("Layer") + 1),
					     *(str + strlen("Layer") + 2 + strlen ("Key")),
					     *(str + strlen("Layer") + 3 + strlen ("Key")));
	return tmp;
    }
    
    delimit = g_strrstr (str, KEY_SEPARATOR);
    (const gchar*)tmp = str;
    
    if (delimit)
	tmp = delimit + 1;

    for (iter = 0 ; iter < G_N_ELEMENTS (key_list) ; iter++)
    {
	if (!strcmp (tmp, key_list[iter].key_trans))
	{
	    gchar *rv;
	    tmp2 = cmdui_get_modifier_from_text (str);
	    if (tmp2)
		rv = g_strconcat (tmp2, key_list[iter].key, NULL);
	    else
		rv = g_strdup (key_list[iter].key);
	    g_free (tmp2);
	    return rv;
	}
    }	
    
    return g_strdup (str);
}

static GSList*
cmdui_remove_keyitem_from_list (GSList 	*list,
				GtkTreeView *tree_view);

static void
cmdui_command_list_selection_add_func (GtkTreeModel 	*model,
		    		       GtkTreePath	*path,
				       GtkTreeIter	*iter,
				       gpointer		data)
{       
    gint pos;
    gchar *text = NULL; 

    sru_return_if_fail (GTK_IS_LIST_STORE (model));
    
    gtk_tree_model_get (model, 	 iter,
                    	CMD_COLUMN, &text,
                    	-1);
			    
    for (pos = 0 ; cmd_function[pos].descr ; pos++)
	if (!strcmp (_(cmd_function [pos].descr), text)) 
	    break;
	    
    g_free (text);
    
    if (cmd_function[pos].descr)
    {
	    (cmd_current_item.current_keyitem)->command_list = 
		g_slist_prepend ((cmd_current_item.current_keyitem)->command_list, 
				g_strdup (cmd_function [pos].name));
    }	
}


static gboolean
cmdui_command_list_set_changes (void)
{
    gchar *txt;
    GtkTreeModel *model = NULL;
    GtkTreeSelection *selection = 
	    gtk_tree_view_get_selection (GTK_TREE_VIEW (tv_cmd_list));	    

    sru_return_val_if_fail (cmd_current_item.current_keyitem, FALSE);	    
        		    
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (cmd_current_item.current_tree_view));

    (cmd_current_item.current_keyitem)->command_list =
	cmdconf_free_list_and_data ((cmd_current_item.current_keyitem)->command_list);

    gtk_tree_selection_selected_foreach (selection, 
    					cmdui_command_list_selection_add_func, 
					NULL);

    (cmd_current_item.current_keyitem)->command_list = 
        cmdconf_check_integrity ((cmd_current_item.current_keyitem)->command_list);
	    
	
    if (!g_slist_length ((cmd_current_item.current_keyitem)->command_list))
    {
	gn_show_message (_("No selected command!"));
	return FALSE;
    }

    txt = cmdconf_create_view_string ((cmd_current_item.current_keyitem)->command_list);
	
    sru_return_val_if_fail (txt, FALSE);
    sru_return_val_if_fail (GTK_IS_LIST_STORE (model), FALSE);
    
    gtk_list_store_set (GTK_LIST_STORE (model), 
			&cmd_current_item.current_iter, 
			FUNC_COLUMN, txt,
			-1 );
			
    (cmd_current_item.current_keyitem)->commands = txt;
	    
    cmdui_changes_set (TRUE);
    
    return TRUE;
}

static GtkListStore*
cmdui_populate_command_list_store_with_keys (GtkListStore *store)
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
cmdui_command_list_create_model (void)
{
    GtkListStore *store;
  
    store = gtk_list_store_new (NO_OF_CMD_COLUMNS, 
				G_TYPE_STRING);

    cmdui_populate_command_list_store_with_keys (store);
  
    return GTK_TREE_MODEL (store);
}

void 
cmdui_command_list_treeviews_set_current_value (void)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    GtkTreeIter      iter;
    gboolean 	     valid;

    if (!w_command_list) 
	return;
	
    sru_return_if_fail (cmd_current_item.current_keyitem);
    sru_return_if_fail ((cmd_current_item.current_keyitem)->key_code);
    
    model 	  = gtk_tree_view_get_model 	( GTK_TREE_VIEW (tv_cmd_list));
    selection     = gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_cmd_list));

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

    gtk_tree_selection_unselect_all (selection);	
    
    valid = gtk_tree_model_get_iter_first (model, &iter);
    
    if (!strcmp ((cmd_current_item.current_keyitem)->key_code, BLANK))
	return;
	
    while (valid)
    {
	gchar *key = NULL;
	gint   pos = 0;
	    
	if (GTK_IS_LIST_STORE (model))
	{
	    gtk_tree_model_get (model, 	&iter,
    				CMD_COLUMN, &key,
                    		-1);
	}
	    
	for (pos = 0 ; cmd_function[pos].descr ; pos++)
	    if (!strcmp (_(cmd_function[pos].descr), key)) break;
				
	g_free (key);
	    
	if (cmd_function[pos].descr)
	{
	    GSList *elem = NULL;
	    for (elem = (cmd_current_item.current_keyitem)->command_list ; elem ; elem = elem->next)
	    {
	        if (!strcmp (cmd_function[pos].name,
	    		    (gchar*)elem->data))
	    	{
		    gtk_tree_selection_select_iter (selection, &iter);
		    break;
		}
	    }
	}

	valid = gtk_tree_model_iter_next (model, &iter);	    
    }
}

static void
cmdui_command_list_restore_state (GtkTreeView  *tree_view,
				  GtkTreeIter	iter,
				  GSList	*list,
				  KeyItem 	*key_item,
				  gboolean	is_new,
				  TypeOfUsingList type)
{
    GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
    if (!is_new)
	return;

    if (GTK_IS_LIST_STORE (model))
    {
	gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    }
    
    if (cmdconf_check_if_item_exist (list, key_item->key_code))
    {
	list = cmdconf_remove_item_from_list (list, key_item->key_code);
    }
	    
    if (type == KEY_PAD)
	    key_pad_list  = list;
	else
	    user_def_list = list;

}

static void
cmdui_response_command_list_event (GtkDialog *dialog,
		    		   gint       response_id,
		    	  	   gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_OK)
    {
	if (!cmdui_command_list_set_changes ())
	{
	    cmdui_command_list_restore_state (cmd_current_item.current_tree_view,
					      cmd_current_item.current_iter,
					      cmd_current_item.current_list,
					      cmd_current_item.current_keyitem,
					      cmd_current_item.current_is_new,
					      cmd_current_item.current_type);
	}
    }	
    if (response_id == GTK_RESPONSE_CANCEL)
    {
	    cmdui_command_list_restore_state (cmd_current_item.current_tree_view,
					      cmd_current_item.current_iter,
					      cmd_current_item.current_list,
					      cmd_current_item.current_keyitem,
					      cmd_current_item.current_is_new,
					      cmd_current_item.current_type);
    }
    gtk_widget_hide ((GtkWidget*)dialog);	
}


static gint
cmdui_delete_emit_response_cancel (GtkDialog *dialog,
				   GdkEventAny *event,
				   gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}

static void
cmdui_command_list_activated (GtkTreeView       *tree_view,
                	      GtkTreePath       *path,
			      GtkTreeViewColumn *column)
{
    cmdui_response_command_list_event (GTK_DIALOG (w_command_list),
				       GTK_RESPONSE_OK,
				       NULL);
}

static gboolean
cmdui_search_equal_func (GtkTreeModel  *model,
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
cmdui_set_handlers_command_list (GladeXML *xml)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;
    GtkWidget		*bt_ok;
    
    w_command_list 	= glade_xml_get_widget (xml, "w_add_modify");
    tv_cmd_list		= glade_xml_get_widget (xml, "tv_cmd_list");
    bt_ok		= glade_xml_get_widget (xml, "okbutton1");
    
    g_signal_connect (w_command_list , "response",
		      G_CALLBACK (cmdui_response_command_list_event), NULL);
    g_signal_connect (w_command_list , "delete_event",
                      G_CALLBACK (cmdui_delete_emit_response_cancel), NULL);

    model = cmdui_command_list_create_model ();
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_cmd_list), model);

        
    g_signal_connect (tv_cmd_list, "row_activated", 
		      G_CALLBACK (cmdui_command_list_activated), 
		      model);

    
    gtk_tree_sortable_set_sort_column_id (  GTK_TREE_SORTABLE (model), 
					    CMD_COLUMN, 
					    GTK_SORT_ASCENDING);
	    
    g_object_unref (G_OBJECT (model));

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes   (_("Commands"),
    							cell_renderer,
							"text", CMD_COLUMN,
							NULL);							
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_cmd_list), column);
    gtk_tree_view_column_set_sort_column_id (column, CMD_COLUMN);

    gtk_tree_view_set_search_column (GTK_TREE_VIEW (tv_cmd_list), CMD_COLUMN);    
    gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (tv_cmd_list),
					cmdui_search_equal_func,
					NULL, NULL);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv_cmd_list), TRUE);
        
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv_cmd_list));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE); 

}

gboolean
cmdui_load_command_list (void)
{
    if (!w_command_list)
    {
	GladeXML *xml;
	xml = gn_load_interface ("User_Properties/user_properties.glade2", "w_add_modify");
	sru_return_val_if_fail (xml, FALSE);
	cmdui_set_handlers_command_list (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for ( GTK_WINDOW (w_command_list),
					   GTK_WINDOW (w_command_map));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_command_list), 
						TRUE);
    }
    else
        gtk_widget_show (w_command_list);
	
    
    cmdui_command_list_treeviews_set_current_value ();
    
    return TRUE;
}

/**************************************************************************/

GSList*
cmdui_add_item_to_list (GtkTreeView *tree_view, 
			GSList      *list,
			const gchar  *key_code);


static const gchar*
cmdui_get_key_keystring (const gchar *key)
{
    gint iter;
    
    for (iter = 0 ; iter < G_N_ELEMENTS (key_list) ; iter++)
    {
	if (!strcmp (key, key_list[iter].key_trans))
	    return key_list[iter].key;
    }
    
    return NULL;
}

static gchar*
cmdui_get_key (void)
{
    gboolean alt, shift, ctrl;
    gboolean line;
    gchar *key = NULL;
    gchar *rv  = NULL;
    
    alt 	= gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_alt));
    shift 	= gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_shift));
    ctrl 	= gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_ctrl));
    
    line = alt || shift || ctrl ;
    
    (const gchar*)key = cmdui_get_key_keystring (gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cb_key_list)->entry)));
    
    if (!key || strlen (key) == 0)
	return rv;
	
    rv = g_strconcat(MODIFIER(alt,"A"),
		     MODIFIER(ctrl,"C"),
		     MODIFIER(shift,"S"),
		     LINE(line),
		     STR(key) ,NULL);
    return rv;
}

static void
cmdui_set_key_value (void)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_alt), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_shift), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_ctrl), FALSE);
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (cb_key_list)->entry), "");    
}

static void
cmdui_response_user_keys_event (GtkDialog *dialog,
		    		gint       response_id,
		    		gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_OK) 
    {
	gchar *key_code = cmdui_get_key ();
	
	if (key_code)
	{
	    cmd_current_item.current_type = USER_DEFINED;
	    
	    user_def_list = cmdui_add_item_to_list (GTK_TREE_VIEW (tv_user_def), 
						    user_def_list,
						    key_code);
	    g_free (key_code);
	}
    }
    gtk_widget_hide ((GtkWidget*)dialog);
}


static void
cmdui_foreach_key (gpointer data,
		   gpointer user_data)
{
    g_free ((gchar*)data);
}
		
static void
cmdui_set_handlers_user_keys (GladeXML *xml)
{
    GList *g_key_list = NULL;
    gint iter;
    
    w_user_keys	= glade_xml_get_widget (xml, "w_user_keys");
    cb_key_list	= glade_xml_get_widget (xml, "cb_key_list");
    ck_alt	= glade_xml_get_widget (xml, "ck_alt");
    ck_ctrl	= glade_xml_get_widget (xml, "ck_ctrl");
    ck_shift	= glade_xml_get_widget (xml, "ck_shift");

    g_signal_connect (w_user_keys, "response",
		      G_CALLBACK (cmdui_response_user_keys_event), NULL);
    g_signal_connect (w_user_keys, "delete_event",
                      G_CALLBACK (cmdui_delete_emit_response_cancel), NULL);

    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (cb_key_list)->entry), FALSE);		      
    
    for (iter = 0 ; iter < G_N_ELEMENTS (key_list) ; iter++)
	g_key_list = g_list_append (g_key_list, g_strdup (key_list[iter].key_trans));
    
    gtk_combo_set_popdown_strings (GTK_COMBO (cb_key_list), g_key_list);
    
    g_key_list = g_list_first (g_key_list);
    g_list_foreach (g_key_list,(gpointer)cmdui_foreach_key, NULL);
    g_list_free (g_key_list);
}

gboolean
cmdui_load_user_keys (GtkWidget *parent_window)
{
    if (!w_user_keys)
    {
	GladeXML *xml;
	xml = gn_load_interface ("User_Properties/user_properties.glade2", "w_user_keys");
	sru_return_val_if_fail (xml, FALSE);
	cmdui_set_handlers_user_keys (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_user_keys),
				      GTK_WINDOW (parent_window));
    	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_user_keys), TRUE);
    }
    else
        gtk_widget_show (w_user_keys);
	    
    cmdui_set_key_value ();
    
    return TRUE;
}

/***************************************************************************/
static gchar*
cmdui_get_layer_key (void)
{
    gchar *str =
	g_strdup_printf ("L%02dK%02d",
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_layer_no)),
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_key_no)));
    return str;
}

static void
cmdui_response_layer_key_event (GtkDialog *dialog,
		    		gint       response_id,
		        	gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_OK) 
    {
	gchar *layer_key = cmdui_get_layer_key ();
	if (layer_key)
	{
	    cmd_current_item.current_type = KEY_PAD;
	    key_pad_list =
		    cmdui_add_item_to_list (GTK_TREE_VIEW (tv_key_pad), 
					    key_pad_list,
					    layer_key);

	    g_free (layer_key);
	}
    }
    gtk_widget_hide ((GtkWidget*)dialog);
}

		
static void
cmdui_set_handlers_layer_key (GladeXML *xml)
{
    w_layer_key = glade_xml_get_widget (xml, "w_layer_key");
    sp_layer_no	= glade_xml_get_widget (xml, "sp_layer_no");
    sp_key_no	= glade_xml_get_widget (xml, "sp_key_no");

    gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_layer_no), CMD_MIN_LAYER, CMD_MAX_LAYER);
    gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_key_no),   CMD_MIN_LAYER, CMD_MAX_LAYER);
    g_signal_connect (w_layer_key, "response",
		      G_CALLBACK (cmdui_response_layer_key_event), NULL);
    g_signal_connect (w_layer_key, "delete_event",
                      G_CALLBACK (cmdui_delete_emit_response_cancel), NULL);
}

gboolean
cmdui_load_layer_key (GtkWidget *parent_window)
{
    if (!w_layer_key)
    {
	GladeXML *xml;
	xml = gn_load_interface ("User_Properties/user_properties.glade2", "w_layer_key");
	sru_return_val_if_fail (xml, FALSE);
	cmdui_set_handlers_layer_key (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_layer_key),
				      GTK_WINDOW (parent_window));
    	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_layer_key), TRUE);
    }
    else
        gtk_widget_show (w_layer_key);
		
    return TRUE;
}




/***************************************************************************/

static GSList*
cmdui_main_window_free_removed_list (void)
{
    if (!removed_items_list) 
	return NULL;
    
    removed_items_list =
	cmdconf_free_list_and_data (removed_items_list);
	
    return removed_items_list;
}

static GSList*
cmdui_main_window_removed_list_add (GSList *list,
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


static void
cmdui_main_window_apply_set (void)
{
    removed_items_list =
	cmdconf_remove_items_from_gconf_list (removed_items_list);
    cmdui_main_window_free_removed_list ();

    cmdconf_user_def_set (user_def_list);
    cmdconf_key_cmd_list_set (user_def_list);
    cmdconf_key_pad_set (key_pad_list);
    cmdconf_key_cmd_list_set (key_pad_list);
    
    cmdconf_changes_end_event 	();
}

static void
cmdui_main_window_clear_treeviews (void)
{

    GtkTreeModel *model_ud = 
        gtk_tree_view_get_model (GTK_TREE_VIEW (tv_user_def));
    GtkTreeModel *model_kp = 
        gtk_tree_view_get_model (GTK_TREE_VIEW (tv_key_pad));
	
    gtk_list_store_clear (GTK_LIST_STORE (model_ud));
    gtk_list_store_clear (GTK_LIST_STORE (model_kp));
}


static GSList*
cmdui_remove_keyitem_from_list (GSList 	*list,
				GtkTreeView *tree_view)
{
    GtkTreeIter  iter;
    GtkTreeModel *model = gtk_tree_view_get_model ( tree_view );
    GtkTreeSelection *selection = gtk_tree_view_get_selection ( tree_view );	    
    GSList *root = list;
    GSList *elem = NULL;
    gchar *key = NULL;
    gchar *tmp = NULL;

    
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
	gn_show_message (_("No selected item to remove!"));
	return root;
    }
    
    if (GTK_IS_LIST_STORE (model))
    {
	gtk_tree_model_get (model, &iter,
        		    KEYS_COLUMN, &key,
            		    -1);
    }
	
    if (!key || strlen (key) == 0)
    {
	sru_message (_("Invalid selected item!"));
	g_free (key);
	return list;
    }
			
    tmp = cmdui_get_code_from_text (key);	
    g_free (key);
	
    if (!strcmp (tmp, BLANK))
	return root;
			
    for (elem = root; elem ; elem = elem->next)
	if (!strcmp (tmp, ((KeyItem*)elem->data)->key_code)) 
	break;
		
    g_free (tmp);	
	    
    if (!elem)
	return root;
    
    cmdui_changes_set (TRUE);
		
    root = g_slist_remove_link (root, elem);
		
    removed_items_list =
	    cmdui_main_window_removed_list_add (removed_items_list,
					       ((KeyItem*)elem->data)->key_code);
    elem = cmdconf_free_keyitem_list_item (elem);
		    
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

    return root;
}
				    

void
cmdui_main_window_user_remove_clicked (GtkWidget *button,
	        		       gpointer user_data)
{
    
    user_def_list = 
	cmdui_remove_keyitem_from_list (user_def_list,
				    	GTK_TREE_VIEW (tv_user_def));
}

void
cmdui_main_window_key_pad_remove_clicked (GtkWidget *button,
	        	    		  gpointer user_data)
{
    key_pad_list = 
	cmdui_remove_keyitem_from_list (key_pad_list,
				    	GTK_TREE_VIEW (tv_key_pad));
}


GSList*
cmdui_add_item_to_list (GtkTreeView *tree_view, 
			GSList      *list,
			const gchar  *key_code)
{
    KeyItem 	     *new_code  = NULL;
    gchar 	     *tmp = NULL;
    GtkTreeIter      iter;
    GtkTreeModel     *model;
    GtkTreeSelection *selection;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    if (!key_code || strlen (key_code) == 0)
	return list;

    if (cmdconf_check_if_item_exist (list, key_code))
    {
	gn_show_message (_("Invalid key or existent key."));
	return list;
    }
    
    new_code = cmdconf_new_keyitem ();
    sru_return_val_if_fail (new_code, list);
    new_code->key_code = g_strdup (key_code);

    /* append new key combination with NULL command in list */
    list = g_slist_prepend (list , new_code);	

    tmp  = cmdui_get_text_from_code (new_code->key_code);

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
	    		KEYS_COLUMN, tmp, 
			FUNC_COLUMN, "",
	    		-1);
    g_free (tmp);
    gtk_tree_selection_select_iter (selection, &iter);
    
    cmd_current_item.current_keyitem = new_code;
    cmd_current_item.current_iter = iter;
    cmd_current_item.current_list = list;
    cmd_current_item.current_tree_view = tree_view;
    cmd_current_item.current_is_new = TRUE;
    cmdui_changes_set (TRUE);
    /* load command list to add command to new key */
    cmdui_load_command_list ();
    return list;
}

GSList*
cmdui_main_window_modify_item_to_list (GtkTreeView *tree_view, 
				       GSList      *list)
{
    KeyItem 	     *new_code  = NULL;
    GtkTreeIter      iter;
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    
    GSList *elem = NULL;
    gchar *key = NULL;
    gchar *tmp = NULL;

    
    model 	= gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
    selection 	= gtk_tree_view_get_selection ( GTK_TREE_VIEW (tree_view));

    sru_return_val_if_fail (GTK_IS_LIST_STORE (model), list);

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
	gn_show_message (_("No selected item to modify!"));
    	return list;
    }	
    
    gtk_tree_model_get (model, 	     &iter,
            		KEYS_COLUMN, &key,
            		-1);
	/* select key csongor */
    if (!key || strlen (key) == 0)
    {
	sru_message (_("Invalid selected item!"));	    
	return list;
    }
			
    tmp = cmdui_get_code_from_text (key);
    g_free (key);
    for (elem = list; elem ; elem = elem->next)
    {
	if (!strcmp (((KeyItem*)elem->data)->key_code, tmp))
	{
	        new_code = elem->data;
	        break;
	}
    }		
    g_free (tmp);
		
    if (!new_code)
	return list;
	
    cmd_current_item.current_keyitem = new_code;
    cmd_current_item.current_iter = iter;
    cmd_current_item.current_list = list;
    cmd_current_item.current_tree_view = tree_view;
    cmd_current_item.current_is_new = FALSE;
    cmdui_load_command_list ();

    return list;
}

void
cmdui_main_window_key_pad_add_clicked (GtkWidget *button,
	        		       gpointer user_data)
{
    cmdui_load_layer_key ((GtkWidget*)user_data);
}

static void
cmdui_main_window_user_add_clicked (GtkWidget *button,
	            		    gpointer user_data)
{
    cmdui_load_user_keys ((GtkWidget*)user_data);
}


static void
cmdui_main_window_key_pad_modify_clicked (GtkWidget *button,
	        	    		  gpointer  user_data)
{
    cmd_current_item.current_type = KEY_PAD;
    key_pad_list = 
	cmdui_main_window_modify_item_to_list (GTK_TREE_VIEW (tv_key_pad), 
						key_pad_list);

}

static void
cmdui_main_window_user_modify_clicked (GtkWidget 	*button,
	            			gpointer 	user_data)
{
    cmd_current_item.current_type = USER_DEFINED;
    user_def_list =
	cmdui_main_window_modify_item_to_list (GTK_TREE_VIEW (tv_user_def), 
						user_def_list);

}

/**
 * cmdui_main_window_key_pad_row_activated_cb
 *
 * @treeview:
 * @path:
 * @column:
 *
 * Double click event on item in treeview.
 *
 * return:
**/
static void
cmdui_main_window_key_pad_row_activated_cb (GtkTreeView       *tree_view,
                			    GtkTreePath       *path,
					    GtkTreeViewColumn *column)
{
    cmd_current_item.current_type = KEY_PAD;
    key_pad_list = 
	cmdui_main_window_modify_item_to_list (GTK_TREE_VIEW (tv_key_pad), 
						key_pad_list);
}

/**
 * cmdui_main_window_user_def_row_activated_cb
 *
 * @treeview:
 * @path:
 * @column:
 *
 * Double click event on item in treeview.
 *
 * return:
**/
static void
cmdui_main_window_user_def_row_activated_cb (GtkTreeView      *tree_view,
                			    GtkTreePath       *path,
					    GtkTreeViewColumn *column)
{
    cmd_current_item.current_type = USER_DEFINED;
    user_def_list =
	cmdui_main_window_modify_item_to_list (GTK_TREE_VIEW (tv_user_def), 
						user_def_list);
}


static void
cmdui_selection_changed (GtkTreeSelection *selection,
			gpointer  user_data)
{
    gboolean 	sensitive;
    gint 	mode = (gint)user_data;
    
    sensitive = gtk_tree_selection_get_selected (selection, NULL, NULL);
    
    if (mode == USER_DEFINED)    
    {
	gtk_widget_set_sensitive (GTK_WIDGET (bt_user_remove), sensitive);
	gtk_widget_set_sensitive (GTK_WIDGET (bt_user_modify), sensitive);
    }
    else
    {
	gtk_widget_set_sensitive (GTK_WIDGET (bt_key_pad_remove), sensitive);
	gtk_widget_set_sensitive (GTK_WIDGET (bt_key_pad_modify), sensitive);
    }
}

/**
 * cmdui_main_window_populate_list_store_with_keys
 *
 * @store:
 * @list:
 *
 * Populate list store with elements from list.
 *
 * return: populated list strore.
**/
static GtkListStore*
cmdui_main_window_populate_list_store_with_keys (GtkListStore *store, 
						 GSList *list)
{
    GtkTreeIter iter;
    GSList *elem = NULL;
    
    if (!list) 
	return store;
    
    for (elem = list; elem ; elem = elem->next)
    {
	gchar *tmp = cmdui_get_text_from_code (((KeyItem*)elem->data)->key_code);
	gtk_list_store_append ( GTK_LIST_STORE (store), &iter);
	gtk_list_store_set (GTK_LIST_STORE (store), &iter, 
		    	    KEYS_COLUMN, tmp,
		    	    FUNC_COLUMN, ((KeyItem*)elem->data)->commands,
		    	    -1);
	g_free (tmp);
    }
    
    return store;
}

/**
 * cmdui_main_window_update_list_stores
 *
 * @user_def_list:
 * @key_pad_list:
 *
 * Updated the entries of tables.
 *
 * return:
**/
void
cmdui_main_window_update_list_stores (GSList *user_def_list,
				      GSList *key_pad_list)
{

    GtkTreeModel *model_ud = 
	gtk_tree_view_get_model (GTK_TREE_VIEW (tv_user_def));

    GtkTreeModel *model_kp = 
	gtk_tree_view_get_model (GTK_TREE_VIEW (tv_key_pad));
        
    if (user_def_list && GTK_IS_LIST_STORE (model_ud))
	cmdui_main_window_populate_list_store_with_keys (GTK_LIST_STORE (model_ud), 
							user_def_list);
					    
    if (key_pad_list && GTK_IS_LIST_STORE (model_kp))
	cmdui_main_window_populate_list_store_with_keys (GTK_LIST_STORE (model_kp), 
							key_pad_list);
    
}

/**
 * cmdui_main_window_create_model_list
 *
 * @list: list of elements with which will fill the table.
 *
 * Create a model for treeview and fill it.
 *
 * return: A new created treemodel.
**/
static GtkTreeModel*
cmdui_main_window_create_model_list (GSList *list)
{
    GtkListStore *store;
      
    store = gtk_list_store_new (NO_OF_KEYS_COLUMNS, 
				G_TYPE_STRING, 
				G_TYPE_STRING);
    cmdui_main_window_populate_list_store_with_keys (store, list);
    return GTK_TREE_MODEL (store);
}

/**
 * cmdui_main_window_response_event
 *
 * @dialog: Where the signal is emited.
 * @response_id: Signal id.
 * @user_data:
 *
 * Response event callback (Cancel, Help, Ok, Close)
 *
 * return:
**/
static void
cmdui_main_window_response_event (GtkDialog *dialog,
		    		  gint       response_id,
		    		  gpointer   user_data)
{
    switch (response_id)
    {
     case GTK_RESPONSE_OK: 
        {
	    cmdui_main_window_clear_treeviews ();
	    
	    if (cmdui_changes_get ())
		cmdui_main_window_apply_set ();
		
	    key_pad_list  = cmdconf_free_keyitem_list_item (key_pad_list);
	    user_def_list = cmdconf_free_keyitem_list_item (user_def_list);

	    gtk_widget_hide ((GtkWidget*)dialog);
	}
        break;
     case GTK_RESPONSE_APPLY:
        {
	    cmdui_main_window_apply_set ();
    
	    cmdui_changes_set (FALSE);
	}
        break;
    case GTK_RESPONSE_HELP:
	    gn_load_help ("gnopernicus-cm-prefs");
	break;	
    case GTK_RESPONSE_CANCEL:
    default:
            cmdui_main_window_clear_treeviews ();
		    
	    key_pad_list  = cmdconf_free_keyitem_list_item (key_pad_list);
	    user_def_list = cmdconf_free_keyitem_list_item (user_def_list);
	
	    cmdui_main_window_free_removed_list ();

            gtk_widget_hide ((GtkWidget*)dialog);
        break;
    }
}

/**
 * cmdui_main_window_set_handlers
 *
 * @xml: Glade pointer for UI tree.
 * @user_def_list:
 * @key_pad_list:
 *
 * Get pointer for widgets, set signals, construct tables structure.
 *
 * return:
**/
static void
cmdui_main_window_set_handlers (GladeXML *xml,
				GSList *user_def_list,
				GSList *key_pad_list)
{
    GtkTreeModel 	*model_user_def;
    GtkTreeModel 	*model_key_pad;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;
    
    w_command_map 	= glade_xml_get_widget (xml, "w_command_map");
    tv_key_pad		= glade_xml_get_widget (xml, "tv_layers");
    tv_user_def		= glade_xml_get_widget (xml, "tv_user_def");
    
    bt_user_remove	= glade_xml_get_widget (xml, "bt_user_remove");
    bt_user_modify	= glade_xml_get_widget (xml, "bt_user_modify");

    bt_key_pad_remove	= glade_xml_get_widget (xml, "bt_key_pad_remove");
    bt_key_pad_modify	= glade_xml_get_widget (xml, "bt_key_pad_modify");

    g_signal_connect (w_command_map, "response",
		      G_CALLBACK (cmdui_main_window_response_event), NULL);
    g_signal_connect (w_command_map, "delete_event",
                      G_CALLBACK (cmdui_delete_emit_response_cancel), NULL);

		      
    glade_xml_signal_connect (xml,"on_bt_user_remove_clicked",  
		GTK_SIGNAL_FUNC (cmdui_main_window_user_remove_clicked));
    glade_xml_signal_connect_data (xml,"on_bt_user_add_clicked",  	
		GTK_SIGNAL_FUNC (cmdui_main_window_user_add_clicked), 
		w_command_map);
    glade_xml_signal_connect (xml,"on_bt_user_modify_clicked",	
		GTK_SIGNAL_FUNC (cmdui_main_window_user_modify_clicked));
		
    glade_xml_signal_connect (xml,"on_bt_key_pad_remove_clicked",	
		GTK_SIGNAL_FUNC (cmdui_main_window_key_pad_remove_clicked));
    glade_xml_signal_connect_data (xml,"on_bt_key_pad_add_clicked",		
		GTK_SIGNAL_FUNC (cmdui_main_window_key_pad_add_clicked), 
		w_command_map);
    glade_xml_signal_connect (xml,"on_bt_key_pad_modify_clicked",		
		GTK_SIGNAL_FUNC (cmdui_main_window_key_pad_modify_clicked));

    model_user_def 	= cmdui_main_window_create_model_list (user_def_list);
    model_key_pad 	= cmdui_main_window_create_model_list (key_pad_list);

    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_key_pad),  model_key_pad);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_user_def), model_user_def);
    
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_key_pad));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    g_signal_connect (selection, "changed",
                      G_CALLBACK (cmdui_selection_changed), (gpointer)KEY_PAD);
    cmdui_selection_changed (selection, (gpointer)KEY_PAD);

    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv_user_def));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);    
    g_signal_connect (selection, "changed",
                      G_CALLBACK (cmdui_selection_changed), (gpointer)USER_DEFINED);
    cmdui_selection_changed (selection, (gpointer)USER_DEFINED);
        
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model_key_pad), 
					    KEYS_COLUMN, 
					    GTK_SORT_ASCENDING);

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model_user_def), 
					    KEYS_COLUMN, 
					    GTK_SORT_ASCENDING);

    g_signal_connect (tv_key_pad, "row_activated", 
		      G_CALLBACK (cmdui_main_window_key_pad_row_activated_cb), 
		      model_key_pad);
    g_signal_connect (tv_user_def, "row_activated", 
		      G_CALLBACK (cmdui_main_window_user_def_row_activated_cb), 
		      model_user_def);

	    
    g_object_unref (G_OBJECT (model_key_pad));
    g_object_unref (G_OBJECT (model_user_def));
    
    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes   (_("Layer Keys"),
    							cell_renderer,
							"text", KEYS_COLUMN,
							NULL);	
    gtk_tree_view_column_set_sort_column_id (column, KEYS_COLUMN);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_key_pad), column);
    column = gtk_tree_view_column_new_with_attributes   (_("Commands"),
    							cell_renderer,
							"text", FUNC_COLUMN,
							NULL);	
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_key_pad),column);
    
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (tv_key_pad), KEYS_COLUMN);    
    gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (tv_key_pad),
					cmdui_search_equal_func,
					NULL, NULL);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv_key_pad), TRUE);

        
    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes   (_("User Keys"),
    							cell_renderer,
							"text", KEYS_COLUMN,
							NULL);
    gtk_tree_view_column_set_sort_column_id (column, KEYS_COLUMN);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_user_def),column);
    column = gtk_tree_view_column_new_with_attributes   (_("Commands"),
    							cell_renderer,
							"text", FUNC_COLUMN,
							NULL);    
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_user_def), column);

    gtk_tree_view_set_search_column (GTK_TREE_VIEW (tv_user_def), KEYS_COLUMN);    
    gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (tv_user_def),
					cmdui_search_equal_func,
					NULL, NULL);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv_user_def), TRUE);
        
}

/**
 * cmdui_load_user_properties
 *
 * @parent_window: 
 *
 * Load main UI for command map options.
 *
 * return: TRUE if succes
**/
gboolean
cmdui_load_user_properties (GtkWidget *parent_window)
{
    user_def_list = cmdconf_user_def_get (user_def_list);
    key_pad_list = cmdconf_key_pad_get (key_pad_list);

    /* check if the window is already created*/
    if (!w_command_map)
    {
	GladeXML *xml;
	xml = gn_load_interface ("User_Properties/user_properties.glade2", "w_command_map");
	sru_return_val_if_fail (xml, FALSE);
	cmdui_main_window_set_handlers (xml, 
					user_def_list, 
					key_pad_list);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_command_map),
				      GTK_WINDOW (parent_window));
    	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_command_map), TRUE);
    }
    else
    {
        cmdui_main_window_update_list_stores (user_def_list,
					      key_pad_list);
        gtk_widget_show (w_command_map);
    }

    cmdui_changes_set (TRUE);
		    
    return TRUE;
}
