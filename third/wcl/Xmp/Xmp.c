#include <X11/Xmp/COPY.h>
/*
* SCCS_data: %Z% %M%	%I% %E% %U%
*
* Xmp - Motif Public Utilities for Wcl - Xmp.c
*
* This file contains Motif specific converters, actions, and convenience
* functions for Motif and Xmp widgets.
*
*******************************************************************************
*/

#include <X11/IntrinsicP.h>

#ifdef sun
#include <X11/ObjectP.h>        /* why don't they just use X from mit!?! */
#include <X11/RectObjP.h>
#endif

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/RowColumnP.h>	/* RC_Type */
#include <Xm/MenuShell.h>	/* xmMenuShellWidgetClass */
#include <Xm/Text.h>		/* xmTextWidgetClass */
#include <X11/Shell.h>		/* wmShellWidgetClass */
#include <X11/Wc/WcCreateP.h>

#include <X11/Xmp/Table.h>
#include <X11/Xmp/XmpP.h>

/******************************************************************************
* Private_macro_definitions.
******************************************************************************/

/* The `done' macro, used by the converters, is defined in WcCreateP.h */

/******************************************************************************
**  Motif specific resource converters
******************************************************************************/

/*
    -- Convert String To XmString
*******************************************************************************
    This conversion converts strings into XmStrings.  This one overrides
    the one provided by Motif which calls XmCreateString.  Instead, this
    one uses XmCreateStringLtoR which allows multi-line strings to be
    created by embedding "\n" in a resource specification.  In order to
    allow this to be used, XmpRegAll MUST install this converter AFTER
    Motif has installed its broken converter.
*/
/*ARGSUSED*/
void XmpCvtStringToXmString (args, num_args, fromVal, toVal)
    XrmValue *args;
    Cardinal *num_args;
    XrmValue *fromVal;
    XrmValue *toVal;
{
    XmString xmString;
    xmString = XmStringCreateLtoR( fromVal->addr, XmSTRING_DEFAULT_CHARSET );
    done( XmString, xmString );
}

/* -- Register all converters provided by Motif
*******************************************************************************
*/

void XmpAddMotifConverters ( app )
    XtAppContext app;
{
    ONCE_PER_XtAppContext( app );

    XmRegisterConverters();
#ifdef XmRTearOffModel
    XmRepTypeInstallTearOffModelConverter();
#endif
#ifndef VMS
    _XmRegisterPixmapConverters();
#endif
    XtAppAddConverter(	app, XmRString, XmRUnitType,
			XmCvtStringToUnitType,
			(XtConvertArgList)0, (Cardinal)0 );
}

/* -- Register all converters provided by Xmp
*******************************************************************************
*/

void XmpAddConverters ( app )
    XtAppContext app;
{
    ONCE_PER_XtAppContext( app );

    /* Some, not all, old versions of Motif incorrectly say some resources
    ** which should be of type "Widget" are of type "Window"
    */
#ifdef XtSpecificationRelease
    /* Must use new style converter if available to avoid cache,
     * because the cache is not cleared when named widgets get destroyed!
     */
    XtAppSetTypeConverter( app, XtRString, "Window",
			   WcCvtStringToWidget,
			   wcWidgetCvtArgs, wcWidgetCvtArgsCount,
			   XtCacheNone, (XtDestructor)0 );

    XtAppSetTypeConverter( app, XtRString, XmRMenuWidget,
			   WcCvtStringToWidget,
			   wcWidgetCvtArgs, wcWidgetCvtArgsCount,
			   XtCacheNone, (XtDestructor)0 );
#else
    /* R3 does not cache conversions, so there is nothing to worry about.
    */
    XtAppAddConverter(	app, XtRString, "Window",
			WcCvtStringToWidget,
			wcWidgetCvtArgs, wcWidgetCvtArgsCount );

    XtAppAddConverter(	app, XtRString, XmRMenuWidget,
			WcCvtStringToWidget,
			wcWidgetCvtArgs, wcWidgetCvtArgsCount );

#endif

    XtAppAddConverter(	app, XtRString, XmRXmString,
			XmpCvtStringToXmString,
			(XtConvertArgList)0, (Cardinal)0 );
}


/* -- Action to fix broken translations in XmText widgets.
*******************************************************************************
*/
/*ARGSUSED*/
void XmpFixTranslationsACT( w, unusedEvent, params, num_params )
    Widget      w;
    XEvent     *unusedEvent;
    String     *params;
    Cardinal   *num_params;
{
    static int            firstTime = 1;
    static XtTranslations transTable;
    static char           fixedTranslations[] = "<Key>Return: newline()";

    int     i;

    if (firstTime)
    {
	firstTime = 0;
	transTable = XtParseTranslationTable(fixedTranslations);
    }

    if ( *num_params == 0 )
    {
	WcWARN( w, "XmpFixTranslations", "usage",
		"Xmp Usage: XmpFixTranslations( widget [widgetName] ...)");
	return;
    }

    for ( i = 0 ; i < *num_params ; i++ )
    {
	Widget toFix = WcFullNameToWidget( w, params[i] );

        if (!toFix)
        {
	    WcWARN1( w, "XmpFixTranslations", "notFound",
		   "Xmp Warning: XmpFixTranslations(%s) - Widget not found",
		    params[i] );
	    return;
        }

	if (XtIsSubclass( toFix, xmTextWidgetClass ))
	    XtOverrideTranslations( toFix, transTable );
	else
	{
	    WcWARN1( w, "XmpFixTranslations", "notText",
		   "Xmp Warning: XmpFixTranslations(%s) - Not an XmText widget",
		     params[i] );
	    return;
        }
    }
}

/*ARGSUSED*/
void XmpFixTranslationsCB( w, widgetNames, ignore )
    Widget	w;
    XtPointer	widgetNames;
    XtPointer	ignore;
{
    WcInvokeAction( XmpFixTranslationsACT, w, widgetNames );
}


/* -- Action to popup a Motif PopupMenu
*******************************************************************************
   Gotta do debugging with fprintf() because there is a server grab
   in effect, in all probability.
*/

void XmpPopupACT( w, event, params, num_params )
    Widget      w;
    XEvent     *event;
    String     *params;
    Cardinal   *num_params;
{
    Widget        menuPane, menu;
    unsigned char rowColumnType;

    if (*num_params < 1)
    {
	WcWARN( w, "XmpPopup", "noParams",
"Xmp Warning: XmpPopup() - No menu pane name provided." );
	return;
    }

    if ( *num_params > 1 )
    {
        WcWARN2( w, "XmpPopup", "tooManyParams",
"Xmp Warning: XmpPopup( %s %s ... ) - Only one menu pane name allowed." ,
                params[0], params[1] );
        return;
    }

    menuPane = WcFullNameToWidget( w, params[0] );

    if ( menuPane == (Widget)0 )
    {
        String fullName = WcWidgetToFullName( w );
        WcWARN3( w, "XmpPopup", "notFound",
"Xmp Warning: XmpPopup( %s ) - Couldn't find menu pane widget `%s' from `%s'",
                params[0], params[0], fullName );
        XtFree( fullName );
        return;
    }

    if ( !XtIsSubclass( menuPane, xmRowColumnWidgetClass ) )
    {
        String fullName = WcWidgetToFullName( menuPane );
        WcWARN2( w, "XmpPopup", "notXmRowColumn",
"Xmp Warning: XmpPopup( %s ) - %s is not menu pane (not XmRowColumn)",
                params[0], fullName );
        XtFree( fullName );
        return;
    }

    XtVaGetValues( menuPane, XmNrowColumnType, &rowColumnType, NULL );

    if ( rowColumnType != XmMENU_POPUP )
    {
        String fullName = WcWidgetToFullName( menuPane );
        WcWARN2( w, "XmpPopup", "notPopupMenuRowColumn",
"Xmp Warning: XmpPopup( %s ) - %s is not menu pane (not XmMENU_POPUP)",
                params[0], fullName );
        XtFree( fullName );
        return;
    }

    menu = XtParent( menuPane );

    if ( !XtIsSubclass( menu, xmMenuShellWidgetClass ) )
    {
        String fullName = WcWidgetToFullName( menuPane );
        WcWARN2( w, "XmpPopup", "notChildOfXmMenu",
"Xmp Warning: XmpPopup( %s ) - %s is not menu pane (child of XmMenuShell)",
                params[0], fullName );
        XtFree( fullName );
        return;
    }

    if ( event->type != ButtonPress )
    {
        WcWARN2( w, "XmpPopup", "notButtonPressEvent",
"Xmp Warning: XmpPopup( %s ) - event type `%s' is not ButtonPress event.",
                params[0], WcXEventName( event ) );
        return;
    }

    /* OK! we have 1 named widget which resolves to a XmRowColumn of
     * type XmMENU_POPUP which is a child of a XmMenuShell, and a
     * ButtonPress event invoked this action.  Therefore, lets go ahead.
     */
    XmMenuPosition( menuPane, (XButtonPressedEvent*)event );
    XtManageChild( menuPane );
}


/*
    -- Provide `Close' callback
*******************************************************************************
    Provide a callback which gets invoked when the user select `Close' from
    the Mwm frame menu on the top level shell.  MUST be done after shell widget
    is realized!
*/

void XmpAddMwmCloseCallback( shell, callback, clientData )
    Widget         shell;
    XtCallbackProc callback;
    XtPointer      clientData;
{
    Atom wmProtocol, wmDeleteWindow, wmSaveYourself;
    wmProtocol    =XmInternAtom( XtDisplay(shell), "WM_PROTOCOLS",     False );

    wmDeleteWindow=XmInternAtom( XtDisplay(shell), "WM_DELETE_WINDOW", False );
    wmSaveYourself=XmInternAtom( XtDisplay(shell), "WM_SAVE_YOURSELF", False );
    XmAddProtocolCallback( shell, wmProtocol, wmDeleteWindow,
			   callback, clientData );
    XmAddProtocolCallback( shell, wmProtocol, wmSaveYourself,
			   callback, clientData );
}

/*
    -- Action and callback to invoke XmpAddMwmCloseCallback
*******************************************************************************
*/
/*ARGSUSED*/
void XmpAddMwmCloseCallbackCB( w, widget_callbackList, notUsed )
    Widget	w;
    XtPointer	widget_callbackList;
    XtPointer	notUsed;
{
    char		widgetName[MAX_XRMSTRING];
    String		callbackListString;
    Widget		target;
    XtCallbackRec*	callbackList;
    XtCallbackRec*	cb;

    callbackListString = WcCleanName( (char*)widget_callbackList, widgetName );
    callbackListString = WcSkipWhitespace_Comma( callbackListString );

    target = WcFullNameToWidget( w, widgetName );

    if ( (Widget)0 == (target = WcFullNameToWidget( w, widgetName )) )
    {
	WcWARN1( w, "XmpAddMwmCloseCallback", "targetNotFound",
"Xmp Warning: XmpAddMwmCloseCallback( %s ... ) - Target widget not found",
		widgetName );
	return;
    }

    callbackList = WcStringToCallbackList( w, callbackListString );

    for ( cb = callbackList  ;
	  cb != (XtCallbackRec*)0 && cb->callback != (XtCallbackProc)0  ;
	  cb++ )
	XmpAddMwmCloseCallback( target, cb->callback, cb->closure );

    WcFreeCallbackList( callbackList );
}

/*ARGSUSED*/
void XmpAddMwmCloseCallbackACT( w, unusedEvent, params, num_params )
    Widget      w;
    XEvent     *unusedEvent;
    String     *params;
    Cardinal   *num_params;
{
    WcInvokeCallback( XmpAddMwmCloseCallbackCB, w, params, num_params );
}

/*  -- Define a Widget as a Tab Group
*******************************************************************************
    If no widget is named, then use the widget argument.
    If the widget is a  manager widget, then this means all the children
    are in a tab group together.  If a widget is a primitive, then that
    widget is in its own tab group (this is the default).
*/
/*ARGSUSED*/
void XmpAddTabGroupACT( w, unusedEvent, params, num_params )
    Widget      w;
    XEvent     *unusedEvent;
    String     *params;
    Cardinal   *num_params;
{
    if (*num_params == 0)
    {
	XmAddTabGroup(w);
    }
    else if (*num_params == 1)
    {
	Widget target = WcFullNameToWidget( w, params[0] );

	if ((Widget)0 != target)
	{
	    XmAddTabGroup(w);
	}
	else
	{
	    WcWARN1( w, "XmpAddTabGroup", "notFound",
		"Xmp Warning: XmpAddTabGroup(%s) - Widget not found",
		params[0] );
	}
	
    }
    else
    {
	WcWARN( w, "XmpAddTabGroup", "usage",
"Xmp Warning: XmpAddTabGroup() requires 0 or 1 widget name argument.");
    }
}

/*ARGSUSED*/
void XmpAddTabGroupCB( w, widgetName, notUsed )
    Widget	w;
    XtPointer	widgetName;
    XtPointer	notUsed;
{
    WcInvokeAction( XmpAddTabGroupACT,  w, widgetName );
}

/*  -- Specify a text widget to take Wcl messages
*******************************************************************************
*/

static void   XmpxPrint                  _(( String, ... ));
static void   XmpxMessageWidgetDestroyCB _(( Widget, XtPointer, XtPointer ));
static int    XmpxLinesIn                _(( String ));
static int    XmpxEntireMessageLen       _(( String, va_list ));
static String XmpxMakeEntireMessage      _(( String, va_list ));

static Widget xmpMessageWidget;		/* an XmText widget */

static WcPrintProc XmpxDefaultWcPrintProc;

typedef struct _MessageOptions {
    int		maxLines;
    int		columns;
    String	linesLostMsg;
    int		linesInLinesLostMsg;
    Boolean	popup, raise, brokenXmTextShowPosition;
} MessageOptionsRec, *MessageOptions;

static MessageOptionsRec xmpMessageOptionsRec;
static MessageOptions    xmpMessageOptions = &xmpMessageOptionsRec;

#ifdef XtOffsetOf
#define FLD(n)  XtOffsetOf(MessageOptionsRec,n)
#else
#define FLD(n)  XtOffset(MessageOptions,n)
#endif

static XtResource xmxMessageRes[] = {
  {  XmpNmessageMaxLines, XmpCMessageMaxLines, XtRInt, sizeof(int),
     FLD(maxLines), XtRString, (XtPointer)"100"
  },
  {  XmpNmessageColumns, XmpCMessageColumns, XtRInt, sizeof(int),
     FLD(columns), XtRString, (XtPointer)"80"
  },
  {  XmpNmessageLinesLostMsg, XmpCMessageLinesLostMsg,
     XtRString, sizeof(String),
     FLD(linesLostMsg), XtRString, (XtPointer)"... some lines lost...\n"
  },
  {  XmpNmessagePopup, XmpCMessagePopup, XtRBoolean, sizeof(Boolean),
     FLD(popup), XtRString, (XtPointer)"True"
  },
  {  XmpNmessageRaise, XmpCMessageRaise, XtRBoolean, sizeof(Boolean),
     FLD(raise), XtRString, (XtPointer)"False"
  },
  {  XmpNmessageBrokenXmTextShowPosition,
     XmpCMessageBrokenXmTextShowPosition, XtRBoolean, sizeof(Boolean),
     FLD(brokenXmTextShowPosition), XtRString, (XtPointer)"False"
  },
};
#undef FLD

/* Set Messages to go to an XmText widget
*******************************************************************************
   Only get the default print proc once, but allow the message widget
   to be changed at any time.
*/
/*ARGSUSED*/
void XmpMessageWidgetCB( widget, client, call )
    Widget	widget;
    XtPointer	client, call;
{
    String	name = (String)client;

    if ( WcNonNull( name ) )
    {
	Widget messageWidget = WcFullNameToWidget( widget, name );

	if ( messageWidget == (Widget)0 )
	{
	    String fullName = WcWidgetToFullName( widget );
	    WcWARN2( widget, "XmpMessageWidget", "notFound",
"Xmp Warning: XmpMessageWidget could not find %s from %s\n",
		     name, fullName );
	    XtFree(fullName);
	    return;
	}

	XmpMessageWidget( messageWidget );
    }
    else
	XmpMessageWidget( widget );
}

/*ARGSUSED*/
void XmpMessageWidgetACT( widget, unused, params, num_params )
    Widget	widget;
    XEvent*	unused;
    char**	params;
    Cardinal*	num_params;
{
    if ( *num_params == 0 )
	XmpMessageWidgetCB( widget, NULL, NULL );
    else if ( *num_params == 1 )
	XmpMessageWidgetCB( widget, params[0], NULL );
    else
	WcWARN( widget, "XmpMessageWidget", "usage",
		"Xmp Usage: XmpMessageWidget( [messageWidgetName] )" );
}

void XmpMessageWidget( widget )
    Widget widget;
{
    if ( widget == (Widget)0 && XmpxDefaultWcPrintProc != (WcPrintProc)0 )
    {
	/* Reset back to initial state - use default WcPrint method
	*/
	WcPrint = XmpxDefaultWcPrintProc;
	XmpxDefaultWcPrintProc = (WcPrintProc)0;
	xmpMessageWidget = (Widget)0;
	return;
    }

    if ( !XmIsText( widget ) )
    {
	/* Message widget MUST be an XmText widget - leave in current state.
	*/
	String fullName = WcWidgetToFullName( widget );
	WcWARN1( widget, "XmpMessageWidget", "notXmText",
		 "Xmp Warning: XmpMessageWidget(%s) - not an XmTextWidget",
		 fullName );
	XtFree( fullName );
	return;
    }

    /* New XmText widget
    */
    if ( XmpxDefaultWcPrintProc == (WcPrintProc)0 )
    {
	XmpxDefaultWcPrintProc = WcPrint;
	WcPrint = XmpxPrint;
    }

    xmpMessageWidget = widget;

    XtGetApplicationResources(	xmpMessageWidget, (XtPointer)xmpMessageOptions,
				xmxMessageRes, XtNumber( xmxMessageRes ),
				NULL, 0 );

    xmpMessageOptions->linesInLinesLostMsg =
				XmpxLinesIn( xmpMessageOptions->linesLostMsg );

    XtAddCallback( widget, XtNdestroyCallback,
		   XmpxMessageWidgetDestroyCB, (XtPointer)0 );
}

/* Send Messages to the Message Widget
*******************************************************************************
   Invoked via pointer to procedure WcPrint.  Used to print Wcl messages
   and any other messages printed via WcWARN or WcPrint.

   Note: XmpMessageWidget must ensure that if WcPrint is XmpPrint,
   then xmpMessageWidget is an XmTextWidget.
*/
#if NeedFunctionPrototypes
static void XmpxPrint( String msg, ... )
#else
static void XmpxPrint( msg, va_alist )
    String	msg;
    va_dcl
#endif
{
    String	entireMsg;
    va_list	argPtr;

    Va_start( argPtr, msg );		/* point at 1st unnamed arg	*/

    entireMsg = XmpxMakeEntireMessage( msg, argPtr );

    if ( WcNonNull( entireMsg ) )
    {
	/* Put the message into the XmTextWidget
	*/
	XmTextPosition	chPos;  /* a long int */
	String		existingText = XmTextGetString( xmpMessageWidget );
	int		linesInExistingText = XmpxLinesIn( existingText );
	int		linesInEntireMsg    = XmpxLinesIn( entireMsg );
	int		lines = linesInExistingText + linesInEntireMsg;

	if ( lines >= xmpMessageOptions->maxLines )
	{
	    /* Too many lines:  First line in XmText probably has the
	     * "... some lines lost ..." message already.  Therefore,
	     * always throw that message away and enough to show entire
	     * new message.
	     */
	    int linesToReplace = xmpMessageOptions->linesInLinesLostMsg +
				 (lines - xmpMessageOptions->maxLines);

	    for ( lines = 0, chPos = 0  ;  existingText[chPos]  ;  chPos++ )
	    {
		if ( existingText[chPos] == '\n' )
		    ++lines;
		if ( lines > linesToReplace )
		    break;
	    }
	    /* lines >= 2 because we must always replace at least one line
	     * AFTER the linesLostMsg which is (except the 1st time) on the
	     * first line.  Replace the 1st lines with the linesLostMsg.
	     */
	    XmTextReplace( xmpMessageWidget, (XmTextPosition)0, chPos, 
			   xmpMessageOptions->linesLostMsg );
	}
	/* Find the new last position, append entireMsg to end.
	*/
	chPos = XmTextGetLastPosition( xmpMessageWidget );

	if ( chPos != 0 )
	    XmTextInsert( xmpMessageWidget, chPos++, "\n" );
	XmTextInsert( xmpMessageWidget, chPos, entireMsg );

	if ( xmpMessageOptions->brokenXmTextShowPosition )
	{
	    /* Cause text to scroll to show beginning of new message.
	    */
	    XmTextShowPosition(  xmpMessageWidget, chPos );
	}

	XtFree( existingText );
	XtFree( entireMsg );

	/* Maybe cause shell to appear, and maybe raise that shell.
	*/
	if ( xmpMessageOptions->popup || xmpMessageOptions->raise )
	{
	    Widget childOfShell = xmpMessageWidget;
	    Widget shell        = XtParent( childOfShell );

	    /* Go up widget tree to find nearest shell ancestor, and its child
	    */
	    while ( !XtIsShell( shell ) )
	    {
		childOfShell = shell;
		shell = XtParent( childOfShell );
	    }

	    if ( xmpMessageOptions->popup )
	    {
		if ( XmIsDialogShell( shell ) )
		{
		    XtManageChild( childOfShell );
		}
		else
		{
		    if ( !XtIsManaged( childOfShell ) )
		    {
			XtManageChild( childOfShell );
		    }
		    XtPopup( shell, XtGrabNone );
		}
	    }

	    if ( xmpMessageOptions->raise )
	    {
		if ( !XtIsManaged( childOfShell ) )
		{
		    XtManageChild( childOfShell );
		}
		XtRealizeWidget( shell );
		XRaiseWindow( XtDisplay(shell), XtWindow(shell) );
		XFlush( XtDisplay(shell) );
	    }
	}
    }

    va_end( argPtr );			/* clean up */
}

/*ARGSUSED*/
static void XmpxMessageWidgetDestroyCB( widget, client, call )
    Widget	widget;
    XtPointer	client, call;
{
    /* Reset back to initial state - use default WcPrint method
    */
    WcPrint = XmpxDefaultWcPrintProc;
    XmpxDefaultWcPrintProc = (WcPrintProc)0;
    xmpMessageWidget = (Widget)0;
}

static int XmpxLinesIn( cp )
    String cp;
{
    int lines;

    for ( lines = 1  ;  *cp  ;  ++cp )
    {
	if ( *cp == '\n' )
	    ++lines;
    }
    return lines;
}

static int XmpxEntireMessageLen( msg, argPtr )
    String	msg;
    va_list	argPtr;
{
    int len = WcStrLen( msg );

    if ( msg == NULL )
	return 0;

    msg = va_arg( argPtr, String );	/* get unnamed string arg	*/
    while ( msg != NULL )
    {
	len += WcStrLen( msg );
	msg = va_arg( argPtr, String );	/* get next unnamed string arg	*/
    }

    return len;
}

static String XmpxMakeEntireMessage( msg, argPtr )
    String	msg;
    va_list	 argPtr;
{
    int    entireLen = XmpxEntireMessageLen( msg, argPtr );
    String entireMsg = XtMalloc( entireLen + 1 );

    if ( entireMsg == NULL )
	return NULL;		/* malloc failed */

    strcpy( entireMsg, msg );

    msg = va_arg( argPtr, String );	/* get unnamed string arg	*/
    while ( msg != NULL )
    {
	strcat( entireMsg, msg );
	msg = va_arg( argPtr, String );	/* get next unnamed string arg	*/
    }

    return entireMsg;
}

/*  -- XmpTable Callbacks and Actions
*******************************************************************************
    These callbacks and actions make the XmpTable convenience functions
    accessible via the Xrm database.
*/

/*ARGSUSED*/
void XmpTableChildConfigCB( w, client, ignored )
    Widget	w;
    XtPointer	client;
    XtPointer	ignored;
{
    WcInvokeAction( XmpTableChildConfigACT, w, client );
}

/*ARGSUSED*/
void XmpTableChildConfigACT( w, unusedEvent, params, num_params )
    Widget      w;
    XEvent     *unusedEvent;
    String     *params;
    Cardinal   *num_params;
{
    Widget	widget;
    Position	col;
    Position	row;
    Dimension	hSpan = 1;		/* optional arg, default */
    Dimension	vSpan = 1;		/* optional arg, default */
    XmpTableOpts opt  = TBL_DEF_OPT;	/* optional arg, default */

    if (*num_params < 3 || 6 < *num_params )
    {
	WcWARN( w, "XmpTableChildConfig", "wrongNumArgs", 
"Xmp Warning: XmpTableChildConfig( ... ) - Wrong number of arguments.\n\
Xmp Usage: XmpTableChildConfig( w col row [h_span [v_span [opts]]] )" );
	return;
    }

    widget = WcFullNameToWidget( w, params[0] );
    col = atoi( params[1] );
    row = atoi( params[2] );

    if ((Widget)0 == widget)
    {
	WcWARN1( w, "XmpTableChildConfig", "widgetNotFound", 
"Xmp Warning: XmpTableChildConfig( %s ... ) - Could not find widget.",
			params[0] );
	return;
    }
    else if ( ! XtIsSubclass( XtParent(widget), xmpTableWidgetClass ))
    {
	WcWARN1( w, "XmpTableChildConfig", "notXmpTable", 
"Xmp Warning: XmpTableChildConfig( %s ... ) - Widget not child of an XmpTable.",
			params[0] );
	return;
    }

    if ( 3 < *num_params )
	hSpan = atoi( params[3] );
    if ( 4 < *num_params )
	vSpan = atoi( params[4] );
    if ( 5 < *num_params )
	opt = XmpTableOptsParse( params[5] );

    XmpTableChildConfig( widget, col, row, hSpan, vSpan, opt );
}

/*ARGSUSED*/
void XmpTableChildPositionCB( w, client, ignored )
    Widget	w;
    XtPointer	client;
    XtPointer	ignored;
{
    WcInvokeAction( XmpTableChildPositionACT, w, client );
}

/*ARGSUSED*/
void XmpTableChildPositionACT( w, unusedEvent, params, num_params )
    Widget      w;
    XEvent     *unusedEvent;
    String     *params;
    Cardinal   *num_params;
{
    Widget	widget;
    Position	col;
    Position	row;

    if (*num_params != 3 )
    {
	WcWARN( w, "XmpTableChildPosition", "wrongNumArgs", 
"Xmp Warning: XmpTableChildPosition( ... ) - Wrong number of arguments.\n\
Xmp Usage: XmpTableChildPosition( w col row )" );
	return;
    }

    widget = WcFullNameToWidget( w, params[0] );
    col = atoi( params[1] );
    row = atoi( params[2] );

    if ((Widget)0 == widget)
    {
	WcWARN1( w, "XmpTableChildPosition", "widgetNotFound", 
"Xmp Warning: XmpTableChildPosition( %s ... ) could not find widget.",
			params[0] );
	return;
    }
    else if ( ! XtIsSubclass( XtParent(widget), xmpTableWidgetClass ))
    {
	WcWARN1( w, "XmpTableChildPosition", "notXmpTable", 
"Xmp Warning: XmpTableChildPosition( %s ... ) Widget not child of an XmpTable.",
			params[0] );
	return;
    }

    XmpTableChildPosition( widget, col, row );
}

/*ARGSUSED*/
void XmpTableChildResizeCB( w, client, ignored )
    Widget	w;
    XtPointer	client;
    XtPointer	ignored;
{
    WcInvokeAction( XmpTableChildResizeACT, w, client );
}

/*ARGSUSED*/
void XmpTableChildResizeACT( w, unusedEvent, params, num_params )
    Widget      w;
    XEvent     *unusedEvent;
    String     *params;
    Cardinal   *num_params;
{
    Widget	widget;
    Dimension	hSpan;
    Dimension	vSpan;

    if (*num_params != 3 )
    {
	WcWARN( w, "XmpTableChildResize", "wrongNumArgs", 
"Xmp Warning: XmpTableChildResize( ... ) Wrong number of arguments.\n\
Xmp Usage: XmpTableChildResize( w h_span v_span )" );
	return;
    }

    widget = WcFullNameToWidget( w, params[0] );
    hSpan = atoi( params[1] );
    vSpan = atoi( params[2] );

    if ((Widget)0 == widget)
    {
	WcWARN1( w, "XmpTableChildResize", "widgetNotFound", 
"Xmp Warning: XmpTableChildResize( %s ... ) - Widget not found.",
			params[0] );
	return;
    }
    else if ( ! XtIsSubclass( XtParent(widget), xmpTableWidgetClass ))
    {
	WcWARN1( w, "XmpTableChildResize", "notXmpTable", 
"Xmp Warning: XmpTableChildResize( %s ... ) - Widget not child of an XmpTable.",
			params[0] );
	return;
    }

    XmpTableChildResize( widget, hSpan, vSpan );
}

/*ARGSUSED*/
void XmpTableChildOptionsCB( w, client, ignored )
    Widget	w;
    XtPointer	client;
    XtPointer	ignored;
{
    WcInvokeAction( XmpTableChildOptionsACT, w, client );
}

/*ARGSUSED*/
void XmpTableChildOptionsACT( w, unusedEvent, params, num_params )
    Widget      w;
    XEvent     *unusedEvent;
    String     *params;
    Cardinal   *num_params;
{
    Widget 	 widget;
    XmpTableOpts opt;

    if (*num_params != 2 )
    {
	WcWARN( w, "XmpTableChildOptions", "wrongNumArgs", 
"Xmp Warning: XmpTableChildOptions() - Wrong number of arguments.\n\
Xmp Usage: XmpTableChildOptions( w options )" );
	return;
    }

    if ((Widget)0 == (widget = WcFullNameToWidget( w, params[0] )) )
    {
	WcWARN1( w, "XmpTableChildOptions", "widgetNotFound", 
"Xmp Warning: XmpTableChildOptions( %s ... ) - Widget not found.",
			params[0] );
	return;
    }
    else if ( ! XtIsSubclass( XtParent(widget), xmpTableWidgetClass ))
    {
	WcWARN1( w, "XmpTableChildOptions", "notXmpTable", 
"Xmp Warning: XmpTableChildOptions( %s ... ) Widget not child of an XmpTable.",
			params[0] );
	return;
    }

    opt = XmpTableOptsParse( params[1] );
    XmpTableChildOptions( widget, opt );
}

/* -- Register all Xmp provided actions and callbacks.
*******************************************************************************
*/

void XmpAddActionsAndCallbacks ( app )
    XtAppContext app;
{
    static XtActionsRec XmpActions[] = {
	{"XmpPopup",			XmpPopupACT			},
	{"XmpPopupACT",			XmpPopupACT			},
	{"XmpFixTranslations",		XmpFixTranslationsACT		},
	{"XmpFixTranslationsACT",	XmpFixTranslationsACT		},
	{"XmpAddMwmCloseCallback",	XmpAddMwmCloseCallbackACT	},
	{"XmpAddMwmCloseCallbackACT",	XmpAddMwmCloseCallbackACT	},
	{"XmpAddTabGroup",		XmpAddTabGroupACT		},
	{"XmpAddTabGroupACT",		XmpAddTabGroupACT		},
	{"XmpMessageWidget",		XmpMessageWidgetACT		},
	{"XmpMessageWidgetACT",		XmpMessageWidgetACT		},
	{"XmpTableChildConfig",		XmpTableChildConfigACT		},
	{"XmpTableChildConfigACT",	XmpTableChildConfigACT		},
	{"XmpTableChildPosition",	XmpTableChildPositionACT	},
	{"XmpTableChildPositionACT",	XmpTableChildPositionACT	},
	{"XmpTableChildResize",		XmpTableChildResizeACT		},
	{"XmpTableChildResizeACT",	XmpTableChildResizeACT		},
	{"XmpTableChildOptions",	XmpTableChildOptionsACT		},
	{"XmpTableChildOptionsACT",	XmpTableChildOptionsACT		},
	/* -- compatibility: */
	{"MriPopup",			XmpPopupACT			},
	{"MriPopupACT",			XmpPopupACT			},
    };

    ONCE_PER_XtAppContext( app );

/* -- Register Motif specific action functions */
    XtAppAddActions(app, XmpActions, XtNumber(XmpActions));

/* -- Register Motif specific callback functions */
#define RCALL( name, func ) WcRegisterCallback ( app, name, func, (XtPointer)0 )

    RCALL("XmpFixTranslations",		XmpFixTranslationsCB		);
    RCALL("XmpFixTranslationsCB",	XmpFixTranslationsCB		);
    RCALL("XmpAddMwmCloseCallback",	XmpAddMwmCloseCallbackCB	);
    RCALL("XmpAddMwmCloseCallbackCB",	XmpAddMwmCloseCallbackCB	);
    RCALL("XmpAddTabGroup",		XmpAddTabGroupCB		);
    RCALL("XmpAddTabGroupCB",		XmpAddTabGroupCB		);
    RCALL("XmpMessageWidget",		XmpMessageWidgetCB		);
    RCALL("XmpMessageWidgetCB",		XmpMessageWidgetCB		);
    RCALL("XmpTableChildConfig",	XmpTableChildConfigCB		);
    RCALL("XmpTableChildConfigCB",	XmpTableChildConfigCB		);
    RCALL("XmpTableChildPosition",	XmpTableChildPositionCB		);
    RCALL("XmpTableChildPositionCB",	XmpTableChildPositionCB		);
    RCALL("XmpTableChildResize",	XmpTableChildResizeCB		);
    RCALL("XmpTableChildResizeCB",	XmpTableChildResizeCB		);
    RCALL("XmpTableChildOptions",	XmpTableChildOptionsCB		);
    RCALL("XmpTableChildOptionsCB",	XmpTableChildOptionsCB		);
}
