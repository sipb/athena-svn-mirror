/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-init.c - Initialization for the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

#include <config.h>
#include "gnome-vfs-init.h"

#include "gnome-vfs-ssl-private.h"
#include "gnome-vfs-mime.h"

#include "gnome-vfs-configuration.h"
#include "gnome-vfs-i18n.h"
#include "gnome-vfs-method.h"
#include "gnome-vfs-process.h"
#include "gnome-vfs-utils.h"

#include "gnome-vfs-async-job-map.h"
#include "gnome-vfs-thread-pool.h"
#include "gnome-vfs-job-queue.h"

#include <errno.h>
#include <bonobo-activation/bonobo-activation.h>
#include <glib/gmessages.h>
#include <glib/gfileutils.h>
#include <libgnomevfs/gnome-vfs-job-slave.h>
#include <sys/stat.h>
#include <sys/types.h>

static gboolean vfs_already_initialized = FALSE;
G_LOCK_DEFINE_STATIC (vfs_already_initialized);

static GPrivate * private_is_primary_thread;

static gboolean
ensure_dot_gnome_exists (void)
{
	gboolean retval = TRUE;
	gchar *dirname;

	dirname = g_build_filename (g_get_home_dir (), ".gnome", NULL);

	if (!g_file_test (dirname, G_FILE_TEST_EXISTS)) {
		if (mkdir (dirname, S_IRWXU) != 0) {
			g_warning ("Unable to create ~/.gnome directory: %s",
				   g_strerror (errno));
			retval = FALSE;
		}
	} else if (!g_file_test (dirname, G_FILE_TEST_IS_DIR)) {
		g_warning ("Error: ~/.gnome must be a directory.");
		retval = FALSE;
	}

	g_free (dirname);
	return retval;
}

static void
gnome_vfs_pthread_init (void)
{
	private_is_primary_thread = g_private_new (NULL);
	g_private_set (private_is_primary_thread, GUINT_TO_POINTER (1));
	
	gnome_vfs_module_callback_private_init ();
	
	gnome_vfs_async_job_map_init ();
	gnome_vfs_thread_pool_init ();
	gnome_vfs_job_queue_init ();
}

gboolean 
gnome_vfs_init (void)
{
	gboolean retval;
	char *bogus_argv[2] = { "dummy", NULL };

	if (!ensure_dot_gnome_exists ())
		return FALSE;

	if (!g_thread_supported ())
		g_thread_init (NULL);

	G_LOCK (vfs_already_initialized);

	if (!vfs_already_initialized) {
#ifdef ENABLE_NLS
		bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif   
		gnome_vfs_pthread_init ();

		if (bonobo_activation_orb_get() == NULL) {
			bonobo_activation_init (0, bogus_argv);
		}

		gnome_vfs_ssl_init ();

		retval = gnome_vfs_method_init ();

		if (retval) {
			retval = gnome_vfs_process_init ();
		}
		if (retval) {
			retval = gnome_vfs_configuration_init ();
		}
		if (retval) {
			signal (SIGPIPE, SIG_IGN);
		}
	} else {
		retval = TRUE;	/* Who cares after all.  */
	}

	vfs_already_initialized = TRUE;
	G_UNLOCK (vfs_already_initialized);

	return retval;
}


gboolean
gnome_vfs_initialized (void)
{
	gboolean out;

	G_LOCK (vfs_already_initialized);
	out = vfs_already_initialized;
	G_UNLOCK (vfs_already_initialized);
	return out;
}

void
gnome_vfs_shutdown (void)
{
	gnome_vfs_thread_backend_shutdown ();
	gnome_vfs_mime_shutdown ();
}

void
gnome_vfs_loadinit (gpointer app, gpointer modinfo)
{
}

void
gnome_vfs_preinit (gpointer app, gpointer modinfo)
{
}

void
gnome_vfs_postinit (gpointer app, gpointer modinfo)
{
	G_LOCK (vfs_already_initialized);

	gnome_vfs_pthread_init ();

	gnome_vfs_method_init ();
	gnome_vfs_process_init ();
	gnome_vfs_configuration_init ();

	signal (SIGPIPE, SIG_IGN);

	vfs_already_initialized = TRUE;
	G_UNLOCK (vfs_already_initialized);
}

gboolean
gnome_vfs_is_primary_thread (void)
{
	if (g_thread_supported()) {
		return GPOINTER_TO_UINT(g_private_get (private_is_primary_thread)) == 1;
	} else {
		return TRUE;
	}
}
