/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * xalf - X application launch feedback
 * A wrapper for starting X applications. Provides three indicators:
 *
 * 1. An invisible window, to be shown in pagers like Gnomes tasklist_applet
 * or KDE taskbar. 
 *
 * 2. Generic splashscreen
 *
 * 3. Add hourglass to mouse cursor for root window and Gnome's panel. 
 *
 * Copyright Peter Åstrand <altic@lysator.liu.se> 2000. GPLV2. 
 *
 * Source is (hopefully) formatted according to the GNU Coding standards. 
 *
 */


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
#include "sp1.xpm"
#include "sp2.xpm"
#include "sp3.xpm"
#include "sp4.xpm"
#include "sp5.xpm"
#include "sp6.xpm"
#include "sp7.xpm"
#include "sp8.xpm"
#include "sp9.xpm"



#undef DEBUG 

#ifdef DEBUG
#   define DPRINTF(args) fprintf args
#else
#   define DPRINTF(args) 
#endif


#define PID_PROPERTY_NAME "XALF_LAUNCH_PID"
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


enum { VERSION_MAJOR = 0 };
enum { VERSION_MINOR = 3 };
#define MAINTAINER		"altic@lysator.liu.se"
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
static GdkWindow*	gdk_window_ref_from_xid	(Window xwin);
static GdkFilterReturn	root_event_monitor (GdkXEvent *gdk_xevent,
					    GdkEvent *event,
					    gpointer gdk_root);
char *find_in_path (char *filename);
int is_setid (char *filename);
Window find_window (Display *dpy, Window root, char *wname, char *wclass);
int match_window (Display *dpy, Window w, Atom leader_atom, char *wname, char *wclass);
void restore_cursor ();

/* animation data */
#define MAX_FRAMES 9
struct anim_data_struct {
    int active_frame_number;
    GdkPixmap *frames[MAX_FRAMES];
    GdkBitmap *masks[MAX_FRAMES];
    GtkWidget *active_frame, *win;
};
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
int timeout_tag = 0;

void 
restore_cursor ()
{
    if (cursor_opt) 
	{
	    gtk_timeout_remove (timeout_tag);
	    change_cursor (FALSE);
	}
}


void 
timeout (int signo) 
{                                                                               
    remove_sighandlers ();
    ((void) fprintf (stderr, "%s: timeout launching %s\n", programname, taskname));
    restore_cursor ();
    gtk_exit (1);
}    


void
child_terminated (int signo)
{
    /* Most of the code in this function is disabled, because if one have 
       shellscripts that starts the application in the background, the script
       will immediatley exit. Nevertheless, we should keep tracking the launch. */
    
    /* int status; */
    /*    remove_sighandlers (); */

    /* fprintf (stderr, "%s: child (%s) died prematurely\n", programname, taskname); */
    
    /*    if (cursor_opt)  */
    /*  	{ */
    /*  	    gtk_timeout_remove (timeout_tag); */
    /*  	    change_cursor (FALSE); */
    /*  	} */
    /*      wait (&status); */
    /*      gtk_exit (WIFEXITED(status) ? WEXITSTATUS(status) : 1); */
}


void 
mapped (int signo) 
{      
    remove_sighandlers ();
    DPRINTF((stderr, "%s: App is now mapped: %s\n", programname, taskname));
    restore_cursor ();
    gtk_exit (0);
}    


void 
terminate (int signo) 
{      
    
    DPRINTF((stderr, "%s: Got termination signal %d\n", programname, signo));
    
    restore_cursor ();
    gtk_exit (1);
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
main (int argc, char **argv)
{
    int pid;
    int arg = 1;
    int noxalf_opt = FALSE;
    int invisiblewindow_opt = FALSE;
    int splash_opt = FALSE; 
    int anim_opt = FALSE;
    int mappingmode_opt = FALSE;
    unsigned timeouttime = DEFAULT_TIMEOUT;
    char *endptr;
    char *preload_string, *oldpreload;
    int optchar;
    struct anim_data_struct* anim_data;
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
		    fprintf (stdout, "%s version %d.%d\n", CANONICAL_NAME, 
			     VERSION_MAJOR, VERSION_MINOR);
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
		    break;

		case 's':
		    splash_opt = TRUE;
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

    gtk_init (&argc, &argv); 
    taskname = g_strdup (argv[optind]);	

    /* The user didn't supply an title. Use the name of the binary. */
    if (!title)
	title = taskname;
    
    /* Show indicators. We do this as early as possible, to give rapid feedback */

    dpy = GDK_DISPLAY ();

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
	    anim_data->active_frame_number = 0;
	    init_animation (anim_data);
	    anim_timer = gtk_timeout_add(50, update_anim, (gpointer) anim_data);
	}
    if (cursor_opt)
	{
	    change_cursor (TRUE);
	    timeout_tag = gtk_timeout_add (500, redraw_cursor, NULL);
	}
    
    while (gtk_events_pending ())
	gtk_main_iteration ();
    
    if (!mappingmode_opt)
	{

#ifdef MULTI_PRELOAD
	    if (getenv("LD_PRELOAD") != NULL)
		preload_string = g_strconcat ("LD_PRELOAD=", getenv("LD_PRELOAD"), ":", PRELOAD_LIBRARY, NULL);
	    else
		preload_string = g_strconcat ("LD_PRELOAD=", PRELOAD_LIBRARY, NULL);
	    
#else
	    preload_string = g_strconcat ("LD_PRELOAD=", PRELOAD_LIBRARY, NULL);

	    /* If LD_PRELOAD is alread set and this system does not support 
	       multiple libs in LD_PRELOAD, use use --mappingmode. */

            oldpreload = getenv("LD_PRELOAD");
	    if (oldpreload && *oldpreload)
		{
		    fprintf (stderr, 
			     "%s: LD_PRELOAD is already set. Using --mappingmode\n", 
			     programname);
		    mappingmode_opt = TRUE;
		}
#endif       

	    /* Check if preload library is available */
	    {
		void *handle = dlopen (PRELOAD_LIBRARY, RTLD_LAZY);
		if (!handle)
		    {
			fprintf (stderr, 
				 "%s: preload library not found. Using --mappingmode\n", 
				 programname);
			mappingmode_opt = TRUE;
		    }
		else
		    dlclose (handle);
	    }
	    /* Check if program is setuid or setgid */
	    {
		gchar *abs_name;
		
		abs_name = find_in_path (taskname);
		if (!abs_name)
		    {
			fprintf (stderr, "%s: error: couldn't find %s in PATH\n", 
				 programname, taskname);
			restore_cursor ();
			gtk_exit (1);
		    }

		if (is_setid (abs_name))
		    {
			fprintf (stderr, "%s: %s is setuid and/or setgid. Using --mappingmode\n", 
				 programname, taskname);
			mappingmode_opt = TRUE;
		    }
	    }
	}


    install_sighandlers ();
    alarm (timeouttime);  

    if (mappingmode_opt)
	{
	    GdkWindow *window;
	    XWindowAttributes attribs = { 0, };
	    
	    window = gdk_window_ref_from_xid (GDK_ROOT_WINDOW ());
	    if (!window) fprintf(stderr, "%s: fatal error.\n", programname);
	    gdk_window_add_filter (window, root_event_monitor, window);     
	    /* set event mask for events on root window */                                
	    XGetWindowAttributes (GDK_DISPLAY (), 
				  GDK_ROOT_WINDOW (), 
				  &attribs); 
	    XSelectInput (GDK_DISPLAY (), 
			  GDK_ROOT_WINDOW (), 
			  attribs.your_event_mask | SubstructureNotifyMask);
	    gdk_flush (); 
	}
    
    
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

	pid_string = g_strconcat (PID_PROPERTY_NAME, "=", launch_pid, NULL);
	switch (pid = fork ())
	    {
	    case -1:
		fprintf (stderr, "%s: error forking\n", programname);
		gtk_exit (1);
	    case 0:
		if (!mappingmode_opt) 
		    {
			putenv (preload_string);
			putenv (pid_string);
		    }
		execvp (argv[optind], argv+optind);
		fprintf (stderr, "%s: error executing\n", programname);
		kill (atol (launch_pid), SIGUSR1);
		_exit (1);
	    }
    }

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
    gtk_window_set_wmclass (GTK_WINDOW(window), "xalf", "xalf");     
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
    gtk_window_set_wmclass (GTK_WINDOW(dialog), "xalf", "xalf");     
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
    signal (SIGALRM, timeout);
    signal (SIGCHLD, child_terminated);
    signal (SIGUSR1, mapped);
    signal (SIGTERM, terminate);
    signal (SIGINT, terminate);
    signal (SIGQUIT, terminate);
}


void 
remove_sighandlers ()
{
    signal (SIGALRM, SIG_IGN);
    signal (SIGCHLD, SIG_IGN);
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
            DPRINTF((stderr, "Got MapNotify\n"));
            mapped (SIGUSR1);
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
    
    /* If an absolue filename are given, return it */
    if (filename[0] == '/')
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



/* initialize shaped window and frames for animation */
void
init_animation (struct anim_data_struct *anim_data)
{
    GtkWidget *fixed;
    GtkStyle *style;
    GdkGC *gc;

    anim_data->win = gtk_window_new( GTK_WINDOW_POPUP );
    gtk_widget_realize (anim_data->win);

    style = gtk_widget_get_default_style();
    gc = style->black_gc;
    anim_data->frames[0] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[0]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp1_xpm);
    anim_data->frames[1] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[1]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp2_xpm);
    anim_data->frames[2] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[2]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp3_xpm);
    anim_data->frames[3] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[3]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp4_xpm);
    anim_data->frames[4] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[4]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp5_xpm);
    anim_data->frames[5] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[5]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp6_xpm);
    anim_data->frames[6] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[6]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp7_xpm);
    anim_data->frames[7] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[7]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp8_xpm);
    anim_data->frames[8] = gdk_pixmap_create_from_xpm_d ( anim_data->win->window, &(anim_data->masks[8]),
                                                          &style->bg[GTK_STATE_NORMAL],  sp9_xpm);

    anim_data->active_frame = gtk_pixmap_new( anim_data->frames[0], anim_data->masks[0] );

    fixed = gtk_fixed_new();
    gtk_widget_set_usize( fixed, 200, 200 );
    gtk_fixed_put ( GTK_FIXED(fixed), anim_data->active_frame, 0, 0 );
    gtk_container_add ( GTK_CONTAINER(anim_data->win), fixed );

    /* This masks out everything except for the image itself */
    gtk_widget_shape_combine_mask (anim_data->win, anim_data->masks[0], 0, 0 );
    gtk_widget_show (anim_data->win);
    gtk_widget_show (anim_data->active_frame );
    gtk_widget_show (fixed );

    /* show the window */
    gtk_widget_set_uposition ( anim_data->win, 50, 20 );
    gtk_widget_show (anim_data->win );
}


/* show the next frame of animation */
gint
update_anim (gpointer data)
{
    int indx;
    struct anim_data_struct *anim_data = (struct anim_data_struct *)data;

    ++anim_data->active_frame_number;
    if ( anim_data->active_frame_number > MAX_FRAMES-1)
        anim_data->active_frame_number = 0;
    indx = anim_data->active_frame_number;
	
    gtk_pixmap_set(GTK_PIXMAP(anim_data->active_frame), anim_data->frames[indx], anim_data->masks[indx]);

    /* This masks out everything except for the image itself */
    gtk_widget_shape_combine_mask( anim_data->win, anim_data->masks[indx], 0, 0 );

    return TRUE;
}



