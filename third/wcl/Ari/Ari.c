#include <X11/Wc/COPY.h>

/*
* SCCS_data: %Z% %M% %I% %E% %U%
*
* Athena Resource Interpreter - Ari.c
*
* Ari.c implements an Athena Resource Interpreter which allows prototype
* Athena widget based user interfaces to be built from resource files.  The
* Widget Creation library is used.
*
******************************************************************************
*/

#include <X11/Intrinsic.h>
#include <X11/Wc/WcCreate.h>
#include <X11/Xp/Xp.h>

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
    int   argc;
    char* argv[];
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

    /*  -- Register all Athena and Public widget classes, CBs, ACTs
    */
    XpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
    */
    WcWidgetCreation ( appShell );

    /*  -- Realize the widget tree and enter the main application loop
    */
    XtRealizeWidget ( appShell );
    XtMainLoop ( );
}
