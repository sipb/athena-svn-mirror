/*
** Copyright (c) 1990 David E. Smyth
**
** This file was derived from work performed by Martin Brunecky at
** Auto-trol Technology Corporation, Denver, Colorado, under the
** following copyright:
**
*******************************************************************************
* Copyright 1990 by Auto-trol Technology Corporation, Denver, Colorado.
*
*                        All Rights Reserved
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted, provided
* that the above copyright notice appears on all copies and that both the
* copyright and this permission notice appear in supporting documentation
* and that the name of Auto-trol not be used in advertising or publicity
* pertaining to distribution of the software without specific, prior written
* permission.
*
* Auto-trol disclaims all warranties with regard to this software, including
* all implied warranties of merchantability and fitness, in no event shall
* Auto-trol be liable for any special, indirect or consequential damages or
* any damages whatsoever resulting from loss of use, data or profits, whether
* in an action of contract, negligence or other tortious action, arising out
* of or in connection with the use or performance of this software.
*******************************************************************************
**
** Redistribution and use in source and binary forms are permitted
** provided that the above copyright notice and this paragraph are
** duplicated in all such forms and that any documentation, advertising
** materials, and other materials related to such distribution and use
** acknowledge that the software was developed by David E. Smyth.  The
** name of David E. Smyth may not be used to endorse or promote products
** derived from this software without specific prior written permission.
** THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*/

/*
* SCCS_data: @(#)WcCallb.c 1.04 ( 30 September 1990 )
*
* Subsystem_group:
*
*     Widget Creation Library
*
* Module_description:
*
*     This module contains the convenience callbacks used to create and 
*     manage a widget tree using the Xrm databse.
*
*     Several convenience callbacks are provided with the package, allowing 
*     deferred widget creation, control (manage/unmanage) and other utility
*     functions.
*
* Module_interface_summary: 
*
*     Convenience Callbacks:
*
* Module_history:
*                                                  
*   Several of the callbacks and the callback registration routine were
*   originally written by Martin Brunecky at Auto-Trol, between about
*   the first of February 1990 until about 18 April 1990.
*
*   Additional callbacks and modifications to all callbacks and the
*   callback registration routine were made by David Smyth at Jet
*   Propulsion Laboratories following about 15 March 1990.
*
*   WcLoadResourceFileCB enhanced to remember which resource files have
*   been loaded (by resource file name, not complete path name).

* Design_notes:
*
*   For VMS, we could have used LIB$FIND_IMAGE_SYMBOL and use dynamic
*   (runtime) binding. But since most UNIX systems lack such capability,
*   we stick to the concept of "registration" routines.
*
*   All these callbacks could probably be declared as static.  They
*   were not because applications may want to link them to widgets
*   via C code.  
*
*   When Motif runs on release 4 Intrinsics, then all argument parsing
*   should be replaced with coverters, so conversions get cached.  This
*   will improve performance, especially for pop-ups.
*
*******************************************************************************
*/
/*
*******************************************************************************
* Include_files.
*******************************************************************************
*/

#include "WcCreate.h"
#include "WcCreateP.h"

/*
*******************************************************************************
* Private_type_declarations.
*******************************************************************************
*/

/*
*******************************************************************************
* Private_macro_definitions.
*******************************************************************************
*/

/*
*******************************************************************************
* Private_data_definitions.
*******************************************************************************
*/

/* NOTE: These are shared arrays because they are large: i.e.,
** this is a performance optimization intended to reduce page
** faults which can occur while making large extensions to the
** stack space.  Wait a minute: wouldn't this just happen
** once, and then the stack space is alloc'd?  Yes, but a
** huge stack space runs the risk of getting swapped, which causes
** page faults.  This is probably a nit-picky sort of optimization.
** Remember that D.Smyth is an old sys programmer from the stone
** ages, and pity him instead of flaming him.
** Be careful when filling msg not to call any funcs in here, 
** so the message does not get garbled.
*/

static char	msg[MAX_ERRMSG];
static char	cleanName[MAX_PATHNAME];
static Widget	widget_list[MAX_CHILDREN];

/*
*******************************************************************************
* Private_function_declarations.
*******************************************************************************
*/

/*
*******************************************************************************
* Public_callback_function_declarations.
*******************************************************************************
    The client data argument of callbacks MUST be a null terminated
    string.  If client == (char*)0 this is an error.  The correct way
    to pass no client information is *client == '\0'. The CvtStringToCallback
    converter which actually causes these functions to be called (adds
    these functions to widget's callback lists) does ensure that the
    client data is a proper null terminated string.

    Callbacks are not intended to be re-entrant nor recursive.  Many of
    these use static buffers.
*******************************************************************************
*/

/*
    -- Create Dynamically Created Children from Xrm Database
*******************************************************************************
    This callback creates one or more specified children of a parent widget.
    If parent name is `this' then widget invoking callback is used as the
    parent.  Parent name can also be a wildcarded path name.  Child names
    must be single part, specific children of the parent. Client data format:

             parent, child [,child] ...
    
    This callback is used for deferred sub-tree creation, where named child
    creation has been postponed because it was not included in a wcChildren
    resource value.
*/

void WcCreateChildrenCB ( w, parent_children, unused )
    Widget w;
    char* parent_children;	/* parent + list of named children */
    caddr_t unused;		/* call data from widget, not used */
{
    char*	children;
    Widget	parent;

    if ( *parent_children == NUL ) 
    {
	XtWarning(
            "WcCreateChildrenCB ( ) - Failed \n\
             Usage: WcCreateChildrenCB ( parent, child [, child ] ...) \n\
                    Name of parent can be `this' or wildcarded pathname, \n\
                    Name of child must be single part name from parent. \n\
             Problem: No widget names provided.");
	return;
    }

    children = WcCleanName( parent_children, cleanName );

    children = WcSkipWhitespace_Comma( children );

    if ((Widget)NULL == (parent = WcFullNameToWidget( w, cleanName )) )
    {
	sprintf( msg,
            "WcCreateChildrenCB (%s) - Failed \n\
             Usage: WcCreateChildrenCB ( parent, child [, child ] ...) \n\
                    Name of parent can be `this' or wildcarded pathname, \n\
                    Name of child must be single part name from parent. \n\
             Problem: Parent widget named `%s' not found.",
	    parent_children, cleanName);
	XtWarning( msg );
	return;
    }

    if (*children == NUL)
    {
        sprintf( msg,
            "WcCreateChildrenCB (%s) - Failed \n\
             Usage: WcCreateChildrenCB ( parent, child [, child ] ...) \n\
                    Name of parent can be `this' or wildcarded pathname, \n\
                    Name of child must be single part name from parent. \n\
             Problem: No children names provided.",
	    parent_children );
	XtWarning( msg );
        return;
    }

    WcCreateNamedChildren ( parent, children );
}

/******************************************************************************
**  Manage or Unmanage named widget(s)
*******************************************************************************
    This callback translates string passed in as client data into one or more
    widget ids, and manages or unmanages it or them.  Client data format:
	name [, name] ...
    Name can include `this' and other path names.
******************************************************************************/

#ifdef FUNCTION_PROTOTYPES
static void ManageOrUnmanage( Widget, char*, char*, void (*proc)() );
#else
static void ManageOrUnmanage();
#endif

void WcManageCB ( w, widgetNames, unused )
    Widget w;
    char* widgetNames;
    caddr_t unused;		/* call data from widget, not used */
{
    ManageOrUnmanage( w, widgetNames, "WcManageCB", XtManageChildren );
}

void WcUnmanageCB ( w, widgetNames, unused )
    Widget w;
    char* widgetNames;
    caddr_t unused;             /* call data from widget, not used */
{
    ManageOrUnmanage( w, widgetNames, "WcUnmanageCB", XtUnmanageChildren );
}

static void ManageOrUnmanage ( w, widgetNames, callbackName, proc )
    Widget w;
    char* widgetNames;
    char* callbackName;
    void (*proc)();
{
    int         numWidgets = 0;
    char*       s = widgetNames;

    while (*s && numWidgets < MAX_CHILDREN)
    {
        s = WcCleanName ( s, cleanName );
        s = WcSkipWhitespace_Comma ( s );
        if ( widget_list[numWidgets] = WcFullNameToWidget ( w, cleanName ) )
            numWidgets++;
        else
        {
            sprintf(msg,
            "%s (%s) - Widget `%s' ignored \n\
             Usage:   %s ( widget_name [, widget_name] ... ) \n\
             Problem: Could not find widget named `%s'.",
             callbackName, widgetNames, cleanName, callbackName, cleanName);
            XtWarning( msg );
        }
    }
    if (numWidgets >= MAX_CHILDREN)
    {
	sprintf(msg,
            "%s (%s) - Failed \n\
             Usage:   %s ( widget_name [, widget_name] ... ) \n\
             Problem: Too many widgets (more than MAX_CHILDREN).",
             callbackName, widgetNames, callbackName);
	XtWarning( msg );
	numWidgets = 0;
    }

    if ( numWidgets )
        /* proc is either XtManageChildren or XtUnmanageChildren */
        proc ( widget_list, numWidgets );
}

/*
    -- Manage or unamange named children callback
*******************************************************************************
    These callbacks translates a string passed in as client data into a parent
    widget id, and names of children of that parent.  If parent name is
    `this' then widget invoking callback is used as the parent.  Parent
    name can also be a wildcarded path name.  Child names must be single
    part, specific children of the parent. Client data format:

             parent, child [,child] ...

    This callback can be used as an alternate for WcManageCB but it is
    really intended to be used to manage/unmanage children of widgets 
    which have multiple instances, and where the parent name is `this'.
*/
#ifdef FUNCTION_PROTOTYPES
static void ManageOrUnmanageChildren( Widget, char*, char*, void (*proc)() );
#else
static void ManageOrUnmanageChildren();
#endif

void WcManageChildrenCB ( w, parent_children, unused )
    Widget 	w;
    char*	parent_children;/* client data, list of named children  */
    caddr_t	unused;		/* call data from widget		*/
{
    ManageOrUnmanageChildren( w, parent_children, 
				"WcManageChildrenCB", XtManageChildren );
}

void WcUnmanageChildrenCB ( w, parent_children, unused )
    Widget      w;
    char*       parent_children;/* client data, list of named children  */
    caddr_t     unused;         /* call data from widget                */
{
    ManageOrUnmanageChildren( w, parent_children,
                                "WcUnmanageChildrenCB", XtUnmanageChildren );
}

static void ManageOrUnmanageChildren( w, parent_children, callbackName, proc )
    Widget w;
    char* parent_children;      /* client data, list of named children  */
    char* callbackName;		/* WcManageChildrenCB or WcUnmanageChildrenCB */
    void (*proc)();		/* XtManageChildren or XtUnmanageChildren */
{
    int         	numWidgets = 0;
    char*		next;
    Widget		parent;

    if ( *parent_children == NUL )
    {
        sprintf(msg,
            "%s ( ) - Failed \n\
             Usage: %s ( parent, child [, child ] ...) \n\
                    Name of parent can be `this' or wildcarded pathname, \n\
                    Name of child must be single part name from parent. \n\
             Problem: No widget names provided.",
	    callbackName, callbackName);
	XtWarning( msg );
        return;
    }

    next = WcCleanName( parent_children, cleanName );
    if ((Widget)NULL == (parent = WcFullNameToWidget( w, cleanName )) )
    {
        sprintf( msg,
            "%s (%s) - Failed \n\
             Usage: %s ( parent, child [, child ] ...) \n\
                    Name of parent can be `this' or wildcarded pathname, \n\
                    Name of child must be single part name from parent. \n\
             Problem: Parent widget named `%s' not found.",
            callbackName, parent_children, callbackName, cleanName);
        XtWarning( msg );
        return;
    }

    while (*next && numWidgets < MAX_CHILDREN)
    {
        next = WcCleanName( next, cleanName );
	if (widget_list[numWidgets] = WcChildNameToWidget( parent, cleanName ))
	    numWidgets++;
	else
	{
	    sprintf( msg,
            "%s (%s) - Child `%s' ignored \n\
             Usage: %s ( parent, child [, child ] ...) \n\
                    Name of parent can be `this' or wildcarded pathname, \n\
                    Name of child must be single part name from parent. \n\
             Problem: Child widget named `%s' not found.",
	    callbackName, parent_children, callbackName, cleanName);
	    XtWarning( msg );
	}
    }

    if (numWidgets >= MAX_CHILDREN)
    {
        sprintf(msg,
            "%s (%s) - Failed \n\
             Usage: %s ( parent, child [, child ] ...) \n\
                    Name of parent can be `this' or wildcarded pathname, \n\
                    Name of child must be single part name from parent. \n\
             Problem: Too many widgets (more than MAX_CHILDREN).",
             callbackName, parent_children, callbackName);
        XtWarning( msg );
        numWidgets = 0;
    }

    if ( numWidgets )
        /* proc is either XtManageChildren or XtUnmanageChildren */
        proc ( widget_list, numWidgets );
}

/*
    -- Destroy named children callback
*******************************************************************************
    This callback translates string passed in as client data into a widget id
    and destroys it. A comma separated list of widgets can be specified.
    `this' means the invoking widget.
*/

void WcDestroyCB ( w, widgetNames, unused )
    Widget  w;
    char*   widgetNames;	/* client data, widgets to be destroyed */
    caddr_t unused;		/* call data from widget, not used	*/
{
    int		widget_count = MAX_CHILDREN;
    char*	unConvertedNames;
    int         i;

    unConvertedNames = WcNamesToWidgetList ( w, widgetNames, 
				    widget_list, &widget_count );
    if ( unConvertedNames[0] != NUL )
    {
	sprintf(msg,
            "WcDestroyCB (%s) \n\
             Usage: WcDestroyCB ( widget [, widget ] ...) \n\
                    Name of widget can be `this' or wildcarded pathname. \n\
		    Problem: No widgets found named %s.",
             widgetNames, unConvertedNames);
        XtWarning( msg );
    }

    for (i=0; i<widget_count; i++)
       XtDestroyWidget ( widget_list[i] );
}

/*
    -- Parse name_res_resVal into name, res, resVal
*******************************************************************************
    This is a private function to WcSetValueCB()
*/

static void 
ParseNameResResVal( name_res_resVal, target_name, resource, res_val )
    char* name_res_resVal;	/* input */
    char* target_name;		/* output */
    char* resource;		/* output */
    char* res_val;		/* output */
{
    register char *d,*s;

    s = name_res_resVal;
    /* copy from name_res_resVal into target_name[],
    ** ignore initial whitespace, stop at trailing `:'  
    ** Then backup to get final segment, the resource name
    */
    for ( s = name_res_resVal ; *s && *s <= ' ' ; s++ )
	;	/* skip initial whitespace */
    for ( d = target_name ; *s && *s != ':' ; s++, d++ )
	*d = *s;
    for ( ; target_name < d && (*d != '.' && *d != '*') ; s--, d-- )
	;
    *d = NUL;

    /* OK, now target_name is null terminated.
    ** s points at final `.' or `*' in name_resName, 
    ** now we copy to resource[].
    */
    for ( s++, d = resource ; *s && *s != ':' ; s++, d++ )
        *d = *s;
    *d = NUL;

    /* OK, now resource is null terminated.
    ** s points at the `:' now we skip whitespace and the rest is value
    ** until we hit the NUL terminator.
    */
    for (s++ ; *s && *s <= ' ' ; s++ )
        ;       /* skip initial whitespace */
    for (d = res_val ; *s ; s++, d++ )
	*d = *s;
    do
	*d = NUL;	/* null terminate, delete trailing whitespace */
    while (*--d <= ' ');
}

/*
   -- Set Resource Value on Widget
*******************************************************************************
    This callback sets a resource value on the named widget.

    The client data argument has a format:

        target_widget_name.resource_name: resource value

    The special resource value of "this" means "this widget."  Typically,
    using "this" as the resource value is used to set the "XmNdefaultButton"
    resource on a XmbulletinBoard, "menuBar", "workArea", etc on XmMainWindows,
    the subMenuId resource on menuBar cascade buttons, and so on.

    Just about everything this function does is related to providing
    nice error messages.
*/

void WcSetValueCB ( w, name_res_resVal, unused )
    Widget  w;  
    char*   name_res_resVal;		/* client data: name.resName: resVal */
    caddr_t unused;			/* call data from wudget, not used   */
{
    /* Note: static buffers make this routine NON-REENTRANT!! */
    static char    target_name[MAX_XRMSTRING];
    static char    resource[MAX_XRMSTRING];
    static char    res_val[MAX_XRMSTRING];

    Widget	target;
    char*	res_type;	/* must be XtFree'd */

    ParseNameResResVal( name_res_resVal, target_name, resource, res_val );

    /* Check for syntax error: if any of the strings are null, wrongo!
    */
    if ( target_name[0] == NUL || resource[0] == NUL || res_val[0] == NUL )
    {
        char *missing;

        if (target_name[0] == NUL) missing = "target_widget_name";
        else if (resource[0] == NUL) missing = "res_name";
        else if (res_val[0] == NUL) missing = "res_value";

        sprintf ( msg,
            "WcSetValueCB (%s) - Failed \n\
             Usage:   WcSetValueCB( target_widget_name.res_name: res_value ) \n\
             Problem: Missing %s argument.",
            name_res_resVal, missing );
        XtWarning( msg ); 
	return;
    }

    /* See if we can find the target widget */
    if (! (target = WcFullNameToWidget( w, target_name )) )
    {
        sprintf ( msg,
            "WcSetValueCB (%s)  \n\
             Usage:   WcSetValueCB( target_widget_name.res_name: res_value ) \n\
             Problem: Could not find widget named `%s'",
             name_res_resVal, target_name );
        XtWarning( msg );
        return;
    }

    /* Make certain widget has this resource type */
    if (! (res_type = WcGetResourceType( target, resource )) )
    {
	sprintf ( msg,
            "WcSetValueCB (%s) - Failed \n\
             Usage:   WcSetValueCB( target_widget_name.res_name: res_value ) \n\
             Problem: widget %s does not have a resource named %s.",
	name_res_resVal, target_name, resource);
        XtWarning(msg);
	return;
    }

    /* Try this heuristic: if the resource type has "Widget" somewhere within
    ** it, then see if we can convert the resource value to a widget using 
    ** WcFullNameToWidget.  This allow relative naming from the calling
    ** widget, including `this' or ^^foobar.
    ** WcStrStr( res_type, "Widget") returns NULL if "Widget" is not in res_type
    */
    if ( WcStrStr( res_type, "Widget") 
#ifdef MOTIF
	|| WcStrStr( res_type, "Window")  /* wrong: but that's a menuWidget */
#endif
	)
    {
	Widget	res_widget;
	Arg	arg[1];

	if (res_widget = WcFullNameToWidget( w, res_val ))
	{
	    XtSetArg( arg[0], resource, res_widget);
	    XtSetValues( target, arg, 1 );
	    return;
	}
    }

    /* We have the type, so do conversion, and then call XtSetValues. */

    WcSetValueFromStringAndType( target, resource, res_val, res_type );
    XtFree( res_type );
}

/*
    -- Change sensitivity of widgets.
*******************************************************************************
    This callback translates string passed in as client data into widget ids
    and sets each to be sensitve/insensitive. A comma separated list of 
    widgets can be specified.  `this' means the invoking widget.  

    This callback someday should take care of the problem with text
    widgets - they don't get grey when insensitive, which must be a bug.
*/

#ifdef FUNCTION_PROTOTYPES
static void ChangeSensitivity( Widget, char*, char*, int );
#else
static void ChangeSensitivity();
#endif

void WcSetSensitiveCB ( w, widgetNames, unused )
    Widget  w;
    char*   widgetNames;        /* client data, widgets to be destroyed */
    caddr_t unused;		/* call data from widget is not used    */
{
    ChangeSensitivity ( w, widgetNames, "WcSetSensitiveCB", (Boolean)TRUE );
}

void WcSetInsensitiveCB ( w, widgetNames, unused )
    Widget  w;
    char*   widgetNames;        /* client data, widgets to be destroyed */
    caddr_t unused;             /* call data from widget is not used    */
{
    ChangeSensitivity ( w, widgetNames, "WcSetInsensitiveCB", (Boolean)FALSE );
}

static void ChangeSensitivity ( w, widgetNames, callbackName, sensitive )
    Widget  w;
    char*   widgetNames;        /* client data, widgets to be destroyed */
    char*   callbackName;	/* "WcSetSensitiveCB" or "WcSetInsensitiveCB" */
    int     sensitive;
{
    int    widget_count = MAX_CHILDREN;
    char*  unConvertedNames;
    int    i;

    unConvertedNames = WcNamesToWidgetList ( w, widgetNames,
                                    widget_list, &widget_count );
    if ( unConvertedNames[0] != NUL )
    {
        sprintf(msg,
            "%s (%s) - One or more widget names ignored \n\
             Usage: %s ( widget [, widget ] ...) \n\
                    Name of widget can be `this' or wildcarded pathname. \n\
                    Problem: No widgets found named %s.",
             callbackName, widgetNames, callbackName, unConvertedNames);
        XtWarning( msg );
    }

    for (i=0; i<widget_count; i++)
	XtSetSensitive ( widget_list[i], sensitive );
}

/*
    -- Load Resource File
*******************************************************************************
    This callbacks loads specified resource file into application
    resource database. It allows to load resources on as-needed
    basis, reducing the intitial resource file load overhead. 
    The file to load is specified as client data. The directory search 
    for the file (should be) the same as for application class resource file.
    
    To prevent repeated loads of the same file, the callback keeps
    track of each filename.  Note that I do not allow a file to be
    re-loaded even if it is changed, or if a new file of the same 
    name appears on the search path.  This was done for two reasons:
    first, it makes the code more portable, as I don't have to depend
    upon various system calls.  Second, resources can't be un-written,
    so a user might get the wrong impression that a resource specification
    can be deleted, and the resource file re-loaded, and something will
    happen.  It just isn't so.

    NOTE:
    The file search list rule used here is a gross simplification of the R3
    resource file search mechanism, without the $LANG provision.
    I hope I can use R4 soon and do it RIGHT, but I depend on Motif for now,
    and do not want to duplicate all the Motif code here.
    Here I look into two directories only, which may be defined as environmental
    variables:
         XAPPLRESDIR  - defaults to "/usr/lib/X11/app-defaults/"
	 XUSERRESDIR  - defaults to HOME directory
*/

#ifdef VAX
#define XAPPLRESDIR "sys$library:"
#else
#define XAPPLRESDIR "/usr/lib/X11/app-defaults/"
#endif

void WcLoadResourceFileCB ( w,  resFileName, unused )
    Widget w;
    char*  resFileName;	/* client data, X resources file name */
    caddr_t unused;	/* call data,   not used */
{
    static char		name[MAX_PATHNAME];		/* so not on stack */
    XrmQuark		nameQ;
    static XrmQuark	nameQuarks[MAX_RES_FILES];	/* initially all 0 */
    int			i;
    char*		path;
    static char		filename[MAX_PATHNAME];		/* so not on stack */
    XrmDatabase		rdb;
    Display*		dpy = XtDisplay(w);
    extern char*	getenv();
    Boolean		found = FALSE;

    (void) WcCleanName( resFileName, name );

/*  -- check pathname presence */
    if ( *name == NUL )
    {
        XtWarning ( 
            "WcLoadResourceFileCB () - Failed \n\
             Usage:   WcLoadResourceFileCB( resource_file_name ) \n\
             Problem: No file name provided.");
        return;
    }

/*  -- check for repeated load, remember if first load */
    nameQ = XrmStringToQuark( name );
    i = 0;
    while ( nameQuarks[i] && i < MAX_RES_FILES )
    {
	if ( nameQuarks[i] == nameQ )
	    return;			/* NB: not an error, its even common */
	else
	    i++;
    }
    nameQuarks[i] = nameQ;

/*  -- Look for file in application class resources file directory */
    if ((path = getenv("XAPPLRESDIR")) == NULL) 
    {
	path = XAPPLRESDIR ;
    } 
    strcpy ( filename, path );
    strcat ( filename, name );
 
    if ((rdb = XrmGetFileDatabase(filename)) != NULL )
    {
	XrmMergeDatabases (rdb, &(dpy->db) );
	found = TRUE;
    }	

#ifdef I_DONT_KNOW_WHY_THIS_IS_HERE
/*  -- Look for file in user class resources file directory */
    if ((path = getenv("XUSERRESDIR")) == NULL) 
    {
	path = ( char* )malloc (MAX_PATHNAME);
        strcpy ( path, RootDirName(filename));
    } 
    strcpy ( filename, path );
    strcat ( filename, name );
    free   ( path );
    if ((rdb = XrmGetFileDatabase(filename)) != NULL )
    {
	XrmMergeDatabases (rdb, &(dpy->db) );
        found = TRUE;
    }
#endif

/*  -- Look for file in current working directory */
    strcpy ( filename, name );
    if ((rdb = XrmGetFileDatabase(filename)) != NULL )
    {
        XrmMergeDatabases (rdb, &(dpy->db) );
        found = TRUE;
    }

/*  -- warn the user if no file found */
    if (!found)
    {
	sprintf  ( msg, 
            "WcLoadResourceFileCB (%s) - Failed \n\
             Usage:   WcLoadResourceFileCB( resource_file_name ) \n\
             Problem: Cannot load resource file %s",
	resFileName, name );
	XtWarning( msg );
    }
}

/*
  -- WcTraceCallback
*******************************************************************************
    This is a simple traceback callback, used to assist in interface
    debugging. The callback prints the invoking wiget pathname and
    a specified message on std. output.
*/

void WcTraceCB ( w, annotation, unused )
    Widget w;
    char* annotation;	/* client data, traceback annotation */
    caddr_t unused;	/* call data,   not used */
{
    char* name = WcWidgetToFullName( w );
    
    printf("TraceCB for %s: %s\n", name, annotation );
    XtFree( name );
}

/*
  -- Popup named widget
*******************************************************************************
    These callbacks translate a string passed in as client data into a 
    widget id.  

    A grab kind value of XtGrabNone has the effect of allowing 
    non-modal popups.  This is the preferred type: rarely use modal pop-ups.
    This is registered as PopupCB.

    A grab kind value of XtGrabExclusive has the effect of grabbing all
    application events, allowing modal popups.  This is registered as 
    PopupGrabCB.
*/

#ifdef FUNCTION_PROTOTYPES
static void Popup ( Widget, char*, char*, XtGrabKind );
#else
static void Popup ();
#endif

void WcPopupCB ( w, name, unused )
    Widget      w;
    char*       name;
    caddr_t	unused;
{
    Popup ( w, name, "WcPopupCB", XtGrabNone );
}

void WcPopupGrabCB ( w, name, unused )
    Widget      w;
    char*       name;
    caddr_t     unused;
{
    Popup ( w, name, "WcPopupGrabCB", XtGrabExclusive );
}

static void Popup ( w, name, callbackName, grab )
    Widget	w;
    char*	name;
    char*	callbackName;
    XtGrabKind	grab;
{
    Widget      widget;

    (void)WcCleanName ( name, cleanName );
    widget = WcFullNameToWidget ( w, cleanName );

    if (XtIsShell(widget))
    {
        XtPopup  ( widget, grab );
    }
    else
    {
        sprintf( msg,
            "%s (%s) - Failed \n\
             Usage: %s (shell_widget_name) \n\
             Problem: `%s' is not a shell widget.",
             callbackName, name, callbackName, cleanName);
        XtWarning( msg );
    }
}

/*
  -- Popdown named widget
*******************************************************************************
    This callback translates string passed in as client data into a widget id
    and pops-down a popup shell widget.
*/

void WcPopdownCB ( w, name, unused ) 
    Widget	w;
    char*	name;
    caddr_t	unused;
{
    Widget      widget;

    (void)WcCleanName ( name, cleanName );
    widget = WcFullNameToWidget ( w, cleanName );

    if (XtIsShell(widget))
    {
        XtPopdown  ( widget );
    }
    else
    {
        sprintf( msg,
            "WcPopdownCB (%s) Failed \n\
             Usage: WcPopdownCB (shell_widget_name) \n\
             Problem: `%s' is not a shell widget.",
             name, cleanName);
        XtWarning( msg );
    }
}

/*
  -- Map and Unmap named widget
*******************************************************************************
    These callbacks translate a string passed as client data into a widget id
    and invokes either XtMapWidget() os XtUnmapWidget() as appropriate.
*/

void WcMapCB (w, name, unused )
    Widget      w;
    char*       name;
    caddr_t     unused;
{
    Widget      widget;

    (void)WcCleanName ( name, cleanName );
    widget = WcFullNameToWidget ( w, cleanName );

    if (XtIsShell(widget))
    {
        XtMapWidget ( widget );
    }
    else
    {
        sprintf( msg,
            "WcMapCB (%s) Failed \n\
             Usage: WcMapCB (shell_widget_name) \n\
             Problem: `%s' is not a shell widget.",
             name, cleanName);
        XtWarning( msg );
    }
}

void WcUnmapCB (w, name, unused )
    Widget      w;
    char*       name;
    caddr_t     unused;
{
    Widget      widget;

    (void)WcCleanName ( name, cleanName );
    widget = WcFullNameToWidget ( w, cleanName );

    if (XtIsShell(widget))
    {
        XtUnmapWidget ( widget );
    }
    else
    {
        sprintf( msg,
            "WcUnmapCB (%s) Failed \n\
             Usage: WcUnmapCB (shell_widget_name) \n\
             Problem: `%s' is not a shell widget.",
             name, cleanName);
        XtWarning( msg );
    }
}

/*
  -- Invoke shell command
*******************************************************************************
    Call system().
*/

void WcSystemCB ( w, shellCmdString, unused )
    Widget      w;
    char*       shellCmdString;
    caddr_t     unused;
{
    system( shellCmdString );
}

/*
  -- Exit the application
*******************************************************************************
    Call exit().
*/

void WcExitCB ( w, exitValString, unused )
    Widget	w;
    char*	exitValString;
    caddr_t	unused;
{
    int exitval = 0;

    /* skip leading garbage before int */
    while (*exitValString)
    {
        if ('0' < *exitValString && *exitValString <= '9')
            break; /* found numbers, convert to exitval */
        exitValString++;
    }

    /* convert to int */
    while (*exitValString)
    {
        if ('0' < *exitValString && *exitValString <= '9')
        {
            exitval = exitval * 10 + (*exitValString - '0');
            exitValString++;
        }
        else
            break;  /* ignore trailing garbage */
    }

    exit( exitval );
}

/*
  -- WcRegisterCreateCallbacks
*******************************************************************************
   Convenience routine, registering all standard callbacks in one application
   call.   Called from WcWidgetCreation(), so application usually never needs
   to call this.
*/

void WcRegisterWcCallbacks ( app )
XtAppContext app;
{
#define RCALL( name, func ) WcRegisterCallback ( app, name, func, NULL )

    /* ONCE_PER_XtAppContext( app ); */
    static int already = 0;
    if (already++)
	return;

    RCALL( "WcCreateChildrenCB",	WcCreateChildrenCB	);
    RCALL( "WcManageCB",		WcManageCB		);
    RCALL( "WcUnmanageCB",		WcUnmanageCB		);
    RCALL( "WcManageChildrenCB",	WcManageChildrenCB	);
    RCALL( "WcUnmanageChildrenCB",	WcUnmanageChildrenCB	);
    RCALL( "WcDestroyCB",		WcDestroyCB		);
    RCALL( "WcSetValueCB",		WcSetValueCB		);
    RCALL( "WcSetSensitiveCB",		WcSetSensitiveCB	);
    RCALL( "WcSetInsensitiveCB",	WcSetInsensitiveCB	);
    RCALL( "WcLoadResourceFileCB",	WcLoadResourceFileCB	);
    RCALL( "WcTraceCB",			WcTraceCB		);
    RCALL( "WcPopupCB",			WcPopupCB		);
    RCALL( "WcPopupGrabCB",		WcPopupGrabCB		);
    RCALL( "WcPopdownCB",		WcPopdownCB		);
    RCALL( "WcMapCB",			WcMapCB			);
    RCALL( "WcUnmapCB",			WcUnmapCB		);
    RCALL( "WcSystemCB",		WcSystemCB		);
    RCALL( "WcExitCB",			WcExitCB		);

#undef CALLBACK
}
