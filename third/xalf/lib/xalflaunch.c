/*
 * xalf_launch - overloaded Xlib function XMapWindow
 * 
 * To be used with xalf. 
 *
 * Peter Åstrand <altic@lysator.liu.se> 2000. GPLV2. 
 *
 * Based on scwm_set_pid_property.c (C) 1999 Toby Sargeant and Greg J. Badros
 * 
 * */
 
#undef DEBUG 
/* #define DEBUG /**/

#ifdef DEBUG
#   define DPRINTF(args) fprintf args
#else
#   define DPRINTF(args) 
#endif

#include "config.h"
#include <dlfcn.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>


#define PID_PROPERTY_NAME "XALF_LAUNCH_PID"
#define PRELOAD_LIBRARY LIBDIR"/libxalflaunch.so.0"

static void *pfXMapWindow = NULL;
static void *pfXMapRaised = NULL;
static void restore_env();
static long int launch_pid = 0;

void
_init () {
    char *pid_string;
    void *dlh = NULL;

    DPRINTF ((stderr, "_init xalf_launch\n"));

    pid_string = getenv (PID_PROPERTY_NAME);
    if (pid_string)
	launch_pid = atol (pid_string);
    DPRINTF ((stderr, "launch_pid is %ld\n", launch_pid));
    if (launch_pid == 0) {
        restore_env ();
    }

    dlh = dlopen (NULL, RTLD_GLOBAL | RTLD_NOW);
    if (dlsym (dlh, "XSync") == NULL) {
        DPRINTF ((stderr, "no XSync\n"));
        return;
    }

    dlh = dlopen ("libX11.so", RTLD_GLOBAL | RTLD_NOW);
    if (dlh == NULL) 
  	dlh = dlopen ("libX11.so.6", RTLD_GLOBAL | RTLD_NOW); 
    if (dlh == NULL)
	fprintf (stderr, "libxalflaunch: %s\n", dlerror ());
    if (dlh != NULL) {
        pfXMapWindow = dlsym (dlh,"XMapWindow");
        pfXMapRaised = dlsym (dlh,"XMapRaised");
        dlclose (dlh);
    }

    DPRINTF ((stderr, "pfXMapWindow is at %p\n", pfXMapWindow));

    restore_env ();
}


typedef int (*XMType)(Display* display, Window w);


extern int 
XMapWindow (Display* display, Window w)                               
{                                                                               
    int i = 0;
    
    if (pfXMapWindow != NULL)
        i = ((XMType)pfXMapWindow)(display, w); 

    
    /* we don't want to kill neither 0 nor init */
    if (launch_pid > 1) {
        DPRINTF ((stderr, "XMapWindow: Sending signal to process %d\n", launch_pid));
        kill (launch_pid, SIGUSR1);
        launch_pid = 0;
    }
	
    return i;                                                                   
}

extern int 
XMapRaised (Display* display, Window w)                               
{                                                                               
    int i = 0;
    
    if (pfXMapRaised != NULL)
        i = ((XMType)pfXMapRaised)(display, w); 
    
    /* we don't want to kill neither 0 nor init */
    if (launch_pid > 1) {
        DPRINTF ((stderr, "XMapRaised: Sending signal to process %d\n", launch_pid));
    
        kill (launch_pid, SIGUSR1);
        launch_pid = 0;
    }
	
    return i;                                                                   
}


#ifdef HAVE_UNSETENV
 #define PRELOAD_UNSETTER unsetenv ("LD_PRELOAD")
 #define PID_UNSETTER unsetenv (PID_PROPERTY_NAME)
#else
 #define PRELOAD_UNSETTER putenv ("LD_PRELOAD=")
/* If we can't remove this variable name, lets leave it */
 #define PID_UNSETTER 
#endif

/* Remove our PRELOAD_LIBRARY from LD_PRELOAD */
void
restore_env()
{
#ifdef MULTI_PRELOAD
    char *envstring = NULL;
    char *newenv = NULL;
    char *orgenv = NULL;
    char *p = NULL;
    char *rest = NULL;

    orgenv = getenv ("LD_PRELOAD");

    if (!orgenv)
	return;
    
    /* Copy the string. LD_PRELOAD= is 11 chars. */
    envstring = malloc (strlen(orgenv) + 11);
    strcpy (envstring, "LD_PRELOAD=");
    strcat (envstring, orgenv);
    newenv = envstring + 11;

    /* Find the substring PRELOAD_LIBRARY */
    p = strstr (newenv, PRELOAD_LIBRARY);

    if (!p) {
	free (envstring);
	return;
    }

    /* Find the string after the next colon */
    rest = strchr (p, ':');
    if (rest) 
	rest++;

    /* Cut off the string before our substring */
    if (p != newenv) 
	p--;
    *p = '\0';

    if (*newenv && rest)
	strcat (newenv, ":");
    
    if (rest)
	strcat (newenv, rest);

    if (! (*newenv) )
	{
	    DPRINTF ((stderr, "unsetting LD_PRELOAD\n")); 
	    PRELOAD_UNSETTER;
	}
    else
	{
	    DPRINTF ((stderr, "putenv with %s\n", envstring));    
	    putenv (envstring);
	}
    
#else
    
    PRELOAD_UNSETTER;    

#endif

    PID_UNSETTER;
    return;
}

#undef PRELOAD_UNSETTER

