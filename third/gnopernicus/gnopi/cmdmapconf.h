/* cmdmapconf.h
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

#ifndef _BRLPROPCONF_
#define _BRLPROPCONF_

#include <glib.h>

#define BLANK 	""

#define MAX_FUNCTIONS 5
typedef struct
{
    gchar *key;
    gchar *functions[MAX_FUNCTIONS];
}CmdFctType;

enum
{
    CMD_COLUMN,
    NO_OF_CMD_COLUMNS
};

typedef enum
{
    KEYS_COLUMN,
    FUNC_COLUMN,
    NO_OF_KEYS_COLUMNS
} KeyListType;

typedef struct
{
    gchar *name;
    gchar *descr;
}CmdMapFunctionsList;

typedef struct
{
    gchar  	*key_code;
    GSList 	*command_list;
    gchar  	*commands;	
} KeyItem;

gboolean 	cmdconf_gconf_client_init 	(void);
void		cmdconf_terminate 		(void);

void 		cmdconf_default_list 		(gboolean force);
void		cmdconf_set_defaults_for_table  (const gchar 	*list_path,
						CmdFctType	*table,
						gboolean 	force);

void 		cmdconf_remove_lists_from_gconf (void);
GSList*		cmdconf_remove_items_from_gconf_list (GSList *list);
void		cmdconf_changes_end_event 	(void);

GSList*		cmdconf_check_integrity 	(GSList *list);
gchar*		cmdconf_create_view_string 	(GSList *list);
gboolean	cmdconf_check_if_item_exist 	(GSList *list,
						const gchar *item);

KeyItem*	cmdconf_new_keyitem 		(void);
GSList*		cmdconf_free_keyitem_list_item  (GSList *list);
GSList*		cmdconf_free_list_and_data 	(GSList *list);
GSList*		cmdconf_remove_item_from_list   (GSList *list,
					         gchar *item);


GSList*		cmdconf_braille_key_get 	(GSList *list);
GSList*		cmdconf_key_pad_get 		(GSList *list);
GSList*		cmdconf_user_def_get 		(GSList *list);

gboolean	cmdconf_user_def_set 		(GSList *list);
gboolean	cmdconf_key_pad_set 		(GSList *list);
gboolean	cmdconf_braille_key_set 	(GSList *list);

gboolean	cmdconf_key_cmd_list_set 	(GSList *list);
#endif
