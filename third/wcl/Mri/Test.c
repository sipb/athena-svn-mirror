#include <X11/Wc/COPY.h>

/*
* SCCS_data: %Z% %M% %I% %E% %U%
*
* Widget Creation Library Test Program - Test.c
*
* Test.c is derived directly from Mri.c and is used to perform white-box
* testing of Wcl.
*
******************************************************************************
*/


#include <X11/IntrinsicP.h>

#ifdef sun
#include <X11/ObjectP.h>        /* why don't they just use X from mit!?! */
#include <X11/RectObjP.h>
#endif

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <X11/Wc/WcCreateP.h>
#include <X11/Xmp/Xmp.h>
#include <Xm/FileSB.h>

#include <stdio.h>
#include <errno.h>

static XrmOptionDescRec options[] = {
    WCL_XRM_OPTIONS
};

#define RCP( name, class  ) WcRegisterClassPtr   ( app, name, class );
#define RCO( name, constr ) WcRegisterConstructor( app, name, constr );
#define RAC( name, func   ) WcRegisterAction     ( app, name, func );
#define RCB( name, func   ) WcRegisterCallback   ( app, name, func, NULL );
#define RME( class, name, data, func ) \
			    WcRegisterMethod( app, class, name, data, func );

static char *event_names[] = {
"",
"",
"KeyPress",		/* 2 */
"KeyRelease",		/* 3 */
"ButtonPress",		/* 4 */
"ButtonRelease",	/* 5 */
"MotionNotify",		/* 6 */
"EnterNotify",		/* 7 */
"LeaveNotify",		/* 8 */
"FocusIn",		/* 9 */
"FocusOut",		/* 10 */
"KeymapNotify",		/* 11 */
"Expose",		/* 12 */
"GraphicsExpose",	/* 13 */
"NoExpose",		/* 14 */
"VisibilityNotify",	/* 15 */
"CreateNotify",		/* 16 */
"DestroyNotify",	/* 17 */
"UnmapNotify",		/* 18 */
"MapNotify",		/* 19 */
"MapRequest",		/* 20 */
"ReparentNotify",	/* 21 */
"ConfigureNotify",	/* 22 */
"ConfigureRequest",	/* 23 */
"GravityNotify",	/* 24 */
"ResizeRequest",	/* 25 */
"CirculateNotify",	/* 26 */
"CirculateRequest",	/* 27 */
"PropertyNotify",	/* 28 */
"SelectionClear",	/* 29 */
"SelectionRequest",	/* 30 */
"SelectionNotify",	/* 31 */
"ColormapNotify",	/* 32 */
"ClientMessage",	/* 33 */
"MappingNotify",	/* 34 */
"LASTEvent",		/* 35 */	/* must be bigger than any event # */
};

int   Test_TraceEvents = 0;
char* Test_TraceEventName;

static void TestTraceEventsCB(widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    Test_TraceEvents = 1;
    Test_TraceEventName = (char*)clientData;
}

static void TestNoTraceEventsCB(widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    Test_TraceEvents = 0;
    Test_TraceEventName = (char*)0;
}

static void TestDeleteWindowCB( widget, clientData, callData )
    Widget	widget;
    XtPointer	clientData;
    XtPointer	callData;
{
    printf("Closed from Mwm frame menu.\n");
}

typedef XrmResource **CallbackTable;

typedef struct internalCallbackRec {
    unsigned short count;
    char           is_padded;   /* contains NULL padding for external form */
    char           call_state;  /* combination of _XtCB{FreeAfter}Calling */
    /* XtCallbackList */
} InternalCallbackRec, *InternalCallbackList;

#define ToList(p) ((XtCallbackList) ((p)+1))

static void TestInvestigateCB( widget, clientData, callData )
    Widget	widget;
    XtPointer	clientData;
    XtPointer	callData;
{
    int				numCbs, numOffs;
    int				inx;
    CallbackTable		offsets;
    char*			cbName;
    char*			cbClass;
    char*			cbType;
    InternalCallbackList*	iclp;
    InternalCallbackList	icl;
    InternalCallbackRec		icr;
    XtCallbackList		xtcbl;

    offsets = (CallbackTable)
	widget->core.widget_class->core_class.callback_private;

    numOffs = (int)*(offsets);
    for (inx = (int) *(offsets++); --inx >= 0; offsets++)
    {
	cbName  = XrmQuarkToString( (*offsets)->xrm_name  );
	cbClass = XrmQuarkToString( (*offsets)->xrm_class );
	cbType  = XrmQuarkToString( (*offsets)->xrm_type  );
	iclp    = (InternalCallbackList*)( (char*)widget - (*offsets)->xrm_offset - 1 );
	icl     = *iclp;
	icr	= *icl; 
	numCbs  = icr.count;
	xtcbl   = (XtCallbackList)( icl+1 );		/* ToList(icl); */
    }
    return;
}

Widget resultWidget;
char*  resultFmt;

static void Thing_ResultWidgetCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    resultWidget = widget;
    resultFmt    = (char*)clientData;
    if ( WcStrStr( resultFmt, "%s" ) == NULL )
	XtError("You must have a %s in the Thing_Result format string.");
}

static void Thing_DisplayResultCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    char* buf;
    if (!resultFmt)
	XtError("You must provide a Thing_Result format string first.");
    buf = XtMalloc( WcStrLen( (char*)clientData ) + WcStrLen( resultFmt) + 1 );
    sprintf( buf, resultFmt, (char*)clientData );
    WcSetValue( resultWidget, buf );
    XtFree( buf );
}


/* Test behavior of Wcl when a widget constructor fails.
*/
static Widget TestConstructorFail( pw, name, args, nargs )
    Widget	pw;
    String	name;
    Arg*	args;
    Cardinal	nargs;
{
    return (Widget)NULL;
}

/* An object, and object methods
 */
typedef struct _TestObj {
    char* name;
    void (*FirstIMF)();		/* instance member function (ptr to func) */
    void (*SecondIMF)();	/* instance member function (ptr to func) */
} TestObjRec, *TestObj;

/* Callback methods
*/
static void TestObj_1stCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    TestObj this = (TestObj)(methodData->object);
    if ( this != (TestObj)0 )
	fprintf(stderr,"TestObj_1stCBM: name=%s, args=%s, closure=%s\n",
		this->name,
		(WcNonNull(methodData->args)?methodData->args:"(nil)"),
		methodData->closure );
    else
	fprintf(stderr,"TestObj_1stCBM: NULL object, args=%s\n",
		(WcNonNull(methodData->args)?methodData->args:"(nil)") );
}

static void TestObj_2ndCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    TestObj this = (TestObj)(methodData->object);
    if ( this != (TestObj)0 )
	fprintf(stderr,"TestObj_2ndCBM: name=%s, args=%s, closure=%s\n",
		   this->name,
		   (WcNonNull(methodData->args)?methodData->args:"(nil)"),
		   methodData->closure );
    else
	fprintf(stderr,"TestObj_2ndCBM: NULL object, args=%s\n",
		(WcNonNull(methodData->args)?methodData->args:"(nil)") );
}

static void TestObj_AttachToCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    TestObj this = (TestObj)(methodData->object);
    Widget target = WcFullNameToWidget( widget, methodData->args );
    if ( target != (Widget)0 )
	WcAttachThisToWidget( (XtPointer)this, "TestObj", target );
    else
	fprintf(stderr,"Could not find %s from %s\n",
			methodData->args, XtName(widget) );
}


/* Does not need object within methodData.
*/
static void TestObj_CreateCBM( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData, callData;
{
    WcMethodData methodData = (WcMethodData)clientData;
    XtAppContext app = XtWidgetToApplicationContext( widget );
    TestObj this = (TestObj)XtCalloc(sizeof(TestObjRec),1);
    this->name = XtNewString( methodData->args );
    WcAttachThisToWidget( (XtPointer)this, "TestObj", widget );
    WcRegisterMethod( app, "TestObj", "1st", TestObj_1stCBM, "first-data" );
    WcRegisterMethod( app, "TestObj", "2nd", TestObj_2ndCBM, "second-data" );
    WcRegisterMethod( app, "TestObj", "AttachTo", TestObj_AttachToCBM, "" );
}

static int QsortFileNameCompare( left, right )
    char** left;
    char** right;
{
    return strcmp( *left, *right );
}

#include <dirent.h>
/* fileType is one of { XmFILE_DIRECTORY, XmFILE_ANY_TYPE, XmFILE_REGULAR }
*/
static void GetListOfFiles( dirStr, xmStrings, count )
    XmString	dirStr;
    XmString*	xmStrings;	/* allocated by caller */
    int*	count;
{
    String		dirName;
    DIR*		dir;
    struct dirent*	ent;
    String		strings[1024];	/* names in directory */
    int			inx;

    if ( !XmStringGetLtoR( dirStr, XmSTRING_DEFAULT_CHARSET, &dirName ) )
    {
	*count = 0;
	return;
    }

    dir = opendir( dirName );
    if ( (DIR*)0 == dir )
    {
	*count = 0;
	return;
    }

    for ( inx = 0 ; (struct dirent*)0 != (ent = readdir(dir)) ; inx++ )
    {
	/* May want to confirm that files are always versandformula files.
	*/
	strings[inx] = XtNewString( ent->d_name );
    }
    strings[  inx] = (String)0;
    xmStrings[inx] = (XmString)0;
    *count = inx;

    qsort( strings, inx, sizeof(char*), QsortFileNameCompare );
    while ( inx )
    {
	char fullPath[BUFSIZ];
	--inx;
	strcpy( fullPath, dirName );
	strcat( fullPath, strings[inx] );
	xmStrings[inx] = XmStringCreateSimple( fullPath );
	XtFree( strings[inx] );
    }

    closedir( dir );
}

/*ARGSUSED*/
static void FSB_FileSearchProc( fsb, cbs )
    XmFileSelectionBoxWidget		fsb;
    XmFileSelectionBoxCallbackStruct*	cbs;
{
    XmString	xmStrings[1024];
    int		count;

    GetListOfFiles( cbs->dir, xmStrings, &count );

    XtVaSetValues( (Widget)fsb,
	XmNfileListItems,	xmStrings,
	XmNfileListItemCount,	count,
	XmNdirSpec,		cbs->value,
	XmNlistUpdated,		True,
	NULL );

    while ( count )
    {
	--count;
	XmStringFree( xmStrings[count] );
    }
}

/*ARGSUSED*/
static void FSBCB( w, clientData, callData )
    Widget	w;
    XtPointer	clientData;
    XtPointer	callData;
{
    char* fileName = (char*)clientData;
    Widget fsb = WcFullNameToWidget( w, "*fsb" );
    XmString dirStr, patStr, specStr;

    patStr  = XmStringCreateSimple( "*" );
    specStr = XmStringCreateSimple( fileName );

    if ( WcNull( fileName ) )
    {
	/* NULL means CWD?
	*/
	dirStr = XmStringCreateSimple( "" );
    }
    else
    {
        int      end     = WcStrLen( fileName ) - 1;
        int      slash   = end;

        while ( slash >= 0 && fileName[slash] != '/' )
            --slash;

        if ( slash >= 0 && fileName[slash] == '/' )
        {
            /* to left is dir, to right is filename
            */
            char*       dirCp = XtCalloc( slash + 2, sizeof(char) );

            /* directory MUST end in slash
            */
            if ( slash > 0 )
                strncpy( dirCp, fileName, slash );

            strcat(  dirCp, "/" );

            dirStr = XmStringCreateSimple( dirCp );

            XtFree( (char*)dirCp );
        }
        else
        {
            /* not full path.  Null dir means CWD?
            */
	    dirStr = XmStringCreateSimple( "" );
        }
    }
	fprintf(stderr,"before XtVaSetValues\n");
        XtVaSetValues( fsb,
		XmNfileSearchProc,	FSB_FileSearchProc,
		XmNdirectory,   	dirStr,
                XmNpattern,     	patStr,
                NULL );
	fprintf(stderr,"after XtVaSetValues\n");

	XmStringFree( dirStr );
        XmStringFree( patStr );
	

    fprintf(stderr,"before XtManageChild or XtPopup\n");
    XtManageChild(fsb);
    fprintf(stderr,"Done!\n");
}

static void FSB_OkCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    if ( !XtIsSubclass(widget,xmFileSelectionBoxWidgetClass))
    {
	fprintf(stderr,"NOT FSB!!\n");
    }
    else
    {
	char* value;
	XmFileSelectionBoxCallbackStruct* cbd = 
		(XmFileSelectionBoxCallbackStruct*)callData;
	XmStringGetLtoR(cbd->value,XmSTRING_DEFAULT_CHARSET,&value);
	fprintf(stderr,"selected: %s\n",value);
    }
}

/* This is a normal callback which is late-bound because it is not
 * originally registered when some widget was created.
 */
static void TestInitiallyNotRegisteredCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    fprintf(stderr,"TestInitiallyNotRegistered(%s)\n",XtName(widget));
}

static void TestLateRegisterCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    XtAppContext app = XtWidgetToApplicationContext( widget );
    RCB( "TestInitiallyNotRegistered", TestInitiallyNotRegisteredCB );
}

static XrmQuark testFirstQuark, testSecondQuark;

Boolean TestFirstHook( clientData, lb )
    XtPointer  clientData;
    WcLateBind lb;
{
    if ( lb->nameQ == testFirstQuark )
    {
	fprintf(stderr,"TestFirstHook resolved\n");
	return True;
    }
    return False;
}

Boolean TestSecondHook( clientData, lb )
    XtPointer  clientData;
    WcLateBind lb;
{
    if ( lb->nameQ == testSecondQuark )
    {
        fprintf(stderr,"TestSecondHook resolved\n");
        return True;
    }
    return False;
}

void TestClosureCB( widget, clientData, callData )
    Widget      widget;
    XtPointer   clientData;
    XtPointer   callData;
{
    fprintf(stderr,"TestClosureCB(%s)\n",(char*)clientData);
}


static void TestRegisterApplication ( app )
    XtAppContext app;
{
    testFirstQuark  = XrmStringToQuark( "TestFirstHook" );
    testSecondQuark = XrmStringToQuark( "TestSecondHook" );

    WcAddLateBinderHook( TestFirstHook,  (XtPointer)1 );
    WcAddLateBinderHook( TestSecondHook, (XtPointer)2 );

    RCO( "TestConstructorFail", TestConstructorFail	);

    RCB( "TestDeleteWindow",	TestDeleteWindowCB	);
    RCB( "TestTraceEvents",	TestTraceEventsCB	);
    RCB( "TestNoTraceEvents",	TestNoTraceEventsCB	);
    RCB( "TestInvestigate",	TestInvestigateCB	);
    RCB( "Thing_ResultWidget",	Thing_ResultWidgetCB	);
    RCB( "Thing_DisplayResult",	Thing_DisplayResultCB	);
    RCB( "TestLateRegister",	TestLateRegisterCB	);
    RCB( "FSB",			FSBCB			);
    RCB( "FSB_Ok",		FSB_OkCB		);

    WcRegisterCallback( app, "TestClosure", TestClosureCB, "registered-data" );

    WcRegisterMethod( app, "TestObj", "TestObj", TestObj_CreateCBM,  "" );
    WcRegisterMethod( app, "TestObj", "Create",  TestObj_CreateCBM,  "" );
}

static char* SysErrorMsg(n)
    int n;
{
    extern char *sys_errlist[];
    extern int sys_nerr;
    char *s = ((n >= 0 && n < sys_nerr) ? sys_errlist[n] : "unknown error");

    return (s ? s : "no such error");
}

/* Identical to the Xlib IO error handler, except this dumps core.
*/
static (*DefaultXIOErrorHandler) _((Display*));

int IOErrorHandler( dpy )
    Display* dpy;
{
    (void) fprintf (stderr,
"XIO:  fatal IO error %d (%s) on X server \"%s\"\n",
	errno, SysErrorMsg (errno), DisplayString (dpy));
    (void) fprintf (stderr,
"      after %lu requests (%lu known processed) with %d events remaining.\n",
	NextRequest(dpy) - 1, LastKnownRequestProcessed(dpy), QLength(dpy));

    if (errno == EPIPE) {
	(void) fprintf (stderr,
"      The connection was probably broken by a server shutdown or KillClient.\n"
	);
    }
    abort();
}

/* Try out converters, make sure they work!!
********************************************
*/
typedef struct _TestConverter {
    WidgetList	widgetList;
} TestConverterStruct, *TestConverter;

#ifdef XtOffsetOf
#define FLD(n)  XtOffsetOf(TestConverterStruct,n)
#else
#define FLD(n)  XtOffset(TestConverterStruct,n)
#endif

XtResource widgetListRes[] = {
 { "testWidgetList", "TestWidgetList", XtRWidgetList, sizeof(WidgetList),
    FLD(widgetList), XtRString, (XtPointer)"this"
 },
};

static void TestConverters( widget )
    Widget widget;
{
    TestConverterStruct	testStruct;
    TestConverter	test = &testStruct;

    XtGetApplicationResources( widget, (XtPointer)test,
	widgetListRes, XtNumber(widgetListRes), NULL, 0 );
}

/* Trace attepts at loading resource files
******************************************
   Requires hacks to Xt.
*/
#if XTRACEFILEHACKS
static XtFilePredicate OriginalPredicate;
static Boolean TraceFile( file )
    char* file;
{
    Boolean retval = OriginalPredicate(file);
    fprintf(stderr,"%s: %s\n",(retval?"OK":"No"),file);
    return retval;
}
#endif

/*  -- Main
***********
*/

#if defined(XtSpecificationRelease) && XtSpecificationRelease == 4
#define CARDINAL(argc) (Cardinal*)(&argc)
#else
#define CARDINAL(argc) (&argc)
#endif

main ( argc, argv )
    int    argc;
    String argv[];
{   
    XtAppContext app;
    Widget       appShell;

    argv[0] = "Mri";

#if XTRACEFILEHACKS
    OriginalPredicate = XtSetDefaultFilePredicate( TraceFile );
#endif

    DefaultXIOErrorHandler = XSetIOErrorHandler(IOErrorHandler);

    appShell = XtInitialize ( 
	WcAppName(  argc, argv ),    /* application shell name               */
	WcAppClass( argc, argv ),    /* class name is 1st resource file name */
	options, XtNumber(options),  /* resources which can be set from argv */
	CARDINAL(argc), argv 
    );
    app = XtWidgetToApplicationContext(appShell);

    TestRegisterApplication ( app );

    XmpRegisterAll ( app );

    WcWidgetCreation ( appShell );

    XtRealizeWidget ( appShell );

    XmpAddMwmCloseCallback( appShell, TestDeleteWindowCB, NULL );

    TestConverters( appShell );

    /* Replacement for XtAppMainLoop()
    */
    {
      XEvent event;
      FILE*  log = fopen("log","w");

      for(;;) {
        XtAppNextEvent(app, &event);
        if (Test_TraceEvents)
        {
          Widget widget = XtWindowToWidget(event.xany.display, 
                                           event.xany.window);
	  if ( WcStrEq( Test_TraceEventName, event_names[event.type] ) )
	  {
	    /* Set breakpoint here if interested in some specific event type
	    */
	    fprintf(log, "%16s %9ld %s %s **** special interest\n",
                  event_names[event.type], event.xany.serial,
                  ( event.xany.send_event ? "c" : " " ),
                  ( XtIsWidget(widget) ? widget->core.name : XtName(widget)) );
	  }
	  else
	  {
            fprintf(log, "%16s %9ld %s %s\n", 
                  event_names[event.type], event.xany.serial,
                  ( event.xany.send_event ? "c" : " " ),
                  ( XtIsWidget(widget) ? widget->core.name : XtName(widget)) );
	  }
	  fflush(log);
        }
        XtDispatchEvent(&event);
      }
    }
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
