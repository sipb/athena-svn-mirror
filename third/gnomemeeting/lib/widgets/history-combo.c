/*  history-combo.c
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 *           Miguel Rodríguez <migrax@terra.es>
 *           De Michele Cristiano
 */

/*
 *                         history-combo.c -  description
 *                         ------------------------------
 *   begin                : Mon Jun 17 2002 (original: Thu Nov 22 2001)
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create a new history combo box GObject.
 */


#include "history-combo.h"

#define GM_HISTORY_ENTRIES_NUM 10

static void gm_history_combo_class_init   (GmHistoryComboClass *klass);
static void gm_history_combo_init         (GmHistoryCombo *combo);
static void gm_history_combo_construct    (GmHistoryCombo *combo);
static void gm_history_combo_finalize     (GObject *object);
static void gm_history_combo_set_property (GObject *object,
                                           guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec);
static void gm_history_combo_get_property (GObject *object,
                                           guint prop_id,
                                           GValue *value,
                                           GParamSpec *pspec);
GSList     *g_slist_from_glist            (GList *list);
GList      *g_list_from_gslist            (GSList *list);

struct GmHistoryComboPrivate
{
  char *key;
};

enum
{
  PROP_0,
  PROP_KEY,
};

static GtkComboClass *parent_class = NULL;

GType
gm_history_combo_get_type (void)
{
  static GType gm_history_combo_type = 0;
  
  if (gm_history_combo_type == 0)
  {
    static const GTypeInfo our_info =
    {
      sizeof (GmHistoryComboClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_history_combo_class_init,
      NULL,
      NULL,
      sizeof (GmHistoryCombo),
      0,
      (GInstanceInitFunc) gm_history_combo_init
    };
    
    gm_history_combo_type = g_type_register_static (GTK_TYPE_COMBO,
                                                    "GmHistoryCombo",
                                                    &our_info,
						    (GTypeFlags) 0);
  }
  
  return gm_history_combo_type;
}

static void
gm_history_combo_class_init (GmHistoryComboClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  parent_class = g_type_class_peek_parent (klass);
  
  object_class->finalize = gm_history_combo_finalize;
  
  object_class->set_property = gm_history_combo_set_property;
  object_class->get_property = gm_history_combo_get_property;

  g_object_class_install_property (object_class,
                                   PROP_KEY,
                                   g_param_spec_string ("key",
                                                        "Key",
                                                        "GConf key to store history",
                                                        "", /* default value */
                                                        (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY)));
}


static void
gm_history_combo_init (GmHistoryCombo *combo)
{
  combo->priv = g_new0 (GmHistoryComboPrivate, 1);
  combo->contact_list = NULL;
}


static void
gm_history_combo_construct (GmHistoryCombo *combo)
{
  GConfClient *client;
  
  GSList *contact_gconf;
  const char *key;

  g_object_get (G_OBJECT (combo), "key", &key, NULL);

  client = gconf_client_get_default ();
  contact_gconf = 
    gconf_client_get_list (client, key, GCONF_VALUE_STRING, NULL);
  
  combo->contact_list = g_list_from_gslist (contact_gconf);
  g_slist_free (contact_gconf);

  if (combo->contact_list != NULL)
  {    
    gtk_combo_set_popdown_strings (GTK_COMBO (combo), combo->contact_list);
  }
  
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), ""); 
}


static void
gm_history_combo_finalize (GObject *object)
{
  GmHistoryCombo *combo;
  
  g_return_if_fail (object != NULL);
  g_return_if_fail (GM_IS_HISTORY_COMBO (object));
  
  combo = GM_HISTORY_COMBO (object);
  
  g_return_if_fail (combo->priv != NULL);
  
  g_free (combo->priv->key);
  g_free (combo->priv);
  
  while (combo->contact_list != NULL)
  {
    g_free (combo->contact_list->data);
    combo->contact_list = combo->contact_list->next;
  }
  
  g_list_free (combo->contact_list);
  
  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static void
gm_history_combo_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  GmHistoryCombo *combo = GM_HISTORY_COMBO (object);
  
  switch (prop_id)
  {
  case PROP_KEY:
    combo->priv->key = g_strdup (g_value_get_string (value));
    gm_history_combo_construct (combo);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}


static void 
gm_history_combo_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
  GmHistoryCombo *combo = GM_HISTORY_COMBO (object);
  
  switch (prop_id)
  {
  case PROP_KEY:
    g_value_set_string (value, combo->priv->key);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}


GtkWidget *
gm_history_combo_new (const char *key)
{
  GmHistoryCombo *combo;
  
  combo = 
    GM_HISTORY_COMBO (g_object_new (GM_TYPE_HISTORY_COMBO, "key", key, NULL));
  
  g_return_val_if_fail (combo->priv != NULL, NULL);
  
  return GTK_WIDGET (combo);
}


GSList *
g_slist_from_glist (GList *list)
{
  GSList *new_list = NULL;
  GList  *iter;

    if (list != NULL)
    {
      for (iter = list; iter != 0; iter = g_list_next (iter))
      {
        new_list = g_slist_prepend (new_list, iter->data);
      }
      return new_list;
    }
    else
      return NULL;
}


GList *
g_list_from_gslist (GSList *list)
{
  GList *new_list = NULL;
  GSList  *iter;

  if (list != NULL)
  {
    for (iter = list; iter != 0; iter = g_slist_next (iter))
    {
      new_list = g_list_prepend (new_list, iter->data);
    }
    return new_list;
  }
  else
    return NULL;
}


/**
 * gm_history_combo_add_entry:
 *
 * @key is the gconf key used to store the history.
 * 
 * Add a new entry to the history combo and saves it
 * in the GConf database.
 **/
void 
gm_history_combo_add_entry (GmHistoryCombo *combo, 
                            const char *key,
                            const char *new_entry)
{
  GConfClient  *client;
  GSList       *contact_gconf;
  GList        *iter;
  char         *entry_content;
  unsigned int  max_entries;
  
  /* we make a dup, because the entry text will change */
  entry_content = g_strdup (gtk_entry_get_text 
			    (GTK_ENTRY (GTK_COMBO (combo)->entry)));  
  
  /* if it is an empty entry_content, return */
  if (!strcmp (entry_content, "")) 
  {
    g_free (entry_content);
    return;
  }
  
  /* We read the max_contact setting */
  client = gconf_client_get_default ();
  max_entries = GM_HISTORY_ENTRIES_NUM;

  if (combo->contact_list) 
  {
    for (iter = combo->contact_list; iter != 0; iter = g_list_next (iter)) 
    {
      if (!strcasecmp ((gchar *) iter->data, entry_content)) 
      {
	/* If we find already an existing entry we remove it, and add it
	   back later to the front of the list */
	combo->contact_list = g_list_delete_link (combo->contact_list, iter);
	break;
      }
    }
  }
  
  /* this will not store a copy of entry_content, but entry_content itself */
  combo->contact_list = g_list_prepend (combo->contact_list, entry_content);
  
  contact_gconf = g_slist_from_glist (combo->contact_list);
  gconf_client_set_list (client, key, GCONF_VALUE_STRING, contact_gconf, NULL);
  g_slist_free (contact_gconf);

  gm_history_combo_update (combo);  
  
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), entry_content);
}


void 
gm_history_combo_update (GmHistoryCombo *combo)
{
  GConfClient  *client;
  GList        *last_item;
  unsigned int  max_entries;

  client = gconf_client_get_default ();
  max_entries = GM_HISTORY_ENTRIES_NUM;
 
  /* this is a while loop as people dynamically can change the number of
   * max_contacts via GConf */
  while (g_list_length (combo->contact_list) > max_entries)
  {
    last_item = g_list_last (combo->contact_list);
    combo->contact_list = g_list_remove (combo->contact_list, last_item->data);
    g_free (last_item->data);
  }     

  if (combo->contact_list != NULL)
  {    
    gtk_combo_set_popdown_strings (GTK_COMBO (combo), combo->contact_list);
  }
}

