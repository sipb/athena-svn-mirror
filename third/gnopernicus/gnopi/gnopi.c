/* gnopi.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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

#include "config.h"
#include "util.h"
#include "SRMessages.h"
#include "gnopiconf.h"
#include "gnopiui.h"
#include "brlui.h"
#include "defui.h"
#include "genui.h"
#include "kbui.h"
#include "magui.h"
#include "spui.h"
#include "cmdmapui.h"
#include "findui.h"
#include "presui.h"
#include "langui.h"
#include "scrui.h"
#include "bmui.h"


#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "srintl.h"

#define GN_SHKEY 34556
#define GN_SEGSIZE 10

#define ACCESSIBILITY_GCONF_KEY "/desktop/gnome/interface/accessibility"

/*#define __WAIT_DEBUG__  */


static gboolean gn_run_with_parameters (void);
static gboolean gn_no_multiple_instance (void);
static gboolean gn_destroy_instance (void);
static gboolean gn_init (void);
static gboolean	gn_init_gconf (void);
static gboolean gn_terminate (void);
static gboolean gn_run_srcore (void);
static gboolean gn_child_process_wait (void);
static gboolean gn_check_accessibility_key (void);
static gboolean gn_gnopernicus_wait_to_core (void);

/**
 *
 * Instance of "Braille setting" structure
 *
**/
Braille   		*braille_setting = NULL;

/**
 *
 * Instance of "General setting" structure
 *
**/
General   		*general_setting = NULL;

/**
 *
 * Instance of "Keyboard setting" structure
 *
**/
Keyboard  		*keyboard_setting = NULL;

/**
 *
 * Instance of "Speech setting" structure				
 *
**/
Speech 	  		*speech_setting = NULL;

/**
 *
 * Instance of "Magnifier setting"used in gnopi				
 *
**/
Magnifier 		*magnifier_setting = NULL;

/**
 *
 * If the settings loaded with default values?
 *
**/
static gboolean gn_loaddefault = FALSE;

/**
 *
 * Shared memory id.
 *
**/
static gint 	gn_semid;

/**
 *
 * sr_pid - process id of srcore 
 *
**/
static gint	gn_srcore_pid;

/**
 *
 * srcore_status - exit status of srcore process
 *
**/
static gint	gn_srcore_status;

extern gboolean init_finish;

static gchar  **gn_argv;

static gboolean gn_defaultmode    = FALSE;
static gboolean gn_braille_enable = FALSE;
static gboolean gn_speech_enable  = FALSE;
static gboolean gn_magnifier_enable = FALSE;
static gboolean gn_braille_monitor_enable = FALSE;
static gboolean gn_braille_disable = FALSE;
static gboolean gn_speech_disable  = FALSE;
static gboolean gn_magnifier_disable = FALSE;
static gboolean gn_braille_monitor_disable = FALSE;
static gboolean gn_login = FALSE;

static gchar   *gn_braille_device = NULL;
static gint     gn_braille_port = -1;

struct poptOption poptopt[] = 
    {		
	{
	 "default", 	
	 'd', 	
	 POPT_ARG_NONE, 
	 &gn_defaultmode, 	
	 0, 
	 "Launch in execution with default settings", 
	 NULL
	},
	{
	 "enable-braille",	
	 'b', 	
	 POPT_ARG_NONE, 
	 &gn_braille_enable,		
	 0, 
	 "enable braille service", 
	 NULL
	},
	{
	 "disable-braille",	
	 'B', 	
	 POPT_ARG_NONE, 
	 &gn_braille_disable,		
	 0, 
	 "enable braille service", 
	 NULL
	},
	{
	 "enable-speech",	
	 's', 	
	 POPT_ARG_NONE, 
	 &gn_speech_enable,		
	 0, 
	 "enable speech service", 
	 NULL
	},
	{
	 "disable-speech",	
	 'S', 	
	 POPT_ARG_NONE, 
	 &gn_speech_disable,		
	 0, 
	 "disable speech service", 
	 NULL
	},
	{
	 "enable-magnifier",	
	 'm', 	
	 POPT_ARG_NONE, 
	 &gn_magnifier_enable,		
	 0, 
	 "enable magnifier service",  
	 NULL
	},
	{
	 "disable-magnifier",	
	 'M', 	
	 POPT_ARG_NONE, 
	 &gn_magnifier_disable,		
	 0, 
	 "disable magnifier service",  
	 NULL
	},

	{
	 "enable-braille-monitor", 
	 'o', 
	 POPT_ARG_NONE, 
	 &gn_braille_monitor_enable, 	
	 0, 
	 "enable braille monitor service", 
	 NULL
	},
	{
	 "disable-braille-monitor", 
	 'O', 
	 POPT_ARG_NONE, 
	 &gn_braille_monitor_disable, 	
	 0, 
	 "disable braille monitor service", 
	 NULL
	},
	{
	 "login", 
	 'l', 
	 POPT_ARG_NONE, 
	 &gn_login, 	
	 0, 
	 "Used at login time", 
	 NULL
	},
	{
	 "braille-port", 
	 'p',   
	 POPT_ARG_INT,  
	 &gn_braille_port, 	
	 0, 
	 "Serial port (ttyS)", 
	 "ttyS[1..4]"
	},
	{
	 "braille-device", 
	 'e', 
	 POPT_ARG_STRING, 
	 &gn_braille_device, 	
	 0, 
	 "Braille Device", 
	 "DEVICE NAME"
	},
	{
	 NULL, 0,     0, NULL, 0
	}
    };

gint 
main (gint argc,gchar *argv[])
{
    gint i;
    gboolean is_gtk_module = FALSE;

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, GNOPERNICUSLOCALEDIR);
    textdomain (GETTEXT_PACKAGE);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
#endif    

    is_gtk_module = FALSE;
    for (i = 0; i < argc; i++)
	if (strncmp (argv[i], "--gtk-module", 12) == 0)
	{
	    is_gtk_module = TRUE;
	    break;
	}
    
    if (!is_gtk_module)
    {
	gchar **argv1;

	argv1 = g_new (gchar*, argc + 1);
	for (i = 0; i < argc; i++)
	    argv1[i] = argv[i];
	argv1[argc] = "--gtk-module=gail:atk-bridge:gail-gnome";
	argc++;
	argv = argv1;
    }

    gnome_program_init ("gnopernicus", VERSION,
			LIBGNOMEUI_MODULE,
			argc, argv,
			GNOME_PARAM_POPT_TABLE, poptopt,
			GNOME_PARAM_HUMAN_READABLE_NAME, _("gnopernicus"),
			GNOME_PARAM_APP_DATADIR, DATADIR,
			NULL);
    sru_log_init ();

    if (!gn_init_gconf ()) 
    {
        sru_warning (_("Gconf initialization failed."));
	srcore_exit_all (TRUE);
        return EXIT_FAILURE;
    }
                        
    if (!gn_run_with_parameters ())
    {
	sru_warning (_("Invalid parameters."));
	return EXIT_FAILURE;
    }
	
    if (!gn_no_multiple_instance ())
    {
        sru_message (_("Multiple instances are NOT ALLOWED. This instance will end."));
        return EXIT_FAILURE;
    }

    if (!gtk_init_check (&argc, &argv))
    {
        sru_warning (_("GTK initialization failed."));
        return EXIT_FAILURE;
    }
    
    if (!gn_run_srcore ())
    {
        sru_warning (_("Can not create a new process."));
        return EXIT_FAILURE;
    }
    
    gn_gnopernicus_wait_to_core ();
    
    if (!gn_login)
    {
	if (!gn_check_accessibility_key ())
	{
	    sru_warning (_("Exit gnopernicus."));
	    srcore_exit_all (TRUE);
	    return FALSE;
	}
    }

    if (!gn_init ())
    {
	sru_warning (_("Gnopernicus initialization failed."));
	srcore_exit_all (TRUE);
        return EXIT_FAILURE;
    }
	
    if (!gn_load_gnopi ())
    {
        sru_warning (_("Can not load .glade2 file"));
        return EXIT_FAILURE;
    }
	
    if (srcore_minimize_get ())
    	gn_iconify_gnopernicus 	();
    
    gtk_main ();

    gn_terminate ();

    gn_child_process_wait ();
    
    gn_destroy_instance ();

    sru_log_terminate ();
    
    if (!is_gtk_module)
    	g_free (argv);
    
    return EXIT_SUCCESS;
}



/**
 *
 * Test the parameters list from line command.
 *
**/
gboolean 
gn_run_with_parameters (void)
{    
    gint     index   = 0;
    gboolean rv      = TRUE;
    
    if (gn_defaultmode)
	gn_loaddefault = TRUE;
	
#define COUNT_OF_PARAMETERS_FOR_FORK 8
    gn_argv = (gchar**) g_new0 (gchar*, COUNT_OF_PARAMETERS_FOR_FORK);
#undef  COUNT_OF_PARAMETERS_FOR_FORK

    gn_argv[index] = NULL;
    
    index++;

    if (gn_magnifier_disable)
    {
	gn_argv[index] = g_strdup ("--disable-magnifier");
	srcore_magnifier_status_set (FALSE);
	index++;
    }
    else
    if (gn_magnifier_enable)
    {
    	gn_argv[index] = g_strdup ("--enable-magnifier");
	srcore_magnifier_status_set (TRUE);
	index++;
    }
    
    if (gn_speech_disable)
    {
	gn_argv[index] = g_strdup ("--disable-speech");
	srcore_speech_status_set (FALSE);
	index++;
    }
    else
    if (gn_speech_enable)
    {
	gn_argv[index] = g_strdup ("--enable-speech");
	srcore_speech_status_set (TRUE);
	index++;
    }
    
    if (gn_braille_disable)
    {
    	gn_argv[index] = g_strdup ("--disable-braille");
	srcore_braille_status_set (FALSE);
	index++;
    }
    else
    if (gn_braille_enable)
    {
	gn_argv[index] = g_strdup ("--enable-braille");
	srcore_braille_status_set (TRUE);
	index++;
    }
    

    if (gn_braille_monitor_disable)    
    {
    	gn_argv[index] = g_strdup ("--disable-braille-monitor");
	srcore_braille_monitor_status_set (FALSE);
	index++;
    }
    else
    if (gn_braille_monitor_enable)    
    {
    	gn_argv[index] = g_strdup ("--enable-braille-monitor");
	srcore_braille_monitor_status_set (TRUE);
	index++;
    }

    if (gn_braille_port > -1)
    {
    	gn_argv[index] = g_strdup_printf ("--braille-port=%d", gn_braille_port);
	brlconf_port_no_set (gn_braille_port);
	index++;
    }
    
    if (gn_braille_device)
    {
	gchar *tmp = NULL;
	tmp = g_utf8_strup (gn_braille_device, -1);
	g_free (gn_braille_device);
	gn_braille_device = tmp;
    	gn_argv[index] = g_strdup_printf ("--braille-device=%s", gn_braille_device);
	brlconf_device_set (gn_braille_device);
	index++;
    }

    gn_argv[index] = NULL;

    return rv;
}

#define COUNT_OF_TIME_OUT_CALLS 	7
static gboolean 
gn_gnopernicus_syncronize (gpointer data)
{
    static int n = 0;

    if (n < COUNT_OF_TIME_OUT_CALLS && 
	init_finish == FALSE)
    {
	n++;
	return TRUE;
    }

    gtk_main_quit ();
    
    return FALSE;
}

gboolean
gn_gnopernicus_wait_to_core 	(void)
{    
    
#define LOOP_EXIT_TIME_OUT_INTERVAL 300
    g_timeout_add (LOOP_EXIT_TIME_OUT_INTERVAL, gn_gnopernicus_syncronize, NULL);
    
    gtk_main ();
#undef  LOOP_EXIT_TIME_OUT_INTERVAL    

    return init_finish; 
}


/**
 * gn_check_accessibility_key
 *
 * Check if the accessibility gconf key is turned on. Turned on off the user want.
 *
 * return: FALSE if the gnopernicus needed to restart.
**/
gboolean
gn_check_accessibility_key (void)
{
    GError 	*error  = NULL;
    GtkWidget 	*dialog = NULL;
    GladeXML	*xml    = NULL;
    gint         response_id;
    gboolean     accessibility_on;
    
    /* check if accessibility flag is TRUE */
    accessibility_on = 	gnopiconf_get_bool (ACCESSIBILITY_GCONF_KEY, 
					    &error);
        
    if (error)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	return TRUE;
    }
        
    if (accessibility_on)
	return TRUE;

    xml 	= gn_load_interface ("gnopi.glade2", "dl_enable");	
    dialog 	= glade_xml_get_widget (xml, "dl_enable");
    
    g_object_unref (G_OBJECT (xml));
    
    AtkObject *obj = gtk_widget_get_accessible (dialog);
    atk_object_set_role (obj, ATK_ROLE_ALERT);
    
    response_id = gtk_dialog_run (GTK_DIALOG (dialog));

/*    gtk_widget_destroy (dialog);*/ /* FIXME: if uncomment this it crash the application */
    gtk_widget_hide (dialog);

    if (response_id == GTK_RESPONSE_NO)
	return TRUE;
	
    if (response_id == GTK_RESPONSE_YES)
    {
	gnopiconf_set_bool (TRUE, ACCESSIBILITY_GCONF_KEY);
	
	xml 	= gn_load_interface ("gnopi.glade2", "dl_logout");	
	dialog 	= glade_xml_get_widget (xml, "dl_logout");
	
	g_object_unref (G_OBJECT (xml));
	
        AtkObject *obj = gtk_widget_get_accessible (dialog);
        atk_object_set_role (obj, ATK_ROLE_ALERT);
        
	response_id = gtk_dialog_run (GTK_DIALOG (dialog));
	
	gtk_widget_hide (dialog);

	if (response_id == GTK_RESPONSE_NO)
	    return TRUE;
	if (response_id == GTK_RESPONSE_YES)
	{
	    GnomeClient *client = NULL;
	    if (client = gnome_master_client ()) 
		gnome_client_request_save (client, GNOME_SAVE_GLOBAL, TRUE, 
					GNOME_INTERACT_ANY, FALSE, TRUE);
	    return TRUE;
	}
	
    }
    
    return TRUE;
}

gboolean
gn_init_gconf (void)
{
    sru_return_val_if_fail (gnopiconf_gconf_client_init (), FALSE);
    sru_return_val_if_fail (brlconf_gconf_client_init (), FALSE);
    sru_return_val_if_fail (kbconf_gconf_client_init (), FALSE);
    sru_return_val_if_fail (spconf_gconf_client_init (), FALSE);
    sru_return_val_if_fail (magconf_gconf_client_init (), FALSE);
    sru_return_val_if_fail (srcore_gconf_client_init (), FALSE);
    sru_return_val_if_fail (cmdconf_gconf_client_init (), FALSE);
    sru_return_val_if_fail (presconf_gconf_init (), FALSE);
    sru_return_val_if_fail (bmconf_gconf_client_init (), FALSE);

    return TRUE;
}

/**
 *
 * Initialize all gconf client and load all settings.
 * return - FALSE error
 *
**/
gboolean 
gn_init (void)
{        
    braille_setting   = brlconf_setting_init (TRUE);
    keyboard_setting  = kbconf_setting_init (TRUE);
    speech_setting    = spconf_setting_init (TRUE);
    general_setting   = srcore_general_setting_init (TRUE);

    srcore_exit_all (FALSE);
    
    if (gn_loaddefault) 
	defui_load_all_default ();

    return TRUE;
}

/**
 *
 * Destroy all gconf client and all internal structure
 *
**/
gboolean 
gn_terminate (void)
{
    brlconf_terminate	(braille_setting);
    kbconf_terminate	(keyboard_setting);
    spconf_terminate	(speech_setting);
    magconf_terminate	(magnifier_setting);
    srcore_terminate	(general_setting);
    cmdconf_terminate	();
    presconf_gconf_terminate();
    return TRUE;
}

static void
gn_child_exited (gint signo)
{
    if (signo == SIGCHLD)
    {
        if (waitpid (gn_srcore_pid, &gn_srcore_status, WNOHANG) == gn_srcore_pid)
	{
	    if (WIFEXITED (gn_srcore_status) == 0)
	    {
/*	        if (WIFSIGNALED (gn_srcore_status) != 0)
		    sru_message ("signal no: %d", WTERMSIG (gn_srcore_status));*/
		sru_warning (_("srcore exited."));
		gdk_beep ();
	    }
	}
    }
}

/**
 *
 * Launch in executions SRCORE
 *
**/

gboolean 
gn_run_srcore (void)
{
    signal (SIGCHLD, gn_child_exited);			
    
    if (g_file_test ("../srcore/srcore", G_FILE_TEST_EXISTS) &&
        g_file_test ("../srcore/srcore", G_FILE_TEST_IS_EXECUTABLE) &&
	g_file_test ("../srcore/srcore", G_FILE_TEST_IS_REGULAR))
    {
	gn_argv[0] = g_strdup ("../srcore/srcore");
	
	if (!g_spawn_async ( ".", gn_argv, NULL, 
			    G_SPAWN_DO_NOT_REAP_CHILD,
			    NULL, NULL, &gn_srcore_pid, NULL))
	{
	    sru_message ("No \"%s\" binary file found [errno: %i].",
			"srcore",
			errno);
	    g_strfreev (gn_argv);
	    return FALSE;
	}
    }
    else
    if (g_file_test ("./srcore", G_FILE_TEST_EXISTS) &&
    	g_file_test ("./srcore", G_FILE_TEST_IS_EXECUTABLE) &&
	g_file_test ("./srcore", G_FILE_TEST_IS_REGULAR))
    {
	gn_argv[0] = g_strdup ("./srcore");
	
	if (!g_spawn_async ( ".", gn_argv , NULL, 
			G_SPAWN_DO_NOT_REAP_CHILD,
			NULL, NULL, &gn_srcore_pid, NULL))
	{
	    sru_message ("No \"%s\" binary file found [errno: %i].",
			"srcore",
			errno);
	    g_strfreev (gn_argv);
	    return FALSE;
	}
    }
    else
    {
	gn_argv[0] = g_strdup ("srcore");
	if (!g_spawn_async ( ".", gn_argv, NULL, 
			    G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
			    NULL, NULL, &gn_srcore_pid, NULL))
	{
	    sru_message ("No \"%s\" binary file found [errno: %i].",
			"srcore",
			errno);
	    g_strfreev (gn_argv);
	    return FALSE;
	}
    }
			
    g_strfreev (gn_argv);
    
    return TRUE;
}

/**
 *
 * Test if it lunched more then one time.
 * return - TRUE no multiple execution
 *
**/
gboolean 
gn_no_multiple_instance (void)
{
    key_t key;
    gchar *shmptr;
    gchar *text;
    gint pd = 0;

    signal (SIGUSR1,SIG_IGN);

    /*if ((key = ftok(".",'g')) == -1)	return FALSE; */
    
    key = GN_SHKEY;

    if ((gn_semid = shmget (key, GN_SEGSIZE, IPC_CREAT|IPC_EXCL|0666)) == -1)
    {
	if ((gn_semid = shmget (key, GN_SEGSIZE, 0)) == -1) return FALSE;
	    else
	    {
		if ((shmptr = (gchar*) shmat (gn_semid, 0, 0)) == NULL) return FALSE;

		pd = atoi (shmptr);
		
		if (pd == 0)
    		{
		    sru_error ("No srcore in execution.");
		    return FALSE;
		}

		if (kill (pd, SIGUSR1) == -1)
		{
		    if (errno == ESRCH)
		    {
			pd   = getpid ();
                        text = g_strdup_printf ("%i", pd);
			g_stpcpy (shmptr, text);
			g_free (text);
			text = NULL;
			return TRUE;
		    }
		}
	    }
	return FALSE;
    }
    else
    {
	if ((shmptr = (gchar*)shmat (gn_semid, 0, 0)) == NULL) 
	    return FALSE;
	pd   = getpid ();
        text = g_strdup_printf ("%i", pd);
	g_stpcpy (shmptr, text);
	g_free (text);
	text = NULL;
    }
    return TRUE;
}

/**
 *
 * Destroy shared memory and message queue.
 * return - FALSE if error;
 *
**/
gboolean 
gn_destroy_instance (void)
{
    shmctl (gn_semid, IPC_RMID, 0);
    gn_semid = 0;
    return TRUE;
}

/**
 *
 * Wait for end of execiton of child process
 *
**/
gboolean 
gn_child_process_wait (void)
{    
    if (waitpid (gn_srcore_pid, &gn_srcore_status, WNOHANG) == gn_srcore_pid)
    {
#ifdef __WAIT_DEBUG__
	    if (WIFEXITED (gn_srcore_status)  != 0)
		    sru_message ("Exit cod: %d", WEXITSTATUS(gn_srcore_status));
    
	    if (WIFSIGNALED(gn_srcore_status) != 0)
		    sru_message ("Signaled: %d", WTERMSIG(gn_srcore_status));
#endif
	gn_srcore_pid = 0;
    }

    
    /* Emit terminate signal to SRCore that it to be sure it is exited */
    if (gn_srcore_pid != 0)
    {
	kill (gn_srcore_pid, SIGTERM);
    }
    
    return TRUE;    
}
