/* $Id: gdict-pref-dialog.c,v 1.1.1.4 2003-01-29 20:32:42 ghudson Exp $ */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict main window
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

enum {
    APPLY_CHANGES_SIGNAL,
    SOCKET_ERROR_SIGNAL,
    LAST_SIGNAL
};

static gchar *typeface_sel_names[NUM_TYPEFACES] = {
    N_("Headword:"),
    N_("Sub-number:"),
    N_("Pronunciation:"),
    N_("Etymology:"),
    N_("Part of speech:"),
    N_("Example:"),
    N_("Main body:"),
    N_("Cross-reference:")
};

#define NUM_TYPEFACE_ROWS 4
#define NUM_TYPEFACE_COLS 2

GtkWidget *db_label, *strat_label;
GtkWidget *error_label1 = NULL;
GtkWidget *error_label2 = NULL;
GtkWidget *alignment1 = NULL;
GtkWidget *alignment2 = NULL;

static gint gdict_pref_dialog_signals[LAST_SIGNAL] = { 0 };

static void gdict_pref_dialog_init (GDictPrefDialog *pref_dialog);
static void gdict_pref_dialog_class_init (GDictPrefDialogClass *class);

static void pref_dialog_add_db      (GDictPrefDialog *pref_dialog, 
                                     gchar *db, gchar *desc);
static void pref_dialog_add_strat   (GDictPrefDialog *pref_dialog, 
                                     gchar *strat, gchar *desc);
static void pref_dialog_reset_db    (GDictPrefDialog *pref_dialog);
static void pref_dialog_reset_strat (GDictPrefDialog *pref_dialog);
static gint pref_dialog_update_pref (GDictPrefDialog *pref_dialog,
                                     gboolean save_prefs);

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

static void pref_dialog_ok_cb       (GtkWidget *widget, gpointer data);
static void pref_dialog_apply_cb    (GtkWidget *widget, gpointer data);
static void pref_dialog_close_cb    (GtkWidget *widget, gpointer data);
static void phelp_cb		    (GtkDialog *dialog, gint response);
static void response_cb		    (GtkDialog *dialog, gint response);
static gboolean	set_server_cb 	    (GtkWidget *widget, GdkEventFocus *event, gpointer data);
static gboolean set_port_cb	    (GtkWidget *widget, GdkEventFocus *event, gpointer data);
static void set_default_server      (GtkButton *button, gpointer data);
static void set_default_port        (GtkButton *button, gpointer data);
static void smart_lookup_cb	    (GtkToggleButton *toggle, gpointer data);


/* gdict_pref_dialog_get_type
 *
 * Register the GDictPrefDialog type with Gtk's type system if necessary and
 * return the type identifier code
 */

GType
gdict_pref_dialog_get_type (void) {
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

/* gdict_pref_dialog_init
 *
 * Initialises an instance of a GDictPrefDialog object
 */

static void 
gdict_pref_dialog_init (GDictPrefDialog *pref_dialog) {

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
    GtkObjectClass *object_class;
    
    object_class = (GtkObjectClass *) class;
    
    gdict_pref_dialog_signals[APPLY_CHANGES_SIGNAL] =
        gtk_signal_new ("apply_changes", GTK_RUN_FIRST,
			GTK_CLASS_TYPE(object_class),
                        GTK_SIGNAL_OFFSET (GDictPrefDialogClass,
					   apply_changes),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_pref_dialog_signals[SOCKET_ERROR_SIGNAL] =
        gtk_signal_new ("socket_error", GTK_RUN_FIRST,
			GTK_CLASS_TYPE(object_class),
                        GTK_SIGNAL_OFFSET (GDictPrefDialogClass, socket_error),
                        g_cclosure_marshal_VOID__STRING, GTK_TYPE_NONE, 1,
                        GTK_TYPE_STRING);
#if 0    
    gtk_object_class_add_signals (object_class, gdict_pref_dialog_signals,
                                  LAST_SIGNAL);
#endif    
    object_class->destroy = (void (*) (GtkObject *)) gdict_pref_dialog_destroy;
    
    class->apply_changes = NULL;
}

static void
create_dialog (GDictPrefDialog *pref_dialog)
{
    GtkWidget *server_label, *port_label, *alignment;
    GtkWidget *vbox, *table;
    GtkWidget *button;    
    gchar *port_str;
    gint i;
    
    pref_dialog->database = gdict_pref.database;
    pref_dialog->dfl_strat = gdict_pref.dfl_strat;
    
    gtk_window_set_title (GTK_WINDOW (pref_dialog), _("Dictionary Preferences"));
    gtk_window_set_policy (GTK_WINDOW (pref_dialog), FALSE, FALSE, FALSE);    
    vbox = GTK_DIALOG (pref_dialog)->vbox;

    
    pref_dialog->table = GTK_TABLE (gtk_table_new (5, 3, FALSE));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pref_dialog)->vbox), GTK_WIDGET (pref_dialog->table), TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (pref_dialog->table), 8);
    gtk_table_set_row_spacings (pref_dialog->table, 6);
    gtk_table_set_col_spacings (pref_dialog->table, 6);

    pref_dialog->smart_lookup_btn = 
        GTK_CHECK_BUTTON (gtk_check_button_new_with_mnemonic (_("Smart _lookup")));
    g_signal_connect (G_OBJECT (pref_dialog->smart_lookup_btn), "toggled",
    		      G_CALLBACK (smart_lookup_cb), pref_dialog);
    gtk_table_attach_defaults (pref_dialog->table, GTK_WIDGET (pref_dialog->smart_lookup_btn),
    			       1, 3, 2, 3);
    
    pref_dialog->server_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_table_attach_defaults (pref_dialog->table, 
                               GTK_WIDGET (pref_dialog->server_entry), 
                               1, 2, 0, 1);
    g_signal_connect (G_OBJECT (pref_dialog->server_entry), "focus_out_event",
    		      G_CALLBACK (set_server_cb), pref_dialog);
    
    server_label = gtk_label_new_with_mnemonic (_("_Server:"));
    gtk_label_set_mnemonic_widget (GTK_LABEL (server_label), 
    				   GTK_WIDGET (pref_dialog->server_entry));
    gtk_label_set_justify (GTK_LABEL (server_label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (server_label), 1, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, server_label, 0, 1, 0, 1);
    
    button = gtk_button_new_with_mnemonic (_("De_fault Server"));
    gtk_table_attach_defaults (pref_dialog->table, button, 2, 3, 0, 1);
    g_signal_connect (G_OBJECT (button), "clicked",
    		      G_CALLBACK (set_default_server), pref_dialog);

    if( gail_loaded )
        add_atk_namedesc(GTK_WIDGET(button), NULL, _("Reset server to default"));

    port_label = gtk_label_new_with_mnemonic (_("_Port:"));
    gtk_table_attach_defaults (pref_dialog->table, port_label, 0, 1, 1, 2);
    gtk_label_set_justify (GTK_LABEL (port_label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (port_label), 1, 0.5);

    pref_dialog->port_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_label_set_mnemonic_widget (GTK_LABEL (port_label), 
    				   GTK_WIDGET (pref_dialog->port_entry));
    gtk_table_attach_defaults (pref_dialog->table, 
                               GTK_WIDGET (pref_dialog->port_entry), 
                               1, 2, 1, 2);
    g_signal_connect (G_OBJECT (pref_dialog->port_entry), "focus_out_event",
    		      G_CALLBACK (set_port_cb), pref_dialog);
                               
    button = gtk_button_new_with_mnemonic (_("Def_ault Port"));
    gtk_table_attach_defaults (pref_dialog->table, button, 2, 3, 1, 2);
    g_signal_connect (G_OBJECT (button), "clicked",
    		      G_CALLBACK (set_default_port), pref_dialog);
    
    if( gail_loaded )
    {
        add_atk_namedesc(GTK_WIDGET(button), NULL, _("Reset port to default"));
        add_atk_namedesc(server_label, _("Server"), _("Server"));
        add_atk_namedesc( GTK_WIDGET (pref_dialog->server_entry), _("Server Entry"), _("Enter the Server Name"));
        add_atk_relation( GTK_WIDGET (pref_dialog->server_entry), server_label, ATK_RELATION_LABELLED_BY);

        add_atk_namedesc(port_label, _("Port"), _("Port"));
        add_atk_namedesc( GTK_WIDGET(pref_dialog->port_entry), _("Port Entry"), _("Enter the Port Number"));
        add_atk_relation( GTK_WIDGET(pref_dialog->port_entry), port_label, ATK_RELATION_LABELLED_BY);

    }

    db_label = gtk_label_new_with_mnemonic (_("_Database:"));
    gtk_label_set_justify (GTK_LABEL (db_label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (db_label), 1, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, db_label, 0, 1, 3, 4);
    pref_dialog->db_list = GTK_MENU (gtk_menu_new ());
    gtk_widget_show (GTK_WIDGET (pref_dialog->db_list));
    
    strat_label = gtk_label_new_with_mnemonic (_("D_efault strategy:"));
    gtk_label_set_justify (GTK_LABEL (strat_label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (strat_label), 1, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, strat_label, 0, 1, 4, 5);
    pref_dialog->strat_list = GTK_MENU (gtk_menu_new ());
    gtk_widget_show (GTK_WIDGET (pref_dialog->strat_list));
    
    gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));

    gtk_dialog_add_buttons (GTK_DIALOG (pref_dialog), GTK_STOCK_CLOSE,
    			    GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);  

    gtk_dialog_set_default_response (GTK_DIALOG (pref_dialog), GTK_RESPONSE_CLOSE);
    
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

/* gdict_pref_dialog_new
 *
 * Creates a new GDictPrefDialog object
 */

GtkWidget *
gdict_pref_dialog_new (dict_context_t *context) 
{
    GDictPrefDialog *pref_dialog;
    GtkLabel *label;
    
    /* g_return_val_if_fail (context != NULL, NULL); */
    /* the above is commented out because it prints ugly debug messages
     * to the console */
    if (context == NULL)
	    return NULL;
    
    pref_dialog =
	GDICT_PREF_DIALOG (gtk_type_new (gdict_pref_dialog_get_type ()));
    pref_dialog->context = context;    
    create_dialog (pref_dialog);
    pref_dialog_reset_db (pref_dialog);
    pref_dialog_reset_strat (pref_dialog);
   
    return GTK_WIDGET (pref_dialog);
}

/* gdict_pref_dialog_destroy
 *
 * Destroys a pref_dialog dialog
 */

void
gdict_pref_dialog_destroy (GDictPrefDialog *pref_dialog) 
{
    dict_command_destroy (pref_dialog->get_db_cmd);
    dict_command_destroy (pref_dialog->get_strat_cmd);
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
    pref_dialog->database = 
        gtk_object_get_data (GTK_OBJECT (widget), "db_name");
        
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
    pref_dialog->dfl_strat = 
        gtk_object_get_data (GTK_OBJECT (widget), "strat_name");
    
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
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", 
                        GTK_SIGNAL_FUNC (pref_set_db_cb), pref_dialog);
    gtk_object_set_data (GTK_OBJECT (menu_item), "db_name", db);
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
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", 
                        GTK_SIGNAL_FUNC (pref_set_strat_cb), pref_dialog);
    gtk_object_set_data (GTK_OBJECT (menu_item), "strat_name", strat);
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
pref_dialog_reset_db (GDictPrefDialog *pref_dialog) {
    GtkWidget *error_label, *alignment;

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

    if ((alignment1) && (error_label1))
    {
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
pref_dialog_reset_strat (GDictPrefDialog *pref_dialog) {
    GtkWidget *error_label, *alignment;

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

    if ((alignment2) && (error_label2))
    {
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
    g_return_if_fail (IS_GDICT_PREF_DIALOG (data));
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    
    if (code != DICT_SOCKET_ERROR) {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                  	 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                  	 "%s", message, NULL); 
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
    }
    else {
        gtk_signal_emit (GTK_OBJECT (pref_dialog),
                         gdict_pref_dialog_signals[SOCKET_ERROR_SIGNAL],
                         message);
    }
}

/* pref_data_cb
 *
 * Callback used when a new database or strategy definition has arrived 
 * over the link
 */

static void
pref_data_cb (dict_command_t *command, dict_res_t *res, gpointer data) {
    GDictPrefDialog *pref_dialog;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (data));
    
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
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (data));
    
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
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        /*gtk_container_add (GTK_CONTAINER (alignment),
			   GTK_WIDGET (option_menu));*/
        gtk_table_attach_defaults (pref_dialog->table, GTK_WIDGET (option_menu), 
                                   1, 3, row, row + 1);
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
        
        if (command->cmd == C_SHOW_DB)
        {
	    pref_dialog->db_sel = option_menu;
            gtk_label_set_mnemonic_widget (GTK_LABEL (db_label), GTK_WIDGET (pref_dialog->db_sel));

            if ( gail_loaded)
            {
                add_atk_namedesc(db_label, _("Database"), _("Database Name"));
                add_atk_relation( GTK_WIDGET (option_menu), db_label, ATK_RELATION_LABELLED_BY);

            }
        }
        else if (command->cmd == C_SHOW_STRAT)
        {
	    pref_dialog->strat_sel = option_menu;
            gtk_label_set_mnemonic_widget (GTK_LABEL (strat_label), GTK_WIDGET (pref_dialog->strat_sel));

            if ( gail_loaded )
            {
                add_atk_namedesc(strat_label, _("Default Strategy"), _("Default Strategy"));
                add_atk_relation( GTK_WIDGET (option_menu), strat_label, ATK_RELATION_LABELLED_BY);

            }
        }
    }
}

static void 
response_cb (GtkDialog *dialog, gint response)
{
    if(response == GTK_RESPONSE_HELP)
    {
       phelp_cb (dialog, response);
       return;
    }
    gtk_widget_hide (GTK_WIDGET (dialog));
}

static gboolean
set_server_cb (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    GDictPrefDialog *pref_dialog = data;
    gchar *server;
    server = gtk_editable_get_chars (GTK_EDITABLE (pref_dialog->server_entry), 0, -1);

    g_return_val_if_fail (server != NULL, FALSE);

    if ((server != NULL) && ((gdict_pref.server == NULL)
			    || (strcmp (gdict_pref.server, server)))) {
	
	if (gdict_pref.server)
		g_free (gdict_pref.server);
        gdict_pref.server = g_strdup(server);
        gdict_init_context ();
        pref_dialog->context = context;
        pref_dialog_reset_db (pref_dialog);
        pref_dialog_reset_strat (pref_dialog);
           
        gconf_client_set_string (gdict_get_gconf_client (), "/apps/gnome-dictionary/server", gdict_pref.server, NULL);

    }
   
    if (server)
    	g_free (server);
    	
    return FALSE;
}


static gboolean 
set_port_cb (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    GDictPrefDialog *pref_dialog = data;
    gchar *port;
    gint old_port;
    
    port = gtk_editable_get_chars (GTK_EDITABLE (pref_dialog->port_entry), 0, -1);
    
    if (gdict_pref.port != atoi (port)) {
    
        gdict_pref.port = atoi (port);
        gdict_init_context ();
        
        pref_dialog->context = context;
        
        pref_dialog_reset_db (pref_dialog);
        pref_dialog_reset_strat (pref_dialog);
            
        gconf_client_set_int (gdict_get_gconf_client (), "/apps/gnome-dictionary/port", gdict_pref.port, NULL);
       
    }
    
    if (port)
    	g_free (port);

    return FALSE;
}

static void
set_default_server (GtkButton *button, gpointer data)
{
    GDictPrefDialog *pref_dialog = data;
    
    gtk_entry_set_text (GTK_ENTRY (pref_dialog->server_entry), DICT_DEFAULT_SERVER);
    set_server_cb (GTK_WIDGET (pref_dialog->port_entry), NULL, data);
    
}

static void
set_default_port (GtkButton *button, gpointer data)
{
    GDictPrefDialog *pref_dialog = data;
    gchar *port;
    
    port = g_strdup_printf ("%d", DICT_DEFAULT_PORT);
    gtk_entry_set_text (GTK_ENTRY (pref_dialog->port_entry), port);
    g_free (port);
    set_port_cb (GTK_WIDGET (pref_dialog->port_entry), NULL, data);
    
}    

static void 
smart_lookup_cb	(GtkToggleButton *toggle, gpointer data)
{
    GDictPrefDialog *pref_dialog = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active (toggle);
    if (toggled != gdict_pref.smart) {
        gconf_client_set_bool (gdict_get_gconf_client (), "/apps/gnome-dictionary/smart", toggled, NULL);
    }
    

}

static void
phelp_cb    (GtkDialog *dialog, gint response)
{
	GError *error = NULL;
	gnome_help_display("gnome-dictionary","gdict-settings",&error);

	if (error) {
		GtkWidget *msg_dialog;

		msg_dialog = gtk_message_dialog_new (GTK_WINDOW(dialog),
						     GTK_DIALOG_DESTROY_WITH_PARENT,
						     GTK_MESSAGE_ERROR,
						     GTK_BUTTONS_CLOSE,
						     ("There was an error displaying help: \n%s"),
						     error->message);

		g_signal_connect (G_OBJECT (msg_dialog), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (msg_dialog), FALSE);
		gtk_widget_show (msg_dialog);
		g_error_free (error);
	}
}
