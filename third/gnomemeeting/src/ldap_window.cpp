
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         ldap_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */

#include "../config.h"

#include "ldap_window.h"
#include "gconf_widgets_extensions.h"
#include "gnomemeeting.h"
#include "ils.h"
#include "menu.h"
#include "callbacks.h"
#include "misc.h"
#include "dialog.h"
#include "stock-icons.h"


/* Declarations */
extern GtkWidget *gm;

struct GmEditContactDialog_ {

  GtkWidget *dialog;
  GtkWidget *name_entry;
  GtkWidget *url_entry;
  GtkWidget *speed_dial_entry;
  
  GtkListStore *groups_list_store;

  int selected_groups_number;
  gchar *old_contact_url;
};
typedef struct GmEditContactDialog_ GmEditContactDialog;


/* Callbacks */
static void filter_option_menu_changed (GtkWidget *,
					gpointer);

/* Callbacks: Drag and drop management */
static gboolean dnd_drag_motion_cb (GtkWidget *,
				    GdkDragContext *,
				    int,
				    int,
				    guint,
				    gpointer);

static void dnd_drag_data_received_cb (GtkWidget *,
				       GdkDragContext *,
				       int,
				       int,
				       GtkSelectionData *,
				       guint,
				       guint,
				       gpointer);

static void dnd_drag_data_get_cb (GtkWidget *,
				  GdkDragContext *,
				  GtkSelectionData *,
				  guint,
				  guint,
				  gpointer);

/* Callbacks: Operations on a contact */
static void groups_list_store_toggled (GtkCellRendererToggle *cell,
				       gchar *path_str,
				       gpointer data);

static void new_contact_cb (GtkWidget *,
			    gpointer);

static void edit_contact_cb (GtkWidget *,
			     gpointer);

static GmEditContactDialog *addressbook_edit_contact_dialog_new (const char *,
								 const char *,
								 const char *,
								 const char *);

static gboolean addressbook_edit_contact_valid (GmEditContactDialog *);

static void delete_contact_from_group_cb (GtkWidget *,
					  gpointer);

								
/* Callbacks: Operations on contact sections */
static void copy_url_to_clipboard_cb (GtkWidget *,
				      gpointer);

static void new_contact_section_cb (GtkWidget *,
				    gpointer);

static void modify_contact_section_cb (GtkWidget *,
				       gpointer);

static void contact_section_changed_cb (GtkTreeSelection *,
					gpointer);

static gint contact_section_clicked_cb (GtkWidget *,
					GdkEventButton *,
					gpointer);

/* Callbacks: Misc */
static void delete_cb (GtkWidget *,
		       gpointer);

static void call_user_cb (GtkWidget *,
			  gpointer);

static void contact_section_activated_cb (GtkTreeView *,
					  GtkTreePath *,
					  GtkTreeViewColumn *);

static void refresh_server_content_cb (GtkWidget *,
				       gpointer);


/* Local functions: Operations on a contact or on a contact section */
static gboolean is_contact_member_of_group (GMURL,
					    const char *);

static gboolean is_contact_member_of_addressbook (GMURL);

static gboolean is_contact_section_member_of_addressbook (const char *,
							  BOOL);

static GSList *find_contact_in_group_content (const char *,
					      GSList *);

static void add_contact_to_group (const char *,
				  const char *,
				  const char *,
				  const char *,
				  const char *);

static void delete_contact_from_group (const char *,
				       const char *);

static void add_contact_section (const char *,
				 BOOL);

static void delete_contact_section (const char *,
				    gboolean);

static gboolean get_selected_contact_info (gchar ** = NULL,
					   gchar ** = NULL,
					   gchar ** = NULL,
					   gchar ** = NULL,
					   gboolean * = NULL);

static void get_contact_info_from_url (GMURL url,
				       gchar **name,
				       gchar **speeddial);
				       
static gchar *escape_contact_section (const char *,
				      BOOL = TRUE);

/* Misc */
static void notebook_page_destroy (gpointer data);

static void edit_dialog_destroy (gpointer data);

static gint speed_dials_compare (gconstpointer,
				 gconstpointer);


/* COMMON NOTICE
 *
 * A contact section is either a group name with local contacts or
 * a server name with remote contacts.
 *
 * The group names are stored in the gconf key CONTACTS_KEY "groups_list" and
 * the server names are stored in the gconf key
 * CONTACTS_KEY "ldap_servers_list". Both are stored in a gconf_escaped way.
 * The case matters, but 2 servers and 2 groups with the same case are
 * considered as identical.
 *
 * The content of groups are stored in the key CONTACTS_GROUPS_KEY followed
 * by the group name. The group name is also in the escaped form, but in lower
 * case. If you have another key with a different case, it will be ignored.
 *
 * You have in general 2 type of functions, those ending with _cb that are
 * the GTK callbacks, displaying confirmation dialogs and such, and the
 * corresponding functions, not ending with _cb, that directly manipulate
 * the gconf keys.
 */


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user changes the filter
 *                 type in an ILS page.
 * BEHAVIOR     :  If "Find all" is selected, unsensitive the search entry.
 * PRE          :  data = the search entry.
 */
static void
filter_option_menu_changed (GtkWidget *menu,
			    gpointer data)
{
  guint item_index;
  GtkWidget *active_item;

  active_item = gtk_menu_get_active (GTK_MENU (menu));
  item_index = g_list_index (GTK_MENU_SHELL (GTK_MENU (menu))->children, 
			     active_item);
 
  if (item_index == 0)
    gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
  else
    gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
}


/* DESCRIPTION  :  This callback is called when the user moves the drag.
 * BEHAVIOR     :  Draws a rectangle around the groups in which the user info
 *                 can be dropped.
 * PRE          :  /
 */
static gboolean
dnd_drag_motion_cb (GtkWidget *tree_view,
		    GdkDragContext *context,
		    int x,
		    int y,
		    guint time,
		    gpointer data)		     
{
  GtkWidget *src_widget = NULL;
  GtkTreeModel *src_model = NULL;
  GtkTreeSelection *src_selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;

  gchar *group_name = NULL;
  gchar *contact_url = NULL;

  GmLdapWindow *lw = NULL;
  GmCallsHistoryWindow *chw = NULL;
  
  GValue value =  {0, };
  GtkTreeIter iter;

  lw = GnomeMeeting::Process ()->GetLdapWindow ();
  chw = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  
  src_widget = gtk_drag_get_source_widget (context);
  src_model = gtk_tree_view_get_model (GTK_TREE_VIEW (src_widget));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

  /* The source can be either the addressbook OR the calls history */
  if (src_model == GTK_TREE_MODEL (chw->given_calls_list_store)
      || src_model == GTK_TREE_MODEL (chw->received_calls_list_store)
      || src_model == GTK_TREE_MODEL (chw->missed_calls_list_store)) {

    src_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (src_widget));

    if (gtk_tree_selection_get_selected (src_selection, &src_model, &iter))
      gtk_tree_model_get (GTK_TREE_MODEL (src_model), &iter,
			  2, &contact_url, -1);
  }
  else
    get_selected_contact_info (NULL, NULL, &contact_url, NULL, NULL);


  /* Get the url field of the contact info from the source GtkTreeView */
  if (contact_url) {

    
    /* See if the path in the destination GtkTreeView corresponds to a valid
       row (ie a group row, and a row corresponding to a group the user
       doesn't belong to */
    if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (tree_view),
					   x, y, &path, NULL)) {

      if (gtk_tree_model_get_iter (model, &iter, path)) {
	    
	gtk_tree_model_get_value (model, &iter, 
				  COLUMN_CONTACT_SECTION_NAME, &value);
	group_name = (gchar *) g_value_get_string (&value);

	/* If the user doesn't belong to the selected group and if
	   the selected row corresponds to a group and not a server */
	if (gtk_tree_path_get_depth (path) >= 2 &&
	    gtk_tree_path_get_indices (path) [0] >= 1 
	    && group_name && contact_url &&
	    !is_contact_member_of_group (GMURL (contact_url), group_name)) {
    
	  gtk_tree_view_set_drag_dest_row (GTK_TREE_VIEW (tree_view),
					   path,
					   GTK_TREE_VIEW_DROP_INTO_OR_AFTER);
	}

	g_value_unset (&value);
	
	gtk_tree_path_free (path);
	gdk_drag_status (context, GDK_ACTION_COPY, time);
      }
    } 
  }
  else
    return false;

  g_free (contact_url);
  
  return true;
}


/* DESCRIPTION  :  This callback is called when the user has released
 *                 the drag.
 * BEHAVIOR     :  Adds the user gconf key of the group where the drop
 *                 occured.
 * PRE          :  /
 */
static void
dnd_drag_data_received_cb (GtkWidget *tree_view,
			   GdkDragContext *context,
			   int x,
			   int y,
			   GtkSelectionData *selection_data,
			   guint info,
			   guint time,
			   gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;

  GtkTreeIter iter;

  gchar **contact_info = NULL;
  gchar *group_name = NULL;
  gchar *gconf_key = NULL;

  GSList *group_content = NULL;

  GValue value = {0, };
 

  /* Get the path at the current position in the destination GtkTreeView
     so that we know to what group it corresponds. Once we know the
     group, we can update the gconf key of that group with the data received */
  if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (tree_view),
					 x, y, &path, NULL)) {

    if (gtk_tree_path_get_depth (path) >= 2 &&
	gtk_tree_path_get_indices (path) [0] >= 1) {

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

      if (gtk_tree_model_get_iter (model, &iter, path)) {
      
	gtk_tree_model_get_value (model, &iter, 
				  COLUMN_CONTACT_SECTION_NAME, &value);
	group_name = escape_contact_section (g_value_get_string (&value));

	if (group_name && selection_data && selection_data->data) {

	  contact_info = g_strsplit ((char *) selection_data->data, "|", 0);

	  if (contact_info [1] &&
	      !is_contact_member_of_group (GMURL (contact_info [1]),
					   g_value_get_string (&value))) {

	    gconf_key = 
	      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, 
			       (char *) group_name);
	    
	    group_content = gconf_get_string_list (gconf_key);
	    
	    group_content =
	      g_slist_append (group_content, (char *) selection_data->data);
	    
	    gconf_set_string_list (gconf_key, group_content);
	  
	    g_slist_free (group_content);
	    g_free (gconf_key);
	  }

	  g_strfreev (contact_info);
	}
	
	g_value_unset (&value);

	g_free (group_name);
      }
    }
    
    gtk_tree_path_free (path);
  } 
}
  

/* DESCRIPTION  :  This callback is called when the user has released the drag.
 * BEHAVIOR     :  Puts the required data into the selection_data, we put
 *                 name, the url fields and the speed-dial.
 * PRE          :  data = the type of the page from where the drag occured :
 *                 CONTACTS_GROUPS or CONTACTS_SERVERS.
 */
static void
dnd_drag_data_get_cb (GtkWidget *tree_view,
		      GdkDragContext *dc,
		      GtkSelectionData *selection_data,
		      guint info,
		      guint t,
		      gpointer data)
{
  gchar *contact_name = NULL;
  gchar *contact_url = NULL;
  gchar *contact_speed_dial = NULL;
  gchar *drag_data = NULL;


  if (get_selected_contact_info (NULL, &contact_name,
				 &contact_url, &contact_speed_dial, NULL)
      && contact_name && contact_url) {
      
    if (contact_speed_dial)
      drag_data = g_strdup_printf ("%s|%s|%s", contact_name, contact_url, contact_speed_dial);
    else
      drag_data = g_strdup_printf ("%s|%s", contact_name, contact_url);
    
    gtk_selection_data_set (selection_data, selection_data->target, 
			    8, (const guchar *) drag_data,
			    strlen (drag_data));
    g_free (drag_data);
  }

  g_free (contact_name);
  g_free (contact_url);
  g_free (contact_speed_dial);
}


/* DESCRIPTION  :  This callback is called when the user toggles a group in
 *                 the groups list in the popup permitting to edit
 *                 the properties of a contact.
 * BEHAVIOR     :  Update the toggles for that group, updates the number
 *                 of selected groups field of the GmEditContactDialog given
 *                 as parameter.
 * PRE          :  data = a valid GmEditContactDialog.
 */
static void
groups_list_store_toggled (GtkCellRendererToggle *cell,
			   gchar *path_str,
			   gpointer data)
{
  GmEditContactDialog *edit_dialog = (GmEditContactDialog *) data;
  GtkTreeIter iter;
  GtkTreeModel *model = GTK_TREE_MODEL (edit_dialog->groups_list_store);
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  
  gboolean member_of_group = false;

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &member_of_group, -1);
  
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0,
		      !member_of_group, -1);

  if (member_of_group)
    edit_dialog->selected_groups_number--;
  else
    edit_dialog->selected_groups_number++;
}


/* DESCRIPTION  :  This callback is called when the user chooses to create a
 *                 new contact
 * BEHAVIOR     :  Opens a popup, with empty entries
 */
static void
new_contact_cb (GtkWidget *widget,
		gpointer data)
{
  gchar *contact_section = NULL;
  
  get_selected_contact_info (&contact_section, NULL, NULL, NULL);
  gnomemeeting_addressbook_edit_contact_dialog (contact_section,
						NULL, NULL, NULL);
  g_free (contact_section);
}


/* DESCRIPTION  :  This callback is called when the user chooses to edit the
 *                 info of a contact from a group or to add an user.
 * BEHAVIOR     :  Opens a popup, and save the modified info.
 * PRE          :  If data = 1, then we add a new user, no need to see 
 *                 if something is selected and needs to be edited.
 */
static void
edit_contact_cb (GtkWidget *widget,
		 gpointer data)
{
  gchar *contact_section = NULL;
  gchar *contact_name = NULL;
  gchar *contact_url = NULL;
  gchar *contact_speed_dial = NULL;

  get_selected_contact_info (&contact_section, &contact_name, &contact_url, &contact_speed_dial, NULL);

  // perhaps the contact is already known?
  if(contact_speed_dial == NULL)
    get_contact_info_from_url (GMURL (contact_url), NULL, &contact_speed_dial);
	
  gnomemeeting_addressbook_edit_contact_dialog (contact_section, contact_name, contact_url, contact_speed_dial);
  
  g_free (contact_section);
  g_free (contact_name);
  g_free (contact_url);
  g_free (contact_speed_dial);
}


void
gnomemeeting_addressbook_edit_contact_dialog (gchar *contact_url)
{
  gchar *contact_name = NULL;
  gchar *contact_speed_dial = NULL;

  get_contact_info_from_url (GMURL (contact_url),
			     &contact_name,
			     &contact_speed_dial);
			     
  gnomemeeting_addressbook_edit_contact_dialog ("",
						contact_name,
						contact_url,
						contact_speed_dial);

  g_free (contact_name);
  g_free (contact_speed_dial);
}


void  
gnomemeeting_addressbook_edit_contact_dialog (gchar *contact_section,
					      gchar *contact_name,
					      gchar *contact_url,
					      gchar *contact_speed_dial)
{
  gboolean valid_answer = false;
  gboolean selected = false;
  int result = 0;
  GmEditContactDialog *edit_dialog = NULL;
  gchar *group_name = NULL;
  const char *name_entry_text = NULL;
  const char *url_entry_text = NULL;
  const char *speed_dial_entry_text = NULL;
  GtkTreeIter iter;
 
  edit_dialog =
    addressbook_edit_contact_dialog_new (contact_section,
					 contact_name,
					 contact_url,
					 contact_speed_dial);
  
  while (!valid_answer) {
    
    result = gtk_dialog_run (GTK_DIALOG (edit_dialog->dialog));
    
    switch (result) {
      
    case GTK_RESPONSE_ACCEPT:

      valid_answer = addressbook_edit_contact_valid (edit_dialog);

      if (valid_answer) {
	
	name_entry_text =
	  gtk_entry_get_text (GTK_ENTRY (edit_dialog->name_entry));
	url_entry_text =
	  gtk_entry_get_text (GTK_ENTRY (edit_dialog->url_entry));
	speed_dial_entry_text =
	  gtk_entry_get_text (GTK_ENTRY (edit_dialog->speed_dial_entry));
      
	/* Determine the groups where we want to add the contact */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (edit_dialog->groups_list_store), &iter)) {
	
	  do {
	  
	    gtk_tree_model_get (GTK_TREE_MODEL (edit_dialog->groups_list_store), &iter, 0, &selected, 1, &group_name, -1);
	  
	    if (group_name) {

	      if (selected)
		add_contact_to_group (name_entry_text,
				      url_entry_text,
				      speed_dial_entry_text,
				      edit_dialog->old_contact_url,
				      group_name);
	      else
		delete_contact_from_group (url_entry_text, group_name);
	      }

	    valid_answer = true;
	    
	    g_free (group_name);
	  
	  } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (edit_dialog->groups_list_store), &iter));

	}
      }
      break;


    case GTK_RESPONSE_REJECT:
      valid_answer = true;
      break;
      
    }
  }

  
  if (edit_dialog->dialog) 
    gtk_widget_destroy (edit_dialog->dialog);
}


/* DESCRIPTION  :  Called when the EditContactDialog gets destroyed.
 * BEHAVIOR     :  Frees the data when the EditContactDialog is destroyed.
 * PRE          :  data = valid pointer to a GmEditContactDialog.
 */
static void
edit_dialog_destroy (gpointer data)
{
  GmEditContactDialog *edit_dialog = (GmEditContactDialog *) data;

  if (data) {
    
    g_free (edit_dialog->old_contact_url);
    delete (edit_dialog);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Compares 2 entries following their speed dials. An entry
 *                 is "Username" + "|" + "Speed Dial". It will permit to sort
 *                 a slist following their speed dials.
 * PRE          :  /
 */
static gint
speed_dials_compare (gconstpointer ent1,
		     gconstpointer ent2)
{
  gint result = 0;
  gchar **couple1 = NULL;
  gchar **couple2 = NULL;

  if (ent1)
    couple1 = g_strsplit ((gchar *) ent1, "|", 0);
  if (ent2)
    couple2 = g_strsplit ((gchar *) ent2, "|", 0);

  if (couple1 [1] && couple2 [1])
    result = strcmp (couple1 [1], couple2 [1]);

  g_strfreev (couple1);
  g_strfreev (couple2);
  
  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates and display the EditContactDialog with the provided
 *                 default values.
 * PRE          :  /
 */
static GmEditContactDialog*
addressbook_edit_contact_dialog_new (const char *contact_section,
				     const char *contact_name,
				     const char *contact_url,
				     const char *contact_speed_dial)
{
  GmWindow *gw = NULL;
  GmEditContactDialog *edit_dialog = NULL;
  
  GtkWidget *scroll = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *tree_view = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeIter iter;
  
  GSList *groups_list = NULL;
  GSList *groups_list_iter = NULL;

  gchar *label_text = NULL;
  gchar *group_name = NULL;
  gboolean selected = false;

  
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  edit_dialog = new (GmEditContactDialog);
  memset (edit_dialog, 0, sizeof (GmEditContactDialog));
  
  /* Create the dialog to easily modify the info of a specific contact */
  edit_dialog->dialog =
    gtk_dialog_new_with_buttons (_("Edit the Contact Information"), 
				 GTK_WINDOW (gw->ldap_window),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (edit_dialog->dialog),
				   GTK_RESPONSE_ACCEPT);
  g_object_set_data_full (G_OBJECT (edit_dialog->dialog), "data", edit_dialog,
			  edit_dialog_destroy);
  
  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  edit_dialog->name_entry = gtk_entry_new ();
  if (contact_name)
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->name_entry), contact_name);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), edit_dialog->name_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (edit_dialog->name_entry),
				   true);

  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("URL:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);
  
  edit_dialog->url_entry = gtk_entry_new ();
  gtk_widget_set_size_request (GTK_WIDGET (edit_dialog->url_entry), 300, -1);
  if (contact_url) {
    
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->url_entry), contact_url);
    edit_dialog->old_contact_url = g_strdup (contact_url);
  }
  else
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->url_entry),
			GMURL ().GetDefaultURL ());
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), edit_dialog->url_entry, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (edit_dialog->url_entry),
				   true);

  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Speed Dial:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  edit_dialog->speed_dial_entry = gtk_entry_new ();
  if (contact_speed_dial)
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->speed_dial_entry),
			contact_speed_dial);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), edit_dialog->speed_dial_entry,
		    1, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (edit_dialog->speed_dial_entry),
				   true);

  
  /* The list store that contains the list of possible groups */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Groups:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  frame = gtk_frame_new (NULL);
  edit_dialog->groups_list_store =
    gtk_list_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING);
  tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (edit_dialog->groups_list_store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
  
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
						     "active", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (groups_list_store_toggled), 
		    gpointer (edit_dialog));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
						     "text", 1, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scroll), tree_view);
  gtk_container_add (GTK_CONTAINER (frame), scroll);
  gtk_widget_set_size_request (GTK_WIDGET (frame), -1, 90);
  
  gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  
  /* Populate the list store with available groups */
  groups_list = gconf_get_string_list (CONTACTS_KEY "groups_list");
  groups_list_iter = groups_list;
  
  while (groups_list_iter) {
    
    if (groups_list_iter->data) {

      group_name =
	gconf_unescape_key ((char *) groups_list_iter->data, -1);

      selected =
	((contact_url
	 && is_contact_member_of_group (GMURL (contact_url), group_name))
	 || !strcasecmp (group_name, contact_section));
      
      gtk_list_store_append (edit_dialog->groups_list_store, &iter);
      gtk_list_store_set (edit_dialog->groups_list_store, &iter,
			  0, selected,
			  1, group_name, -1);

      if (selected)
	edit_dialog->selected_groups_number++;

      g_free (group_name);
    }

    groups_list_iter = g_slist_next (groups_list_iter);
  }
  g_slist_free (groups_list);

  
  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (edit_dialog->dialog)->vbox), table,
		      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (edit_dialog->dialog);

  return edit_dialog;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if the EditContactDialog contains valid fields,
 *                 or false if it is not the case. A popup is displayed with
 *                 the error message in that case.
 * PRE          :  /
 */
static gboolean
addressbook_edit_contact_valid (GmEditContactDialog *edit_dialog)
{
  const char *name_entry_text = NULL;
  const char *url_entry_text = NULL;
  const char *speed_dial_entry_text = NULL;

  GMURL other_speed_dial_url;
  GMURL entry_url;
  GMURL old_entry_url;
  
  name_entry_text = gtk_entry_get_text (GTK_ENTRY (edit_dialog->name_entry));
  url_entry_text = gtk_entry_get_text (GTK_ENTRY (edit_dialog->url_entry));
  entry_url = GMURL (url_entry_text);
  speed_dial_entry_text =
    gtk_entry_get_text (GTK_ENTRY (edit_dialog->speed_dial_entry));
  if (edit_dialog->old_contact_url)
    old_entry_url = GMURL (edit_dialog->old_contact_url);
    
  /* If there is no name or an empty url, display an error message
     and exit */
  if (!strcmp (name_entry_text, "") || entry_url.IsEmpty ()) {

    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid user name or URL"), _("Please provide a valid name and URL for the contact you want to add to the address book."));
    return false;
  }


  /* If the user selected no groups, display an error message and exit */
  if (edit_dialog->selected_groups_number == 0) {

    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid group"), _("You have to select a group to which you want to add your contact."));
    return false;
  }


  /* If we can find another url for the same speed dial, display an error
     message and exit */
  other_speed_dial_url =
    gnomemeeting_addressbook_get_url_from_speed_dial (speed_dial_entry_text);

  if (!other_speed_dial_url.IsEmpty () && !entry_url.IsEmpty ()
      && other_speed_dial_url != entry_url
      && other_speed_dial_url != old_entry_url) {
		
    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid speed dial"), _("Another contact with the same speed dial already exists in the address book."));

    return false;
  }


  /* If the user is adding a new user, and there is already an identical
     url OR if the user modified an existing user to an user having
     an identical url, display a warning and exit */
  if (url_entry_text && is_contact_member_of_addressbook (entry_url)
      && (old_entry_url != entry_url)) {
    
    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid URL"), _("Another contact with the same URL already exists in the address book."));
    return false;
  }

  return true;
}


/* DESCRIPTION  :  This callback is called when the user chooses to delete the
 *                 a contact from a group.
 * BEHAVIOR     :  Removes the user from the group if the user confirms to do
 *                 in the confirm dialog.
 * PRE          :  /
 */
static void
delete_contact_from_group_cb (GtkWidget *widget,
			      gpointer data)
{
  GmWindow *gw = NULL;
  GtkWidget *dialog = NULL;

  gchar *confirm_msg = NULL;
  gchar *contact_url = NULL;
  gchar *contact_name = NULL;
  gchar *contact_section = NULL;

  int result = 0;
  gboolean is_group;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  /* Check that a contact is selected in a group */
  if (get_selected_contact_info (&contact_section, &contact_name,
				 &contact_url, NULL, &is_group)
      && is_group) {

    confirm_msg =
      g_strdup_printf (_("Are you sure you want to remove %s [%s] from this group?"), contact_name, contact_url);
    dialog =
      gtk_message_dialog_new (GTK_WINDOW (gw->ldap_window),
			      GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			      GTK_BUTTONS_YES_NO, confirm_msg);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				     GTK_RESPONSE_YES);
    gtk_widget_show_all (dialog);

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (result) {

    case GTK_RESPONSE_YES:

      delete_contact_from_group (contact_url, contact_section);
      break;
    }
  }

  gtk_widget_destroy (dialog);
  
  g_free (contact_section);
  g_free (contact_name);
  g_free (contact_url);
}


/* DESCRIPTION  :  This callback is called when the user chooses to copy
 *                 a contact URL to the clipboard.
 * BEHAVIOR     :  Copy the URL given as data in the clipboard.
 * PRE          :  data != NULL
 */
static void
copy_url_to_clipboard_cb (GtkWidget *w,
			  gpointer data)
{
  GtkClipboard *cb = NULL;

  if (!data)
    return;
  
  cb = gtk_clipboard_get (GDK_NONE);
  gtk_clipboard_set_text (cb, (char *) data, -1);
}


/* DESCRIPTION  :  This callback is called when the user right-clicks on
 *                 contact in the calls history or in the addressbook.
 * BEHAVIOR     :  Displays a menu to call that contact, or transfer a call
 *                 to that contact, or simply manipulate it.
 * PRE          :  data == 1 if the contact is clicked in the calls history.
 */
gint
contact_clicked_cb (GtkWidget *w,
		    GdkEventButton *e,
		    gpointer data)
{
  GmLdapWindow *lw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;
  
  GtkWidget *menu = NULL;
  GtkWidget *child = NULL;

  gchar *contact_url = NULL;
  gchar *contact_name = NULL;
  gchar *msg = NULL;

  unsigned calling_state = GMH323EndPoint::Standby;
  GMH323EndPoint *ep = NULL;

  gboolean is_group = false;
  bool already_member = false;
  
  lw = GnomeMeeting::Process ()->GetLdapWindow ();


  if (e->type == GDK_BUTTON_PRESS || e->type == GDK_KEY_PRESS) {

    /* The contact was clicked in the calls history */
    if (GPOINTER_TO_INT (data) == 1) {

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (w));
    
      if (gtk_tree_selection_get_selected (selection, &model, &iter)) 
      	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			    1, &contact_name,
			    2, &contact_url, -1);
      is_group = true;
    }
    else  /* The contact was clicked in the addressbook */
      get_selected_contact_info (NULL, &contact_name,
				 &contact_url, NULL, &is_group);

    
    if (contact_name && contact_url) {

      if (contact_url)
	already_member =
	  is_contact_member_of_addressbook (GMURL (contact_url));

      msg = g_strdup_printf (_("Add %s to Address Book"), contact_name);

      /* Update the main menu sensitivity if the contact is selected
	 from the addressbook */
      if (GPOINTER_TO_INT (data) == 0) {
	
	gnomemeeting_addressbook_update_menu_sensitivity ();
	
	child = GTK_BIN (gtk_menu_get_widget (lw->main_menu, "add"))->child;
	gtk_label_set_text (GTK_LABEL (child), msg);
      }

      
      if (e->button == 3) {
	
	menu = gtk_menu_new ();

       	MenuEntry server_contact_menu [6];

	MenuEntry sep =
	  GTK_MENU_SEPARATOR;

	MenuEntry endt =
	  GTK_MENU_END;
	
	MenuEntry call =
	  GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
			 NULL, 0, 
			 GTK_SIGNAL_FUNC (call_user_cb), (gpointer) w,
			 TRUE);

	MenuEntry transfer =
	  GTK_MENU_ENTRY("transfer", _("_Tranfer Call to Contact"), NULL,
			 NULL, 0, 
			 GTK_SIGNAL_FUNC (call_user_cb), (gpointer) w,
			 TRUE);

	MenuEntry add =
	  GTK_MENU_ENTRY("add", msg, NULL,
			 GTK_STOCK_ADD, 0,
			 GTK_SIGNAL_FUNC (edit_contact_cb),
			 NULL, TRUE);
	
	MenuEntry props =
	  GTK_MENU_ENTRY("properties", _("Contact _Properties"), NULL,
			 GTK_STOCK_PROPERTIES, 0,
			 GTK_SIGNAL_FUNC (edit_contact_cb),
			 NULL, TRUE);

	MenuEntry clipb =
	  GTK_MENU_ENTRY("clipboard", _("Copy URL to the Clipboard"), NULL,
			 GTK_STOCK_COPY, 0,
			 GTK_SIGNAL_FUNC (copy_url_to_clipboard_cb),
			 g_strdup (contact_url), TRUE);
	
	MenuEntry del =
	  GTK_MENU_ENTRY("del", _("_Delete"), NULL,
			 GTK_STOCK_DELETE, 0, 
			 GTK_SIGNAL_FUNC (delete_cb), NULL, TRUE);

	ep = GnomeMeeting::Process ()->Endpoint ();
	calling_state = ep->GetCallingState ();

	if (calling_state == GMH323EndPoint::Connected)
	  server_contact_menu [0] = transfer;
	else 
	  server_contact_menu [0] = call;

	if (GPOINTER_TO_INT (data) == 0) {
	  
	  if (!already_member) {

	    server_contact_menu [1] = sep;
	    server_contact_menu [2] = clipb;
	    server_contact_menu [3] = add;
	    server_contact_menu [4] = endt;
	  }
	  else {
	    
	    if (!is_group) {

	      server_contact_menu [1] = sep;
	      server_contact_menu [2] = clipb;
	      server_contact_menu [3]= props;
	      server_contact_menu [4] = endt;
	    }
	    else {

	      server_contact_menu [1]= props;
	      server_contact_menu [2] = clipb;
	      server_contact_menu [3] = sep;
	      server_contact_menu [4] = del;
	      server_contact_menu [5] = endt;
	    }
	  }
	}
	else /* No props and delete if the contact is in the calls history */
	  server_contact_menu [1] = endt;	

	gtk_build_menu (menu, server_contact_menu, NULL, NULL);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			e->button, e->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));
		
	g_free (contact_url);
	g_free (contact_name);
	
	return TRUE;
      }
    
      g_free (msg);
    }

    g_free (contact_name);
    g_free (contact_url);
  }

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the user chooses to add
 *                 a new contact section, server or group.
 * BEHAVIOR     :  Opens a pop up to ask for the contact section name
 *                 and updates the right gconf key if the contact section
 *                 doesn't already exist.
 * PRE          :  data = CONTACTS_GROUPS or CONTACTS_SERVERS
 */
static void
new_contact_section_cb (GtkWidget *widget,
			gpointer data)
{
  GmWindow *gw = NULL;
  GtkWidget *dialog = NULL;
  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;

  BOOL is_group = FALSE;
  
  gchar *entry_text = NULL;
  gint result = 0;

  gchar *dialog_text = NULL;
  gchar *dialog_error_text = NULL;
  gchar *dialog_title = NULL;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  is_group = (GPOINTER_TO_INT (data) == CONTACTS_GROUPS);
  
  if (!is_group) {

    dialog_title = g_strdup (_("Add a New Server"));
    dialog_text = g_strdup (_("Enter the server name:"));
    dialog_error_text = g_strdup (_("Sorry but there is already a server with the same name in the address book."));
  }
  else {

    dialog_title = g_strdup (_("Add a New Group"));
    dialog_text = g_strdup (_("Enter the group name:"));
    dialog_error_text = g_strdup (_("Sorry but there is already a group with the same name in the address book."));
  }
    
  dialog = gtk_dialog_new_with_buttons (dialog_title, 
					GTK_WINDOW (gw->ldap_window),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);
  label = gtk_label_new (dialog_text);
  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), label,
		      FALSE, FALSE, 4);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), entry,
		      FALSE, FALSE, 4);
  gtk_entry_set_activates_default (GTK_ENTRY (entry), true);

  
  gtk_widget_show_all (dialog);
  
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  switch (result) {

    case GTK_RESPONSE_ACCEPT:
	 
      entry_text = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));

      if (entry_text && strcmp (entry_text, "")) {

	if (is_contact_section_member_of_addressbook (entry_text, is_group)) 
	  gnomemeeting_error_dialog (GTK_WINDOW (gw->ldap_window),
				     _("Invalid server or group name"),
				     dialog_error_text);
	else 
	  add_contact_section (entry_text, is_group);
      }
      
      break;
  }

  gtk_widget_destroy (dialog);
  
  g_free (dialog_title);
  g_free (dialog_text);
  g_free (dialog_error_text);
}


/* DESCRIPTION  :  This callback is called when the user chooses to delete
 *                 a contact section, server or group.
 * BEHAVIOR     :  Removes the corresponding contact section from the gconf
 *                 key and updates it.
 * PRE          :  data = 1 if rename is invoked instead of delete.
 */
static void
modify_contact_section_cb (GtkWidget *widget,
			   gpointer data)
{
  GmLdapWindow *lw = NULL;
  GmWindow *gw = NULL;

  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *dialog = NULL;

  gchar *dialog_error_text = NULL;
  gchar *entry_text = NULL;
  gchar *name = NULL;
  gchar *confirm_msg = NULL;
  gchar *gconf_key = NULL;
  gboolean is_group = false;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  lw = GnomeMeeting::Process ()->GetLdapWindow ();

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    
    path = gtk_tree_model_get_path (model, &iter);
    
    gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_CONTACT_SECTION_NAME, &name, -1);
      
    if (gtk_tree_path_get_depth (path) >= 2) {
      
      if (gtk_tree_path_get_indices (path) [0] == 0) {
	
	gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
	if (GPOINTER_TO_INT (data) == 1)
	  confirm_msg = g_strdup_printf (_("Please enter a new name for server %s:"), name);
	else
	  confirm_msg = g_strdup_printf (_("Are you sure you want to delete server %s?"), name);

	dialog_error_text = g_strdup (_("Sorry but there is already a server with the same name in the address book."));
      }
      else {

	is_group = true;
	gconf_key = g_strdup (CONTACTS_KEY "groups_list");
	if (GPOINTER_TO_INT (data) == 1)
	  confirm_msg = g_strdup_printf (_("Please enter a new name for group %s:"), name);
	else
	  confirm_msg = g_strdup_printf (_("Are you sure you want to delete group %s and all its contacts?"), name);

	dialog_error_text = g_strdup (_("Sorry but there is already a group with the same name in the address book."));
      }


      if (GPOINTER_TO_INT (data) == 1) {
	
	dialog =
	  gtk_dialog_new_with_buttons (NULL,
				       GTK_WINDOW (gw->ldap_window),
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_STOCK_CANCEL, GTK_RESPONSE_NO,
				       GTK_STOCK_OK, GTK_RESPONSE_YES,
				       NULL);

	label = gtk_label_new (confirm_msg);
	entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (entry), name);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), label,
			    FALSE, FALSE, 4);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), entry,
			    FALSE, FALSE, 4);
	gtk_entry_set_activates_default (GTK_ENTRY (entry), true);

      }
      else
	dialog =
	  gtk_message_dialog_new (GTK_WINDOW (gw->ldap_window),
				  GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
				  GTK_BUTTONS_YES_NO, confirm_msg);

      gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				       GTK_RESPONSE_YES);
      gtk_widget_show_all (dialog);
      
      switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

      case GTK_RESPONSE_YES:

	if (GPOINTER_TO_INT (data) == 1) {

	  entry_text = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));

	  /* Do nothing if no text is entered, and display an error if
	     another group of the same name already exists */
	  if (strcmp (entry_text, "")) {
	    
	    if (is_contact_section_member_of_addressbook (entry_text,
							  is_group))
	      gnomemeeting_error_dialog (GTK_WINDOW (gw->ldap_window),
					 _("Invalid server or group name"),
					 dialog_error_text);
	    else
	      rename_contact_section (name, entry_text, is_group);
	  }
	}
	else
	  delete_contact_section (name, is_group);
				  
	break;

      }

      gtk_tree_path_free (path);
      g_free (name);
      
      g_free (confirm_msg);
      g_free (dialog_error_text);
      
      if (dialog)
	gtk_widget_destroy (dialog);
    }
  }
}


/* DESCRIPTION  :  This callback is called when there is a "changed" event
 *                 signal on one of the contact section.
 * BEHAVIOR     :  Selects the right notebook page and updates the menu.
 * PRE          :  /
 */
static void
contact_section_changed_cb (GtkTreeSelection *selection,
			    gpointer data)
{
  GtkWidget *page = NULL;
  GtkTreeSelection *lselection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gint page_num = -1;

  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;

  lw = GnomeMeeting::Process ()->GetLdapWindow ();
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_NOTEBOOK_PAGE, &page_num, -1);
    
    /* Selectes the good notebook page for the contact section */
    if (page_num != -1) {

      /* Selects the good notebook page */
      gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), 
				     page_num);
	
      /* Unselect all rows of the list store in that notebook page */
      page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
					page_num);
      lwp = gnomemeeting_get_ldap_window_page (page);
	
      if (lwp && lwp->tree_view) {
	  
	lselection =
	  gtk_tree_view_get_selection (GTK_TREE_VIEW (lwp->tree_view));
	  
	if (lselection)
	  gtk_tree_selection_unselect_all (GTK_TREE_SELECTION (lselection));
      }
    }
  }

  
  /* Update the sensitivity of DELETE */
  gnomemeeting_addressbook_update_menu_sensitivity ();
}


/* DESCRIPTION  :  This callback is called when there is an "event_after"
 *                 signal on one of the contact section.
 * BEHAVIOR     :  Displays a popup menu with the required options.
 * PRE          :  /
 */
static gint
contact_section_clicked_cb (GtkWidget *w,
			    GdkEventButton *e,
			    gpointer data)
{
  GmLdapWindow *lw = NULL;

  GtkWidget *menu = NULL;
  
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  gint page_num;
  gchar *path_string = NULL;

  tree_view = GTK_TREE_VIEW (w);

  
  if (e->window != gtk_tree_view_get_bin_window (tree_view)) 
    return FALSE;

  lw = GnomeMeeting::Process ()->GetLdapWindow ();

  if (e->type == GDK_BUTTON_PRESS || e->type == GDK_KEY_PRESS) {

    if (gtk_tree_view_get_path_at_pos (tree_view, (int) e->x, (int) e->y,
				       &path, NULL, NULL, NULL)) {

      
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
      
      if (selection &&
	  gtk_tree_selection_get_selected (selection, &model, &iter)) 
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			    COLUMN_NOTEBOOK_PAGE, &page_num, -1);
      
      /* If it is a right-click, then popup a menu */
      if (e->type == GDK_BUTTON_PRESS && e->button == 3 && 
	  gtk_tree_selection_path_is_selected (selection, path)) {
	
	menu = gtk_menu_new ();

	path_string = gtk_tree_path_to_string (path);	
	
	MenuEntry new_server_menu [] =
	  {
	    GTK_MENU_ENTRY("new_server", _("New Server"), NULL,
			   GTK_STOCK_NEW, 0,
			   GTK_SIGNAL_FUNC (new_contact_section_cb), 
			   GINT_TO_POINTER (CONTACTS_SERVERS), TRUE),

	    GTK_MENU_END
	  };
	
	MenuEntry new_group_menu [] =
	  {
	    GTK_MENU_ENTRY("new_group", _("New Group"), NULL,
			   GTK_STOCK_NEW, 0,
			   GTK_SIGNAL_FUNC (new_contact_section_cb), 
			   GINT_TO_POINTER (CONTACTS_GROUPS), TRUE),

	    GTK_MENU_END
	  };
	
	MenuEntry delete_refresh_contact_section_menu [] =
	  {
	    GTK_MENU_ENTRY("find", _("_Find"), NULL,
			   GTK_STOCK_FIND, 0,
			   GTK_SIGNAL_FUNC (refresh_server_content_cb), 
			   GINT_TO_POINTER (page_num), TRUE),

	    GTK_MENU_SEPARATOR,

	    GTK_MENU_ENTRY("rename", _("_Rename"), NULL,
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (modify_contact_section_cb),
			   GINT_TO_POINTER (1), TRUE),
	    
	    GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
			   GTK_STOCK_DELETE, 0, 
			   GTK_SIGNAL_FUNC (modify_contact_section_cb), NULL,
			   TRUE),

	    GTK_MENU_END
	  };
	
	MenuEntry delete_group_new_contact_section_menu [] =
	  {
	    GTK_MENU_ENTRY("new_contact", _("New Contact"), NULL,
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (new_contact_cb), 
			   NULL, TRUE),

	    GTK_MENU_SEPARATOR,

	    GTK_MENU_ENTRY("rename", _("_Rename"), NULL,
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (modify_contact_section_cb),
			   GINT_TO_POINTER (1), TRUE),

	    GTK_MENU_ENTRY("delete", _("Delete"), NULL,
			   GTK_STOCK_DELETE, 0, 
			   GTK_SIGNAL_FUNC (modify_contact_section_cb), 
			   NULL, TRUE),

	    GTK_MENU_END
	  };


	/* Build the appropriate popup menu */
	if (gtk_tree_path_get_depth (path) >= 2)
	  if (gtk_tree_path_get_indices (path) [0] == 0)
	    gtk_build_menu (menu, delete_refresh_contact_section_menu,
			    NULL, NULL);
				     
	  else
	    gtk_build_menu (menu, delete_group_new_contact_section_menu,
			    NULL, NULL);
	else
	  if (gtk_tree_path_get_indices (path) [0] == 0) 
	    gtk_build_menu (menu, new_server_menu,
			    NULL, NULL);
	  else
	    gtk_build_menu (menu, new_group_menu,
			    NULL, NULL);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			e->button, e->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			  GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));       

	return TRUE;
      }

      gtk_tree_path_free (path);
    }
  }

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when to delete a contact section
 *                 (server or group), or to delete a contact from a group.
 * BEHAVIOR     :  If a contact is selected, then deletes the contact, if
 *                 not, then deletes the selected contact section.
 * PRE          :  /
 */
static void
delete_cb (GtkWidget *w,
	   gpointer data)
{
  GmLdapWindow *lw = NULL;

  lw = GnomeMeeting::Process ()->GetLdapWindow ();


  if (get_selected_contact_info (NULL, NULL, NULL, NULL, NULL)) 
    delete_contact_from_group_cb (NULL, NULL);
  /* No contact is selected, but perhaps a contact section to delete
     is selected */
  else 
    modify_contact_section_cb (NULL, NULL);
}


/* DESCRIPTION  :  This callback is called when the user chooses in the menu
 *                 to call, it can be the addressbook menu or the right-click
 *                 menu in the calls history and addressbook.
 * BEHAVIOR     :  Calls the user of the selected line in the GtkTreeView or
 *                 transfer the call to him if we are in a call.
 * PRE          :  If it is called from the right-click menu, data is non-NULL
 *                 and corresponds to the current selected tree view. NULL in
 *                 other cases.
 */
static void
call_user_cb (GtkWidget *w,
	      gpointer data)
{
  GtkWidget *page = NULL;
  
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;
  
  gboolean is_group = false;

  /* Called from the addressbook main menu */
  if (!data) {

    lw = GnomeMeeting::Process ()->GetLdapWindow ();
    
    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
				      gtk_notebook_get_current_page
				      (GTK_NOTEBOOK (lw->notebook)));

    if (page)
      lwp = gnomemeeting_get_ldap_window_page (page);

    if (lwp) {
      
      get_selected_contact_info (NULL, NULL, NULL, NULL, &is_group);
      contact_activated_cb (GTK_TREE_VIEW (lwp->tree_view), NULL, NULL, 
			    is_group ? GINT_TO_POINTER (CONTACTS_GROUPS)
			    : GINT_TO_POINTER (CONTACTS_SERVERS));
    }
  }
  else { /* Called from the right-click menu */
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (data));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (data));
    
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_view_row_activated (GTK_TREE_VIEW (data), path, NULL);
      gtk_tree_path_free (path);
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user double clicks on
 *                 a row corresonding to an user.
 * BEHAVIOR     :  Add the user name in the combo box and call him or transfer
 *                 the call to that user.
 * PRE          :  data is the page type or 3 if contact activated from calls history
 */
void 
contact_activated_cb (GtkTreeView *tree_view,
		      GtkTreePath *path,
		      GtkTreeViewColumn *column,
		      gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;
  
  gchar *contact_url = NULL;
  
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    /* Get the server name */
    if (GPOINTER_TO_INT (data) == CONTACTS_GROUPS)
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_URL, &contact_url, -1);
    else if (GPOINTER_TO_INT (data) == CONTACTS_SERVERS)
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ILS_IP, &contact_url, -1);
    else if (GPOINTER_TO_INT (data) == 3)
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  2, &contact_url, -1);
      
      
    if (contact_url) {	  
    
      /* if we are waiting for a call, add the IP
	 to the calls history, and call that user       */
      if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) {
      
	/* this function will store a copy of text */
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
			    contact_url);
      
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button),
				      true);
      }
      else if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Connected) {

	transfer_call_cb (NULL, (gpointer) contact_url);
      }
    }
  }

  
  g_free (contact_url);
}


/* DESCRIPTION  :  This callback is called when the user activates 
 *                 (double click) a server in the tree_store.
 * BEHAVIOR     :  Browse the selected server.
 * PRE          :  /
 */
static void
contact_section_activated_cb (GtkTreeView *tree_view, 
			      GtkTreePath *path,
			      GtkTreeViewColumn *column) 
{
  int page_num = -1;

  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;

  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  lw = GnomeMeeting::Process ()->GetLdapWindow ();

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    if (path) {

      if (gtk_tree_path_get_depth (path) >= 2) {

	/* We refresh the list only if the user double-clicked on a row
	   corresponding to a server */
	if (gtk_tree_path_get_indices (path) [0] == 0) {
      
	  /* Get the server name */
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			      COLUMN_NOTEBOOK_PAGE, &page_num, -1);

	  if (page_num != - 1) {
    
	    refresh_server_content_cb (NULL, GINT_TO_POINTER (page_num));
	  }
	}
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user chooses to refresh
 *                 the server content.
 * BEHAVIOR     :  Browse the selected server.
 * PRE          :  data = page_num of GtkNotebook containing the server.
 */
static void
refresh_server_content_cb (GtkWidget *w,
			   gpointer data)
{
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;

  int option_menu_option = 0;
  int page_num = GPOINTER_TO_INT (data);

  gchar *filter = NULL;
  gchar *search_entry_text = NULL;
  
  GtkWidget *page = NULL;

  lw = GnomeMeeting::Process ()->GetLdapWindow ();
  gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), 
				 page_num);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
				    page_num);

  lwp = gnomemeeting_get_ldap_window_page (page);

  if (lwp) {

    search_entry_text =
      (gchar *) gtk_entry_get_text (GTK_ENTRY (lwp->search_entry));

    if (search_entry_text && strcmp (search_entry_text, "")) {

      option_menu_option =
	gtk_option_menu_get_history (GTK_OPTION_MENU (lwp->option_menu));
	      
      switch (option_menu_option)
	{
	case 0:
	  filter = NULL;
	  break;

	case 1:
	  filter =
	    g_strdup_printf ("(givenname=%%%s%%)", search_entry_text);
	  break;
		  
	case 2:
	  filter =
	    g_strdup_printf ("(surname=%%%s%%)", search_entry_text);
	  break;
		  
	case 3:
	  filter =
	    g_strdup_printf ("(rfc822mailbox=%%%s%%)",
			     search_entry_text);
	  break;
	};
    }
  }	    

  /* Check if there is already a search running */
  if (lwp && !lwp->ils_browser && page_num != -1) 
    lwp->ils_browser =
      new GMILSBrowser (lwp, lwp->contact_section_name, filter);

  g_free (filter);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if contact_url corresponds to a contact
 *                 of group group_name.
 * PRE          :  The group name is the non-escaped form.
 */
static gboolean
is_contact_member_of_group (GMURL contact_url,
			    const char *g_name)
{
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  bool found = false;
  gchar *gconf_key = NULL;
  gchar *group_name = NULL;
  gchar **contact_info = NULL;

  if (contact_url.IsEmpty () || !g_name)
    return false;
  
  group_name = escape_contact_section (g_name);

  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);

  group_content = gconf_get_string_list (gconf_key);
  group_content_iter = group_content;
  
  while (group_content_iter && !found) {

    if (group_content_iter->data) {

      contact_info = g_strsplit ((gchar *) group_content_iter->data, "|", 0);

      if (contact_info && contact_info [COLUMN_URL])
	if (GMURL (contact_info [COLUMN_URL]) == contact_url) {
	  
	  found = true;
	  break;
	}
      g_strfreev (contact_info);
    }

    group_content_iter = g_slist_next (group_content_iter);
  }
  
  g_slist_free (group_content);
  g_free (gconf_key);
  g_free (group_name);

  return found;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns a link to the contact corresponding to the given
 *                 contact_url in the group_content list, or NULL if none.
 * PRE          :  /
 */
static GSList *
find_contact_in_group_content (const char *contact_url,
			       GSList *group_content)
{
  GSList *group_content_iter = NULL;
  
  gchar **group_content_split = NULL;
  
  group_content_iter = group_content;
  while (group_content_iter) {

    if (group_content_iter->data) {
      
      group_content_split =
	g_strsplit ((char *) group_content_iter->data, "|", 0);

      if ((group_content_split && group_content_split [1] && contact_url
	   && GMURL (group_content_split [1]) == GMURL (contact_url))
	  || (!contact_url && !group_content_split [1]))
	break;
	    
      g_strfreev (group_content_split);
      group_content_split = NULL;
    }

    group_content_iter = g_slist_next (group_content_iter);
  }

  return group_content_iter;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create the contact section (group or server) given as
 *                 parameter under its non-escaped form if it doesn't exist
 *                 yet.
 * PRE          :  /
 */
static void
add_contact_section (const char *c_section,
		     gboolean is_group)
{
  gchar *gconf_key = NULL;
  gchar *contact_section = NULL;
  
  GSList *contacts_sections = NULL;

  if (!c_section)
    return;

  contact_section = escape_contact_section (c_section, FALSE);

  if (is_group)
    gconf_key = g_strdup (CONTACTS_KEY "groups_list");
  else
    gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
  
  contacts_sections = gconf_get_string_list (gconf_key); 

  if (!is_contact_section_member_of_addressbook (c_section, is_group)) {

    contacts_sections =
      g_slist_append (contacts_sections, g_strdup (contact_section));
	  
    gconf_set_string_list (gconf_key, contacts_sections);
  }
  
  g_slist_free (contacts_sections);
  g_free (gconf_key);
  g_free (contact_section);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Delete the contact section (group or server) given as
 *                 parameter under its non-escaped form.
 * PRE          :  /
 */
static void
delete_contact_section (const char *c_section,
			gboolean is_group)
{
  GSList *contacts_sections = NULL;
  GSList *contacts_sections_iter = NULL;

  gchar *contact_section = NULL;
  gchar *unset_group_gconf_key = NULL;
  gchar *gconf_key = NULL;

  if (!c_section)
    return;
  
  contact_section = escape_contact_section (c_section);
  
  for (int i = 0 ; i < 2 ; i++) {

    if (i == 0)
      gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
    else
      gconf_key = g_strdup (CONTACTS_KEY "groups_list");
    
    contacts_sections = gconf_get_string_list (gconf_key);
    contacts_sections_iter = contacts_sections;

    unset_group_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, contact_section);

    while (contacts_sections_iter && contacts_sections_iter->data) {

      if ((!strcasecmp ((char *) contacts_sections_iter->data, contact_section)
	   || !strcasecmp ((char *) contacts_sections_iter->data, c_section))
	  && (is_group == (i == 1))) {

	contacts_sections =
	  g_slist_remove_link (contacts_sections, contacts_sections_iter);
	g_slist_free_1 (contacts_sections_iter);
	

	gconf_set_string_list (unset_group_gconf_key, NULL);

	gconf_client_remove_dir (gconf_client_get_default (),
				 "/apps/gnomemeeting", 0);
	gconf_client_unset (gconf_client_get_default (),
			    unset_group_gconf_key, NULL);
	gconf_client_add_dir (gconf_client_get_default (),
			      "/apps/gnomemeeting",
			      GCONF_CLIENT_PRELOAD_RECURSIVE, 0);

	contacts_sections_iter = contacts_sections;
      }

      contacts_sections_iter = g_slist_next (contacts_sections_iter);
    }
    
    gconf_set_string_list (gconf_key, contacts_sections);

    g_free (gconf_key);
    g_free (unset_group_gconf_key);
    
    g_slist_free (contacts_sections);
    g_slist_free (contacts_sections_iter);
  }

  g_free (contact_section);    
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Adds a contact to the given group. If old_contact_url is
 *                 NULL, the contact is simply added. If another contact
 *                 with the old_onctact_url exists, it is simply replaced
 *                 by the new one.
 * PRE          :  contact_url != NULL
 */
static void
add_contact_to_group (const char *contact_name,
		      const char *contact_url,
		      const char *contact_speed_dial,
		      const char *old_contact_url,
		      const char *g_name)
{
  gchar *contact_info = NULL;
  gchar *group_name = NULL;
  gchar *gconf_key = NULL;

  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  if (!contact_url)
    return;
  
  contact_info =
    g_strdup_printf ("%s|%s|%s",
		     contact_name?contact_name:"",
		     contact_url?contact_url:"",
		     contact_speed_dial?contact_speed_dial:"");

  group_name = escape_contact_section (g_name);
  
  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);

  group_content = gconf_get_string_list (gconf_key);

  if (group_name) {
	    	    
	    
    /* Once we find the contact corresponding to the old saved url
       (if we are editing an existing user and not adding a new one),
       we delete him and insert the new one at the same position;
       if the group is not selected for that user, we delete him from
       the group.
    */
    if (old_contact_url) {
	      
      group_content_iter =
	find_contact_in_group_content (old_contact_url, group_content);
	      
      /* Only reinsert the contact if the group is selected for him,
	 otherwise, only delete him from the group */
      group_content =
	g_slist_insert (group_content, g_strdup (contact_info),
			g_slist_position (group_content,
					  group_content_iter));
	      
      group_content = g_slist_remove_link (group_content,
					   group_content_iter);
      g_slist_free_1 (group_content_iter);
    }
    else
      group_content =
	g_slist_append (group_content, g_strdup (contact_info));
    
    gconf_set_string_list (gconf_key, group_content);
  }

  g_slist_free (group_content);
  g_free (gconf_key);
  g_free (group_name);
  g_free (contact_info);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Removes the given contact from the give group if
 *                 he was member of that group.
 * PRE          :  /
 */
static void
delete_contact_from_group (const char *contact_url,
			   const char *c_section)
{
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  gchar *contact_section = NULL;
  gchar *gconf_key = NULL;

  if (!contact_url || !c_section)
    return;

  contact_section = escape_contact_section (c_section);
  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, contact_section);
  
  group_content = gconf_get_string_list (gconf_key);

  group_content_iter =
    find_contact_in_group_content (contact_url, group_content);

  if (group_content_iter) {
    
    group_content = g_slist_remove_link (group_content,
					 group_content_iter);
    g_slist_free_1 (group_content_iter);
    
    gconf_set_string_list (gconf_key, group_content);
  }
  
  g_slist_free (group_content);

  g_free (gconf_key);
  g_free (contact_section);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the given pointers so that they point to the
 *                 good sections in the GtkNotebook holding the GtkTreeViews
 *                 storing the contacts. Returns false on failure (nothing
 *                 selected). The contact_speed_dial pointer is not udpated
 *                 if *is_group = FALSE, ie if the selection corresponds to
 *                 server and not a group of contacts. All allocated
 *                 pointers should be freed.
 * PRE          :  /
 */
static gboolean
get_selected_contact_info (gchar **contact_section,
			   gchar **contact_name,
			   gchar **contact_url,
			   gchar **contact_speed_dial,
			   gboolean *is_group)
{
  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  int page_num = 0;

  GmLdapWindowPage *lwp = NULL;
  GmLdapWindow *lw = NULL;

  lw = GnomeMeeting::Process ()->GetLdapWindow ();
  
  /* Get the required data from the GtkNotebook page */
  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), page_num);
  lwp = gnomemeeting_get_ldap_window_page (page);
  
  if (page && lwp) {

    if (contact_section)
      *contact_section = g_utf8_strdown (lwp->contact_section_name, -1);
    // should be freed
      
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lwp->tree_view));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (lwp->tree_view));
    
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
      /* If the callback is called because we add a contact from the
	 server listing */
      if (lwp->page_type == CONTACTS_SERVERS) {

	if (contact_name)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_ILS_NAME, contact_name, -1);

	if (contact_url)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_ILS_URL, contact_url, -1);

	if (is_group)
	  *is_group = false;
	}
      else {

	if (contact_name)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_NAME, contact_name, -1);
	
	if (contact_url)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_URL, contact_url, -1);

	if (contact_speed_dial)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_SPEED_DIAL, contact_speed_dial, -1);

	if (is_group)
	  *is_group = true;
      }
    }
    else
      return false;
  }
  else
    return false;

  
  return true;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Gives access to the speed dial and name corresponding to the
 *                 URL given as parameter. NULL if none.
 * PRE          :  A NULL name or speed dial means it is not used.
 *
 */
static void
get_contact_info_from_url (GMURL url,
			   gchar **name,
			   gchar **speed_dial)
{
  gchar *group_content_gconf_key = NULL;
  char **contact_info = NULL;

  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  GSList *groups = NULL;
  GSList *groups_iter = NULL;

  gchar *group_name = NULL;

  gboolean found_something = FALSE;

  if (name)
    *name = NULL;
  if (speed_dial)
    *speed_dial = NULL;

  if (url.IsEmpty ())
    return; 

  groups = gconf_get_string_list (CONTACTS_KEY "groups_list");
  groups_iter = groups;

  while (groups_iter && groups_iter->data) {

    group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);


    group_content = gconf_get_string_list (group_content_gconf_key);
    group_content_iter = group_content;

    while (group_content_iter
	   && group_content_iter->data && !found_something) {

      contact_info =
	g_strsplit ((char *) group_content_iter->data, "|", 0);

      if (contact_info [1] && url == GMURL (contact_info [1])) {

	if (name && contact_info [0])
	  *name = g_strdup (contact_info [0]);
	if (speed_dial && contact_info [2])
	  *speed_dial = g_strdup (contact_info [2]);

	found_something = TRUE;
      }

      g_strfreev (contact_info);
      group_content_iter = g_slist_next (group_content_iter);
    }

    g_free (group_content_gconf_key);
    g_slist_free (group_content);

    groups_iter = g_slist_next (groups_iter);
    g_free (group_name);
  }

  g_slist_free (groups);   
}


/* The functions */
void
gnomemeeting_addressbook_update_menu_sensitivity ()
{
  gboolean is_new = false;
  gboolean is_group = false;
  gboolean is_section = false;

  gchar *contact_url = NULL;
  
  GmLdapWindow *lw = NULL;
  GMH323EndPoint *ep = NULL;
  int calling_state = GMH323EndPoint::Standby;
  
  lw = GnomeMeeting::Process ()->GetLdapWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  if (!get_selected_contact_info (NULL, NULL, &contact_url, NULL, &is_group))
    is_section = true;

  if (contact_url)
    is_new =
      !is_contact_member_of_addressbook (GMURL (contact_url));

  if (ep)
    calling_state = ep->GetCallingState ();

  if (!is_section) {
    
    if (calling_state == GMH323EndPoint::Standby) {

      gtk_menu_set_sensitive (lw->main_menu, "call", TRUE);
      gtk_menu_set_sensitive (lw->main_menu, "transfer", FALSE);
    }
    else if (calling_state == GMH323EndPoint::Connected) {
    
      gtk_menu_set_sensitive (lw->main_menu, "call", FALSE);
      gtk_menu_set_sensitive (lw->main_menu, "transfer", TRUE);
    }
  }

  
  if (is_group || is_section) {

    gtk_menu_set_sensitive (lw->main_menu, "delete", TRUE);
  }
  else {

    gtk_menu_set_sensitive (lw->main_menu, "delete", FALSE);
  }
  
  if (is_group || !is_new) {

    gtk_menu_set_sensitive (lw->main_menu, "add", FALSE);
    gtk_menu_set_sensitive (lw->main_menu, "properties", TRUE);
  }
  else {

    gtk_menu_set_sensitive (lw->main_menu, "add", TRUE);
    gtk_menu_set_sensitive (lw->main_menu, "properties", FALSE);
  }

  
  if (is_section) {

    gtk_menu_section_set_sensitive (lw->main_menu, "call", FALSE);

    gtk_menu_set_sensitive (lw->main_menu, "add", FALSE);
    gtk_menu_set_sensitive (lw->main_menu, "properties", FALSE);
    gtk_menu_set_sensitive (lw->main_menu, "rename", TRUE);
  }
  else
    gtk_menu_set_sensitive (lw->main_menu, "rename", FALSE);

  g_free (contact_url);
}


GtkWidget *
gnomemeeting_ldap_window_new (GmLdapWindow *lw)
{
  GtkWidget *window = NULL;
  GtkWidget *hpaned = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *vbox2 = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *menubar = NULL;
  GtkWidget *scroll = NULL;
  GdkPixbuf *icon = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkAccelGroup *accel = NULL;
  GtkTreeStore *model = NULL;

  static GtkTargetEntry dnd_targets [] =
    {
      {"text/plain", GTK_TARGET_SAME_APP, 0}
    };

  /* Hack to make sure that they are translated in the addressbook
   * now that unfortunately all people are using the english version
   */
  gchar *a = NULL;
  a = g_strdup (_("Friends"));
  g_free (a);
  a = g_strdup (_("Family"));
  g_free (a);
  a = g_strdup (_("Work"));
  g_free (a);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  icon = gtk_widget_render_icon (GTK_WIDGET (window),
				 GM_STOCK_ADDRESSBOOK_16,
				 GTK_ICON_SIZE_MENU, NULL);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("address_book_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), 
			_("Address Book"));
  gtk_window_set_icon (GTK_WINDOW (window), icon);
  gtk_window_set_position (GTK_WINDOW (window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (window), 670, 370);
  g_object_unref (icon);
  
  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);
  
  /* A vbox that will contain the menubar, and also the hbox containing
     the rest of the window */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  menubar = gtk_menu_bar_new ();
  
  static MenuEntry addressbook_menu [] =
    {
      GTK_MENU_NEW(_("_File")),

      GTK_MENU_ENTRY("new_server", _("New _Server"), NULL,
		     GM_STOCK_REMOTE_CONTACT, 0,
		     GTK_SIGNAL_FUNC (new_contact_section_cb),
		     GINT_TO_POINTER (CONTACTS_SERVERS), TRUE),
      GTK_MENU_ENTRY("new_group", _("New _Group"), NULL,
		     GM_STOCK_LOCAL_CONTACT, 0, 
		     GTK_SIGNAL_FUNC (new_contact_section_cb),
		     GINT_TO_POINTER (CONTACTS_GROUPS), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
		     GTK_STOCK_DELETE, 'd', 
		     GTK_SIGNAL_FUNC (delete_cb), NULL, FALSE),

      GTK_MENU_ENTRY("rename", _("_Rename"), NULL,
		     NULL, 0,
		     GTK_SIGNAL_FUNC (modify_contact_section_cb),
		     GINT_TO_POINTER (1), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", _("_Close"), NULL,
		     GTK_STOCK_CLOSE, 'w',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_NEW(_("C_ontact")),

      GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (call_user_cb), NULL, FALSE),
      GTK_MENU_ENTRY("transfer", _("_Tranfer Call to Contact"), NULL,
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (call_user_cb), NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("new_contact", _("New _Contact"), NULL,
		     GTK_STOCK_NEW, 'n', 
		     GTK_SIGNAL_FUNC (new_contact_cb), 
		     NULL, TRUE),
      GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		     GTK_STOCK_ADD, 0,
		     GTK_SIGNAL_FUNC (edit_contact_cb), 
		     NULL, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("properties", _("Contact _Properties"), NULL,
		     GTK_STOCK_PROPERTIES, 0, 
		     GTK_SIGNAL_FUNC (edit_contact_cb), 
		     NULL, FALSE),

      GTK_MENU_END
    };

  gtk_build_menu (menubar, addressbook_menu, accel, NULL);
  lw->main_menu = menubar;
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

  
  /* A hbox to put the tree and the ldap browser */
  hpaned = gtk_hpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 6);
  gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
  

  /* The GtkTreeView that will store the contacts sections */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_paned_add1 (GTK_PANED (hpaned), frame);
  model = gtk_tree_store_new (NUM_COLUMNS_CONTACTS, GDK_TYPE_PIXBUF, 
			      G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN,
			      G_TYPE_INT);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scroll);

  lw->tree_view = gtk_tree_view_new ();  
  gtk_tree_view_set_model (GTK_TREE_VIEW (lw->tree_view), 
			   GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  gtk_container_add (GTK_CONTAINER (scroll), lw->tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (lw->tree_view), FALSE);

  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_BROWSE);


  /* Two renderers for one column */
  column = gtk_tree_view_column_new ();
  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, "pixbuf", COLUMN_PIXBUF, 
				       "visible", COLUMN_PIXBUF_VISIBLE, NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, "text", 
				       COLUMN_CONTACT_SECTION_NAME, NULL);
  gtk_tree_view_column_add_attribute (column, cell, "weight", 
				      COLUMN_WEIGHT);
  gtk_tree_view_append_column (GTK_TREE_VIEW (lw->tree_view),
			       GTK_TREE_VIEW_COLUMN (column));

  /* a vbox to put the frames and the user list */
  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_paned_add2 (GTK_PANED (hpaned), vbox2);  

  /* We will put a GtkNotebook that will contain the contacts list */
  lw->notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (lw->notebook), 0);
  gtk_box_pack_start (GTK_BOX (vbox2), lw->notebook, 
		      TRUE, TRUE, 0);

  
  /* Populate the tree_viw with groups and servers */
  gnomemeeting_addressbook_sections_populate ();


  /* Drag and Drop Setup */
  gtk_drag_dest_set (GTK_WIDGET (lw->tree_view),
		     GTK_DEST_DEFAULT_ALL,
		     dnd_targets, 1,
		     GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (lw->tree_view), "drag_motion",
		    G_CALLBACK (dnd_drag_motion_cb), 0);
  g_signal_connect (G_OBJECT (lw->tree_view), "drag_data_received",
		    G_CALLBACK (dnd_drag_data_received_cb), 0);
  
  /* Double-click on a server name or on a contact group */
  g_signal_connect (G_OBJECT (lw->tree_view), "row_activated",
		    G_CALLBACK (contact_section_activated_cb), NULL);  

  /* Click or right-click on a server name or on a contact group */
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (contact_section_changed_cb), NULL);
  g_signal_connect_object (G_OBJECT (lw->tree_view), "event_after",
			   G_CALLBACK (contact_section_clicked_cb), 
			   NULL, (GConnectFlags) 0);

  /* Hide but do not delete the ldap window */
  g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (delete_window_cb), NULL);
  
  gtk_widget_show_all (GTK_WIDGET (vbox));
  
  return window;
}


GmLdapWindowPage *
gnomemeeting_get_ldap_window_page (GtkWidget *page)
{
  GmLdapWindowPage *lwp = NULL;

  if (page)
    lwp = (GmLdapWindowPage *) g_object_get_data (G_OBJECT (page), "lwp");

  return lwp;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Free the memory associated with the GmLdapWindowPage for
 *                 the notebook page currently destroyed.
 * PRE          :  data = pointer to the GmLdapWindowPage for that page.
 *                 GmLdapWindowPage contains a Mutex released when the server
 *                 corresponding to that page has been browsed. Another
 *                 function must use it to ensure that the browse is terminated
 *                 before this function is called, you can use
 *                 void gnomemeeting_ldap_window_destroy_notebook_pages ();
 *
 */
static void
notebook_page_destroy (gpointer data)
{
  GmLdapWindowPage *lwp = (GmLdapWindowPage *) data;

  if (data) {

    g_free (lwp->contact_section_name);
    delete (lwp);
  }
}


void gnomemeeting_ldap_window_destroy_notebook_pages ()
{
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;
  GtkWidget *page = NULL;
  int i = 0;

  lw = GnomeMeeting::Process ()->GetLdapWindow ();
  
  while ((page =
	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), i))) {

    lwp = gnomemeeting_get_ldap_window_page (page);
    if (lwp) {

      lwp->search_quit_mutex.Wait ();
      gtk_widget_destroy (page);
    }
    
    i++;
  }
}


int
gnomemeeting_init_ldap_window_notebook (gchar *text_label,
					int type)
{
  GtkWidget *page = NULL;
  GtkWidget *scroll = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *handle = NULL;
  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *find_button = NULL;

  PangoAttrList *attrs = NULL; 
  PangoAttribute *attr = NULL; 

  gchar *section = NULL;
  
  GtkListStore *users_list_store = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  int cpt = 0, page_num = 0;
  
  static GtkTargetEntry dnd_targets [] =
    {
      {"text/plain", GTK_TARGET_SAME_APP, 0}
    };
 
  GmLdapWindow *lw = GnomeMeeting::Process ()->GetLdapWindow ();
  GmLdapWindowPage *current_lwp = NULL;

  
  section = gconf_escape_key (text_label, strlen (text_label));
			      
  while ((page =
	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), cpt))){

    current_lwp = gnomemeeting_get_ldap_window_page (page);
    if (current_lwp && current_lwp->contact_section_name && text_label
	&& !strcasecmp (current_lwp->contact_section_name, text_label)
	&& (type == current_lwp->page_type)) 
      return cpt;

    cpt++;
  }

  
  GmLdapWindowPage *lwp = new (GmLdapWindowPage);
  lwp->contact_section_name = g_strdup (text_label);
  lwp->ils_browser = NULL;
  lwp->search_entry = NULL;
  lwp->option_menu = NULL;
  lwp->page_type = type;
  
  if (type == CONTACTS_SERVERS)
    users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_SERVERS, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
			  G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING);
  else
    users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_GROUPS, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING);
			  
  vbox = gtk_vbox_new (FALSE, 0);
  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  lwp->tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (users_list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (lwp->tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (lwp->tree_view),
				   COLUMN_ILS_NAME);

  /* Set all Colums */
  if (type == CONTACTS_SERVERS) {
    
    renderer = gtk_cell_renderer_pixbuf_new ();
    /* Translators: This is "S" as in "Status" */
    column = gtk_tree_view_column_new_with_attributes (_("S"),
						       renderer,
						       "pixbuf", 
						       COLUMN_ILS_STATUS,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 150);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_toggle_new ();
    /* Translators: This is "A" as in "Audio" */
    column = gtk_tree_view_column_new_with_attributes (_("A"),
						       renderer,
						       "active", 
						       COLUMN_ILS_AUDIO,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 150);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_toggle_new ();
    /* Translators: This is "V" as in "Video" */
    column = gtk_tree_view_column_new_with_attributes (_("V"),
						       renderer,
						       "active", 
						       COLUMN_ILS_VIDEO,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 150);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Name"),
						       renderer,
						       "text", 
						       COLUMN_ILS_NAME,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_NAME);
    gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
					COLUMN_ILS_COLOR);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Comment"),
						       renderer,
						       "text", 
						       COLUMN_ILS_COMMENT,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_COMMENT);
    gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
					COLUMN_ILS_COLOR);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Location"),
						       renderer,
						       "text", 
						       COLUMN_ILS_LOCATION,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_LOCATION);
    gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
					COLUMN_ILS_COLOR);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("URL"),
						       renderer,
						       "text", 
						       COLUMN_ILS_URL,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_URL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "foreground", "blue",
		  "underline", TRUE, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Version"),
						       renderer,
						       "text", 
						       COLUMN_ILS_VERSION,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_VERSION);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("IP"),
						       renderer,
						       "text", 
						       COLUMN_ILS_IP,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_IP);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
  }
  else {

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Name"),
						       renderer,
						       "text", 
						       COLUMN_NAME,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 125);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("URL"),
						       renderer,
						       "text", 
						       COLUMN_URL,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_URL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "foreground", "blue",
		  "underline", TRUE, NULL);

        renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Speed Dial"),
						       renderer,
						       "text", 
						       COLUMN_SPEED_DIAL,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_SPEED_DIAL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
  }


  /* Copied from the prefs window, please use the same logic */
  lwp->section_name = gtk_label_new (text_label);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_misc_set_alignment (GTK_MISC (lwp->section_name), 0.0, 0.5);
  gtk_container_add (GTK_CONTAINER (frame), lwp->section_name);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  attrs = pango_attr_list_new ();
  attr = pango_attr_scale_new (PANGO_SCALE_LARGE);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT; 
  pango_attr_list_insert (attrs, attr); 
  attr = pango_attr_weight_new (PANGO_WEIGHT_HEAVY);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (lwp->section_name), attrs);
  pango_attr_list_unref (attrs);


  /* Add the tree view*/
  gtk_container_add (GTK_CONTAINER (scroll), lwp->tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (lwp->tree_view), 0);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

  
  if (type == CONTACTS_SERVERS) {

    hbox = gtk_hbox_new (FALSE, 0);
    
    /* The toolbar */
    handle = gtk_handle_box_new ();
    gtk_box_pack_start (GTK_BOX (vbox), handle, FALSE, FALSE, 0);  
    gtk_container_add (GTK_CONTAINER (handle), hbox);
    gtk_container_set_border_width (GTK_CONTAINER (handle), 0);

    
    /* option menu */
    menu = gtk_menu_new ();

    menu_item =
      gtk_menu_item_new_with_label (_("Find all contacts"));
    gtk_widget_show (menu_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    menu_item =
      gtk_menu_item_new_with_label (_("First name contains"));
    gtk_widget_show (menu_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    menu_item = gtk_menu_item_new_with_label (_("Last name contains"));
    gtk_widget_show (menu_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    menu_item = gtk_menu_item_new_with_label (_("E-mail contains"));
    gtk_widget_show (menu_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    lwp->option_menu = gtk_option_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (lwp->option_menu),
			      menu);
    gtk_option_menu_set_history (GTK_OPTION_MENU (lwp->option_menu),
				 0);
    gtk_box_pack_start (GTK_BOX (hbox), lwp->option_menu, FALSE, FALSE, 2);

    
    /* entry */
    lwp->search_entry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), lwp->search_entry, TRUE, TRUE, 2);
    gtk_widget_set_sensitive (GTK_WIDGET (lwp->search_entry), FALSE);

    /* The Find button */
    find_button = gtk_button_new_from_stock (GTK_STOCK_FIND);
    gtk_box_pack_start (GTK_BOX (hbox), find_button, FALSE, FALSE, 2);
    gtk_widget_show_all (handle);
    
    /* The statusbar */
    lwp->statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), lwp->statusbar, FALSE, FALSE, 0);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (lwp->statusbar), FALSE);


    /* The search entry is unsensitive when "Find all" is selected */
    g_signal_connect (G_OBJECT (GTK_OPTION_MENU (lwp->option_menu)->menu),
		      "deactivate", G_CALLBACK (filter_option_menu_changed),
		      lwp->search_entry);
  }


  /* The drag and drop information */
  gtk_drag_source_set (GTK_WIDGET (lwp->tree_view),
		       GDK_BUTTON1_MASK, dnd_targets, 1,
		       GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (lwp->tree_view), "drag_data_get",
		    G_CALLBACK (dnd_drag_data_get_cb), NULL);

  gtk_notebook_append_page (GTK_NOTEBOOK (lw->notebook), vbox, NULL);
  gtk_widget_show_all (GTK_WIDGET (lw->notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (lw->notebook), FALSE);

  while ((page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
					    page_num))) 
    page_num++;

  page_num--;
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), page_num);


  /* Connect to the search entry, and refresh button, if any */
  if (lwp->search_entry) {
    
    g_signal_connect (G_OBJECT (lwp->search_entry), "activate",
		      G_CALLBACK (refresh_server_content_cb), 
		      GINT_TO_POINTER (page_num));
    g_signal_connect (G_OBJECT (find_button), "clicked",
		      G_CALLBACK (refresh_server_content_cb), 
		      GINT_TO_POINTER (page_num));
  }
  
  g_object_set_data_full (G_OBJECT (page), "lwp", (gpointer) lwp,
			  notebook_page_destroy);
			       
  /* If the type of page is "groups", then we populate the page */
  if (type == CONTACTS_GROUPS) 
    gnomemeeting_addressbook_group_populate (users_list_store,
					     text_label);
  
  /* Signal to call the person on the double-clicked row */
  g_signal_connect (G_OBJECT (lwp->tree_view), "row_activated", 
		    G_CALLBACK (contact_activated_cb), GINT_TO_POINTER (type));

  /* Right-click on a contact */
  g_signal_connect (G_OBJECT (lwp->tree_view), "event_after",
		    G_CALLBACK (contact_clicked_cb), GINT_TO_POINTER (0));

  g_free (section);
  
  return page_num;
}


void
gnomemeeting_addressbook_group_populate (GtkListStore *list_store,
					 char *g_name)
{
  GtkTreeIter list_iter;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  char **contact_info = NULL;
  gchar *gconf_key = NULL;
  gchar *group_name = NULL;

  gtk_list_store_clear (GTK_LIST_STORE (list_store));

  group_name = escape_contact_section (g_name);

  if (group_name)
    gconf_key =  g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY,
				  (char *) group_name);

  group_content = gconf_get_string_list (gconf_key);
  group_content_iter = group_content;
  
  while (group_content_iter && group_content_iter->data) {

    gtk_list_store_append (list_store, &list_iter);

    contact_info =
      g_strsplit ((char *) group_content_iter->data, "|", 0);

    if (contact_info [0])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_NAME, contact_info [0], -1);
    if (contact_info [1])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_URL, contact_info [1], -1);

    if (contact_info [2])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_SPEED_DIAL, contact_info [2], -1);
    
    g_strfreev (contact_info);
    group_content_iter = g_slist_next (group_content_iter);
  }

  g_free (gconf_key);
  g_free (group_name);
  g_slist_free (group_content);

  gnomemeeting_addressbook_update_menu_sensitivity ();  
}


void
gnomemeeting_addressbook_sections_populate ()
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter, child_iter, selected_iter;

  GdkPixbuf *contact_icon = NULL;

  gchar *section = NULL;
  
  GSList *ldap_servers_list = NULL;
  GSList *ldap_servers_list_iter = NULL;
  GSList *groups_list = NULL;
  GSList *groups_list_iter = NULL;

  GmLdapWindow *lw = NULL;

  int p = 0, cpt = 0;
  int selected_page = 0;
  
  lw = GnomeMeeting::Process ()->GetLdapWindow ();
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
    
  if (gtk_tree_selection_get_selected (selection, &model, &selected_iter)) 
    path = gtk_tree_model_get_path (model, &selected_iter);    

  gtk_tree_store_clear (GTK_TREE_STORE (model));
  
  /* Populate the tree view : servers */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  contact_icon = 
    gtk_widget_render_icon (lw->tree_view, GM_STOCK_REMOTE_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_CONTACT_SECTION_NAME, _("Servers"), 
		      COLUMN_NOTEBOOK_PAGE, 0, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, -1);
  
  ldap_servers_list = gconf_get_string_list (CONTACTS_KEY "ldap_servers_list");     
  ldap_servers_list_iter = ldap_servers_list;
  while (ldap_servers_list_iter) {

    /* This will only add a notebook page if the server was not already
     * present */
    section =
      gconf_unescape_key ((char *) ldap_servers_list_iter->data,
			  strlen ((char *) ldap_servers_list_iter->data));
    p = 
      gnomemeeting_init_ldap_window_notebook ((char *) section,
					      CONTACTS_SERVERS);

    gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&child_iter, 
			COLUMN_PIXBUF, contact_icon,
			COLUMN_CONTACT_SECTION_NAME, section,
			COLUMN_NOTEBOOK_PAGE, p, 
			COLUMN_PIXBUF_VISIBLE, TRUE,
			COLUMN_WEIGHT, PANGO_WEIGHT_NORMAL, -1);

    ldap_servers_list_iter = ldap_servers_list_iter->next;

    g_free (section);
    cpt++;
  }
  g_slist_free (ldap_servers_list);
  g_object_unref (contact_icon);


  /* Populate the tree view : groups */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  contact_icon = 
    gtk_widget_render_icon (lw->tree_view, GM_STOCK_LOCAL_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_CONTACT_SECTION_NAME, _("Groups"), 
		      COLUMN_NOTEBOOK_PAGE, 0, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, -1);

  groups_list = gconf_get_string_list (CONTACTS_KEY "groups_list"); 
  groups_list_iter = groups_list;
  while (groups_list_iter) {

    /* This will only add a notebook page if the server was not already
     * present */
    section =
      gconf_unescape_key ((char *) groups_list_iter->data,
			  strlen ((char *) groups_list_iter->data));
    p = 
      gnomemeeting_init_ldap_window_notebook ((char *) section,
					      CONTACTS_GROUPS);

    gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&child_iter, 
			COLUMN_PIXBUF, contact_icon,
			COLUMN_CONTACT_SECTION_NAME, section,
			COLUMN_NOTEBOOK_PAGE, p,
			COLUMN_PIXBUF_VISIBLE, TRUE,
			COLUMN_WEIGHT, PANGO_WEIGHT_NORMAL, -1);

    groups_list_iter = groups_list_iter->next;
    g_free (section);
    cpt++;
  }
  g_slist_free (groups_list);
  g_object_unref (contact_icon);


  /* Expand servers and groups, and selects the last selected one
     or the first one */
  gtk_tree_view_expand_all (GTK_TREE_VIEW (lw->tree_view));
  if (path)
    gtk_tree_view_set_cursor (GTK_TREE_VIEW (lw->tree_view),
			      path, NULL, false);

  if (!path ||
      !gtk_tree_selection_path_is_selected (selection, path)) {

    path = gtk_tree_path_new_from_string ("0:0");
    gtk_tree_view_set_cursor (GTK_TREE_VIEW (lw->tree_view),
			      path, NULL, false);
  }

  if (gtk_tree_selection_get_selected (selection, &model, &selected_iter)) 
    gtk_tree_model_get (model, &selected_iter,
			COLUMN_CONTACT_SECTION_NAME, &selected_page, -1);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), selected_page);
  gtk_tree_path_free (path);  

  /* Update sensitivity */
  gnomemeeting_addressbook_update_menu_sensitivity ();
}


GMURL
gnomemeeting_addressbook_get_url_from_speed_dial (const char *speed_dial)
{
  gchar *group_content_gconf_key = NULL;
  gchar *group_name = NULL;
  char **contact_info = NULL;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  GSList *groups = NULL;
  GSList *groups_iter = NULL;

  GMURL url;
  
  if (!speed_dial || (speed_dial && !strcmp (speed_dial, "")))
    return url;

  groups = gconf_get_string_list (CONTACTS_KEY "groups_list");
  groups_iter = groups;
  
  while (groups_iter && groups_iter->data) {
    
    group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);
		       

    group_content = gconf_get_string_list (group_content_gconf_key);
    group_content_iter = group_content;
    
    while (group_content_iter
	   && group_content_iter->data && url.IsEmpty ()) {
      
      contact_info =
	g_strsplit ((char *) group_content_iter->data, "|", 0);

      if (contact_info [2] && strcmp (contact_info [2], "")
	  && !strcmp (contact_info [2], (const char *) speed_dial)) 
	url = GMURL (contact_info [1]);

      g_strfreev (contact_info);
      group_content_iter = g_slist_next (group_content_iter);
    }

    g_free (group_content_gconf_key);
    g_slist_free (group_content);

    groups_iter = g_slist_next (groups_iter);
    g_free (group_name);
  }

  g_slist_free (groups);

  return url;
}


GSList *
gnomemeeting_addressbook_get_speed_dials ()
{
  gchar *group_content_gconf_key = NULL;
  gchar *group_name = NULL;
  gchar *speed_dial_info = NULL;
  char **contact_info = NULL;
  char **rcontact_info = NULL;

  BOOL found = FALSE;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  GSList *groups = NULL;
  GSList *groups_iter = NULL;

  GSList *result = NULL;
  GSList *result_iter = NULL;

  groups = gconf_get_string_list (CONTACTS_KEY "groups_list");
  groups_iter = groups;
  
  while (groups_iter && groups_iter->data) {
    
    group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);
		       

    group_content = gconf_get_string_list (group_content_gconf_key);
    group_content_iter = group_content;
    
    while (group_content_iter
	   && group_content_iter->data) {
      
      contact_info =
	g_strsplit ((char *) group_content_iter->data, "|", 0);

      if (contact_info [2] && strcmp (contact_info [2], "")) {

	speed_dial_info = 
	  g_strdup_printf ("%s|%s", contact_info [0], contact_info [2]);

	result_iter = result;

	if (result_iter) {

	  while (result_iter && result_iter->data && !found) {

	    rcontact_info =
	      g_strsplit ((char *) result_iter->data, "|", 0);

	    if (rcontact_info [1] &&
		!strcmp (contact_info [2], rcontact_info [1]))
	      found = TRUE;

	    result_iter = g_slist_next (result_iter);
	    g_strfreev (rcontact_info);
	  }
	}

	if (!found) 
	  result =
	    g_slist_insert_sorted (result,
				   g_strdup (speed_dial_info),
				   (GCompareFunc) (speed_dials_compare));
				   

	found = FALSE;
	
	g_free (speed_dial_info);
      }

      g_strfreev (contact_info);
      group_content_iter = g_slist_next (group_content_iter);
    }

    g_free (group_content_gconf_key);
    g_slist_free (group_content);

    groups_iter = g_slist_next (groups_iter);
    g_free (group_name);
  }

  g_slist_free (groups);

  result = g_slist_nth (result, 0);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if the contact corresponding to the given URL
 *                 is member of the addressbook.
 * PRE          :  /
 *
 */
static gboolean
is_contact_member_of_addressbook (GMURL url)
{
  gchar *group_name = NULL;
  gchar *group_content_gconf_key = NULL;
  char **contact_info = NULL;

  gboolean result = false;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  GSList *groups = NULL;
  GSList *groups_iter = NULL;
  
  if (url.IsEmpty ())
    return result;

  groups = gconf_get_string_list (CONTACTS_KEY "groups_list");
  groups_iter = groups;
  
  while (groups_iter && groups_iter->data) {
    
    group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);
		       

    group_content = gconf_get_string_list (group_content_gconf_key);
    group_content_iter = group_content;
    
    while (group_content_iter && group_content_iter->data && !result) {
      
      contact_info =
	g_strsplit ((char *) group_content_iter->data, "|", 0);
      
      if (contact_info [1] && (GMURL (contact_info [1]) ==  url))
	result = true;

      g_strfreev (contact_info);
      group_content_iter = g_slist_next (group_content_iter);
    }

    g_free (group_content_gconf_key);
    g_free (group_name);
    g_slist_free (group_content);

    groups_iter = g_slist_next (groups_iter);
  }

  g_slist_free (groups);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if the group given as parameter already
 *                 is member of the addressbook. If is_group is true, the
 *                 given contact section will be searched in the groups, else
 *                 it will be searched in the servers.
 * PRE          :  /
 *
 */
static gboolean
is_contact_section_member_of_addressbook (const char *c_section,
					  BOOL is_group)
{
  gboolean result = false;

  gchar *contact_section = NULL;

  GSList *contacts_sections = NULL;
  GSList *contacts_sections_iter = NULL;
  
  if (!c_section)
    return result;

  contact_section = escape_contact_section (c_section, FALSE);
  
  if (is_group)
    contacts_sections = gconf_get_string_list (CONTACTS_KEY "groups_list");
  else
    contacts_sections = gconf_get_string_list (CONTACTS_KEY "ldap_servers_list");
  contacts_sections_iter = contacts_sections;
  
  while (contacts_sections_iter && contacts_sections_iter->data && !result) {

    if (contact_section &&
	(!strcasecmp (contact_section, (char *) contacts_sections_iter->data)
	 || !strcasecmp (c_section, (char *) contacts_sections_iter->data)))
      result = true;

    contacts_sections_iter = g_slist_next (contacts_sections_iter);
  }

  g_slist_free (contacts_sections);
  g_free (contact_section);
  
  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the escaped form of the given contact_section name.
 *                 That escaped form is converted to lower case if no_case is
 *                 TRUE.
 * PRE          :  /
 *
 */
static gchar *
escape_contact_section (const char *contact_section, BOOL no_case)
{
  gchar *contact_section_escaped = NULL;
  gchar *contact_section_no_case = NULL;

  if (no_case)
    contact_section_no_case = g_utf8_strdown ((char *) contact_section, -1);
  else
    contact_section_no_case = g_strdup ((char *) contact_section);

  contact_section_escaped =
    gconf_escape_key (contact_section_no_case,
		      strlen (contact_section_no_case));

  g_free (contact_section_no_case);

  return contact_section_escaped;
}


void
rename_contact_section (const char *contact_section_old,
			const char *contact_section_new,
			gboolean is_group)
{
  GSList *contacts_sections = NULL;
  GSList *contacts_sections_iter = NULL;
  GSList *old_list = NULL;
  
  gchar *old_name = NULL;
  gchar *new_name = NULL;
  gchar *new_name_down = NULL;
  gchar *old_gconf_key = NULL;
  gchar *new_gconf_key = NULL;
  gchar *gconf_key = NULL;

  int pos = 0;
  
  /* Do nothing if no contact section is specified or if they are 
   * identical */
  if (!contact_section_old || !contact_section_new 
      || !strcmp (contact_section_new, contact_section_old))
    return; 

  for (int i = 0 ; i < 2 ; i++) {

    if (i == 1)
      gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
    else
      gconf_key = g_strdup (CONTACTS_KEY "groups_list");
    
    contacts_sections = gconf_get_string_list (gconf_key);
    contacts_sections_iter = contacts_sections;

    old_name = escape_contact_section (contact_section_old);
    old_gconf_key = g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, old_name);

    new_name = escape_contact_section (contact_section_new, FALSE);
    new_name_down = g_utf8_strdown (new_name, -1);
    new_gconf_key = g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY,
				     new_name_down);
    

    while (contacts_sections_iter && contacts_sections_iter->data) {

      if ((!strcasecmp ((char *) contacts_sections_iter->data, old_name)
	  || !strcasecmp ((char *) contacts_sections_iter->data,
			  contact_section_old))
	  && (is_group == (i == 0))) {

	contacts_sections =
	  g_slist_remove_link (contacts_sections, contacts_sections_iter);
	g_slist_free_1 (contacts_sections_iter);

	contacts_sections =
	  g_slist_insert (contacts_sections, g_strdup (new_name), pos);

	/* If it was a group, copy the old group content to the new one
	   and unset the old key */
	if (i == 0) {
	  
	  old_list = gconf_get_string_list (old_gconf_key);
	
	  gconf_set_string_list (new_gconf_key, old_list);

	  gconf_client_remove_dir (gconf_client_get_default (),
				   "/apps/gnomemeeting", 0);
	  gconf_client_unset (gconf_client_get_default (),
			      old_gconf_key, NULL);
	  gconf_client_add_dir (gconf_client_get_default (),
				"/apps/gnomemeeting",
				GCONF_CLIENT_PRELOAD_RECURSIVE, 0);

	  g_slist_free (old_list);
	}
	
	contacts_sections_iter = contacts_sections;
      }

      contacts_sections_iter = g_slist_next (contacts_sections_iter);
      pos++;
    }    

    gconf_set_string_list (gconf_key, contacts_sections);

    g_slist_free (contacts_sections);
    g_slist_free (contacts_sections_iter);

    g_free (new_gconf_key);
    g_free (old_gconf_key);
    g_free (new_name);
    g_free (new_name_down);
    g_free (old_name);    
    g_free (gconf_key);
  }
}
