/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@ximian.com>
 *
 *  Copyright 2002 Iain Holmes
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
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <gconf/gconf-client.h>

#include "gnet.h"

static GConfClient *client = NULL;

typedef enum {
	CDDB_ACCESS_READWRITE,
	CDDB_ACCESS_READONLY,
	CDDB_ACCESS_NONE
} CDDBSlaveAccess;

typedef enum {
	CONNECTION_MODE_NEED_HELLO,
	CONNECTION_MODE_NEED_HELLO_RESPONSE,
	CONNECTION_MODE_NEED_SITES_RESPONSE,
	CONNECTION_MODE_NEED_GOODBYE
} ConnectionMode;

typedef struct _PropertyDialog {
	GtkWidget *dialog;

	GtkWidget *no_info;
	GtkWidget *real_info;
	GtkWidget *specific_info;
	GtkWidget *name_box;
	GtkWidget *real_name;
	GtkWidget *real_host;

	GtkWidget *round_robin;
	GtkWidget *other_freedb;
	GtkWidget *freedb_box;
	GtkWidget *freedb_server;
	GtkWidget *update;

	GtkWidget *other_server;
	GtkWidget *other_box;
	GtkWidget *other_host;
	GtkWidget *other_port;

	GtkTreeModel *model;

	/* Connection stuff */
	GTcpSocket *socket;
	GIOChannel *iochannel;
	guint tag;

	ConnectionMode mode;
	CDDBSlaveAccess access;
} PropertyDialog;

enum {
	CDDB_SEND_FAKE_INFO,
	CDDB_SEND_REAL_INFO,
	CDDB_SEND_OTHER_INFO
};

enum {
	CDDB_ROUND_ROBIN,
	CDDB_OTHER_FREEDB,
	CDDB_OTHER_SERVER
};

static void
destroy_window (GtkWidget *window,
		PropertyDialog *pd)
{
	g_free (pd);
	gtk_main_quit ();
}

static void
dialog_button_clicked_cb (GtkDialog *dialog,
			  int response_id)
{
	GError *error = NULL;
	switch (response_id) {
	case GTK_RESPONSE_HELP:
		gnome_help_display_desktop (NULL, "user-guide",
					    "user-guide.xml",
					    "goscustlookandfeel-39", &error);
		if (error) {
			GtkWidget *msg_dialog;
			msg_dialog = gtk_message_dialog_new (GTK_WINDOW(dialog),
							     GTK_DIALOG_MODAL,
							     GTK_MESSAGE_ERROR,
							     GTK_BUTTONS_CLOSE,
							     _("There was an error displaying help: \n%s"),
							     error->message);
			g_signal_connect (G_OBJECT (msg_dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);
			gtk_window_set_resizable (GTK_WINDOW (msg_dialog), FALSE);
			gtk_widget_show (msg_dialog);
			g_error_free (error);
                }
	break;

	default:
		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	}
}

static void
no_info_toggled (GtkToggleButton *tb,
		 PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gtk_widget_set_sensitive (pd->name_box, FALSE);
	gconf_client_set_int (client, "/apps/CDDB-Slave2/info", CDDB_SEND_FAKE_INFO, NULL);
}

static void
real_info_toggled (GtkToggleButton *tb,
		   PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gtk_widget_set_sensitive (pd->name_box, FALSE);
	gconf_client_set_int (client, "/apps/CDDB-Slave2/info", CDDB_SEND_REAL_INFO, NULL);
}

static void
specific_info_toggled (GtkToggleButton *tb,
		       PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gtk_widget_set_sensitive (pd->name_box, TRUE);
	gconf_client_set_int (client, "/apps/CDDB-Slave2/info", CDDB_SEND_OTHER_INFO, NULL);
}

static void
real_name_changed (GtkEntry *entry,
		   PropertyDialog *pd)
{
	gconf_client_set_string (client, "/apps/CDDB-Slave2/name",
				 gtk_entry_get_text (entry), NULL);
}

static void
real_host_changed (GtkEntry *entry,
		   PropertyDialog *pd)
{
	gconf_client_set_string (client, "/apps/CDDB-Slave2/hostname",
				 gtk_entry_get_text (entry), NULL);
}

#define DEFAULT_SERVER "freedb.freedb.org"
#define DEFAULT_PORT 888

static void
round_robin_toggled (GtkToggleButton *tb,
		     PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gtk_widget_set_sensitive (pd->freedb_box, FALSE);
	gtk_widget_set_sensitive (pd->other_box, FALSE);
	gconf_client_set_int (client, "/apps/CDDB-Slave2/server-type", CDDB_ROUND_ROBIN, NULL);
	gconf_client_set_string (client, "/apps/CDDB-Slave2/server", DEFAULT_SERVER, NULL);
	gconf_client_set_int (client, "/apps/CDDB-Slave2/port", DEFAULT_PORT, NULL);
}

static void
other_freedb_toggled (GtkToggleButton *tb,
		      PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gtk_widget_set_sensitive (pd->freedb_box, TRUE);
	gtk_widget_set_sensitive (pd->other_box, FALSE);

	gconf_client_set_int (client, "/apps/CDDB-Slave2/server-type", CDDB_OTHER_FREEDB, NULL);
	/* Set it to the default selection */
}

static void
other_server_toggled (GtkToggleButton *tb,
		      PropertyDialog *pd)
{
	const char *str;

	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gtk_widget_set_sensitive (pd->freedb_box, FALSE);
	gtk_widget_set_sensitive (pd->other_box, TRUE);

	gconf_client_set_int (client,
			      "/apps/CDDB-Slave2/server-type", CDDB_OTHER_SERVER, NULL);
	str = gtk_entry_get_text (GTK_ENTRY (pd->other_host));
	if (str != NULL) {
		gconf_client_set_string (client,
					 "/apps/CDDB-Slave2/server", str, NULL);
	}

	str = gtk_entry_get_text (GTK_ENTRY (pd->other_port));
	if (str != NULL) {
		gconf_client_set_int (client,
				      "/apps/CDDB-Slave2/port", atoi (str), NULL);
	}
}

static void
other_host_changed (GtkEntry *entry,
		    PropertyDialog *pd)
{
	gconf_client_set_string (client, "/apps/CDDB-Slave2/server",
				 gtk_entry_get_text (entry), NULL);
}

static void
other_port_changed (GtkEntry *entry,
		    PropertyDialog *pd)
{
	gconf_client_set_int (client, "/apps/CDDB-Slave2/port",
			      atoi (gtk_entry_get_text (entry)), NULL);
}

static void
do_goodbye (PropertyDialog *pd)
{
	guint bytes_writen;
	GIOError status;

	status = gnet_io_channel_writen (pd->iochannel, "quit\n",
					 5, &bytes_writen);
	pd->mode = CONNECTION_MODE_NEED_GOODBYE;
}

static gboolean
do_goodbye_response (PropertyDialog *pd,
		     const char *response)
{
	int code;

	code = atoi (response);
	switch (code) {
	case 230:
		g_print ("Disconnected\n");
		g_print ("%s\n", response);
		break;

	default:
		g_print ("Unknown response\n");
		g_print ("%s\n", response);
		break;
	}

	/* Disconnect */
	if (pd->tag) {
		g_source_remove (pd->tag);
	}

	gnet_tcp_socket_unref (pd->socket);
	g_io_channel_unref (pd->iochannel);

	pd->mode = CONNECTION_MODE_NEED_HELLO;
	gtk_widget_set_sensitive (pd->update, TRUE);

	return FALSE;
}

static gboolean
do_sites_response (PropertyDialog *pd,
		   const char *response)
{
	int code;
	static gboolean waiting_for_terminator = FALSE;
	gboolean more = FALSE;

	if (waiting_for_terminator == TRUE) {
		code = 210;
	} else {
		code = atoi (response);
	}

	switch (code) {
	case 210:
		if (response[0] == '.') {
			/* Terminator */
			waiting_for_terminator = FALSE;
			more = FALSE;
			break;
		}

		if (waiting_for_terminator == TRUE) {
			char **vector, *res, *end;
			GtkTreeIter iter;

			res = g_strdup (response);
			res[strlen (res) - 1] = 0;
			end = strchr (res, '\r');
			if (end != NULL) {
				*end = 0;
			}

			vector = g_strsplit (res, " ", 5);
			g_free (res);
			if (vector == NULL) {
				g_print ("Erk!\n");
				waiting_for_terminator = FALSE;
				return FALSE;
			}

			gtk_list_store_append (GTK_LIST_STORE (pd->model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (pd->model), &iter, 0, vector[0], 1, vector[1], 2, vector[4], -1);
			g_strfreev (vector);
		}

		waiting_for_terminator = TRUE;
		more = TRUE;

		break;

	case 401:
		g_print ("No site information available\n");
		g_print ("%s\n", response);
		more = FALSE;
		break;

	default:
		g_print ("Unknown response\n");
		g_print ("%s\n", response);
		more = FALSE;
		break;
	}

	if (more == FALSE) {
		do_goodbye (pd);
	}
	return more;
}

static void
do_sites (PropertyDialog *pd)
{
	guint bytes_writen;
	GIOError status;

	status = gnet_io_channel_writen (pd->iochannel, "sites\n",
					 6, &bytes_writen);
	pd->mode = CONNECTION_MODE_NEED_SITES_RESPONSE;
}

static gboolean
do_hello_response (PropertyDialog *pd,
		   const char *response)
{
	int code;

	code = atoi (response);
	switch (code) {
	case 200:
		g_print ("Hello ok - Welcome\n");
		g_print ("%s\n", response);
		break;

	case 431:
		g_print ("Hello unsuccessful\n");
		g_print ("%s\n", response);

		do_goodbye (pd);
		break;

	case 402:
		g_print ("Already shook hands\n");
		g_print ("%s\n", response);
		break;

	default:
		g_print ("Unknown response\n");
		g_print ("%s\n", response);
		break;
	}

	if (pd->access != CDDB_ACCESS_NONE) {
		do_sites (pd);
	}

	return FALSE;
}

static void
do_hello (PropertyDialog *pd)
{
	char *hello;
	guint bytes_writen;
	GIOError status;

	/* Use the gconf values */
	hello = g_strdup_printf ("cddb hello %s %s CDDBSlave2 %s\n", "johnsmith",
				 "198.172.174.22", VERSION);
	status = gnet_io_channel_writen (pd->iochannel, hello,
					 strlen (hello), &bytes_writen);

	pd->mode = CONNECTION_MODE_NEED_HELLO_RESPONSE;
}

static gboolean
do_open_response (PropertyDialog *pd,
		  const char *response)
{
	int code;

	/* Did we get the hello? */
	code = atoi (response);
	switch (code) {
	case 200:
		g_print ("Hello ok - Read/Write access allowed\n");
		g_print ("%s\n", response);
		pd->access = CDDB_ACCESS_READWRITE;
		break;

	case 201:
		g_print ("Hello ok - Read only access\n");
		g_print ("%s\n", response);
		pd->access = CDDB_ACCESS_READONLY;
		break;

	case 432:
		g_print ("No more connections allowed\n");
		g_print ("%s\n", response);
		pd->access = CDDB_ACCESS_NONE;
		break;

	case 433:
		g_print ("No connections allowed: X users allowed, Y currently active\n");
		g_print ("%s\n", response);
		pd->access = CDDB_ACCESS_NONE;
		break;

	case 434:
		g_print ("No connections allowed: system load too high\n");
		g_print ("%s\n", response);
		pd->access = CDDB_ACCESS_NONE;
		break;

	default:
		g_print ("Unknown response\n");
		g_print ("%s\n", response);
		pd->access = CDDB_ACCESS_NONE;
		break;
	}

	if (pd->access != CDDB_ACCESS_NONE) {
		do_hello (pd);
	} else {
		do_goodbye (pd);
	}

	return FALSE;
}

static gboolean
read_from_server (GIOChannel *iochannel,
		  GIOCondition condition,
		  gpointer data)
{
	PropertyDialog *pd = data;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		g_warning ("Socket error");
		goto error;
	}

	if (condition & (G_IO_IN | G_IO_PRI)) {
		GIOError error;
		char *buffer;
		gsize bytes_read;

		/* Read the data into our buffer */
		error = g_io_channel_read_line (iochannel, &buffer,
						&bytes_read, NULL, NULL);
		while (error == G_IO_STATUS_NORMAL) {
			gboolean more = FALSE;

			switch (pd->mode) {
			case CONNECTION_MODE_NEED_HELLO:
				more = do_open_response (pd, buffer);
				break;

			case CONNECTION_MODE_NEED_HELLO_RESPONSE:
				more = do_hello_response (pd, buffer);
				break;

			case CONNECTION_MODE_NEED_SITES_RESPONSE:
				more = do_sites_response (pd, buffer);
				break;

			case CONNECTION_MODE_NEED_GOODBYE:
				more = do_goodbye_response (pd, buffer);
				break;

			default:
				g_print ("Dunno what to do with %s\n", buffer);
				more = FALSE;
				break;
			}

			g_free (buffer);

			if (more == TRUE) {
				error = g_io_channel_read_line (iochannel, &buffer,
								&bytes_read, NULL, NULL);
			} else {
				break;
			}
		}
	}

	return TRUE;

 error:
	gtk_widget_set_sensitive (pd->update, TRUE);
	return FALSE;
}

static void
open_cb (GTcpSocket *sock,
	 GInetAddr *addr,
	 GTcpSocketConnectAsyncStatus status,
	 gpointer data)
{
	PropertyDialog *pd = data;
	GIOChannel *sin;

	pd->socket = sock;
	if (status != GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK) {
		g_warning ("Error updating server list");
		gtk_widget_set_sensitive (pd->update, TRUE);
		return;
	}

	sin = gnet_tcp_socket_get_iochannel (sock);
	pd->iochannel = sin;

	pd->tag = g_io_add_watch (sin, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
				  read_from_server, data);
}

static void
update_clicked (GtkButton *update,
		PropertyDialog *pd)
{
	GTcpSocketConnectAsyncID *sock;
	GtkTreeIter iter;

	/* Clear the list and put the default there */
	gtk_list_store_clear (GTK_LIST_STORE (pd->model));
	gtk_list_store_append (GTK_LIST_STORE (pd->model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (pd->model), &iter, 0, "freedb.freedb.org", 1, "888", 2, "Random server", -1);

	gtk_widget_set_sensitive (pd->update, FALSE);
	/* Should use the gconf values */
	sock = gnet_tcp_socket_connect_async ("freedb.freedb.org", 888,
					      open_cb, pd);
	if (sock == NULL) {
		g_warning ("Could not update server list");
	}
}

static void
server_selection_changed (GtkTreeSelection *selection,
			  PropertyDialog *pd)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (selection, &model, &iter) == TRUE) {
		char *server;
		char *port;

		gtk_tree_model_get (model, &iter, 0, &server, 1, &port, -1);
		gconf_client_set_string (client, "/apps/CDDB-Slave2/server", server, NULL);
		gconf_client_set_int (client, "/apps/CDDB-Slave2/port", atoi (port), NULL);
	}
}

static GtkTreeModel *
make_tree_model (void)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	/* Default */
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "freedb.freedb.org", 1, "888", 2, "Random server", -1);

	return GTK_TREE_MODEL (store);
}

static GtkWidget *
hig_category_new (GtkWidget *parent, gchar *title, gboolean expand, gboolean fill)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkWidget *label;
	gchar *tmp;

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (parent), vbox, expand, fill, 0);

	tmp = g_strdup_printf ("<b>%s</b>", _(title));
	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	return vbox2;
}

static void
create_dialog (GtkWidget *window)
{
	PropertyDialog *pd;

	GtkWidget *main_vbox;
	GtkWidget *frame;
	GtkWidget *align;
	GtkWidget *vbox, *hbox, *hbox2, *hbox3;
	GtkWidget *label, *sw;
	GtkWidget *icon;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *col;
	GtkTreeSelection *selection;

	char *str;
	int info = CDDB_SEND_FAKE_INFO;
	int port;

	pd = g_new (PropertyDialog, 1);
	pd->dialog = window;
	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (destroy_window), pd);

	main_vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_set_spacing (GTK_BOX (main_vbox), 18);
	gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (window)->vbox), 2);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), main_vbox, TRUE, TRUE, 0);

	/* Log on info */
	frame = hig_category_new (main_vbox, _("Login Information"), FALSE, FALSE);
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	info = gconf_client_get_int (client, "/apps/CDDB-Slave2/info", NULL);
	g_print ("info: %d\n", info);
	pd->no_info = gtk_radio_button_new_with_mnemonic (NULL, _("Sen_d no information"));
	if (info == CDDB_SEND_FAKE_INFO) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->no_info), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->no_info), "toggled",
			  G_CALLBACK (no_info_toggled), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->no_info, FALSE, FALSE, 0);

	pd->real_info = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->no_info),
									_("Send real _information"));
	if (info == CDDB_SEND_REAL_INFO) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->real_info), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->real_info), "toggled",
			  G_CALLBACK (real_info_toggled), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->real_info, FALSE, FALSE, 0);

	pd->specific_info = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->real_info),
									    _("Send _other information:"));
	if (info == CDDB_SEND_OTHER_INFO) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->specific_info), TRUE);
	}

	g_signal_connect (G_OBJECT (pd->specific_info), "toggled",
			  G_CALLBACK (specific_info_toggled), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->specific_info, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	pd->name_box = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (pd->name_box), 6);
	gtk_table_set_col_spacings (GTK_TABLE (pd->name_box), 12);
	if (info != CDDB_SEND_OTHER_INFO) {
		gtk_widget_set_sensitive (pd->name_box, FALSE);
	}

	gtk_box_pack_start (GTK_BOX (hbox), pd->name_box, TRUE, TRUE, 0);

	align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
	label = gtk_label_new_with_mnemonic (_("_Name:"));
	gtk_container_add (GTK_CONTAINER (align), label);
	gtk_table_attach (GTK_TABLE (pd->name_box), align,
			  0, 1, 0, 1, GTK_FILL, GTK_FILL,
			  0, 0);

	pd->real_name = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), pd->real_name);
	str = gconf_client_get_string (client, "/apps/CDDB-Slave2/name", NULL);
	if (str != NULL) {
		gtk_entry_set_text (GTK_ENTRY (pd->real_name), str);
		g_free (str);
	}
	g_signal_connect (G_OBJECT (pd->real_name), "changed",
			  G_CALLBACK (real_name_changed), pd);
	gtk_table_attach (GTK_TABLE (pd->name_box), pd->real_name,
			  1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL,
			  0, 0);

	align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
	label = gtk_label_new_with_mnemonic (_("Hostna_me:"));
	gtk_container_add (GTK_CONTAINER (align), label);
	gtk_table_attach (GTK_TABLE (pd->name_box), align,
			  0, 1, 1, 2, GTK_FILL, GTK_FILL,
			  0, 0);

	pd->real_host = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), pd->real_host);
	str = gconf_client_get_string (client, "/apps/CDDB-Slave2/hostname", NULL);
	if (str != NULL) {
		gtk_entry_set_text (GTK_ENTRY (pd->real_host), str);
		g_free (str);
	}
	g_signal_connect (G_OBJECT (pd->real_host), "changed",
			  G_CALLBACK (real_host_changed), pd);
	gtk_table_attach (GTK_TABLE (pd->name_box), pd->real_host,
			  1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL,
			  0, 0);

	/* Server info */
	frame = hig_category_new (main_vbox, _("Server"), TRUE, TRUE);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	info = gconf_client_get_int (client, "/apps/CDDB-Slave2/server-type", NULL);
	pd->round_robin = gtk_radio_button_new_with_mnemonic (NULL, _("FreeDB _round robin server"));
	g_signal_connect (G_OBJECT (pd->round_robin), "toggled",
			  G_CALLBACK (round_robin_toggled), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->round_robin, FALSE, FALSE, 0);

	pd->other_freedb = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->round_robin),
									   _("Other _FreeDB server:"));
	g_signal_connect (G_OBJECT (pd->other_freedb), "toggled",
			  G_CALLBACK (other_freedb_toggled), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->other_freedb, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	pd->freedb_box = gtk_vbox_new (FALSE, 6);
	gtk_widget_set_sensitive (pd->freedb_box, FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), pd->freedb_box, TRUE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					     GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX (pd->freedb_box), sw, TRUE, TRUE, 2);

	pd->model = make_tree_model ();
	pd->freedb_server = gtk_tree_view_new_with_model (pd->model);
	g_object_unref (pd->model);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pd->freedb_server));
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (server_selection_changed), pd);

	cell = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Server"), cell,
							"text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pd->freedb_server), col);
	col = gtk_tree_view_column_new_with_attributes (_("Port"), cell,
							"text", 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pd->freedb_server), col);
	col = gtk_tree_view_column_new_with_attributes (_("Location"), cell,
							"text", 2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pd->freedb_server), col);

	gtk_container_add (GTK_CONTAINER (sw), pd->freedb_server);

	/* create the update server list button */
	align = gtk_alignment_new (1.0, 0.5, 0.0, 0.0);

	pd->update = gtk_button_new ();
 	g_signal_connect (G_OBJECT (pd->update), "clicked",
 			  G_CALLBACK (update_clicked), pd);

	gtk_container_add (GTK_CONTAINER (align), pd->update);

	gtk_box_pack_start (GTK_BOX (pd->freedb_box), align, FALSE, FALSE, 0);

	/* ... and it's contents */
	align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (pd->update), align);

	hbox = gtk_hbox_new (FALSE, 2);

	gtk_container_add (GTK_CONTAINER (align), hbox);

	icon = gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_BUTTON);

	gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic (_("_Update Server List"));

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), pd->update);

	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	pd->other_server = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->other_freedb),
									   _("Other _server:"));
	g_signal_connect (G_OBJECT (pd->other_server), "toggled",
			  G_CALLBACK (other_server_toggled), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->other_server, FALSE, FALSE, 0);

	pd->other_box = gtk_vbox_new (TRUE, 0);
	gtk_widget_set_sensitive (pd->other_box, FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), pd->other_box, FALSE, FALSE, 0);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (pd->other_box), hbox3, FALSE, FALSE, 0);

	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (hbox3), hbox2, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (hbox2), hbox, FALSE, FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("Hos_tname:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	pd->other_host = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), pd->other_host);
	g_signal_connect (G_OBJECT (pd->other_host), "changed",
			  G_CALLBACK (other_host_changed), pd);
	str = gconf_client_get_string (client, "/apps/CDDB-Slave2/server", NULL);
	if (str != NULL) {
		gtk_entry_set_text (GTK_ENTRY (pd->other_host), str);
		g_free (str);
	}
	gtk_box_pack_start (GTK_BOX (hbox), pd->other_host, TRUE, TRUE, 0);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (hbox2), hbox, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic (_("_Port:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	pd->other_port = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), pd->other_port);
	g_signal_connect (G_OBJECT (pd->other_port), "changed",
			  G_CALLBACK (other_port_changed), pd);
	port = gconf_client_get_int (client, "/apps/CDDB-Slave2/port", NULL);
	str = g_strdup_printf ("%d", port);
	gtk_entry_set_text (GTK_ENTRY (pd->other_port), str);
	g_free (str);

	gtk_box_pack_start (GTK_BOX (hbox), pd->other_port, FALSE, FALSE, 0);

	switch (info) {
	case CDDB_ROUND_ROBIN:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->round_robin), TRUE);
		break;

	case CDDB_OTHER_FREEDB:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->other_freedb), TRUE);
		break;

	case CDDB_OTHER_SERVER:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->other_server), TRUE);
		break;

	default:
		break;
	}
}

static GdkPixbuf *
pixbuf_from_file (const char *filename)
{
	GdkPixbuf *pixbuf;
	char *fullname;

	fullname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
                   filename, TRUE, NULL);
	g_return_val_if_fail (fullname != NULL, NULL);

	pixbuf = gdk_pixbuf_new_from_file (fullname, NULL);
	g_free (fullname);

	return pixbuf;
}

int
main (int argc,
      char **argv)
{
	GtkWidget *dialog_win;
	GdkPixbuf *pixbuf;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init (argv[0], VERSION, LIBGNOMEUI_MODULE, argc, argv, NULL);

	client = gconf_client_get_default ();
	gconf_client_add_dir (client, "/apps/CDDB-Slave2",
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);

	dialog_win = gtk_dialog_new_with_buttons (_("CD Database Preferences"),
						  NULL, -1,
						  GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						  NULL);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog_win), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_win), 5);

	gtk_window_set_default_size (GTK_WINDOW (dialog_win), 440, 570);
	create_dialog (dialog_win);

	gtk_dialog_set_default_response(GTK_DIALOG (dialog_win), GTK_RESPONSE_CLOSE);

  	g_signal_connect (G_OBJECT (dialog_win), "response",
  			  G_CALLBACK (dialog_button_clicked_cb), NULL);

	pixbuf = pixbuf_from_file ("gnome-cd/cd.png");
	if (pixbuf == NULL) {
		g_warning ("Error finding gnome-cd/cd.png");
	} else {
		gtk_window_set_icon (GTK_WINDOW (dialog_win), pixbuf);
		g_object_unref (G_OBJECT (pixbuf));
	}

	gtk_widget_show_all (dialog_win);

	gtk_main ();

	return 0;
}
