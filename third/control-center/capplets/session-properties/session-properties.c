/* session-properties.c - Edit session properties.

   Copyright 1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. 

   Authors: Felix Bellaby, Owen Taylor */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <string.h>
#include <gnome.h>
#include <parser.h>
#include "capplet-widget.h"
#include "gsm-client-list.h"
#include "gsm-protocol.h"
#include "session-properties.h"

#define GSM_OPTION_CONFIG_PREFIX "session-options/Options/"

/* Current state */
static gboolean trash_changes;
static gboolean trash_changes_revert;

static gboolean logout_prompt;
static gboolean logout_prompt_revert;

static GSList *startup_list;
static GSList *startup_list_revert;

static GtkObject *protocol;

/* capplet widgets */
static GtkWidget *capplet;
static GtkWidget *trash_changes_button;
static GtkWidget *logout_prompt_button;

static GtkWidget *clist;

static GtkWidget *startup_command_dialog;

/* CORBA callbacks and intialization */
static void capplet_build (void);

/* Capplet callback prototypes */
static void try (void);
static void revert (void);
static void ok (void);
static void cancel (void);
static void help (void);
static void page_hidden (void);
static void page_shown (void);

/* Other callbacks */

static void update_gui (void);
static void dirty_cb (GtkWidget *widget, GtkWidget *capplet);
static void add_cb (void);
static void edit_cb (void);
static void delete_cb (void);
static void browse_session_cb (void);

static GtkWidget *
left_aligned_button (gchar *label)
{
  GtkWidget *button = gtk_button_new_with_label (label);
  gtk_misc_set_alignment (GTK_MISC (GTK_BIN (button)->child),
			  0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (GTK_BIN (button)->child),
			GNOME_PAD_SMALL, 0);

  return button;
}

static void
capplet_build (void)
{
  GtkWidget *hbox, *vbox;
  GtkWidget *util_vbox;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *scrolled_window;
  GtkWidget *alignment;

  /* Retrieve options */

  gnome_config_push_prefix (GSM_OPTION_CONFIG_PREFIX);
  trash_changes = gnome_config_get_bool ("TrashMode=true");
  trash_changes_revert = trash_changes;

  logout_prompt = gnome_config_get_bool ("LogoutPrompt=true");
  logout_prompt_revert = logout_prompt;
  gnome_config_pop_prefix ();

  startup_list = NULL;
  startup_list = startup_list_read ("Default");
  startup_list_revert = startup_list_duplicate (startup_list);

  /* capplet callbacks */
  capplet = capplet_widget_new ();
  gtk_signal_connect (GTK_OBJECT (capplet), "try",
		      GTK_SIGNAL_FUNC (try), NULL);
  gtk_signal_connect (GTK_OBJECT (capplet), "revert",
		      GTK_SIGNAL_FUNC (revert), NULL);
  gtk_signal_connect (GTK_OBJECT (capplet), "cancel",
		      GTK_SIGNAL_FUNC (cancel), NULL);
  gtk_signal_connect (GTK_OBJECT (capplet), "ok",
		      GTK_SIGNAL_FUNC (ok), NULL);
  gtk_signal_connect (GTK_OBJECT (capplet), "page_hidden",
		      GTK_SIGNAL_FUNC (page_hidden), NULL);
  gtk_signal_connect (GTK_OBJECT (capplet), "page_shown",
		      GTK_SIGNAL_FUNC (page_shown), NULL);
  gtk_signal_connect (GTK_OBJECT (capplet), "help",
		      GTK_SIGNAL_FUNC (help), NULL);

  /**** GUI ****/

  vbox = gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_add (GTK_CONTAINER (capplet), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
  
  /* frame for options */

  frame = gtk_frame_new (_("Options"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

  util_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), util_vbox);
  gtk_container_set_border_width (GTK_CONTAINER (util_vbox), GNOME_PAD_SMALL);
  
  alignment = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (util_vbox), alignment, FALSE, FALSE, 0);

  logout_prompt_button = gtk_check_button_new_with_label
    (_("Prompt on logout"));
  gtk_container_add (GTK_CONTAINER (alignment), logout_prompt_button);

  alignment = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (util_vbox), alignment, FALSE, FALSE, 0);

  trash_changes_button = gtk_check_button_new_with_label
    (_("Automatically save changes to session"));
  gtk_container_add (GTK_CONTAINER (alignment), trash_changes_button);

  /* frame for manually started programs */

  frame = gtk_frame_new (_("Non-session-managed Startup Programs"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), GNOME_PAD);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
        
  clist = gtk_clist_new (2);

  gtk_clist_column_titles_show (GTK_CLIST (clist));
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist), 0, TRUE);
  gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_BROWSE);
  gtk_clist_set_column_title (GTK_CLIST (clist), 0, _("Priority"));
  gtk_clist_set_column_title (GTK_CLIST (clist), 1, _("Command"));

  gtk_container_add (GTK_CONTAINER (scrolled_window), clist);
  
  gtk_box_pack_start (GTK_BOX (hbox), scrolled_window, TRUE, TRUE, 0);
  
  util_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (hbox), util_vbox, FALSE, FALSE, 0);

  button = left_aligned_button (_("Add..."));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (add_cb), NULL);
  gtk_box_pack_start (GTK_BOX (util_vbox), button, FALSE, FALSE, 0);
  
  button = left_aligned_button (_("Edit..."));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (edit_cb), NULL);
  gtk_box_pack_start (GTK_BOX (util_vbox), button, FALSE, FALSE, 0);
  
  button = left_aligned_button (_("Delete"));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (delete_cb), NULL);
  gtk_box_pack_start (GTK_BOX (util_vbox), button, FALSE, FALSE, 0);

  /* Button for running session-properties */

  alignment = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
  
  button = gtk_button_new_with_label (_("Browse Currently Running Programs..."));
  gtk_misc_set_padding (GTK_MISC (GTK_BIN (button)->child),
			GNOME_PAD, 0);
  
  gtk_container_add (GTK_CONTAINER (alignment), button);

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
  		      GTK_SIGNAL_FUNC (browse_session_cb), NULL);

  update_gui ();

  gtk_signal_connect (GTK_OBJECT (trash_changes_button), "toggled",
		      GTK_SIGNAL_FUNC (dirty_cb), capplet);
  gtk_signal_connect (GTK_OBJECT (logout_prompt_button), "toggled",
		      GTK_SIGNAL_FUNC (dirty_cb), capplet);

  gtk_widget_show_all (capplet);
}

static void
read_from_xml (xmlDocPtr doc) 
{
        xmlNodePtr root_node, node;

        root_node = xmlDocGetRootElement (doc);
        if (strcmp (root_node->name, "session-prefs")) return;

        for (node = root_node->childs; node; node = node->next) {
                if (!strcmp (node->name, "trash-mode"))
                        trash_changes = FALSE;
                else if (!strcmp (node->name, "logout-prompt"))
                        logout_prompt = FALSE;
                else if (!strcmp (node->name, "startup-programs"))
                        startup_list = startup_list_read_from_xml (node);
        }
}

static xmlDocPtr
write_to_xml (void) 
{
        xmlDocPtr doc;
        xmlNodePtr root_node;
        char *str;

        doc = xmlNewDoc ("1.0");
        root_node = xmlNewDocNode (doc, NULL, "session-prefs", NULL);
        xmlDocSetRootElement (doc, root_node);

        if (trash_changes)
                xmlNewChild (root_node, NULL, "trash-mode", NULL);

        if (logout_prompt)
                xmlNewChild (root_node, NULL, "logout-prompt", NULL);

	xmlAddChild (root_node, startup_list_write_to_xml (startup_list));

        return doc;
}

/* CAPPLET CALLBACKS */
static void
write_state (void)
{
  if (trash_changes_button)
    trash_changes = !GTK_TOGGLE_BUTTON (trash_changes_button)->active;
  if (logout_prompt_button)
    logout_prompt = GTK_TOGGLE_BUTTON (logout_prompt_button)->active;
  
  gnome_config_push_prefix (GSM_OPTION_CONFIG_PREFIX);
  gnome_config_set_bool ("TrashMode", trash_changes);
  gnome_config_set_bool ("LogoutPrompt", logout_prompt);
  gnome_config_pop_prefix ();

  /* Set the trash mode in the session manager so it takes effect immediately.
   */
  gsm_protocol_set_trash_mode (protocol, trash_changes);
  
  gnome_config_sync ();

  startup_list_write (startup_list, "Default");
}

static void
revert_state (void)
{
  trash_changes = trash_changes_revert;
  logout_prompt = logout_prompt_revert;

  startup_list_free (startup_list);
  startup_list = startup_list_duplicate (startup_list_revert);
}

static void
try (void)
{
  write_state ();
}

static void
revert (void)
{
  revert_state ();
  write_state ();

  update_gui ();
}

static void
ok (void)
{
  write_state ();

  gtk_main_quit();
}

static void
cancel (void)
{
  revert_state ();
  write_state ();
  
  gtk_main_quit();  
}

static void
help (void)
{
  gchar* file = gnome_help_file_find_file(program_invocation_short_name, 
					  "index.html");
  if (file) {
    gnome_help_goto (NULL, file);
  } else {
    GtkWidget *mbox;
    
    mbox = gnome_message_box_new(_("No help is available/installed for these settings. Please make sure you\nhave the GNOME User's Guide installed on your system."),
				 GNOME_MESSAGE_BOX_ERROR,
				 _("Close"), NULL);
    
    gtk_widget_show(mbox);
  }
}

static void
page_shown (void)
{
  if (startup_command_dialog)
    gtk_widget_hide (startup_command_dialog);
}

static void
page_hidden (void)
{
  if (startup_command_dialog)
    gtk_widget_show (startup_command_dialog);
}

/* Called to make the contents of the GUI reflect the current settings */
static void
update_gui (void)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trash_changes_button),
				!trash_changes);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logout_prompt_button),
				logout_prompt);
  
  startup_list_update_gui (startup_list, clist);
}

/* This is called when an change is made in the client list.  */
static void
dirty_cb (GtkWidget *widget, GtkWidget *capplet)
{
  capplet_widget_state_changed (CAPPLET_WIDGET (capplet), TRUE);
}

/* Add a startup program to the list */
static void
add_cb (void)
{
  /* This is bad, bad, bad. We mark the capplet as changed at
   * this point so our dialog doesn't die if the user switches
   * away to a different capplet
   */
  dirty_cb (NULL, capplet);
  startup_list_add_dialog (&startup_list, clist, &startup_command_dialog);
  update_gui ();
}

/* Edit a startup program in the list */
static void
edit_cb (void)
{
  /* This is bad, bad, bad. We mark the capplet as changed at
   * this point so our dialog doesn't die if the user switches
   * away to a different capplet
   */
  dirty_cb (NULL, capplet);
  startup_list_edit_dialog (&startup_list, clist, &startup_command_dialog);
  update_gui ();
}

/* Remove a startup program from the list */
static void
delete_cb (void)
{
  dirty_cb (NULL, capplet);
  startup_list_delete (&startup_list, clist);
  update_gui ();
}

/* Run a browser for the currently running session managed clients */
static void
browse_session_cb (void)
{
  static char *const command[] = {
    "session-properties"
  };

  gnome_execute_async (NULL, 1, command);
}


/* STARTUP CODE */

static void do_get_xml (void) 
{
        xmlDocPtr doc;

	gnome_config_push_prefix (GSM_OPTION_CONFIG_PREFIX);
	trash_changes = gnome_config_get_bool ("TrashMode=true");
	logout_prompt = gnome_config_get_bool ("LogoutPrompt=true");
	gnome_config_pop_prefix ();

	startup_list = NULL;
	startup_list = startup_list_read ("Default");

        doc = write_to_xml ();
        xmlDocDump (stdout, doc);
}

static void do_set_xml (void) 
{
        xmlDocPtr doc;
	char *buffer;
	int len = 0;

	while (!feof (stdin)) {
		if (!len) buffer = g_new (char, 16384);
		else buffer = g_renew (char, buffer, len + 16384);
		fread (buffer + len, 1, 16384, stdin);
		len += 16384;
	}

	doc = xmlParseMemory (buffer, strlen (buffer));

	protocol = gsm_protocol_new (gnome_master_client ());

	if (!protocol) g_error ("Could not connect to gnome-session.");

	read_from_xml (doc);
        write_state ();
}

static gboolean warner = FALSE;

static struct poptOption options[] = {
  {"warner", '\0', POPT_ARG_NONE, &warner, 0, N_("Only display warnings."), NULL},
  {NULL, '\0', 0, NULL, 0}
};

int
main (int argc, char *argv[])
{
  gint init_result;

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  init_result = gnome_capplet_init("session-properties", VERSION, argc, argv,
				   options, 0, NULL);

  gtk_signal_connect (GTK_OBJECT (gnome_master_client ()), "die",
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gnome_client_set_restart_style (gnome_master_client(), GNOME_RESTART_NEVER);
  switch(init_result) 
    {
    case 0:
    case 1:
      break;

    case 3:
	    do_get_xml ();
	    return 0;
    case 4:
	    do_set_xml ();
	    return 0;

    default:
	    /* We need better error handling.  This prolly just means
	     * one already exists.*/
	    exit (0);
    }

  protocol = gsm_protocol_new (gnome_master_client());
  if (!protocol)
    {
      g_warning ("Could not connect to gnome-session.");
      exit (1);
    }

  /* We make this call immediately, as a convenient way
   * of putting ourselves into command mode; if we
   * don't do this, then the "event loop" that
   * GsmProtocol creates will leak memory all over the
   * place.
   
   * We ignore the resulting "last_session" signal.
   */
  gsm_protocol_get_last_session (GSM_PROTOCOL (protocol));

  switch(init_result) 
    {
    case 0:
      capplet_build ();
      capplet_gtk_main ();
      break;

    case 1:
      if (warner)
	gsm_session_live (gsm_client_new, NULL) ;
      else
	chooser_build ();
      gtk_main ();
      break;
    }
  return 0;
}
