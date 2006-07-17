/*  Protocol-independent Key structures                                   */
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

/* #define GAIM_PLUGINS */

#include <glib.h>
#if GLIB_CHECK_VERSION(2,6,0)
#	include <glib/gstdio.h>
#else
#	define g_freopen freopen
#	define g_fopen fopen
#	define g_rmdir rmdir
#	define g_remove remove
#	define g_unlink unlink
#	define g_lstat lstat
#	define g_stat stat
#	define g_mkdir mkdir
#	define g_rename rename
#	define g_open open
#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk/gtkplug.h>

#include <debug.h>
#include <gaim.h>
#include <util.h>

#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "keys.h"
#include "cryptutil.h"
#include "prefs.h"
#include "encrypt.h"
#include "keys_ui.h"
#include "ge_ui.h"
#include "nls.h"

#ifdef _WIN32
#include "win32dep.h"
#endif

/* List of all the keys we know about */
key_ring *GE_buddy_ring = 0, *GE_saved_buddy_ring = 0, *GE_my_priv_ring = 0, *GE_my_pub_ring = 0;

typedef enum {KEY_MATCH, KEY_NOT_THERE, KEY_CONFLICT} KeyCheckVal;


static KeyCheckVal GE_check_known_key(const char *filename, key_ring_data* key);


crypt_key * GE_find_key_by_name(key_ring *ring, const char *name, GaimAccount *acct) {
   key_ring *i = GE_find_key_node_by_name(ring, name, acct);
   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "find key by name: %s\n", name);
   return (i == NULL) ? NULL : ((key_ring_data *)i->data)->key;
}

crypt_key * GE_find_own_key_by_name(key_ring **ring, char *name, GaimAccount *acct, GaimConversation *conv) {
   crypt_key *key = GE_find_key_by_name(*ring, name, acct);
   if (key) return key;

   /* Can't find the key, but it's ours, so we'll make one */
   gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error!  Can't find own key for %s\n",
              name);
   gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Dumping public keyring:\n");      
   GE_debug_dump_keyring(GE_my_pub_ring);
      
   if (conv != 0) {
      gaim_conversation_write(conv, "Encryption Manager",
                              _("Making new key pair..."),
                              GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
   }
   
   GE_make_private_pair((crypt_proto *)crypt_proto_list->data, name, conv->account, 1024);
   
   key = GE_find_key_by_name(*ring, name, conv->account);
   if (key) return key;

   /* Still no key: something is seriously wrong.  Probably having trouble saving the */
   /* key to the key file, or some such.                                              */

   gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error!  Can't make new key for %s\n",
              name);

   if (conv != 0) {
      gaim_conversation_write(conv, "Encryption Manager",
                              _("Error trying to make key."),
                              GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
   }
   
   return 0;
}

key_ring * GE_find_key_node_by_name(key_ring *ring, const char *name, GaimAccount* acct) {
   key_ring *i = 0;

   for( i = ring; i != NULL; i = i->next ) {
      if( (strncmp(name, ((key_ring_data *)i->data)->name, sizeof(((key_ring_data*)i->data)->name)) == 0 ) &&
          (acct == ((key_ring_data*)i->data)->account))
         break;
   }

   return (i == NULL) ? NULL : i;
}

void GE_debug_dump_keyring(key_ring * ring) {
   key_ring *i = 0;

   for( i = ring; i != NULL; i = i->next ) {
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Key ring::%*s::%p\n",
                 sizeof(((key_ring_data *)i->data)->name),
                 ((key_ring_data *)i->data)->name,
                 ((key_ring_data *)i->data)->account);
   }
}

/* add_key_to_ring will ensure that there is only one key on a ring that matches
   a given name.  So your buddy switches computers (and keys), we will discard
   his old key when he sends us his new one.                                  */

key_ring* GE_add_key_to_ring(key_ring* ring, key_ring_data* key) {
   key_ring* old_key = GE_find_key_node_by_name(ring, key->name, key->account);

   if (old_key != NULL) {
      ring = g_slist_remove_link(ring, old_key);
   }
   ring = g_slist_prepend(ring, key);
   return ring;
}

key_ring* GE_del_key_from_ring(key_ring* ring, const char* name, GaimAccount* acct) {
   key_ring* old_key = GE_find_key_node_by_name(ring, name, acct);

   if (old_key != NULL) {
      gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Removing key for %s\n", name);
      ring = g_slist_remove_link(ring, old_key);
   }
   return ring;
}

key_ring* GE_clear_ring(key_ring* ring) {
   crypt_key* key;
   key_ring *iter = ring;
   
   while (iter != NULL) {
      key = ((key_ring_data *)(iter->data))->key;
      GE_free_key(key);
      g_free(iter->data);
      iter = iter->next;
   }
   g_slist_free(ring);
   return NULL;
}

void GE_received_key(char *key_msg, char *name, GaimAccount* acct, GaimConversation* conv, char** orig_msg) {
   GSList *protoiter;
   crypt_proto* proto=0;
   unsigned char* key_len_msg=0;
   unsigned int length;
	int realstart;
   gchar** after_key;
   gchar* resend_msg_id = 0;

   key_ring_data *new_key;
   KeyCheckVal keycheck_return;
   
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "received_key\n");
   
   if (strncmp(key_msg, ": Prot ", sizeof(": Prot ") - 1) != 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error in received key\n");
      return;
   }
   key_msg += sizeof(": Prot ") - 1;

   protoiter = crypt_proto_list;
   while (protoiter != 0 && proto == 0) {
      if( (key_len_msg = 
           ((crypt_proto *)protoiter->data)->parseable(key_msg)) != 0 ) {
         proto = ((crypt_proto *) protoiter->data);
      }
   }
   if (proto == 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Unknown protocol type: %10s\n", key_msg);
      return;
   }

   if ( (sscanf(key_len_msg, ": Len %u:%n", &length, &realstart) < 1) ||
        (realstart == 0) ) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error in key header\n");
      return;
   }

   key_len_msg += realstart;
   if (strlen(key_len_msg) < length) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Length doesn't match in add_key\n");
      return;
   }

   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "After key:%s\n", key_len_msg+length);   
   after_key = g_strsplit(key_len_msg+length, ":", 3);
   if (after_key[0] && (strcmp(after_key[0], "Resend") == 0)) {
      if (after_key[1]) {
         resend_msg_id = g_strdup(after_key[1]);
      }
   }
   g_strfreev(after_key);

   key_len_msg[length] = 0;
   
   /* Make a new node for the linked list */
   new_key = g_malloc(sizeof(key_ring_data));
   new_key->account = acct;
   new_key->key = proto->parse_sent_key(key_len_msg);

   if (new_key->key == 0) {
      g_free(new_key);
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Invalid key received\n");
      return;
   }

   strncpy(new_key->name, name, sizeof(new_key->name));
   
   keycheck_return = GE_check_known_key(Buddy_key_file, new_key);

   /* Now that we've pulled the key out of the original message, we can free it */
   /* so that (maybe) a stored message can be returned in it                    */

   (*orig_msg)[0] = 0;
   g_free(*orig_msg);
   *orig_msg = 0;
   
   switch(keycheck_return) {
   case KEY_NOT_THERE:
      GE_choose_accept_unknown_key(new_key, resend_msg_id, conv);
      break;
   case KEY_MATCH:
      GE_buddy_ring = GE_add_key_to_ring(GE_buddy_ring, new_key);
      GE_send_stored_msgs(new_key->account, new_key->name);
      GE_show_stored_msgs(new_key->account, new_key->name, orig_msg);
      if (resend_msg_id) {
         GE_resend_msg(new_key->account, new_key->name, resend_msg_id);
      }

      break;
   case KEY_CONFLICT:
      if (conv) {
         gaim_conversation_write(conv, "Encryption Manager", _("Conflicting Key Received!"),
                                 GAIM_MESSAGE_SYSTEM, time((time_t)NULL));
      }
      GE_choose_accept_conflict_key(new_key, resend_msg_id, conv);
      break;
   }
   if (resend_msg_id) {
      g_free(resend_msg_id);
      resend_msg_id = 0;
   }
}

static KeyCheckVal GE_check_known_key(const char* filename, key_ring_data* key) {
   char line[MAX_KEY_STORLEN];
   GString *line_str, *key_str, *name_str;
   char path[4096];
   
   struct stat fs;
   FILE* fp;
   int fd;
   int found_name = 0;
   
   g_snprintf(path, sizeof(path), "%s%s%s", gaim_user_dir(), G_DIR_SEPARATOR_S, filename);
      
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Checking key file %s for name %s\n", path, key->name);

   /* check file permissions */
   if (stat(path, &fs) == -1) {
      /* file doesn't exist, so make it */
      fd = g_open(path, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
      if (fd == -1) {
         /* Ok, maybe something else strange is going on... */
         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error trying to create a known key file\n");
         return KEY_NOT_THERE;
      }
      fstat(fd, &fs);
      fchmod(fd, fs.st_mode & S_IRWXU);  /* zero out non-owner permissions */
      close(fd);
   } else {
#ifdef S_IWGRP
      /* WIN32 doesn't have user-based file permissions, so skips this */
      if (fs.st_mode & (S_IWGRP | S_IWOTH)) {
         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Invalid permissions, rejecting file: %s\n", path);
         return KEY_CONFLICT;
      }
#endif
   }

   /* build string from key */
   name_str = g_string_new(key->name);
   GE_escape_name(name_str);
   g_string_append_printf(name_str, ",%s", gaim_account_get_protocol_id(key->account));
   line_str = g_string_new(name_str->str);
   g_string_append_printf(line_str, " %s ", key->key->proto->name);
   key_str = GE_key_to_gstr(key->key);
   g_string_append(line_str, key_str->str);

   /*   gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "built line '%s'\n", line_str->str); */

   /* look for key in file */   
   if( (fp = g_fopen(path, "r")) != NULL ) {
      while (!feof(fp)) {
         fgets(line, sizeof(line), fp);
         /* gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "checking line '%s'\n", line); */
         if ( (strchr(line, ' ') == line + name_str->len) &&
              (strncmp(line_str->str, line, name_str->len) == 0) ) {
            gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Got Name\n");
            found_name = 1;
            if (strncmp(line_str->str, line, line_str->len) == 0) {
               gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "Got Match\n");
               fclose(fp);
               g_string_free(line_str, TRUE);
               g_string_free(key_str, TRUE);
               g_string_free(name_str, TRUE);
               return KEY_MATCH;
            }
         }
      }
      fclose(fp);
   }

   g_string_free(line_str, TRUE);
   g_string_free(key_str, TRUE);
   g_string_free(name_str, TRUE);
   
   if (found_name) return KEY_CONFLICT;
   return KEY_NOT_THERE;
}

/* For now, we'll make all key files privately owned, even though the
   id.pub and known_keys files could be public.                        */
   
void GE_add_key_to_file(const char *filename, key_ring_data* key) {
   GString *line_str, *key_str;

   char path[4096];
   char errbuf[500];
   
   FILE* fp;
   int fd;
   char c;
   struct stat fdstat;
   
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Saving key to file:%s:%p\n", key->name, key->account);
   g_snprintf(path, sizeof(path), "%s%s%s", gaim_user_dir(), G_DIR_SEPARATOR_S, filename);

   fd = g_open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
   if (fd == -1) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error opening key file %s for write\n", path);
      /* WIN32 doesn't have user-based file permissions, so skips this */
#ifdef S_IRWXG
      if (chmod(path, S_IRUSR | S_IWUSR) == -1) {

         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Unable to change file mode, aborting\n");
         g_snprintf(errbuf, sizeof(errbuf),
                    _("Error changing access mode for file: %s\nCannot save key."), filename);
         GE_ui_error(errbuf);
         return;
      }
#endif
      fd = g_open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      if (fd == -1) {
         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Changed mode, but still wonky.  Aborting.\n");
         g_snprintf(errbuf, sizeof(errbuf),
                    _("Error (2) changing access mode for file: %s\nCannot save key."), filename);
         GE_ui_error(errbuf);
         return;
      } else {
         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Key file '%s' no longer read-only.\n");
      }
   }
   fstat(fd, &fdstat);
#ifdef S_IRWXG
   /* WIN32 doesn't have user-based file permissions, so skips this */
   if (fdstat.st_mode & (S_IRWXG | S_IRWXO)) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Bad permissions on key file: %s\n", path);
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "I won't save to a world-accesible key file.\n");
      g_snprintf(errbuf, sizeof(errbuf),
                 _("Bad permissions on key file: %s\nGaim-Encryption will not save keys to a world- or group-accessible file."),
                   filename);
      GE_ui_error(errbuf);
      return;
   } 
#endif

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "%p\n", gaim_account_get_protocol_id(key->account));
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "%s\n", gaim_account_get_protocol_id(key->account));
   line_str = g_string_new(key->name);
   GE_escape_name(line_str);
   g_string_append_printf(line_str, ",%s", gaim_account_get_protocol_id(key->account));
   g_string_append_printf(line_str, " %s ", key->key->proto->name);
   key_str = GE_key_to_gstr(key->key);
   g_string_append(line_str, key_str->str);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "here\n");
   /* To be nice to users... we'll allow the last key in the file to not    */
   /* have a trailing \n, so they can cut-n-paste with abandon.             */
   fp = fdopen(fd, "r");   
   fseek(fp, -1, SEEK_END);
   c = fgetc(fp);
   if (feof(fp)) c = '\n';  /*if file is empty, we don't need to write a \n */
   fclose(fp);
   
   fd = g_open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
   fp = fdopen(fd, "a+");
   fseek(fp, 0, SEEK_END);   /* should be unnecessary, but needed for WIN32 */
   
   if (c != '\n') fputc('\n', fp);
   fputs(line_str->str, fp);
   fclose(fp);

   g_string_free(key_str, TRUE);
   g_string_free(line_str, TRUE);
}

void GE_del_one_key_from_file(const char *filename, int key_num, const char *name) {
   char line[MAX_KEY_STORLEN];
   char path[4096], tmp_path[4096];
   int foundit = 0;
   FILE *fp, *tmp_fp;
   int fd;
   int line_num;

   GString *line_start, *old_style_start, *normalized_start;

   line_start = g_string_new(name);
   GE_escape_name(line_start);
   g_string_append_printf(line_start, ",");

   old_style_start = g_string_new(name);
   GE_escape_name(old_style_start);
   g_string_append_printf(old_style_start, " ");

   normalized_start = g_string_new(name);
   GE_escape_name(normalized_start);
   g_string_append_printf(normalized_start, " ");

   g_snprintf(path, sizeof(path), "%s%s%s", gaim_user_dir(), G_DIR_SEPARATOR_S, filename);
   
   /* Look for name in the file.  If it's not there, we're done */
   fp = g_fopen(path, "r");

   if (fp == NULL) {
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);
      return;
   }

   for (line_num = 0; line_num <= key_num; ++line_num) {
      fgets(line, sizeof(line), fp);
   }

   if ( (strncmp(line, line_start->str, line_start->len) == 0) ||
        (strncmp(line, old_style_start->str, old_style_start->len) == 0) ||
        (strncmp(line, normalized_start->str, normalized_start->len) == 0) ) {
      foundit = 1;
   }

   fclose(fp);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Delete one key: found(%d)\n", foundit);

   if (!foundit) {
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);
      return;
   }

   /* It's there.  Move file to a temporary, and copy the other lines */

   g_snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);
   rename(path, tmp_path);
   
   fd = g_open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
   if (fd == -1) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error opening key file %s\n", path);
      perror("Error opening key file");
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);

      return;
   }
   fp = fdopen(fd, "a+");
   
   tmp_fp = g_fopen(tmp_path, "r");
   if (tmp_fp == NULL) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Wah!  I moved a file and now it is gone\n");
      fclose(fp);
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);

      return;
   }

   line_num = 0;
   while (fgets(line, sizeof(line), tmp_fp)) {
      if (line_num != key_num) {
         fputs(line, fp);
      } else {
         gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Skipping line %d\n", line_num);
      }
      ++line_num;
   }
   
   fclose(fp);
   fclose(tmp_fp);
   unlink(tmp_path);
   g_string_free(line_start, TRUE);
}


void GE_del_key_from_file(const char *filename, const char *name, GaimAccount *acct) {
   char line[MAX_KEY_STORLEN];
   char path[4096], tmp_path[4096];
   int foundit = 0;
   FILE *fp, *tmp_fp;
   int fd;
   GString *line_start, *old_style_start, *normalized_start;

   line_start = g_string_new(name);
   GE_escape_name(line_start);
   if (acct != 0) {
      g_string_append_printf(line_start, ",%s", gaim_account_get_protocol_id(acct));
   } else {
      g_string_append_printf(line_start, ",");
   }

   old_style_start = g_string_new(name);
   GE_escape_name(old_style_start);
   g_string_append_printf(old_style_start, " ");

   normalized_start = g_string_new(name);
   GE_escape_name(normalized_start);
   g_string_append_printf(normalized_start, " ");

   g_snprintf(path, sizeof(path), "%s%s%s", gaim_user_dir(), G_DIR_SEPARATOR_S, filename);
   
   /* Look for name in the file.  If it's not there, we're done */
   fp = g_fopen(path, "r");

   if (fp == NULL) {
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);
      return;
   }

   while (fgets(line, sizeof(line), fp)) {
      if ( (strncmp(line, line_start->str, line_start->len) == 0) ||
           (strncmp(line, old_style_start->str, old_style_start->len) == 0) ||
           (strncmp(line, normalized_start->str, normalized_start->len) == 0) ) {
         foundit = 1;
      }
   }
   fclose(fp);
   if (!foundit) {
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);
      return;
   }

   /* It's there.  Move file to a temporary, and copy the lines */
   /* that don't match.                                         */
   g_snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);
   rename(path, tmp_path);
   
   fd = g_open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
   if (fd == -1) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error opening key file %s\n", path);
      perror("Error opening key file");
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);

      return;
   }
   fp = fdopen(fd, "a+");
   
   tmp_fp = g_fopen(tmp_path, "r");
   if (tmp_fp == NULL) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Wah!  I moved a file and now it is gone\n");
      fclose(fp);
      g_string_free(line_start, TRUE);
      g_string_free(old_style_start, TRUE);
      g_string_free(normalized_start, TRUE);

      return;
   }
   while (fgets(line, sizeof(line), tmp_fp)) {
      if ( (strncmp(line, line_start->str, line_start->len) != 0) &&
           (strncmp(line, old_style_start->str, old_style_start->len) != 0) &&
           (strncmp(line, normalized_start->str, normalized_start->len) != 0) ) {
         fputs(line, fp);
      }
   }
   
   fclose(fp);
   fclose(tmp_fp);
   unlink(tmp_path);
   g_string_free(line_start, TRUE);
}



key_ring * GE_load_keys(const char *filename) {
   FILE* fp;
   char name[64], nameacct[164], proto[20], proto_name[10], proto_ver[10], key_str_buf[MAX_KEY_STORLEN];
   char path[4096];
   int rv;
   key_ring *new_ring = 0;
   key_ring_data *new_key;
   GSList* proto_node;
   gchar **nameaccount_split;
   GaimAccount* account;


   g_snprintf(path, sizeof(path), "%s%s%s", gaim_user_dir(), G_DIR_SEPARATOR_S, filename);
   if( (fp = g_fopen(path, "r")) != NULL ) {
      do {
      	 /* 7999 = MAX_KEY_STORLEN - 1 */
         rv = fscanf(fp, "%163s %9s %9s %7999s\n", nameacct, proto_name,
                     proto_ver, key_str_buf);

         if( rv == 4 ) {
            if (strlen(key_str_buf) > MAX_KEY_STORLEN - 2) {                    
               gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Problem in key file.  Increase key buffer size.\n");
               continue;
            }
            nameaccount_split = g_strsplit(nameacct, ",", 2);
            strncpy(name, nameaccount_split[0], sizeof(name));
            name[sizeof(name)-1] = 0;
            GE_unescape_name(name);
            
            /* This will do the right thing: if no account, it will match any */
            account = gaim_accounts_find(name, nameaccount_split[1]);

            gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "load_keys: name(%s), protocol (%s): %p\n",
                       nameaccount_split[0],
                       ((nameaccount_split[1]) ? nameaccount_split[1] : "none"),
                        account);

            gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "%p\n", gaim_account_get_protocol_id(account));

            g_strfreev(nameaccount_split);

            /* gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "load_keys() %i: Read: %s:%s %s %s\n",
               __LINE__, filename, name, proto_name, proto_ver); */

            /* find the make_key_from_str for this protocol */
            g_snprintf(proto, sizeof(proto), "%s %s", proto_name, proto_ver);
            proto_node = crypt_proto_list;
            while (proto_node != NULL) {
               if (strcmp(((crypt_proto *)proto_node->data)->name, proto) == 0)
                  break;
               proto_node = proto_node->next;
            }

            if (proto_node == NULL) {
               gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "load_keys() %i: invalid protocol: %s\n",
                            __LINE__, proto);
               continue;
            }

            new_key = g_malloc(sizeof(key_ring_data));
            new_key->key = 
               ((crypt_proto *)proto_node->data)->make_key_from_str(key_str_buf);
            
            new_key->account = account;
            strncpy(new_key->name, name, sizeof(new_key->name));
            gaim_debug(GAIM_DEBUG_MISC, "gaim-encryption", "load_keys() %i: Added: %*s %s %s\n", __LINE__,
                         sizeof(new_key->name), new_key->name, proto_name, proto_ver);

            new_ring = g_slist_append(new_ring, new_key);
         } else if (rv > 0) {
            gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Bad key (%s) in file: %s\n", name, path);
         }
      } while( rv != EOF );

      fclose(fp);
   } else {
      if (errno != ENOENT) {
         gaim_debug(GAIM_DEBUG_WARNING, "gaim-encryption", "Couldn't open file:%s\n", path);
         perror("Error opening file");
      } else {
         gaim_debug(GAIM_DEBUG_WARNING, "gaim-encryption",
                    "File %s doesn't exist (yet).  A new one will be created.\n", path);
      }
   }

   return new_ring;
}

void GE_key_rings_init() {
   GSList *proto_node;
   GList *cur_sn;
   crypt_key *pub_key = 0, *priv_key = 0;
   key_ring_data *new_key;
   char *name;
   GaimAccount *acct;

   /* Load the keys */
   GE_my_pub_ring = GE_load_keys(Public_key_file);
   GE_my_priv_ring = GE_load_keys(Private_key_file);
   GE_saved_buddy_ring = GE_load_keys(Buddy_key_file);

   /* Create a key for each screen name if we don't already have one */
   
   for (cur_sn = gaim_accounts_get_all(); cur_sn != NULL; cur_sn = cur_sn->next) {
      acct = (GaimAccount *)cur_sn->data;
      name = acct->username;
      priv_key = GE_find_key_by_name(GE_my_priv_ring, name, acct);
      pub_key = GE_find_key_by_name(GE_my_pub_ring, name, acct);
            
      if (priv_key == NULL) { /* No key for this username.  Make one.  */
         proto_node = crypt_proto_list;
         /* make a pair using the first protocol that comes to mind. */
         /* user can override using the config tool */
         GE_make_private_pair((crypt_proto *)proto_node->data, name, (GaimAccount*)(cur_sn->data), 1024);
      } else {  /* There is a private key  */
         if (pub_key == NULL) { /* but no public key */
            gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "No public key found for %s\n", name);
            gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "  Making one from private key and saving...\n");
            pub_key = priv_key->proto->make_pub_from_priv(priv_key);
            new_key = g_malloc(sizeof(key_ring_data));
            new_key->key = pub_key;
            new_key->account = acct;
            strncpy(new_key->name, name, sizeof(new_key->name));
            GE_my_pub_ring = g_slist_append(GE_my_pub_ring, new_key);
            GE_add_key_to_file(Public_key_file, new_key);
         }
      }
   }
}

void GE_make_private_pair(crypt_proto* proto, const char* name, GaimAccount* acct, int keylength) {
   crypt_key *pub_key, *priv_key;
   key_ring_data *new_key;

   proto->gen_key_pair(&pub_key, &priv_key, name, keylength);

   new_key = g_malloc(sizeof(key_ring_data));
   new_key->key = pub_key;
   new_key->account = acct;
   strncpy(new_key->name, name, sizeof(new_key->name));
   GE_my_pub_ring = GE_add_key_to_ring(GE_my_pub_ring, new_key);

   GE_del_key_from_file(Public_key_file, name, acct);
   GE_add_key_to_file(Public_key_file, new_key);

   new_key = g_malloc(sizeof(key_ring_data));
   new_key->key = priv_key;
   new_key->account = acct;
   strncpy(new_key->name, name,
           sizeof(new_key->name));
   GE_my_priv_ring = GE_add_key_to_ring(GE_my_priv_ring, new_key);

   GE_del_key_from_file(Private_key_file, name, acct);
   GE_add_key_to_file(Private_key_file, new_key);
}

