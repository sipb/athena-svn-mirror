/* smb-method.h - VFS modules for SMB
 *
 * Docs:
 * http://www.ietf.org/internet-drafts/draft-crhertel-smb-url-05.txt
 * http://samba.org/doxygen/samba/group__libsmbclient.html
 * http://ubiqx.org/cifs/
 *
 *  Copyright (C) 2001,2002,2003 Red Hat
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Bastien Nocera <hadess@hadess.net>
 *         Alexander Larsson <alexl@redhat.com>
 */

#include <config.h>

/* libgen first for basename */
#include <libgen.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime.h>

#include <libgnomevfs/gnome-vfs-i18n.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <libgnomevfs/gnome-vfs-module.h>
#include <libgnomevfs/gnome-vfs-module-shared.h>
#include <libgnomevfs/gnome-vfs-module-callback-module-api.h>
#include <libgnomevfs/gnome-vfs-standard-callbacks.h>

#include <libsmbclient.h>

int smbc_remove_unused_server(SMBCCTX * context, SMBCSRV * srv);

typedef enum {
	SMB_URI_ERROR,
	SMB_URI_WHOLE_NETWORK,
	SMB_URI_WORKGROUP_LINK,
	SMB_URI_WORKGROUP,
	SMB_URI_SERVER_LINK,
	SMB_URI_SERVER,
	SMB_URI_SHARE,
	SMB_URI_SHARE_FILE
}  SmbUriType;

typedef struct {
	char *server_name;
	char *share_name;
	char *domain;
	char *username;
	
	SMBCSRV *server;
	time_t last_time;
} SmbServerCacheEntry;

typedef struct {
	char *server_name; /* Server or workgroup */
	char *share_name; /* NULL for just the server or a workgroup,
			     NULL used as default for shares if there is no
			     specific one for the share */
	
	char *username;
	char *domain;
} SmbDefaultUser;

static GMutex *smb_lock;

static SMBCCTX *smb_context = NULL;

static GHashTable *server_cache = NULL;

#define SMB_BLOCK_SIZE (32*1024)

/* Reap unused server connections after 30 minutes */
#define SERVER_CACHE_REAP_TIMEOUT (30*60)
static guint server_cache_reap_timeout = 0;

/* The magic "default workgroup" hostname */
#define DEFAULT_WORKGROUP_NAME "X-GNOME-DEFAULT-WORKGROUP"


/* 5 minutes before we re-read the workgroup cache again */
#define WORKGROUP_CACHE_TIMEOUT (5*60)

static GHashTable *workgroups = NULL;
static int workgroups_errno;
static time_t workgroups_timestamp = 0;

static GHashTable *default_user_hashtable = NULL;

static GnomeVFSURI *current_uri = NULL;

/* Auth stuff: */

static gboolean done_pre_auth;
static gboolean done_auth;
static gboolean auth_cancelled;
static gboolean auth_save_password;
static char *auth_keyring;
static char *last_pwd;

/* Used to detect failed logins */
static gboolean cache_access_failed = FALSE;

static void init_auth (GnomeVFSURI *uri);

static gboolean invoke_save_auth (const char *server,
				  const char *share,
				  const char *username,
				  const char *domain,
				  const char *password,
				  const char *keyring);

#if 0
#define DEBUG_SMB_ENABLE
#endif
#if 0
#define DEBUG_SMB_LOCKS
#endif

#ifdef DEBUG_SMB_ENABLE
#define DEBUG_SMB(x) g_print x
#else
#define DEBUG_SMB(x) 
#endif

#ifdef DEBUG_SMB_LOCKS
#define LOCK_SMB() 	{g_mutex_lock (smb_lock); g_print ("LOCK %s\n", G_GNUC_PRETTY_FUNCTION);}
#define UNLOCK_SMB() 	{g_print ("UNLOCK %s\n", G_GNUC_PRETTY_FUNCTION); g_mutex_unlock (smb_lock);}
#else
#define LOCK_SMB() 	g_mutex_lock (smb_lock)
#define UNLOCK_SMB() 	g_mutex_unlock (smb_lock)
#endif

static void auth_fn (const char *server_name, const char *share_name,
		     char *domain, int domainmaxlen,
		     char *username, int unmaxlen,
		     char *password, int pwmaxlen);
static gboolean
string_compare (const char *a, const char *b)
{
	if (a != NULL && b != NULL) {
		return strcmp (a, b) == 0;
	} else {
		return a == b;
	}
}

static gboolean
server_equal (gconstpointer  v1,
	      gconstpointer  v2)
{
	const SmbServerCacheEntry *e1, *e2;

	e1 = v1;
	e2 = v2;

	return (string_compare (e1->server_name, e2->server_name) &&
		string_compare (e1->share_name, e2->share_name) &&
		string_compare (e1->domain, e2->domain) &&
		string_compare (e1->username, e2->username));
}

static guint
server_hash (gconstpointer  v)
{
	const SmbServerCacheEntry *e;
	guint hash;

	e = v;
	hash = 0;
	if (e->server_name != NULL) {
		hash = hash ^ g_str_hash (e->server_name);
	}
	if (e->share_name != NULL) {
		hash = hash ^ g_str_hash (e->share_name);
	}
	if (e->domain != NULL) {
		hash = hash ^ g_str_hash (e->domain);
	}
	if (e->username != NULL) {
		hash = hash ^ g_str_hash (e->username);
	}

	return hash;
}

static void
server_free (SmbServerCacheEntry *entry)
{
	g_free (entry->server_name);
	g_free (entry->share_name);
	g_free (entry->domain);
	g_free (entry->username);

	/* TODO: Should we remove the server here? */
	
	g_free (entry);
}

static gboolean
default_user_equal (gconstpointer  v1,
		    gconstpointer  v2)
{
	const SmbDefaultUser *e1, *e2;

	e1 = v1;
	e2 = v2;
	
	return (string_compare (e1->server_name, e2->server_name) &&
		string_compare (e1->share_name, e2->share_name));
}

static guint
default_user_hash (gconstpointer  v)
{
	const SmbDefaultUser *e;
	guint hash;

	e = v;
	hash = 0;
	if (e->server_name) {
		hash = g_str_hash (e->server_name);
	}
	if (e->share_name) {
		hash = hash ^ g_str_hash (e->share_name);
	}
	return hash;
}

static void
default_user_free (SmbDefaultUser *entry)
{
	g_free (entry->server_name);
	g_free (entry->share_name);
	
	g_free (entry->username);
	g_free (entry->domain);

	g_free (entry);
}

static void
add_old_servers (gpointer key,
		gpointer value,
		gpointer user_data)
{
	SmbServerCacheEntry *entry;
	GPtrArray *array;
	time_t now;

	now = time (NULL);

	entry = key;
	array = user_data;
	if (now > entry->last_time + SERVER_CACHE_REAP_TIMEOUT ||
	    now < entry->last_time) {
		g_ptr_array_add (array, entry->server);
	}
}


static gboolean
server_cache_reap_cb (void)
{
     	int size;
	GPtrArray *servers;
	int i;

	size = g_hash_table_size (server_cache);
	servers = g_ptr_array_sized_new (size);

	/* The remove can change the hashtable, make a copy */
	g_hash_table_foreach (server_cache, add_old_servers, servers);
		
	for (i = 0; i < servers->len; i++) {
		smbc_remove_unused_server (smb_context,
					   (SMBCSRV *)g_ptr_array_index (servers, i));
	}

	g_ptr_array_free (servers, TRUE);

	if (g_hash_table_size (server_cache) == 0) {
		server_cache_reap_timeout = 0;
		return FALSE;
	} else {
		return TRUE;
	}
}

static void
schedule_server_cache_reap (void)
{
	if (server_cache_reap_timeout == 0) {
		server_cache_reap_timeout = g_timeout_add (SERVER_CACHE_REAP_TIMEOUT*1000,
							   (GSourceFunc)server_cache_reap_cb, NULL);
	}
}

static int
add_cached_server (SMBCCTX *context, SMBCSRV *new,
		   const char *server_name, const char *share_name, 
		   const char *domain, const char *username)
{
	SmbServerCacheEntry *entry = NULL;
	GnomeVFSToplevelURI *toplevel;
	SmbDefaultUser *default_user;
	const char *ask_share_name;

	DEBUG_SMB(("add_cached_server: server: %s, share: %s, domain: %s, user: %s\n",
		   server_name, share_name, domain, username));
	
	schedule_server_cache_reap ();
	
	entry = g_new0 (SmbServerCacheEntry, 1);
	
	entry->server = new;
	
	entry->server_name = g_strdup (server_name);
	entry->share_name = g_strdup (share_name);
	entry->domain = g_strdup (domain);
	entry->username = g_strdup (username);
	entry->last_time = time (NULL);

	g_hash_table_insert (server_cache, entry, entry);

	cache_access_failed = FALSE;

	if (current_uri != NULL) {
		toplevel = (GnomeVFSToplevelURI *)current_uri;

		if (toplevel->user_name == NULL ||
		    toplevel->user_name[0] == 0) {
			default_user = g_new0 (SmbDefaultUser, 1);
			default_user->server_name = g_strdup (server_name);
			default_user->share_name = g_strdup (share_name);
			default_user->username = g_strdup (username);
			default_user->domain = g_strdup (domain);
			g_hash_table_insert (default_user_hashtable, default_user, default_user);
		}
	}

	if (auth_save_password) {
		if (strcmp (share_name,"IPC$") == 0) {
			ask_share_name = NULL;
		} else {
			ask_share_name = share_name;
		}
		invoke_save_auth (server_name, ask_share_name,
				  username, domain,
				  last_pwd?last_pwd:"",
				  auth_keyring);
	}
	
	return 0;
}

static SMBCSRV *
get_cached_server (SMBCCTX * context,
		   const char *server_name, const char *share_name,
		   const char *domain, const char *username)
{
	SmbServerCacheEntry entry;
	SmbServerCacheEntry *res;

	entry.server_name = (char *)server_name;
	entry.share_name = (char *)share_name;
	entry.domain = (char *)domain;
	entry.username = (char *)username;

	res = g_hash_table_lookup (server_cache, &entry);

	if (res != NULL) {
		cache_access_failed = FALSE;
		res->last_time = time (NULL);
		return res->server;
	}
	cache_access_failed = TRUE;
	return NULL;
}

static gboolean
remove_server  (gpointer key,
		gpointer value,
		gpointer user_data)
{
	SmbServerCacheEntry *entry;
	SmbDefaultUser default_user;
	SMBCSRV *server;
	
	entry = key;
	server = user_data;

	if (entry->server == user_data) {
		default_user.server_name = entry->server_name;
		default_user.share_name = entry->share_name;
		default_user.username = entry->username;
		default_user.domain = entry->domain;
		
		g_hash_table_remove (default_user_hashtable, &default_user);
		
		entry->server = NULL;
		return TRUE;
	} 
	return FALSE;
}

static int remove_cached_server(SMBCCTX * context, SMBCSRV * server)
{
	int removed;
	
	removed = g_hash_table_foreach_remove (server_cache, remove_server, server);

	/* return 1 if failed */
	return removed == 0;
}


static void
add_server (gpointer key,
	    gpointer value,
	    gpointer user_data)
{
	SmbServerCacheEntry *entry;
	GPtrArray *array;

	entry = key;
	array = user_data;
	g_ptr_array_add (array, entry->server);
}

static int
purge_cached(SMBCCTX * context)
{
	int size;
	GPtrArray *servers;
	gboolean could_not_purge_all;
	int i;

	size = g_hash_table_size (server_cache);
	servers = g_ptr_array_sized_new (size);

	/* The remove can change the hashtable, make a copy */
	g_hash_table_foreach (server_cache, add_server, servers);
		
	could_not_purge_all = FALSE;
	for (i = 0; i < servers->len; i++) {
		if (smbc_remove_unused_server(context,
					      (SMBCSRV *)g_ptr_array_index (servers, i))) {
			/* could not be removed */
			could_not_purge_all = TRUE;
		}
	}

	g_ptr_array_free (servers, TRUE);
	
	return could_not_purge_all;
}


static gboolean
remove_all (gpointer  key,
	    gpointer  value,
	    gpointer  user_data)
{
	return TRUE;
}

static void
update_workgroup_cache (void)
{
	SMBCFILE *dir;
	time_t t;
	struct smbc_dirent *dirent;
	
	t = time (NULL);

	if (workgroups_timestamp != 0 &&
	    workgroups_timestamp < t &&
	    t < workgroups_timestamp + WORKGROUP_CACHE_TIMEOUT) {
		/* Up to date */
		return;
	}
	workgroups_timestamp = t;

	DEBUG_SMB(("update_workgroup_cache: enumerating workgroups\n"));
	
	g_hash_table_foreach_remove (workgroups, remove_all, NULL);
	
	LOCK_SMB();
	workgroups_errno = 0;
	init_auth (NULL);
	dir = smb_context->opendir (smb_context, "smb://");
	if (dir != NULL) {
		while ((dirent = smb_context->readdir (smb_context, dir)) != NULL) {
			if (dirent->smbc_type == SMBC_WORKGROUP &&
			    dirent->name != NULL &&
			    strlen (dirent->name) > 0) {
				g_hash_table_insert (workgroups,
						     g_ascii_strdown (dirent->name, -1),
						     GINT_TO_POINTER (1));
			} else {
				g_warning ("non-workgroup at smb toplevel\n");
			}
		}
		smb_context->closedir (smb_context, dir);
	} else {
		workgroups_errno = errno;
	}
	UNLOCK_SMB();
}

static SmbUriType
smb_uri_type (GnomeVFSURI *uri)
{
	GnomeVFSToplevelURI *toplevel;
	char *first_slash;

	toplevel = (GnomeVFSToplevelURI *)uri;

	if (toplevel->host_name == NULL || toplevel->host_name[0] == 0) {
		/* smb:/// or smb:///foo */
		if (uri->text == NULL ||
		    uri->text[0] == 0 ||
		    strcmp (uri->text, "/") == 0) {
			return SMB_URI_WHOLE_NETWORK;
		}
		if (strchr (uri->text + 1, '/')) {
			return SMB_URI_ERROR;
		}
		return SMB_URI_WORKGROUP_LINK;
	}
	if (uri->text == NULL ||
	    uri->text[0] == 0 ||
	    strcmp (uri->text, "/") == 0) {
		/* smb://foo/ */
		update_workgroup_cache ();
		if (!g_ascii_strcasecmp(toplevel->host_name,
					DEFAULT_WORKGROUP_NAME) ||
		    g_hash_table_lookup (workgroups, toplevel->host_name)) {
			return SMB_URI_WORKGROUP;
		} else {
			return SMB_URI_SERVER;
		}
	}
	first_slash = strchr (uri->text + 1, '/');
	if (first_slash == NULL) {
		/* smb://foo/bar */
		update_workgroup_cache ();
		if (!g_ascii_strcasecmp(toplevel->host_name,
					DEFAULT_WORKGROUP_NAME) ||
		    g_hash_table_lookup (workgroups, toplevel->host_name)) {
			return SMB_URI_SERVER_LINK;
		} else {
			return SMB_URI_SHARE;
		}
	}
	
	return SMB_URI_SHARE_FILE;
}



static gboolean
is_hidden_entry (char *name)
{
	if (name == NULL) return TRUE;

	if (*(name + strlen (name) -1) == '$') return TRUE;

	return FALSE;

}

static gboolean
try_init (void)
{
	char *path;
	struct stat statbuf;

	LOCK_SMB();

	/* We used to create an empty ~/.smb/smb.conf to get
	 * default settings, but this breaks a lot of smb.conf
	 * configurations, so we remove this again. If you really
	 * need an empty smb.conf, put a newline in it */
	path = g_build_filename (G_DIR_SEPARATOR_S, g_get_home_dir (),
			".smb", "smb.conf", NULL);

	if (stat (path, &statbuf) == 0) {
		if (S_ISREG (statbuf.st_mode) &&
		    statbuf.st_size == 0) {
			unlink (path);
		}
	}
	g_free (path);

	smb_context = smbc_new_context ();
	if (smb_context != NULL) {
		smb_context->debug = 0;
		smb_context->callbacks.auth_fn = auth_fn;
		smb_context->callbacks.add_cached_srv_fn    = add_cached_server;
		smb_context->callbacks.get_cached_srv_fn    = get_cached_server;
		smb_context->callbacks.remove_cached_srv_fn = remove_cached_server;
		smb_context->callbacks.purge_cached_fn      = purge_cached;

		if (!smbc_init_context (smb_context)) {
			smbc_free_context (smb_context, FALSE);
			smb_context = NULL;
		}

#if defined(HAVE_SAMBA_FLAGS) && defined(SMB_CTX_FLAG_USE_KERBEROS) && defined(SMB_CTX_FLAG_FALLBACK_AFTER_KERBEROS)
		smb_context->flags |= SMB_CTX_FLAG_USE_KERBEROS | SMB_CTX_FLAG_FALLBACK_AFTER_KERBEROS;
#endif
		
	}

	server_cache = g_hash_table_new_full (server_hash,
					      server_equal,
					      (GDestroyNotify)server_free,
					      NULL);
	workgroups = g_hash_table_new_full (g_str_hash,
					    g_str_equal,
					    g_free, NULL);
	
	default_user_hashtable = g_hash_table_new_full (default_user_hash,
							default_user_equal,
							(GDestroyNotify)default_user_free, NULL);
	
	UNLOCK_SMB();

	if (smb_context == NULL) {
		g_warning ("Could not initialize samba client library\n");
		return FALSE;
	}

	return TRUE;
}

static gboolean
invoke_fill_auth (const char *server,
		 const char *share,
		 const char *username,
		 const char *domain,
		 char **username_out,
		 char **domain_out,
		 char **password_out)
{
	GnomeVFSModuleCallbackFillAuthenticationIn in_args;
	GnomeVFSModuleCallbackFillAuthenticationOut out_args;
	gboolean invoked;

	if (username != NULL && username[0] == 0) {
		username = NULL;
	}
	if (domain != NULL && domain[0] == 0) {
		domain = NULL;
	}

	memset (&in_args, 0, sizeof (in_args));
	in_args.uri = gnome_vfs_uri_to_string (current_uri, 0);
	in_args.protocol = "smb";
	in_args.server = (char *)server;
	in_args.object = (char *)share;
	in_args.username = (char *)username;
	in_args.domain = (char *)domain;
	in_args.port = ((GnomeVFSToplevelURI *)current_uri)->host_port;

	memset (&out_args, 0, sizeof (out_args));

	invoked = gnome_vfs_module_callback_invoke
		(GNOME_VFS_MODULE_CALLBACK_FILL_AUTHENTICATION,
		 &in_args, sizeof (in_args),
		 &out_args, sizeof (out_args));

	if (invoked && out_args.valid) {
		*username_out = g_strdup (out_args.username);
		*domain_out = g_strdup (out_args.domain);
		*password_out = g_strdup (out_args.password);
	} else {
		*username_out = NULL;
		*domain_out = NULL;
		*password_out = NULL;
	}

	g_free (in_args.uri);
	g_free (out_args.username);
	g_free (out_args.domain);
	g_free (out_args.password);

	return invoked && out_args.valid;
}

static gboolean
invoke_full_auth (const char *server,
		  const char *share,
		  const char *username,
		  const char *domain,
		  gboolean *cancel_auth_out,
		  char **username_out,
		  char **domain_out,
		  char **password_out,
		  gboolean *save_password_out,
		  char **keyring_out)
{
	GnomeVFSModuleCallbackFullAuthenticationIn in_args;
	GnomeVFSModuleCallbackFullAuthenticationOut out_args;
	gboolean invoked;
	
	if (username != NULL && username[0] == 0) {
		username = NULL;
	}
	if (domain != NULL && domain[0] == 0) {
		domain = NULL;
	}
	
	memset (&in_args, 0, sizeof (in_args));
	in_args.uri = gnome_vfs_uri_to_string (current_uri, 0);
	in_args.flags = GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_PASSWORD | GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_SAVING_SUPPORTED;
	if (done_auth) {
		in_args.flags |= GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_PREVIOUS_ATTEMPT_FAILED;
	}
	if (((GnomeVFSToplevelURI *)current_uri)->user_name == NULL) {
		in_args.flags |=
			GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_USERNAME |
			GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION_NEED_DOMAIN;
	}
	in_args.protocol = "smb";
	in_args.server = (char *)server;
	in_args.object = (char *)share;
	in_args.username = (char *)username;
	in_args.domain = (char *)domain;
	in_args.port = ((GnomeVFSToplevelURI *)current_uri)->host_port;

	/* TODO: set default_user & default_domain? */
	
	memset (&out_args, 0, sizeof (out_args));

	invoked = gnome_vfs_module_callback_invoke
		(GNOME_VFS_MODULE_CALLBACK_FULL_AUTHENTICATION,
		 &in_args, sizeof (in_args),
		 &out_args, sizeof (out_args));

	if (invoked) {
		*cancel_auth_out = out_args.abort_auth;
		*username_out = g_strdup (out_args.username);
		*domain_out = g_strdup (out_args.domain);
		*password_out = g_strdup (out_args.password);
		*save_password_out = out_args.save_password;
		*keyring_out = g_strdup (out_args.keyring);
	} else {
		*cancel_auth_out = FALSE;
		*username_out = NULL;
		*domain_out = NULL;
		*password_out = NULL;
	}
	g_free (out_args.username);
	g_free (out_args.domain);
	g_free (out_args.password);
	g_free (out_args.keyring);

	g_free (in_args.uri);

	return invoked;
}


static gboolean
invoke_save_auth (const char *server,
		  const char *share,
		  const char *username,
		  const char *domain,
		  const char *password,
		  const char *keyring)
{
	GnomeVFSModuleCallbackSaveAuthenticationIn in_args;
	GnomeVFSModuleCallbackSaveAuthenticationOut out_args;
	gboolean invoked;

	if (username != NULL && username[0] == 0) {
		username = NULL;
	}
	if (domain != NULL && domain[0] == 0) {
		domain = NULL;
	}
	if (keyring != NULL && keyring[0] == 0) {
		keyring = NULL;
	}
	
	memset (&in_args, 0, sizeof (in_args));
	in_args.uri = gnome_vfs_uri_to_string (current_uri, 0);
	in_args.keyring = (char *)keyring;
	in_args.protocol = "smb";
	in_args.server = (char *)server;
	in_args.object = (char *)share;
	in_args.port = ((GnomeVFSToplevelURI *)current_uri)->host_port;
	in_args.authtype = NULL;
	in_args.username = (char *)username;
	in_args.domain = (char *)domain;
	in_args.password = (char *)password;

	memset (&out_args, 0, sizeof (out_args));

	invoked = gnome_vfs_module_callback_invoke
		(GNOME_VFS_MODULE_CALLBACK_SAVE_AUTHENTICATION,
		 &in_args, sizeof (in_args),
		 &out_args, sizeof (out_args));

	g_free (in_args.uri);
	return invoked;
}


static void
init_auth (GnomeVFSURI *uri)
{
	done_pre_auth = FALSE;
	done_auth = FALSE;
	auth_cancelled = FALSE;
	cache_access_failed = FALSE;
	current_uri = uri;
	auth_save_password = FALSE;
	if (last_pwd != NULL) {
		memset (last_pwd, 0, strlen (last_pwd));
		g_free (last_pwd);
		last_pwd = NULL;
	}
}

static gboolean
auth_failed (void)
{
	return cache_access_failed &&
		(errno == EACCES || errno == EPERM) &&
		!auth_cancelled;
}

static void
auth_fn (const char *server_name, const char *share_name,
	 char *domain_out, int domainmaxlen,
	 char *username_out, int unmaxlen,
	 char *password_out, int pwmaxlen)
{
	char *username, *domain, *tmp;
	char *real_username, *real_domain, *real_password;
	char *ask_share_name;
	GnomeVFSToplevelURI *current_toplevel_uri;
	gboolean cancel_auth;
	gboolean got_default_user;

	DEBUG_SMB (("auth_fn called: server: %s share: %s\n",
		    server_name, share_name));

	if (server_name == NULL || server_name[0] == 0) {
		/* We never authenticate for the toplevel (enumerating workgroups) */
		return;
	}

	if (current_uri == NULL) {
		/* TODO: What to do with this,
		   comes from e.g. enumerating workgroups which needs login on
		   master browser $IPC.
		*/
		DEBUG_SMB (("auth_fn - no current_uri, ignoring\n"));
		return;
	}
	
	got_default_user = FALSE;
	username = NULL;
	domain = NULL;
	current_toplevel_uri =	(GnomeVFSToplevelURI *)current_uri;
	if (current_toplevel_uri->user_name != NULL &&
	    current_toplevel_uri->user_name[0] != 0) {
		tmp = strchr (current_toplevel_uri->user_name, ';');
		if (tmp != NULL) {
			domain = g_strndup (current_toplevel_uri->user_name,
					    tmp - current_toplevel_uri->user_name);
			username = g_strdup (tmp + 1);
		} else {
			username = g_strdup (current_toplevel_uri->user_name);
			domain = NULL;
		}
	} else {
		SmbDefaultUser lookup;
		SmbDefaultUser *default_user;
		
		/* lookup default user/domain */
		lookup.server_name = (char *)server_name;
		lookup.share_name = (char *)share_name;

		default_user = g_hash_table_lookup (default_user_hashtable, &lookup);
		if (default_user != NULL) {
			got_default_user = TRUE;
			username = g_strdup (default_user->username);
			domain = g_strdup (default_user->domain);
		}
	}

	if (strcmp (share_name,"IPC$") == 0) {
		/* Don't authenticate to IPC$ using dialog, but allow name+domain in uri */
		if (username != NULL) {
			strncpy (username_out, username, unmaxlen);
		}
		if (domain != NULL) {
			strncpy (domain_out, domain, domainmaxlen);
		}
		strncpy (password_out, "", pwmaxlen);
		g_free (username);
		g_free (domain);
		return;
	}

	if (got_default_user ||
	    (username != NULL && username[0] != 0)) {
		SmbServerCacheEntry server_lookup;
		SmbServerCacheEntry *server;
		
		server_lookup.server_name = (char *)server_name;
		server_lookup.share_name = (char *)share_name;
		server_lookup.username = username;
		server_lookup.domain = domain;
		
		server = g_hash_table_lookup (server_cache, &server_lookup);
		if (server != NULL) {
			strncpy (username_out, username, unmaxlen);
			if (domain != NULL) {
				strncpy (domain_out, domain, domainmaxlen);
			} 
			strncpy (password_out, "", pwmaxlen);

			/* Server is in cache already, no need to get password */
			return ;
		}
	}
	
	if (strcmp (share_name,"IPC$") == 0) {
		ask_share_name = NULL;
	} else {
		ask_share_name = (char *)share_name;
	}
	
	if (!done_pre_auth) {
		/* call pre-auth, if got filled, return to test auth */
		done_pre_auth = TRUE;
		if (invoke_fill_auth (server_name, ask_share_name,
				      username, domain,
				      &real_username,
				      &real_domain,
				      &real_password)) {
			g_free (username);
			g_free (domain);
			
			if (real_username != NULL) {
				strncpy (username_out, real_username, unmaxlen);
			}
			if (real_domain != NULL) {
				strncpy (domain_out, real_domain, domainmaxlen);
			}
			if (real_password != NULL) {
				strncpy (password_out, real_password, pwmaxlen);
			}

			g_free (real_username);
			g_free (real_domain);
			g_free (real_password);

			return;
		}
	}

	g_free (auth_keyring);
	auth_keyring = NULL;
	if (invoke_full_auth (server_name, ask_share_name,
			      username, domain,
			      &cancel_auth,
			      &real_username,
			      &real_domain,
			      &real_password,
			      &auth_save_password,
			      &auth_keyring)) {
		if (cancel_auth) {
			auth_cancelled = TRUE;
			strncpy (username_out, "not", unmaxlen);
			strncpy (password_out, "matching", unmaxlen);
		} else {
			/* Try this auth */
			if (real_username != NULL) {
				strncpy (username_out, real_username, unmaxlen);
			}
			if (real_domain != NULL) {
				strncpy (domain_out, real_domain, domainmaxlen);
			}
			if (real_password != NULL) {
				strncpy (password_out, real_password, pwmaxlen);
			}

			if (auth_save_password) {
				last_pwd = g_strdup (real_password);
			}
			
			g_free (real_username);
			g_free (real_domain);
			g_free (real_password);
		}
	} else {
		if (done_auth) {
			auth_cancelled = TRUE;
			strncpy (username_out, "not", unmaxlen);
			strncpy (password_out, "matching", unmaxlen);
		}
		/* else, no auth callback registered and first try, try anon */
	}
	
	done_auth = TRUE;

	return;
}


static char *
get_workgroup_data (const char *display_name, const char *name)
{
	return g_strdup_printf ("[Desktop Entry]\n"
			"Encoding=UTF-8\n"
			"Name=%s\n"
			"Type=Link\n"
			"URL=smb://%s/\n"
			"Icon=gnome-fs-network\n",
			display_name, name);
}
                                                                                
static char *
get_computer_data (const char *display_name, const char *name)
{
	return g_strdup_printf ("[Desktop Entry]\n"
			"Encoding=UTF-8\n"
			"Name=%s\n"
			"Type=Link\n"
			"URL=smb://%s/\n"
			"Icon=gnome-fs-server\n",
			display_name, name);
}


static gchar *
get_base_from_uri (GnomeVFSURI const *uri)
{
	gchar *escaped_base, *base;

	escaped_base = gnome_vfs_uri_extract_short_path_name (uri);
	base = gnome_vfs_unescape_string (escaped_base, G_DIR_SEPARATOR_S);
	g_free (escaped_base);
	return base;
}



typedef struct {
	SMBCFILE *file;
	gboolean is_data;
	char *file_data;
	int fnum;
	GnomeVFSFileOffset offset;
	GnomeVFSFileOffset file_size;
} FileHandle;

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	FileHandle *handle = NULL;
	char *path, *name, *unescaped_name;
	int type;
	mode_t unix_mode;
	SMBCFILE *file;
	
	DEBUG_SMB(("do_open() %s mode %d\n",
				gnome_vfs_uri_to_string (uri, 0), mode));

	type = smb_uri_type (uri);

	if (type == SMB_URI_ERROR) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}
	
	if (type == SMB_URI_WHOLE_NETWORK ||
	    type == SMB_URI_WORKGROUP ||
	    type == SMB_URI_SERVER ||
	    type == SMB_URI_SHARE) {
		return GNOME_VFS_ERROR_IS_DIRECTORY;
	}

	if (type == SMB_URI_WORKGROUP_LINK) {
		if (mode & GNOME_VFS_OPEN_WRITE) {
			return GNOME_VFS_ERROR_READ_ONLY;
		}
		handle = g_new (FileHandle, 1);
		handle->is_data = TRUE;
		handle->offset = 0;
		unescaped_name = get_base_from_uri (uri);
		name = gnome_vfs_uri_extract_short_path_name (uri);
		handle->file_data = get_workgroup_data (unescaped_name, name);
		handle->file_size = strlen (handle->file_data);
		g_free (unescaped_name);
		g_free (name);
		
		*method_handle = (GnomeVFSMethodHandle *)handle;
		
		return GNOME_VFS_OK;
	}

	if (type == SMB_URI_SERVER_LINK) {
		if (mode & GNOME_VFS_OPEN_WRITE) {
			return GNOME_VFS_ERROR_READ_ONLY;
		}
		handle = g_new (FileHandle, 1);
		handle->is_data = TRUE;
		handle->offset = 0;
		unescaped_name = get_base_from_uri (uri);
		name = gnome_vfs_uri_extract_short_path_name (uri);
		handle->file_data = get_computer_data (unescaped_name, name);
		handle->file_size = strlen (handle->file_data);
		g_free (unescaped_name);
		g_free (name);
		
		*method_handle = (GnomeVFSMethodHandle *)handle;
		
		return GNOME_VFS_OK;
	}

	g_assert (type == SMB_URI_SHARE_FILE);

	if (mode & GNOME_VFS_OPEN_READ) {
		if (mode & GNOME_VFS_OPEN_WRITE)
			unix_mode = O_RDWR;
		else
			unix_mode = O_RDONLY;
	} else {
		if (mode & GNOME_VFS_OPEN_WRITE)
			unix_mode = O_WRONLY;
		else
			return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	}

	if (! (mode & GNOME_VFS_OPEN_RANDOM) && (mode & GNOME_VFS_OPEN_WRITE))
		unix_mode |= O_TRUNC;
	
	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);
	
	LOCK_SMB();
	init_auth (uri);
 again:
	file = smb_context->open (smb_context, path, unix_mode, 0666);
	if (file == NULL && auth_failed ()) {
		goto again;
	}
	UNLOCK_SMB();
	
	if (file == NULL) {
		g_free (path);
		return gnome_vfs_result_from_errno ();

	}
	g_free (path);
	handle = g_new (FileHandle, 1);
	handle->is_data = FALSE;
	handle->file = file;

	*method_handle = (GnomeVFSMethodHandle *)handle;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)

{
	FileHandle *handle = (FileHandle *)method_handle;
	GnomeVFSResult res;

	DEBUG_SMB(("do_close()\n"));

	res = GNOME_VFS_OK;

	if (handle->is_data) {
		g_free (handle->file_data);
	} else {
		LOCK_SMB();
		init_auth (NULL);
		if (smb_context->close (smb_context, handle->file) < 0) {
			res = gnome_vfs_result_from_errno ();
		}
		UNLOCK_SMB();
	}

	g_free (handle);

	return res;
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	FileHandle *handle = (FileHandle *)method_handle;
	GnomeVFSResult res;
	ssize_t n;

	DEBUG_SMB(("do_read() %Lu bytes\n", num_bytes));

	if (handle->is_data) {
		if (handle->offset >= handle->file_size) {
			n = 0;
		} else {
			n = MIN (num_bytes, handle->file_size - handle->offset);
			memcpy (buffer, handle->file_data + handle->offset, n);
		}
	} else {
		LOCK_SMB();
		init_auth (NULL);
		n = smb_context->read (smb_context, handle->file, buffer, num_bytes);
		UNLOCK_SMB();
	}

	/* Can only happen when reading from smb: */
	if (n < 0) {
		*bytes_read = 0;
		res = gnome_vfs_result_from_errno ();
	} else {
		res = GNOME_VFS_OK;
	}

	*bytes_read = n;

	if (n == 0) {
		return GNOME_VFS_ERROR_EOF;
	}

	handle->offset += n;

	return res;
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)


{
	GnomeVFSResult res;
	FileHandle *handle = (FileHandle *)method_handle;
	ssize_t written;

	DEBUG_SMB (("do_write() %p\n", method_handle));

	if (handle->is_data) {
		return GNOME_VFS_ERROR_READ_ONLY;
	}

	LOCK_SMB();
	init_auth (NULL);
	written = smb_context->write (smb_context, handle->file, (void *)buffer, num_bytes);
	UNLOCK_SMB();

	if (written < 0) {
		res = gnome_vfs_result_from_errno ();
		*bytes_written = 0;
	} else {
		res = GNOME_VFS_OK;
		*bytes_written = written;
	}

	return res;
}

static GnomeVFSResult
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	int type;
	mode_t unix_mode;
	char *path;
	SMBCFILE *file;
	FileHandle *handle;
	
	DEBUG_SMB (("do_create() %s mode %d\n",
				gnome_vfs_uri_to_string (uri, 0), mode));

	
	type = smb_uri_type (uri);

	if (type == SMB_URI_ERROR) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	if (type == SMB_URI_WHOLE_NETWORK ||
	    type == SMB_URI_WORKGROUP ||
	    type == SMB_URI_SERVER ||
	    type == SMB_URI_SHARE) {
		return GNOME_VFS_ERROR_IS_DIRECTORY;
	}

	if (type == SMB_URI_WORKGROUP_LINK ||
	    type == SMB_URI_SERVER_LINK) {
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}
	
	unix_mode = O_CREAT | O_TRUNC;
	
	if (!(mode & GNOME_VFS_OPEN_WRITE))
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;

	if (mode & GNOME_VFS_OPEN_READ)
		unix_mode |= O_RDWR;
	else
		unix_mode |= O_WRONLY;

	if (exclusive)
		unix_mode |= O_EXCL;

	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);

	LOCK_SMB();
	init_auth (uri);
 again:
	file = smb_context->open (smb_context, path, unix_mode, perm);
	if (file == NULL && auth_failed ()) {
		goto again;
	}
	if (file == NULL) {
		UNLOCK_SMB();
		g_free (path);
		return gnome_vfs_result_from_errno ();

	}
	UNLOCK_SMB();
	g_free (path);
	handle = g_new (FileHandle, 1);
	handle->is_data = FALSE;
	handle->file = file;

	*method_handle = (GnomeVFSMethodHandle *)handle;
	
	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)

{
	struct stat st;
	char *path;
	int err, type;
	const char *mime_type;

	DEBUG_SMB (("do_get_file_info() %s\n",
				gnome_vfs_uri_to_string (uri, 0)));

	type = smb_uri_type (uri);

	if (type == SMB_URI_ERROR) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}
	
	if (type == SMB_URI_WHOLE_NETWORK ||
	    type == SMB_URI_WORKGROUP ||
	    type == SMB_URI_SERVER ||
	    type == SMB_URI_SHARE) {
		file_info->name = get_base_from_uri (uri);
		file_info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
			GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		if (type == SMB_URI_SHARE) {
			file_info->mime_type = g_strdup ("x-directory/smb-share");
		} else {
			file_info->mime_type = g_strdup ("x-directory/normal");
		}
		/* Make sure you can't write to smb:// or smb://foo. For smb://server/share we
		 * leave this empty, since accessing the data for real can cause authentication
		 * while e.g. browsing smb://server
		 */
		if (type != SMB_URI_SHARE) {
			file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS;
			file_info->permissions =
				GNOME_VFS_PERM_USER_READ |
				GNOME_VFS_PERM_OTHER_READ |
				GNOME_VFS_PERM_GROUP_READ;
		}
		return GNOME_VFS_OK;
	}

	if (type == SMB_URI_WORKGROUP_LINK ||
	    type == SMB_URI_SERVER_LINK) {
		file_info->name = get_base_from_uri (uri);
		file_info->valid_fields = file_info->valid_fields
			| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE
			| GNOME_VFS_FILE_INFO_FIELDS_TYPE
			| GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS;
		file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		file_info->mime_type = g_strdup ("application/x-desktop");
		file_info->permissions =
			GNOME_VFS_PERM_USER_READ |
			GNOME_VFS_PERM_OTHER_READ |
			GNOME_VFS_PERM_GROUP_READ;
		return GNOME_VFS_OK;
	}

	g_assert (type == SMB_URI_SHARE_FILE);
	    
	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);

	LOCK_SMB();
	init_auth (uri);
 again:
	err = smb_context->stat (smb_context, path, &st);
	if (err < 0 && auth_failed ()) {
		goto again;
	}
	
	if (err < 0) {
		UNLOCK_SMB();
		g_free (path);
		return gnome_vfs_result_from_errno ();
	}

	UNLOCK_SMB();
	
	g_free (path);
	
	gnome_vfs_stat_to_file_info (file_info, &st);
	file_info->name = get_base_from_uri (uri);

	file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE;
	file_info->io_block_size = SMB_BLOCK_SIZE;
	
	if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {		
		if (S_ISDIR(st.st_mode)) {
			mime_type = "x-directory/normal";
		} else {
			mime_type = gnome_vfs_mime_type_from_name_or_default (file_info->name, NULL);
		}
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		file_info->mime_type = g_strdup (mime_type);
	}

	DEBUG_SMB (("do_get_file_info()\n"
				"name: %s\n"
				"smb type: %d\n"
				"mimetype: %s\n"
				"type: %d\n",
				file_info->name, type,
				file_info->mime_type, file_info->type));

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
		GnomeVFSMethodHandle *method_handle,
		GnomeVFSFileInfo *file_info,
		GnomeVFSFileInfoOptions options,
		GnomeVFSContext *context)
{
	FileHandle *handle = (FileHandle *)method_handle;
	struct stat st;
	int err;

	LOCK_SMB();
	init_auth (NULL);
	err = smb_context->fstat (smb_context, handle->file, &st);
	UNLOCK_SMB();
	if (err < 0) {
		return gnome_vfs_result_from_errno ();
	}
	
	gnome_vfs_stat_to_file_info (file_info, &st);

	file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE;
	file_info->io_block_size = SMB_BLOCK_SIZE;
	return GNOME_VFS_OK;
}

static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)
{
	return FALSE;
}

typedef struct {
	GList *workgroups;
	SMBCFILE *dir;
	char *path;
} DirectoryHandle;

static void
add_workgroup (gpointer key,
	    gpointer value,
	    gpointer user_data)
{
	DirectoryHandle *directory_handle;

	directory_handle = user_data;
	directory_handle->workgroups = g_list_prepend (directory_handle->workgroups,
						       g_strdup (key));
}


static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   GnomeVFSContext *context)

{
	DirectoryHandle *directory_handle;
	GnomeVFSURI *new_uri = NULL;
	const char *host_name;
	char *path;
	SmbUriType type;
	SMBCFILE *dir;

	DEBUG_SMB(("do_open_directory() %s\n",
		gnome_vfs_uri_to_string (uri, 0)));

	type = smb_uri_type (uri);

	if (type == SMB_URI_WHOLE_NETWORK) {
		update_workgroup_cache ();
		
		if (workgroups_errno != 0) {
			gnome_vfs_result_from_errno_code (workgroups_errno);
		}
		
		directory_handle = g_new0 (DirectoryHandle, 1);
		g_hash_table_foreach (workgroups, add_workgroup, directory_handle);
		*method_handle = (GnomeVFSMethodHandle *) directory_handle;
		return GNOME_VFS_OK;
	}

	if (type == SMB_URI_ERROR ||
	    type == SMB_URI_WORKGROUP_LINK ||
	    type == SMB_URI_SERVER_LINK) {
		return GNOME_VFS_ERROR_NOT_A_DIRECTORY;
	}

	/* if it is the magic default workgroup name, map it to the 
	 * SMBCCTX's workgroup, which comes from the smb.conf file. */
	host_name = gnome_vfs_uri_get_host_name (uri);
	if (type == SMB_URI_WORKGROUP && host_name != NULL &&
	    !g_ascii_strcasecmp(host_name, DEFAULT_WORKGROUP_NAME)) {
		new_uri = gnome_vfs_uri_dup (uri);
		gnome_vfs_uri_set_host_name (new_uri,
					     smb_context->workgroup
					     ? smb_context->workgroup
					     : "WORKGROUP");
		uri = new_uri;
	}
		
	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);

	DEBUG_SMB(("do_open_directory() path %s\n", path));

	LOCK_SMB();
	init_auth (uri);
 again:
	dir = smb_context->opendir (smb_context, path);
	if (dir == NULL && auth_failed ()) {
		goto again;
	}
	
	if (dir == NULL) {
		UNLOCK_SMB();
		g_free (path);
		if (new_uri) gnome_vfs_uri_unref (new_uri);
		return gnome_vfs_result_from_errno ();
	}

	UNLOCK_SMB();

	if (new_uri) gnome_vfs_uri_unref (new_uri);

	/* Construct the handle */
	directory_handle = g_new0 (DirectoryHandle, 1);
	directory_handle->dir = dir;
	directory_handle->path = path;
	*method_handle = (GnomeVFSMethodHandle *) directory_handle;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	DirectoryHandle *directory_handle = (DirectoryHandle *) method_handle;
	GnomeVFSResult res;
	GList *l;
	int err;

	DEBUG_SMB(("do_close_directory: %p\n", directory_handle));

	if (directory_handle == NULL)
		return GNOME_VFS_OK;

	if (directory_handle->workgroups != NULL) {
		for (l = directory_handle->workgroups; l != NULL; l = l->next) {
			g_free (l->data);
		}
		g_list_free (directory_handle->workgroups);
	}

	res = GNOME_VFS_OK;
	
	if (directory_handle->dir != NULL) {
		LOCK_SMB ();
		init_auth (NULL);
		err = smb_context->closedir (smb_context, directory_handle->dir);
		if (err < 0) {
			res = gnome_vfs_result_from_errno ();
		} 
		UNLOCK_SMB ();
	}
	g_free (directory_handle->path);
	g_free (directory_handle);

	return res;
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	DirectoryHandle *dh = (DirectoryHandle *) method_handle;
	struct smbc_dirent *entry;
	struct stat st;
	char *statpath;
	char *path;
	GList *l;

	DEBUG_SMB (("do_read_directory()\n"));

	if (dh->dir == NULL) {
		if (dh->workgroups == NULL) {
			return GNOME_VFS_ERROR_EOF;
		} else {
			/* workgroup link */
			l = dh->workgroups;
			dh->workgroups = g_list_remove_link (dh->workgroups, l);
			file_info->name = l->data;
			g_list_free_1 (l);
			
			file_info->valid_fields =
				GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE
				| GNOME_VFS_FILE_INFO_FIELDS_TYPE;
			file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
			file_info->mime_type = g_strdup ("application/x-desktop");
			return GNOME_VFS_OK;
		}
	}
	
	
	LOCK_SMB();
	do {
		errno = 0;
		init_auth (NULL);
		entry = smb_context->readdir (smb_context, dh->dir);
		
		if (entry == NULL) {
			UNLOCK_SMB();
			
			if (errno != 0) {
				return gnome_vfs_result_from_errno ();
			} else {
				return GNOME_VFS_ERROR_EOF;
			}
		}
	} while (entry->smbc_type == SMBC_COMMS_SHARE ||
		 entry->smbc_type == SMBC_IPC_SHARE ||
		 entry->smbc_type == SMBC_PRINTER_SHARE ||
		 entry->name == NULL ||
		 strlen (entry->name) == 0 ||
		 (entry->smbc_type == SMBC_FILE_SHARE &&
		  is_hidden_entry (entry->name)));
		
	UNLOCK_SMB();

	file_info->name = g_strndup (entry->name, entry->namelen);
	DEBUG_SMB (("do_read_directory (): read %s\n", file_info->name));

	file_info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE;

	switch (entry->smbc_type) {
	case SMBC_FILE_SHARE:
		file_info->valid_fields = file_info->valid_fields
			| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE
			| GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		file_info->mime_type = g_strdup ("x-directory/smb-share");
		break;
	case SMBC_WORKGROUP:
	case SMBC_SERVER:
		file_info->valid_fields = file_info->valid_fields
			| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE
			| GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		file_info->mime_type = g_strdup ("application/x-desktop");
		break;
	case SMBC_PRINTER_SHARE:
		/* Ignored above for now */
		file_info->valid_fields = file_info->valid_fields
			| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE
			| GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		file_info->mime_type = g_strdup ("application/x-smb-printer");
	case SMBC_COMMS_SHARE:
	case SMBC_IPC_SHARE:
		break;
	case SMBC_DIR:
	case SMBC_FILE:
		path = dh->path;
		
		if (path[strlen(path)-1] == '/') {
			statpath = g_strconcat (path, 
						gnome_vfs_escape_string (file_info->name),
						NULL);
		} else {
			statpath = g_strconcat (path,
						"/",
						gnome_vfs_escape_string (file_info->name),
						NULL);
		}
		/* TODO: might give an auth error, but should be rare due
		   to the succeeding opendir. If this happens and we can't
		   auth, we should terminate the readdir to avoid multiple
		   password dialogs
		*/
		
		LOCK_SMB();
		init_auth (NULL);
		if (smb_context->stat (smb_context, statpath, &st) == 0) {
			gnome_vfs_stat_to_file_info (file_info, &st);

			file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE;
			file_info->io_block_size = SMB_BLOCK_SIZE;
		}
		UNLOCK_SMB();
		g_free (statpath);

		file_info->valid_fields = file_info->valid_fields
			| GNOME_VFS_FILE_INFO_FIELDS_TYPE;

		if (entry->smbc_type == SMBC_DIR) {
			file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
			file_info->mime_type = g_strdup ("x-directory/normal");
			file_info->valid_fields = file_info->valid_fields
				| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		} else {
			file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
			file_info->mime_type = g_strdup (
				gnome_vfs_mime_type_from_name(file_info->name));
			file_info->valid_fields = file_info->valid_fields
				| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
		break;
	case SMBC_LINK:
		g_warning ("smb links not supported");
		/*FIXME*/
		break;
	default:
		g_assert_not_reached ();
	}

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
		GnomeVFSMethodHandle *method_handle,
		GnomeVFSSeekPosition whence,
		GnomeVFSFileOffset offset,
		GnomeVFSContext *context)
{
	FileHandle *handle = (FileHandle *)method_handle;
	GnomeVFSResult res;
	int meth_whence;
	off_t ret;

	if (handle->is_data) {
		switch (whence) {
		case GNOME_VFS_SEEK_START:
			handle->offset = MIN (offset, handle->file_size);
			break;
		case GNOME_VFS_SEEK_CURRENT:
			handle->offset = MIN (handle->offset + offset, handle->file_size);
			break;
		case GNOME_VFS_SEEK_END:
			if (offset > handle->file_size) {
				handle->offset = 0;
			} else {
				handle->offset = handle->file_size - offset;
			}
			break;
		default:
			return GNOME_VFS_ERROR_NOT_SUPPORTED;
		}
		return GNOME_VFS_OK;
	}
	
	switch (whence) {
	case GNOME_VFS_SEEK_START:
		meth_whence = SEEK_SET;
		break;
	case GNOME_VFS_SEEK_CURRENT:
		meth_whence = SEEK_CUR;
		break;
	case GNOME_VFS_SEEK_END:
		meth_whence = SEEK_END;
		break;
	default:
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

	LOCK_SMB();
	init_auth (NULL);
	ret = smb_context->lseek (smb_context, handle->file, (off_t) offset, meth_whence);
	UNLOCK_SMB();
	if (ret == (off_t) -1) {
		res = gnome_vfs_result_from_errno ();
	} else {
		res = GNOME_VFS_OK;
	}

	return res;
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
		GnomeVFSMethodHandle *method_handle,
		GnomeVFSFileOffset *offset_return)
{
	FileHandle *handle = (FileHandle *)method_handle;
	GnomeVFSResult res;
	off_t ret;

	if (handle->is_data) {
		*offset_return = handle->offset;
		return GNOME_VFS_OK;
	}
	
	LOCK_SMB();
	init_auth (NULL);
	ret = smb_context->lseek (smb_context, handle->file, (off_t) 0, SEEK_CUR);
	UNLOCK_SMB();
	if (ret == (off_t) -1) {
		*offset_return = 0;
		res = gnome_vfs_result_from_errno ();
	} else {
		*offset_return = (GnomeVFSFileOffset) ret;
		res = GNOME_VFS_OK;
	}

	return res;
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method,
	   GnomeVFSURI *uri,
	   GnomeVFSContext *context)
{
	char *path;
	int type, err;

	DEBUG_SMB (("do_unlink() %s\n",
				gnome_vfs_uri_to_string (uri, 0)));

	type = smb_uri_type (uri);

	if (type == SMB_URI_ERROR) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	if (type == SMB_URI_WHOLE_NETWORK ||
	    type == SMB_URI_WORKGROUP ||
	    type == SMB_URI_SERVER ||
	    type == SMB_URI_SHARE ||
	    type == SMB_URI_WORKGROUP_LINK ||
	    type == SMB_URI_SERVER_LINK) {
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}

	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);

	LOCK_SMB();
	init_auth (uri);
 again:
	err = smb_context->unlink (smb_context, path);
	if (err < 0 && auth_failed ()) {
		goto again;
	}
	UNLOCK_SMB();
	g_free (path);
	
	if (err < 0) {
		return gnome_vfs_result_from_errno ();
	}
	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *a,
		  GnomeVFSURI *b,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	char *server1;
	char *server2;
	char *path1;
	char *path2;
	char *p1, *p2;

	DEBUG_SMB (("do_check_same_fs()\n"));

	server1 =
		gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (a),
					   NULL);
	server2 =
		gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (b),
					   NULL);
	path1 =
		gnome_vfs_unescape_string (gnome_vfs_uri_get_path (a), NULL);
	path2 =
		gnome_vfs_unescape_string (gnome_vfs_uri_get_path (b), NULL);
                                                    
	if (!server1 || !server2 || !path1 || !path2 ||
	    (strcmp (server1, server2) != 0)) {
		g_free (server1);
		g_free (server2);
		g_free (path1);
		g_free (path2);
		*same_fs_return = FALSE;
		return GNOME_VFS_OK;
	}
                             
	p1 = path1;
	p2 = path2;
	if (*p1 == '/') {
		p1++;
	}
	if (*p2 == '/') {
		p2++;
	}

	/* Make sure both URIs are on the same share: */
	while (*p1 && *p2 && *p1 == *p2 && *p1 != '/') {
		p1++;
		p2++;
	}
	if (*p1 == 0 || *p2 == 0 || *p1 != *p2) {
		*same_fs_return = FALSE;
	} else {
		*same_fs_return = TRUE;
	}
                                            
	g_free (server1);
	g_free (server2);
	g_free (path1);
	g_free (path2);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	GnomeVFSResult res;
	char *old_path, *new_path;
	int err;
	gboolean tried_once;
	int old_type, new_type;
	
	
	DEBUG_SMB (("do_move() %s %s\n",
				gnome_vfs_uri_to_string (old_uri, 0),
				gnome_vfs_uri_to_string (new_uri, 0)));

	old_type = smb_uri_type (old_uri);
	new_type = smb_uri_type (new_uri);

	if (old_type != SMB_URI_SHARE_FILE ||
	    new_type != SMB_URI_SHARE_FILE) {
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}

	/* Transform the URI into a completely unescaped string */
	old_path = gnome_vfs_uri_to_string (old_uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);
	new_path = gnome_vfs_uri_to_string (new_uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);

	tried_once = FALSE;
 retry:
	res = GNOME_VFS_OK;
	LOCK_SMB();
	init_auth (old_uri);
 again:
	err = smb_context->rename (smb_context, old_path,
				   smb_context, new_path);
	if (err < 0 && auth_failed ()) {
		goto again;
	}
	UNLOCK_SMB();
	if (err < 0) {
		err = errno;
		if (err == EXDEV) {
			res = GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM;
		} else if (err == EEXIST && force_replace != FALSE) {
			/* If the target exists and force_replace is TRUE */
			LOCK_SMB();
			init_auth (new_uri);
		again2:
			err = smb_context->unlink (smb_context, new_path);
			if (err < 0 && auth_failed ()) {
				goto again2;
			}
			UNLOCK_SMB();
			if (err < 0) {
				res = gnome_vfs_result_from_errno ();
			} else {
				if (!tried_once) {
					tried_once = TRUE;
					goto retry;
				}
				res = GNOME_VFS_ERROR_FILE_EXISTS;
			}
		} else {
			res = gnome_vfs_result_from_errno ();
		}
	}

	g_free (old_path);
	g_free (new_path);

	return res;
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)

{
	DEBUG_SMB(("do_truncate_handle\n"));
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   guint perm,
		   GnomeVFSContext *context)
{
	char *path;
	int err, type;

	type = smb_uri_type (uri);

	if (type == SMB_URI_ERROR) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	if (type == SMB_URI_WHOLE_NETWORK ||
	    type == SMB_URI_WORKGROUP ||
	    type == SMB_URI_SERVER ||
	    type == SMB_URI_SHARE ||
	    type == SMB_URI_WORKGROUP_LINK ||
	    type == SMB_URI_SERVER_LINK) {
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}

	/* Transform the URI into a completely unescaped string */
	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);

	LOCK_SMB();
	init_auth (uri);
 again:
	err = smb_context->mkdir (smb_context, path, perm);
	if (err < 0 && auth_failed ()) {
		goto again;
	}
	UNLOCK_SMB();
	g_free (path);

	if (err < 0) {
		return gnome_vfs_result_from_errno ();
	}

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	char *path;
	int err, type;

	type = smb_uri_type (uri);

	if (type == SMB_URI_ERROR) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	if (type == SMB_URI_WHOLE_NETWORK ||
	    type == SMB_URI_WORKGROUP ||
	    type == SMB_URI_SERVER ||
	    type == SMB_URI_SHARE ||
	    type == SMB_URI_WORKGROUP_LINK ||
	    type == SMB_URI_SERVER_LINK) {
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}

	/* Transform the URI into a completely unescaped string */
	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);

	LOCK_SMB();
	init_auth (uri);
 again:
	err = smb_context->rmdir (smb_context, path);
	if (err < 0 && auth_failed ()) {
		goto again;
	}
	UNLOCK_SMB();
	g_free (path);

	if (err < 0) {
		return gnome_vfs_result_from_errno ();
	}

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{
	char *path;
	int err, type;
	GnomeVFSResult res;

	DEBUG_SMB (("do_set_file_info: mask %x\n", mask));

	type = smb_uri_type (uri);

	if (type == SMB_URI_ERROR) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	if (type == SMB_URI_WHOLE_NETWORK ||
	    type == SMB_URI_WORKGROUP ||
	    type == SMB_URI_SERVER ||
	    type == SMB_URI_SHARE ||
	    type == SMB_URI_WORKGROUP_LINK ||
	    type == SMB_URI_SERVER_LINK) {
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}

	path = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);
	
	if (mask & GNOME_VFS_SET_FILE_INFO_NAME) {
		GnomeVFSURI *parent, *new_uri;
		char *new_path;

		parent = gnome_vfs_uri_get_parent (uri);
		new_uri = gnome_vfs_uri_append_file_name (parent, info->name);
		gnome_vfs_uri_unref (parent);
		new_path = gnome_vfs_uri_to_string (new_uri, GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD);
		gnome_vfs_uri_unref (new_uri);


		LOCK_SMB();
		init_auth (uri);
	again:
		err = smb_context->rename (smb_context, path,
					   smb_context, new_path);
		if (err < 0 && auth_failed ()) {
			goto again;
		}
		UNLOCK_SMB();

		res = GNOME_VFS_OK;
		if (err < 0) {
			if (errno == EXDEV) {
				res = GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM;
			} else {
				res = gnome_vfs_result_from_errno ();
			}
		}
		
		g_free (path);
		path = new_path;

		if (res != GNOME_VFS_OK) {
			g_free (path);
			return res;
		}
	}

	if (gnome_vfs_context_check_cancellation (context)) {
		g_free (path);
		return GNOME_VFS_ERROR_CANCELLED;
	}

	if (mask & GNOME_VFS_SET_FILE_INFO_PERMISSIONS) {
		g_free (path);
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

	if (mask & GNOME_VFS_SET_FILE_INFO_OWNER) {
		g_free (path);
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}
	
	if (mask & GNOME_VFS_SET_FILE_INFO_TIME) {
		g_free (path);
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

	g_free (path);
	return GNOME_VFS_OK;
}


static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	do_seek,
	do_tell,
	do_truncate_handle,
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	do_make_directory,
	do_remove_directory,
	do_move,
	do_unlink,
	do_check_same_fs,
	do_set_file_info,
	NULL, /* do_truncate */
	NULL, /* do_find_directory */
	NULL  /* do_create_symbolic_link */
};

GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
	smb_lock = g_mutex_new ();

	DEBUG_SMB (("<-- smb module init called -->\n"));

	if (try_init ()) {
		return &method;
	} else {
		return NULL;
	}
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	LOCK_SMB();
	if (smb_context != NULL) {
		smbc_free_context (smb_context, 1);
		smb_context = NULL;
	}
	UNLOCK_SMB();

	g_hash_table_destroy (server_cache);
	g_hash_table_destroy (workgroups);
	g_hash_table_destroy (default_user_hashtable);
	
	g_mutex_free (smb_lock);

	DEBUG_SMB (("<-- smb module shutdown called -->\n"));
}

