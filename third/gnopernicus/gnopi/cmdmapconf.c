/* cmdmapconf.c
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
#include "cmdmapconf.h"
#include "SRMessages.h"
#include "gnopiconf.h"
#include <gnome.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include "srintl.h"
#include "libsrconf.h"
#define CMD_STR(X) (X == NULL ? "" : X)


extern CmdMapFunctionsList cmd_function[];

CmdFctType cmd_key_cmds[] = 
{
	/*______________< MAGNIFIER>________________*/

	/*_______________<LAYER 6>__________________*/
	{"L06K04", {"decrease x scale", 	NULL}},
	{"L06K06", {"increase x scale", 	NULL}},
	{"L06K02", {"decrease y scale", 	NULL}},
	{"L06K08", {"increase y scale", 	NULL}},
	{"L06K05", {"lock xy scale", 		NULL}},

	{"L06K07", {"invert on/off", 		NULL}},
	{"L06K09", {"smoothing toggle",		NULL}},
	{"L06K10", {"load magnifier defaults",	NULL}},

	{"L06K01", {"cursor on/off", 		NULL}},
	{"L06K03", {"crosswire on/off",		NULL}},
	{"L06K11", {"cursor mag on/off",	NULL}},

	{"L06K12", {"automatic panning on/off",	NULL}},
	{"L06K13", {"mouse tracking toggle",	NULL}},
	/*_______________</LAYER 6>_________________*/

	/*_______________< LAYER 7>_________________*/
	{"L07K09", {"cursor toggle", 		NULL}},

	{"L07K01", {"cursor on/off", 		NULL}},
	{"L07K03", {"crosswire on/off",		NULL}},
	{"L07K11", {"cursor mag on/off",	NULL}},

	{"L07K08", {"increase cursor size", 	NULL}},
	{"L07K02", {"decrease cursor size", 	NULL}},

	{"L07K06", {"increase crosswire size", 	NULL}},
	{"L07K04", {"decrease crosswire size", 	NULL}},
	{"L07K05", {"crosswire clip on/off",	NULL}},

	{"L07K13", {"mouse tracking toggle",	NULL}},

	{"L07K10", {"load magnifier defaults",	NULL}},
	/*________________</LAYER 7>_________________*/

	/*_______________</MAGNIFIER>________________*/


	{"L08K07", {"decrease pitch", 		NULL}},
	{"L08K09", {"increase pitch", 		NULL}},
	{"L08K08", {"default pitch", 		NULL}},
	{"L08K04", {"decrease rate", 		NULL}},
	{"L08K06", {"increase rate", 		NULL}},
	{"L08K05", {"default rate", 		NULL}},
	{"L08K01", {"decrease volume", 		NULL}},
	{"L08K03", {"increase volume", 		NULL}},
	{"L08K02", {"default volume", 		NULL}},
	{"L08K10", {"speech default", 		NULL}},	
	{"L08K13", {"pause/resume",		NULL}},

/*			<NAVIGATION - LAYER 0>			*/	
	{"L00K08", {"goto parent", 		NULL}},	
	{"L00K02", {"goto child", 		NULL}},
	{"L00K04", {"goto previous", 		NULL}},	
	{"L00K06", {"goto next", 		NULL}},	
	{"L00K05", {"repeat last", 		NULL}},	
	{"L00K10", {"toggle tracking mode",	NULL}},	
	{"L00K07", {"goto title", 		NULL}},	
	{"L00K09", {"goto menu", 		NULL}},	
	{"L00K01", {"goto toolbar", 		NULL}},	
	{"L00K03", {"goto statusbar", 		NULL}},
	{"L00K11", {"widget surroundings", 	NULL}},	
	{"L00K12", {"goto focus",	 	NULL}},
	{"L00K15", {"goto first",	 	NULL}},	
	{"L00K13", {"change navigation mode", 	NULL}},	
	{"L00K14", {"goto last",	 	NULL}},	

	{"L01K12", {"goto caret",	 	NULL}},	
	{"L01K01", {"attributes at caret", 	NULL}},
	{"L01K02", {"watch current object", 	NULL}},
	{"L01K03", {"unwatch all objects", 	NULL}},

	{"L03K05", {"flat review",	 	NULL}},	
	{"L03K04", {"window hierarchy",	 	NULL}},
	{"L03K07", {"read whole window",	NULL}},
	{"L03K06", {"detailed informations", 	NULL}},	
	{"L03K02", {"do default action", 	NULL}},	
	{"L03K03", {"window overview", 		NULL}},	
	{"L03K10", {"find next", 		NULL}},	
	{"L03K01", {"find set",			NULL}},


	{"L05K01", {"mouse left press",		NULL}},
	{"L05K02", {"mouse left click", 	NULL}},
	{"L05K03", {"mouse left release",	NULL}},
	{"L05K04", {"mouse right press",	NULL}},
	{"L05K05", {"mouse right click", 	NULL}},
	{"L05K06", {"mouse right release",	NULL}},
	{"L05K07", {"mouse middle press", 	NULL}},
	{"L05K08", {"mouse middle click", 	NULL}},
	{"L05K09", {"mouse middle release",	NULL}},
	{"L05K10", {"mouse goto current", 	NULL}},

	{"L09K01", {"char left", 		NULL}},
	{"L09K03", {"char right", 		NULL}},
	{"L09K04", {"display left", 		NULL}},
	{"L09K06", {"display right", 		NULL}},
	
	{"L10K01", {"braille on/off",		NULL}},
	{"L10K02", {"speech on/off",		NULL}},
	{"L10K03", {"magnifier on/off",		NULL}},
	{"L10K04", {"braille monitor on/off",	NULL}},
	
	{NULL,	   {NULL}}
};
    
CmdFctType cmd_usr_cmds[]=
{
/*	{"ACS-S",  {"widget surroundings", 	NULL}},	*/
/*	{"Control_L",{"shutup", 		NULL}},*/
	{NULL,	   {NULL}}
};

extern GConfClient *gnopernicus_client;
extern GSList 	   *key_pad_list;
extern GSList 	   *user_def_list;

static void 
cmdconf_free_func  (gpointer data,
		    gpointer user_data)
{
    g_free((gchar*)data);
    data = NULL;
}

GSList*
cmdconf_free_list_and_data (GSList *list)
{
    if (!list) 
	return NULL;
    g_slist_foreach (list, cmdconf_free_func, NULL);
    g_slist_free (list);
    list = NULL;
    return list;
}

static void
cmdconf_free_keyitem (gpointer data,
		      gpointer user_data)
{
    KeyItem *item = (KeyItem *)data;

    if (!item) 
	return;
    
    g_free (item->commands);
    g_free (item->key_code);
    item->command_list = cmdconf_free_list_and_data (item->command_list);
    g_free (item);    
    item = NULL;
}

GSList*
cmdconf_free_keyitem_list_item (GSList *list)
{
    if (!list) 
	return NULL;
    g_slist_foreach (list, cmdconf_free_keyitem, NULL);
    g_slist_free (list);
    list = NULL;
    return list;
}

GSList*
cmdconf_remove_item_from_list (GSList *list,
				gchar *item)
{
    GSList *elem = NULL;
    
    if (!list || 
	!item) 
	return list;
    
    for (elem = list; elem ; elem = elem->next)
    {
	if (!strcmp (((KeyItem*)elem->data)->key_code, item)) 
	{
	    list = g_slist_remove_link (list, elem);
	}
    }
    
    elem = cmdconf_free_keyitem_list_item (elem);
    
    return list;
}

KeyItem*
cmdconf_new_keyitem (void)
{
    KeyItem *item = NULL;
    
    item = ( KeyItem *) g_new0 ( KeyItem, 1);
    
    if (!item)	
	sru_error (_("Unable to allocate memory."));
    
    item->key_code 	= NULL;
    item->commands 	= g_strdup (BLANK);
    item->command_list  = NULL;
    
    return item ;
}

gchar*
cmdconf_create_view_string (GSList *list)
{
    GSList *elem = list;
    gchar *txt = NULL;
    
    if (!list) 
	return g_strdup (BLANK);
    
    while (elem)
    {
	gchar *data = elem->data;
	gint  pos = 0;
	gchar *tmp_txt = NULL;
	
	if (!strcmp (data,BLANK))
	{
	    g_free (txt);
	    return g_strdup (BLANK);
	}
	
	for (pos = 0 ; cmd_function[pos].name ; pos++)
	{
	    if (! strcmp ((gchar*)elem->data,
			cmd_function[pos].name)) 
		    break;
	}
	
	if (cmd_function[pos].descr)
	{
	    if (txt) 
		tmp_txt = g_strdup_printf ("%s <%s>", CMD_STR(txt), 
					    _(cmd_function[pos].descr));
	    else 
		tmp_txt = g_strdup_printf ("<%s>", _(cmd_function[pos].descr));
	    g_free (txt);
	    txt = tmp_txt;
	}
	    
	elem = elem->next;
    }
    return txt;
}

/* Check if the item exist in KeyItem list*/
gboolean
cmdconf_check_if_item_exist (GSList *list, const gchar *item)
{
    GSList *elem = NULL;
    
    if (!list || !item) 
	return FALSE;
    
    for (elem = list; elem ; elem = elem->next)
	if (!strcmp (((KeyItem*)elem->data)->key_code, item)) 
	    return TRUE;
    
    return FALSE;
}

GSList*
cmdconf_check_integrity (GSList *list)
{
    GSList *elem = list;
    
    if (!list) 
	return NULL;
    
    while (elem)
    {
	gchar *txt = (gchar*)elem->data;
	gint iter = 0;
	
	if (txt != NULL && !strcmp (txt, BLANK))
	{
	    list = cmdconf_free_list_and_data (list);
	    return NULL;
	}

	for (iter = 0 ; cmd_function[iter].name ; iter++)
	    if (!strcmp (txt, cmd_function [iter].name)) 
		break;
		    	
	if (cmd_function[iter].name)
	{
	    elem = elem->next;
	}
	else
	{
	    GSList *remove = elem;
	    elem = elem->next;
	    list = g_slist_remove_link (list, remove);
	    g_free (remove->data);
    	    g_slist_free (remove);
	}	    
    }
	
    return list;
}

void
cmdconf_set_defaults_for_table (const gchar 	*list_path,
				CmdFctType	*table,
				gboolean 	force)
{
    GConfValue *val = NULL;
    gchar *path;
    
    sru_return_if_fail (list_path);
	
    path = g_strdup_printf ("%s%s", CONFIG_PATH, list_path);
    val = gconf_client_get (gnopernicus_client, path, NULL);

    if (!val || force)
    {
	GSList *list = NULL;
	gint i;
	for (i = 0 ; table[i].key ; i++)
	{
	    GSList *list_tmp = NULL;
	    gchar *gconf_path = NULL;
	    gint iter_func = 0;
	    list = g_slist_append (list, g_strdup (table[i].key));
	    for (iter_func = 0 ; table[i].functions[iter_func] ; iter_func ++)
	    {
		if (table[i].functions[iter_func])
		    list_tmp = g_slist_append (list_tmp, table[i].functions[iter_func]);
	    }
	
	    if (list_tmp)
	    {
		GConfValue *val = NULL;
		gconf_path = g_strdup_printf ("%s/%s/key_list", 
					    CONFIG_PATH SRC_KEY_SECTION,
					    table[i].key);
		val = gconf_client_get (gnopernicus_client, gconf_path, NULL);
		if (!val || force)
		    gnopiconf_set_list (list_tmp, GCONF_VALUE_STRING, gconf_path);
		else
		    gconf_value_free (val);

		g_slist_free (list_tmp);
		g_free (gconf_path);	
		list_tmp = NULL;
	    }	    
	}    	
	
	gnopiconf_set_list (list, GCONF_VALUE_STRING, path);
			
	list = cmdconf_free_list_and_data (list);
    }
    else
	gconf_value_free (val);

    g_free (path);
        
}

void 
cmdconf_default_list (gboolean force)
{
    cmdconf_set_defaults_for_table (CMDMAP_KEY_PAD_LIST_PATH,  cmd_key_cmds, force);
    cmdconf_set_defaults_for_table (CMDMAP_USER_DEF_LIST_PATH, cmd_usr_cmds, force);
}

void 
cmdconf_remove_lists_from_gconf (void)
{
    GConfValueType type;
    GConfChangeSet *change_set;
    GError	   *error = NULL;
    GSList *list = 
	gnopiconf_get_list_with_default (CONFIG_PATH CMDMAP_KEY_PAD_LIST_PATH, 
					NULL, 
					&type);
    change_set = gconf_change_set_new ();
    gconf_change_set_ref (change_set);
    
    
    while (list)
    {
	if (strcmp ((gchar*)list->data, BLANK))
	{
	    gchar *path;
	    path = g_strdup_printf ("%s/%s/key_list", 
				    CONFIG_PATH SRC_KEY_SECTION,
				    (gchar*)list->data);
	    gconf_change_set_unset (change_set, path);
	    g_free (path);
	}
	list = list->next;
    }
    
    list = 
	gnopiconf_get_list_with_default (CONFIG_PATH CMDMAP_USER_DEF_LIST_PATH, 
					NULL, 
					&type);
    while (list)
    {
	if (strcmp ((gchar*)list->data, BLANK))
	{

	    gchar *path;
	    path = g_strdup_printf ("%s/%s/key_list", 
				    CONFIG_PATH SRC_KEY_SECTION,
				    (gchar*)list->data);
	    gconf_change_set_unset (change_set, path);
	    g_free (path);
	}
	
	list = list->next;
    }
    
    gconf_client_commit_change_set (gnopernicus_client, change_set, TRUE, &error);
    if (error)
    {
	sru_warning (_("Change commit error."));
	sru_warning (_(error->message));
	g_error_free (error);
    }
    
    gconf_change_set_clear (change_set);
    gconf_change_set_unref (change_set);

    key_pad_list = cmdconf_free_keyitem_list_item (key_pad_list);
    user_def_list = cmdconf_free_keyitem_list_item (user_def_list);
}

GSList*
cmdconf_remove_items_from_gconf_list (GSList *list)
{
    GConfChangeSet *change_set;
    GError *error = NULL;
    GSList *elem = NULL;
    
    if (!list) 
	return NULL;
    
    change_set = gconf_change_set_new ();
    gconf_change_set_ref (change_set);
    
    for (elem = list ; elem ; elem = elem->next)
    {
	if (strcmp ((gchar*)elem->data, BLANK))
	{
	    gchar *path;
	    path = g_strdup_printf ("%s/%s/key_list", 
				    CONFIG_PATH SRC_KEY_SECTION,
				    (gchar*)elem->data);
	    gconf_change_set_unset (change_set, path);
	    g_free (path);
	}
    }
    
    gconf_client_commit_change_set (gnopernicus_client, change_set, TRUE, &error);
    if (error)
    {
	sru_warning (_("Change commit error."));
	sru_warning (_(error->message));
	g_error_free (error);
    }
    gconf_change_set_clear (change_set);
    gconf_change_set_unref (change_set);
    return list;
}


void
cmdconf_changes_end_event (void)
{
    gboolean bval = TRUE;
    bval = gnopiconf_get_bool_with_default 	
		(CONFIG_PATH SRC_KEY_SECTION CMDMAP_CHANGES_END, 
		bval);
    bval = !bval;
    gnopiconf_set_bool (bval, CONFIG_PATH SRC_KEY_SECTION CMDMAP_CHANGES_END);
}


/****************************************************************/
gboolean 
cmdconf_gconf_client_init (void)
{
    sru_return_val_if_fail (gnopiconf_client_add_dir (CONFIG_PATH COMMAND_MAP_PATH), FALSE);

    cmdconf_default_list (FALSE);
    cmdconf_changes_end_event ();
	    
    return TRUE;
}

void
cmdconf_terminate (void)
{
    if (gconf_client_dir_exists (gnopernicus_client, CONFIG_PATH COMMAND_MAP_PATH, NULL))
	gconf_client_remove_dir (gnopernicus_client, CONFIG_PATH COMMAND_MAP_PATH, NULL);
	
    cmdconf_free_keyitem_list_item (key_pad_list);
    cmdconf_free_keyitem_list_item (user_def_list);
}


/*****************************************************************/
/*			Set Methods				 */
/* Verify and correct if it is need a correction in gconf path */
static gchar* 
cmdconf_correct_path (const gchar *path)
{
    gchar *rv = NULL;
    gchar *iter;
    
    sru_return_val_if_fail (path, NULL);
    
    rv = g_strdup (path);
    iter = rv;
    
    while (*iter != '\0')
    {
	switch (*iter)
	{
	 case ' ':*iter = '_';break;
	 case '\\':*iter = '_';break;
	 default:
	    break;
	}
	iter ++;
    }
    
    return rv;
}

gboolean
cmdconf_key_cmd_list_set (GSList *list)
{
    GSList *elem = NULL;
        
    if (!list)
	return FALSE;

    for (elem = list ; elem ; elem = elem->next)
    {
	gchar 	*gconf_path 	= NULL;
	KeyItem *item 		= elem->data;
	gchar 	*item_command 	= cmdconf_correct_path (item->key_code);
	
	gconf_path = g_strdup_printf ("%s/%s/key_list", 
				      CONFIG_PATH SRC_KEY_SECTION, 
				      item_command);
	gnopiconf_set_list (item->command_list, 
			    GCONF_VALUE_STRING, 
			    gconf_path);
			    
	g_free (gconf_path);
	g_free (item_command);
    }
    return TRUE;
}


static gboolean
cmdconf_key_set (GSList *list,
		const gchar *path)
{
    GSList *tmp = NULL;
    GSList *iter = NULL;
    gboolean rv;

    sru_return_val_if_fail (path, FALSE);

    for (iter = list ; iter ; iter = iter->next)
    {
	tmp = g_slist_insert_sorted (tmp, 
				    g_strdup (((KeyItem*)iter->data)->key_code),
				    (GCompareFunc)strcmp);				    
    }
    rv = gnopiconf_set_list (tmp, GCONF_VALUE_STRING, path);
    tmp = cmdconf_free_list_and_data (tmp);
    
    return rv;
}

gboolean
cmdconf_user_def_set (GSList *list)
{
    return cmdconf_key_set (list, 
			     CONFIG_PATH CMDMAP_USER_DEF_LIST_PATH);
}

gboolean
cmdconf_key_pad_set (GSList *list)
{
    return cmdconf_key_set (list, 
			     CONFIG_PATH CMDMAP_KEY_PAD_LIST_PATH);
}

gboolean
cmdconf_braille_key_set (GSList *list)
{
    return cmdconf_key_set (list, 
			     CONFIG_PATH CMDMAP_BRAILLE_KEYS_LIST_PATH);
}

/*Get Methods*/
static GSList*
cmdconf_key_list_get (GSList *gconf_list,
		      GSList *key_list)
{
    GConfValueType type;
    GSList *tmp_list = NULL;

    if (!gconf_list) 
	return NULL;
    
    tmp_list = gconf_list;
    
    while (tmp_list)
    {
	KeyItem *item 	    = NULL;
	gchar   *gconf_path = NULL;
	
	item = cmdconf_new_keyitem ();
	
	if (tmp_list->data)
	{
	    item->key_code = tmp_list->data;
	    tmp_list->data = NULL;
		
	    if (!strcmp (item->key_code,BLANK)) continue;
		
	    gconf_path = g_strdup_printf ("%s/%s/key_list", 
					CONFIG_PATH SRC_KEY_SECTION,
					item->key_code);
	    item->command_list = 
		gnopiconf_get_list_with_default (gconf_path, 
						NULL, 
						&type);
	    g_free (gconf_path);
	    
	    item->command_list = 
		cmdconf_check_integrity (item->command_list);
		
	    if (item->command_list)
	    {
		item->commands = 
		    cmdconf_create_view_string (item->command_list);
		key_list = g_slist_append (key_list, item);
	    }
	    else
		cmdconf_free_keyitem ((gpointer) item, NULL);
		
	}
	
	tmp_list = tmp_list->next;
    }
        
    return key_list;
}

GSList*
cmdconf_braille_key_get (GSList *list)
{
    GConfValueType type;
    GSList *key_list = 
	gnopiconf_get_list_with_default (CONFIG_PATH CMDMAP_BRAILLE_KEYS_LIST_PATH, 
					NULL, &type);
					
    list = cmdconf_free_keyitem_list_item (list);
    list = cmdconf_key_list_get (key_list, list);
    g_slist_free (key_list);
    
    return list;
}


GSList*
cmdconf_key_pad_get (GSList *list)
{
    GConfValueType type;
    GSList *key_list = 
	gnopiconf_get_list_with_default (CONFIG_PATH CMDMAP_KEY_PAD_LIST_PATH, 
					NULL, &type);

    list = cmdconf_free_keyitem_list_item (list);
    list = cmdconf_key_list_get (key_list, list);    
    g_slist_free (key_list);
    
    return list;
}

GSList*
cmdconf_user_def_get (GSList *list)
{
    GConfValueType type;
    GSList *key_list = 
	gnopiconf_get_list_with_default (CONFIG_PATH CMDMAP_USER_DEF_LIST_PATH, 
					NULL, &type);
    list = cmdconf_free_keyitem_list_item (list);
    list = cmdconf_key_list_get (key_list, list);    
    g_slist_free (key_list);
    
    return list;
}

