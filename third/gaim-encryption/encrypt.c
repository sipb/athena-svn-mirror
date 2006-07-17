/*                    Gaim encryption plugin                              */
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


#include "gaim-encryption-config.h"

#include <gdk/gdk.h>
#include <gtk/gtkplug.h>

#include <config.h>
#include <debug.h>
#include <gaim.h>
#include <core.h>
#include <gtkutils.h>
#include <gtkplugin.h>
#include <gtkconv.h>
#include <gtkdialogs.h>
#include <gtkprefs.h>
#include <blist.h>
#include <gtkblist.h>
#include <signals.h>
#include <util.h>
#include <version.h>
#include <internal.h>

#include "cryptproto.h"
#include "cryptutil.h"
#include "state.h"
#include "state_ui.h"
#include "keys.h"
#include "nonce.h"
#include "prefs.h"
#include "config_ui.h"
#include "ge_blist.h"

#include "encrypt.h"
#include "nls.h"

#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef _WIN32
#include "win32dep.h"
#endif


/* from Gaim's internal.h, but it isn't critical that it is in sync: */
/* #define BUF_LONG 4096 */

G_MODULE_IMPORT GSList *gaim_accounts;
G_MODULE_IMPORT guint im_options;


#define ENCRYPT_PLUGIN_ID "gtk-obobo-gaim-encryption"

/* Types */
struct msg_node {
   char who[64];
   time_t time;
   GaimConnection* gc;
   struct msg_node* next;
   unsigned char msg[1];
};
typedef struct msg_node msg_node;


GaimPlugin *GE_plugin_handle;

/* Outgoing message queue (waiting on a public key to encrypt) */
static msg_node* first_out_msg = 0;
static msg_node* last_out_msg = 0;

/* Incoming message queue (waiting on a public key to verify) */
static msg_node* first_inc_msg = 0;
static msg_node* last_inc_msg = 0;

static int GE_get_msg_size_limit(GaimAccount*);
static void GE_send_key(GaimAccount *, const char *name, int, char*);
static crypt_key * GE_get_key(GaimConnection *, const char *name);
static int decrypt_msg(unsigned char **decrypted, unsigned char *msg,
                       const unsigned char *name, crypt_key *, crypt_key *);
static void GE_store_msg(const char *name, GaimConnection*, char *,
                         msg_node**, msg_node**);
static void got_encrypted_msg(GaimConnection *, const char *name, char **);

static void reap_all_sent_messages(GaimConversation*);
static void reap_old_sent_messages(GaimConversation*);

/* Function pointers exported to Gaim */
static gboolean GE_got_msg_cb(GaimAccount *, char **, char **, GaimConvImFlags flags, void *);
static void GE_send_msg_cb(GaimAccount *, char *, char **, void *);
static void GE_new_conv_cb(GaimConversation *, void *);
static void GE_del_conv_cb(GaimConversation *, void *);

static GHashTable *header_table, *footer_table, *notify_table;
static gchar* header_default;

/* #define CRYPT_HEADER "*** Encrypted with the Gaim-Encryption plugin <A HREF=\"" */
/* #define CRYPT_FOOTER "\"></A>" */
/* #define CRYPT_NOTIFY_HEADER "<A HREF=\"Gaim-Encryption Capable\"></A>" */

// Jabber seems to turn our double quotes into single quotes at times, so define
// the same headers, only with single quotes.  Lengths MUST be the same as above
/* #define CRYPT_HEADER_MANGLED "*** Encrypted with the Gaim-Encryption plugin <A HREF='" */
/* #define CRYPT_NOTIFY_HEADER_MANGLED "<A HREF='Gaim-Encryption Capable'></A>" */

/* Send key to other side.  If msg_id is non-null, we include a request to re-send */
/* a certain message, as well.                                                     */

static void GE_send_key(GaimAccount *acct, const char *name, int asError, gchar *msg_id) {
   /* load key somehow */
   char *msg;
   GString *key_str;
   crypt_key *pub_key;
   GaimConversation *conv;

   int header_size, footer_size;
   const gchar* header = g_hash_table_lookup(header_table, gaim_account_get_protocol_id(acct));
   const gchar* footer = g_hash_table_lookup(footer_table, gaim_account_get_protocol_id(acct));

   if (!header) header = header_default;
   if (!footer) footer = "";

   header_size = strlen(header);
   footer_size = strlen(footer);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "send_key: %s\n", acct->username);
   
   conv = gaim_find_conversation_with_account(name, acct);
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "send_key: %s, %p, %s\n", name, conv, acct->username);

   pub_key = GE_find_own_key_by_name(&GE_my_pub_ring, acct->username, acct, conv);
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "send_key2: %s\n", acct->username);
   if (!pub_key) return;

   key_str = GE_make_sendable_key(pub_key, name);
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "send_key3: %s\n", acct->username);

   msg = alloca(header_size + footer_size + key_str->len + 100);
   if (msg == 0) return;
   if (asError) {
      if (msg_id) {
         sprintf(msg, "%s: ErrKey: Prot %s: Len %d:%sResend:%s:%s", header,
                 pub_key->proto->name, (int)key_str->len, key_str->str, msg_id, footer);
      } else {
         sprintf(msg, "%s: ErrKey: Prot %s: Len %d:%s%s", header,
                 pub_key->proto->name, (int)key_str->len, key_str->str, footer);
      }
   } else {
      sprintf(msg, "%s: Key: Prot %s: Len %d:%s%s", header,
              pub_key->proto->name, (int)key_str->len, key_str->str, footer);
   }

   if (strlen(msg) > GE_get_msg_size_limit(acct)) {
      g_free(msg);
      gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Key too big to send in message\n");
      conv = gaim_find_conversation_with_account(name, acct);
      if (conv == NULL) {
         conv = gaim_conversation_new(GAIM_CONV_IM, acct, name);
      }
      gaim_conversation_write(conv, 0,
                              _("This account key is too large for this protocol. "
                                "Unable to send."),
                              GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
      return;
   }

   serv_send_im(acct->gc, name, msg, GAIM_CONV_IM_AUTO_RESP);
   g_string_free(key_str, TRUE);
}



static crypt_key *GE_get_key(GaimConnection *gc, const char *name) {
   crypt_key *bkey;
   unsigned char* tmpmsg;

   int header_size, footer_size;
   const gchar* header = g_hash_table_lookup(header_table, gaim_account_get_protocol_id(gc->account));
   const gchar* footer = g_hash_table_lookup(footer_table, gaim_account_get_protocol_id(gc->account));

   if (!header) header = header_default;
   if (!footer) footer = "";

   header_size = strlen(header);
   footer_size = strlen(footer);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "get_key: %s\n", name);
   bkey = GE_find_key_by_name(GE_buddy_ring, name, gc->account);
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "got key: %p\n", bkey);

   if( bkey == 0 ) {
      tmpmsg = alloca(header_size + footer_size +
                      sizeof (": Send Key")); // sizeof() gets the trailing null too

      sprintf(tmpmsg, "%s%s%s", header, ": Send Key", footer);

      gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Sending: %s\n", tmpmsg);
      serv_send_im(gc, name, tmpmsg, GAIM_CONV_IM_AUTO_RESP);
      return 0;
   }

   return bkey;
}


static int decrypt_msg(unsigned char **decrypted, unsigned char *msg, const unsigned char *name, 
                       crypt_key *priv_key, crypt_key *pub_key) {
   int realstart = 0;
	unsigned int length;
   int retval;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "decrypt_msg\n");

   if ( (sscanf(msg, ": Len %u:%n", &length, &realstart) < 1) || (realstart == 0)) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Garbled length in decrypt\n");
      return -1;
   }

   msg += realstart;

   if (strlen(msg) < length) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Length doesn't match in decrypt\n");
      return -1;
   }
   msg[length] = 0;
   
   retval = GE_decrypt_signed(decrypted, msg, priv_key, pub_key, name);
   
   return retval;
}


static void GE_store_msg(const char *who, GaimConnection *gc, char *msg, msg_node** first_node,
               msg_node** last_node) {
   msg_node* newnode;
   

   newnode = g_malloc(sizeof(msg_node) + strlen(msg));
   
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "store_msg: %p : %s\n", newnode, who);

   strncpy(newnode->who, gaim_normalize(gc->account, who), sizeof(newnode->who));
   newnode->who[sizeof(newnode->who)-1] = 0;

   newnode->gc = gc;
   newnode->time = time((time_t)NULL);
   strcpy(newnode->msg, msg);
   newnode->next = 0;

   
   if (*first_node == 0) {
      *last_node = newnode;
      *first_node = newnode;
   } else {
      (*last_node)->next = newnode;
      *last_node = newnode;
   }

   for (newnode = *first_node; newnode != *last_node; newnode = newnode->next) {
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "   In store stack: %p\n",
                 newnode, newnode->who);
   }
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "   In store stack: %p\n",
              *last_node, (*last_node)->who);
}

void GE_send_stored_msgs(GaimAccount* acct, const char* who) {
   msg_node* node = first_out_msg;
   msg_node* prev = 0;
   char *tmp_msg;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "send_stored_msgs\n");

   while (node != 0) {
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
                 "Looking for stored msg:%s:%s\n",node->who, who);
      if ((strcmp(node->who, who) == 0) && (node->gc->account == acct)) {
         tmp_msg = g_strdup(node->msg);
         GE_send_msg_cb(node->gc->account, (char*)who, &tmp_msg, 0);
         GE_clear_string(node->msg);
         if (tmp_msg != 0) {
            g_free(tmp_msg);
         }
         if (node == last_out_msg) {
            last_out_msg = prev;
         }
         if (prev != 0) { /* a random one matched */
            prev->next = node->next;
            g_free(node);
            node = prev->next;
         } else {  /* the first one matched */
            first_out_msg = node->next;
            g_free(node);
            node = first_out_msg;
         }
      } else { /* didn't match */
         prev = node;
         node = node->next;
      }
   }
}

void GE_delete_stored_msgs(GaimAccount* acct, const char* who) {
   msg_node* node = first_out_msg;
   msg_node* prev = 0;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "delete_stored_msgs\n");

   while (node != 0) {
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
                 "Looking for stored msg:%s:%s\n",node->who, who);
      if ((strcmp(node->who, who) == 0) && (node->gc->account == acct)) {
         GE_clear_string(node->msg);
         if (node == last_out_msg) {
            last_out_msg = prev;
         }
         if (prev != 0) { /* a random one matched */
            prev->next = node->next;
            g_free(node);
            node = prev->next;
         } else {  /* the first one matched */
            first_out_msg = node->next;
            g_free(node);
            node = first_out_msg;
         }
      } else { /* didn't match */
         prev = node;
         node = node->next;
      }
   }
}

void GE_show_stored_msgs(GaimAccount*acct, const char* who, char** return_msg) {
   msg_node* node = first_inc_msg;
   msg_node* prev = 0;
   char *tmp_msg;
   GaimConversation *conv = gaim_find_conversation_with_account(who, acct);

   while (node != 0) {
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "show_stored_msgs:%p:%s:%s:\n", node, node->who, who);
		if (strcmp(node->who, who) == 0) {
         tmp_msg = g_strdup(node->msg);
         got_encrypted_msg(node->gc, who, &tmp_msg);
         if (tmp_msg != 0) {
            gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "showing msg:%s\n", tmp_msg);

				if (return_msg) {
               /* We should return the last received message in *return_msg */
               if (*return_msg) {
                  /* We've already got a queued message to show, so swap, and force display of
                     the first one */
                  if (!conv) {
                     conv = gaim_conversation_new(GAIM_CONV_IM, node->gc->account, who);
                  }
                  gaim_conv_im_write(GAIM_CONV_IM(conv), who, *return_msg,
                                     GAIM_MESSAGE_RECV, time((time_t)NULL));
                  gaim_conv_window_flash(gaim_conversation_get_window(conv));
                  g_free(*return_msg);
                  *return_msg = 0;
               } else {
                  *return_msg = tmp_msg;
               }
            } else {
               /* Just display it */
               if (!conv) {
                  conv = gaim_conversation_new(GAIM_CONV_IM, node->gc->account, who);
               }
               gaim_conv_im_write(GAIM_CONV_IM(conv), who, tmp_msg,
                                  GAIM_MESSAGE_RECV, time((time_t)NULL));
               gaim_conv_window_flash(gaim_conversation_get_window(conv));
               g_free(tmp_msg);
            }
         }
         if (node == last_inc_msg) {
            last_inc_msg = prev;
         }
         if (prev != 0) { /* a random one matched */
            prev->next = node->next;
            g_free(node);
            node = prev->next;
         } else {  /* the first one matched */
            first_inc_msg = node->next;
            g_free(node);
            node = first_inc_msg;
         }
      } else { /* didn't match */
         prev = node;
         node = node->next;
      }
   }
}

static void reap_all_sent_messages(GaimConversation* conv){

   GQueue *sent_msg_queue = g_hash_table_lookup(conv->data, "sent messages");

   GE_SentMessage *sent_msg_item;

   /* gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "ZZZ Reaping all messages: %p\n", conv); */
   
   while (!g_queue_is_empty(sent_msg_queue)) {
      sent_msg_item = g_queue_pop_tail(sent_msg_queue);
      /* gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "ZZZ Message: %s\n", sent_msg_item->id); */
      g_free(sent_msg_item->id);
      g_free(sent_msg_item->msg);
      g_free(sent_msg_item);
   }
}

static void reap_old_sent_messages(GaimConversation* conv){
   GQueue *sent_msg_queue = g_hash_table_lookup(conv->data, "sent messages");
   
   GE_SentMessage *sent_msg_item;
   time_t curtime = time(0);
   
   /* gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "ZZZ Reaping old messages: %p\n", conv); */

   while (!g_queue_is_empty(sent_msg_queue)) {
      sent_msg_item = g_queue_peek_tail(sent_msg_queue);
      /* gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "ZZZ Message: %s\n", sent_msg_item->id); */
      if (curtime - sent_msg_item->time > 60) { /* message is over 1 minute old */
         sent_msg_item = g_queue_pop_tail(sent_msg_queue);
         /* gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "ZZZ  Deleted\n"); */
         g_free(sent_msg_item->id);
         g_free(sent_msg_item->msg);
         g_free(sent_msg_item);
      } else {
         /* These were pushed on in order, so if this one is not old, we're done */
         break;
      }
   }
}

static gboolean GE_got_msg_cb(GaimAccount *acct, char **who, char **message, GaimConvImFlags flags, void *m) {
   char *name;

   GaimConversation *conv;
   gchar *headerpos;    /* Header is allowed to be anywhere in message now */
   gchar *notifypos;
   gchar *caps_header, *caps_message,    /* temps for ascii_strup() versions of each */
         *caps_notify;                   /* since Jabber mucks with case             */

   gchar *unescaped_message;             /* temps for html_unescaped       */
                                         /* since ICQ will now escape HTML */

   int header_size, footer_size;
   const gchar* header = g_hash_table_lookup(header_table, gaim_account_get_protocol_id(acct));
   const gchar* footer = g_hash_table_lookup(footer_table, gaim_account_get_protocol_id(acct));
   const gchar* notify = g_hash_table_lookup(notify_table, gaim_account_get_protocol_id(acct));

   if (!header) header = header_default;
   if (!footer) footer = "";

   header_size = strlen(header);
   footer_size = strlen(footer);

   /* Since we don't have a periodic callback, we do some housekeeping here */
   gaim_conversation_foreach(reap_old_sent_messages);

   conv = gaim_find_conversation_with_account(*who, acct);
   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Finding conversation: %s, %p\n", *who, conv);
   
   name = g_strdup(gaim_normalize(acct, *who));
   
   if (*message != NULL) {
      caps_message = g_ascii_strup(*message, -1);
      caps_header = g_ascii_strup(header, -1);
      unescaped_message = gaim_unescape_html(*message);

      headerpos = strstr(caps_message, caps_header);
      g_free(caps_header);

      if (notify) {
         caps_notify = g_ascii_strup(notify, -1);
         notifypos = strstr(caps_message, caps_notify);
         g_free(caps_notify);
      } else {
         notifypos = 0;
      }
      if (headerpos != 0) {
         /* adjust to where the header is in the _real_ message, if */
         /* we found it in the caps_message                         */
         headerpos += (*message) - caps_message;
      }

      g_free(caps_message);

      if (headerpos == 0 && notifypos == 0) {
         /* Check for ICQ-escaped header*/
         headerpos = strstr(unescaped_message, header);
         if (notify) {
            notifypos = strstr(unescaped_message, notify);
         }
         if (headerpos != 0 || notifypos != 0) {
            /* ICQ PRPL escaped our HTML header, but we outsmarted it     */
            /* replace message with unescaped message.                    */
            gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Escaped header: replacing %s with %s\n",
                       *message, unescaped_message);
            g_free(*message);
            *message = unescaped_message;
         } else {
            g_free(unescaped_message);
         }
      }

      /* Whew.  Enough of this header-finding.  */
      

      if (headerpos != 0) {
         GE_set_capable(acct, name, TRUE);
         if (gaim_prefs_get_bool("/plugins/gtk/encrypt/encrypt_response")) {
            GE_set_tx_encryption(acct, name, TRUE);
         }
         if (strncmp(headerpos + header_size, ": Send Key",
                     sizeof(": Send Key")-1) == 0) {
            GE_send_key(acct, name, 0, 0);
            (*message)[0] = 0;
            g_free(*message);
            *message = NULL;
            gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Sent key per request\n");
         } else if (strncmp(headerpos + header_size, ": Key",
                            sizeof(": Key") - 1) == 0) {
            gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Got key\n");
            GE_received_key(headerpos + header_size + sizeof(": Key") - 1, name, acct,
                            conv, message);
         } else if (strncmp(headerpos + header_size, ": ErrKey",
                            sizeof(": ErrKey") - 1) == 0) {
            gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Got key in response to error\n");
            gaim_conversation_write(conv, 0,
                                    _("Last outgoing message not received properly- resetting"),
                                    GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
            gaim_conv_window_flash(gaim_conversation_get_window(conv));
            GE_received_key(headerpos + header_size + sizeof(": ErrKey") - 1, name, acct,
                            conv, message);
         } else if (strncmp(headerpos + header_size, ": Msg",
                            sizeof(": Msg") - 1) == 0){
            gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
                       "Got encrypted message: %d\n", strlen(*message)); 
            gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
                       "Message is:%s:\n", *message);
            memmove(*message, headerpos + header_size + sizeof(": Msg") - 1,
                    strlen(headerpos + header_size + sizeof(": Msg") -1)+1);
            got_encrypted_msg(acct->gc, name, message);
            GE_set_rx_encryption(acct, name, TRUE);
         } else {
            gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption",
                       "Invalid Gaim-Encryption packet type\n"); 
         }            
      } else if (notifypos != 0) {
         GE_set_rx_encryption(acct, name, FALSE);
         GE_set_capable(acct, name, TRUE);
         if (gaim_prefs_get_bool("/plugins/gtk/encrypt/encrypt_if_notified")) {
            GE_set_tx_encryption(acct, name, TRUE);
         }
      } else {  /* No encrypt-o-header */
         GE_set_rx_encryption(acct, name, FALSE);
         gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "No header: %s\n", *message);
      }
   }
   
	if (*message) {
		return FALSE;
	}
	else {
		return TRUE;
	}

   g_free(name);
}

static void got_encrypted_msg(GaimConnection *gc, const char* name, char **message){
   unsigned char send_key_sum[KEY_DIGEST_LENGTH], recv_key_sum[KEY_DIGEST_LENGTH];
   unsigned char *tmp_msg;
   crypt_key *priv_key, *pub_key;
   int msg_pos = 0;
   GaimConversation* conv;

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "got_encrypted_msg\n");

   if ( (sscanf(*message, ": S%10c: R%10c%n", send_key_sum, recv_key_sum, &msg_pos) < 2) ||
        (msg_pos == 0) ) {
      gaim_debug(GAIM_DEBUG_WARNING, "gaim-encryption", "Garbled msg header\n");
      return;
   }

   priv_key = GE_find_key_by_name(GE_my_priv_ring, gc->account->username, gc->account);
   pub_key = GE_get_key(gc, name);
   
   if (strncmp(priv_key->digest, recv_key_sum, KEY_DIGEST_LENGTH) != 0) {
      /*  Someone sent us a message, but didn't use our correct public key */
      GE_send_key(gc->account, name, 1, 0);
      gaim_debug(GAIM_DEBUG_WARNING, "gaim-encryption", 
                 "Digests aren't same: {%*s} and {%*s}\n",
                 KEY_DIGEST_LENGTH, priv_key->digest,
                 KEY_DIGEST_LENGTH, recv_key_sum);
      conv = gaim_find_conversation_with_account(name, gc->account);
      if (conv != 0) {
         gaim_conversation_write(conv, 0,
                                 _("Received message encrypted with wrong key"),
                                 GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
         gaim_conv_window_flash(gaim_conversation_get_window(conv));
      } else {
         gaim_debug(GAIM_DEBUG_WARNING, "gaim-encryption",
                    "Received msg with wrong key, "
                    "but can't write err msg to conv: %s\n", name);
      }
      g_free(*message);
      *message = NULL;
      return;
   }
   
   if (pub_key && (strncmp(pub_key->digest, send_key_sum, KEY_DIGEST_LENGTH) != 0)) {
      /* We have a key for this guy, but the digest didn't match.  Store the message */
      /* and ask for a new key */
      GE_del_key_from_ring(GE_buddy_ring, name, gc->account);
      pub_key = GE_get_key(gc, name); /* will be 0 now */
   }
   
   if (pub_key == 0) {
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "g_e_m: Storing message on Show stack\n");
      GE_store_msg(name, gc, *message, &first_inc_msg, &last_inc_msg);
      g_free(*message);
      *message = NULL;
      return;
   }
   
   memmove(*message, *message + msg_pos, strlen(*message + msg_pos) + 1);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "attempting decrypt on '%s'\n", *message);

   if (decrypt_msg(&tmp_msg, *message, name, priv_key, pub_key) < 0) {     
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error in decrypt\n");
      conv = gaim_find_conversation_with_account(name, gc->account);
      if (conv != 0) {
         gaim_conversation_write(conv, 0,
                                 _("Error in decryption- asking for resend..."),
                                 GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
         gaim_conv_window_flash(gaim_conversation_get_window(conv));
      } else {
         gaim_debug(GAIM_DEBUG_WARNING, "gaim-encryption",
                    "Asking for resend, but can't write err msg to conv: %s\n", name);
      }
      GE_send_key(gc->account, name, 1, tmp_msg);
      g_free(*message);
      if (tmp_msg) g_free(tmp_msg);
      *message = NULL;     
      return;
   }
   
   /* Successful Decryption */

   /* Note- we're feeding gaim an arbitrarily formed message, which could
      potentially have lots of nasty control characters and stuff.  But, that
      has been tested, and at present, at least, Gaim won't barf on any
      characters that we give it.

      As an aside- Gaim does now use g_convert() to convert to UTF-8 from
      other character streams.  If we wanted to be all i18n, we could
      do the same, and even include the encoding type with the message.
      We're not all that, at least not yet.
   */
      
   /* Why the extra space (and the extra buffered copy)?  Well, the  *
    * gaim server.c code does this, and having the extra space seems *
    * to prevent at least one possible type of crash.  Pretty scary. */

   g_free(*message);
   *message = g_malloc(MAX(strlen(tmp_msg) + 1, BUF_LONG));
   strcpy(*message, tmp_msg);
}

/* Get account-specific message size limit*/

static int GE_get_msg_size_limit(GaimAccount *acct) {
   const char* protocol_id = gaim_account_get_protocol_id(acct);

   if (strcmp(protocol_id, "prpl-yahoo") == 0) {
      return 945;
   } else if (strcmp(protocol_id, "prpl-msn") == 0) {
      return 1500; /* This may be too small... somewhere in the 1500-1600 (+ html on front/back) */
   } else {
      /* Well, ok, this isn't too exciting.  Someday we can actually check  */
      /* to see what the real limits are.  For now, 2500 works for everyone */
      /* but Yahoo.                                                         */
      return 2500;
   }
}      

static void GE_send_msg_cb(GaimAccount *acct, char *who, char **message, void* data) {
   unsigned char *out_msg, *crypt_msg = 0;
   char *dupname;
   int msgsize;
   const char msg_format[] = "%s: Msg:S%.10s:R%.10s: Len %d:%s%s";
   GaimConversation *conv;
   crypt_key *our_key, *his_key;
   GSList *cur_msg;
   GQueue *sent_msg_queue;
   GE_SentMessage *sent_msg_item;

   int unencrypted_size_limit, msg_size_limit;
   int baggage_size;
   char baggage[BUF_LONG];

   const gchar* header = g_hash_table_lookup(header_table, gaim_account_get_protocol_id(acct));
   const gchar* footer = g_hash_table_lookup(footer_table, gaim_account_get_protocol_id(acct));
   const gchar* notify = g_hash_table_lookup(notify_table, gaim_account_get_protocol_id(acct));

   if (!header) header = header_default;
   if (!footer) footer = "";

   msg_size_limit = GE_get_msg_size_limit(acct);

   /* who: name that you are sending to */
   /* gc->username: your name           */

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "send_msg: %s\n", who);

   /* Since we don't have a periodic callback, we do some housekeeping here */
   gaim_conversation_foreach(reap_old_sent_messages);

   /* Message might have been eaten by another plugin: */
   if ((message == NULL) || (*message == NULL)) {
      return;
   }

   conv = gaim_find_conversation_with_account(who, acct);

   if (conv == NULL) {
      conv = gaim_conversation_new(GAIM_CONV_IM, acct, who);
   }

   if( GE_get_tx_encryption(acct, who) == FALSE) {
      if (notify && gaim_prefs_get_bool("/plugins/gtk/encrypt/broadcast_notify")
          && !GE_has_been_notified(acct, who)) {
         GE_set_notified(acct, who, TRUE);
         if (GE_msg_starts_with_link(*message) == TRUE) {
            /* This is a hack- AOL's client has a bug in the html parsing
               so that adjacent links (like <a href="a"></a><a href="b"></a>)
               get concatenated (into <a href="ab"></a>).  So we insert a
               space if the first thing in the message is a link.
            */
            out_msg = g_strconcat(notify, " ", *message, 0);
         } else {
            out_msg = g_strconcat(notify, *message, 0);
         }
         g_free(*message);
         *message = out_msg;
      }
      gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Outgoing Msg::%s::\n", *message);
      return;
   }

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "send_msg B: %s, %p, %p, %p\n",
              who, &GE_my_priv_ring, acct, conv);

   our_key = GE_find_own_key_by_name(&GE_my_priv_ring, acct->username, acct, conv);
   
   if (!our_key) {
      message[0] = 0; /* Nuke message so it doesn't look like it was sent.       */
                      /* find_own_key (above) will have displayed error messages */
		gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "leaving\n");

      return;
   }

   dupname = g_strdup(gaim_normalize(acct, who));
   his_key = GE_get_key(acct->gc, dupname);

   if (his_key == 0) { /* Don't have key for this guy yet */
      /* GE_get_key will have sent the key request, just let user know */

      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "requesting key\n");
      gaim_conversation_write(conv, 0, _("Requesting key..."),
                              GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
      gaim_conv_window_flash(gaim_conversation_get_window(conv));

      GE_store_msg(who, acct->gc, *message, &first_out_msg, &last_out_msg);

   } else {  /* We have a key.  Encrypt and send. */
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "has key\n", dupname);
      baggage_size = sprintf(baggage, msg_format, header, our_key->digest,
                             his_key->digest, 10000, "", footer);
      
      /* Warning:  message_split keeps static copies, so if our */
      /*   caller uses it, we're hosed.  Looks like nobody else */
      /*   uses it now, though.                                 */
      unencrypted_size_limit =
         GE_calc_unencrypted_size(our_key, his_key, msg_size_limit - baggage_size);

      cur_msg = GE_message_split(*message, unencrypted_size_limit);
      while (cur_msg) {
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "im_write: %s\n", dupname);
         gaim_conv_im_write(GAIM_CONV_IM(conv), NULL, cur_msg->data,
                            GAIM_MESSAGE_SEND, time((time_t)NULL));

         /* Add message to stash of sent messages: in case a key or nonce is wrong, we */
         /* can then re-send the message when asked.                                   */
         sent_msg_queue = g_hash_table_lookup(conv->data, "sent messages");
         sent_msg_item = g_malloc(sizeof(GE_SentMessage));
         sent_msg_item->time = time(0);
         sent_msg_item->id = GE_make_key_id(his_key);   /* current nonce value */
         sent_msg_item->msg = g_strdup(cur_msg->data);

         g_queue_push_head(sent_msg_queue, sent_msg_item);

         GE_encrypt_signed(&crypt_msg, cur_msg->data, our_key, his_key);
         msgsize = strlen(crypt_msg);

         out_msg = g_malloc(msgsize + baggage_size + 1);
      
         sprintf(out_msg, msg_format, header,
                 our_key->digest, his_key->digest, msgsize, crypt_msg,
                 footer);
         serv_send_im(acct->gc, who, out_msg, 0);
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
                    "send_im: %s: %d\n", who, strlen(out_msg));
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
                    "outgoing:%s:\n", out_msg);
         g_free(out_msg);
         g_free(crypt_msg);
         cur_msg = cur_msg->next;
         /* if (gaim_prefs_get_bool("/gaim/gtk/conversations/im/hide_on_send")) {
            gaim_window_hide(gaim_conversation_get_window(conv));
            } */
      }
   }

   message[0] = 0;
   g_free(dupname);

	return;
}


void GE_resend_msg(GaimAccount* acct, const char* name, gchar *msg_id) {
   unsigned char *out_msg, *crypt_msg = 0, *msg = 0;
   GaimConversation* conv = gaim_find_conversation_with_account(name, acct);
   int msgsize;
   const char msg_format[] = "%s: Msg:S%.10s:R%.10s: Len %d:%s%s";
   crypt_key *our_key, *his_key;

   GQueue *sent_msg_queue;
   GE_SentMessage *sent_msg_item;

   int baggage_size;
   char baggage[BUF_LONG];
   const gchar *header, *footer;

   if (msg_id == 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Bad call to resend_msg: %p %p\n", conv, msg_id);
      return;
   }

   if (conv == 0) {
      conv = gaim_conversation_new(GAIM_CONV_IM, acct, name);
   }

   header = g_hash_table_lookup(header_table, gaim_account_get_protocol_id(conv->account));
   footer = g_hash_table_lookup(footer_table, gaim_account_get_protocol_id(conv->account));

   if (!header) header = header_default;
   if (!footer) footer = "";

   /*Sometimes callers don't know whether there's a msg to send... */
   if (msg_id == 0 || conv == 0) return;

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
              "resend_encrypted_msg: %s:%s\n", conv->name, msg_id);

   our_key = GE_find_key_by_name(GE_my_priv_ring, conv->account->username, conv->account);

   his_key = GE_find_key_by_name(GE_buddy_ring, name, conv->account);

   if (his_key == 0) { /* Don't have key for this guy */
      gaim_conversation_write(conv, 0,
                              _("No key to resend message.  Message lost."),
                              GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
      gaim_conv_window_flash(gaim_conversation_get_window(conv));

   } else {  /* We have a key.  Encrypt and send. */

      sent_msg_queue = g_hash_table_lookup(conv->data, "sent messages");

      /* Root through the queue looking for the right message.  Any that are older than this */
      /* one we will throw out, since they would have already been asked for.                */

      while (!g_queue_is_empty(sent_msg_queue)) {
         sent_msg_item = g_queue_pop_tail(sent_msg_queue);
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Examining Message: %s\n",
                    sent_msg_item->id);
         
         if (strcmp(sent_msg_item->id, msg_id) == 0) { /* This is the one to resend */
            msg = sent_msg_item->msg;
            g_free(sent_msg_item->id);
            g_free(sent_msg_item);
            break; 
         }
         /* Not the one to resend: pitch it */
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "  Deleted\n");
         g_free(sent_msg_item->id);
         g_free(sent_msg_item->msg);
         g_free(sent_msg_item);
      }
      
      if (msg) {
         baggage_size = sprintf(baggage, msg_format, header, our_key->digest,
                                his_key->digest, 10000, "", footer);
      
         GE_encrypt_signed(&crypt_msg, msg, our_key, his_key);
         msgsize = strlen(crypt_msg);
         out_msg = g_malloc(msgsize + baggage_size + 1);
         
         sprintf(out_msg, msg_format, header,
                 our_key->digest, his_key->digest, msgsize, crypt_msg,
                 footer);
         gaim_conversation_write(conv, 0,
                                 "Resending...",
                                 GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
         serv_send_im(conv->account->gc, name, out_msg, 0);
         
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
                    "resend_im: %s: %d\n", name, strlen(out_msg));
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption",
                    "resend outgoing:%s:\n", out_msg);
         g_free(msg);
         g_free(out_msg);
         g_free(crypt_msg);
      } else {
         gaim_conversation_write(conv, 0, _("Outgoing message lost."),
                                 GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
      }
   } 
}



static void GE_new_conv(GaimConversation *conv) {
   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "New conversation\n");

   if ((conv != NULL) && (gaim_conversation_get_type(conv) == GAIM_CONV_IM)) {
      g_hash_table_insert(conv->data, g_strdup("sent messages"), g_queue_new());
      g_hash_table_insert(conv->data, g_strdup("sent_capable"), FALSE);
      GE_add_buttons(conv);
   } else {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "New conversation IS NULL\n");
   }
}

static void GE_new_conv_cb(GaimConversation *conv, void* data) {
   GE_new_conv(conv);
}

static void GE_del_conv_cb(GaimConversation *conv, void* data)
{
   GQueue *sent_msg_queue;

   if ((conv != NULL) && (gaim_conversation_get_type(conv) == GAIM_CONV_IM)) {
      gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
                 "Got conversation delete event for %s\n", conv->name);
      
      /* Remove cached copies of sent messages */
      reap_all_sent_messages(conv);
      sent_msg_queue = g_hash_table_lookup(conv->data, "sent messages");
      g_queue_free(sent_msg_queue);
      g_hash_table_remove(conv->data, "sent messages");
      
      /* Remove to-be-sent-on-receipt-of-key messages: */
      GE_delete_stored_msgs(conv->account, gaim_normalize(conv->account, conv->name));
      
      GE_buddy_ring = GE_del_key_from_ring(GE_buddy_ring,
                                           gaim_normalize(conv->account, conv->name), conv->account);
      
      /* Would be good to add prefs for these, but for now, just reset: */
      GE_reset_state(conv->account, conv->name);
      
      gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption",
                 "Finished conversation delete event for %s\n", conv->name);   
      /* button widgets (hopefully) destroyed on window close            */
      /* hash table entries destroyed on hash table deletion, except     */
      /*   for any dynamically allocated values (keys are ok).           */ 
   }
}  

static void GE_headers_init() {
   header_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
   footer_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
   notify_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

   g_hash_table_insert(header_table, g_strdup("prpl-toc"),
                       g_strdup("*** Encrypted with the Gaim-Encryption plugin <A HREF=\""));
   g_hash_table_insert(footer_table, g_strdup("prpl-toc"),
                       g_strdup("\"></A>"));
   g_hash_table_insert(notify_table, g_strdup("prpl-toc"),
                       g_strdup("<A HREF=\"Gaim-Encryption Capable\"></A>"));

   g_hash_table_insert(header_table, g_strdup("prpl-oscar"),
                       g_strdup("*** Encrypted with the Gaim-Encryption plugin <A HREF=\""));
   g_hash_table_insert(footer_table, g_strdup("prpl-oscar"),
                       g_strdup("\"></A>"));
   g_hash_table_insert(notify_table, g_strdup("prpl-oscar"),
                       g_strdup("<A HREF=\"Gaim-Encryption Capable\"></A>"));

/* If jabber stops stripping HTML, we can go back to these headers */
/*    g_hash_table_insert(header_table, g_strdup("prpl-jabber"), */
/*                        g_strdup("*** Encrypted with the Gaim-Encryption plugin <A HREF='")); */
/*    g_hash_table_insert(footer_table, g_strdup("prpl-jabber"), */
/*                        g_strdup("'></A>")); */
/*    g_hash_table_insert(notify_table, g_strdup("prpl-jabber"), */
/*                        g_strdup("<A HREF='Gaim-Encryption Capable'> </A>")); */


   g_hash_table_insert(header_table, g_strdup("prpl-jabber"),
                       g_strdup("*** Encrypted with the Gaim-Encryption plugin "));
   g_hash_table_insert(footer_table, g_strdup("prpl-jabber"),
                       g_strdup(" "));
   g_hash_table_insert(notify_table, g_strdup("prpl-jabber"),
                       g_strdup("<A HREF='Gaim-Encryption Capable'> </A>"));

   header_default = g_strdup("*** Encrypted :");
}

/* #define CRYPT_HEADER "*** Encrypted with the Gaim-Encryption plugin <A HREF=\"" */
/* #define CRYPT_FOOTER "\"></A>" */
/* #define CRYPT_NOTIFY_HEADER "<A HREF=\"Gaim-Encryption Capable\"></A>" */

// Jabber seems to turn our double quotes into single quotes at times, so define
// the same headers, only with single quotes.  Lengths MUST be the same as above
/* #define CRYPT_HEADER_MANGLED "*** Encrypted with the Gaim-Encryption plugin <A HREF='" */
/* #define CRYPT_NOTIFY_HEADER_MANGLED "<A HREF='Gaim-Encryption Capable'></A>" */


static void init_prefs() {
   /* These only add/set a pref if it doesn't currently exist: */

   int default_width;

   if (gaim_prefs_get_type("/plugins/gtk/encrypt/accept_unknown_key") == GAIM_PREF_NONE) {
      /* First time loading the plugin, since we don't have our prefs set yet */

      /* so up the default window width to accomodate new buttons */
      default_width = gaim_prefs_get_int("/gaim/gtk/conversations/im/default_width");

      if (default_width == 410) { /* the stock gaim default width */
         gaim_prefs_set_int("/gaim/gtk/conversations/im/default_width", 490);
      }
   }

   gaim_prefs_add_none("/plugins/gtk");
   gaim_prefs_add_none("/plugins/gtk/encrypt");
   
   gaim_prefs_add_bool("/plugins/gtk/encrypt/accept_unknown_key", FALSE);
   gaim_prefs_add_bool("/plugins/gtk/encrypt/accept_conflicting_key", FALSE);
   gaim_prefs_add_bool("/plugins/gtk/encrypt/encrypt_response", TRUE);
   gaim_prefs_add_bool("/plugins/gtk/encrypt/broadcast_notify", FALSE);
   gaim_prefs_add_bool("/plugins/gtk/encrypt/encrypt_if_notified", TRUE);

   GE_convert_legacy_prefs();
}

/* Called by Gaim when plugin is first loaded */
static gboolean GE_plugin_load(GaimPlugin *h) {

	void *conv_handle;

#ifdef ENABLE_NLS
   bindtextdomain (ENC_PACKAGE, LOCALEDIR);
   bind_textdomain_codeset (ENC_PACKAGE, "UTF-8");
   setlocale(LC_ALL, "");
#endif

   if (strcmp(gaim_core_get_version(), VERSION) != 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption",
                 "Compiled with Gaim v'%s', running with v'%s'.\n",
                 VERSION, gaim_core_get_version());
      /* GE_error_window(_("Gaim-Encryption plugin was compiled with a different " */
      /*                   "version of Gaim.  You may experience problems.")); */
   }

   init_prefs();

   conv_handle = gaim_conversations_get_handle();
   
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "plugin_load called\n");
   GE_plugin_handle = h;

   GE_state_init();
   GE_pixmap_init();

   if (!rsa_nss_init()) {
      return FALSE;
   }

   GE_key_rings_init();
   GE_nonce_map_init();

   GE_headers_init();

   gaim_signal_connect(conv_handle, "receiving-im-msg", h, 
                       GAIM_CALLBACK(GE_got_msg_cb), NULL);
   gaim_signal_connect(conv_handle, "sending-im-msg", h,
                       GAIM_CALLBACK(GE_send_msg_cb), NULL);
   gaim_signal_connect(conv_handle, "conversation-created", h,
                       GAIM_CALLBACK(GE_new_conv_cb), NULL);
   gaim_signal_connect(conv_handle, "deleting-conversation", h, 
                       GAIM_CALLBACK(GE_del_conv_cb), NULL);
  
   gaim_signal_connect(gaim_blist_get_handle(), "blist-node-extended-menu", h,
                       GAIM_CALLBACK(GE_buddy_menu_cb), NULL);


   gaim_conversation_foreach(GE_add_buttons);

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "done loading\n");


   return TRUE;
}

/* Called by Gaim when plugin is removed */
static gboolean GE_plugin_unload(GaimPlugin *h) {
   void * conv_handle = gaim_conversations_get_handle();

   gaim_signal_disconnect(conv_handle, "receiving-im-msg", h, 
                          GAIM_CALLBACK(GE_got_msg_cb));
   gaim_signal_disconnect(conv_handle, "sending-im-msg", h,
                          GAIM_CALLBACK(GE_send_msg_cb));
   gaim_signal_disconnect(conv_handle, "conversation-created", h,
                          GAIM_CALLBACK(GE_new_conv_cb));
   gaim_signal_disconnect(conv_handle, "deleting-conversation", h, 
                          GAIM_CALLBACK(GE_del_conv_cb));
   
   gaim_signal_disconnect(gaim_blist_get_handle(), "blist-node-extended-menu", h,
                          GAIM_CALLBACK(GE_buddy_menu_cb));

   GE_config_unload();
   
   gaim_conversation_foreach(GE_remove_buttons);

   GE_my_priv_ring = GE_clear_ring(GE_my_priv_ring);
   GE_my_pub_ring = GE_clear_ring(GE_my_pub_ring);
   GE_buddy_ring = GE_clear_ring(GE_buddy_ring);

   GE_state_delete();
   return TRUE;
}

static GaimGtkPluginUiInfo ui_info =
{
   GE_get_config_frame
};

static GaimPluginInfo info =
{
   GAIM_PLUGIN_MAGIC,                                /**< I'm a plugin!  */
   GAIM_MAJOR_VERSION,
   GAIM_MINOR_VERSION,
   GAIM_PLUGIN_STANDARD,                             /**< type           */
   GAIM_GTK_PLUGIN_TYPE,                             /**< ui_requirement */
   0,                                                /**< flags          */
   NULL,                                             /**< dependencies   */
   GAIM_PRIORITY_DEFAULT,                            /**< priority       */

   ENCRYPT_PLUGIN_ID,                                /**< id             */
   0,                                                /**< name           */
   ENC_VERSION " (Gaim " VERSION ")",                /**< version        */
   0,                                                /**  summary        */
   0,                                                /**  description    */
   0,                                                /**< author         */
   ENC_WEBSITE,                                      /**< homepage       */

   GE_plugin_load,                                   /**< load           */
   GE_plugin_unload,                                 /**< unload         */
   NULL,                                             /**< destroy        */

   &ui_info,                                         /**< ui_info        */
   NULL,                                             /**< extra_info     */
   NULL,                                             /**< prefs_info     */
   NULL                                              /**< actions        */
};

static void
init_plugin(GaimPlugin *plugin)
{

#ifdef ENABLE_NLS
   bindtextdomain (ENC_PACKAGE, LOCALEDIR);
   bind_textdomain_codeset (ENC_PACKAGE, "UTF-8");
   setlocale(LC_ALL, "");
#endif

   info.name = _("Gaim-Encryption");
   info.summary = _("Encrypts conversations with RSA encryption.");
   info.description = _("RSA encryption with keys up to 4096 bits,"
                        " using the Mozilla NSS crypto library.\n");
   /* Translators: Feel free to add your name to the author field, with text like  */
   /*   "Bill Tompkins, translation by Phil McGee"                                   */
   info.author = _("Bill Tompkins");

}


GAIM_INIT_PLUGIN(gaim_encryption, init_plugin, info);
