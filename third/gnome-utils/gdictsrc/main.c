/* $Id: main.c,v 1.1.1.4 2003-01-29 20:33:38 ghudson Exp $ */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main program function
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include "dict.h"
#include "gdict-pref.h"
#include "gdict-applet.h"
#include "gdict-app.h"


static gint save_yourself_cb (GnomeClient       *client,
                              gint               phase,
                              GnomeRestartStyle  save_style,
                              gint               shutdown,
                              GnomeInteractStyle interact_style,
                              gint               fast,
                              gpointer           client_data)
{
    gchar *argv[] = {NULL, NULL, NULL, NULL, NULL};
    gchar *word;
    gint argc = 1;

    argv[0] = (gchar *)client_data;

    if ((word = gdict_defbox_get_word(defbox)) != NULL)
        argv[argc++] = word;

    gnome_client_set_restart_command(client, argc, argv);
    gnome_client_set_clone_command(client, 0, NULL);

    return TRUE;
}

static gint client_die_cb (GnomeClient *client, gpointer client_data)
{
	gtk_main_quit ();
}

const char **
get_command_line_args (GnomeProgram *program)
{
	GValue value = { 0, };
	poptContext ctx;

	g_value_init (&value, G_TYPE_POINTER);
	g_object_get_property (G_OBJECT (program), GNOME_PARAM_POPT_CONTEXT, &value);
	ctx = g_value_get_pointer (&value);
	g_value_unset (&value);

	return poptGetArgs (ctx);
}

int main (int argc, char *argv[])
{
    gint i;
    GDictApplet * applet = NULL;
    GnomeClient *client;
    GnomeProgram *program;
    const char **args;

    bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    program = gnome_program_init ("gnome-dictionary",VERSION,
				  LIBGNOMEUI_MODULE,
				  argc, argv,
				  GNOME_PARAM_APP_DATADIR,DATADIR,NULL);

	

    if ((client = gnome_master_client()) != NULL) {
            g_signal_connect (client, "save_yourself",
                              G_CALLBACK (save_yourself_cb),
                              (gpointer) argv[0]);
	    /* FIXME: We should really connect to this, but gnome-session wants to 
	     * remove non-reponsive clients after a timeout - in this case the 
	     * client is busy trying to connect to the dict server
             * g_signal_connect (client, "die", G_CALLBACK (client_die_cb), NULL);
	     */
    }
    
    
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gdict.png");
    gdict_app_create (FALSE);
    gdict_pref_load ();

    gdict_init_context ();
    defbox->context = context;

    args = get_command_line_args (program);

    for (i = 0; args != NULL && args[i] != NULL; i++) {
	    gdict_defbox_lookup(defbox, (char *)args[i]);
	    break;
    }

    gtk_widget_show(gdict_app);
    gtk_main();
    
    dict_context_destroy (context); /* FIXME */

    return 0;
}

