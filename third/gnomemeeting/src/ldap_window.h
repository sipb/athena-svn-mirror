
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
 *                         ldap_window.h  -  description
 *                         -----------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */


#ifndef _LDAP_WINDOW_H_
#define _LDAP_WINDOW_H_

#include "common.h"
#include "urlhandler.h"


/* The different cell renderers for the ILS browser */
enum {

  COLUMN_ILS_STATUS,
  COLUMN_ILS_AUDIO,
  COLUMN_ILS_VIDEO,
  COLUMN_ILS_NAME,
  COLUMN_ILS_COMMENT,
  COLUMN_ILS_LOCATION,
  COLUMN_ILS_URL,
  COLUMN_ILS_VERSION,
  COLUMN_ILS_IP,
  COLUMN_ILS_COLOR,
  NUM_COLUMNS_SERVERS
};

 
/* The different cell renderers for the local contacts */
enum {

  COLUMN_NAME,
  COLUMN_URL,
  COLUMN_SPEED_DIAL,
  NUM_COLUMNS_GROUPS
};


/* The different cell renderers for the different contacts sections (servers
   or groups */
enum {

  COLUMN_PIXBUF,
  COLUMN_CONTACT_SECTION_NAME,
  COLUMN_NOTEBOOK_PAGE,
  COLUMN_PIXBUF_VISIBLE,
  COLUMN_WEIGHT,
  NUM_COLUMNS_CONTACTS
};


/* The functions  */
void gnomemeeting_addressbook_edit_contact_dialog (gchar *url);

void gnomemeeting_addressbook_edit_contact_dialog (gchar *section,
						   gchar *name,
						   gchar *url,
						   gchar *speeddial);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build the LDAP window and returns it.
 * PRE          :  /
 */
GtkWidget *gnomemeeting_ldap_window_new (GmLdapWindow *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Destroy all notebook pages after having wait that the
 *                 searches are terminated.
 * PRE          :  /
 */
void gnomemeeting_ldap_window_destroy_notebook_pages ();


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the ldap window page.
 * PRE          :  The GtkWidget must be a pointer to a page of the addressbook
 *                 notebook.
 */
GmLdapWindowPage *gnomemeeting_get_ldap_window_page (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Build the notebook inside the LDAP window if the server
 *                 name was not already present. Returns its page number
 *                 if it was already present.
 * PRE          :  The server name, the type of page to create 
 *                 (CONTACTS_SERVERS / CONTACTS_GROUPS)
 */
int gnomemeeting_init_ldap_window_notebook (gchar *,
					    int);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Fills in the GtkListStore with the members of the group
 *                 given as second parameter.
 * PRE          :  /
 */
void gnomemeeting_addressbook_group_populate (GtkListStore *,
					      char *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Fills in the arborescence of servers and groups.
 * PRE          :  /
 */
void gnomemeeting_addressbook_sections_populate ();


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the GMURL for the given speed dial or NULL if none.
 * PRE          :  /
 */
GMURL gnomemeeting_addressbook_get_url_from_speed_dial (const char *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the list of user name|speed dials or NULL if none.
 * PRE          :  /
 */
GSList *gnomemeeting_addressbook_get_speed_dials ();


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Updates the sensitivity of the different menu items
 *                 following what is selected in the addressbook (section,
 *                 contact in a group or in ILS) and following if we are
 *                 in a call or not.
 * PRE          :  /
 */
void gnomemeeting_addressbook_update_menu_sensitivity ();


/* DESCRIPTION  :  This callback is called when there is an "event_after"
 *                 signal on one of the contacts.
 * BEHAVIOR     :  Displays a popup menu with the required options.
 * PRE          :  gpointer = 0 if the contact is clicked in the addressbook,
 *                 1 otherwise.
 */
gint contact_clicked_cb (GtkWidget *,
			 GdkEventButton *,
			 gpointer);


/* DESCRIPTION  :  This callback is called when the user double clicks on
 *                 a row corresonding to an user.
 * BEHAVIOR     :  Add the user name in the combo box and call him or transfer
 *                 the call to that user.
 * PRE          :  data is the page type or 3 if contact activated from calls history
 */
void contact_activated_cb (GtkTreeView *,
			   GtkTreePath *,
			   GtkTreeViewColumn *,
			   gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Rename the contact section (group or server) given as
 *                 parameter under its non-escaped form into the other
 *                 parameter.
 * PRE          :  Valid old contact section name, new contact section name.
 */
void rename_contact_section (const char *,
                             const char *,
                             gboolean);
#endif
