/* GConf
 * Copyright (C) 1999, 2000, 2002 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <gconf/gconf-backend.h>
#include <gconf/gconf-internals.h>
#include <gconf/gconf.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

#include "markup-tree.h"

/*
 * Overview
 * 
 * Basically we have a directory tree underneath an arbitrary root
 * directory.  The directory tree reflects the configuration
 * namespace. Each directory contains an XML-like file which contains
 * metadata for the directory and the key-value pairs in that
 * directory.  The magic file in each directory is called %gconf.xml,
 * and can't clash with the database namespace because names containing
 * % aren't allowed.  So:
 *
 * /
 *  %gconf.xml
 *   guppi/
 *     %gconf.xml
 *   gnumeric/
 *     %gconf.xml
 *
 */

typedef struct
{
  GConfSource source; /* inherit from GConfSource */
  char *root_dir;
  guint timeout_id;
  GConfLock* lock;
  MarkupTree *tree;
  guint dir_mode;
  guint file_mode;
} MarkupSource;

static MarkupSource* ms_new     (const char   *root_dir,
                                 guint         dir_mode,
                                 guint         file_mode,
                                 GConfLock    *lock,
                                 gboolean      read_only);
static void          ms_destroy (MarkupSource *source);

/*
 * VTable functions
 */

 /* shutdown() is a BSD libc function */
static void           x_shutdown      (GError           **err);
static GConfSource*   resolve_address (const char        *address,
                                       GError           **err);
static void           lock            (GConfSource       *source,
                                       GError           **err);
static void           unlock          (GConfSource       *source,
                                       GError           **err);
static gboolean       readable        (GConfSource       *source,
                                       const char        *key,
                                       GError           **err);
static gboolean       writable        (GConfSource       *source,
                                       const char        *key,
                                       GError           **err);
static GConfValue*    query_value     (GConfSource       *source,
                                       const char        *key,
                                       const char       **locales,
                                       char             **schema_name,
                                       GError           **err);
static GConfMetaInfo* query_metainfo  (GConfSource       *source,
                                       const char        *key,
                                       GError           **err);
static void           set_value       (GConfSource       *source,
                                       const char        *key,
                                       const GConfValue  *value,
                                       GError           **err);
static GSList*        all_entries     (GConfSource       *source,
                                       const char        *dir,
                                       const char       **locales,
                                       GError           **err);
static GSList*        all_subdirs     (GConfSource       *source,
                                       const char        *dir,
                                       GError           **err);
static void           unset_value     (GConfSource       *source,
                                       const char        *key,
                                       const char        *locale,
                                       GError           **err);
static gboolean       dir_exists      (GConfSource       *source,
                                       const char        *dir,
                                       GError           **err);
static void           remove_dir      (GConfSource       *source,
                                       const char        *dir,
                                       GError           **err);
static void           set_schema      (GConfSource       *source,
                                       const char        *key,
                                       const char        *schema_key,
                                       GError           **err);
static gboolean       sync_all        (GConfSource       *source,
                                       GError           **err);
static void           destroy_source  (GConfSource       *source);
static void           clear_cache     (GConfSource       *source);



static GConfBackendVTable markup_vtable = {
  x_shutdown,
  resolve_address,
  lock,
  unlock,
  readable,
  writable,
  query_value,
  query_metainfo,
  set_value,
  all_entries,
  all_subdirs,
  unset_value,
  dir_exists,
  remove_dir,
  set_schema,
  sync_all,
  destroy_source,
  clear_cache
};

static void          
x_shutdown (GError **err)
{
  gconf_log (GCL_DEBUG, _("Unloading text markup backend module."));
}

static void
lock (GConfSource *source,
      GError **err)
{
  

}

static void
unlock (GConfSource *source,
        GError **err)
{


}

static gboolean
readable (GConfSource *source,
          const char  *key,
          GError     **err)
{

  return TRUE;
}

static gboolean
writable (GConfSource  *source,
          const char   *key,
          GError      **err)
{

  return TRUE;
}

guint
_gconf_mode_t_to_mode(mode_t orig)
{
  /* I don't think this is portable. */
  guint mode = 0;
  guint fullmask = S_IRWXG | S_IRWXU | S_IRWXO;  

  mode = orig & fullmask;
  
  g_return_val_if_fail (mode <= 0777, 0700);

  return mode;
}

static GConfSource*  
resolve_address (const char *address,
                 GError    **err)
{
  char* root_dir;
  MarkupSource* xsource;
  GConfSource *source;
  guint len;
  gint flags = 0;
  GConfLock* lock = NULL;
  guint dir_mode = 0700;
  guint file_mode = 0600;
  char** address_flags;
  char** iter;
  gboolean force_readonly;
  
  root_dir = gconf_address_resource (address);

  if (root_dir == NULL)
    {
      gconf_set_error (err, GCONF_ERROR_BAD_ADDRESS, _("Couldn't find the root directory in the address \"%s\""), address);
      return NULL;
    }

  /* Chop trailing '/' to canonicalize */
  len = strlen (root_dir);

  if (root_dir[len-1] == '/')
    root_dir[len-1] = '\0';

  if (mkdir (root_dir, dir_mode) < 0)
    {
      if (errno != EEXIST)
        {
          gconf_set_error (err, GCONF_ERROR_FAILED,
                           _("Could not make directory `%s': %s"),
                           root_dir, g_strerror (errno));
          g_free (root_dir);
          return NULL;
        }
      else
        {
          /* Already exists, base our dir_mode on it */
          struct stat statbuf;
          if (stat (root_dir, &statbuf) == 0)
            {
              dir_mode = _gconf_mode_t_to_mode (statbuf.st_mode);
              /* dir_mode without search bits */
              file_mode = dir_mode & (~0111);
            }
        }
    }

  force_readonly = FALSE;
  
  address_flags = gconf_address_flags (address);  
  if (address_flags)
    {
      iter = address_flags;
      while (*iter)
        {
          if (strcmp (*iter, "readonly") == 0)
            {
              force_readonly = TRUE;
              break;
            }

          ++iter;
        }
    }

  g_strfreev (address_flags);

  {
    /* See if we're writable */
    gboolean writable;
    int fd;
    char* testfile;

    writable = FALSE;
    
    if (!force_readonly)
      {
        testfile = g_strconcat (root_dir, "/.testing.writeability", NULL);    
        
        fd = open (testfile, O_CREAT|O_WRONLY, S_IRWXU);
        
        if (fd >= 0)
          {
            writable = TRUE;
            close (fd);
          }
        
        unlink (testfile);
        
        g_free (testfile);
      }
    
    if (writable)
      flags |= GCONF_SOURCE_ALL_WRITEABLE;


    /* FIXME locking code is out of sync with XML backend
     * (doesn't support the GCONF_LOCAL_LOCKS stuff)
     */
    
    /* We only do locking if it's writable,
     * which is sort of broken but close enough
     */
    if (writable)
      {
        char* lockdir;

        /* use same lockfile name as XML backend, for safety */
        lockdir = gconf_concat_dir_and_key (root_dir, "%gconf-xml-backend.lock");
        
        lock = gconf_get_lock (lockdir, err);

        if (lock != NULL)
          gconf_log (GCL_DEBUG, "Acquired lock directory `%s'", lockdir);
        
        g_free (lockdir);
        
        if (lock == NULL)
          {
            g_free (root_dir);
            return NULL;
          }
      }
  }

  {
    /* see if we're readable */
    gboolean readable = FALSE;
    DIR* d;

    d = opendir (root_dir);

    if (d != NULL)
      {
        readable = TRUE;
        closedir (d);
      }
    
    if (readable)
      flags |= GCONF_SOURCE_ALL_READABLE;
  }

  if (!(flags & GCONF_SOURCE_ALL_READABLE) &&
      !(flags & GCONF_SOURCE_ALL_WRITEABLE))
    {
      gconf_set_error (err, GCONF_ERROR_BAD_ADDRESS,
                       _("Can't read from or write to the XML root directory in the address \"%s\""),
                       address);
      g_free (root_dir);
      return NULL;
    }
  
  /* Create the new source */

  xsource = ms_new (root_dir, dir_mode, file_mode, lock,
                    (flags & GCONF_SOURCE_ALL_WRITEABLE) == 0);

  gconf_log (GCL_DEBUG,
             _("Directory/file permissions for XML source at root %s are: %o/%o"),
             root_dir, dir_mode, file_mode);
  
  source = (GConfSource*)xsource;

  source->flags = flags;
  
  g_free (root_dir);
  
  return source;
}

static MarkupEntry*
tree_lookup_entry (MarkupTree *tree,
                   const char *key,
                   gboolean    create_if_not_found,
                   GError    **err)
{
  char* parent;
  MarkupDir *dir;
  GError* error = NULL;

  parent = gconf_key_directory (key);
  
  g_assert (parent != NULL);

  if (create_if_not_found)
    dir = markup_tree_ensure_dir (tree, parent, &error);
  else
    dir = markup_tree_lookup_dir (tree, parent, &error);

  if (error != NULL)
    {
      g_propagate_error (err, error);
      return NULL;
    }
  
  g_free (parent);
  parent = NULL;
  
  if (dir != NULL)
    {
      const char *relative_key;
      MarkupEntry *entry;
      
      relative_key = gconf_key_key (key);

      error = NULL;
      if (create_if_not_found)
        entry = markup_dir_ensure_entry (dir, relative_key, &error);
      else
        entry = markup_dir_lookup_entry (dir, relative_key, &error);
      if (error != NULL)
        {
          g_propagate_error (err, error);
          g_return_val_if_fail (entry == NULL, NULL);
          return NULL;
        }
      
      return entry;
    }
  else
    return NULL;
}

static GConfValue* 
query_value (GConfSource *source,
             const char  *key,
             const char **locales,
             char       **schema_name,
             GError     **err)
{
  MarkupSource* ms = (MarkupSource*)source;
  GError* error = NULL;
  MarkupEntry *entry;
  GConfValue *retval;

  retval = NULL;
  
  error = NULL;
  entry = tree_lookup_entry (ms->tree, key, FALSE, &error);
  if (error != NULL)
    {
      g_propagate_error (err, error);
      return NULL;
    }
      
  if (entry != NULL)
    {
      retval = markup_entry_get_value (entry, locales);
      if (schema_name)
        *schema_name = g_strdup (markup_entry_get_schema_name (entry));
    }
  else
    {
      retval = NULL;
      if (schema_name)
        *schema_name = NULL;
    }
  
  return retval;
}

static GConfMetaInfo*
query_metainfo (GConfSource *source,
                const char  *key,
                GError     **err)
{
  MarkupSource* ms = (MarkupSource*)source;
  GError* error = NULL;
  MarkupEntry *entry;

  error = NULL;
  entry = tree_lookup_entry (ms->tree, key, FALSE, &error);
  if (error != NULL)
    {
      g_propagate_error (err, error);
      return NULL;
    }
      
  if (entry != NULL)
    {
      GConfMetaInfo* gcmi;
      const char *schema_name;
      GTime mtime;
      const char *mod_user;
      
      gcmi = gconf_meta_info_new ();

      schema_name = markup_entry_get_schema_name (entry);
      mtime = markup_entry_get_mod_time (entry);
      mod_user = markup_entry_get_mod_user (entry);
      
      if (schema_name)
        gconf_meta_info_set_schema (gcmi, schema_name);
      
      gconf_meta_info_set_mod_time (gcmi, mtime);
      
      if (mod_user)
        gconf_meta_info_set_mod_user (gcmi, mod_user);

      return gcmi;
    }
  else
    {
      return NULL;
    }
}

static void          
set_value (GConfSource      *source,
           const char       *key,
           const GConfValue *value,
           GError          **err)
{
  MarkupSource* ms = (MarkupSource*)source;
  MarkupEntry *entry;
  GError *tmp_err;
  
  g_return_if_fail (value != NULL);
  g_return_if_fail (source != NULL);

  tmp_err = NULL;
  entry = tree_lookup_entry (ms->tree,
                             key, TRUE, &tmp_err);
  if (tmp_err != NULL)
    {
      g_propagate_error (err, tmp_err);
      return;
    }

  g_return_if_fail (entry != NULL);

  markup_entry_set_value (entry, value);
}

static GConfEntry*
gconf_entry_from_markup_entry (MarkupEntry *entry,
                               const char **locales)
{
  GConfValue *value;
  const char *schema_name;
  GConfEntry *gconf_entry;

  value = markup_entry_get_value (entry, locales);
  schema_name = markup_entry_get_schema_name (entry);

  /* Entries here are created with relative names, not the usual
   * full paths, for efficiency. Kind of lame though.
   */
  gconf_entry = gconf_entry_new_nocopy (g_strdup (markup_entry_get_name (entry)),
                                        value);
  gconf_entry_set_schema_name (gconf_entry, schema_name);

  return gconf_entry;
}

static GSList*             
all_entries (GConfSource *source,
             const char  *key,
             const char **locales,
             GError     **err)
{
  MarkupSource *ms = (MarkupSource*)source;
  MarkupDir *dir;
  GError* error;
  GSList *retval;
  GSList *tmp;
  
  retval = NULL;

  error = NULL;
  dir = markup_tree_lookup_dir (ms->tree, key, &error);
  if (error != NULL)
    {
      g_propagate_error (err, error);
      return NULL;
    }
  
  if (dir == NULL)
    return NULL;

  error = NULL;
  tmp = markup_dir_list_entries (dir, &error);
  if (error != NULL)
    {
      g_propagate_error (err, error);
      return NULL;
    }

  while (tmp != NULL)
    {
      retval = g_slist_prepend (retval,
                                gconf_entry_from_markup_entry (tmp->data,
                                                               locales));
      
      tmp = tmp->next;
    }

  return retval;
}

static GSList*
all_subdirs (GConfSource *source,
             const char  *key,
             GError     **err)
{
  MarkupSource *ms = (MarkupSource*)source;
  MarkupDir *dir;
  GError* error;
  GSList *retval;
  GSList *tmp;
  
  retval = NULL;

  error = NULL;
  dir = markup_tree_lookup_dir (ms->tree, key, &error);
  if (error != NULL)
    {
      g_propagate_error (err, error);
      return NULL;
    }
  
  if (dir == NULL)
    return NULL;

  error = NULL;
  tmp = markup_dir_list_subdirs (dir, &error);
  if (error != NULL)
    {
      g_propagate_error (err, error);
      return NULL;
    }

  while (tmp != NULL)
    {
      retval = g_slist_prepend (retval,
                                g_strdup (markup_dir_get_name (tmp->data)));
      
      tmp = tmp->next;
    }

  return retval;
}

static void          
unset_value (GConfSource *source,
             const char  *key,
             const char  *locale,
             GError     **err)
{
  MarkupSource* ms = (MarkupSource*)source;
  MarkupEntry *entry;
  GError *tmp_err;
  
  g_return_if_fail (key != NULL);
  g_return_if_fail (source != NULL);

  tmp_err = NULL;
  entry = tree_lookup_entry (ms->tree,
                             key, TRUE, &tmp_err);
  if (tmp_err != NULL)
    {
      g_propagate_error (err, tmp_err);
      return;
    }

  g_return_if_fail (entry != NULL);

  markup_entry_unset_value (entry, locale);
}

static gboolean
dir_exists (GConfSource *source,
            const char  *key,
            GError     **err)
{
  MarkupSource *ms = (MarkupSource*)source;
  MarkupDir *dir;
  GError* error;

  error = NULL;
  dir = markup_tree_lookup_dir (ms->tree, key, &error);
  if (error != NULL)
    {
      g_propagate_error (err, error);
      return FALSE;
    }

  return dir != NULL;
}  

static void          
remove_dir (GConfSource *source,
            const char  *key,
            GError     **err)
{
  g_set_error (err, GCONF_ERROR,
               GCONF_ERROR_FAILED,
               _("Remove dir operation is no longer supported, just remove all the values in the directory"));
}

static void          
set_schema (GConfSource *source,
            const char  *key,
            const char  *schema_name,
            GError     **err)
{
  MarkupSource* ms = (MarkupSource*)source;
  MarkupEntry *entry;
  GError *tmp_err;
  
  g_return_if_fail (key != NULL);
  g_return_if_fail (source != NULL);
  /* schema_name can be NULL to unset */
  
  tmp_err = NULL;
  entry = tree_lookup_entry (ms->tree,
                             key, TRUE, &tmp_err);
  if (tmp_err != NULL)
    {
      g_propagate_error (err, tmp_err);
      return;
    }

  g_return_if_fail (entry != NULL);

  markup_entry_set_schema_name (entry, schema_name);
}

static gboolean      
sync_all (GConfSource *source,
          GError     **err)
{
  MarkupSource* ms = (MarkupSource*)source;

  return markup_tree_sync (ms->tree, err);
}

static void          
destroy_source (GConfSource *source)
{
  ms_destroy ((MarkupSource*)source);
}

static void
clear_cache (GConfSource *source)
{
  MarkupSource* ms = (MarkupSource*)source;

  /* To blow the entire cache we just rebuild the tree */
  if (!markup_tree_sync (ms->tree, NULL))
    {
      /* not translated since cache clearing is debug-only */
      gconf_log (GCL_WARNING, "Could not sync data in order to drop cache");
      return;
    }
  
  markup_tree_free (ms->tree);
  
  ms->tree = markup_tree_new (ms->root_dir,
                              ms->dir_mode, ms->file_mode,
                              (ms->source.flags & GCONF_SOURCE_ALL_WRITEABLE) == 0);
}

/* Initializer */

G_MODULE_EXPORT const char*
g_module_check_init (GModule *module)
{
  gconf_log (GCL_DEBUG, _("Initializing Markup backend module"));

  return NULL;
}

G_MODULE_EXPORT GConfBackendVTable* 
gconf_backend_get_vtable (void)
{
  return &markup_vtable;
}

/* ****************************************************/

/*
 *  MarkupSource
 */ 

/* This timeout periodically unloads
 * data that hasn't been used in a while.
 */
static gboolean
cleanup_timeout (void  *data)
{
  MarkupSource* ms = (MarkupSource*)data;

#if 0
  cache_clean(xs->cache, 60*5 /* 5 minutes */);
#endif
  
  return TRUE;
}

static MarkupSource*
ms_new (const char* root_dir,
        guint       dir_mode,
        guint       file_mode,
        GConfLock  *lock,
        gboolean    read_only)
{
  MarkupSource* ms;

  g_return_val_if_fail(root_dir != NULL, NULL);

  ms = g_new0(MarkupSource, 1);

  ms->timeout_id = g_timeout_add (1000*60*5, /* 1 sec * 60 s/min * 5 min */
                                  cleanup_timeout,
                                  ms);

  ms->root_dir = g_strdup (root_dir);
  
  ms->lock = lock;

  ms->dir_mode = dir_mode;
  ms->file_mode = file_mode;
  
  ms->tree = markup_tree_new (ms->root_dir,
                              ms->dir_mode, ms->file_mode,
                              read_only);
  
  return ms;
}

static void
ms_destroy (MarkupSource* ms)
{
  GError* error = NULL;
  
  g_return_if_fail (ms != NULL);

  /* do this first in case we're in a "fast cleanup just before exit"
   * situation
   */
  if (ms->lock != NULL && !gconf_release_lock (ms->lock, &error))
    {
      gconf_log (GCL_ERR, _("Failed to give up lock on XML dir \"%s\": %s"),
                 ms->root_dir, error->message);
      g_error_free(error);
      error = NULL;
    }
  
  if (!g_source_remove (ms->timeout_id))
    {
      /* should not happen, don't translate */
      gconf_log (GCL_ERR, "timeout not found to remove?");
    }

  markup_tree_free (ms->tree);

  g_free (ms->root_dir);
  g_free (ms);
}
