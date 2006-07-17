/*  Gaim-Encryption Legacy Preferences file interface                     */
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

#include "prefs.h"
#include "util.h"
#include "prefs.h"

#ifdef _WIN32
#include "win32dep.h"
#endif

static gboolean Prefs_accept_key_unknown = FALSE;
static gboolean Prefs_accept_key_conflict = FALSE;
/*static gboolean Prefs_encrypt_response = TRUE; */
static gboolean Prefs_broadcast_notify = FALSE;
static gboolean Prefs_encrypt_if_notified = TRUE;

const static char key_file[] = "encrypt.prefs";

static gboolean parse_key_val(char* val, gboolean def) {
   if (strcmp(val, "TRUE") == 0) {
      return TRUE;
   }
   if (strcmp(val, "FALSE") == 0) {
      return FALSE;
   }
   return def;
}

void GE_convert_legacy_prefs() {
   char key[51], value[51];
   
   char* filename = g_build_filename(gaim_user_dir(), key_file, NULL);

   FILE* fp = g_fopen(filename, "r");

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Checking for old prefs file (%s)...\n", filename);

   if (fp == NULL) return;
   
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Converting...\n");

   while (fscanf(fp, "%50s%50s", key, value) != EOF) {
      if (strcmp(key, "AcceptUnknown") == 0) {
         gaim_prefs_set_bool("/plugins/gtk/encrypt/accept_unknown_key",
                             parse_key_val(value, Prefs_accept_key_unknown));
      } else if (strcmp(key, "AcceptDuplicate") == 0) {
         gaim_prefs_set_bool("/plugins/gtk/encrypt/accept_conflicting_key",
                             parse_key_val(value, Prefs_accept_key_conflict));
      } else if (strcmp(key, "BroadcastNotify") == 0) {
         gaim_prefs_set_bool("/plugins/gtk/encrypt/broadcast_notify",
                             parse_key_val(value, Prefs_broadcast_notify));
      } else if (strcmp(key, "EncryptIfNotified") == 0) {
         gaim_prefs_set_bool("/plugins/gtk/encrypt/encrypt_if_notified",
                             parse_key_val(value, Prefs_encrypt_if_notified));
      } else {
         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Bad Preference Key %s\n", value);
      }
   }
   fclose(fp);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Deleting old prefs\n");
   unlink(filename);

   g_free(filename);
}
