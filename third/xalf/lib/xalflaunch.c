/*
 * xalf_launch - overloaded Xlib function XMapWindow and XMapRaised
 * 
 * To be used with xalf. 
 *
 * Peter Åstrand <astrand@lysator.liu.se> 2001. GPLV2. 
 *
 * Based on scwm_set_pid_property.c (C) 1999 Toby Sargeant and Greg J. Badros
 * 
 * This library is hopefully threadsafe. Please report bugs. 
 * */
 
#undef DEBUG 
/* Uncomment below for debugging */
/* #define DEBUG */

#ifdef DEBUG
#   define DPRINTF(args) fprintf args
#else
#   define DPRINTF(args) 
#endif

#define _GNU_SOURCE

#include "config.h"
#include <dlfcn.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define PID_ENV_NAME "XALF_LAUNCH_PID"
#define SAVED_PRELOAD_NAME "XALF_SAVED_PRELOAD"

#ifdef RLD_LIST
#define PRELOAD_VARIABLE RLD_LIST
#define UNSET_VALUE "DEFAULT"
#else
#define PRELOAD_VARIABLE "LD_PRELOAD"
#define UNSET_VALUE ""
#endif

/* Prototypes */
static void restore_env();
static long int launch_pid = 0;


/* Init function. I wish there were some better way of doing this... */
#ifdef __FreeBSD__
static void _init(void) __attribute__ ((section (".init")));
#endif /* __FreeBSD__ */

/* Athena mod -- force use of _init, since __attribute__ ((constructor))
 * extension does not seem to work for shared libraries.
 */
#if !defined(linux) && defined __GNUC__ && ( ( __GNUC__ == 2 ) && ( __GNUC_MINOR__ >= 96) || ( __GNUC__ >= 3) )
void initialize (void) __attribute__ ((constructor));
void initialize (void)
#else
void
_init () 
#endif
{
    char *pid_string = NULL;
    void *dlh = NULL;

    DPRINTF ((stderr, "libxalflaunch: _init\n"));

    pid_string = getenv (PID_ENV_NAME);
    if (pid_string) 
	launch_pid = atol (pid_string);
    DPRINTF ((stderr, "libxalflaunch: launch_pid is %ld\n", launch_pid));

    if (launch_pid == -1) 
	{
	    /* The xalf wrapper was only testing for our existence. */
	    DPRINTF ((stderr, "libxalflaunch: Exiting immediately from _init()\n"));
	    return;
	}
    else if (launch_pid == 0) 
	{
	    /* PID_PROPERTY_NAME was not defined. 
	       Restore env, so that we don't get loaded again. */
 	    restore_env ();
	    return;
	}

    dlh = dlopen (NULL, RTLD_GLOBAL | RTLD_NOW);
    if (dlsym (dlh, "XSync") == NULL) 
	{
	    /* This is not an X11 app. Maybe our app is started via a shellscript. 
	       Hold on... */
	    DPRINTF ((stderr, "libxalflaunch: No XSync\n"));
	    return;
	}
    
    restore_env ();
}


extern int 
XMapWindow (Display* display, Window w)                               
{                                                                 
    /* declare fptr as static pointer to function returning int */              
    static int (*fptr)() = 0;

    DPRINTF ((stderr, "libxalflaunch: XMapWindow\n"));

    /* we don't want to kill neither 0 nor init */
    if (launch_pid > 1) 
	{
	    DPRINTF ((stderr, "libxalflaunch: Sending signal to process %d\n", launch_pid));
	    kill (launch_pid, SIGUSR1);
	    launch_pid = 0;
	}
    
    if (fptr == 0) 
	/* This is the first call to XMapWindow. Fetch function pointer. */
	{
#ifdef RTLD_NEXT
	    fptr = (int (*)())dlsym (RTLD_NEXT, "XMapWindow");
#else
	    /* This platform does not support RTLD_NEXT (Irix, for example). 
	       Use dlopen() instead. */
	    void *dlh = NULL;

	    dlh = dlopen ("libX11.so", RTLD_GLOBAL | RTLD_NOW);
	    if (dlh == NULL) 
		dlh = dlopen ("libX11.so.6", RTLD_GLOBAL | RTLD_NOW); 
	    if (dlh == NULL)
		fprintf (stderr, "libxalflaunch: %s\n", dlerror ());
	    if (dlh != NULL) {
		fptr = (int (*)())dlsym (dlh, "XMapWindow");
	    }
	    
	    DPRINTF ((stderr, "libxalflaunch: XMapWindow is at %p\n", fptr));
#endif
	    if (fptr == NULL) 
		{
		    fprintf (stderr, "libxalflaunch: dlsym: %s\n", dlerror());
		    /* Zero means an error in Xlib */
		    return 0;
		}
	}

    /* Call XMapWindow and return result. */
    return (*fptr)(display, w);
}


extern int 
XMapRaised (Display* display, Window w)                               
{                            
    /* declare fptr as static pointer to function returning int */
    static int (*fptr)() = 0;

    DPRINTF ((stderr, "libxalflaunch: XMapRaised\n"));

    /* we don't want to kill neither 0 nor init */
    if (launch_pid > 1) 
	{
	    DPRINTF ((stderr, "libxalflaunch: Sending signal to process %d\n", launch_pid));
	    kill (launch_pid, SIGUSR1);
	    launch_pid = 0;
	}
    
    if (fptr == 0) 
	/* This is the first call to XMapRaised. Fetch function pointer. */
	{
#ifdef RTLD_NEXT
	    fptr = (int (*)())dlsym (RTLD_NEXT, "XMapRaised");
#else
	    /* This platform does not support RTLD_NEXT (Irix, for example). 
	       Use dlopen() instead. */
	    void *dlh = NULL;

	    dlh = dlopen ("libX11.so", RTLD_GLOBAL | RTLD_NOW);
	    if (dlh == NULL) 
		dlh = dlopen ("libX11.so.6", RTLD_GLOBAL | RTLD_NOW); 
	    if (dlh == NULL)
		fprintf (stderr, "libxalflaunch: %s\n", dlerror ());
	    if (dlh != NULL) {
		fptr = (int (*)())dlsym (dlh, "XMapRaised");
	    }
	    
	    DPRINTF ((stderr, "libxalflaunch: XMapRaised is at %p\n", fptr));
#endif
	    if (fptr == NULL) 
		{
		    fprintf (stderr, "libxalflaunch: dlsym: %s\n", dlerror());
		    /* Zero means an error in Xlib */
		    return 0;
		}
	}

    /* Call XMapRaised and return result. */
    return (*fptr)(display, w);
}


#ifdef HAVE_UNSETENV
 #define PRELOAD_UNSETTER unsetenv (PRELOAD_VARIABLE);
 #define SAVED_PRELOAD_UNSETTER unsetenv (SAVED_PRELOAD_NAME);
 #define PID_UNSETTER unsetenv (PID_ENV_NAME);

#else
 #define PRELOAD_UNSETTER \
     if (putenv (PRELOAD_VARIABLE "=" UNSET_VALUE)) \
         fprintf (stderr, "libxalflaunch: unsetting %s failed\n", PRELOAD_VARIABLE);
 #define SAVED_PRELOAD_UNSETTER \
     if (putenv (SAVED_PRELOAD_NAME"=")) \
         fprintf (stderr, "libxalflaunch: unsetting "SAVED_PRELOAD_NAME" failed\n");
 #define PID_UNSETTER \
     if (putenv (PID_ENV_NAME"=")) \
         fprintf (stderr, "libxalflaunch: unsetting "PID_ENV_NAME" failed\n");
#endif /* HAVE_UNSETENV */


/* Unset or restore LD_PRELOAD
   and unset PID_ENV_NAME */
static void restore_env()
{
    char *saved_preload = NULL;
    char *new_preload = NULL;

    saved_preload = getenv (SAVED_PRELOAD_NAME);

    if (saved_preload)
	/* LD_PRELOAD was set before Xalf was called. Restore it. */
	{
	    DPRINTF ((stderr, "libxalflaunch: Restoring %s=%s\n", PRELOAD_VARIABLE, saved_preload));
	    /* Allocate space for the variable, =, value, and \0 */
	    new_preload = malloc (strlen (PRELOAD_VARIABLE) + strlen (saved_preload) + 2);
	    if (new_preload == NULL)
		/* Malloc failed. */
		{
		    fprintf (stderr, "libxalflaunch: malloc failed\n");
		}
	    else 
		{
		    strcpy (new_preload, PRELOAD_VARIABLE"=");
		    strcat (new_preload, saved_preload);
		    /* Note: saved_preload+11 becomes a part of the new environment, 
		       and should not be free:d. */
		    if (putenv (new_preload)) 
			fprintf (stderr, "libxalflaunch: putenv failed\n");
		}
	    
	    /* Unset the saved LD_PRELOAD. */
	    SAVED_PRELOAD_UNSETTER;
	}
    else
	{
	    DPRINTF ((stderr, "libxalflaunch: Unsetting %s\n", PRELOAD_VARIABLE));
	    PRELOAD_UNSETTER;
	}
    
    /* Unset PID_ENV_NAME */
    PID_UNSETTER;
}
