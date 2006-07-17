#include <blist.h>
#include <debug.h>
#include <gtkutils.h>

#include "ge_blist.h"
#include "state.h"
#include "nls.h"

gboolean GE_get_buddy_default_autoencrypt(const GaimAccount* account, const char* buddyname) {
   GaimBuddy *buddy;
   gboolean retval;

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
              "get_buddy_default_autoencrypt for %p:%s\n", account, buddyname);
   
   if (!account) return FALSE;

   buddy = gaim_find_buddy((GaimAccount*)account, buddyname);

   if (buddy) {
      if (!buddy->node.settings) {
         /* Some users have been getting a crash because buddy->node.settings is/was
            null.  I can't replicate the problem on my system...  So we sanity check
            until the bug in Gaim is found/fixed */
         gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
                    "Bad buddy settings for \n", buddyname);
         return FALSE;
      }

      retval = gaim_blist_node_get_bool(&buddy->node, "GE_Auto_Encrypt");
      gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Found buddy:%s:%d\n", buddyname, retval);

      return retval;
   }

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "No setting found for buddy:%s\n", buddyname);
   return FALSE;
}

static void buddy_autoencrypt_callback(GaimBuddy* buddy, gpointer data) {
   gboolean setting;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
              "encrypt callback hit (%p) %s\n", buddy, buddy->name);

   setting = gaim_blist_node_get_bool(&buddy->node, "GE_Auto_Encrypt");
   gaim_blist_node_set_bool(&buddy->node, "GE_Auto_Encrypt", !setting);
   GE_set_tx_encryption(buddy->account, buddy->name, !setting);
}

void GE_buddy_menu_cb(GaimBlistNode* node, GList **menu, void* data) {
   GaimBlistNodeAction *action;
   GaimBuddy* buddy;
   gboolean setting;

   if (!GAIM_BLIST_NODE_IS_BUDDY(node)) return;
   /* else upcast to the buddy that we know it is: */
   buddy = (GaimBuddy*) node;

   setting = gaim_blist_node_get_bool(node, "GE_Auto_Encrypt");

   if (setting) {
      action = gaim_blist_node_action_new(_("Turn Auto-Encrypt Off"), /* it is now turned on */ 
                                          (gpointer)buddy_autoencrypt_callback, buddy->account->gc);
   } else {
      action = gaim_blist_node_action_new(_("Turn Auto-Encrypt On"),  /* it is now turned off */ 
                                          (gpointer)buddy_autoencrypt_callback, buddy->account->gc);
   }
   *menu = g_list_append(*menu, action);
}
