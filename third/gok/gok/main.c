/*
* main.c
*
* Main file for the mighty GOK
*
* Copyright 2001 - 2004 Sun Microsystems, Inc.,
* Copyright 2001 - 2004 University Of Toronto
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <dirent.h>
#include <gnome.h>
#include <cspi/spi.h>
#include <libbonobo.h>
#include <bonobo-activation/bonobo-activation.h>
#include <atk/atkobject.h>
#include <libspi/Accessibility.h>
#include <libspi/accessible.h>
#include <libspi/application.h>
#include <gconf/gconf-client.h>
#include <locale.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "main.h"
#include "gok.h"
#include "gok-word-complete.h"
#include "gok-branchback-stack.h"
#include "gok-data.h"
#include "gok-scanner.h"
#include "gok-settings-dialog.h"
#include "callbacks.h"
#include "switchapi.h"
#include "gok-sound.h"
#include "gok-log.h"
#include "gok-gconf-keys.h"
#include "gok-predictor.h"
#include "gok-editor.h"
#include "gok-spy.h"
#include "gok-feedback.h"
#include "gok-modifier.h"
#include "gok-action.h"
#include <signal.h>
#include "gok-bounds.h"

#define USE_GCONF 1

#define APP_STATIC_BUFF_SZ 30
#define MAX_DELETABLE_KEYBOARDS 50

/* gok error exit codes */

#define GOK_ERROR_SETTINGSDIALOG_OPEN 3
#define GOK_ERROR_MAIN_CREATE_WINDOW  4
#define GOK_ERROR_DISPLAY_SCAN_MAIN   5
#define GOK_ERROR_ACTION_OPEN         6
#define GOK_ERROR_FEEDBACK_OPEN       7
#define GOK_ERROR_NO_XKB_EXTENSION    8
#define GOK_ERROR_NOSTICKYKEYS		  9

#define GOKMAINKEYBOARDNAME "main"
#define GOKLOGINKEYBOARDNAME "Keyboard"

#define KEYBOARD_ACCESSIBILITY_STICKY_KEYS_KEY "/desktop/gnome/accessibility/keyboard/stickykeys_enable"
#define GCONF_ACCESSIBILITY_KEY "/desktop/gnome/interface/accessibility"
#define KEYBOARD_ACCESSIBILITY_ENABLE_KEY "/desktop/gnome/accessibility/keyboard/enable"

static gboolean respawn_on_segv = TRUE;

static GokApplication *_gok_app = NULL;

/* pointer to the first keyboard in the list of keyboards */
static GokKeyboard* m_pKeyboardFirst = NULL;

/* pointer to the keyboard that is currently displayed */
static GokKeyboard* m_pCurrentKeyboard = NULL;

/* pointer to the window that holds the GOK keyboard */
static GtkWidget* m_pWindowMain = NULL;

/* pointer to the foreground window's accessible interface */
static Accessible* m_pForegroundWindowAccessible = NULL;

/* will be zero if the window center location should be stored */
static gint m_countIgnoreConfigure = 0;

/* width and height that we have resized the window to */
static gint m_OurResizeWidth = 0;
static gint m_OurResizeHeight = 0;

/* size and location of GOK window specified by given geometry */
static gboolean m_bUseGivenGeometry = FALSE;
static gint m_GeometryWidth = -1;
static gint m_GeometryHeight = -1;
static gint m_GeometryX = -1;
static gint m_GeometryY = -1;

/* pointer to the gok command predictor */
static Gok_Predictor m_pCommandPredictor = NULL;

/* poptOption storage structure */
typedef struct _GokArgs {
	char* accessmethodname;
	char* inputdevicename;
	char* mainkeyboardname;
	char* custom_compose_kbd_name;
	int display_keyboard_editor;
	char *geometry;
	int geometry_bitmask;
	gboolean is_login;
	int remember_geometry;
	int display_settings_dialog;
	gboolean use_extras;
	char* scanactionname;
	char* selectactionname;
	gboolean list_actions;
	gboolean list_accessmethods;
	gdouble valuator_sensitivity;
} GokArgs;

static GokArgs gok_args;

/* Private functions */
static void gok_main_read_rc (void);
static void gok_main_initialize_access_methods (GokArgs *args);
static void gok_main_initialize_wordcomplete (void);
static void gok_main_initialize_commandprediction (void);
static gboolean gok_main_display_scan_main(void);
static void gok_main_display_geometry_error (void);
static void gok_main_object_state_listener (Accessible* pAccessible);
static gboolean gok_main_has_xkb_extension ();
static gboolean gok_main_check_sticky_keys (GtkWidget *widget);

static gint _screen_height, _screen_width;


static struct poptOption options[] = {
	{
		"access-method", 
		'a',
		POPT_ARG_STRING, 
		&gok_args.accessmethodname, 
		0,
		N_("Use the specified access method. NAME is a string and can be found in the various access method files (.xam) assigned to the \"name\" property of <gok:accessmethod> tag. Note this is not necessarily the same as the name of the .xam file. (See --list-accessmethods)"), 
		N_("NAME")
	},
	{
		"editor", 
		'e', 
		POPT_ARG_NONE, 
		&gok_args.display_keyboard_editor, 
		0,
		N_("Start the GOK keyboard editor"), 
		NULL},
	{
		"extras", 
		'\0', 
		POPT_ARG_NONE, 
		&gok_args.use_extras, 
		0,
		N_("Use special, but possibly unstable, gok stuff"),
		NULL},
	{
		"geometry", 
		'\0', 
		POPT_ARG_STRING, 
		&gok_args.geometry, 
		0,
		N_("Whenever --geometry is not used gok remembers its position between invocations and starts in the position that it had when it was last shutdown.  When --geometry is used gok positions itself within the rectangular area of screen described by the given X11 geometry specification.  When --geometry is used gok does not remember its position when it shuts down.  This behaviour can be changed with the --remembergeometry flag which forces gok to remember its position when shutdown even when it was started with --geometry."),
		N_("GEOMETRY")
	},
	{
		"input-device", 
		'i', 
		POPT_ARG_STRING, 
		&gok_args.inputdevicename, 
		0,
		N_("Use the specified input device"), 
		N_("DEVICENAME")
	},
	{
		"keyboard", 
		'k', 
		POPT_ARG_STRING, 
		&gok_args.mainkeyboardname, 
		0,
		N_("Start GOK with the specified keyboard."), 
		N_("KEYBOARDNAME")
	},
	{
		"list-accessmethods", 
		'\0', 
		POPT_ARG_NONE, 
		&gok_args.list_accessmethods, 
		0,
		N_("List the access methods that can used as options to other arguments."),
		NULL
	},
	{
		"list-actions", 
		'\0', 
		POPT_ARG_NONE, 
		&gok_args.list_actions, 
		0,
		N_("List the actions that can used as options to other arguments."),
		NULL
	},
	{
		"login", 
		'l', 
		POPT_ARG_NONE, 
		&gok_args.is_login, 
		0,
		N_("GOK will be used to login"),
		NULL
	},


	{
		"remembergeometry", 
		'\0', 
		POPT_ARG_NONE,
		&gok_args.remember_geometry, 
		0,
		N_("Can be used with --geometry.  Forces GOK to remember its position when shutdown even when it was started with --geometry.  Please see the discussion under the --geometry flag for more information."),
		NULL
	},
	{
		/* primarily for use at login since there is no gconf */
		/* note: not enough info for 5-switch directed scanning */
		"scan-action", 
		'\0', 
		POPT_ARG_STRING, 
		&gok_args.scanactionname, 
		0,
		N_("Start GOK and hook this action to scan operations. (See --list-actions)"), 
		NULL
	},
	{
		/* primarily for use at login since there is no gconf */
		"select-action", 
		'\0', 
		POPT_ARG_STRING, 
		&gok_args.selectactionname, 
		0,
		N_("Start GOK and hook this action to select operations. (See --list-actions)"), 
		NULL
	},
	{
		"settings", 
		's', 
		POPT_ARG_NONE, 
		&gok_args.display_settings_dialog, 
		0,
		N_("Open the settings dialog box when GOK starts"),
		NULL
	},
	{
		"valuator-sensitivity", 
		'\0', 
		POPT_ARG_DOUBLE, 
		&gok_args.valuator_sensitivity, 
		0,
		N_("A multiplier to be applied to input device valuator events before processing"),
		NULL
	},

	/* End the list */
	{NULL, '\0', 0, NULL, 0, NULL, NULL}
};

static int segfaults = 0;
/* private */
static int gok_sig_handler(int sig)
{
	switch (sig) {
	case SIGSEGV:
		segfaults++;
		fprintf (stderr, "gok: Critical (nonrecoverable) error.\n");
		/* N.B.: if SPI_exit SEGVs, GOK will keep restarting until killed by SIGTERM */
#if ! defined ENABLE_LOGGING_NORMAL
		if (respawn_on_segv)
		{
		    fprintf (stderr, "Restarting GOK.\n"); 
		    g_spawn_command_line_async ("gok", NULL); 
		}
#endif
		break;
	case SIGTERM:
		fprintf (stderr, "gok: exiting (terminated)\n");
		break;
	case SIGINT:
	default:
		break;
	}
	
	_exit(1);
	return -1;  /* this line gets rid of compiler warning */ 
}

static void
gok_args_init (GokArgs *args)
{
	/* initialize command line option (popt) storage */
	args->accessmethodname = NULL;
	args->inputdevicename = NULL;
	args->mainkeyboardname = NULL;
	args->custom_compose_kbd_name = NULL;
	args->display_keyboard_editor = FALSE;
	args->geometry = NULL;
	args->geometry_bitmask;
	args->is_login = 0;
	args->remember_geometry = FALSE;
	args->display_settings_dialog = FALSE;
	args->use_extras = FALSE;
	args->scanactionname = NULL;
	args->selectactionname = NULL;
	args->valuator_sensitivity = 0;
}

/* callback for gok window state events */
gboolean
on_gok_window_state_event ( GtkWidget       *widget,
			    GdkEventWindowState  *event,
			    gpointer         user_data)
{
	if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED) {
		if (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED) {
			gtk_window_present((GtkWindow*)widget);
		}
	}
	return FALSE;
}

const gchar *
gok_main_get_custom_compose_kbd_name ()
{
	return gok_args.custom_compose_kbd_name;
}

gboolean
gok_main_safe_mode ()
{
	return _gok_app->safe;
}

static gboolean
gok_application_set_safe (LoginHelper *helper, gboolean safe)
{
    GOK_APPLICATION (helper)->safe = safe;
    gok_main_display_scan_reset ();
    return TRUE;
}

static LoginHelperDeviceReqFlags
gok_application_get_device_reqs (LoginHelper *helper)
{
    return LOGIN_HELPER_GUI_EVENTS | LOGIN_HELPER_POST_WINDOWS | LOGIN_HELPER_EXT_INPUT | 
	LOGIN_HELPER_AUDIO_OUT;
    /* 
     * FIXME return values based on current access method, i.e. may include
     * mouse/keyboard, or omit AUDIO.
     */
}

static Window*
gok_application_get_raise_windows (LoginHelper *helper)
{
    Window *mainwin = NULL;
    GtkWidget *widget = gok_main_get_main_window ();

    if (widget)
    {
	mainwin = g_new0 (Window, 2);
	mainwin[0] = GDK_WINDOW_XWINDOW (widget->window);
	mainwin[1] = None;
    }
    return mainwin;
}

static void
gok_application_init (GokApplication *app)
{
    app->safe = FALSE;
}

static void
gok_application_class_init (GokApplicationClass *klass)
{
    LoginHelperClass *login_helper_class = LOGIN_HELPER_CLASS(klass);

    login_helper_class->get_raise_windows = gok_application_get_raise_windows;
    login_helper_class->get_device_reqs = gok_application_get_device_reqs;
    login_helper_class->set_safe = gok_application_set_safe;
}

BONOBO_TYPE_FUNC (GokApplication,
		  LOGIN_HELPER_TYPE,
		  gok_application)

/**
* main
* @argc: ignored
* @argv: ignored
*
* The GOK main function.
*
* returns: Program exit code.
**/
gint main (gint argc, gchar *argv[])
{
	gint result;
	
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, GOK_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

	/* intialization */
	result = gok_main_open(argc, argv);
	
	if (result != 0)
	{
		return result;
	}
	
	gok_spy_add_idle_handler ();
	
	/* start the main hook */
	SPI_event_main();	

	/* cleanup */
	gok_main_close();
	 
	return 0;
}

/* private helper */
gint
comparebasenames (gconstpointer a, gconstpointer b)
{
	gchar* a_file;
	gchar* b_file;
	gint returncode;
	a_file = g_path_get_basename(a);
	b_file = g_path_get_basename(b);

	returncode = g_strcasecmp(a_file, b_file);
	
	g_free (a_file);
	g_free (b_file);
	
	return returncode;
}

/**
* gok_main_open
* @argc: ignored
* @argv: ignored
*
* The GOK initialization function.
*
* returns: a program exit code.
**/
gint
gok_main_open(gint argc, gchar *argv[])
{
	GnomeProgram *gok_program;

	/* initialize member data */
	m_pForegroundWindowAccessible = NULL;
	m_pKeyboardFirst = NULL;
	m_pWindowMain = NULL;
	m_pCurrentKeyboard = NULL;
	m_countIgnoreConfigure = 0;

	gok_args_init (&gok_args);

	/* Initialize Gnome */
	gok_program = gnome_program_init (PACKAGE, VERSION,
		LIBGNOMEUI_MODULE, argc, argv,
		GNOME_PARAM_POPT_TABLE, options,
		GNOME_PROGRAM_STANDARD_PROPERTIES,
	        GNOME_PARAM_APP_DATADIR, DATADIR"/gok", 
		LIBGNOMEUI_PARAM_CRASH_DIALOG, FALSE,
		NULL);
		
	
	if (!bonobo_init (&argc, argv))
	{
	    if (gok_args.is_login)
		g_error ("Could not initialize Bonobo");
	    else
		g_warning ("Could not initialize Bonobo");
	}
	
	else
	{
	    BonoboObject *obj;
	    const gchar  *display_name;
	    GSList       *reg_env = NULL;
	    Bonobo_RegistrationResult ret;
	    CORBA_Environment ev;

	    bonobo_activate ();

	    CORBA_exception_init (&ev);

	    _gok_app = g_object_new (GOK_TYPE_APPLICATION, NULL);
	    obj = BONOBO_OBJECT (_gok_app);
	    display_name = gdk_display_get_name (gdk_display_get_default ());
	    reg_env = bonobo_activation_registration_env_set (reg_env, "DISPLAY",
							      display_name);
	    
	    ret = bonobo_activation_register_active_server ("OAFIID:GNOME_GOK:1.0", 
							    BONOBO_OBJREF (obj), reg_env);
	    
	    bonobo_activation_registration_env_free (reg_env);
	
	    if (ret != Bonobo_ACTIVATION_REG_SUCCESS)
	    {
		if (gok_args.is_login)
		    return 1;
		else {
		    gchar *reason = NULL;
		    if (ret == Bonobo_ACTIVATION_REG_ALREADY_ACTIVE) reason = "already active";
		    else if (ret == Bonobo_ACTIVATION_REG_ERROR) reason = "error";
		    else if (ret == Bonobo_ACTIVATION_REG_NOT_LISTED) reason = "not listed";
		    else if (ret == Bonobo_ACTIVATION_REG_SUCCESS) reason = "success";
		    else reason = "unknown reason";
		    g_warning ("Error registering GOK as a server: %s", reason);
		}
	    }
	}
	
	/* Parse geometry */
	if (gok_args.geometry != NULL)
	{
		gok_args.geometry_bitmask = XParseGeometry (gok_args.geometry, &m_GeometryX,
		                                   &m_GeometryY,
		                                   (unsigned int *)&m_GeometryWidth,
		                                   (unsigned int *)&m_GeometryHeight);

		if ( (gok_args.geometry_bitmask & XValue)
		     && (gok_args.geometry_bitmask & YValue)
		     && (gok_args.geometry_bitmask & WidthValue)
		     && (gok_args.geometry_bitmask & HeightValue) )
		{
			gok_bounds_get_upper_left (gok_args.geometry_bitmask,
							m_GeometryX, m_GeometryY,
							m_GeometryWidth,
							m_GeometryHeight,
							gdk_screen_width (),
							gdk_screen_height (),
							&m_GeometryX, &m_GeometryY);

			gok_log_x ("gok screen space = %dx%d+%d+%d",
					m_GeometryWidth, m_GeometryHeight,
					m_GeometryX, m_GeometryY);

			/* given geometry is good so use it for GOK location */
			m_bUseGivenGeometry = TRUE;
		}
		else
		{
			fprintf (stderr, _("gok: Unsupported geometry specification\n"));
			fprintf (stderr, _("gok: Currently GOK requires that the x, y, width and height all be given\n"));
			gok_main_display_geometry_error ();
		}
	}

	/* initialise GConf */
	gconf_init (argc, argv, NULL);

	/* read the user profile data */
	gok_data_initialize ();

	/* check for --list-actions */
	if (gok_args.list_actions) { 
		GConfClient *gconf_client = gconf_client_get_default ();
		GError* error = NULL;
		GSList* list = gconf_client_all_dirs (gconf_client,
			GOK_GCONF_ACTIONS, &error);
		
		fprintf (stderr, "\nGOK Actions:\n");
		if (error == NULL) {
			list = g_slist_sort (list, (GCompareFunc)comparebasenames);
			GSList* listhead = list;
			while (list) {
				fprintf(stderr,"%s\n",g_path_get_basename(list->data));
				g_free(list->data);
				list = g_slist_next(list);
			}
			g_slist_free (listhead);
		}
		else {
			fprintf (stderr, "mousebutton<n>\nswitch<n>\ndwell\n\n");
			fprintf (stderr, "\n\n***WARNING: There was an error querying gconf.\nExhaustive list unavailable.\n\n");
		}
			
		_exit(0);
	}

	/* check for --list-accessmethods */
	if (gok_args.list_accessmethods) {
		/* note: code dupe of check for --list-actions */
		GConfClient *gconf_client = gconf_client_get_default ();
		GError* error = NULL;
		GSList* list = gconf_client_all_dirs (gconf_client,
			GOK_GCONF_ACCESS_METHOD_SETTINGS, &error);
		
		fprintf (stderr, "\nGOK Access Methods:\n");
		if (error == NULL) {
			gchar* base;
			list = g_slist_sort (list, (GCompareFunc)comparebasenames);
			GSList* listhead = list;
			while (list) {
				base = g_path_get_basename(list->data);
				fprintf(stderr,"%s\n",base);
				g_free(base);
				g_free(list->data);
				list = g_slist_next(list);
			}
			g_slist_free (listhead);
		}
		else {
			fprintf (stderr, "\nGOK Access Methods:\nautomaticscanning\ndirected\ndirectselection\ndwellselection\ninversescanniing\nkeyautoscanning\nkeyinversescanning\n\n");
			fprintf (stderr, "\n\n***WARNING: There was an error querying gconf.\nExhaustive list unavailable.\n\n");
		}
			
		_exit(0);
	}
	
	/* initialize the word prediction engine with gconf values */
	if (gok_data_get_use_aux_dictionaries ())
		gok_wordcomplete_set_aux_dictionaries (gok_wordcomplete_get_default (),
						       (gchar *) gok_data_get_aux_dictionaries ());
	/* cast silences compiler; we are discarding 'const' */

	gok_log ("gok_data_initialize has finished");

	/* load the ".rc" files used for the key styles */
	if (gok_data_get_use_gtkplus_theme () == FALSE)
	{
		gok_main_read_rc ();
		gok_log ("gok_main_read_rc has finished");
	}

	/* display the keyboard editor not the GOK */
	if (gok_args.display_keyboard_editor)
	{
		SPI_init ();
		gok_editor_run();
		SPI_event_main();	
		gok_editor_close();
		SPI_exit();
		return 1;
	}
	
	/* check for xkb extension */
	if (!gok_main_has_xkb_extension())
	{
		gok_main_display_error (_("XKB extension is required."));
		return GOK_ERROR_NO_XKB_EXTENSION;
	}		

	/* read the keyboards */
	gok_main_read_keyboards ();

	/* initialize the actions */
	if (gok_action_open() == FALSE)
	{
		gok_main_display_error (_("Can't initialize actions."));
		return GOK_ERROR_ACTION_OPEN;
	}

	/* initialize the button callbacks */
	gok_control_button_callback_open();
	
	/* initialize the feedbacks */
	if (gok_feedback_open() == FALSE)
	{
		gok_main_display_error (_("Can't initialize feedbacks."));
		return GOK_ERROR_FEEDBACK_OPEN;
	}

	/* initialize the access methods */
	gok_main_initialize_access_methods (&gok_args);

	/* initialize the SPI */
	SPI_init ();
	
	/* initialize the switch API */
	initSwitchApi();
	
	/* initialize the spy routines */
	gok_spy_open();
	gok_spy_register_windowchangelistener ((void *)gok_main_window_change_listener);
	gok_spy_register_mousebuttonlistener ((void *)gok_main_mousebutton_listener);	
	gok_spy_register_objectstatelistener ((void *)gok_main_object_state_listener);
	
	/* initialize sound */
	gok_sound_initialize();

	/* create the main window */
	m_pWindowMain = gok_main_create_window (
		gok_data_get_dock_type () != GOK_DOCK_NONE);
	if (m_pWindowMain == NULL)
	{
		gok_main_display_error (_("Can't create the main GOK window!"));
		return GOK_ERROR_MAIN_CREATE_WINDOW;
	}
	gtk_widget_show (m_pWindowMain);

	/* initialize these things */
	gok_chunker_initialize();
	gok_keyboard_initialize();
	gok_branchbackstack_initialize();

	fprintf (stderr, "login mode = %s\n", gok_args.is_login ? "true" : "false");
	if (!gok_args.is_login) {
		gok_main_initialize_wordcomplete ();
		gok_main_initialize_commandprediction ();
	}

	/* create the settings dialog */
	if (gok_settingsdialog_open (gok_args.display_settings_dialog) == FALSE)
	{
		gok_main_display_error (_("Can't create the settings dialog window!"));
		return GOK_ERROR_SETTINGSDIALOG_OPEN;
	}


	/* a specified keyboard takes precedance, even if the login flag is specifed
	   we use the login keyboard, otherwise the regular main keyboard */
	if (gok_args.mainkeyboardname == NULL) {
		if (gok_args.is_login) {
			gok_args.mainkeyboardname = (gchar*) 
				g_malloc (strlen (GOKLOGINKEYBOARDNAME) + 1);
			strcpy (gok_args.mainkeyboardname, (gchar *) GOKLOGINKEYBOARDNAME);
		}
		else {
			gok_args.mainkeyboardname = (gchar*) 
				g_malloc (strlen (GOKMAINKEYBOARDNAME) + 1);
			strcpy (gok_args.mainkeyboardname, (gchar *) GOKMAINKEYBOARDNAME);
		}
	}

	if (!gok_args.is_login) {
		gok_main_check_accessibility();
		gok_main_warn_if_corepointer_mode (_("You are using GOK in \'core pointer\' mode."), FALSE);
	}
	
	if (!gok_main_check_sticky_keys (m_pWindowMain)) {
		return GOK_ERROR_NOSTICKYKEYS;
	}

	/* display the "main" keyboard */
	if (gok_main_display_scan_main () == FALSE)
	{
		return GOK_ERROR_DISPLAY_SCAN_MAIN;
	}

gok_log ("check for currently active frame");
	if (m_pForegroundWindowAccessible == NULL) {
		/* check for currently active frame */
		Accessible* accessible;
		accessible = gok_spy_get_active_frame();
		if (accessible)
			gok_main_window_change_listener (accessible);
	}
gok_log ("finished check");
	
	/* connect signal handlers; don't trap SIGKILL */ 
	signal(SIGSEGV, (void (*)(int))gok_sig_handler);
	signal(SIGTERM, (void (*)(int))gok_sig_handler);
	signal(SIGINT,  (void (*)(int))gok_sig_handler);

	return 0;
}

/* private */
static gboolean
gok_main_display_scan_main()
{
	if (gok_main_display_scan ((GokKeyboard*)NULL, gok_args.mainkeyboardname, KEYBOARD_TYPE_UNSPECIFIED, KEYBOARD_LAYOUT_NORMAL, KEYBOARD_SHAPE_BEST) == FALSE)
	{
		/* if no "main" keyboard then display first keyboard in list */
		if (gok_main_display_scan ((GokKeyboard*)NULL, NULL, KEYBOARD_TYPE_UNSPECIFIED, KEYBOARD_LAYOUT_NORMAL, KEYBOARD_SHAPE_BEST) == FALSE)
		{
			gok_main_display_error (_("No keyboards to display!"));
			return FALSE;
		}
	}
	return TRUE;
}

/**
* gok_main_get_first_keyboard
* 
* Accessor function.
*
* returns: A pointer to the first keyboard in the list of keyboards.
*/
GokKeyboard* gok_main_get_first_keyboard ()
{
	return m_pKeyboardFirst;
}

/**
* gok_main_set_first_keyboard
* @pKeyboard: Pointer to the keyboard that will be set as the first keyboard.
*
* Sets the first keyboard in the list of keyboards.
*/
void gok_main_set_first_keyboard (GokKeyboard* pKeyboard)
{
	m_pKeyboardFirst = pKeyboard;
}

/**
* gok_main_get_current_keyboard
* 
* Accessor function.
*
* returns: A pointer to the keyboard that is currently displayed.
*/
GokKeyboard* gok_main_get_current_keyboard ()
{
	return m_pCurrentKeyboard;
}

/**
* gok_main_get_main_window
* 
* Accessor function.
*
* returns: A pointer to main GOK window.
*/
GtkWidget* 
gok_main_get_main_window ()
{
	return m_pWindowMain;
}

static char cursor_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
 
void
gok_main_set_cursor (GdkCursor *cursor)
{
	GdkPixmap *source;
	GdkColor color = { 0, 0, 0, 0 };
	static GdkCursor *nilCursor = NULL;
	if (!cursor) {
		if (!nilCursor) {
			GdkPixmap *source = 
				gdk_bitmap_create_from_data (NULL, cursor_bits,
							     8, 8);
			nilCursor = gdk_cursor_new_from_pixmap (source, source, 
							     &color, &color, 1, 1);
		}		
		cursor = nilCursor;
	}
	gdk_window_set_cursor (gok_main_get_main_window ()->window, cursor);
}

/**
* gok_main_get_command_predictor
* 
* Accessor function.
*
* returns: A pointer to command prediction engine
*/
Gok_Predictor gok_main_get_command_predictor ()
{
	return m_pCommandPredictor;
}

/**
* gok_main_get_foreground_window_accessible
* 
* Accessor function.
*
* returns: A pointer to foreground window's accessible interface.
*/
Accessible* gok_main_get_foreground_window_accessible()
{
	gok_log_enter();
	gok_log ("address:%d",m_pForegroundWindowAccessible);
	gok_log_leave();
	return m_pForegroundWindowAccessible;
}


/**
* gok_main_window_change_listener
* @pAccessible: Pointer to the foreground window's accessible interface.
* 
* This function is called each time the foreground window changes.
*
*/
void gok_main_window_change_listener (Accessible* pAccessible)
{
	GokKeyboard* pKeyboard = NULL;
	Accessible *acc = NULL;

	gok_log_enter();	

	/* note: ref counting happens in gok-spy's listeners */
	m_pForegroundWindowAccessible = pAccessible;

	/* reset the context menu until we receive a focus event */
	/*gok_spy_set_context_menu_accessible (NULL, 0);*/
	
	if (pAccessible && Accessible_getChildCount (pAccessible) == 1
	    && (acc = Accessible_getChildAtIndex (pAccessible, 0))
	    && Accessible_getRole (acc) == SPI_ROLE_MENU) 
	{
	    AccessibleNode *node = g_malloc (sizeof (AccessibleNode));
	    GokSpyUIFlags menu_flags;
	    menu_flags.value = 0;
	    menu_flags.data.menus = 1;
	    node->paccessible = acc;
	    node->flags.data.is_menu = 1;
	    node->link = 0;
	    node->pname = g_strdup (_("popup menu"));
	    gok_spy_update_component_list (acc, menu_flags);
	    gok_keyboard_branch_gui (node, GOK_SPY_SEARCH_MENU);
	    g_free (node);
	}
	else
	{
	    /* we might currently have a dynamic keyboard so we need to branch to be safe */
	    gok_main_display_scan_previous_premade();
	}

	if (acc) /* popup menu detection leaked this reference */
	{
	    Accessible_unref (acc); /* dont use gok_spy_accessible_unref since we didn't implicit ref above */
	}

	/*if (*/gok_keyboard_validate_dynamic_keys (pAccessible);/* == TRUE)*/
	{
		/* rechunk the keyboard because keys have changed state */
		gok_scanner_stop();
		pKeyboard = gok_main_get_current_keyboard();
		
		gok_chunker_chunk (pKeyboard);
		pKeyboard->bRequiresChunking = FALSE;
	
		/* fixme?: this is a kludge workaround for direct scanning */
		if (strcmp ("directed", gok_data_get_name_accessmethod()) == 0) {
			gok_chunker_highlight_center_key();
		}
		else {
			/* highlight the first chunk */
			gok_chunker_highlight_first_chunk();
		}
		gok_chunker_select_chunk();
		
		
		/* restart the scanning process */
		gok_scanner_start();
	}

	gok_main_raise_window ();

	gok_log_leave();	
}

/**
* gok_main_object_state_listener
* @pAccessible: Pointer to the foreground window's accessible interface.
* 
* This function is called each time the foreground window changes.
*
*/
static void 
gok_main_object_state_listener (Accessible* pAccessible)
{
	GokKeyboard* pKB;
	GokKey* pK;
	AccessibleStateSet* ass = NULL;
	gboolean update_everything = FALSE;

	pKB = gok_main_get_current_keyboard();
	if (pKB != NULL) {
		pK = pKB->pKeyFirst;
		while (pK != NULL) {
			if (pK->accessible_node && (pK->accessible_node->paccessible == pAccessible)) {
				/* note: this is very specific for now because we are doing
				   this to make sure checkbox and radio menu item state
				   is represented properly. TODO: generalize
				*/
				ass = Accessible_getStateSet(pAccessible);
				if (AccessibleStateSet_contains( ass, SPI_STATE_CHECKED )) {
					pK->ComponentState.active = TRUE;
				}
				else {
					pK->ComponentState.active = FALSE;
				}
				if (!AccessibleStateSet_contains( ass, SPI_STATE_ENABLED ))
				{
					pK->Style = KEYSTYLE_INSENSITIVE;
					gok_key_set_button_name (pK);
				}
				AccessibleStateSet_unref(ass);
				gok_key_update_toggle_state (pK);
				/* unfortunately, another special case for now... */
				if (Accessible_getRole (pAccessible) == SPI_ROLE_PAGE_TAB) 
				{
					update_everything = TRUE;
				}
				break;
			}
			pK = pK->pKeyNext;
		}
	}

	/* notes:
	   we now check to see if the current keyboard requires updating, currently
	   we simply call gok_main_ds, but if this is too slow we can create a 
	   method that updates the specific key in question 
	*/
	if (update_everything) 
	{
		GokKeyboard *current_kbd = gok_main_get_current_keyboard ();
		gok_spy_update_component_list (gok_main_get_foreground_window_accessible (), 
					       current_kbd->flags);
		gok_main_ds (current_kbd);
	}
}	


/**
* gok_main_motion_listener
* @n_axes: Number of axes on which motion may have occurred
* @motion_data: An array of long ints containing the motion data. 
* @mods: long int, ignored
* @timestamp: long int, ignored
*
* This handler is called each time there is a motion event from the connected input device.
*/
void gok_main_motion_listener (gint n_axes, int *motion_data, long mods, long timestamp)
{
        GtkWidget *window = gok_main_get_main_window ();
	if (window) 
	{
#ifdef GOK_LOG_INPUT_EVENTS
		gok_log_enter();
		gok_log ("%d axes: [%d] [%d] mods[%lx] timestamp[%ld]", n_axes,
			motion_data[0], (n_axes > 1) ? motion_data[1] : 0, 
			mods, timestamp);
		gok_log_leave ();
#endif
		gok_scanner_input_motion (motion_data, n_axes);

		if (gok_data_get_drive_corepointer ()) {
			Display *display;
			GdkWindow *root;
			display = GDK_WINDOW_XDISPLAY (window->window);
			root = gdk_screen_get_root_window (gdk_drawable_get_screen (window->window));
			XWarpPointer (display, None, 
				      GDK_WINDOW_XWINDOW (root),
				      0, 0, 0, 0, motion_data[0], motion_data[1]);
		}
	}
}

/**
* gok_main_button_listener
* @button: Switch number that has changed state.
* @state: State of the switch.
* @mods: long int, ignored
* @timestamp: long int, ignored
*
* This handler is called each time there is a button event.
*/
void gok_main_button_listener (gint button, gint state, long mods, long timestamp)
{
	gok_log_enter();
	gok_log("button[%d] state[%d] mods[%lx] timestamp[%ld]",button,state,mods,timestamp);

	if (gok_data_get_drive_corepointer ())
	{
		int x, y, win_x, win_y;
		Window root_ret, child_ret;
		unsigned int mask_ret;
		GtkWidget *window;
		char ename[12];
		snprintf (ename, 12, "b%d%c", button, (state) ? 'p' : 'r');
		window = gok_main_get_main_window ();
		XQueryPointer (GDK_WINDOW_XDISPLAY (window->window), 
			       GDK_WINDOW_XWINDOW (window->window), 
			       &root_ret, &child_ret, &x, &y,
			       &win_x, &win_y, &mask_ret);
		SPI_generateMouseEvent (x, y, ename);
	}

	if (state == 0)
	{
		gok_main_raise_window ();

		switch (button)
		{
			case 1:
				gok_scanner_on_switch1_up();
				break;
			case 2:
				gok_scanner_on_switch2_up();
				break;
			case 3:
				gok_scanner_on_switch3_up();
				break;
			case 4:
				gok_scanner_on_switch4_up();
				break;
			case 5:
				gok_scanner_on_switch5_up();
				break;
			default:
				gok_log_x("this mouse button wasted by gok!");
				break;
		}
	}
	else
	{
		switch (button)
		{
			case 1:
				gok_scanner_on_switch1_down();
				break;
			case 2:
				gok_scanner_on_switch2_down();
				break;
			case 3:
				gok_scanner_on_switch3_down();
				break;
			case 4:
				gok_scanner_on_switch4_down();
				break;
			case 5:
				gok_scanner_on_switch5_down();
				break;
			default:
				gok_log_x("this mouse button wasted by gok!");
				break;
		}
	}

	gok_log_leave();	
}

/**
 * gok_main_raise_window:
 * hack to make sure gok is not occluded - TODO: seems to be unstable
 *
 **/
void
gok_main_raise_window (void)
{
	if (m_pWindowMain && m_pWindowMain->window) 
	{
		gdk_window_raise (m_pWindowMain->window);
	}
}

/**
* gok_main_mouse_button_listener
* @button: Mouse button number that has changed state.
* @state: State of the button.
* @mods: long int, ignored
* @timestamp: long int, ignored
*
* This handler is called each time there is a mouse button event.
*/
void gok_main_mousebutton_listener (gint button, gint state, long mods, long timestamp)
{
	gok_log_enter();
	gok_log("mouse button[%d] state[%d] mods[%lx] timestamp[%ld]",button,state,mods,timestamp);

	if (state == 0)
	{
		gok_main_raise_window ();

		/* TODO: account for reversed button order, which is possible! 
		 * i.e. leftbutton == 3, etc.
		 */
		/*
		 * Since we now get global mouse button notifications
		 * from at-spi, we need to filter out those that occur
		 * _outside_ the GOK window if the current access
		 * method is "Direct Selection."
		 */
		switch (button)
		{
			case 1:
				gok_scanner_left_button_up();
				break;
			case 2:
				gok_scanner_middle_button_up();
				break;
			case 3:
				gok_scanner_right_button_up();
				break;
			case 4:
				gok_scanner_on_button4_up();
				break;
			case 5:
				gok_scanner_on_button5_up();
				break;
			default:
				gok_log_x("this mouse button wasted by gok!");
				break;
		}
	}
	else
	{
		switch (button)
		{
			case 1:
				gok_scanner_left_button_down();
				break;
			case 2:
				gok_scanner_middle_button_down();
				break;
			case 3:
				gok_scanner_right_button_down();
				break;
			case 4:
				gok_scanner_on_button4_down();
				break;
			case 5:
				gok_scanner_on_button5_down();
				break;
			default:
				gok_log_x("this mouse button wasted by gok!");
				break;
		}
	}

	gok_log_leave();	
}

/**
* gok_main_display_scan_reset
* 
* Display and scan the current keyboard. This should be called after the user settings 
* have been changed.
*
* returns: TRUE if the keyboard was displayed, FALSE if not.
*/
gboolean gok_main_display_scan_reset ()
{
	return gok_main_ds (m_pCurrentKeyboard);
}

/**
* gok_main_display_scan_previous_premade
* 
* Display and scan the first keyboard pulled from the branch-back-stack
* that is premade. Delete any dynamic keyboards that are on the stack 'along
* the way'. Don't push the current keyboard on the branch-back-stack.
*
* returns: TRUE if the previous keyboard was displayed, FALSE if not.
*/
gboolean gok_main_display_scan_previous_premade ()
{
	gboolean bReturnCode;
	GokKeyboard* arrayKeyboardsToDelete[MAX_DELETABLE_KEYBOARDS];
	GokKeyboard* pKeyboard;
	int index;
	
	gok_log_enter();

	pKeyboard = m_pCurrentKeyboard;
	if (pKeyboard->bDynamicallyCreated != TRUE)
	{
		gok_log_leave();
		return gok_main_display_scan_previous();
	}

	/* initialize this array */
	for (index = 0; index < MAX_DELETABLE_KEYBOARDS; index++)
	{
		arrayKeyboardsToDelete[index] = NULL;
	}
	
	if (gok_branchbackstack_is_empty() == TRUE)
	{
		/* no keyboard on the branch back stack */
		gok_log_leave();
		return FALSE;
	}

	/* start pulling off the branch back stack */
	/* stop when we get to a premade keyboard */
	index = -1;
	
	while ((pKeyboard->bDynamicallyCreated == TRUE) && (index < MAX_DELETABLE_KEYBOARDS))
	{
		/* store the dynamic keyboards in the array */
		gok_log("adding [%s] (%x) to the list of keyboards to delete", pKeyboard->Name, pKeyboard);
		arrayKeyboardsToDelete[++index] = pKeyboard;
		pKeyboard = gok_branchbackstack_pop();
		if (pKeyboard == NULL)
		{
			break;
		}
	}
	
	if (pKeyboard == NULL)
	{
		gok_log_x ("No premade keyboards in stack in gok_main_display_scan_previous_premade!\n");
		gok_log_leave();
		return FALSE;
	}

	/* display and scan the premade keyboard */
	bReturnCode = gok_main_ds (pKeyboard);

	/* delete all the dynamic keyboards that were on the stack */
	while (index >= 0)
	{
		pKeyboard = arrayKeyboardsToDelete[index];
		if (m_pCurrentKeyboard != pKeyboard)  /* hack to fix potentially nastiness */
		{
			gok_log("deleting a dynamic keyboard with index %d",
			        index);
			gok_keyboard_delete (arrayKeyboardsToDelete[index],FALSE);
		}
		else
		{
			gok_log_x("an attempt was made to delete the currently displayed keyboard!");
		}

		index--;
	}
	
	gok_log_leave();
	return bReturnCode;
}

/**
* gok_main_display_scan_previous
* 
* Display and scan the previous keyboard (pulled from the branch-back-stack).
* Don't push the current keyboard on the branch-back-stack.
*
* returns: TRUE if the previous keyboard was displayed, FALSE if not.
*/
gboolean gok_main_display_scan_previous ()
{
	gboolean bReturnCode;
	GokKeyboard* pKeyboardToDelete;
	
	gok_log_enter();
	
	bReturnCode = TRUE;
	pKeyboardToDelete = NULL;

	if (gok_branchbackstack_is_empty() == TRUE) {
		/* no keyboard on the branch back stack */
		gok_log_leave();
		return FALSE;
	}

	/* are we leaving a dynamically created keyboard? */
	if (m_pCurrentKeyboard->bDynamicallyCreated == TRUE)
	{
		/* delete this keyboard after we switch to the new keyboard */
		pKeyboardToDelete = m_pCurrentKeyboard;
	}
	
	/* display and scan the previous keyboard */
	bReturnCode = gok_main_ds (gok_branchbackstack_pop());
	
	if (pKeyboardToDelete != NULL)
	{
		/* delete the dynamically created keyboard */
		/* make sure we branched back before deleting it */
		if (bReturnCode == TRUE) {
			if (m_pCurrentKeyboard != pKeyboardToDelete) {
				gok_keyboard_delete (pKeyboardToDelete,FALSE);
			}
			else {
				gok_log_x("an attempt was made to delete the currently displayed keyboard!");
			}
		}
	}
	
	gok_log_leave();
	return bReturnCode;
}

GokKeyboard *
gok_main_keyboard_find_byname (const gchar *NameKeyboard)
{
	GokKeyboard *pKB = NULL;
	
	if (NameKeyboard == NULL)
	{
		/* if keyboard name is NULL then use first keyboard in list */
		pKB = m_pKeyboardFirst;
	}
	else
	{
		/* find the keyboard in the list (according to its name) */
		pKB = m_pKeyboardFirst;
		while (pKB != NULL)
		{
			if (g_strcasecmp (pKB->Name, NameKeyboard) == 0)
			{
				/* found the keyboard */
				break;
			}
			else
			{
				pKB = pKB->pKeyboardNext;
			}
		}
	}
	return pKB;
}

/**
* gok_main_display_scan
* @pKeyboard: If this is supplied it takes precedence over the name parameter (useful for dynamic keyboards with the same name)
* @nameKeyboard: Name of the keyboard you want displayed (must be in the list of keyboards.)
# @typeKeyboard: Can be used to describe a runtime keyboard type (e.g. a Menus keyboard)
* @layout: Can be used to specify a particular type of keyboard (Example: center-weighted or upper-left-weighted).
* @shape: Can be used to specify a particular shape of keyboard (Example: square).
*
* Display a keyboard and allow the user to make selections from it.
* The keyboard must have already been created and in the list of keyboards.
* The keyboard is specified by name and keyboard type. A name of NULL means 
* the first keyboard in the list. 
* The previous keyboard is stored on the "branch back stack".
*
* returns: TRUE if keyboard is displayed, FALSE if not.
*/
gboolean 
gok_main_display_scan (GokKeyboard* pKeyboard, gchar* nameKeyboard, KeyboardTypes typeKeyboard, KeyboardLayouts layout, KeyboardShape shape)
{
	/* FIXME we're not using the type params */
	
	GokKeyboard* pKB;
	gboolean bPushed;
	gboolean bReturnCode;

	gok_log_enter();
	g_assert (m_pKeyboardFirst != NULL);

	bPushed = FALSE;
	bReturnCode = FALSE;
	
	if (pKeyboard != NULL)
	{
		pKB = pKeyboard;
	}
	else
	{
		pKB = gok_main_keyboard_find_byname (nameKeyboard);
	}

	if (pKB == NULL)
	{
		/* keyboard not found in list! */
		gok_log_x("keyboard not found in list!");
		gok_log_leave();
		return FALSE;
	}

	/* push the current keyboard on the branch back stack */
	if ((m_pCurrentKeyboard != NULL) && (m_pCurrentKeyboard->bDynamicallyCreated != TRUE))
	{
		bPushed = TRUE;
		gok_branchbackstack_push (m_pCurrentKeyboard);
	}

	gok_log_leave();
	
	/* display the keyboard and start the scanning process */
	bReturnCode = gok_main_ds (pKB);
	
	/* hack */
	if ((bReturnCode == FALSE) && (bPushed == TRUE))
	{
		gok_branchbackstack_pop ();
	}
		
	return bReturnCode;
}

/**
* gok_main_ds
* @pKeyboard: Pointer to the keyboard that will be displayed and scanned.
*
* Does the actual work of displaying and scanning the keyboard.
*
* returns: TRUE if the keyboard was displayed, FALSE if not.
*/
gboolean gok_main_ds (GokKeyboard* pKeyboard)
{
	gok_log_enter();
	g_assert (pKeyboard != NULL);

	/* stop any key flashing */
	gok_feedback_timer_stop_key_flash();
	
	/* stop the scanning process */
	gok_scanner_stop();

	/* raise the window as a precaution */
	gok_main_raise_window ();

	/* create new keys for dynamic keyboards */
	if (pKeyboard->bDynamicallyCreated == TRUE)
	{
		if (gok_keyboard_update_dynamic (pKeyboard) == FALSE)
		{
			gok_log_leave();
			return FALSE;
		}
	}
	
	/* layout the keys on the keyboard */
	if (gok_keyboard_layout (pKeyboard, pKeyboard->LayoutType, 
		 	/*KEYBOARD_SHAPE_WIDE KEYBOARD_SHAPE_SQUARE*/ 
			KEYBOARD_SHAPE_KEYSQUARE
			, FALSE) == FALSE)
	{
		gok_log_leave();
		return FALSE;
	}
		
	/* display the keyboard */
	gok_keyboard_display (pKeyboard, m_pCurrentKeyboard, m_pWindowMain, TRUE);
	m_pCurrentKeyboard = pKeyboard;
	
	/* enable/disable the menu and toolbar keys */
	gok_keyboard_validate_dynamic_keys (gok_main_get_foreground_window_accessible());

	/* if there are command prediction keys - fill them in */
	gok_predictor_predict(m_pCommandPredictor);

	/* initialize the current access method */
	gok_scanner_reset_access_method ();
	
	/* chunk the keyboard if required */
	if (pKeyboard->bRequiresChunking == TRUE)
	{
		gok_chunker_chunk (pKeyboard);
		pKeyboard->bRequiresChunking = FALSE;
	}

	/* highlight the first chunk */
	gok_chunker_highlight_first_chunk();
	
	/* restart the scanning process */
	gok_scanner_start();

	gok_log_leave();
	return TRUE;
}

/**
* gok_main_create_window
* 
* Creates the window that holds the keyboards.
*
* returns: A pointer to the window, NULL if it couldn't be created.
*/
GtkWidget* gok_main_create_window (gboolean is_dock)
{
	GtkWidget *window1;
	GtkWidget *fixed1;

	gok_log_enter();
					
	_screen_height = gdk_screen_get_height (gdk_screen_get_default ());
	_screen_width = gdk_screen_get_width (gdk_screen_get_default ());
	     
	window1 = g_object_connect (gtk_widget_new (gtk_window_get_type (),
				"user_data", NULL,
				"can_focus", FALSE,
				"type", GTK_WINDOW_TOPLEVEL,
				"window-position", GTK_WIN_POS_CENTER,
				"title","GOK",
				"allow_grow", TRUE,
				"allow_shrink", TRUE,
				"border_width", 0,
				"accept_focus", FALSE,
				NULL),
			     "signal::realize", on_window1_realize, is_dock,
			     "signal::destroy", on_window1_destroy, NULL,
			     NULL);

	m_OurResizeWidth = 200;
	m_OurResizeHeight = 100;
	gtk_window_set_default_size (GTK_WINDOW (window1), m_OurResizeWidth, m_OurResizeHeight);


	fixed1 = gtk_fixed_new ();
	gtk_widget_ref (fixed1);
	gtk_object_set_data_full (GTK_OBJECT (window1), "fixed1", fixed1,
                            (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (fixed1);
	gtk_container_add (GTK_CONTAINER (window1), fixed1);

	gtk_signal_connect (GTK_OBJECT (window1), "motion_notify_event",
                      GTK_SIGNAL_FUNC (on_window1_motion_notify_event),
                      NULL);

	gtk_signal_connect (GTK_OBJECT (window1), "leave_notify_event",
                      GTK_SIGNAL_FUNC (on_window1_leave_notify_event),
                      NULL);

	gtk_signal_connect (GTK_OBJECT (window1), "enter_notify_event",
                      GTK_SIGNAL_FUNC (on_window1_enter_notify_event),
                      NULL);

	gtk_widget_add_events (window1, GDK_POINTER_MOTION_MASK);

	gtk_signal_connect (GTK_OBJECT (window1), "size_allocate",
                      GTK_SIGNAL_FUNC (on_window1_size_allocate),
                      NULL);
                      
	gtk_signal_connect (GTK_OBJECT (window1), "delete_event",
                      GTK_SIGNAL_FUNC (on_window1_delete_event),
                      NULL);

	gtk_signal_connect (GTK_OBJECT (window1), "configure_event",
                      GTK_SIGNAL_FUNC (on_window1_configure_event),
                      NULL);
					  
	gtk_signal_connect (GTK_OBJECT (window1), "window_state_event",
                      GTK_SIGNAL_FUNC (on_gok_window_state_event),
                      NULL);

	gok_log_leave();
	return window1;
}

/**
* gok_main_read_rc:
*
* Retreives the name of the directory containing gok.rc from
* GConf and loads it.
*/
static void
gok_main_read_rc ()
{
    GConfClient *gconf_client = NULL;
    GError *gconf_err = NULL;
    gchar *directory_name = NULL;
    gchar *complete_path = NULL;

    gconf_client = gconf_client_get_default ();
    
    directory_name = gconf_client_get_string (gconf_client,
					      GOK_GCONF_RESOURCE_DIRECTORY,
					      &gconf_err);
    if (directory_name == NULL)
    {
	gok_log_x ("Got NULL resource directory key from GConf");
    }
    else if (gconf_err != NULL)
    {
	gok_log_x ("Error getting resource directory key from GConf");
	g_error_free (gconf_err);
    }
    else
    {
        complete_path = g_build_filename (directory_name, "gok.rc", NULL);
	gtk_rc_parse (complete_path);
	g_free (complete_path);
	g_free (directory_name);
    }
}

static GokKeyboard*
gok_append_keyboard_from_file (GokKeyboard *keyboard, const gchar *complete_path)
{
    GokKeyboard *new = gok_keyboard_read (complete_path);

    if (new) 
    {
	new->pKeyboardPrevious = keyboard;
	if (keyboard)
	{
	    new->pKeyboardNext = keyboard->pKeyboardNext;
	    keyboard->pKeyboardNext = new;
	}
    
	/* all predefined keyboards are assumed to be laid out */
	new->bRequiresLayout = FALSE;
	new->bLaidOut = TRUE;
	keyboard = new;
    }

    return keyboard;
}

static GokKeyboard*
gok_main_create_compose_keyboards ()
{
    GokKeyboard *keyboard = NULL, *prev_kbd = NULL;
    GokKeyboard *first = NULL;

    /* create the default "core" keyboard (aka qwerty keyboard) */
    if (gok_data_get_use_xkb_kbd ()) {
	keyboard = first = gok_keyboard_get_core ();
	if (keyboard)
	    keyboard->pKeyboardPrevious = NULL;
	else
	    gok_data_set_use_xkb_kbd (FALSE);

	prev_kbd = keyboard;
    }
    
    /* create the Alpha and Alpha-Freq keyboards */
    keyboard = gok_keyboard_get_alpha ();
    if (prev_kbd) 
    {
	prev_kbd->pKeyboardNext = keyboard;
    }
    if (keyboard)
    {
	keyboard->pKeyboardPrevious = prev_kbd;
	prev_kbd = keyboard;
	if (first == NULL) first = keyboard;
    }
    keyboard = gok_keyboard_get_alpha_by_frequency ();
    if (prev_kbd) 
    {
	prev_kbd->pKeyboardNext = keyboard;
    }
    if (keyboard)
    {
	keyboard->pKeyboardPrevious = prev_kbd;
	prev_kbd = keyboard;
	if (first == NULL) 
	    first = keyboard;
    }
    
    /* read the user-defined compose keyboard, if one is specified */
    if (gok_data_get_compose_keyboard_type () == GOK_COMPOSE_CUSTOM) 
    {
	keyboard = gok_append_keyboard_from_file (keyboard, 
						  gok_data_get_custom_compose_filename ());
	gok_args.custom_compose_kbd_name = keyboard->Name;
	if (first == NULL)
	    first = keyboard;
    }
    
    return first;
}

/**
* gok_main_read_keyboards:
*
* Retrieves the name of the directory containing keyboards from GConf.
* Then loads the keyboards in that directory using
* gok_main_read_keyboards_from_dir.
*/
void gok_main_read_keyboards ()
{
    GokKeyboard *keyboard;
    GConfClient *gconf_client = NULL;
    GError *gconf_err = NULL;
    gchar *directory_name = NULL;
    gboolean gconf_keyboard_directory_error = FALSE;
	
    gconf_client = gconf_client_get_default ();
    
    /* create the compose keyboards first... */
    keyboard = m_pKeyboardFirst = gok_main_create_compose_keyboards ();

    if (m_pKeyboardFirst == NULL)
    {
	gok_main_display_error (_("Can't create a compose keyboard!"));
	exit (1);
    }

    directory_name = gconf_client_get_string (gconf_client,
					      GOK_GCONF_KEYBOARD_DIRECTORY,
					      &gconf_err);
    if (directory_name == NULL)
    {
	gok_log_x ("Got NULL keyboard directory key from GConf");
        gok_main_display_gconf_error ();
	exit (1);
    }
    else if (gconf_err != NULL)
    {
	gok_log_x ("Error getting keyboard directory key from GConf");
	g_error_free (gconf_err);
	gconf_keyboard_directory_error = TRUE;
    }
    else
    {
	/* read the keyboard files from system directory */
	gok_modifier_open();
	keyboard = gok_main_read_keyboards_from_dir (directory_name, keyboard);
    }

    g_free (directory_name);
    
    directory_name = gconf_client_get_string (gconf_client,
					      GOK_GCONF_AUX_KEYBOARD_DIRECTORY,
					      &gconf_err);

    if ((directory_name != NULL) && (strlen (directory_name) > 0) && (gconf_err == NULL))
    {
	/* read the keyboard files from directory_name */
	keyboard = gok_main_read_keyboards_from_dir (directory_name, keyboard);
    }

    g_free (directory_name);
    
    if ( gconf_keyboard_directory_error || (m_pKeyboardFirst == NULL) )
    {
	gok_main_display_error (_("Can't read any keyboards!"));
	exit (1);
    }
}

/**
* gok_main_read_keyboards_from_dir:
* @directory: The name of the keyboard root directory location.
* 
* Reads all the keyboard files from the given directory or a subdirectory that
* is a closer match based on user's language if it exists.  
*
* Returns: A pointer to the first keyboard, NULL if no keyboards could be read.
*/
GokKeyboard* 
gok_main_read_keyboards_from_dir (const char *directory, GokKeyboard *keyboard)
{
	GokKeyboard *pKeyboardFirst = keyboard;
	DIR* pDirectoryKeyboards;
	struct dirent* pDirectoryEntry;
	gchar* filename;
	gchar* complete_path;
	GList* langlist = NULL;
	gchar dirtemp[1024]; /* no reliable way to avoid magic # here */

	gok_log_enter();

	pDirectoryKeyboards = NULL;

	/* find the locale specific keyboards */
	langlist = (GList*) gnome_i18n_get_language_list(NULL);
	if (langlist != NULL) {
		gint i = 0;
		guint l = g_list_length(langlist);
		while ((pDirectoryKeyboards == NULL) && (i < l)) {
			strncpy(dirtemp, directory, 950);
			strncat(dirtemp, G_DIR_SEPARATOR_S, 1);
			strncat(dirtemp, g_list_nth_data(langlist, i),70);
			pDirectoryKeyboards = opendir (dirtemp);
			i++;
		}
	}

	if (pDirectoryKeyboards == NULL) { /* no locale specific directory found */
		pDirectoryKeyboards = opendir (directory); /* use default */
	}
	else {
		directory = dirtemp; /* used later */
	}
		

	/* get a list of all the files from the given directory name */
	if (pDirectoryKeyboards == NULL)
	{
		gok_log_x ("Can't open keyboard directory in gok_main_read_files!");
		gok_log_leave();
		return FALSE;
	}	

	/* seek to the end of the list before appending */
	while (keyboard && keyboard->pKeyboardNext)
	{
	    keyboard = keyboard->pKeyboardNext;
	}

	/* look at each file in the directory */
	while ((pDirectoryEntry = readdir (pDirectoryKeyboards)) != NULL)
	{
		/* is this a keyboard file? */
		filename = pDirectoryEntry->d_name;
		if ( (strlen(filename) >= 4)
		     && (strcmp(filename + (strlen(filename)-4), ".kbd") == 0))
		{
			/* read the keyboard file and add to list */
                        complete_path = g_build_filename (directory, filename,
                                                          NULL);
			gok_log ("complete_path = %s", complete_path);
			keyboard = gok_append_keyboard_from_file (keyboard, complete_path);
			g_free (complete_path);
			
			if (pKeyboardFirst == NULL)
				pKeyboardFirst = keyboard;
		}
	}
	
	gok_log_leave();
	return pKeyboardFirst;
}

/**
* gok_main_initialize_access_methods:
*
* Retrieves the name of the directory containing access methods from GConf.
* Then calls gok_scanner_initialize passing the directory it
* got from GConf.
*/
static void
gok_main_initialize_access_methods (GokArgs *args)
{
    GConfClient *gconf_client = NULL;
    GError *gconf_err = NULL;
    gchar *directory_name = NULL;

    gconf_client = gconf_client_get_default ();
    
    directory_name = gconf_client_get_string (gconf_client,
					      GOK_GCONF_ACCESS_METHOD_DIRECTORY,
					      &gconf_err);
    if (directory_name == NULL) {
		gok_log_x ("Got NULL access method directory key from GConf");
		gok_main_display_gconf_error ();
		exit (2);
    }
    else if (gconf_err != NULL) {
		gok_log_x ("Error getting access method directory key from GConf");
		g_error_free (gconf_err);
 		gok_main_display_error
			(_("could not access method directory key from GConf!"));
		exit(2);		
    }
    else if (!(gok_scanner_initialize(directory_name, 
				      args->accessmethodname, 
				      args->selectactionname, 
				      args->scanactionname))) {
 		gok_main_display_error
			(_("possibly unknown access method!"));
		exit(2);
			
    }
    
    g_free (directory_name);
    
}

/**
* gok_main_initialize_wordcomplete:
*
* Retreives the name of the directory containing dictionary.txt from
* GConf and calls gok_wordcomplete_open passing that directory.
*/
static void
gok_main_initialize_wordcomplete ()
{
    GConfClient *gconf_client = NULL;
    GError *gconf_err = NULL;
    gchar *directory_name = NULL;
    gboolean user_copy = FALSE;
    gchar *user_directory_name = NULL;
    const gchar *homedir = g_get_home_dir ();

    if (homedir) {
	    user_directory_name = g_build_path (G_DIR_SEPARATOR_S, homedir, ".gnome2", "gok", NULL);
    }
    else 
    {
	    gok_log_x ("gok_main_initialize_wordcomplete: no home directory!");
    }

    gconf_client = gconf_client_get_default ();
    
    user_copy = gconf_client_get_bool (gconf_client,
				       GOK_GCONF_PER_USER_DICTIONARY,
				       &gconf_err);
    if (gconf_err != NULL)
    {
	gok_log_x ("Error getting per-user dictionary key from GConf");
	g_error_free (gconf_err);
    }

    directory_name = gconf_client_get_string (gconf_client,
					      GOK_GCONF_DICTIONARY_DIRECTORY,
					      &gconf_err);

    if (directory_name == NULL)
    {
	    gok_log_x ("Got NULL dictionary directory key from GConf");
    }
    else if (gconf_err != NULL)
    {
	    gok_log_x ("Error getting dictionary directory key from GConf");
	    g_error_free (gconf_err);
	    return;
    }
    if (user_copy)
    {
	    /* TODO: should we consider making the dict name at least a #define? */
	    gchar *filename = g_build_filename (user_directory_name, "dictionary.txt", NULL);
	    /* make sure dictionary exists; if not, copy from system. */
	    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
	    {
		    if (!g_file_test (user_directory_name, G_FILE_TEST_EXISTS))
		    {
			    gok_log_x ("creating directory %s", user_directory_name);
			    mkdir (user_directory_name, S_IRUSR | S_IWUSR | S_IXUSR);
			    /* create user gok dir if necessary; */
			    /*  we can discard the retval since we retest below  */
		    }
		    if (g_file_test (user_directory_name, G_FILE_TEST_IS_DIR))
		    {
			    gchar *content = NULL, *dict;
			    GIOChannel *io;
			    
			    dict = g_build_filename (directory_name, "dictionary.txt", NULL);
			    if (dict)
			    {
				    GError *error = NULL;
				    gsize bytes;
				    io = g_io_channel_new_file (filename, "w", &error);
				    if (io && !error && (g_file_get_contents (dict, &content, NULL, &error)))
				    {
					g_io_channel_write_chars (io, content, -1, &bytes, &error);
					g_free (content);
					if (!error) 
					{
					    g_io_channel_shutdown (io, TRUE, &error);
					    directory_name = user_directory_name;
					}
					else
					{
					    g_warning ("error writing user copy of gok system dictionary");
					    g_free (error);
					}
				    }
				    else
				    {
					g_warning ("Error creating user copy of gok system dictionary (%s)", filename);
				    }
			    }
		    }
		    else
		    {
			    g_warning ("Specified user dictionary path %s does not appear to be a directory", user_directory_name);
		    }
	    }
	    else /* existed already */
	    {
		    gok_log ("Found file dictionary.txt in user dir %s", user_directory_name);
		    directory_name = user_directory_name;
	    }
	    g_free (filename);
    }
    
    gok_log ("passing %s to gok-word-completion engine", directory_name);

    if ( !(gok_wordcomplete_open (gok_wordcomplete_get_default (),
				  directory_name)) ||
	 ! (gok_keyslotter_on (gok_wordcomplete_get_default (), KEYTYPE_WORDCOMPLETE))) {
	    gok_log_x ("Error initializing word completion");
	    
    }
    g_free (directory_name);
}

/**
* gok_main_initialize_commandpredictione:
*
* Retreives the name of the directory containing dictionary.txt from
* GConf and calls gok_wordcomplete_open passing that directory.
*/
static void
gok_main_initialize_commandprediction ()
{
	gok_log_enter();
    /* intialize command predictor */
    m_pCommandPredictor = gok_predictor_open();
   /*
    GConfClient *gconf_client = NULL;
    GError *gconf_err = NULL;
    gchar *directory_name = NULL;

    gconf_client = gconf_client_get_default ();
    
    directory_name = gconf_client_get_string (gconf_client,
					      GOK_GCONF_DICTIONARY_DIRECTORY,
					      &gconf_err);
    if (directory_name == NULL)
    {
	gok_log_x ("Got NULL dictionary directory key from GConf");
    }
    else if (gconf_err != NULL)
    {
	gok_log_x ("Error getting dictionary directory key from GConf");
	g_error_free (gconf_err);
    }
    else
    {
	if ( !(gok_wordcomplete_open (directory_name)) ) {
	    gok_log_x ("Error initializing word completion");
	}
	
	g_free (directory_name);
    }
    */
	gok_log_leave();
}

/**
* gok_main_resize_window
* @pWindow: Pointer to the main window.
* @pKeyboard: Pointer to the relevant (pending) keyboard (may be NULL).
* @Width: Width of the new window.
* @Height: Height of the new window.
* 
* Resizes the main window to the given width and height.
* The main window is centered over the the center location in gok_data
* The new window will not be resized so it appears off screen.
* When the new window is resized it generates 2 calls (configure events)to 
* gok_main_store_window_center. If the new window is not centered over the old 
* window then m_countIgnoreConfigure will be set to 2 so that the window 
* center is not changed.
*/
void gok_main_resize_window (GtkWidget* pWindow, GokKeyboard *pKeyboard, gint Width, gint Height)
{
	gint left;
	gint top;
	gint winX;
	gint winY;
	gint frameX;
	gint frameY;
	gint screenX;
	gint screenY;
	GdkRectangle rectFrame;

	/* ensure the window is at least this big */
	if (Width < 50)
	{
		Width = 50;
	}
	if (Height < 50)
	{
		Height = 50;
	}

	/* this may be the editor window so just resize it */
	if (pWindow != m_pWindowMain)
	{
		gtk_window_resize (GTK_WINDOW(pWindow), Width, Height);
		return;
	}
	
	/* let the keyboard know that the resize was caused by us */
	gok_keyboard_set_ignore_resize (TRUE);
	
	/* calculate the upper left corner of the window */
	switch (gok_data_get_dock_type () ) { /* if we're a dock */
	case GOK_DOCK_BOTTOM:
	  top = gdk_screen_height ();
	  left = 0;
	  if ((pKeyboard->expand == GOK_EXPAND_ALWAYS) || 
	      (gok_data_get_expand () && (pKeyboard->expand != GOK_EXPAND_NEVER))) {
		  Width = gdk_screen_width ();
		  gok_keyboard_set_ignore_resize (FALSE);
	  }
	  break;
	case GOK_DOCK_TOP:
	  top = 0;
	  left = 0;
	  if ((pKeyboard->expand == GOK_EXPAND_ALWAYS) || 
	      (gok_data_get_expand () && (pKeyboard->expand != GOK_EXPAND_NEVER))) {
		  Width = gdk_screen_width ();
		  gok_keyboard_set_ignore_resize (FALSE);
	  }
	  break;
	default:
	  top = gok_data_get_keyboard_y() - (Height / 2);
	  left = gok_data_get_keyboard_x() - (Width / 2);
	  break;
	}

	/* add the frame dimension to the window dimension */
	/* get the frame location */
	gdk_window_get_frame_extents ((GdkWindow*)pWindow->window, &rectFrame);
	/* get the window location */
	gdk_window_get_position (pWindow->window, &winX, &winY);

	if ((winX != 0) &&
		(winY != 0))
	{
		frameX = (winX - rectFrame.x);
		frameY = (winY - rectFrame.y);
	}
	else if (gok_data_get_dock_type () == GOK_DOCK_NONE)
	{
		frameX = 5;/* this is usually true */
		frameY = 22;/* this is usually true */
	}
	else {
	        frameX = frameY = 0;
	}
	left -= frameX;
	top -= frameY;
	
	/* make sure the window does not go off screen */
	screenX = gdk_screen_width();
	screenY = gdk_screen_height();
	m_countIgnoreConfigure = 0;	
	if (left < 0)
	{
		m_countIgnoreConfigure = 2;
		left = 0;
	}
	if (top < 0)
	{
		m_countIgnoreConfigure = 2;
		top = 0;
	}
	if ((Width + left + frameX) > screenX)
	{
		m_countIgnoreConfigure = 2;
		left = screenX - Width - (frameX * 2);
	}
	if ((Height + top + frameY) > screenY)
	{
		m_countIgnoreConfigure = 2;
		top = screenY - Height - frameY - (frameX * 2);
	}
	
	/* ensure the window is within the given geometry */
	if (m_bUseGivenGeometry == TRUE)
	{
		if (left < m_GeometryX)
		{
			left = m_GeometryX;
		}
		if (top < m_GeometryY)
		{
			top = m_GeometryY;
		}
	}

	/* move/resize the main window */
	gdk_window_move_resize ((GdkWindow*)pWindow->window, left, top, Width, Height);

	if (gok_data_get_dock_type () != GOK_DOCK_NONE) {
		/* if this is a "never expand" keyboard, unset struts! */
		if (pKeyboard && (pKeyboard->expand == GOK_EXPAND_NEVER))
			gok_main_update_struts (Width, 1, left, top);
		else
			gok_main_update_struts (Width, Height, left, top);
	}

	/* store these values */
	m_OurResizeWidth = Width;
	m_OurResizeHeight = Height;
}

void
gok_main_set_wm_dock (gboolean is_dock)
{
  Atom atom_type[1], atom_window_type;
  GtkWidget *widget = gok_main_get_main_window ();

  if (widget) gtk_widget_hide (widget);
  gdk_error_trap_push ();
  atom_window_type = gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE");

  if (is_dock) 
	  atom_type[0] = gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE_DOCK");
  else 
	  atom_type[0] = gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE_NORMAL");

  XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
		   GDK_WINDOW_XWINDOW (widget->window), 
		   atom_window_type,
		   XA_ATOM, 32, PropModeReplace,
		   (guchar *) &atom_type, 1);

  gdk_error_trap_pop ();

  if (widget) gtk_widget_show (widget);

  if (m_pCurrentKeyboard) {
	  gok_keyboard_display (m_pCurrentKeyboard, m_pCurrentKeyboard,
				gok_main_get_main_window(), TRUE); 
          /* this should move us to appropriate screen edge */
  }

}


enum {
	STRUT_LEFT = 0,
	STRUT_RIGHT = 1,
	STRUT_TOP = 2,
	STRUT_BOTTOM = 3,
	STRUT_LEFT_START = 4,
	STRUT_LEFT_END = 5,
	STRUT_RIGHT_START = 6,
	STRUT_RIGHT_END = 7,
	STRUT_TOP_START = 8,
	STRUT_TOP_END = 9,
	STRUT_BOTTOM_START = 10,
	STRUT_BOTTOM_END = 11
};

/*
 * TODO: GtkDoc this!
 */
void
gok_main_update_struts (gint width, gint height, gint x, gint y)
{
	GtkWidget *widget = gok_main_get_main_window ();
	Atom atom_strut = gdk_x11_get_xatom_by_name ("_NET_WM_STRUT");
	Atom atom_strut_partial = gdk_x11_get_xatom_by_name ("_NET_WM_STRUT_PARTIAL");
	guint32 struts[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	switch (gok_data_get_dock_type ()) {
	case GOK_DOCK_TOP:
		struts[STRUT_TOP] = height;
		struts[STRUT_TOP_START] = x;
		struts[STRUT_TOP_END] = width;
		break;
	case GOK_DOCK_BOTTOM:
		struts[STRUT_BOTTOM] = height;
		struts[STRUT_BOTTOM_START] = x;
		struts[STRUT_BOTTOM_END] = width;
		break;
	default:
		return;
		break;
	}
	gdk_error_trap_push ();
	/* this means that we are responsible for placing ourselves appropriately on screen */
	if (widget && widget->window)
	{
		XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
				 GDK_WINDOW_XWINDOW (widget->window), 
				 atom_strut,
				 XA_CARDINAL, 32, PropModeReplace,
				 (guchar *) &struts, 4);
		XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
				 GDK_WINDOW_XWINDOW (widget->window), 
				 atom_strut_partial,
				 XA_CARDINAL, 32, PropModeReplace,
				 (guchar *) &struts, 12);
	}
	gdk_error_trap_pop ();
}

/**
* gok_main_get_our_window_size
* @pWidth: Pointer to a variable that will hold the window width.
* @pHeight: Pointer to a variable that will hold the window height.
*
* Retreives the width and height that we last resized the window to.
*/
void gok_main_get_our_window_size (gint* pWidth, gint* pHeight)
{
	*pWidth = m_OurResizeWidth;
	*pHeight = m_OurResizeHeight;
}

/**
* gok_main_store_window_center
* 
* Stores in gok_data the center location of the current keyboard.
*/
void gok_main_store_window_center ()
{
	gint winX;
	gint winY;
	gint winWidthCurrent;
	gint winHeightCurrent;

	/* don't store the window center if geometry is specified */
	if (m_bUseGivenGeometry == TRUE)
	{
		return;
	}
	
	/* this flag may be set in gok_main_resize_window so ignore this call */
	if (m_countIgnoreConfigure != 0)
	{
		m_countIgnoreConfigure--;
		return;
	}

	/* get the center of the current window */	
	gdk_window_get_position (m_pWindowMain->window, &winX, &winY);
	gdk_window_get_size (m_pWindowMain->window, &winWidthCurrent, &winHeightCurrent);
	winX +=  winWidthCurrent / 2;	
	winY +=  winHeightCurrent / 2;

	/* update the gok_data with keyboard center */
	gok_data_set_keyboard_x (winX);
	gok_data_set_keyboard_y (winY);
}

/**
* gok_main_close
* 
* Delete any keyboards that were created.
* This must be called at the end of the program.
*/
void
gok_main_close()
{
	/* NOTE: the order in which things are cleaned up is very important */

	GokKeyboard* pKeyboard;
	GokKeyboard* pKeyboardTemp;

	/* unhook listeners/callbacks */
	gok_spy_deregister_windowchangelistener ((void *)gok_main_window_change_listener);
	gok_spy_deregister_objectstatelistener ((void *)gok_main_object_state_listener);
	/* the call to deregister_mousebuttonlistener is in on_window1_destroy */
	
	gok_spy_stop();
	
	gok_log("BEFORE DELETING KEYBOARDS");
	gok_log("Keyboards news - deletes: [%d]",gok_keyboard_get_keyboards());
	gok_log("Stack pushes - pops:      [%d]",gok_branchbackstack_pushes_minus_pops());

	pKeyboard = gok_main_get_first_keyboard ();
	while (pKeyboard != NULL)
	{
		pKeyboardTemp = pKeyboard->pKeyboardNext;
		gok_keyboard_delete (pKeyboard, TRUE);
		pKeyboard = pKeyboardTemp;
	}
	
	/* TODO delete the current keyboard? */

	gok_log("AFTER DELETING KEYBOARDS");
	gok_log("Keyboards news - deletes: [%d]",gok_keyboard_get_keyboards());
	gok_log("Stack pushes - pops:      [%d]",gok_branchbackstack_pushes_minus_pops());

	gok_spy_close();

	/* note, call all closing functions that delete UI before calling gok_settingsdialog_close */
	gok_scanner_close(); /* deletes UI */

	/* gok_settingsdialog_close(); */
	
/**
 * the above, in context of the following calls,  is fundamentally broken, in that it 
 * destroys widgets which are either already destroyed, or are about to be destroyed below. 
 * Anyhow, any memory it uses will be freed on exit.   Better not to call it until the 
 * whole UI cleanup code can get fixed. - wph
 **/
	
	gok_data_close();

	closeSwitchApi();
	gok_sound_shutdown();
	gok_modifier_close();
	gok_action_close();
	gok_feedback_close();
	gok_control_button_callback_close();
	gok_wordcomplete_close(gok_wordcomplete_get_default ());
	gok_predictor_close(m_pCommandPredictor);
	
	_exit(0);
}

/**
* gok_main_display_error
* @ErrorString: Fatal error message.
* 
* Displays a fatal error dialog (modal).
*/
void gok_main_display_error (gchar* ErrorString)
{
	GtkWidget* pDialog;
	gchar buffer [1024];
	
	strcpy (buffer, _("Sorry, GOK can't run because:\n"));
	strcat (buffer, ErrorString);
	
	pDialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		buffer);
	
	gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Fatal Error"));
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
}

/**
* gok_main_display_gconf_error
* 
* Displays a gconf error message.
*/
void gok_main_display_gconf_error ()
{
    gok_main_display_error ( _("GOK uses GConf 2 to store its settings and requires certain settings to be in GConf to run.  GOK is currently unable to retrieve those settings.  If this is the first time that you have run gok after installing it you may need to restart gconfd, you can use this command: 'gconftool-2 --shutdown' or log out and back in."));
}

/**
* gok_main_display_geometry_error
* 
* Displays a dialog informing user that the given geometry was not correct.
*/
static void
gok_main_display_geometry_error ()
{
	GtkWidget* dialog;
        
	dialog = gtk_message_dialog_new (NULL,
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_ERROR,
	                                 GTK_BUTTONS_CLOSE,
	                                 _("Currently GOK requires that the x, y, width and height all be given.  Sorry, your geometry specification will not be used."));
        
	gtk_window_set_title (GTK_WINDOW (dialog), _("gok: Unsupported geometry specification"));

	g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
	                          G_CALLBACK (gtk_widget_destroy),
	                          GTK_OBJECT (dialog));

	gtk_widget_show_all (dialog);
}

/**
* gok_main_get_use_geometry
* 
* returns: TRUE if the user has specified window geometry for the GOK.
* Returns FALSE if the GOK can use the whole screen.
*/
gboolean gok_main_get_use_geometry ()
{
	return m_bUseGivenGeometry;
}

/**
* gok_main_get_geometry
* @pRectangle: Pointer to a rectangle that will be populated with screen
* geometry that should be used by the GOK.
*/
void gok_main_get_geometry (GdkRectangle* pRectangle)
{
	g_assert (pRectangle != NULL);
	g_assert (m_bUseGivenGeometry == TRUE);

	pRectangle->x = m_GeometryX;
	pRectangle->y = m_GeometryY;
	pRectangle->width = m_GeometryWidth;
	pRectangle->height = m_GeometryHeight;
}

gboolean
gok_main_window_contains_pointer (void)
{
	gint x, y;
	GdkModifierType mask;
	GtkWidget *widget = gok_main_get_main_window ();
	if (gdk_window_get_pointer (widget->window, &x, &y, &mask) != NULL)
		return TRUE;
	return FALSE;
}

Display *
gok_main_display (void)
{
	if (gok_main_get_main_window () != NULL)
		return GDK_WINDOW_XDISPLAY (gok_main_get_main_window ()->window);
	return GDK_DISPLAY ();
}



/**
* gok_main_get_access_method_override
*
* Call this to see if an access method was passed in from the command line
*
* returns: name of access method or NULL
*/
gchar* 
gok_main_get_access_method_override (void)
{
	return gok_args.accessmethodname;
}

/**
* gok_main_get_scan_override
*
* Call this to see if the "scan" action name was passed in from the command line
*
* returns: name of "scan" or "movehighlighter" action or NULL
*/
gchar* 
gok_main_get_scan_override (void)
{
	return gok_args.scanactionname;
}

/**
* gok_main_get_select_override
*
* Call this to see if the "select" action name was passed in from the command line
*
* returns: name of "select" or "outputselected" action or NULL
*/
gchar* 
gok_main_get_select_override (void)
{
	return gok_args.selectactionname;
}

/**
* gok_main_get_valuatorsensitivity_override
*
* Call this to see if a valuator sensitivity multiplier was passed in from the command line
*
* returns: gdouble multiplier
*/
gdouble
gok_main_get_valuatorsensitivity_override (void)
{
	return gok_args.valuator_sensitivity;
}

/**
* gok_main_get_geometry
*
* Call this to see if the extras argument was passed in from the command line
*
* returns: gboolean
*/
gboolean 
gok_main_get_extras (void)
{
	return gok_args.use_extras;
}

/**
* gok_main_get_login
*
* Call this to see if the extras argument was passed in from the command line
*
* returns: gboolean
*/
gboolean 
gok_main_get_login (void)
{
	return gok_args.is_login;
}

/**
* gok_main_get_inputdevice_name
* 
* returns: input device name
*/
gchar*
gok_main_get_inputdevice_name ()
{
	return gok_args.inputdevicename;
}

/**
* gok_main_has_xkb_extension
*
* Call this to see if xkb is enabled.
* 
* returns: gboolean
*/
static gboolean
gok_main_has_xkb_extension ()
{
	/* TODO/revisit - is this a good check? */
	if (gok_keyboard_get_xkb_desc() != NULL)
	{
		return TRUE;
	}
	return FALSE;
}

static gboolean
gok_main_check_sticky_keys (GtkWidget *widget)
{
	int op_rtn, event_rtn, error_rtn;
	Display *display = GDK_WINDOW_XDISPLAY (widget->window);
	GtkWidget *dialog;
	gboolean xkb_ok = FALSE;

	g_assert (display);
	if (XkbQueryExtension (display,
			       &op_rtn, &event_rtn, &error_rtn, NULL, NULL)) {
		XkbDescRec *xkb;
		xkb = XkbGetMap (display, XkbAllComponentsMask, XkbUseCoreKbd);
		if (xkb) {
			XkbGetControls (display,
					XkbControlsEnabledMask,
					xkb);
			xkb_ok = TRUE;
		} 
		if (!xkb || !(xkb->ctrls->enabled_ctrls & XkbStickyKeysMask)) {
			gboolean sticky_is_set = FALSE;
			/* Turn on sticky keys and warn user! */
			if (!gok_args.is_login) {
				gboolean keyboard_access;
				GConfClient *gconf_client = gconf_client_get_default ();
				gok_gconf_get_bool (gconf_client, KEYBOARD_ACCESSIBILITY_ENABLE_KEY, &keyboard_access);
				if (!keyboard_access)
					gok_gconf_set_bool (gconf_client, KEYBOARD_ACCESSIBILITY_ENABLE_KEY, TRUE); /* set keyboard accessibility key before setting sticky keys */
				gok_gconf_set_bool (gconf_client, KEYBOARD_ACCESSIBILITY_STICKY_KEYS_KEY, 
						    TRUE); /* turn on gconf key */
				xkb_ok = TRUE;
				sticky_is_set = TRUE;
			}
			else if (XkbChangeEnabledControls (display,
						      XkbUseCoreKbd,
						      XkbStickyKeysMask,
						      XkbStickyKeysMask)) {
				xkb_ok = TRUE;
				sticky_is_set = TRUE;
				g_message ("Sticky keys enabled.");
			}
			else
			{
				g_warning (_("GOK may not work properly, because it could not enable your desktop's 'sticky keys' feature."));
			}
			if (!gok_args.is_login && sticky_is_set) {
				dialog = gtk_message_dialog_new (NULL,
								 0,
								 GTK_MESSAGE_INFO,
								 GTK_BUTTONS_OK,
								 _("GOK has enabled Sticky Keys, which it requires.\n"));
				
				/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
				g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
							  G_CALLBACK (gtk_widget_destroy),
							  GTK_OBJECT (dialog));
				
				/* reset hint to avoid self-occlusion */
				gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_NORMAL);
				gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
				gtk_widget_show_all (dialog);
			}
		}
		XkbFreeKeyboard (xkb, 0, True);
	}
	else {
		if (!gok_args.is_login) {
                /* post an error dialog */
			dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("GOK cannot run because XKB display extension is missing.\n"));
			g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  GTK_OBJECT (dialog));
			gtk_widget_show_all (dialog);
		}
		else
			g_warning ("XKB extension not present or non-functional.  GOK will exit.");
		return FALSE;
	}
	return TRUE;
}



static GtkWidget* acd = NULL; /* accessibility check dialog */

static GtkWidget* _corepointer_warning = NULL; /* core pointer warning dialog */

void check_accessibility_cb ( GObject* o, gpointer* data )
{
	GnomeClient *client;

	gtk_widget_destroy(acd);	
	
	if (strcmp((gchar*)data,"logout") == 0) {
		gok_gconf_set_bool ( gok_data_get_gconf_client(),
			GCONF_ACCESSIBILITY_KEY, TRUE );
		if (!(client = gnome_master_client())) {
			gok_main_close();
		}
 		gnome_client_request_save (client, GNOME_SAVE_GLOBAL, TRUE,
			GNOME_INTERACT_ANY, FALSE, TRUE); /* code borrowed from 
			gnome-control-center/ capplets/ accessibility/ at-properties/ 
			main.c */
	}
	else if (strcmp((gchar*)data,"quit") == 0) {
		gok_main_close();
		_exit (0);
	}
	else if (strcmp((gchar*)data,"continue") == 0) {
		/* maybe change gok somehow to show user weakened status */
	}

}

void
gok_main_help_cb (gpointer data) 
{
	GError *error = NULL;
	gchar *helpuri = data;
	/* TODO: detect error launching help, and give informative message */
	gnome_help_display_desktop (NULL, "gok", helpuri, NULL, &error);
}
 
void
gok_main_warn_if_corepointer_mode (gchar *message_prefix, gboolean always)
{
	if ((always && !_corepointer_warning) || gok_scanner_current_state_uses_corepointer ()) 
	{
		GtkWidget*	button;
		gint         response_id;
		gboolean 	returnCode = TRUE;
		gchar *message = g_strconcat (message_prefix, "\n\n",
			_("This means that the device you are using to operate GOK"
			"\nis also controlling the system pointer (or \'mouse pointer\')."
			"\nConflicts with applications\' use of the pointer may interfere"
			"\nwith your ability to use applications or GOK.\n"
			"\nWe strongly recommend configuring your input device as an"
                        "\n\'Extended\' input device instead; see GOK Help for more information."), NULL);
		
		if (_corepointer_warning == NULL)
		{
			_corepointer_warning = gtk_message_dialog_new (
				NULL, 
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, 
				GTK_BUTTONS_NONE,			
				message);
			g_free (message);
			
			button = gtk_button_new_from_stock(GTK_STOCK_HELP);
			gtk_container_add (GTK_CONTAINER (GTK_DIALOG(_corepointer_warning)->action_area),
					   button);
			g_signal_connect_swapped (G_OBJECT (button), "clicked", 
						  G_CALLBACK (gok_main_help_cb), "gok.xml");
			
			button = gtk_button_new_from_stock(GTK_STOCK_PREFERENCES);
			gtk_container_add (GTK_CONTAINER (GTK_DIALOG(_corepointer_warning)->action_area),
					   button);
			g_signal_connect (G_OBJECT (button), "clicked", 
					  G_CALLBACK (gok_settingsdialog_show), NULL);
			
			button = gtk_button_new_from_stock(GTK_STOCK_OK);
			gtk_container_add (GTK_CONTAINER (GTK_DIALOG(_corepointer_warning)->action_area),
					   button);
			g_signal_connect_swapped (G_OBJECT (button), "clicked", 
						  G_CALLBACK (gtk_widget_destroy), _corepointer_warning);
			
			/* reset hint to avoid self-occlusion */
			gtk_window_set_type_hint (GTK_WINDOW (_corepointer_warning), GDK_WINDOW_TYPE_HINT_NORMAL);
			gtk_window_set_position (GTK_WINDOW (_corepointer_warning), GTK_WIN_POS_CENTER);
			gtk_widget_show_all (_corepointer_warning);
		}
	}
}

void
gok_main_check_accessibility ()
{
    gboolean     accessibility_on;
    GConfClient* client;
	
	client = gok_data_get_gconf_client();

    /* check if accessibility flag is TRUE */
	gok_gconf_get_bool ( client, GCONF_ACCESSIBILITY_KEY, &accessibility_on );
        
    if (!accessibility_on) {
		GtkWidget*	button;
		gint         response_id;
		gboolean 	returnCode = TRUE;
		
		acd = gtk_message_dialog_new (
			NULL, 
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_NONE,			
			_("Assistive Technology Support Is Not Enabled." 
		"\n\n" 
		"You can start GOK without enabling support for assistive technologies. "
		"However, some of the features of the application might not be available." 
		"\n\n"
		"To enable support for assistive technologies "
		"and log in to a new session with the change enabled, "
		"click "
		"Enable and Log Out" "."
		"\n\n"
		"To continue using GOK, " 
		"click "
		"Continue" "."
		"\n\n"
		"To quit GOK, " 
		"click " 
		"Close" "." 
		"\n\n"));

		button = gtk_button_new_with_label(_("Enable and Log Out"));
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG(acd)->action_area),
						  button);
		g_signal_connect(G_OBJECT (button), "clicked", 
			G_CALLBACK (check_accessibility_cb), (gpointer) "logout");
			
		button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG(acd)->action_area),
						  button);
		g_signal_connect(G_OBJECT (button), "clicked", 
			G_CALLBACK (check_accessibility_cb), (gpointer) "quit");

		button = gtk_button_new_with_label(_("Continue"));
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG(acd)->action_area),
						  button);
		g_signal_connect(G_OBJECT (button), "clicked", 
			G_CALLBACK (check_accessibility_cb), (gpointer) "continue");
						  
		/* reset hint to avoid self-occlusion */
		gtk_window_set_type_hint (GTK_WINDOW (acd), GDK_WINDOW_TYPE_HINT_NORMAL);
		gtk_window_set_position (GTK_WINDOW (acd), GTK_WIN_POS_CENTER);
		gtk_widget_show_all (acd);
		
	}
}
