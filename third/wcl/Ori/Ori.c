#include <X11/Wc/COPY.h>

/*
* SCCS_data: %Z% %M% %I% %E% %U%
*
* Open Look Resource Interpreter - Ori.c
*
* Ori.c implements an Open Look Widget Set Resource Interpreter which
* allows prototype Open Look Widget based user interfaces to be built from
* resource files.  The Widget Creation library is used.
*
******************************************************************************
*/

#include <X11/Intrinsic.h>
#include <Xol/OpenLook.h>
#include <X11/Wc/WcCreate.h>
#include <X11/Xop/Xop.h>

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

main ( argc, argv )
    int   argc;
    char* argv[];
{   
    /*  -- Intialize Toolkit creating the application shell
    */
    Widget appShell = OlInitialize (
        WcAppName(  argc, argv ),    /* application shell name               */
        WcAppClass( argc, argv ),    /* class name is 1st resource file name */
        options, XtNumber(options),  /* resources which can be set from argv */
	&argc, argv
    );
    XtAppContext app = XtWidgetToApplicationContext(appShell);

    /*  -- Register all application specific callbacks and widget classes
    */
    RegisterApplication ( app );

    /*  -- Register all Open Look widget classes, CBs, ACTs
    */
    XopRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
    */
    if ( WcWidgetCreation ( appShell ) )
	exit(1);

    /*  -- Realize the widget tree and enter the main application loop
    */
    XtRealizeWidget ( appShell );
    XtMainLoop ( );
}
