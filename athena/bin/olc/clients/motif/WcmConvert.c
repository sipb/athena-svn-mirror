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
* SCCS_data: @(#)WcConvert.c 1.04 ( 30 September 1990 )
*
* Subsystem_group:
*
*     Widget Creation Library
*
* Module_description:
*
*     This module contains Xt converter functions which convert strings,
*     as found in the Xrm database, into useful types.  
*
*     It also contains the routine which registers all Wc converters.
*
*     The CvtStringToWidget converter takes a pathname which starts 
*     from the application shell an proceeds to a specific widget.  The
*     widget must already have been created.   Note that this converter
*     needs to be used INSTEAD of the XmuCvtStringToWidget which gets
*     registered by the Athena widgets.  The Xmu converter is not so
*     user friendly.  This means this file also declares an external
*     function XmuCvtStringToWidget() which is really CvtStringToWidget,
*     and this needs to be linked before Xmu.
*
*     The CvtStringToCallback converter parses the resource string in 
*     the format:
*
*       ...path:   name[(args)][,name[(args)]]...
*
*     where:  name:   specifies the registered callback function name
*             args:   specifies the string passed to a callback as
*		      "client data".
*
*     Multiple callbacks can be specified for a single callback list
*     resource.  Any callbacks must be "registered" by the application
*     prior converter invocation (.i.e.prior widget creation).
*     If no "args" string is provided, the default "client data" 
*     specified at callback registration are used.
*
*     The CvtStringToConstructor converter searches the Constructor
*     cache for a registered constructor.  
*
*     The CvtStringToClass converter searches the Class cache for a 
*     registered object (widget) class pointer name.
*
*     The CvtStringToClassName converter searches the ClassName cache
*     for a registered object (widget) class name.

*
* Module_interface_summary: 
*
*     Resource converter is invoked indirectly by the toolkit. The
*     converter is added to the toolkit by widgets calling
*     WcAddConverters() in the Wc intialization code.
*
* Module_history:
*                                                  
*   mm/dd/yy  initials  function  action
*   --------  --------  --------  ---------------------------------------------
*   06/08/90  D.Smyth   Class, ClassName, and Constructor converters.
*   05/24/90  D.Smyth   WcAddConverters created from something similar
*   04/03/90  MarBru    CvtStr..  Fixed argument termination with a NUL char
*   02/26/90  MarBru    All       Created
*
* Design_notes:
*
*   For VMS, we could have used LIB$FIND_IMAGE_SYMBOL and use dynamic
*   (runtime) binding. But since most UNIX systems lack such capability,
*   we stick to the concept of "registration" routines.
*
*   One time, I considered applying conversion to callback argument, which
*   would take the burden of conversion from the callback code (runtime)
*   to the callback  conversion (one time initialization). The problem is
*   that some conversions are widget context specific (color to pixel needs
*   display connection), and at the time of callback conversion I do not
*   have a widget. I could require the widget argument, but this would kill
*   caching of the conversion result.
*
*   The sequential search of the callback cache is far from optimal. I should
*   use binary search, or the R4 conversion cache.  I can't use the R4 cache
*   until Motif 1.1 is released which will (supposedly) run on R4 Intrinsics.
*   
*******************************************************************************
*/
/*
*******************************************************************************
* Include_files.
*******************************************************************************
*/

#include <ctype.h>	/* isupper() and tolower macros */
#include <stdio.h>

/*  -- X Window System includes */
#include <X11/StringDefs.h> 

/*  -- Widget Creation Library includes */
#include "WcCreate.h"
#include "WcCreateP.h"

#ifdef MOTIF

/*  -- Motif specific includes for CvtStringToMenuWidget */
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/RowColumnP.h>

#endif /* MOTIF */

/*
*******************************************************************************
* Private_constant_declarations.
*******************************************************************************
*/

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

#define done( type, value ) 			\
{						\
    if ( toVal->addr != NULL )			\
    {						\
	if ( toVal->size < sizeof( type ) )	\
	{					\
	    toVal->size = sizeof( type );	\
	    return;				\
	}					\
	*(type*)(toVal->addr) = (value);	\
    }						\
    else					\
    {						\
	static type static_val;			\
	static_val = (value);			\
	toVal->addr = (caddr_t)&static_val;	\
    }						\
    toVal->size = sizeof(type);			\
    return;					\
}

/*
*******************************************************************************
* Private_data_definitions.
*******************************************************************************
*/

/*
*******************************************************************************
* Private_function_declarations.
*******************************************************************************
*/

/*
    -- Convert String To ClassPtr
*******************************************************************************
    This conversion searches the Object Class cache for the appropriate
    Cache record.  The resource database string is simply the name
    of the class pointer, case insensitive.  The value provided is the
    widget class pointer, as passed to XtCreateWidget().
*/

void CvtStringToClassPtr (args, num_args, fromVal, toVal )
    XrmValue  *args;
    Cardinal  *num_args;
    XrmValue  *fromVal;
    XrmValue  *toVal;
{
    char*       string = (char *) fromVal->addr;
    char        cleanName[MAX_XRMSTRING];
    char*       lowerCase;
    XrmQuark    quark;
    int         i;

    (void)WcCleanName ( string, cleanName );
    lowerCase = WcLowerCaseCopy ( cleanName );
    quark = XrmStringToQuark ( lowerCase );
    XtFree ( lowerCase );

    for (i=0; i<classes_num; i++)
    {
        if ( classes_ptr[i].quark == quark )
        {
            done( WidgetClass, classes_ptr[i].class );
        }
    }
    XtStringConversionWarning (cleanName, "Object Class, not registered.");
}

/*
    -- Convert String To ClassName
*******************************************************************************
    This conversion searches the Class Name cache for the appropriate
    Cache record.  The resource database string is simply the name
    of the class, case insensitive.  The value provided is the widget 
    class pointer, as passed to XtCreateWidget().
*/

void CvtStringToClassName (args, num_args, fromVal, toVal )
    XrmValue  *args;
    Cardinal  *num_args;
    XrmValue  *fromVal;
    XrmValue  *toVal;
{
    char*	string = (char *) fromVal->addr;
    char	cleanName[MAX_XRMSTRING];
    char* 	lowerCase;
    XrmQuark	quark;
    int		i;

    (void)WcCleanName ( string, cleanName );
    lowerCase = WcLowerCaseCopy ( cleanName );
    quark = XrmStringToQuark ( lowerCase );
    XtFree ( lowerCase );

    for (i=0; i<cl_nm_num; i++)
    {
        if ( cl_nm_ptr[i].quark == quark )
        {
	    done( WidgetClass, cl_nm_ptr[i].class );
        }
    }
    XtStringConversionWarning (cleanName, "Class Name, not registered.");
}

/*
    -- Convert String To Constructor
*******************************************************************************
    This conversion searches the Constructor Cache for the appropriate
    Cache record.  The resource database string is simply the name
    of the constructor, case insensitive.  The value provided is a
    Contrstructor Cache Record.  The constructor (func ptr) itself is
    not provided, as the user of this value (generally WcCreateDatabaseChild)
    also likes to have the constructor name as registered for error messages.
*/

void CvtStringToConstructor (args, num_args, fromVal, toVal)
    XrmValue *args;
    Cardinal *num_args;
    XrmValue *fromVal;
    XrmValue *toVal;
{
    char*       string = (char *) fromVal->addr;
    char        cleanName[MAX_XRMSTRING];
    char*       lowerCase;
    XrmQuark    quark;
    int         i;

    (void)WcCleanName ( string, cleanName );
    lowerCase = WcLowerCaseCopy ( cleanName );
    quark = XrmStringToQuark ( lowerCase );
    XtFree ( lowerCase );

    for (i=0; i<constrs_num; i++)
    {
	if ( constrs_ptr[i].quark == quark )
	{
	    done( ConCacheRec*, &(constrs_ptr[i]) );
	}
    }
    XtStringConversionWarning (cleanName, "Constructor, not registered.");
}

/*
    -- Convert String To Callback
*******************************************************************************
    This conversion creates a callback list structure from the X resource
    database string in format:

    name(arg),name(arg).....

    Note "name" is not case sensitive, while "arg" may be - it is passed to
    a callback as client data as a null terminated string (first level
    parenthesis stripped off).  Even if nothing is specified e.g.,
    SomeCallback() there is a null terminated string passed as client
    data to the callback.  If it is empty, then it is the null string.

    Note also that the argument CANNOT be converted at this point: frequently,
    the argument refers to a widget which has not yet been created, or
    uses the context of the callback (i.e., WcUnmanageCB( this ) uses the
    widget which invoked the callback).  
*/

void CvtStringToCallback (args, num_args, fromVal, toVal)
    XrmValue *args;
    Cardinal *num_args;
    XrmValue *fromVal;
    XrmValue *toVal;
{
    static XtCallbackRec *cb;		/* return pointer, MUST be static */

    XtCallbackRec	  callback_list[MAX_CALLBACKS];
    XtCallbackRec	 *rec = callback_list;
    int                   callback_list_len = 0;

    typedef struct 
    {
	char *nsta,*nend;		/* callback name start, end */
	char *asta,*aend;		/* argument string start, end */
    } Segment;

    Segment               name_arg_segments[MAX_CALLBACKS];	
    Segment              *seg = name_arg_segments;
    char		 *string = (char *) fromVal->addr;
    register char        *s;
    register int	  in_parens = 0;

    register int          i;

/*  -- assume error or undefined input argument */
    toVal->size = 0;
    toVal->addr = (caddr_t) NULL;
    if (string == NULL) return;

/*  -- parse input string finding segments   "name(arg)" comma separated */
    seg->nsta = seg->nend = seg->asta = seg->aend = (char*)NULL;

    for ( s=string;  ;  s++ )
    {
	switch (*s)
	{
	case ',':  if ( in_parens ) break;  /* commas in arguments ignored  */
	case NUL:  if ( seg->nend == NULL ) seg->nend = s-1;  /* no argument */
	           seg++;		   /* start the next segment */
    		   seg->nsta = seg->nend = seg->asta = seg->aend = (char*)NULL;
		   break;		   

	case '(':  if ( in_parens++ == 0 ) { seg->nend = s-1; seg->asta = s+1; }
	           break;
		   
	case ')':  if ( --in_parens == 0 ) { seg->aend = s-1; };
		   break;

	default:   if ( *s > ' '  &&  seg->nsta == NULL )
			/* only start a new segment on non-blank char */
	                seg->nsta = s;
	}
	if (*s == NUL) break;
    }

    if (in_parens)
    {
	XtStringConversionWarning (string, "Callback, unbalanced parenthesis");
	return;
    }

/*  -- process individual callback string segments "name(arg)" */
    for( seg = name_arg_segments;  seg->nsta;   seg++)
    {
        char           	  cb_name[MAX_XRMSTRING];
	register char    *d;
	XrmQuark       	  quark;
	int		  found;

	/* our callback cache names are case insensitive, no white space */
	for ( s=seg->nsta, d=cb_name; s<=seg->nend; )
	   if ( *s > ' ')
             *d++ = (isupper(*s) ) ? tolower (*s++) : *s++;
	   else
	      s++;
	*d   = NUL;

        /* try to locate callback in our cache of callbacks */
        quark = XrmStringToQuark (cb_name);
	for (found = 0, i=0 ; !found && i<callbacks_num ; i++)
	    if ( callbacks_ptr[i].quark == quark )
	    {
		rec->callback = callbacks_ptr[i].callback;
		rec->closure  = callbacks_ptr[i].closure;
		found++;
	    }

	/* we have found a registered callback, process arguments */
	if (found)
	{
	   register char *arg;
	   register int   alen;
	   
	   if ( seg->asta )
	   {
	       /* arg in parens - pass as string replacing default closure */
	       alen = (int)seg->aend - (int)seg->asta +1;
	       arg  = XtMalloc(alen+1);
	       strncpy ( arg, seg->asta, alen );
	       arg[alen]    = NUL;
	       rec->closure = (caddr_t)arg;
	   }
	   else
	   {
	       /* no arg in parens.  Make sure closure is something -
	       ** do NOT return NULL in any event.  Causes SEGV too
	       ** easily.  
	       */
	       if (rec->closure == NULL)
		   rec->closure = (caddr_t)"";
	   }
	   rec++;
	   callback_list_len++;
        }
	else
	{
           XtStringConversionWarning (cb_name, 
			"Callback, unknown callback name");
	}
    } /* end for seg loop */

/*  -- terminate the callback list */
    {
        rec->callback = NULL;
	rec->closure  = NULL;
	callback_list_len++;
    }

/*  -- make a permanent copy of the new callback list, and return a pointer */
    cb = (XtCallbackRec*)XtMalloc( callback_list_len * sizeof (XtCallbackRec) );
    memcpy ( (char*)cb, (char*)callback_list,  
              callback_list_len * sizeof (XtCallbackRec));
    toVal->size = sizeof (XtCallbackRec*);
    toVal->addr = (caddr_t)&cb;
}

/*
    -- Convert String To Widget
*******************************************************************************
    This conversion creates a Widget id from the X resource database string.
    The conversion will fail, and WcFullNameToWidget() will issue a warning,
    if the widget so named has not been created when this converter is called.
    For example, if a widget refers to itself for some reason, during
    its creation when this converter is called, it is not yet created: 
    therefore, the converter will fail.
*/

void XmuCvtStringToWidget (args, num_args, fromVal, toVal)
    XrmValue *args;
    Cardinal *num_args;
    XrmValue *fromVal;
    XrmValue *toVal;
{
    toVal->addr = 
	(caddr_t) WcFullNameToWidget( WcRootWidget(NULL), fromVal->addr);
    toVal->size = sizeof(Widget);
}

#ifdef MOTIF
/*
    -- Convert String To MenuWidget
*******************************************************************************
    This conversion converts strings into menu widgets for use on
    cascade button subMenuId resource specifications.
*/

void CvtStringToMenuWidget (args, num_args, fromVal, toVal)
    XrmValue *args;
    Cardinal *num_args;
    XrmValue *fromVal;
    XrmValue *toVal;
{
    char	cleanName[MAX_XRMSTRING];
    Widget	root;
    Widget	widget;

    (void)WcCleanName( fromVal->addr, cleanName );

    if ( NULL == (root = WcRootWidget(NULL)) )
    {
	XtStringConversionWarning (cleanName,
                "MenuWidget - can't find a root widget for WcFullNameToWidget");
	return;
    }

    if (cleanName[0] == '^' || cleanName[0] == '~')
    {
	XtStringConversionWarning (cleanName,
"MenuWidget - Relative paths cannot be converted.  Use path from root widget."
	);
	return;
    }

    widget = WcFullNameToWidget( root, cleanName );

    if ( widget == NULL )
    {
	XtStringConversionWarning (cleanName,
                "MenuWidget - no such widget.  Misspelled? Forgot the path?");
	return;
    }
    else if ( XmIsRowColumn( widget ) 
      && (   RC_Type ( (XmRowColumnWidget)widget ) == XmMENU_PULLDOWN
          || RC_Type ( (XmRowColumnWidget)widget ) == XmMENU_POPUP    ) )
    {
	done ( Widget, widget );
    }
    XtStringConversionWarning (cleanName, 
		"MenuWidget - not XmMENU_PULLDOWN or XmMENU_POPUP.");
}
#endif /* MOTIF */

/*
*******************************************************************************
* Public_function_declarations.
*******************************************************************************
*/

/*
    -- Add String To ... Convertors
*******************************************************************************
*/

void WcAddConverters ( app )
    XtAppContext app;
{
    ONCE_PER_XtAppContext( app );

    XtAddConverter       (XtRString,
                          WcRClassPtr,
                          CvtStringToClassPtr,
                          (XtConvertArgList)NULL,
                          (Cardinal)0);

    XtAddConverter       (XtRString,
                          WcRClassName,
                          CvtStringToClassName,
                          (XtConvertArgList)NULL,
                          (Cardinal)0);

    XtAddConverter       (XtRString,
                          WcRConstructor,
                          CvtStringToConstructor,
                          (XtConvertArgList)NULL,
                          (Cardinal)0);

    XtAddConverter       (XtRString, 
                          XtRCallback,
                          CvtStringToCallback,
                          (XtConvertArgList)NULL,
                          (Cardinal)0);

#ifndef MOTIF
    XtAddConverter       (XtRString,
                          XtRWidget,
                          XmuCvtStringToWidget,
                          (XtConvertArgList)NULL,
                          (Cardinal)0);
#else
    XtAddConverter       (XtRString,
                          WcRWidget,  /* "Window" is wrong, but it works !?! */
                          XmuCvtStringToWidget,
                          (XtConvertArgList)NULL,
                          (Cardinal)0);

    XtAddConverter       (XtRString,
                          XmRMenuWidget,
                          CvtStringToMenuWidget,
                          (XtConvertArgList)NULL,
                          (Cardinal)0);
#endif /* !MOTIF */
}
