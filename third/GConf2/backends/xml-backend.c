
/* GConf
 * Copyright (C) 1999, 2000 Red Hat Inc.
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

#include "xml-cache.h"


#include <libxml/tree.h>
#include <libxml/parser.h>

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

/*
 * Overview
 * 
 * Basically we have a directory tree underneath an arbitrary root
 * directory.  The directory tree reflects the configuration
 * namespace. Each directory contains an XML file which contains
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
 *
 * Locking
 * 
 * Locking doesn't _really_ matter because there's only one instance
 * of the daemon at a time. However, eventually we want a non-daemon
 * command line tool and library, e.g. for the debconf stuff, 
 * so we will eventually have locking. I'll figure out then how
 * it will work.
 *
 * Caching
 *
 * I haven't decided the best way to do caching yet. As a first cut;
 * we'll cache the parse tree for any files we've looked at. The cache
 * will contain time stamps; we'll nuke cache entries that haven't been
 * used in a while, either in a main loop timeout or by checking whenever 
 * we add a new cache entry. Parse trees correspond to "directories" in the
 * configuration namespace.
 *
 * A more precise cache will store specific key-value pairs; this cache
 * will probably contain a pointer to the parse tree node the key-value
 * pair is inside.
 *
 * We'll of course need a "dirty" list of stuff not yet written to disk.
 *
 * We'll save the mod time of parse trees when we load them, so we can 
 * paranoia check that no one has change the file before we save.
 *
 * Ideally we could monitor our own process size and also free up
 * cache whenever we started to use massive RAM. However, not sure
 * this can be done at all portably. Could possibly have some measure
 * of parse tree size.
 *
 * The libxml parse trees are pretty huge, so in theory we could
 * "compress" them by extracting all the information we want into a
 * specialized data structure, then nuking the parse tree. However,
 * that would add more CPU overhead at load and save time. Anyway, as
 * a first cut I'm not going to do this, we might do it later.
 *
 * Atomic Saving
 *
 * We'll want to save atomically by creating a temporary file for the
 * new file version, renaming the original file, moving the temporary
 * file into place, then deleting the original file, checking for
 * errors and mod times along the way.
 *      
 * Failed lookup caching
 *
 * If a key/directory doesn't exist, we create a cache entry anyway
 * so we can rapidly re-determine that it doesn't exist.
 * We also need to save "dirty" nonexistent entries, so we can delete
 * the stuff off disk.  
 * 
 */

typedef struct _XMLSource XMLSource;

/* XMLSource **/

struct _XMLSource {
  GConfSource source; /* inherit from GConfSource */
  Cache* cache;
  gchar* root_dir;
  guint timeout_id;
  int lock_fd;
  guint dir_mode;
  guint file_mode;
};

static XMLSource* xs_new       (const gchar* root_dir,
                                guint dir_mode,
                                guint file_mode,
                                int lock_fd);
static void       xs_destroy   (XMLSource* source);

/*
 * VTable functions
 */

 /* shutdown() is a BSD libc function */
static void          x_shutdown        (GError** err);

static GConfSource*  resolve_address (const gchar* address,
                                      GError** err);

static void          lock            (GConfSource* source,
                                      GError** err);

static void          unlock          (GConfSource* source,
                                      GError** err);

static gboolean     readable         (GConfSource* source,
                                      const gchar* key,
                                      GError** err);

static gboolean     writable        (GConfSource* source,
                                      const gchar* key,
                                      GError** err);

static GConfValue*   query_value     (GConfSource* source,
                                      const gchar* key,
                                      const gchar** locales,
                                      gchar** schema_name,
                                      GError** err);

static GConfMetaInfo*query_metainfo  (GConfSource* source,
                                      const gchar* key,
                                      GError** err);

static void          set_value       (GConfSource* source,
                                      const gchar* key,
                                      const GConfValue* value,
                                      GError** err);

static GSList*       all_entries    (GConfSource* source,
                                     const gchar* dir,
                                     const gchar** locales,
                                     GError** err);

static GSList*       all_subdirs     (GConfSource* source,
                                      const gchar* dir,
                                      GError** err);

static void          unset_value     (GConfSource* source,
                                      const gchar* key,
                                      const gchar* locale,
                                      GError** err);

static gboolean      dir_exists      (GConfSource *source,
                                      const gchar *dir,
                                      GError** err);

static void          remove_dir      (GConfSource* source,
                                      const gchar* dir,
                                      GError** err);

static void          set_schema      (GConfSource* source,
                                      const gchar* key,
                                      const gchar* schema_key,
                                      GError** err);

static gboolean      sync_all        (GConfSource* source,
                                      GError** err);

static void          destroy_source  (GConfSource* source);

static void          clear_cache     (GConfSource* source);

static GConfBackendVTable xml_vtable = {
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
x_shutdown (GError** err)
{
  gconf_log(GCL_DEBUG, _("Unloading XML backend module."));
}

static void
lock (GConfSource* source,
      GError** err)
{
  XMLSource *xs;
  int timeout = 5, status, i;
  struct flock fl;

  xs = (XMLSource *) source;

  if (xs->lock_fd == -1)
    return;

  gconf_log(GCL_DEBUG, _("Locking XML backend"));

  /* Athena local mod: acquire a write lock.  Note that we use
   * a retry/sleep loop, rather than waiting for the lock with
   * a timeout, due to problems using F_SETLKW with alarm() on
   * a file in AFS.
   */
  for (i = 0; ; i++)
    {
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      fl.l_start = 0;
      fl.l_len = 0;
      status = fcntl(xs->lock_fd, F_SETLK, &fl);
      if (status != -1 || i >= timeout)
	break;
      sleep(1);
    }
  if (status == -1)
    gconf_set_error(err, GCONF_ERROR_FAILED,
		    _("Cannot lock the XML backend: %s"),
		    strerror(errno));
  return;
}

static void
unlock (GConfSource* source,
        GError** err)
{
  XMLSource *xs;
  struct flock fl;

  xs = (XMLSource *) source;

  if (xs->lock_fd == -1)
    return;
  
  gconf_log(GCL_DEBUG, _("Unlocking XML backend"));

  /* Athena local mod: release our acquired lock. */
  fl.l_type = F_UNLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  if (fcntl(xs->lock_fd, F_SETLK, &fl) == -1)
    gconf_log(GCL_WARNING, _("Cannot unlock XML backend: %s"),
	      strerror(errno));

  return;
}

static gboolean
readable (GConfSource* source,
          const gchar* key,
          GError** err)
{

  return TRUE;
}

static gboolean
writable (GConfSource* source,
           const gchar* key,
           GError** err)
{

  return TRUE;
}

static GConfSource*  
resolve_address (const gchar* address, GError** err)
{
  gchar* root_dir;
  XMLSource* xsource;
  GConfSource* source;
  guint len;
  gint flags = 0;
  int lock_fd = -1;
  guint dir_mode = 0700;
  guint file_mode = 0600;
  gchar** address_flags;
  gchar** iter;
  gboolean force_readonly;
  
  root_dir = gconf_address_resource(address);

  if (root_dir == NULL)
    {
      gconf_set_error (err, GCONF_ERROR_BAD_ADDRESS, _("Couldn't find the root directory in the address \"%s\""), address);
      return NULL;
    }

  /* Chop trailing '/' to canonicalize */
  len = strlen(root_dir);

  if (root_dir[len-1] == '/')
    root_dir[len-1] = '\0';

  if (gconf_mkdir_private(root_dir) < 0)
    {
      if (errno != EEXIST)
        {
          gconf_set_error(err, GCONF_ERROR_FAILED,
                          _("Could not make directory `%s': %s"),
                          (gchar*)root_dir, strerror(errno));
          g_free(root_dir);
          return NULL;
        }
      else
        {
          /* Already exists, base our dir_mode on it */
          struct stat statbuf;
          if (stat(root_dir, &statbuf) == 0)
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
    gchar* testfile;

    writable = FALSE;
    
    if (!force_readonly)
      {
        testfile = g_strconcat(root_dir, "/.testing.writeability", NULL);    
        
        fd = open(testfile, O_CREAT|O_WRONLY, S_IRWXU);
        
        if (fd >= 0)
          {
            writable = TRUE;
            close(fd);
          }
        
        unlink(testfile);
        
        g_free(testfile);
      }
    
    if (writable)
      flags |= GCONF_SOURCE_ALL_WRITEABLE;

    /* We only do locking if it's writable,
       which is sort of broken but close enough
    */
    if (writable)
      {
        gchar* lockdir;
        gchar* lockfile;

        lockdir = gconf_concat_dir_and_key(root_dir, "%gconf-xml-backend.lock");
	/* Athena local mod: open a file we'll lock when syncing the
	 * database, but leave the backend unlocked so that multiple
	 * gconfd instances can access it.
	 */
	if (gconf_mkdir_private(lockdir) < 0)
	  {
	    if (errno != EEXIST)
	      {
		gconf_set_error(err, GCONF_ERROR_FAILED,
				_("Could not make directory `%s': %s"),
				lockdir, strerror(errno));
		g_free(root_dir);
		return NULL;
	      }
	  }
        lockfile = g_strconcat(lockdir, "/lockfile", NULL);
        g_free(lockdir);
        lock_fd = open(lockfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (lock_fd == -1)
	  {
	    gconf_set_error(err, GCONF_ERROR_FAILED,
			    _("Could not open lock file `%s': %s"),
			    lockfile, strerror(errno));
	    g_free(root_dir);
	    g_free(lockfile);
	    return NULL;
	  }
	g_free(lockfile);
      }
  }

  {
    /* see if we're readable */
    gboolean readable = FALSE;
    DIR* d;

    d = opendir(root_dir);

    if (d != NULL)
      {
        readable = TRUE;
        closedir(d);
      }
    
    if (readable)
      flags |= GCONF_SOURCE_ALL_READABLE;
  }

  if (!(flags & GCONF_SOURCE_ALL_READABLE) &&
      !(flags & GCONF_SOURCE_ALL_WRITEABLE))
    {
      gconf_set_error(err, GCONF_ERROR_BAD_ADDRESS, _("Can't read from or write to the XML root directory in the address \"%s\""), address);
      g_free(root_dir);
      return NULL;
    }  
  
  /* Create the new source */

  xsource = xs_new(root_dir, dir_mode, file_mode, lock_fd);

  gconf_log(GCL_DEBUG,
            _("Directory/file permissions for XML source at root %s are: %o/%o"),
            root_dir, dir_mode, file_mode);
  
  source = (GConfSource*)xsource;

  source->flags = flags;
  
  g_free(root_dir);
  
  return source;
}

static GConfValue* 
query_value (GConfSource* source,
             const gchar* key,
             const gchar** locales,
             gchar** schema_name,
             GError** err)
{
  XMLSource* xs = (XMLSource*)source;
  gchar* parent;
  Dir* dir;
  GError* error = NULL;

  parent = gconf_key_directory(key);
  
  g_assert(parent != NULL);
  
  dir = cache_lookup(xs->cache, parent, FALSE, &error);

  /* We DO NOT want to return an error unless it represents a general
     problem with the backend; since we don't want to report stuff
     like "this key doesn't exist yet" - however this is a maintenance
     problem, since some errors may be added that need reporting. */
  if (error != NULL)
    {
      gconf_log(GCL_WARNING, "%s", error->message);
      g_error_free(error);
      error = NULL;
    }
  
  g_free(parent);
  parent = NULL;
  
  if (dir != NULL)
    {
      const gchar* relative_key;
      GConfValue* retval;
      
      relative_key = gconf_key_key(key);

      retval = dir_get_value(dir, relative_key, locales, schema_name, &error);

      /* perhaps we should be reporting this error... */
      if (error != NULL)
        {
          gconf_log(GCL_WARNING, "%s", error->message);
          g_error_free(error);
          error = NULL;
        }
      
      return retval;
    }
  else
    return NULL;
}

static GConfMetaInfo*
query_metainfo  (GConfSource* source, const gchar* key,
                 GError** err)
{
  XMLSource* xs = (XMLSource*)source;
  gchar* parent;
  Dir* dir;

  parent = gconf_key_directory(key);

  if (parent != NULL)
    {
      dir = cache_lookup(xs->cache, parent, FALSE, err);
      g_free(parent);
      parent = NULL;
      
      if (dir != NULL)
        {
          const gchar* relative_key;
      
          relative_key = gconf_key_key (key);

          return dir_get_metainfo (dir, relative_key, err);
        }
    }

  /* No metainfo found */
  return NULL;
}

static void          
set_value (GConfSource* source, const gchar* key, const GConfValue* value,
           GError** err)
{
  XMLSource* xs = (XMLSource*)source;
  Dir* dir;
  gchar* parent;
  
  g_return_if_fail(value != NULL);
  g_return_if_fail(source != NULL);
  
  parent = gconf_key_directory(key);
  
  g_assert(parent != NULL);
  
  dir = cache_lookup(xs->cache, parent, TRUE, err);
  
  g_free(parent);
  parent = NULL;

  if (dir == NULL)
    {
      g_return_if_fail((err == NULL || *err != NULL));
      return;
    }
  else
    {
      const gchar* relative_key;
      
      relative_key = gconf_key_key(key);
      
      dir_set_value(dir, relative_key, value, err);
    }
}


static GSList*             
all_entries    (GConfSource* source,
                const gchar* key,
                const gchar** locales,
                GError** err)
{
  XMLSource* xs = (XMLSource*)source;
  Dir* dir;

  dir = cache_lookup(xs->cache, key, FALSE, err);
  
  if (dir == NULL)
    return NULL;
  else
    return dir_all_entries(dir, locales, err);
}

static GSList*
all_subdirs     (GConfSource* source,
                 const gchar* key,
                 GError** err)
{
  Dir* dir;
  XMLSource* xs = (XMLSource*)source;
  GError *sync_err;
  
  /* We have to sync before we can do this, to see which
   * subdirs have gone away.
   */
  sync_err = NULL;
  cache_sync (xs->cache, &sync_err);
  if (sync_err)
    {
      gconf_log (GCL_WARNING, _("Error syncing the XML backend directory cache: %s"),
                 sync_err->message);
      g_error_free (sync_err);
      sync_err = NULL;
      /* continue, may as well try our best. */
    }
  
  dir = cache_lookup (xs->cache, key, FALSE, err);
  
  if (dir == NULL)
    return NULL;
  else
    return dir_all_subdirs (dir, err);
}

static void          
unset_value     (GConfSource* source,
                 const gchar* key,
                 const gchar* locale,
                 GError** err)
{
  XMLSource* xs = (XMLSource*)source;
  Dir* dir;
  gchar* parent;

  gconf_log(GCL_DEBUG, "XML backend: unset value `%s'", key);
  
  parent = gconf_key_directory(key);
  
  dir = cache_lookup(xs->cache, parent, FALSE, err);

  g_free(parent);
  
  if (dir == NULL)
    return;
  else
    {
      const gchar* relative_key;
  
      relative_key = gconf_key_key(key);

      dir_unset_value(dir, relative_key, locale, err);
    }
}

static gboolean
dir_exists      (GConfSource*source,
                 const gchar* key,
                 GError** err)
{
  XMLSource *xs = (XMLSource*)source;
  Dir* dir;
  
  dir = cache_lookup(xs->cache, key, FALSE, err);
  
  return (dir != NULL);
}  

static void          
remove_dir      (GConfSource* source,
                 const gchar* key,
                 GError** err)
{
  g_set_error (err, GCONF_ERROR,
               GCONF_ERROR_FAILED,
               _("Remove dir operation is no longer supported, just remove all the values in the directory"));
}

static void          
set_schema (GConfSource *source,
            const gchar *key,
            const gchar *schema_key,
            GError     **err)
{
  XMLSource* xs = (XMLSource*)source;

  Dir* dir;
  gchar* parent;

  g_return_if_fail (source != NULL);
  g_return_if_fail (key != NULL);

  parent = gconf_key_directory (key);
  
  g_assert (parent != NULL);
  
  dir = cache_lookup (xs->cache, parent, TRUE, err);
  
  g_free (parent);
  parent = NULL;

  if (dir == NULL)
    return; /* error should be set */
  else
    {
      const gchar* relative_key;
      
      relative_key = gconf_key_key (key);
      
      dir_set_schema (dir, relative_key, schema_key, err);
    }
}

static gboolean      
sync_all        (GConfSource* source,
                 GError** err)
{
  XMLSource* xs = (XMLSource*)source;

  return cache_sync (xs->cache, err);
}

static void          
destroy_source  (GConfSource* source)
{
  xs_destroy((XMLSource*)source);
}

static void
clear_cache     (GConfSource* source)
{
  XMLSource* xs = (XMLSource*)source;

  /* clean all entries older than 0 seconds */
  cache_clean(xs->cache, 0);
}

/* Initializer */

G_MODULE_EXPORT const gchar*
g_module_check_init (GModule *module)
{
  gconf_log(GCL_DEBUG, _("Initializing XML backend module"));

  LIBXML_TEST_VERSION;
  xmlKeepBlanksDefault(1);

  return NULL;
}

G_MODULE_EXPORT GConfBackendVTable* 
gconf_backend_get_vtable(void)
{
  return &xml_vtable;
}

/* ****************************************************/

/*
 *  XMLSource
 */ 

/* This timeout periodically cleans up
   the old cruft in the cache */
static gboolean
cleanup_timeout(gpointer data)
{
  XMLSource* xs = (XMLSource*)data;

  cache_clean(xs->cache, 60*5 /* 5 minutes */);

  return TRUE;
}

static XMLSource*
xs_new       (const gchar* root_dir, guint dir_mode, guint file_mode, int lock_fd)
{
  XMLSource* xs;

  g_return_val_if_fail(root_dir != NULL, NULL);

  xs = g_new0(XMLSource, 1);

  xs->root_dir = g_strdup(root_dir);

  xs->cache = cache_new(xs->root_dir, dir_mode, file_mode);

  xs->timeout_id = g_timeout_add(1000*60*5, /* 1 sec * 60 s/min * 5 min */
                                 cleanup_timeout,
                                 xs);

  xs->lock_fd = lock_fd;

  xs->dir_mode = dir_mode;
  xs->file_mode = file_mode;
  
  return xs;
}

static void
xs_destroy   (XMLSource* xs)
{
  g_return_if_fail(xs != NULL);

  /* do this first in case we're in a "fast cleanup just before exit"
     situation */
  if (xs->lock_fd != -1)
    close(xs->lock_fd);

  if (!g_source_remove(xs->timeout_id))
    {
      /* should not happen, don't translate */
      gconf_log(GCL_ERR, "timeout not found to remove?");
    }
  
  cache_destroy(xs->cache);
  g_free(xs->root_dir);
  g_free(xs);
}

