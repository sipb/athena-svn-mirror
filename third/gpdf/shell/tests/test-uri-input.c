/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Unit tests for URI Input
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gpdf-uri-input.h>
#include <gpdf-recent-facade.h>

#define TEST_INITIALIZATION	\
g_type_init ();			\
gnome_vfs_init ();

#include <unit-test.h>

#define TEST_STR(expected, got)						\
	TESTING_MESSAGE ("string value of " #got);			\
	if (strcmp (expected, got) != 0) {				\
		g_print ("\n Expected \"%s\", got \"%s\"", expected, got); \
		FAIL_TEST ();						\
	}								\
	g_print ("Okay!\n");

static GPdfURIInput *uri_in;
static char *requested_uris;

void setup (void);
void tear_down (void);

static void
string_cb (GObject *source, const char *string, char **dest)
{
	if (*dest == NULL) {
		*dest = g_strdup (string);
	} else {
		char *old_dest = *dest;

		*dest = g_strconcat (old_dest, " ", string, NULL);
		g_free (old_dest);
	}
}

void
setup (void)
{
	uri_in = g_object_new (GPDF_TYPE_URI_INPUT, NULL);

	requested_uris = NULL;
	g_signal_connect (G_OBJECT (uri_in),
			  "open_request",
			  G_CALLBACK (string_cb),
			  &requested_uris);
}

void
tear_down (void)
{
        g_object_unref (uri_in);
	g_free (requested_uris);	
}

TEST_BEGIN (GPdfURIInput, setup, tear_down)

TEST_NEW (open_uri_fires_open_request)
{
	const char *uri = "file:///etc/hosts";

	gpdf_uri_input_open_uri (uri_in, uri);
	TEST_STR (uri, requested_uris);
}

TEST_NEW (open_glist_two_uris)
{
	GList *uri_glist = NULL;
	const char *uri = "file:///etc/hosts";

	uri_glist = g_list_prepend (uri_glist, gnome_vfs_uri_new (uri));
	uri_glist = g_list_prepend (uri_glist, gnome_vfs_uri_new (uri));
	gpdf_uri_input_open_uri_glist (uri_in, uri_glist);
	TEST_STR ("file:///etc/hosts file:///etc/hosts", requested_uris);
	gnome_vfs_uri_list_unref (uri_glist);
	g_list_free (uri_glist);
}

TEST_NEW (open_uri_list)
{
	const char *uri_list =
		"file:///etc/hosts\r\n"
		"#a comment just for fun\r\n"
		"file:///etc/fonts/fonts.conf\r\n";
	gpdf_uri_input_open_uri_list (uri_in, uri_list);
	TEST_STR ("file:///etc/hosts file:///etc/fonts/fonts.conf",
		  requested_uris);
}

TEST_NEW (open_shell_arg)
{
	gpdf_uri_input_open_shell_arg (uri_in, "/etc/hosts");
	TEST_STR ("file:///etc/hosts", requested_uris);
}

TEST_NEW (open_shell_arg_relative)
{
	char *current_dir;
	char *uri_in_current_dir;

	gpdf_uri_input_open_shell_arg (uri_in, "Makefile");
	current_dir = g_get_current_dir ();
	uri_in_current_dir = g_strdup_printf ("file://%s/Makefile", current_dir);
	TEST_STR (uri_in_current_dir, requested_uris);
	g_free (current_dir);
	g_free (uri_in_current_dir);
}

TEST_NEW (open_uri_adds_to_recent)
{
	const char *uri = "file:///etc/hosts";
	char *added_uri;
        MockRecentFacade *mock_recent;
        
        mock_recent = g_object_new (TYPE_MOCK_RECENT_FACADE, NULL);
        gpdf_uri_input_set_recent_facade (uri_in,
					  GPDF_RECENT_FACADE (mock_recent));
        gpdf_uri_input_open_uri (uri_in, uri);
	added_uri = g_object_get_data (G_OBJECT (mock_recent), "added_uri");
	TEST_STR (uri, added_uri);
	g_object_unref (G_OBJECT (mock_recent));
}

TEST_END ()
