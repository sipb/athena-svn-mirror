/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * xalf - X application launch feedback
 * A wrapper for starting X applications. Provides four indicators:
 *
 * 1. An invisible window, to be shown in pagers like Gnomes tasklist_applet
 * or KDE taskbar. 
 *
 * 2. Generic splashscreen
 *
 * 3. Add hourglass to mouse cursor for root window and Gnome's panel. 
 *
 * 4. Animated star. 
 *
 * Copyright Peter Åstrand <astrand@lysator.liu.se> 2001. GPLV2. 
 *
 * Source is (hopefully) formatted according to the GNU Coding standards. 
 *
 */

#define _GNU_SOURCE

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <getopt.h>
#include <dlfcn.h>
#include <X11/Xmu/WinUtil.h>

#include "hourglass.xpm"
#include "splash.xpm"
#include "hgcursor.h"
#include "hgcursor_mask.h"
/* animation frames */
#include "sp0.xpm"
#include "sp1.xpm"
#include "sp2.xpm"
#include "sp3.xpm"
#include "sp4.xpm"
#include "sp5.xpm"
#include "sp6.xpm"
#include "sp7.xpm"
#include "sp8.xpm"

#undef DEBUG 
/* Uncomment below for debugging */
/* #define DEBUG */

#ifdef DEBUG
#   define DPRINTF(args) fprintf args
#else
#   define DPRINTF(args) 
#endif

#define PID_ENV_NAME "XALF_LAUNCH_PID"
#define SAVED_PRELOAD_NAME "XALF_SAVED_PRELOAD"
#define PRELOAD_LIBRARY LIBDIR"/libxalflaunch.so"
#define USAGE "\
Usage: %s [options] command\n\
options:\n\
   -h, --help               display this help and exit\n\
   -v, --version            output version information and exit\n\
   -t, --timeout n          use a time-out period of n seconds\n\
                            (default is 20)\n\
   -n, --noxalf             do nothing, besides launch application\n\
   -m, --mappingmode        compatibility mode: Do not distinguish between\n\
                            windows. All new mapped windows turns off indicator\n\
   -i, --invisiblewindow    use an invisible window as indicator (default)\n\
                            (for use with Gnome panel, KDE taskbar etc)\n\
   -s, --splash             use splashscreen as indicator\n\
   -c, --cursor             add hourglass to mouse cursor\n\
   -a, --anim               use animated star as indicator\n\
   -l, --title titlestring  Title to show in the tasklist\n"


#define MAINTAINER		"astrand@lysator.liu.se"
#define CANONICAL_NAME          "xalf"


/* Prototypes */
void exit_on_match (Window window);
void monitor_events ();
void create_invisible ();
void create_splash ();
void change_cursor (int launching);
gint redraw_cursor (gpointer data);
void install_sighandlers ();
void remove_sighandlers ();
static GdkWindow* gdk_window_ref_from_xid (Window xwin);
static GdkFilterReturn root_event_monitor (GdkXEvent *gdk_xevent,
                                           GdkEvent *event,
                                           gpointer gdk_root);
char *find_in_path (char *filename);
int is_setid (char *filename);
Window find_window (Display *dpy, Window root, char *wname, char *wclass);
int match_window (Display *dpy, Window w, Atom leader_atom, char *wname, char *wclass);
void restore_cursor ();


/* animation data */
char **xpm_array[9];
#define MAX_FRAMES 9
struct anim_data_struct {
    int active_frame_number;
    GdkPixmap *frames[MAX_FRAMES];
    GdkBitmap *masks[MAX_FRAMES];
    GtkWidget *pixmaps[MAX_FRAMES];
    GtkWidget *windows[MAX_FRAMES];
};
struct anim_data_struct* anim_data;

void init_animation (struct anim_data_struct* anim_data);
gint update_anim (gpointer data);

/* Global data */
/* The atom WM_STATE */
Atom xa_wm_state;
/* The name of this binary */
char *programname;
/* The name of the binary launched */
char *taskname;
/* The title to show in the indicator */
char *title = NULL;
/* The pid of this tracing process */
char launch_pid[22];
/* The current display */
Display *dpy;
/* True if using mouse cursor as indicator */
int cursor_opt = FALSE;
/* GTK timeout tags */
int cursor_timeout_tag = 0;
int exit_timeout_tag = 0;
/* Animated start */
int anim_opt = FALSE;
/* The number of MapEvents in mappingmode to detect before we are done. */
int pending_mapevents = 1;


void 
restore_cursor ()
{
    if (cursor_opt) 
	{
	    gtk_timeout_remove (cursor_timeout_tag);
	    change_cursor (FALSE);
	}
}


gint
timeout_gtk_handler (gpointer data)
{
    /* Ignore SIGUSR1; we're about to exit */
    remove_sighandlers ();
    ((void) fprintf (stderr, "%s: timeout launching %s\n", programname, taskname));
    restore_cursor ();
    gtk_exit (1);

    return FALSE;
}


gint
exit_gtk_handler (gpointer data) 
{      
    gtk_timeout_remove (exit_timeout_tag);
    remove_sighandlers ();
    restore_cursor ();
    gtk_exit (0);

    return FALSE;
}    


void 
mapped_sig_handler (int signo) 
{      
    DPRINTF((stderr, "%s: App is now mapped: %s\n", programname, taskname));
    /* Schedule a GTK call */
    exit_timeout_tag = gtk_timeout_add (50, exit_gtk_handler, NULL);
}    


void 
terminate_sig_handler (int signo) 
{      
    DPRINTF((stderr, "%s: Got termination signal %d\n", programname, signo));
    exit_timeout_tag = gtk_timeout_add (50, exit_gtk_handler, NULL);
}    


void 
set_icon (GdkWindow *window)
{
    static GdkPixmap *w_minipixmap = NULL;
    static GdkBitmap *w_minimask = NULL;
    GdkAtom icon_atom;
    glong data[2];

    w_minipixmap =
	gdk_pixmap_create_from_xpm_d (window,
				      &w_minimask,
				      NULL, 
				      hourglass_xpm);
    
    data[0] = ((GdkPixmapPrivate *)w_minipixmap)->xwindow;
    data[1] = ((GdkPixmapPrivate *)w_minimask)->xwindow;

    icon_atom = gdk_atom_intern ("KWM_WIN_ICON", FALSE);
    gdk_property_change (window, icon_atom, icon_atom,
			 32, GDK_PROP_MODE_REPLACE,
			 (guchar *)data, 2);
}


int 
xalf_error_handler (Display *dpy, XErrorEvent *xerr)
{ 
#ifdef DEBUG
    if (xerr->error_code) 
        {
            char buf[64];
            
            XGetErrorText (dpy, xerr->error_code, buf, 63);
            fprintf (stderr, "X11 error **: %s\n", buf);
            fprintf (stderr, "serial %ld error_code %d request_code %d "\
                     "minor_code %d\n", 
                     xerr->serial, 
                     xerr->error_code, 
                     xerr->request_code, 
                     xerr->minor_code);
        }
#endif

    return 0; 
} 


/* Check if we have to force --mappingmode */
static int 
forced_mappingmode(char **argv) {
    int mappingmode_opt = FALSE;
    
#if !defined(MULTI_PRELOAD) && !defined(RLD_LIST)
    {
        char *preload_env;
        /* If LD_PRELOAD is alread set and this system does not support 
           multiple libs in LD_PRELOAD, use use --mappingmode. */
        preload_env = getenv("LD_PRELOAD");
        if ( (preload_env != NULL) && (*preload_env != '\0') )
            {
                fprintf (stderr, 
                         "%s: LD_PRELOAD is already set. Using --mappingmode\n", 
                         programname);
                mappingmode_opt = TRUE;
            }
    }
#endif
    
    /* Check if preload library is available */
    {
        void *handle = NULL;
        /* Set PID_ENV_NAME to -1. libxalflaunch.so knows about this case
           and refrains from doing anything (like unsetting an 
           original LD_PRELOAD) */
        putenv (PID_ENV_NAME"=-1");
        handle = dlopen (PRELOAD_LIBRARY, RTLD_LAZY);
        if (!handle)
            {
                fprintf (stderr, 
                         "%s: preload library not found. Using --mappingmode\n", 
                         programname);
                mappingmode_opt = TRUE;
            }
        /* Note: We do not close */
        
    }
    /* Check if program is setuid or setgid */
    {
        gchar *abs_name;
        gchar *effective_task_name;
        
        /* Ugly hack. gnome-libs always executes desktop entries through
           /bin/sh -c. This makes the setuid/setgid check fail, with 
           severe consequences: The application won't even start on Solaris, 
           for example. I'm aware of that this is a really ugly solution, 
           but there seems to be no alternative. */
        
        if (!strcmp (argv[optind], "/bin/sh") && !strcmp (argv[optind+1], "-c")) 
            {
                gchar *space_p;
                effective_task_name = g_strdup(argv[optind+2]);
                
                /* Find command in command */
                space_p = strchr (effective_task_name, ' ');
                if (space_p != NULL) 
                    *space_p = '\0';
                
                abs_name = find_in_path (effective_task_name);
            } 
        else 
            {
                effective_task_name = taskname;
                abs_name = find_in_path (taskname);
            }
        
        if (!abs_name)
            {
                fprintf (stderr, "%s: error: couldn't find %s in PATH\n", 
                         programname, effective_task_name);
                restore_cursor ();
                gtk_exit (1);
            }
        
        if (is_setid (abs_name))
            {
                fprintf (stderr, "%s: %s is setuid and/or setgid. Using --mappingmode\n", 
                         programname, abs_name);
                mappingmode_opt = TRUE;
            }
    }

    return mappingmode_opt;
}


static int 
launch_application (int mappingmode_opt, char **argv) 
{
    int pid;
    char *preload_string = NULL;
    char *saved_preload = NULL;
    char *saved_preload_env = NULL;
    
    /* Set up preload_string */
#ifdef RLD_LIST
    saved_preload = getenv(RLD_LIST);
    if (saved_preload != NULL)
        {
            preload_string = g_strconcat (RLD_LIST"=", PRELOAD_LIBRARY,
                                          ":", saved_preload, NULL);
            saved_preload_env = g_strconcat (SAVED_PRELOAD_NAME"=", saved_preload, NULL);
        }
    else
        preload_string = g_strconcat (RLD_LIST"=", PRELOAD_LIBRARY,
                                      ":DEFAULT", NULL);
#else
#ifdef MULTI_PRELOAD
    saved_preload = getenv("LD_PRELOAD");
    if (saved_preload != NULL)
        {
            preload_string = g_strconcat ("LD_PRELOAD=", saved_preload, ":", PRELOAD_LIBRARY, NULL);
            saved_preload_env = g_strconcat (SAVED_PRELOAD_NAME"=", saved_preload, NULL);
        }
    else
        preload_string = g_strconcat ("LD_PRELOAD=", PRELOAD_LIBRARY, NULL);
#else 
    preload_string = g_strconcat ("LD_PRELOAD=", PRELOAD_LIBRARY, NULL);    
#endif /* MULTI_PRELOAD */
#endif /* RLD_LIST */

    /* Make sure that the file descriptor is not passed to the client. */
    if (fcntl (ConnectionNumber (dpy), F_SETFD, 1L) == -1) {
	fprintf (stderr, "%s: warning: one file descriptor unusable for ", programname);
    }
    DPRINTF((stderr, "Close on exec flag: %d\n", fcntl (ConnectionNumber (dpy), F_GETFD)));
    
    /* A string with our pid */
    sprintf (launch_pid, "%ld", (long) getpid ());
    
    /* Spawn application */
    {
	char *pid_string;

	pid_string = g_strconcat (PID_ENV_NAME, "=", launch_pid, NULL);
	switch (pid = fork ())
	    {
	    case -1:
		fprintf (stderr, "%s: error forking\n", programname);
		gtk_exit (1);
	    case 0:
		if (!mappingmode_opt) 
		    {
			putenv (preload_string);
                        if (saved_preload_env)
                            putenv (saved_preload_env);
			putenv (pid_string);
		    }
		execvp (argv[optind], argv+optind);
		fprintf (stderr, "%s: error executing\n", programname);
		kill (atol (launch_pid), SIGUSR1);
		_exit (1);
	    }
    }
    return 0;
}


int 
main (int argc, char **argv)
{
    int arg = 1;
    int noxalf_opt = FALSE;
    int invisiblewindow_opt = FALSE;
    int splash_opt = FALSE; 
    int mappingmode_opt = FALSE;
    unsigned timeouttime = DEFAULT_TIMEOUT;
    char *endptr;
    int optchar;
    guint32 anim_timer = 0;

    /* The name of this binary */
    programname = strrchr (argv[0], '/');
    if (programname) programname++; else programname = argv[0];
    
    while (1)
	{
	    int option_index = 0;
	    static struct option long_options[] =
	    {
		{ "help", 0, 0, 'h' },
		{ "version", 0, 0, 'v' },
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
	    
	    optchar = getopt_long (argc, argv, "+hvt:nmiscal:",
				   long_options, &option_index);
	    if (optchar == -1)
		break;

	    switch (optchar)
		{
		case 'h':
		    fprintf (stdout, USAGE, CANONICAL_NAME);
		    exit (0);
		    break;

		case 'v':
		    fprintf (stdout, "%s version %s\n", CANONICAL_NAME, VERSION);

		    exit (0);
		    break;

		case 't':
		    timeouttime = (unsigned) strtol(optarg, &endptr, 0);
		    if (*endptr || (endptr == argv[arg+1])) 
			{
			    fprintf (stderr, "%s: invalid timeout, using default of %d\n",
				     programname, DEFAULT_TIMEOUT);
			    timeouttime = DEFAULT_TIMEOUT;
			}
		    break;

		case 'l':
		    title = g_strdup (optarg);
		    break;

		case 'n':
		    noxalf_opt = TRUE;
		    break;
		
		case 'm':
		    mappingmode_opt = TRUE;
		    break;

		case 'i':
		    invisiblewindow_opt = TRUE;
                    pending_mapevents++;
		    break;

		case 's':
		    splash_opt = TRUE;
                    pending_mapevents++;
		    break;

		case 'c':
		    cursor_opt = TRUE;
		    break;

		case 'a':
                    anim_opt = TRUE;
                    break;
		    
		case '?':
		    break;

		default:
		    ;
		}
	}
    if (optind >= argc)
	{
	    fprintf (stderr, "%s: too few arguments\n", programname);
	    fprintf (stderr, USAGE, programname);
	    return 1;
	}
    
    if (!invisiblewindow_opt && !splash_opt && !cursor_opt && !anim_opt) 
        cursor_opt = TRUE;
	
    if (noxalf_opt)
	execvp (argv[optind], argv+optind); 

    /* Initialize GTK etc. */
    gtk_init (&argc, &argv); 
    XSetErrorHandler (xalf_error_handler);
    dpy = GDK_DISPLAY ();
    taskname = g_strdup (argv[optind]);	
    
    /**** Listen for events ****/
    if (!mappingmode_opt)
        /* Maybe we need to force mappingmode? */
        mappingmode_opt = forced_mappingmode(argv);
    
    if (mappingmode_opt)
	{
	    GdkWindow *window;
	    XWindowAttributes attribs = { 0, };
	    
	    window = gdk_window_ref_from_xid (GDK_ROOT_WINDOW ());
	    if (!window) 
                fprintf (stderr, "%s: fatal error.\n", programname);
	    gdk_window_add_filter (window, root_event_monitor, window);     
	    /* Set event mask for events on root window */                                
	    XGetWindowAttributes (GDK_DISPLAY (), 
				  GDK_ROOT_WINDOW (), 
				  &attribs); 
	    XSelectInput (GDK_DISPLAY (), 
			  GDK_ROOT_WINDOW (), 
			  attribs.your_event_mask | SubstructureNotifyMask);
	    gdk_flush (); 
	}
    
    /**** Show indicators ****/
    if (!title)
        /* The user didn't supply an title. Use the name of the binary. */
	title = taskname;
    if (invisiblewindow_opt)
	create_invisible ();
    if (splash_opt)
	create_splash ();
    if (anim_opt)
	{
	    anim_data = (struct anim_data_struct *) malloc (sizeof(struct anim_data_struct));
	    if (anim_data == NULL)
		{
		    fprintf(stderr, "%s: fatal: memory allocation error for anim_data\n", 
                            programname);
		    exit(-1);
		}
	    init_animation (anim_data);
	    anim_timer = gtk_timeout_add(75, update_anim, (gpointer) anim_data);
	}
    if (cursor_opt)
	{
	    change_cursor (TRUE);
            /* The cursor may be restored (for example by another Xalf instance),
               so refresh it two times a second. */
	    cursor_timeout_tag = gtk_timeout_add (500, redraw_cursor, NULL);
	}
    
    while (gtk_events_pending ())
	gtk_main_iteration ();
    gdk_flush ();
    
    install_sighandlers ();
    gtk_timeout_add (timeouttime*1000, timeout_gtk_handler, NULL);
    
    /**** Launch application ****/
    launch_application (mappingmode_opt, argv);
    
    gtk_main ();
    
    gtk_exit (0);
    g_free (anim_data);
    
    return (0);
}


/* Create invisible window */
void
create_invisible()
{
    GtkWidget *window;
    char *tasktitle;
	
    tasktitle = g_strconcat ("(", title, ")", NULL);
    /* Create the indicator: An invisible window, to be shown in a pager */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(window), tasktitle); 
    gtk_window_set_policy (GTK_WINDOW(window), FALSE, FALSE, TRUE);         
    gtk_window_set_wmclass (GTK_WINDOW(window), "invisiblewindow", "xalf");     
    gtk_widget_realize (window);
    gdk_window_set_decorations (window->window, 0);
    /* Set a hourglass icon for the indicator via KWM_WIN_ICON */
    set_icon (window->window);
    
    /* Show window */
    gtk_widget_show (window);
}


void
create_splash ()
{
    GtkWidget *dialog;
    GtkWidget *dialog_vbox;
    GtkWidget *pixmap1;
    GtkWidget *dialog_action_area1;
    GtkWidget *label1;
    gchar *labeltext;

    dialog = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (dialog), "Starting...");
    gtk_window_set_policy (GTK_WINDOW (dialog), TRUE, TRUE, FALSE);
    gtk_window_set_wmclass (GTK_WINDOW(dialog), "splash", "xalf");     
    gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    dialog_vbox = GTK_DIALOG (dialog)->vbox;
    gtk_widget_show (dialog_vbox);

    {
	GdkColormap *colormap;
	GdkPixmap *gdkpixmap;
	GdkBitmap *mask;
	colormap = gtk_widget_get_colormap (dialog);
	
	gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask,
							   NULL, splash_xpm);
	pixmap1 = gtk_pixmap_new (gdkpixmap, mask);
	
	gdk_pixmap_unref (gdkpixmap);
	gdk_bitmap_unref (mask);
    }

    gtk_widget_show (pixmap1);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), pixmap1, TRUE, TRUE, 0);

    dialog_action_area1 = GTK_DIALOG (dialog)->action_area;
    gtk_widget_show (dialog_action_area1);
    gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

    labeltext = g_strconcat ("Starting ", title, "...", NULL);
    label1 = gtk_label_new (labeltext);
    
    gtk_widget_show (label1);
    gtk_box_pack_start (GTK_BOX (dialog_action_area1), label1, FALSE, FALSE, 0);

    /* Set a hourglass icon for the indicator via KWM_WIN_ICON */
    gtk_widget_realize (dialog);
    set_icon (dialog->window);
    
    /* Show window */
    gtk_widget_show (dialog);
}



/* Change cursor */
void
change_cursor (int launching)
{

    static GdkCursor *curs = NULL;
    static GdkWindow *panel = NULL;
    GdkPixmap *source, *mask;
    GdkColor gs_white, gs_black;
    GdkColormap *colormap;

    if (launching)
	{
	    if (!curs) /* First call */
		{
		    colormap = gtk_widget_get_default_colormap ();                              
		    gdk_color_white (colormap, &gs_white);                                      
		    gdk_color_black (colormap, &gs_black);
		    
		    source = gdk_bitmap_create_from_data (NULL, hgcursor_bits,
							  hgcursor_width, 
							  hgcursor_height);
		    mask = gdk_bitmap_create_from_data (NULL, hgcursor_mask_bits,
							hgcursor_width, 
							hgcursor_height);
		    curs = gdk_cursor_new_from_pixmap (source, mask, &gs_black, 
						       &gs_white, 8, 8);
		    gdk_pixmap_unref (source);
		    gdk_pixmap_unref (mask);
		    /* Locate panel */
		    {
			Window xpanel;
			xpanel = find_window (dpy, GDK_ROOT_WINDOW (), "panel", "Panel");
			if (xpanel != None)
			    panel = gdk_window_ref_from_xid (xpanel); 
		    }
		}
	}
    else
	{
	    gdk_cursor_destroy (curs);
	    curs = gdk_cursor_new (GDK_LEFT_PTR);
	}
    
    /* Set rootwindow cursor */
    gdk_window_set_cursor (gdk_window_ref_from_xid (GDK_ROOT_WINDOW ()), curs);

    /* Set panel cursor (Note: Intended for Gnome panel, but will probably 
       work with everything that has the name "panel") */
    if (panel != NULL)
	{
	    gdk_window_set_cursor (panel, curs);
	}
}


gint 
redraw_cursor (gpointer data)
{
    change_cursor(TRUE);
    return TRUE;
}


void
install_sighandlers ()
{
    signal (SIGUSR1, mapped_sig_handler);
    signal (SIGTERM, terminate_sig_handler);
    signal (SIGINT, terminate_sig_handler);
    signal (SIGQUIT, terminate_sig_handler);
    signal (SIGCHLD, terminate_sig_handler);
}


void 
remove_sighandlers ()
{
    signal (SIGUSR1, SIG_IGN);
}


static GdkFilterReturn
root_event_monitor (GdkXEvent *gdk_xevent,
		    GdkEvent  *event,
		    gpointer   gdk_root)
{
    XEvent *xevent = gdk_xevent;
    
    switch (xevent->type)
        {
        case MapNotify:
            if (anim_opt) {
                if (matched_star ( ((XMapEvent*)xevent)->window) )
                    /* This came from our own animated star. Ignore. */
                    return GDK_FILTER_CONTINUE;
            }
            DPRINTF((stderr, "Got MapNotify for window %p\n", ((XMapEvent*)xevent)->window));
            if (--pending_mapevents < 1)
                /* All MapEvents detected. We are done. */
                mapped_sig_handler (SIGUSR1);
        default:
            break;
        }
  
    return GDK_FILTER_CONTINUE;
}


static GdkWindow*
gdk_window_ref_from_xid (Window xwin)
{
    GdkWindow *window;

    /* the xid maybe invalid already, in that case we return NULL */
    window = gdk_window_lookup (xwin);
    if (!window)
        window = gdk_window_foreign_new (xwin);
    else
        gdk_window_ref (window);

    return window;
}


char *
find_in_path (char *filename)
{
    gchar *pathenv, *testpath, *foundfile = NULL;
    gchar **chunks;
    int i;
    struct stat statbuf;
    
    /* If an absolut or relative filename are given, return it */
    if ( (filename[0] == '/') || (filename[0] == '.') )
	return filename;

    pathenv = getenv ("PATH");
    
    /* Split path */
    chunks = g_strsplit (pathenv, ":", 0);
    testpath = NULL;
    
    for (i = 0; chunks[i]; i++)
	{
	    g_free (testpath); /* Free string from last loop */
	    testpath = g_strconcat (chunks[i], "/", filename, NULL);

	    if (stat (testpath, &statbuf) < 0)
		continue;      /* Couldn't stat file */
	    
	    if (!S_ISREG(statbuf.st_mode))
		continue;      /* File is not a regular file */

	    if (access(testpath, X_OK) == -1)
		continue;      /* File is not executable */
	    
            foundfile = g_strdup (testpath);
            break;
	}
    g_free (testpath);

    return foundfile;
}


int
is_setid (char *abs_name)
{
    struct stat statbuf;
    
    if (stat (abs_name, &statbuf) < 0)
	{
	    fprintf (stderr, "%s: error: couldn't stat %s\n", programname, abs_name);
	    gtk_exit (1);
	}
    
    return ( (statbuf.st_mode & (S_ISUID)) || (statbuf.st_mode & (S_ISGID)) );
}


Window
find_window (Display *dpy, Window root, char *wname, char *wclass)
{
    Window dummy, *children = NULL, client;
    unsigned int i, nchildren = 0;
    Atom leader_atom;
    
    /*
     * clients are not allowed to stomp on the root and ICCCM doesn't yet
     * say anything about window managers putting stuff there; but, try
     * anyway.
     */
    
    /*
     * get the list of windows
     */
    if (!XQueryTree (dpy, root, &dummy, &dummy, &children, &nchildren)) 
	{
	    fprintf (stderr, "%s: fatal error\n", programname);
	    return None;
	}

    /* Get the WM_CLIENT_LEADER. If we do it here, we only has to do it once. */
    leader_atom = XInternAtom(dpy, "WM_CLIENT_LEADER", 1);

    for (i = 0; i < nchildren; i++) 
	{
	    client = XmuClientWindow (dpy, children[i]);
	    if (client != None)
		{
		    if (match_window (dpy, client, leader_atom, wname, wclass))
			return client;
		}
	}
    return None;
}
    

int
match_window (Display *dpy, Window w, Atom leader_atom, char *wname, char *wclass)
{
    XClassHint clh;
    XWindowAttributes win_attributes;

    /* Check Window state */
    if (!XGetWindowAttributes(dpy, w, &win_attributes))
	return FALSE;
    if (win_attributes.map_state != IsViewable)
	return FALSE;
    
    if (!XGetClassHint (dpy, w, &clh)) 
	return FALSE;

    /* Check resource name */
    if (clh.res_name) 
	{
	    if (strcmp (clh.res_name, wname)) 
		{   /* Name didn't match */
		    XFree (clh.res_name);
		    return FALSE;
		}
	    XFree (clh.res_name);
	}
    
    /* Check resource class */
    if (clh.res_class) 
	{
	    if (strcmp (clh.res_class, wclass)) 
		{   /* Class didn't match */
		    XFree (clh.res_class);
		    return FALSE;
		}
	    XFree (clh.res_class);
	}

    
    /* Check that this window does not have a client leader */
    if (leader_atom != None)
	{
	    int status;
	    Atom actual_type;
	    int actual_format;
	    unsigned long nitems;
	    unsigned long bytes_after;
	    unsigned char *prop;
	    
	    status = XGetWindowProperty(dpy, w, leader_atom, 0, 64,
					False, AnyPropertyType, &actual_type,
					&actual_format, &nitems, &bytes_after,
					&prop);
	    
	    if (actual_type != None)
		return FALSE;
	}
    
    return TRUE;
}


/* Initialize shaped windows and frames for animation.  Note: Xalf
   versions < 0.11 used one single window and re-shaped it every time
   a new frame was displayed. Unfortunately, this seems to crash many
   Xservers under load (for example XFree86 3.3). Therefor, Xalf now 
   uses 9 different windows and shows/hides these to make up the animation. 
   This is both slow and ugly, but at least it doesn't crash the Xserver 
   (hopefully). */
void
init_animation (struct anim_data_struct* anim_data)
{
    GtkWidget *fixed;
    GtkStyle *style;
    GdkGC *gc;
    int i;
    int pointer_x, pointer_y;

    xpm_array[0] = sp0_xpm;
    xpm_array[1] = sp1_xpm;
    xpm_array[2] = sp2_xpm;
    xpm_array[3] = sp3_xpm;
    xpm_array[4] = sp4_xpm;
    xpm_array[5] = sp5_xpm;
    xpm_array[6] = sp6_xpm;
    xpm_array[7] = sp7_xpm;
    xpm_array[8] = sp8_xpm;
    
    style = gtk_widget_get_default_style();
    gc = style->black_gc;

    /* Fetch pointer position */
    gdk_window_get_pointer (gdk_window_ref_from_xid (GDK_ROOT_WINDOW ()) , &pointer_x, &pointer_y, NULL);
    /* The frames are 48x48 pixels. Center star at cursor. */
    pointer_x -= 24;
    if (pointer_x < 0) pointer_x = 0;
    pointer_y -= 24;
    if (pointer_y < 0) pointer_y = 0;
    
    for (i = 0; i < MAX_FRAMES; i++)
        {
            /* Create windows */
            anim_data->windows[i] = gtk_window_new (GTK_WINDOW_POPUP);
            gtk_window_set_wmclass (GTK_WINDOW(anim_data->windows[i]), "anim", "xalf");     
            gtk_widget_realize (anim_data->windows[i]);
            gtk_widget_set_uposition (anim_data->windows[i], pointer_x, pointer_y);

            /* Create fixed container */
            fixed = gtk_fixed_new();
            gtk_container_add (GTK_CONTAINER(anim_data->windows[i]), fixed);
            gtk_widget_show (fixed);
            gtk_widget_set_usize(fixed, 200, 200 );

            /* Create pixmap */
            anim_data->frames[i] = gdk_pixmap_create_from_xpm_d (anim_data->windows[i]->window, &(anim_data->masks[i]),
                                                                 &style->bg[GTK_STATE_NORMAL], xpm_array[i]);
            anim_data->pixmaps[i] = gtk_pixmap_new (anim_data->frames[i], anim_data->masks[i]);
            gtk_widget_show (anim_data->pixmaps[i]);
            
            /* Put pixmap in container */
            gtk_fixed_put (GTK_FIXED(fixed), anim_data->pixmaps[i], 0, 0 );

            /* Reshape window. */
            /* This masks out everything except for the image itself. */
            gtk_widget_shape_combine_mask (anim_data->windows[i], anim_data->masks[i], 0, 0 );
        }

    anim_data->active_frame_number = 0;

    /* Show the first frame */
    gtk_widget_show (anim_data->windows[0]);
}


/* Show the next frame of animation */
gint
update_anim (gpointer data)
{
    int cur_index, next_index;
    struct anim_data_struct *anim_data = (struct anim_data_struct *)data;

    cur_index = anim_data->active_frame_number;
    next_index = (cur_index + 1) % MAX_FRAMES;
    anim_data->active_frame_number = next_index;

    gtk_widget_hide (anim_data->windows[cur_index]);
    gtk_widget_show (anim_data->windows[next_index]);

    return TRUE;
}


gint
matched_star (Window xwin)
{
    int i;
    
    for (i = 0; i < MAX_FRAMES; i++) {
        if (xwin == GDK_WINDOW_XWINDOW(anim_data->windows[i]->window))
            return TRUE;
    }
    return FALSE;
}
