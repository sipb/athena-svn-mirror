/* bug-buddy bug submitting program
 *
 * Copyright (C) 2004 Fernando Herrera
 *
 * Author:  Fernando Herrera <fherrera@onirica.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "bug-buddy.h"

#include <gnome.h>
#include <gconf/gconf-client.h>


extern GConfClient *conf_client;

static
void entry_changed_notify (GConfClient *client,
			   guint        cnxn_id,
			   GConfEntry  *entry,
			   gpointer     user_data)
{

	GtkWidget *gtkentry = user_data;
	g_return_if_fail (GTK_IS_ENTRY (gtkentry));

	if (gconf_entry_get_value (entry) == NULL) {
		gtk_entry_set_text (GTK_ENTRY (gtkentry), "");
	} else if (gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING) {
		gtk_entry_set_text (GTK_ENTRY (gtkentry),
				    gconf_value_get_string (gconf_entry_get_value (entry)));
	}
}

static gboolean
config_entry_commit (GtkWidget *entry)
{
	gchar *text;
	const gchar *key;
	GConfClient *client;
                                                                                                                             
	client = g_object_get_data (G_OBJECT (entry), "client");
                                                                                                                             
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
                                                                                                                             
	key = g_object_get_data (G_OBJECT (entry), "key");
                                                                                                                             
	/* Unset if the string is zero-length, otherwise set */
	if (*text != '\0') {
		gconf_client_set_string (client, key, text, NULL);
	} else {
		gconf_client_unset (client, key, NULL);
	}

	g_free (text);
	
	return FALSE;
}



void
init_gconf_stuff (void)
{

	gchar *tmp;
	GtkWidget *entry;
	guint notify_id;

	conf_client = gconf_client_get_default ();
	gconf_client_add_dir (conf_client, "/apps/bug-buddy",
			      GCONF_CLIENT_PRELOAD_ONELEVEL,
			      NULL);

	tmp = gconf_client_get_string (conf_client,
					"/apps/bug-buddy/name", NULL);
	if (tmp==NULL) {
		tmp = g_strdup (g_get_real_name ());
	}
	if (tmp!=NULL) {
		buddy_set_text ("email-name-entry", tmp);
		g_free (tmp);
	}

	tmp = gconf_client_get_string (conf_client,
					"/apps/bug-buddy/email_address", NULL);
	if (tmp!=NULL) {
		buddy_set_text ("email-email-entry", tmp);
		g_free (tmp);
	}


	tmp = gconf_client_get_string (conf_client,
					"/apps/bug-buddy/mailer", NULL);
	if (tmp!=NULL && tmp!="" && g_file_test (tmp, G_FILE_TEST_EXISTS)) {
		buddy_set_text ("email-sendmail-entry", tmp);
		g_free (tmp);
	} else {
		tmp = g_find_program_in_path ("sendmail");
		if (tmp==NULL) {
			if (g_file_test ("/usr/sbin/sendmail", G_FILE_TEST_EXISTS)) {
				tmp = g_strdup ("/usr/sbin/sendmail");
			} else if (g_file_test ("/usr/lib/sendmail", G_FILE_TEST_EXISTS)) {
				tmp = g_strdup ("/usr/lib/sendmail");
			}
		}
		if (tmp!=NULL) {
			buddy_set_text ("email-sendmail-entry", tmp);
			gconf_client_set_string (conf_client, 
						 "/apps/bug-buddy/mailer",
						 tmp, NULL);
			g_free (tmp);
		}

	}

	tmp = gconf_client_get_string (conf_client,
					"/apps/bug-buddy/bugfile", NULL);
	if (tmp!=NULL) {
		buddy_set_text ("email-file-entry", tmp);
		g_free (tmp);
	}


	druid_data.last_update_check = gconf_client_get_int (conf_client,
							     "/apps/bug-buddy/last_update_check",
							     NULL);
	
	druid_data.state = 0;

	entry = GET_WIDGET ("email-name-entry");
	g_object_set_data (G_OBJECT (entry), "client", conf_client);
	g_object_set_data (G_OBJECT (entry), "key", "/apps/bug-buddy/name");
	g_signal_connect (G_OBJECT (entry), "focus_out_event",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	g_signal_connect (G_OBJECT (entry), "activate",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	notify_id = gconf_client_notify_add (conf_client,
					     "/apps/bug-buddy/name",
					     entry_changed_notify,
					     entry,
					     NULL, NULL);

	entry = GET_WIDGET ("email-email-entry");
	g_object_set_data (G_OBJECT (entry), "client", conf_client);
	g_object_set_data (G_OBJECT (entry), "key", "/apps/bug-buddy/email_address");
	g_signal_connect (G_OBJECT (entry), "focus_out_event",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	g_signal_connect (G_OBJECT (entry), "activate",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	notify_id = gconf_client_notify_add (conf_client,
					     "/apps/bug-buddy/email_address",
					     entry_changed_notify,
					     entry,
					     NULL, NULL);

	entry = GET_WIDGET ("email-sendmail-entry");
	g_object_set_data (G_OBJECT (entry), "client", conf_client);
	g_object_set_data (G_OBJECT (entry), "key", "/apps/bug-buddy/mailer");
	g_signal_connect (G_OBJECT (entry), "focus_out_event",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	g_signal_connect (G_OBJECT (entry), "activate",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	notify_id = gconf_client_notify_add (conf_client,
					     "/apps/bug-buddy/mailer",
					     entry_changed_notify,
					     entry,
					     NULL, NULL);

	entry = GET_WIDGET ("email-file-entry");
	g_object_set_data (G_OBJECT (entry), "client", conf_client);
	g_object_set_data (G_OBJECT (entry), "key", "/apps/bug-buddy/bugfile");
	g_signal_connect (G_OBJECT (entry), "focus_out_event",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	g_signal_connect (G_OBJECT (entry), "activate",
			  G_CALLBACK (config_entry_commit),
			  NULL);
	notify_id = gconf_client_notify_add (conf_client,
					     "/apps/bug-buddy/bugfile",
					     entry_changed_notify,
					     entry,
					     NULL, NULL);


}

