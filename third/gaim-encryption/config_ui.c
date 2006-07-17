/*                    Configure dialog UI                                 */
/*             Copyright (C) 2001-2003 William Tompkins                   */

/* This plugin is free software, distributed under the GNU General Public */
/* License.                                                               */
/* Please see the file "COPYING" distributed with the Gaim source code    */
/* for more details                                                       */
/*                                                                        */
/*                                                                        */
/*    This software is distributed in the hope that it will be useful,    */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*   General Public License for more details.                             */

/*   To compile and use:                                                  */
/*     See INSTALL file.                                                  */

#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk/gtkplug.h>

#include <debug.h>
#include <gtkdialogs.h>
#include <gtkprefs.h>
#include <gtkprefs.h>
#include <gaim.h>

#include "nls.h"
#include "cryptproto.h"
#include "keys.h"
#include "config_ui.h"

#ifdef _WIN32
#include "win32dep.h"
#endif

/*Static vars for the config dialog: */
static GtkWidget *key_size_entry, *proto_combo;

/* static GtkListStore *key_list_store = NULL; */
/* static GtkWidget *key_list_view = NULL; */

static GtkWidget *regen_err_label;
static GtkWidget *regen_window = NULL; /* regenerate key popup */

static GtkWidget* config_vbox = NULL;  /* Our main config pane */

/* Callbacks for the Regenerate popup dialog */
static void config_cancel_regen() {
   if (regen_window) {
      gtk_widget_destroy(regen_window);
   }
   regen_window = NULL;
}

static void config_do_regen(GtkWidget* hitbutton, GtkWidget *key_list_view) {
   GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(key_list_view));
   GtkListStore *key_list_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(key_list_view)));
   
   const gchar* bits_string = gtk_entry_get_text(GTK_ENTRY(key_size_entry));
   const gchar* proto_string = 
      gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(proto_combo)->entry));  // should NOT be freed
   int bits = 0;
   GSList *proto = crypt_proto_list;
   gchar *name;
   GaimAccount *acct;
   gchar key_len[15];

   GString *key_buf;
   GtkTreeIter list_store_iter;
   
   sscanf(bits_string, "%d", &bits);

   if (bits == 0) {
      gtk_label_set_text(GTK_LABEL(regen_err_label),
                         _("Bad key size"));
      return;
   }

   if (bits < 512) {
      gtk_label_set_text(GTK_LABEL(regen_err_label),
                         _("Keys < 512 bits are VERY insecure"));
      return;
   }

   if (bits > 4096) {
      gtk_label_set_text(GTK_LABEL(regen_err_label),
                         _("Keys > 4096 bits will cause extreme\n"
                           "message bloat, causing problems with\n"
                           "message transmission"));
      return;
   }
   
   while (proto != NULL && 
          strcmp(proto_string, ((crypt_proto*)proto->data)->name) != 0) {
      proto = proto->next;
   }
   
   if (proto == NULL) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Can't find protocol in list! Aigh!\n");
      return;
   }
   
   if (gtk_tree_selection_get_selected(selection, NULL, &list_store_iter)) {
      gtk_tree_model_get(GTK_TREE_MODEL(key_list_store), &list_store_iter, 0, &name, 4, &acct, -1);
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "regen for name: '%s', acct: %p\n", name, acct);

      GE_make_private_pair((crypt_proto*)proto->data, name, acct, bits);

      snprintf(key_len, sizeof(key_len), "%d", bits);

      key_buf = g_string_new_len(GE_find_key_by_name(GE_my_pub_ring, name, acct)->fingerprint,
                                 KEY_FINGERPRINT_LENGTH);

      gtk_list_store_set(key_list_store, &list_store_iter,
                         1, key_len,
                         2, key_buf->str,
                         3, proto_string,
                         -1);

      g_string_free(key_buf, TRUE);
      g_free(name);
   }

   config_cancel_regen();

   hitbutton = hitbutton; /* unused */
}

/* Display the Regenerate Key popup, and set up the above callbacks */
/* (used as a callback from the main Config dialog, below)          */
static void config_regen_key(GtkWidget* hitbutton, GtkWidget* key_list_view) {
   GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(key_list_view));
   
   GtkWidget *vbox, *hbox, *label, *table, *button;
   GList *proto_list = NULL;
   key_ring* iter;
   
   if (regen_window != NULL) return;
  
   GAIM_DIALOG(regen_window);
   gtk_widget_set_size_request(regen_window, 300, 150);
   gtk_window_set_title(GTK_WINDOW(regen_window), _("Generate Keys"));
   g_signal_connect(G_OBJECT(regen_window), "destroy", 
                    GTK_SIGNAL_FUNC(config_cancel_regen), NULL);

   vbox = gtk_vbox_new(0, 2);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
   gtk_container_add(GTK_CONTAINER(regen_window), vbox);
   gtk_widget_show (vbox);
   
   if (!gtk_tree_selection_get_selected(selection, NULL, NULL)) {
      label = gtk_label_new(_("No key selected to re-generate!"));
      gtk_box_pack_start(GTK_BOX(vbox), label, 0, 0, 0);
      gtk_widget_show(label);
      
      hbox = gtk_hbox_new(FALSE, 2);
      gtk_box_pack_end(GTK_BOX(vbox), hbox, 0, 0, 0);
      gtk_widget_show(hbox);

      button = gtk_button_new_with_label(_("OK"));
      g_signal_connect(G_OBJECT(button), "clicked",
                         GTK_SIGNAL_FUNC(config_cancel_regen), NULL);
      gtk_box_pack_end(GTK_BOX(hbox), button, 0, 0, 0);
      gtk_widget_set_size_request(button, 100, -1);
      gtk_widget_show(button);
      gtk_widget_show(regen_window);
      return;
   }

   /* Start 2 x 2 table */
   table = gtk_table_new(2, 2, FALSE);
   gtk_box_pack_start(GTK_BOX(vbox), table, 0, 0, 0);
   gtk_widget_show(table);

   /* First column */
   label = gtk_label_new(_("Encryption protocol:"));
   gtk_widget_set_size_request(label, 150, -1);
   gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
   gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
                    0, 0, 0, 0);
   gtk_widget_show(label);
         
   label = gtk_label_new(_("Key size:"));
   gtk_widget_set_size_request(label, 150, -1);
   gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
   gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
                    0, 0, 0, 0);
   gtk_widget_show(label);

   /* Second column: */
   proto_combo = gtk_combo_new();
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(proto_combo)->entry), 
                      ((crypt_proto*)crypt_proto_list->data)->name);
   gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(proto_combo)->entry),
                             FALSE);
   for( iter = crypt_proto_list; iter != NULL; iter = iter->next ) {
      proto_list = g_list_append(proto_list,
                                 ((crypt_proto *)iter->data)->name);
   }
   gtk_combo_set_popdown_strings(GTK_COMBO (proto_combo), proto_list);
   g_list_free(proto_list);
   gtk_table_attach(GTK_TABLE(table), proto_combo, 1, 2, 0, 1,
                    0, 0, 0, 0);

   gtk_widget_set_size_request(proto_combo, 85, -1);
   gtk_widget_show(proto_combo);

   key_size_entry = gtk_entry_new();
   gtk_entry_set_max_length(GTK_ENTRY(key_size_entry), 5);
   gtk_entry_set_text(GTK_ENTRY(key_size_entry), "1024");
   gtk_table_attach(GTK_TABLE(table), key_size_entry, 1, 2, 1, 2,
                    0, 0, 0, 0);
   gtk_widget_set_size_request(key_size_entry, 85, -1);
   gtk_widget_show(key_size_entry);
   /* End of 2x2 table */
   
   regen_err_label = gtk_label_new("");
   gtk_box_pack_start(GTK_BOX(vbox), regen_err_label, 0, 0, 0);
   gtk_widget_show(regen_err_label);

   hbox = gtk_hbox_new(FALSE, 2);
   gtk_box_pack_end(GTK_BOX(vbox), hbox, 0, 0, 0);
   gtk_widget_show(hbox);

   button = gtk_button_new_with_label(_("Cancel"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(config_cancel_regen), NULL);
   gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);
   gtk_widget_set_size_request(button, 100, -1);
   gtk_widget_show(button);
   
   button = gtk_button_new_with_label(_("Ok"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(config_do_regen), key_list_view);
   gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);
   gtk_widget_set_size_request(button, 100, -1);
   gtk_widget_show(button);
   
   gtk_widget_show(regen_window);

   hitbutton = hitbutton; /* unused */
}

/* button handler: */
static void delete_local_key(GtkWidget* hitbutton, GtkWidget* key_list_view) {
   GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(key_list_view));
   GtkListStore *key_list_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(key_list_view)));
   GtkTreeIter list_store_iter;

   gchar *name;
   GaimAccount *acct;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "delete local key\n");

   if (regen_window != NULL) return;

   if (gtk_tree_selection_get_selected(selection, NULL, &list_store_iter)) {

      gtk_tree_model_get(GTK_TREE_MODEL(key_list_store), &list_store_iter, 0, &name, 4, &acct, -1);

      {
         GtkWidget * confirm_dialog =
            gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                                   "%s : %s", _("Delete Key"), name);
         
         gint confirm_response = gtk_dialog_run( GTK_DIALOG(confirm_dialog) );
         gtk_widget_destroy(confirm_dialog);
         
         if (confirm_response != GTK_RESPONSE_OK) return;
      }

      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "deleting '%s' : %p\n", name, acct);

      GE_del_key_from_file(Public_key_file, name, acct);
      GE_del_key_from_file(Private_key_file, name, acct);

      GE_del_key_from_ring(GE_my_pub_ring, name, acct);
      GE_del_key_from_ring(GE_my_priv_ring, name, acct);
      
      gtk_list_store_remove(key_list_store, &list_store_iter);
   }

   hitbutton = hitbutton; /* unused */
}

/* button handler: */
static void delete_buddy_key(GtkWidget* hitbutton, GtkWidget* key_list_view) {
   GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(key_list_view));
   GtkListStore *key_list_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(key_list_view)));
   GtkTreeIter list_store_iter;

   gchar *name;
   GaimAccount *acct;
   gint num;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "delete buddy key\n");

   if (regen_window != NULL) return;

   if (gtk_tree_selection_get_selected(selection, NULL, &list_store_iter)) {

      gtk_tree_model_get(GTK_TREE_MODEL(key_list_store), &list_store_iter, 0,
                         &name, 4, &acct, 5, &num, -1);

      {
         GtkWidget * confirm_dialog =
            gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                                   "%s %s", _("Delete Key"), name);
         
         gint confirm_response = gtk_dialog_run( GTK_DIALOG(confirm_dialog) );
         gtk_widget_destroy(confirm_dialog);
         
         if (confirm_response != GTK_RESPONSE_OK) return;
      }

      /* gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "From file: %d : %s\n", num, name); */
      GE_del_one_key_from_file(Buddy_key_file, num, name);
      GE_del_key_from_ring(GE_buddy_ring, name, acct);
      
      gtk_list_store_remove(key_list_store, &list_store_iter);
   }

   hitbutton = hitbutton; /* unused */
}

/* button handler: */
static void copy_fp_to_clipboard(GtkWidget* hitbutton, GtkWidget* key_list_view) {
   GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(key_list_view));
   GtkListStore *key_list_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(key_list_view)));
   GtkTreeIter list_store_iter;

   gchar *fptext;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "copy to clipboard\n");

   if (regen_window != NULL) return;

   if (gtk_tree_selection_get_selected(selection, NULL, &list_store_iter)) {
      gtk_tree_model_get(GTK_TREE_MODEL(key_list_store), &list_store_iter, 2, &fptext, -1);
      
      /*  gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "copy :%s:\n", fptext); */

      gtk_clipboard_set_text( gtk_clipboard_get (GDK_SELECTION_PRIMARY), fptext, strlen(fptext) );
      gtk_clipboard_set_text( gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), fptext, strlen(fptext) );
      g_free(fptext);
   }

   hitbutton = hitbutton; /* unused */
}

GtkWidget* GE_create_key_vbox(key_ring *ring, gboolean local, GtkWidget** key_list_view) {
   GtkWidget *keybox = gtk_vbox_new(FALSE, 10);
   GtkWidget *keywin = gtk_scrolled_window_new(0, 0);

   GtkListStore *key_list_store;
   GtkTreeIter list_store_iter;
   GtkCellRenderer *renderer;
   GtkTreeViewColumn *col;
   key_ring* iter;
   GString* key_buf;
   gint num;

   gtk_widget_show(keybox);
   gtk_box_pack_start(GTK_BOX(keybox), keywin, 0, 0, 0);

   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (keywin),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_ALWAYS);
   gtk_widget_set_size_request (keywin, -1, 250);
   gtk_widget_show(keywin);

   key_list_store = gtk_list_store_new (6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                        G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT, -1);
   
   *key_list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(key_list_store));

   gtk_container_add(GTK_CONTAINER(keywin), *key_list_view);
   gtk_widget_show(*key_list_view);

   g_object_unref (G_OBJECT(key_list_store));

   renderer = gtk_cell_renderer_text_new();
   
   if (local) {
      col = gtk_tree_view_column_new_with_attributes(_("Account"), renderer, "text", 0, NULL);
   } else {
      col = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", 0, NULL);
   }
   gtk_tree_view_append_column(GTK_TREE_VIEW(*key_list_view), col);

   col = gtk_tree_view_column_new_with_attributes(_("Bits"), renderer, "text", 1, NULL);
   gtk_tree_view_append_column(GTK_TREE_VIEW(*key_list_view), col);

   col = gtk_tree_view_column_new_with_attributes(_("Key Fingerprint"), renderer, "text", 2, NULL);
   gtk_tree_view_append_column(GTK_TREE_VIEW(*key_list_view), col);
   
   num = 0;
   for( iter = ring; iter != NULL; iter = iter->next ) {
      gtk_list_store_append(key_list_store, &list_store_iter);

      key_buf = g_string_new_len(((key_ring_data *)iter->data)->key->fingerprint,
                                 KEY_FINGERPRINT_LENGTH);

      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Set List Item: name: '%s', acct: %p, num: %d\n",
                 ((key_ring_data *)iter->data)->name, ((key_ring_data *)iter->data)->account, num);

      gtk_list_store_set(key_list_store, &list_store_iter,
                         0, ((key_ring_data *)iter->data)->name,
                         1, ((key_ring_data *)iter->data)->key->length,
                         2, key_buf->str,
                         3, ((key_ring_data *)iter->data)->key->proto->name,
                         4, ((key_ring_data *)iter->data)->account,
                         5, num,
                         -1);
      g_string_free(key_buf, TRUE);
      ++num;
   }

   gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(*key_list_view), TRUE);

   return keybox;
}


/* Todo:  */
/* - Make sure we aren't leaking memory on loose widget references */

/* Called when Gaim wants us to show our config dialog */

GtkWidget* GE_get_config_frame(GaimPlugin *plugin) {
   GtkWidget *keybox;
   GtkWidget *hbox;
   GtkWidget *button;
   GtkWidget *notebook;
   GtkWidget *checkbox_vbox;

   GtkWidget *cur_list_view;

   config_vbox = gtk_vbox_new(FALSE, 2);

   gtk_container_set_border_width (GTK_CONTAINER (config_vbox), 12);

   gtk_widget_show (config_vbox);
   
   g_signal_connect(G_OBJECT(config_vbox), "destroy", 
                    GTK_SIGNAL_FUNC(config_cancel_regen), NULL);


   notebook = gtk_notebook_new();
   gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
   gtk_box_pack_start(GTK_BOX(config_vbox), notebook, 0, 0, 0);
   gtk_widget_show(notebook);


   /* Notebook page 1:  Config */

   checkbox_vbox = gtk_vbox_new(FALSE, 2);
   gtk_container_set_border_width(GTK_CONTAINER(checkbox_vbox), 2);
   gtk_widget_show(checkbox_vbox);
   gtk_notebook_append_page (GTK_NOTEBOOK (notebook), checkbox_vbox,
                             gtk_label_new(_("Config")));
   
   gaim_gtk_prefs_checkbox(_("Accept key automatically if no key on file"), 
                           "/plugins/gtk/encrypt/accept_unknown_key", checkbox_vbox);

   gaim_gtk_prefs_checkbox(_("Accept conflicting keys automatically (security risk)"),
                           "/plugins/gtk/encrypt/accept_conflicting_key", checkbox_vbox); 

   gaim_gtk_prefs_checkbox(_("Automatically encrypt if sent an encrypted message"),
                           "/plugins/gtk/encrypt/encrypt_response", checkbox_vbox);
   
   gaim_gtk_prefs_checkbox(_("Broadcast encryption capability"),
                           "/plugins/gtk/encrypt/broadcast_notify", checkbox_vbox);
   
   gaim_gtk_prefs_checkbox(_("Automatically encrypt if buddy has plugin"), 
                           "/plugins/gtk/encrypt/encrypt_if_notified", checkbox_vbox);


   /* Notebook page 2: Local keys */

   keybox = GE_create_key_vbox(GE_my_priv_ring, TRUE, &cur_list_view);

   hbox = gtk_hbox_new(FALSE, 2);
   gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);

   gtk_box_pack_start(GTK_BOX(keybox), hbox, 0, 0, 0);
   gtk_widget_show(hbox);

   button = gtk_button_new_with_label(_("Delete Key"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(delete_local_key), cur_list_view);
   gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);

   gtk_widget_show(button);
   
   button = gtk_button_new_with_label(_("Regenerate Key"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(config_regen_key), cur_list_view);
   gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);

   gtk_widget_show(button);

   button = gtk_button_new_with_label(_("Copy Fingerprint to Clipboard"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(copy_fp_to_clipboard), cur_list_view);
   gtk_box_pack_end(GTK_BOX(hbox), button, 0, 0, 0);

   gtk_widget_show(button);


   gtk_notebook_append_page (GTK_NOTEBOOK (notebook), keybox, gtk_label_new(_("Local Keys")));


   /* Notebook page 3: Saved Buddy Keys */

   keybox = GE_create_key_vbox(GE_saved_buddy_ring, FALSE, &cur_list_view);

   hbox = gtk_hbox_new(FALSE, 2);
   gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
   gtk_box_pack_start(GTK_BOX(keybox), hbox, 0, 0, 0);
   gtk_widget_show(hbox);

   button = gtk_button_new_with_label(_("Delete Key"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(delete_buddy_key), cur_list_view);
   gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);

   gtk_widget_show(button);
   
   button = gtk_button_new_with_label(_("Copy Fingerprint to Clipboard"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(copy_fp_to_clipboard), cur_list_view);
   gtk_box_pack_end(GTK_BOX(hbox), button, 0, 0, 0);

   gtk_widget_show(button);

   gtk_notebook_append_page (GTK_NOTEBOOK (notebook), keybox,
                             gtk_label_new(_("Trusted Buddy Keys")));


   /* Notebook page 4: In-Memory Buddy Keys */

   keybox = GE_create_key_vbox(GE_buddy_ring, FALSE, &cur_list_view);   

   hbox = gtk_hbox_new(FALSE, 2);
   gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
   gtk_box_pack_start(GTK_BOX(keybox), hbox, 0, 0, 0);
   gtk_widget_show(hbox);

   button = gtk_button_new_with_label(_("Delete Key"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(delete_buddy_key), cur_list_view);
   gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);

   gtk_widget_show(button);
   
   button = gtk_button_new_with_label(_("Copy Fingerprint to Clipboard"));
   g_signal_connect(G_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(copy_fp_to_clipboard), cur_list_view);
   gtk_box_pack_end(GTK_BOX(hbox), button, 0, 0, 0);

   gtk_widget_show(button);

   gtk_notebook_append_page (GTK_NOTEBOOK (notebook), keybox,
                             gtk_label_new(_("Recent Buddy Keys")));


   /* make it so that when the config_vbox object is finalized, our pointer to it is nulled out */
   g_object_add_weak_pointer(G_OBJECT(config_vbox), (gpointer*) &config_vbox);

   return config_vbox;
}

void GE_config_unload() {
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "GE_config_unload: %p\n", config_vbox);
   if (config_vbox) {
      /* We don't want our internal static functions getting called after the plugin   */
      /* has been unloaded, so disconnect the callback that kills the key regen window */
      /* For good measure, kill the key regen window too                               */

      g_signal_handlers_disconnect_by_func(GTK_OBJECT(config_vbox), 
                                           GTK_SIGNAL_FUNC(config_cancel_regen), NULL);
      config_cancel_regen();
      config_vbox = NULL;
   }
}
