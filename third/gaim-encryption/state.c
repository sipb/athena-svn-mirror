#include <string.h>
#include <ctype.h>

#include <gdk/gdk.h>
#include <gtk/gtkplug.h>

#include <gtkplugin.h>

#include <debug.h>
#include <util.h>
#include <conversation.h>

#include "ge_blist.h"
#include "state_ui.h"
#include "state.h"

GHashTable *encryption_state_table; /* name -> EncryptionState */

/* Helper function: */
static void reset_state_struct(const GaimAccount* account,
                               const gchar* name,
                               EncryptionState* state);

void GE_state_init() {
   encryption_state_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

void GE_state_delete() {
   g_hash_table_destroy(encryption_state_table);
}

EncryptionState* GE_get_state(const GaimAccount *account, const gchar* name) {
   EncryptionState *state = g_hash_table_lookup(encryption_state_table,
                                                gaim_normalize(account, name));

   /*   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
        "get_state %s: %p\n", name, state); */
   
   if (state == NULL) {
      state = g_malloc(sizeof(EncryptionState));
      g_hash_table_insert(encryption_state_table, g_strdup(gaim_normalize(account, name)), state);

      reset_state_struct(account, name, state);
   }
   return state;
}

void GE_reset_state(const GaimAccount *account, const gchar* name) {
   EncryptionState *state = GE_get_state(account, name);
   
   reset_state_struct(account, name, state);
}

static void reset_state_struct(const GaimAccount* account, const gchar* name,
                               EncryptionState* state) {

   /*   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "reset_state_struct:%s:%s\n",
        name, account->protocol_id); */

   state->outgoing_encrypted = GE_get_buddy_default_autoencrypt(account, name);
   state->has_been_notified = GE_get_default_notified(account, name);

   state->incoming_encrypted = FALSE;
   state->is_capable = FALSE;
}

void GE_set_tx_encryption(const GaimAccount *account, const gchar* name,
                          gboolean do_encrypt) {
   GaimConversation *conv;
   EncryptionState *state = GE_get_state(account, name);

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", 
                 "set_tx_encryption %p : %d : %d\n",
                 state, state->outgoing_encrypted, do_encrypt);
   
   if (state->outgoing_encrypted != do_encrypt) {
      state->outgoing_encrypted = do_encrypt;
      conv = gaim_find_conversation_with_account(name, account);
      if (conv) {
         GE_set_tx_encryption_icon(conv, do_encrypt, state->is_capable);
      } else {
         gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
                    "set_tx_encryption: no such conversation\n");
      }
   }
}

void GE_set_capable(const GaimAccount *account, const gchar* name, gboolean cap) {
   GaimConversation *conv;
   EncryptionState *state = GE_get_state(account, name);

   if (state->is_capable != cap) {
      state->is_capable = cap;
      conv = gaim_find_conversation_with_account(name, account);
      if (conv && (state->outgoing_encrypted == FALSE)) {
         GE_set_capable_icon(conv, cap);
      }
   }
}

void GE_set_rx_encryption(const GaimAccount *account, const gchar* name,
                          gboolean incoming_encrypted) {
   GaimConversation *conv;
   EncryptionState *state = GE_get_state(account, name);

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", 
              "set_rx_encryption '%s': %p : %d : %d\n", name,
              state, state->incoming_encrypted, incoming_encrypted);

   if (state->incoming_encrypted != incoming_encrypted) {
      state->incoming_encrypted = incoming_encrypted;
      conv = gaim_find_conversation_with_account(name, account);

      if (conv) {
         GE_set_rx_encryption_icon(conv, incoming_encrypted);
      } else {
         gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "set_rx_encryption: conv is null for '%s'\n", name);
      }
   }
}

gboolean GE_get_tx_encryption(const GaimAccount *account, const gchar* name) {
   EncryptionState *state = GE_get_state(account, name);

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "tx_button_state: %s : %p\n", name, state);

   return state->outgoing_encrypted;
}

gboolean GE_has_been_notified(const GaimAccount *account, const gchar *name) {
   EncryptionState *state = GE_get_state(account, name);

   return state->has_been_notified;
}

void GE_set_notified(const GaimAccount *account, const gchar *name,
                     gboolean new_state) {
   EncryptionState *state = GE_get_state(account, name);

   state->has_been_notified = new_state;
}


gboolean GE_get_default_notified(const GaimAccount *account, const gchar* name) {
   /* Most protocols no longer have a notify message, since they don't do HTML */
   
   /* The only special case here is Oscar/TOC: If the other user's name is all */
   /* digits, then they're ICQ, so we pretend that we already notified them    */

   const char* protocol_id = gaim_account_get_protocol_id(account);
   
   if (strcmp(protocol_id, "prpl-toc") == 0 || strcmp(protocol_id, "prpl-oscar") == 0) {
      
      while(*name != 0) {
         if (!isdigit(*name++)) {
            /* not ICQ */
            return FALSE;
         }
      }
      /* Hrm.  must be ICQ */
      return TRUE;
   }

   /* default to notifying next time, if protocol allows it */

   return FALSE;
}

   
