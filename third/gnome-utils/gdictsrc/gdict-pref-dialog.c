/* $Id: gdict-pref-dialog.c,v 1.1.1.5 2004-10-04 05:06:29 ghudson Exp $ */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict preferences window
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <gnome.h>
#include <gconf/gconf-client.h>
#include "gdict-pref-dialog.h"
#include "gdict-app.h"

static void gdict_pref_dialog_class_init (GDictPrefDialogClass *class);
static void gdict_pref_dialog_init (GDictPrefDialog *pref_dialog);
static void gdict_pref_dialog_finalize (GObject *object);

static void pref_dialog_add_db      (GDictPrefDialog *pref_dialog, 
                                     gchar *db, gchar *desc);
static void pref_dialog_add_strat   (GDictPrefDialog *pref_dialog, 
                                     gchar *strat, gchar *desc);
static void pref_dialog_reset_db    (GDictPrefDialog *pref_dialog);
static void pref_dialog_reset_strat (GDictPrefDialog *pref_dialog);

static void pref_set_strat_cb       (GtkWidget *widget, gpointer data);
static void pref_set_db_cb          (GtkWidget *widget, gpointer data);

static void pref_error_cb           (dict_command_t *command, 
                                     DictStatusCode code, 
                                     gchar *message, gpointer data);
static void pref_status_cb          (dict_command_t *command, 
                                     DictStatusCode code, 
                                     int num_found, gpointer data);
static void pref_data_cb            (dict_command_t *command, dict_res_t *res,
                                     gpointer data);

static void response_cb		    (GtkDialog *dialog, gint response);
static gboolean	set_server_cb 	    (GtkWidget *widget, GdkEventFocus *event, gpointer data);
static gboolean set_port_cb	    (GtkWidget *widget, GdkEventFocus *event, gpointer data);
static void set_default_server      (GtkButton *button, gpointer data);
static void set_default_port        (GtkButton *button, gpointer data);
static void smart_lookup_cb	    (GtkToggleButton *toggle, gpointer data);


enum {
    SOCKET_ERROR_SIGNAL,
    LAST_SIGNAL
};

static gint gdict_pref_dialog_signals[LAST_SIGNAL] = { 0 };

static GtkDialogClass *parent_class = NULL;

GtkWidget *error_label1 = NULL;
GtkWidget *error_label2 = NULL;
GtkWidget *alignment1 = NULL;
GtkWidget *alignment2 = NULL;


/* gdict_pref_dialog_get_type
 *
 * Register the GDictPrefDialog type with Gtk's type system if necessary and
 * return the type identifier code
 */

GType
gdict_pref_dialog_get_type (void)
{
    static GType gdict_pref_dialog_type = 0;
    
    g_type_init ();
    
    if (!gdict_pref_dialog_type) {
        static const GTypeInfo gdict_pref_dialog_info = {
            sizeof (GDictPrefDialogClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gdict_pref_dialog_class_init,
            NULL,
            NULL,
            sizeof (GDictPrefDialog),
            0,
            (GInstanceInitFunc) gdict_pref_dialog_init,
        };
        
        gdict_pref_dialog_type = g_type_register_static (GTK_TYPE_DIALOG, 
        					         "GDictPrefDialog", 
        					         &gdict_pref_dialog_info,
        					         0);
    }
    
    return gdict_pref_dialog_type;
}

/* gdict_pref_dialog_class_init
 *
 * Initialises a structure describing the GDictPrefDialog class; sets
 * up signals for pref_dialog events in the Gtk signal management
 * system
 */

static void 
gdict_pref_dialog_class_init (GDictPrefDialogClass *class) 
{
    GObjectClass *object_class;
    
    object_class = G_OBJECT_CLASS (class);
    parent_class = g_type_class_peek_parent (class);

    gdict_pref_dialog_signals[SOCKET_ERROR_SIGNAL] =
        g_signal_new ("socket_error",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GDictPrefDialogClass, socket_error),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_STRING);

    object_class->finalize = gdict_pref_dialog_finalize;
}

/* gdict_pref_dialog_init
 *
 * Initialises an instance of a GDictPrefDialog object
 */

static void 
gdict_pref_dialog_init (GDictPrefDialog *pref_dialog)
{
    GtkWidget *server_label, *port_label;
    GtkWidget *vbox;
    GtkWidget *button;    
    gchar *port_str;
    
    pref_dialog->context = NULL;

    pref_dialog->database = gdict_pref.database;
    pref_dialog->dfl_strat = gdict_pref.dfl_strat;
    
    gtk_window_set_title (GTK_WINDOW (pref_dialog), _("Dictionary Preferences"));
    gtk_window_set_policy (GTK_WINDOW (pref_dialog), FALSE, FALSE, FALSE);    
    gtk_container_set_border_width (GTK_CONTAINER (pref_dialog), 5);
    vbox = GTK_DIALOG (pref_dialog)->vbox;
    gtk_box_set_spacing (GTK_BOX (vbox), 2);
    
    pref_dialog->table = GTK_TABLE (gtk_table_new (5, 3, FALSE));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pref_dialog)->vbox), GTK_WIDGET (pref_dialog->table), TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (pref_dialog->table), 5);
    gtk_table_set_row_spacings (pref_dialog->table, 6);
    gtk_table_set_col_spacings (pref_dialog->table, 12);

    pref_dialog->smart_lookup_btn = 
        GTK_CHECK_BUTTON (gtk_check_button_new_with_mnemonic (_("Smart _lookup")));
    g_signal_connect (G_OBJECT (pref_dialog->smart_lookup_btn), "toggled",
    		      G_CALLBACK (smart_lookup_cb), pref_dialog);
    gtk_table_attach_defaults (pref_dialog->table, GTK_WIDGET (pref_dialog->smart_lookup_btn),
    			       1, 3, 2, 3);
    if ( ! gconf_client_key_is_writable (gdict_get_gconf_client (), "/apps/gnome-dictionary/smart", NULL))
	    gtk_widget_set_sensitive (GTK_WIDGET (pref_dialog->smart_lookup_btn), FALSE);
    
    pref_dialog->server_entry = GTK_ENTRY (gtk_entry_new ());			       
    gtk_table_attach (pref_dialog->table, 
                      GTK_WIDGET (pref_dialog->server_entry),
                      1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
		    
    g_signal_connect (G_OBJECT (pref_dialog->server_entry), "focus_out_event",
    		      G_CALLBACK (set_server_cb), pref_dialog);
    
    server_label = gtk_label_new_with_mnemonic (_("_Server:"));
    gtk_label_set_mnemonic_widget (GTK_LABEL (server_label), 
    				   GTK_WIDGET (pref_dialog->server_entry));
    gtk_label_set_justify (GTK_LABEL (server_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (server_label), 0, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, server_label, 0, 1, 0, 1);
    
    button = gtk_button_new_with_mnemonic (_("De_fault Server"));
    gtk_table_attach (pref_dialog->table, 
                      button, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    g_signal_connect (G_OBJECT (button), "clicked",
    		      G_CALLBACK (set_default_server), pref_dialog);

    if (gail_loaded)
        add_atk_namedesc(GTK_WIDGET(button), NULL, _("Reset server to default"));

    if ( ! gconf_client_key_is_writable (gdict_get_gconf_client (), "/apps/gnome-dictionary/server", NULL)) {
	    gtk_widget_set_sensitive (GTK_WIDGET (pref_dialog->server_entry), FALSE);
	    gtk_widget_set_sensitive (server_label, FALSE);
	    gtk_widget_set_sensitive (button, FALSE);
    }

    port_label = gtk_label_new_with_mnemonic (_("_Port:"));
    gtk_table_attach_defaults (pref_dialog->table, port_label, 0, 1, 1, 2);
    gtk_label_set_justify (GTK_LABEL (port_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (port_label), 0, 0.5);

    pref_dialog->port_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_label_set_mnemonic_widget (GTK_LABEL (port_label), 
    				   GTK_WIDGET (pref_dialog->port_entry));
    gtk_table_attach (pref_dialog->table, 
                      GTK_WIDGET (pref_dialog->port_entry),
                      1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    g_signal_connect (G_OBJECT (pref_dialog->port_entry), "focus_out_event",
    		      G_CALLBACK (set_port_cb), pref_dialog);

    button = gtk_button_new_with_mnemonic (_("Def_ault Port"));
    gtk_table_attach (pref_dialog->table, 
                      button, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    g_signal_connect (G_OBJECT (button), "clicked",
    		      G_CALLBACK (set_default_port), pref_dialog);

    if (gail_loaded) {
        add_atk_namedesc(GTK_WIDGET(button), NULL, _("Reset port to default"));
        add_atk_namedesc(server_label, _("Server"), _("Server"));
        add_atk_namedesc( GTK_WIDGET (pref_dialog->server_entry), _("Server Entry"), _("Enter the Server Name"));

        add_atk_namedesc(port_label, _("Port"), _("Port"));
        add_atk_namedesc( GTK_WIDGET(pref_dialog->port_entry), _("Port Entry"), _("Enter the Port Number"));
    }

    if ( ! gconf_client_key_is_writable (gdict_get_gconf_client (), "/apps/gnome-dictionary/port", NULL)) {
	    gtk_widget_set_sensitive (GTK_WIDGET (pref_dialog->port_entry), FALSE);
	    gtk_widget_set_sensitive (port_label, FALSE);
	    gtk_widget_set_sensitive (button, FALSE);
    }

    pref_dialog->db_label = gtk_label_new_with_mnemonic (_("_Database:"));
    gtk_label_set_justify (GTK_LABEL (pref_dialog->db_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (pref_dialog->db_label), 0, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, pref_dialog->db_label, 0, 1, 3, 4);
    pref_dialog->db_list = GTK_MENU (gtk_menu_new ());
    gtk_widget_show (GTK_WIDGET (pref_dialog->db_list));
    
    pref_dialog->strat_label = gtk_label_new_with_mnemonic (_("D_efault strategy:"));
    gtk_label_set_justify (GTK_LABEL (pref_dialog->strat_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (pref_dialog->strat_label), 0, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, pref_dialog->strat_label, 0, 1, 4, 5);
    pref_dialog->strat_list = GTK_MENU (gtk_menu_new ());
    gtk_widget_show (GTK_WIDGET (pref_dialog->strat_list));
    
    gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));

    gtk_dialog_add_buttons (GTK_DIALOG (pref_dialog), GTK_STOCK_CLOSE,
    			    GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);  

    gtk_dialog_set_default_response (GTK_DIALOG (pref_dialog), GTK_RESPONSE_CLOSE);
    gtk_dialog_set_has_separator (GTK_DIALOG (pref_dialog), FALSE);
    
    g_signal_connect (G_OBJECT (pref_dialog), "response",
    		      G_CALLBACK (response_cb), NULL);

    /* To prevent dialog desctruction when pressing ESC */
    g_signal_connect (G_OBJECT (pref_dialog), "delete_event", 
		      G_CALLBACK (gtk_true), NULL);

    if (gdict_pref.server != NULL)
	    gtk_entry_set_text (GTK_ENTRY (pref_dialog->server_entry),
				gdict_pref.server);
    port_str = g_strdup_printf("%d", (int) gdict_pref.port);
    gtk_entry_set_text (GTK_ENTRY (pref_dialog->port_entry), port_str);
    g_free(port_str);
    gtk_toggle_button_set_active
	(GTK_TOGGLE_BUTTON (pref_dialog->smart_lookup_btn), gdict_pref.smart);

    gtk_widget_show_all (GTK_WIDGET (pref_dialog));
}

/* gdict_pref_dialog_finalize
 *
 * Finalize a GDictPrefDialog object
 */

static void
gdict_pref_dialog_finalize (GObject *object)
{
    GDictPrefDialog *pref_dialog;

    g_return_if_fail (GDICT_IS_PREF_DIALOG (object));

    pref_dialog = GDICT_PREF_DIALOG (object);
    dict_command_destroy (pref_dialog->get_db_cmd);
    dict_command_destroy (pref_dialog->get_strat_cmd);

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* gdict_pref_dialog_new
 *
 * Creates a new GDictPrefDialog object
 */

GtkWidget *
gdict_pref_dialog_new (dict_context_t *context) 
{
    GDictPrefDialog *pref_dialog;
    
    /* g_return_val_if_fail (context != NULL, NULL); */
    /* the above is commented out because it prints ugly debug messages
     * to the console */
    if (context == NULL)
	    return NULL;

    pref_dialog = GDICT_PREF_DIALOG (g_object_new (GDICT_TYPE_PREF_DIALOG, NULL));
    pref_dialog->context = context;    
    pref_dialog_reset_db (pref_dialog);
    pref_dialog_reset_strat (pref_dialog);

    return GTK_WIDGET (pref_dialog);
}

/* pref_set_db_cb
 *
 * Sets the current search strategy to the one indicated
 */

static void
pref_set_db_cb (GtkWidget *widget, gpointer data) 
{
    GDictPrefDialog *pref_dialog;

    pref_dialog = GDICT_PREF_DIALOG (data);
    pref_dialog->database = g_object_get_data (G_OBJECT (widget), "db_name");

    gconf_client_set_string (gdict_get_gconf_client (), "/apps/gnome-dictionary/database", pref_dialog->database, NULL);
}

/* pref_set_strat_cb
 *
 * Sets the current search strategy to the one indicated
 */

static void
pref_set_strat_cb (GtkWidget *widget, gpointer data) 
{
    GDictPrefDialog *pref_dialog;
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    pref_dialog->dfl_strat = g_object_get_data (G_OBJECT (widget), "strat_name");

    gconf_client_set_string (gdict_get_gconf_client (), "/apps/gnome-dictionary/strategy", pref_dialog->dfl_strat, NULL);
}

/* pref_dialog_add_db
 *
 * Adds a database to the database list
 */

static void
pref_dialog_add_db (GDictPrefDialog *pref_dialog, gchar *db,
		    gchar *desc) 
{
    GtkWidget *menu_item;
    
    menu_item = gtk_menu_item_new_with_label (desc);
    g_signal_connect (G_OBJECT (menu_item), "activate", 
                      G_CALLBACK (pref_set_db_cb), pref_dialog);
    g_object_set_data (G_OBJECT (menu_item), "db_name", db);
    gtk_widget_show (menu_item);
    gtk_menu_append (pref_dialog->db_list, menu_item);

    if ((pref_dialog->database == NULL)
		    || (!strcmp (pref_dialog->database, db)))
      gtk_menu_set_active (pref_dialog->db_list, pref_dialog->database_idx);
    pref_dialog->database_idx++;
}

/* pref_dialog_add_strat
 *
 * Adds a search strategy to the search strategy list
 */

static void
pref_dialog_add_strat (GDictPrefDialog *pref_dialog, gchar *strat,
		       gchar *desc) 
{
    GtkWidget *menu_item;
    
    menu_item = gtk_menu_item_new_with_label (desc);
    g_signal_connect (G_OBJECT (menu_item), "activate", 
                      G_CALLBACK (pref_set_strat_cb), pref_dialog);
    g_object_set_data (G_OBJECT (menu_item), "strat_name", strat);
    gtk_widget_show (menu_item);
    gtk_menu_append (pref_dialog->strat_list, menu_item);
    
    if ((pref_dialog->dfl_strat == NULL)
		    || (!strcmp (pref_dialog->dfl_strat, strat)))
	gtk_menu_set_active (pref_dialog->strat_list,
			     pref_dialog->dfl_strat_idx);
    pref_dialog->dfl_strat_idx++;
}

/* pref_dialog_reset_db
 *
 * Resets the database option menu and begins a command to retrieve it from
 * the server again
 */

static void
pref_dialog_reset_db (GDictPrefDialog *pref_dialog)
{
    if (pref_dialog->get_db_cmd)
      dict_command_destroy (pref_dialog->get_db_cmd);
    
    if (pref_dialog->db_sel) {
        gtk_option_menu_remove_menu (pref_dialog->db_sel);
        gtk_widget_destroy (GTK_WIDGET (pref_dialog->db_sel));
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
        pref_dialog->db_sel = NULL;
    }

    pref_dialog->db_list = GTK_MENU (gtk_menu_new ());
    
    pref_dialog->get_db_cmd = dict_show_db_command_new ();
    pref_dialog->get_db_cmd->error_notify_cb = pref_error_cb;
    pref_dialog->get_db_cmd->data_notify_cb = pref_data_cb;
    pref_dialog->get_db_cmd->status_notify_cb = pref_status_cb;
    pref_dialog->get_db_cmd->user_data = pref_dialog;
    pref_dialog->database_idx = 0;

    if ((alignment1) && (error_label1)) {
	gtk_container_remove (GTK_CONTAINER (alignment1), error_label1);
	gtk_widget_destroy (GTK_WIDGET (alignment1));
	error_label1 = NULL;
	alignment1 = NULL;
    }
    
    pref_dialog_add_db (pref_dialog, "*", _("Search all databases"));

    if (dict_command_invoke (pref_dialog->get_db_cmd,
			     pref_dialog->context) == -1)
    {
	/* Could not look up search strategies, so just display a
	 * label; FIXME: Memory leak
	 */
	error_label1 = GTK_WIDGET (gtk_label_new (_("Cannot connect to server")));
	alignment1 = GTK_WIDGET (gtk_alignment_new (0, 0.5, 0, 0));
	gtk_container_add (GTK_CONTAINER (alignment1), error_label1);
	gtk_table_attach_defaults (pref_dialog->table, alignment1, 1, 2, 3, 4);
	gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
    }
}

/* pref_dialog_reset_strat
 *
 * Resets the strategies option menu and begins a command to retrieve it from
 * the server again
 */

static void
pref_dialog_reset_strat (GDictPrefDialog *pref_dialog)
{
    if (pref_dialog->get_strat_cmd)
	dict_command_destroy (pref_dialog->get_strat_cmd);
    
    if (pref_dialog->strat_sel) {
        gtk_option_menu_remove_menu (pref_dialog->strat_sel);
        gtk_widget_destroy (GTK_WIDGET (pref_dialog->strat_sel));
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
        pref_dialog->strat_sel = NULL;
    }
    
    pref_dialog->strat_list = GTK_MENU (gtk_menu_new ());
    
    pref_dialog->get_strat_cmd = dict_show_strat_command_new ();
    pref_dialog->get_strat_cmd->error_notify_cb = pref_error_cb;
    pref_dialog->get_strat_cmd->data_notify_cb = pref_data_cb;
    pref_dialog->get_strat_cmd->status_notify_cb = pref_status_cb;
    pref_dialog->get_strat_cmd->user_data = pref_dialog;
    pref_dialog->dfl_strat_idx = 0;

    if ((alignment2) && (error_label2)) {
	gtk_container_remove (GTK_CONTAINER (alignment2), error_label2);
	gtk_widget_destroy (GTK_WIDGET (alignment2));
	error_label2 = NULL;
	alignment2 = NULL;
    }
    
    if (dict_command_invoke (pref_dialog->get_strat_cmd,
			     pref_dialog->context) == -1) 
    {
	/* Could not look up search strategies, so just display a
	 * label; FIXME: Memory leak
	 */
	error_label2 = GTK_WIDGET (gtk_label_new (_("Cannot connect to server")));
	alignment2 = GTK_WIDGET (gtk_alignment_new (0, 0.5, 0, 0));
	gtk_container_add (GTK_CONTAINER (alignment2), error_label2);
	gtk_table_attach_defaults (pref_dialog->table, alignment2, 1, 2, 4, 5);
	gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
    }
}

/* pref_error_cb
 *
 * Callback used when there is a socket error
 */

static void
pref_error_cb (dict_command_t *command, DictStatusCode code, 
               gchar *message, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (GDICT_IS_PREF_DIALOG (data));
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    
    if (code != DICT_SOCKET_ERROR) {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                  	 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                  	 "%s", message); 
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
    }
    else {
        g_signal_emit (G_OBJECT (pref_dialog),
                       gdict_pref_dialog_signals[SOCKET_ERROR_SIGNAL], 0,
                       message);
    }
}

/* pref_data_cb
 *
 * Callback used when a new database or strategy definition has arrived 
 * over the link
 */

static void
pref_data_cb (dict_command_t *command, dict_res_t *res, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (GDICT_IS_PREF_DIALOG (data));
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    if (command->cmd == C_SHOW_DB)
	pref_dialog_add_db (pref_dialog, res->name, res->desc);
    else if (command->cmd == C_SHOW_STRAT)
	pref_dialog_add_strat (pref_dialog, res->name, res->desc);
}

/* pref_status_cb
 *
 * Callback used when a status code has arrived over the link
 */

static void 
pref_status_cb (dict_command_t *command, DictStatusCode code, 
                int num_found, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    GtkWidget *alignment;
    GtkOptionMenu *option_menu = NULL;
    GtkMenu *use_menu = NULL;
    gint row = 0;
    gboolean writable;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (GDICT_IS_PREF_DIALOG (data));
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    
    if (code == DICT_STATUS_OK) {
        if (command->cmd == C_SHOW_DB) {
            row = 3;
            use_menu = pref_dialog->db_list;
        }
        else if (command->cmd == C_SHOW_STRAT) {
            row = 4;
            use_menu = pref_dialog->strat_list;
        }
        
        option_menu = GTK_OPTION_MENU (gtk_option_menu_new ());
        gtk_option_menu_set_menu (option_menu, GTK_WIDGET (use_menu));

        gtk_table_attach_defaults (pref_dialog->table, GTK_WIDGET (option_menu), 
                                   1, 3, row, row + 1);
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
        
        if (command->cmd == C_SHOW_DB) {
	    pref_dialog->db_sel = option_menu;
            gtk_label_set_mnemonic_widget (GTK_LABEL (pref_dialog->db_label),
                                           GTK_WIDGET (pref_dialog->db_sel));

            if (gail_loaded) {
                add_atk_namedesc(pref_dialog->db_label, _("Database"), _("Database Name"));
                add_atk_relation( GTK_WIDGET (option_menu), pref_dialog->db_label, ATK_RELATION_LABELLED_BY);

            }

            writable = gconf_client_key_is_writable (gdict_get_gconf_client (), "/apps/gnome-dictionary/database", NULL);

	    gtk_widget_set_sensitive (pref_dialog->db_label, writable);
	    gtk_widget_set_sensitive (GTK_WIDGET (option_menu), writable);
        }
        else if (command->cmd == C_SHOW_STRAT) {
	    pref_dialog->strat_sel = option_menu;
            gtk_label_set_mnemonic_widget (GTK_LABEL (pref_dialog->strat_label),
                                           GTK_WIDGET (pref_dialog->strat_sel));

            if (gail_loaded) {
                add_atk_namedesc(pref_dialog->strat_label, _("Default Strategy"), _("Default Strategy"));
                add_atk_relation( GTK_WIDGET (option_menu), pref_dialog->strat_label, ATK_RELATION_LABELLED_BY);

            }

            writable = gconf_client_key_is_writable (gdict_get_gconf_client (), "/apps/gnome-dictionary/strategy", NULL);

	    gtk_widget_set_sensitive (pref_dialog->strat_label, writable);
	    gtk_widget_set_sensitive (GTK_WIDGET (option_menu), writable);
        }
    }
}

static void 
response_cb (GtkDialog *dialog, gint response)
{
    if (response == GTK_RESPONSE_HELP) {
        GError *error = NULL;

        gnome_help_display ("gnome-dictionary", "gdict-settings", &error);
        if (error) {
            GtkWidget *msg_dialog;

            msg_dialog = gtk_message_dialog_new (GTK_WINDOW (dialog),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 _("There was an error displaying help: \n%s"),
                                                 error->message);

            g_signal_connect (G_OBJECT (msg_dialog), "response",
                              G_CALLBACK (gtk_widget_destroy), NULL);

            gtk_widget_show (msg_dialog);
            g_error_free (error);
       }

       return;
    }

    gtk_widget_hide (GTK_WIDGET (dialog));
}

static gboolean
set_server_cb (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    gchar *server;

    g_return_val_if_fail (GDICT_IS_PREF_DIALOG (data), FALSE);

    pref_dialog = data;
    server = gtk_editable_get_chars (GTK_EDITABLE (pref_dialog->server_entry), 0, -1);
    if (!server)
        return FALSE;

    if ((gdict_pref.server == NULL) || (strcmp (gdict_pref.server, server))) {
	if (gdict_pref.server)
		g_free (gdict_pref.server);
        gdict_pref.server = g_strdup(server);
        gdict_init_context ();
        pref_dialog->context = context;
        pref_dialog_reset_db (pref_dialog);
        pref_dialog_reset_strat (pref_dialog);
           
        gconf_client_set_string (gdict_get_gconf_client (), "/apps/gnome-dictionary/server", gdict_pref.server, NULL);

    }

    g_free (server);

    return FALSE;
}

static gboolean 
set_port_cb (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    gchar *port;

    g_return_val_if_fail (GDICT_IS_PREF_DIALOG (data), FALSE);

    pref_dialog = data;
    port = gtk_editable_get_chars (GTK_EDITABLE (pref_dialog->port_entry), 0, -1);
    if (gdict_pref.port != atoi (port)) {
    
        gdict_pref.port = atoi (port);
        gdict_init_context ();
        
        pref_dialog->context = context;
        
        pref_dialog_reset_db (pref_dialog);
        pref_dialog_reset_strat (pref_dialog);
            
        gconf_client_set_int (gdict_get_gconf_client (), "/apps/gnome-dictionary/port", gdict_pref.port, NULL);
    }
    
    g_free (port);

    return FALSE;
}

static void
set_default_server (GtkButton *button, gpointer data)
{
    GDictPrefDialog *pref_dialog;

    g_return_if_fail (GDICT_IS_PREF_DIALOG (data));

    pref_dialog = data;
    gtk_entry_set_text (GTK_ENTRY (pref_dialog->server_entry), DICT_DEFAULT_SERVER);
    set_server_cb (GTK_WIDGET (pref_dialog->port_entry), NULL, pref_dialog);
}

static void
set_default_port (GtkButton *button, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    gchar *port;

    g_return_if_fail (GDICT_IS_PREF_DIALOG (data));

    pref_dialog = data;
    port = g_strdup_printf ("%d", DICT_DEFAULT_PORT);
    gtk_entry_set_text (GTK_ENTRY (pref_dialog->port_entry), port);
    g_free (port);
    set_port_cb (GTK_WIDGET (pref_dialog->port_entry), NULL, pref_dialog);
}    

static void 
smart_lookup_cb	(GtkToggleButton *toggle, gpointer data)
{
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active (toggle);
    if (toggled != gdict_pref.smart)
        gconf_client_set_bool (gdict_get_gconf_client (), "/apps/gnome-dictionary/smart", toggled, NULL);
}

