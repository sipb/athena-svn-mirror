/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * main.c: the ggv shell
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#include <config.h>

#include <gnome.h>
#include <bonobo-activation/bonobo-activation.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>
#include <bonobo/bonobo-ui-main.h>

#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "ggv-window.h"
#include "ggv-prefs.h"
#include "ggvutils.h"

#define BONOBO_DEBUG

static gint win_count = 0;
static gchar *win_geometry = NULL;
static gboolean read_stdin = FALSE;
static gchar *filename_stdin;

static void
sig_handler(int signum)
{
        ggv_window_destroy_all();
        bonobo_main_quit();
}

static GgvWindow *
create_window(const gchar *filename)
{
        GgvWindow *win = GGV_WINDOW(ggv_window_new());

        if(win == NULL) {
                g_warning("Failed to create a new window.");
                return NULL;
        }

        if(filename)
                ggv_window_load(win, filename);

        if(win_geometry) {
                if(!gtk_window_parse_geometry(GTK_WINDOW(win), win_geometry))
                        g_warning(_("Invalid geometry string \"%s\"\n"),
                                  win_geometry);
                win_geometry = NULL;
        }

        gtk_widget_show(GTK_WIDGET(win));

        return win;
}

static gboolean
create_windows_from_cmdline(gpointer data)
{
        GgvWindow *win;
        const gchar **files = NULL;
        poptContext *ctx;
        int i;
        gchar *uri;

        ctx = (poptContext *)data;
        if(*ctx)
                files = poptGetArgs(*ctx);

        if(files)
                for(i = 0; files[i]; i++) {
                        if(strchr(files[i], ':') != NULL) {
                                /* this seems like an URI */
                                create_window(files[i]);
                        }
                        else {
                                /* this looks like an ordinary file name */
                                uri = ggv_filename_to_uri(files[i]);
                                create_window(uri);
                                g_free(uri);
                        }
                }

        if(filename_stdin) {
                create_window(filename_stdin);
        }
        else if(!files && win_count == 0)
                win_count = 1;

        while(win_count > 0) {
                create_window(NULL);
                win_count--;
        }

        if(ggv_get_window_list() == NULL) {
                g_warning("No windows could be opened. Exiting...");
                bonobo_main_quit();
        }

        return FALSE;
}

static void
client_die(GnomeClient *client, gpointer data)
{
        bonobo_main_quit ();
}

static gint
save_session(GnomeClient        *client,
             gint                phase,
             GnomeRestartStyle   save_style,
             gint                shutdown,
             GnomeInteractStyle  interact_style,
             gint                fast,
             gpointer            client_data)
{
	gchar *argv[128];
	gint wnc = 0, argc;
        const GList *node;
        gchar *win_opt = NULL;
        GgvWindow *win;

	argv[0] = (gchar *)client_data;
        argc = 1;
        node = ggv_get_window_list();
        while(node && argc < 128) {
                win = GGV_WINDOW(node->data);
                if(win->filename) {
                        if(!filename_stdin ||
                           !strcmp(win->filename, filename_stdin))
                                argv[argc++] = win->filename;
                        else
                                wnc++;
                }
                else
                        wnc++;
                node = node->next;
        }
	if(wnc > 0 && argc < 128) {
                win_opt = g_strdup_printf("--windows=%d", wnc);
                argv[argc++] = win_opt;
        }
        gnome_client_set_clone_command(client, argc, argv);
        gnome_client_set_restart_command(client, argc, argv);
        if(win_opt)
                g_free(win_opt);

	return TRUE;
}

int
main(int argc, char **argv)
{
	CORBA_Environment ev;
	GError *error;
	poptContext ctx;
	GValue value = { 0, };
	GnomeProgram *program;
        GnomeClient *client;
        int i, j;

	static struct poptOption options[] = {
		{ "windows", 'w', POPT_ARG_INT, &win_count, 0, 
		  N_("Specify the number of empty windows to open."), 
		  N_("Number of empty windows") },
                { "geometry", '\0', POPT_ARG_STRING, &win_geometry, 0,
                  N_("X geometry specification (see \"X\" man page)."),
                  N_("GEOMETRY") },
		{ NULL, '\0', 0, NULL, 0 }
	};

	bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

        signal(SIGTERM, sig_handler);
        signal(SIGINT, sig_handler);

        /* see if there is a dash ("-") argument. if so, read PS from
           stdin. */
        for(i = 0; i < argc; i++) {
                if(!strcmp(argv[i], "-")) {
                        read_stdin = TRUE;
                        for(j = i; j < argc - 1; j++)
                                argv[j] = argv[j + 1];
                        argc--;
                        argv[j] = NULL;
                }
        }

	program = gnome_program_init("ggv", VERSION,
                                     LIBGNOMEUI_MODULE,
                                     argc, argv,
                                     GNOME_PARAM_POPT_TABLE, options,
                                     GNOME_PARAM_APP_DATADIR, DATADIR, NULL);

	CORBA_exception_init(&ev);

        g_value_init(&value, G_TYPE_POINTER);
	g_object_get_property(G_OBJECT(program), GNOME_PARAM_POPT_CONTEXT,
                              &value);
	ctx = g_value_get_pointer(&value);
	g_value_unset(&value);

	error = NULL;
	if (gconf_init(argc, argv, &error) == FALSE) {
		g_assert(error != NULL);
		g_message("GConf init failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}

	if (bonobo_ui_init ("Gnome Ghostview", VERSION, &argc, argv) == FALSE)
		g_error (_("Could not initialize Bonobo!\n"));

        client = gnome_master_client();

	g_signal_connect (client, "save_yourself", G_CALLBACK (save_session), argv[0]);
	g_signal_connect (client, "die", G_CALLBACK (client_die), NULL);

        gtk_idle_add (create_windows_from_cmdline, &ctx);

	gnome_window_icon_set_default_from_file (GNOMEICONDIR "/gnome-ghostview.png");

        ggv_prefs_load();

        if(read_stdin) {
                FILE *f;
                int c, fd;

                filename_stdin = g_strconcat (g_get_tmp_dir(),
                                              "/ggvXXXXXX", NULL);
                if((fd = mkstemp(filename_stdin)) < 0) {
                        g_free(filename_stdin);
                        filename_stdin = NULL;
                        read_stdin = FALSE;
                }
                else {
                        f = fdopen(fd, "w");
                        while((c = fgetc(stdin)) != EOF)
                                fputc(c, f);
                        fclose(f);
                }
        }

	bonobo_main ();

#ifdef BONOBO_DEBUG
        bonobo_debug_shutdown ();
#endif /* BONOBO_DEBUG */

        if(filename_stdin) {
                unlink(filename_stdin);
                g_free(filename_stdin);
        }

        ggv_prefs_save();

	CORBA_exception_free (&ev);
	
        return 0;
}
