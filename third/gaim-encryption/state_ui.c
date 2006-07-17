#include "gaim-encryption-config.h"

#include <gdk/gdk.h>
#include <gtk/gtkplug.h>

#include <gtkplugin.h>
#include <debug.h>

#include "state_ui.h"
#include "state.h"
#include "nls.h"


/* Icons */
#include "icon_out_lock.xpm"
#include "icon_out_unlock.xpm"
#include "icon_out_capable.xpm"
#include "icon_in_lock.xpm"
#include "icon_in_unlock.xpm"


void GE_set_tx_encryption_icon(GaimConversation* conv,
                                   gboolean do_encrypt, gboolean iscapable) {
   GtkWidget *tx_button_unencrypted = g_hash_table_lookup(conv->data, "tx_button_unencrypted");
   GtkWidget *tx_button_encrypted = g_hash_table_lookup(conv->data, "tx_button_encrypted");
   GtkWidget *tx_button_capable = g_hash_table_lookup(conv->data, "tx_button_capable");
   
   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
              "set_tx_encryption_icon: %p, %p, %p, %p, %d\n", conv,
              tx_button_unencrypted, tx_button_encrypted, tx_button_capable, iscapable);
   
   if (do_encrypt) {
      gtk_widget_hide(tx_button_unencrypted);
      gtk_widget_hide(tx_button_capable);
      gtk_widget_show(tx_button_encrypted);      
   } else {
      if (iscapable) {
         gtk_widget_hide(tx_button_unencrypted);
         gtk_widget_show(tx_button_capable);
         gtk_widget_hide(tx_button_encrypted);
      } else {
         gtk_widget_show(tx_button_unencrypted);
         gtk_widget_hide(tx_button_capable);
         gtk_widget_hide(tx_button_encrypted);

      }
   }
}

/* Shows either the "unencrypted" or "capable" tx button, depending on cap */

void GE_set_capable_icon(GaimConversation *conv, gboolean cap) {
   GtkWidget *tx_button_capable = g_hash_table_lookup(conv->data,
                                                      "tx_button_capable");

   GtkWidget *tx_button_unencrypted = g_hash_table_lookup(conv->data,
                                                          "tx_button_unencrypted");
   
   
   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "set_capable_icon: %p, %p, %p, %d\n", 
              conv, tx_button_capable, tx_button_unencrypted, cap);
   

   if (cap) {
      gtk_widget_hide(tx_button_unencrypted);
      gtk_widget_show(tx_button_capable);
   } else {
      gtk_widget_hide(tx_button_capable);
      gtk_widget_show(tx_button_unencrypted);
   }
}



void GE_set_rx_encryption_icon(GaimConversation *conv, gboolean encrypted) {
   GtkWidget *rx_button_encrypted = g_hash_table_lookup(conv->data,
                                                      "rx_button_encrypted");

   GtkWidget *rx_button_unencrypted = g_hash_table_lookup(conv->data,
                                                      "rx_button_unencrypted");

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "set_rx_icon: %p, %p, %p, %d\n", 
              conv, rx_button_encrypted, rx_button_unencrypted, encrypted);

   if (encrypted == TRUE) {
      gtk_widget_hide(rx_button_unencrypted);
      gtk_widget_show(rx_button_encrypted);
   } else {
      gtk_widget_hide(rx_button_encrypted);
      gtk_widget_show(rx_button_unencrypted);
   }
}


static void turn_on_encryption_callback(GtkWidget *callback, const GaimConversation* conv) {
   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "turn on encryption callback\n");
   GE_set_tx_encryption(conv->account, conv->name, TRUE);
}

static void turn_off_encryption_callback(GtkWidget *callback, const GaimConversation* conv) {
   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "turn off encryption callback\n");
   GE_set_tx_encryption(conv->account, conv->name, FALSE);
}


void GE_add_buttons(GaimConversation *conv) {
   GaimGtkConversation *gtkconv = GAIM_GTK_CONVERSATION(conv);
   
   GtkWidget *bbox = gtkconv->bbox;
   GtkSizeGroup *sg_rx = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
   GtkSizeGroup *sg_tx = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

   GtkWidget *tx_button_encrypted, *tx_button_unencrypted, *tx_button_capable,
      *rx_button_encrypted, *rx_button_unencrypted;

   /* Get the rx state.  This will be set if this conversation was opened because   */
   /* of a received message.  If it isn't already set, this will initialize it to   */
   /* the defaults.                                                                 */

   EncryptionState* state = GE_get_state(conv->account, conv->name);

   /* we make 5 buttons, but make only show 2 of them at a time:                    */
   /* one from the tx set (capable / encrypted / unencrypted)                       */
   /* and one from the rx set (encrypted / unencrypted)                             */

   /* gaim_gtk_change_text will call pixbuf_button_from_stock, and label the result */
   /* (depending on the user's prefs, this might make text, icon, or both)          */
   /* To avoid wiggling of the icons as they're swapped for each other, we put      */
   /* the icons that are in the same place into size groups                         */


   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Adding buttons to %p\n", conv);

   tx_button_capable = gaim_gtkconv_button_new("Gaim-Encryption_Out_Capable", _("Tx: capable"),
                                               _("Your buddy appears to have the Gaim-Encryption plugin."
                                                 " Still, your next outgoing message will NOT be encrypted "
                                                 " by the Gaim-Encryption plugin"), gtkconv->tooltips,
                                               turn_on_encryption_callback, (gpointer)conv);

   gtk_box_pack_start(GTK_BOX(bbox), tx_button_capable, FALSE, FALSE, 0);

   if (state->outgoing_encrypted == FALSE && state->is_capable) {
      gtk_widget_show(tx_button_capable);
   } else {
      gtk_widget_hide(tx_button_capable);
   }

   gtk_size_group_add_widget(sg_tx, tx_button_capable);
   tx_button_encrypted = gaim_gtkconv_button_new("Gaim-Encryption_Out_Encrypted", _("Tx: secure"),
                                                 _("Your next outgoing message will be encrypted "
                                                   " by the Gaim-Encryption plugin"), gtkconv->tooltips,
                                                 turn_off_encryption_callback, (gpointer)conv);
   
   g_signal_connect(G_OBJECT(tx_button_encrypted), "clicked",
                    GTK_SIGNAL_FUNC(turn_off_encryption_callback), (gpointer)conv);


   gtk_box_pack_start(GTK_BOX(bbox), tx_button_encrypted, FALSE, FALSE, 0);
   gtk_size_group_add_widget(sg_tx, tx_button_encrypted);


   if (state->outgoing_encrypted == TRUE) {
      gtk_widget_show(tx_button_encrypted);
   } else {
      gtk_widget_hide(tx_button_encrypted);
   }

   tx_button_unencrypted = gaim_gtkconv_button_new("Gaim-Encryption_Out_Unencrypted", _("Tx: plain"),
                                                   _("Your next outgoing message will NOT be encrypted "
                                                     " by the Gaim-Encryption plugin"), gtkconv->tooltips,
                                                   turn_on_encryption_callback, (gpointer)conv);

   gtk_box_pack_start(GTK_BOX(bbox), tx_button_unencrypted, FALSE, FALSE, 0);
   gtk_size_group_add_widget(sg_tx, tx_button_unencrypted);

   if (state->outgoing_encrypted == FALSE && (!state->is_capable)) {
      gtk_widget_show(tx_button_unencrypted);
   } else {
      gtk_widget_hide(tx_button_unencrypted);
   }

   rx_button_encrypted = gaim_gtkconv_button_new("Gaim-Encryption_In_Encrypted", _("Rx: secure"),
												 _("The last message received was encrypted "
												 " with the Gaim-Encryption plugin"), gtkconv->tooltips,
												 NULL, NULL);

   gtk_box_pack_start(GTK_BOX(bbox), rx_button_encrypted, FALSE, FALSE, 0);
   gtk_size_group_add_widget(sg_rx, rx_button_encrypted);

   if (state->incoming_encrypted == TRUE) {
      gtk_widget_show(rx_button_encrypted);
   } else {
      gtk_widget_hide(rx_button_encrypted);
   }

   rx_button_unencrypted = gaim_gtkconv_button_new("Gaim-Encryption_In_Unencrypted", _("Rx: plain"),
                                                   _("The last message received was NOT encrypted "
                                                     " with the Gaim-Encryption plugin"), gtkconv->tooltips,
                                                   NULL, NULL);

   gtk_box_pack_start(GTK_BOX(bbox), rx_button_unencrypted, FALSE, FALSE, 0);
   gtk_size_group_add_widget(sg_rx, rx_button_unencrypted);

   if (state->incoming_encrypted == FALSE) {
      gtk_widget_show(rx_button_unencrypted);
   } else {
      gtk_widget_hide(rx_button_unencrypted);
   }

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Almost done adding buttons to %p\n", conv);

   g_hash_table_insert(conv->data, g_strdup("tx_button_unencrypted"), tx_button_unencrypted);
   g_hash_table_insert(conv->data, g_strdup("tx_button_encrypted"), tx_button_encrypted);
   g_hash_table_insert(conv->data, g_strdup("tx_button_capable"), tx_button_capable);
   g_hash_table_insert(conv->data, g_strdup("rx_button_unencrypted"), rx_button_unencrypted);
   g_hash_table_insert(conv->data, g_strdup("rx_button_encrypted"), rx_button_encrypted);

   /* dereference the size group: now when the widgets are killed, they will die too: */
   g_object_unref(G_OBJECT(sg_rx));
   g_object_unref(G_OBJECT(sg_tx));

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Done adding buttons to %p\n", conv);
}

void GE_remove_buttons(GaimConversation *conv) {
   GtkWidget *tx_button_unencrypted = g_hash_table_lookup(conv->data, "tx_button_unencrypted");
   GtkWidget *tx_button_encrypted   = g_hash_table_lookup(conv->data, "tx_button_encrypted");
   GtkWidget *tx_button_capable     = g_hash_table_lookup(conv->data, "tx_button_capable");
   GtkWidget *rx_button_unencrypted = g_hash_table_lookup(conv->data, "rx_button_unencrypted");
   GtkWidget *rx_button_encrypted   = g_hash_table_lookup(conv->data, "rx_button_encrypted");

   if (tx_button_unencrypted) gtk_widget_destroy(tx_button_unencrypted);
   if (tx_button_encrypted)   gtk_widget_destroy(tx_button_encrypted);
   if (tx_button_capable)     gtk_widget_destroy(tx_button_capable);
   if (rx_button_unencrypted) gtk_widget_destroy(rx_button_unencrypted);
   if (rx_button_encrypted) gtk_widget_destroy(rx_button_encrypted);
}

void GE_pixmap_init() {
   /* Here we make a "stock" icon factory to make our icons, and inform GTK */
   /* This is _way_ overkill, but it does mean that we can use Gaim's icon  */
   /* adding code (since it uses Gnome stock icons).                        */
   int i;
   GdkPixbuf *pixbuf;
   GtkIconSet *icon_set;

   static const GtkStockItem items[] = {
      { "Gaim-Encryption_Encrypted", "_GTK!", (GdkModifierType)0, 0, NULL },
      { "Gaim-Encryption_Unencrypted", "_GTK!", (GdkModifierType)0, 0, NULL },
      { "Gaim-Encryption_Capable", "_GTK!", (GdkModifierType)0, 0, NULL }
   };

   static struct StockPixmap{
      const char * name;
      char ** xpm_data;
   } const item_names [] = {
      { "Gaim-Encryption_Out_Encrypted", icon_out_lock_xpm },
      { "Gaim-Encryption_Out_Unencrypted", icon_out_unlock_xpm },
      { "Gaim-Encryption_Out_Capable", icon_out_capable_xpm },
      { "Gaim-Encryption_In_Encrypted", icon_in_lock_xpm },
      { "Gaim-Encryption_In_Unencrypted", icon_in_unlock_xpm },
   };

   GtkIconFactory *factory;

   gtk_stock_add (items, G_N_ELEMENTS (items));
   
   factory = gtk_icon_factory_new();
   gtk_icon_factory_add_default(factory);

   for (i = 0; i < G_N_ELEMENTS(item_names); i++) {
      pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)item_names[i].xpm_data);
      icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
      gtk_icon_factory_add (factory, item_names[i].name, icon_set);
      gtk_icon_set_unref (icon_set);
      g_object_unref (G_OBJECT (pixbuf));
   }

   g_object_unref(factory);
}

void GE_error_window(const char* message) {
   GtkWidget *dialog, *label, *okay_button;
   dialog = gtk_dialog_new();
   label = gtk_label_new(message);

   okay_button = gtk_button_new_with_label(_("Ok"));
      gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
                              GTK_SIGNAL_FUNC (gtk_widget_destroy), dialog);
   gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
                      okay_button);

   gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                      label);
   gtk_widget_show_all (dialog);

}
