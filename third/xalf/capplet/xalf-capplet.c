/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * xalf - X application launch feedback
 *
 * GNOME Control Center capplet. 
 *
 * Copyright Peter Åstrand <altic@lysator.liu.se> 2000. GPLV2. 
 *
 * Source is (hopefully) formatted according to the GNU Coding standards. 
 *
 */

#include <config.h>
#include "capplet-widget.h"
#include <X11/Xlib.h>
#include <assert.h>

#include <gdk/gdkx.h>
#include <getopt.h>

#include "gnome.h"

enum { REAL_SETTINGS, OLD_SETTINGS };

static GtkWidget *capplet;
static GtkWidget *invisiblewindow_checkbox;
static GtkWidget *hourglass_checkbox;
static GtkWidget *splashscreen_checkbox;
static GtkWidget *anim_checkbox;
static GtkWidget *mappingmode_checkbox;
static GtkWidget *timeout_spinbutton;
static gboolean wecare = FALSE;

/* Function prototypes */
static void xalf_help (void);
static void xalf_write (int type);
static void xalf_ok (GtkWidget *widget, gpointer data);
static void xalf_try (void);
static void xalf_revert (void);
static void xalf_read (int type);
static void indicator_toggled (GtkWidget *widget, gpointer data);
static void changes_made (GtkWidget *widget, gpointer data);
static void set_sensitive ();
static void xalf_setup (void);


static void
xalf_help (void)
{
    gchar *tmp;

    tmp = gnome_help_file_find_file ("xalf", "xalf.html#CCENTER");
    if (tmp) {
        gnome_help_goto(0, tmp);
        g_free(tmp);
    } else {
        GtkWidget *mbox;
          
        mbox = gnome_message_box_new(_("No help is available yet. Maybe in next version..."),
                                     GNOME_MESSAGE_BOX_ERROR,
                                     _("Close"), NULL);
        gtk_widget_show(mbox);
    }

}


static void
xalf_write (int type)
{
    /* Write to file. */
    int timeoutval;
    gchar *timeout_string = NULL;
    GString *optionstring = NULL;
        

    optionstring = g_string_new ("");
        
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (invisiblewindow_checkbox)))
        g_string_append (optionstring, "-i ");

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (hourglass_checkbox)))
        g_string_append (optionstring, "-c ");

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (splashscreen_checkbox)))
        g_string_append (optionstring, "-s ");

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (anim_checkbox)))
        g_string_append (optionstring, "-a ");
            
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (mappingmode_checkbox)))
        g_string_append (optionstring, "-m ");
        
    timeoutval = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (timeout_spinbutton));

    if (timeoutval != DEFAULT_TIMEOUT) {
        timeout_string = g_strdup_printf ("-t %d ", timeoutval);
        g_string_append (optionstring, timeout_string);
        g_free (timeout_string);
    }

    if (type == REAL_SETTINGS) {
        gnome_config_set_bool ("/xalf/settings/enabled", 
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (invisiblewindow_checkbox)) ||
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (hourglass_checkbox)) ||
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (splashscreen_checkbox)) ||
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (anim_checkbox)));

        gnome_config_set_string ("/xalf/settings/options", optionstring->str);
    }
    
    if (type == OLD_SETTINGS) {
        gnome_config_set_bool ("/xalf/old_settings/enabled", 
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (invisiblewindow_checkbox)) ||
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (hourglass_checkbox)) ||
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (splashscreen_checkbox)) ||
                               gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (anim_checkbox)));

        gnome_config_set_string ("/xalf/old_settings/options", 
                                 optionstring->str);
    }

    gnome_config_sync ();
}


static void
xalf_ok (GtkWidget *widget, gpointer data)
{
    /* Write to file. This cannot be reverted. */
    xalf_write(REAL_SETTINGS);
    xalf_write(OLD_SETTINGS);
}


static void
xalf_try (void)
{
    xalf_write (REAL_SETTINGS);
}


static void
xalf_revert (void)
{
    wecare = FALSE;
    xalf_read (OLD_SETTINGS);
    set_sensitive();
    wecare = TRUE;
}


static void
xalf_read (int type)
{
    char **options_argv, **real_argv;
    int options_argc, real_argc;
    int i, timeouttime;
    int optchar;
    char *endptr;
    int option_index = 0;
    static struct option long_options[] =
    {
        { "timeout", 1, 0, 't' },
        { "noxalf", 0, 0, 'n' },
        { "mappingmode", 0, 0, 'm' },
        { "invisiblewindow", 0, 0, 'i' },
        { "splash", 0, 0, 's' },
        { "cursor", 0, 0, 'c' },
        { "anim", 0, 0, 'a' },
        { "title",  1, 0, 'l' },
        { 0, 0, 0, 0 }
    };
    char dummy[] = "xalf";


    /* Disable checkboxes */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (invisiblewindow_checkbox), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (hourglass_checkbox), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (splashscreen_checkbox), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (anim_checkbox), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mappingmode_checkbox), FALSE);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (timeout_spinbutton), DEFAULT_TIMEOUT);
    
    if (type == REAL_SETTINGS)
        gnome_config_get_vector ("/xalf/settings/options=-c", 
                                 &real_argc, &real_argv);
    if (type == OLD_SETTINGS) 
        gnome_config_get_vector ("/xalf/old_settings/options=-c", 
                                 &real_argc, &real_argv);
    
    /* Did something fail? */
    if ( (real_argc < 1) || !real_argv || !real_argv[0] || !(*real_argv[0]))
        return;
    
    /* Getopt starts reading at argv[1]. We want real_argv[0] also, 
       so put in an dummy argument in the front. */
    /* One extra argument, and a trailing NULL */
    options_argv = g_malloc ( (real_argc + 2) * sizeof (char*));
    options_argv[0] = dummy;
    for (i = 0; i <= real_argc ; i++) 
        options_argv[i+1] = real_argv[i];
    options_argc = real_argc + 1;

    /* Restore, in case getopt is already initialized */
    optind = 0;
    
    while (1)
        {
	    optchar = getopt_long (options_argc, options_argv, "t:misca",
				   long_options, &option_index);

	    if (optchar == -1)
                break;

	    switch (optchar)
		{
		case 't':
                    timeouttime = (unsigned) strtol(optarg, &endptr, 0); 
                    if (*endptr)  { 
                        fprintf (stderr, "xalf-capplet: invalid timeout, using default of %d\n", 
                                 DEFAULT_TIMEOUT); 
                        timeouttime = DEFAULT_TIMEOUT;
                    }
                    gtk_spin_button_set_value (GTK_SPIN_BUTTON (timeout_spinbutton), timeouttime);
		    break;

		case 'm':
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mappingmode_checkbox), TRUE);
                    break;

		case 'i':
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (invisiblewindow_checkbox), TRUE);
                    break;
                        
		case 's':
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (splashscreen_checkbox), TRUE);
                    break;

		case 'c':
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (hourglass_checkbox), TRUE);
                    break;

		case 'a':
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (anim_checkbox), TRUE);
                    break;

		case '?':
                    break;
                        
		default:
                    ;
		}
        }
    if (optind < options_argc)
        {
            printf ("warning: unknown options");
            while (optind < options_argc)
                printf ("%s ", options_argv[optind++]);
            printf ("\n");
        }
    
    /* Free read array */ 
    g_strfreev (real_argv);
    g_free (options_argv);
    
}


/* Run when some indicator toggle is changed */
static void
indicator_toggled (GtkWidget *widget, gpointer data)
{
    if (wecare)
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
    
    set_sensitive();
}


/* Some other element was changed */
static void
changes_made (GtkWidget *widget, gpointer data)
{
    if (wecare)
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}


static void
set_sensitive()
{
    int active;

    active = 
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (invisiblewindow_checkbox)) ||
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (hourglass_checkbox)) ||
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (splashscreen_checkbox)) ||
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (anim_checkbox));

    gtk_widget_set_sensitive (mappingmode_checkbox, active);
    gtk_widget_set_sensitive (timeout_spinbutton, active);
}


static void
xalf_setup (void)
{
    GtkWidget *vbox_main, *hbox;
    GtkWidget *frame1;
    GtkWidget *vbox2;

    GtkWidget *frame2;
    GtkWidget *vbox3;
    GtkWidget *hbox1;
    GtkWidget *label1;
    GtkWidget *label2;
    GtkObject *timeout_spinbutton_adj;

    gchar *filename;

    vbox_main = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

    /* Icon */
    filename = gnome_pixmap_file ("hourglass-big.png");
    if (filename) {
        GtkWidget *pixmap;

        hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
        pixmap = gnome_pixmap_new_from_file (filename);
        gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox_main), hbox, FALSE, FALSE, 0);
    }   

    capplet = capplet_widget_new();

    /* Indicators frame */
    frame1 = gtk_frame_new (_("Enabled indicators"));
    gtk_box_pack_start (GTK_BOX (vbox_main), frame1, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame1), GNOME_PAD_SMALL);
        
    vbox2 = gtk_vbox_new (TRUE, 0);
    gtk_container_add (GTK_CONTAINER (frame1), vbox2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox2), GNOME_PAD_SMALL);

    invisiblewindow_checkbox = gtk_check_button_new_with_label 
        (_("tasklist (invisible window)"));
    gtk_box_pack_start (GTK_BOX (vbox2), invisiblewindow_checkbox, FALSE, FALSE, 0);

    hourglass_checkbox = gtk_check_button_new_with_label 
        (_("hourglass mouse cursor"));
    gtk_box_pack_start (GTK_BOX (vbox2), hourglass_checkbox, FALSE, FALSE, 0);

    splashscreen_checkbox = gtk_check_button_new_with_label (_("splashscreen"));
    gtk_box_pack_start (GTK_BOX (vbox2), splashscreen_checkbox, FALSE, FALSE, 0);

    anim_checkbox = gtk_check_button_new_with_label (_("animated star"));
    gtk_box_pack_start (GTK_BOX (vbox2), anim_checkbox, FALSE, FALSE, 0);

    /* Miscellaneous frame */
    frame2 = gtk_frame_new (_("Miscellaneous"));
    gtk_box_pack_start (GTK_BOX (vbox_main), frame2, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame2), GNOME_PAD_SMALL);

    vbox3 = gtk_vbox_new (TRUE, 0);
    gtk_container_add (GTK_CONTAINER (frame2), vbox3);
    gtk_container_set_border_width (GTK_CONTAINER (vbox3), GNOME_PAD_SMALL);

    mappingmode_checkbox = gtk_check_button_new_with_label 
        (_("Do not distinguish between windows (compatibility mode)"));
    gtk_box_pack_start (GTK_BOX (vbox3), mappingmode_checkbox, FALSE, FALSE, 0);
        
    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox3), hbox1, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox1), GNOME_PAD_SMALL);

    label1 = gtk_label_new (_("Timeout (seconds)"));
    gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
        
    timeout_spinbutton_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
    timeout_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (timeout_spinbutton_adj), 1, 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (timeout_spinbutton), DEFAULT_TIMEOUT);
    gtk_box_pack_start (GTK_BOX (hbox1), timeout_spinbutton, FALSE,
                        FALSE, 0);

    label2 = gtk_label_new (_("(default is 20)"));
    gtk_box_pack_start (GTK_BOX (hbox1), label2, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);


    /* Connect signals */ 
    gtk_signal_connect (GTK_OBJECT (capplet), "help",
                        GTK_SIGNAL_FUNC (xalf_help), NULL);
    gtk_signal_connect (GTK_OBJECT (capplet), "try",
                        GTK_SIGNAL_FUNC (xalf_try), NULL);
    gtk_signal_connect (GTK_OBJECT (capplet), "revert",
                        GTK_SIGNAL_FUNC (xalf_revert), NULL);
    gtk_signal_connect (GTK_OBJECT (capplet), "ok",
                        GTK_SIGNAL_FUNC (xalf_ok), NULL);
    gtk_signal_connect (GTK_OBJECT (capplet), "cancel",
                        GTK_SIGNAL_FUNC (xalf_revert), NULL);

    gtk_signal_connect (GTK_OBJECT (invisiblewindow_checkbox), "clicked",
                        GTK_SIGNAL_FUNC (indicator_toggled), (gpointer) 1);
    gtk_signal_connect (GTK_OBJECT (hourglass_checkbox), "clicked",
                        GTK_SIGNAL_FUNC (indicator_toggled), (gpointer) 1);
    gtk_signal_connect (GTK_OBJECT (splashscreen_checkbox), "clicked",
                        GTK_SIGNAL_FUNC (indicator_toggled), (gpointer) 1);
    gtk_signal_connect (GTK_OBJECT (anim_checkbox), "clicked",
                        GTK_SIGNAL_FUNC (indicator_toggled), (gpointer) 1);
    gtk_signal_connect (GTK_OBJECT (mappingmode_checkbox), "clicked",
                        GTK_SIGNAL_FUNC (changes_made), (gpointer) 1);
    gtk_signal_connect (GTK_OBJECT (timeout_spinbutton), "changed",
                        GTK_SIGNAL_FUNC (changes_made), (gpointer) 1);

    /* Done */
    gtk_container_add (GTK_CONTAINER (capplet), vbox_main);
    gtk_widget_show_all (capplet);

}


int
main (int argc, char **argv)
{
    GnomeClient *client = NULL;
    GnomeClientFlags flags;
    int init_results;

    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    init_results = gnome_capplet_init("xalf-capplet", VERSION,
                                      argc, argv, NULL, 0, NULL);

    if (init_results < 0) {
        g_warning (_("an initialization error occurred while "
                     "starting 'xalf-capplet'.\n"
                     "aborting...\n"));
        exit (1);
    }

    client = gnome_master_client ();
    flags = gnome_client_get_flags(client);

    if (init_results != 1) {
        xalf_setup ();
        xalf_read (REAL_SETTINGS);
        set_sensitive();
        wecare = TRUE;
        capplet_gtk_main ();
    }
    return 0;
}





