#include <X11/Wc/COPY.h>

/*
* SCCS_data: %Z% %M% %I% %E% %U%
*
* Motif Resource Interpreter - Mri.c
*
* Mri.c implements a Motif Resource Interpreter which allows prototype Motif
* interfaces to be built from resource files.  The Widget Creation library is
* used.
*
******************************************************************************
*/

#include <Xm/Xm.h>		/* Motif and Xt Intrinsics	*/
#include <X11/Wc/WcCreate.h>	/* Widget Creation Library	*/
#include <X11/Xmp/Xmp.h>	/* Motif Public widgets etc.	*/


/******************************************************************************
**  Private Data
******************************************************************************/

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
    WCL_XRM_OPTIONS
};


/******************************************************************************
**  Private Functions
******************************************************************************/

/*ARGSUSED*/
static void DeleteWindowCB( w, clientData, callData )
    Widget	w;
    XtPointer	clientData;
    XtPointer	callData;
{
    /* This callback is invoked when the user selects `Close' from
    ** the mwm frame menu on the upper left of the window border.
    ** Do whatever is appropriate.
    */
    printf("Closed by window manager.\n");
}

/*ARGSUSED*/
static void RegisterApplication ( app )
    XtAppContext app;
{
    /* -- Useful shorthand for registering things with the Wcl library */
#define RCP( name, class  ) WcRegisterClassPtr   ( app, name, class );
#define RCO( name, constr ) WcRegisterConstructor( app, name, constr );
#define RAC( name, func   ) WcRegisterAction     ( app, name, func );
#define RCB( name, func   ) WcRegisterCallback   ( app, name, func, NULL );

    /* -- register widget classes and constructors */
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
}


/******************************************************************************
*   MAIN function
******************************************************************************/

#if defined(XtSpecificationRelease) && XtSpecificationRelease == 4
#define CARDINAL(argc) (Cardinal*)(&argc)
#else
#define CARDINAL(argc) (&argc)
#endif

main ( argc, argv )
    int     argc;
    String  argv[];
{   
    /*  -- Intialize Toolkit creating the application shell
    */
    Widget appShell = XtInitialize ( 
	WcAppName(  argc, argv ),    /* application shell name               */
	WcAppClass( argc, argv ),    /* class name is 1st resource file name */
	options, XtNumber(options),  /* resources which can be set from argv */
	CARDINAL(argc), argv 
    );
    XtAppContext app = XtWidgetToApplicationContext(appShell);

    /*  -- Register all application specific callbacks and widget classes
    */
    RegisterApplication ( app );

    /*  -- Register all Motif classes, constructors, and Xmp CBs & ACTs
    */
    XmpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
    */
    if ( WcWidgetCreation ( appShell ) )
	exit(1);

    /*  -- Realize the widget tree
    */
    XtRealizeWidget ( appShell );

    /*  -- Optional, but frequently desired:  
    ** Provide a callback which gets invoked when the user selects `Close' from
    ** the mwm frame menu on the top level shell.  A real application will need
    ** to provide its own callback instead of DeleteWindowCB, and probably 
    ** client data too.  MUST be done after shell widget is REALIZED!  Hence,
    ** this CANNOT be done using wcCallback (in a creation time callback).
    */
    XmpAddMwmCloseCallback( appShell, DeleteWindowCB, NULL );

    /*  -- and finally, enter the main application loop
    */
    XtMainLoop ( );
}

#ifdef PROFILE
/* HACK!
*/
extern void* dlopen( path, mode )
    char* path;
    int   mode;
{
    fprintf(stderr,"Profiled:dlopen( %s, %d )\n", path, mode );
    return (void*)1;
}

extern void* dlsym( handle, symbol )
    void* handle;
    char* symbol;
{
    fprintf(stderr,"Profiled:dlsym( %d, %s )\n", (int)handle, symbol );
    return (void*)2;
}

extern char* dlerror()
{
    fprintf(stderr,"Profiled:dlerror()\n");
    return "Profiled:dlerror() - why was this called!?!?\n";
}

extern int dlclose( handle )
    void* handle;
{
    fprintf(stderr,"Profiled:dlclose( %d )\n", (int)handle );
    return 0;
}

#endif /*PROFILE*/
