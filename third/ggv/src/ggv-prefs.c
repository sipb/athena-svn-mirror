/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-prefs.c: GGV preferences
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#include <config.h>

#include "ggv-prefs.h"
#include "ggvutils.h"
#include "gsdefaults.h"

static GConfClient *gconf_client = NULL;

gchar    *ggv_print_cmd = NULL;
gboolean  ggv_save_geometry = FALSE;
gint      ggv_default_width = DEFAULT_WINDOW_WIDTH;
gint      ggv_default_height = DEFAULT_WINDOW_HEIGHT;

gboolean ggv_panel = TRUE, ggv_menubar = TRUE, ggv_toolbar = TRUE;
gboolean ggv_statusbar = TRUE, ggv_autojump = TRUE, ggv_pageflip = FALSE;
GgvToolbarStyle ggv_toolbar_style = GGV_TOOLBAR_STYLE_DEFAULT;
gboolean ggv_right_panel = FALSE;

gint ggv_unit_index = 0;

void
ggv_prefs_load()
{
        gtk_gs_defaults_load();

        ggv_prefs_gconf_client();

        if(ggv_print_cmd)
                g_free(ggv_print_cmd);
        ggv_print_cmd = gconf_client_get_string(gconf_client, "/apps/ggv/printing/command",
                                                NULL);
        if(!ggv_print_cmd)
                ggv_print_cmd = g_strdup(LPR_PATH);

        ggv_unit_index = gconf_client_get_int(gconf_client, "/apps/ggv/coordinates/units",
                                              NULL);

        /* read ggv widget defaults */
        ggv_panel = gconf_client_get_bool(gconf_client,
                                         "/apps/ggv/layout/showpanel",
                                         NULL);
	ggv_toolbar = gconf_client_get_bool(gconf_client,
                                           "/apps/ggv/layout/showtoolbar",
                                           NULL);
        ggv_toolbar_style = gconf_client_get_int(gconf_client,
                                                 "/apps/ggv/layout/toolbarstyle",
                                                 NULL);
        ggv_menubar = gconf_client_get_bool(gconf_client,
                                           "/apps/ggv/layout/showmenubar",
                                           NULL);
	ggv_statusbar = gconf_client_get_bool(gconf_client,
                                              "/apps/ggv/layout/showstatusbar",
                                              NULL);
        ggv_autojump = gconf_client_get_bool(gconf_client,
                                              "/apps/ggv/control/autojump",
                                              NULL);
        ggv_pageflip = gconf_client_get_bool(gconf_client,
                                             "/apps/ggv/control/pageflip",
                                             NULL);
        ggv_right_panel = gconf_client_get_bool(gconf_client,
                                               "/apps/ggv/layout/rightpanel",
                                               NULL);
        /* Get geometry */
        ggv_save_geometry = gconf_client_get_bool
                (gconf_client, "/apps/ggv/layout/savegeometry", NULL);
        if((ggv_default_width = gconf_client_get_int
            (gconf_client, "/apps/ggv/layout/windowwidth", NULL)) == 0)
                ggv_default_width = DEFAULT_WINDOW_WIDTH;
        if((ggv_default_height = gconf_client_get_int
            (gconf_client, "/apps/ggv/layout/windowheight", NULL)) == 0)
                ggv_default_height = DEFAULT_WINDOW_HEIGHT;
}

void
ggv_prefs_save()
{
        ggv_prefs_gconf_client();

        gconf_client_set_string(gconf_client, "/apps/ggv/printing/command",
                                ggv_print_cmd, NULL);
        gconf_client_set_int(gconf_client, "/apps/ggv/coordinates/units",
                             ggv_unit_index, NULL);
        gconf_client_set_bool(gconf_client, "/apps/ggv/layout/showpanel", 
                              ggv_panel, NULL);
	gconf_client_set_bool(gconf_client, "/apps/ggv/layout/showtoolbar",
                              ggv_toolbar, NULL);
        gconf_client_set_int(gconf_client, "/apps/ggv/layout/toolbarstyle",
                             ggv_toolbar_style, NULL);
        gconf_client_set_bool(gconf_client, "/apps/ggv/layout/showmenubar",
                              ggv_menubar, NULL);
	gconf_client_set_bool(gconf_client, "/apps/ggv/layout/showstatusbar",
                              ggv_statusbar, NULL);
        gconf_client_set_bool(gconf_client, "/apps/ggv/layout/savegeometry",
                              ggv_save_geometry, NULL);
        gconf_client_set_bool(gconf_client, "/apps/ggv/control/autojump",
                              ggv_autojump, NULL);
        gconf_client_set_bool(gconf_client, "/apps/ggv/control/pageflip",
                              ggv_pageflip, NULL);
        gconf_client_set_bool(gconf_client, "/apps/ggv/layout/rightpanel",
                              ggv_right_panel, NULL);
        if(ggv_save_geometry) {
                gconf_client_set_int(gconf_client, "/apps/ggv/layout/windowwidth",
                                     ggv_default_width, NULL);
                gconf_client_set_int(gconf_client, "/apps/ggv/layout/windowheight",
                                     ggv_default_height, NULL);
        }
}

static void
ggv_prefs_changed(GConfClient* client, guint cnxn_id,
                  GConfEntry *entry, gpointer user_data)
{
        if(!strcmp(entry->key, "/apps/ggv/printing/command")) {
                if(ggv_print_cmd)
                        g_free(ggv_print_cmd);
                ggv_print_cmd = g_strdup(gconf_value_get_string(entry->value));
                if(!ggv_print_cmd)
                        ggv_print_cmd = g_strdup(LPR_PATH " %s");
        }
        else if(!strcmp(entry->key, "/apps/ggv/coordinates/units")) {
                ggv_unit_index = gconf_client_get_int(gconf_client, "/apps/ggv/coordinates/units",
                                                      NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showpanel")) {
                ggv_panel = gconf_client_get_bool(gconf_client,
                                                  "/apps/ggv/layout/showpanel",
                                                  NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showtoolbar")) {
                ggv_toolbar = gconf_client_get_bool(gconf_client,
                                                    "/apps/ggv/layout/showtoolbar",
                                                    NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showstatusbar")) {
                ggv_statusbar = gconf_client_get_bool(gconf_client,
                                                    "/apps/ggv/layout/showstatusbar",
                                                    NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/toolbarstyle")) {
                ggv_toolbar_style = gconf_client_get_int(gconf_client,
                                                         "/apps/ggv/layout/toolbarstyle",
                                                         NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showmenubar")) {
                ggv_menubar = gconf_client_get_bool(gconf_client,
                                                    "/apps/ggv/layout/showmenubar",
                                                    NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/control/autojump")) {
                ggv_autojump = gconf_client_get_bool(gconf_client,
                                                     "/apps/ggv/control/autojump",
                                                     NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/control/pageflip")) {
                ggv_pageflip = gconf_client_get_bool(gconf_client,
                                                     "/apps/ggv/control/pageflip",
                                                     NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/rightpanel")) {
                ggv_right_panel = gconf_client_get_bool(gconf_client,
                                                       "/apps/ggv/layout/rightpanel",
                                                       NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/savegeometry")) {
                ggv_save_geometry = gconf_client_get_bool
                        (gconf_client, "/apps/ggv/layout/savegeometry", NULL);
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/windowwidth")) {
                if((ggv_default_width = gconf_client_get_int
                    (gconf_client, "/apps/ggv/layout/windowwidth", NULL)) == 0)
                        ggv_default_width = DEFAULT_WINDOW_WIDTH;
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/windowheight")) {
                if((ggv_default_height = gconf_client_get_int
                    (gconf_client, "/apps/ggv/layout/windowheight", NULL)) == 0)
                        ggv_default_height = DEFAULT_WINDOW_HEIGHT;
        }
}

GConfClient *
ggv_prefs_gconf_client()
{
        if(!gconf_client) {
                g_assert(gconf_is_initialized());
                gconf_client = gconf_client_get_default();
                g_assert(gconf_client != NULL);
                gconf_client_add_dir(gconf_client, "/apps/ggv",
                                     GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);
                gconf_client_notify_add(gconf_client, "/apps/ggv",
                                        (GConfClientNotifyFunc)ggv_prefs_changed,
                                        NULL, NULL, NULL);
        }

        return gconf_client;
}
