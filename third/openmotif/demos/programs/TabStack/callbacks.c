/*
 * README: This file is appended to at file generation time.
 * Edits can be made throughout the file
 */
/*
 * Generated by the ICS Builder Xcessory (BX).
 *
 *
 * Builder Xcessory 3.5.1.
 *
 */
#include <Xm/Xm.h>


/*
 * Standard includes for builtins.
 */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
 * Macros to make code look nicer between ANSI and K&R.
 */
#ifndef ARGLIST
#if (NeedFunctionPrototypes == 0)
#define PROTOTYPE(p)	()
#define ARGLIST(p)	p
#define ARG(a, b)	a b;
#define GRA(a, b)	a b;
#define UARG(a, b)      a b;
#define GRAU(a, b)      a b;
#else
#define PROTOTYPE(p)	p
#define ARGLIST(p)	(
#define ARG(a, b)	a b,
#define GRA(a, b)	a b)
#ifdef __cplusplus
#define UARG(a, b)      a,
#define GRAU(a, b)      a)
#else
#define UARG(a, b)      a b,
#define GRAU(a, b)      a b)
#endif
#endif
#endif

Widget		BxFindTopShell PROTOTYPE((Widget));
WidgetList	BxWidgetIdsFromNames PROTOTYPE((Widget, char*, char*));


/*      Function Name:	BxExitCB
 *
 *      Description:   	This functions expects an integer to be passed in
 *		       	client data.  It calls the exit() system call with
 *			the integer value as the argument to the function.
 *
 *      Arguments:      Widget		w: 	the activating widget.
 *			XtPointer	client:	the integer exit value.
 *			XtPointer	call:	the call data (unused).
 */

/* ARGSUSED */
void
BxExitCB ARGLIST((w, client, call))
UARG( Widget, w)
ARG( XtPointer, client)
GRAU( XtPointer, call)
{
    int		exitValue = (int)client;
    
    exit(exitValue);
}

/*      Function Name:	BxSetValuesCB
 *
 *      Description:   	This function accepts a string of the form:
 *			"widgetName.resourceName = value\n..."
 *			It then attempts to convert the widget name to a widget
 *			ID and the value to a valid resource value.  It then
 *			sets the value on the given widget.
 *
 *      Arguments:      Widget		w:	the activating widget.
 *			XtPointer	client:	the set values string.
 *			XtPointer	call:	the call data (unused).
 *
 *      Notes:        * This function expects that there is an application
 *                      shell from which all other widgets are descended.
 */
#include <X11/StringDefs.h>

/* ARGSUSED */
void
BxSetValuesCB ARGLIST((w, client, call))
ARG( Widget, w)
ARG( XtPointer, client)
GRAU( XtPointer, call)
{
#define CHUNK	512

    Boolean 	first = True;
    String 	rscs = XtNewString((String)client);
    String 	*valueList = (String *)XtCalloc(CHUNK, sizeof(String));
    char 	*start;
    char 	*ptr, *cptr;
    String 	name;
    String 	rsc;
    int 	i, count = 0;
    Widget 	*current;

    for ( start = rscs; rscs && *rscs; rscs = strtok(NULL, "\n"))
    {
        if ( first )
        {
            rscs = strtok(rscs, "\n");
            first = False;
        }
        valueList[count] = XtNewString(rscs);
        count++;
        if ( count == CHUNK )
        {
            valueList = 
		(String *)XtRealloc((char *)valueList,
				    (count + CHUNK) * sizeof(String));
        }
    }
    XtFree((char *)start);

    for ( i = 0; i < count; i++ )
    {
	/*
	 * First, extract the widget name and generate a string to 
	 * pass to BxWidgetIdsFromNames().
	 */
	cptr = strrchr(valueList[i], '.');
	if ( cptr != NULL )
	{
	    *cptr = '\000';
	}
	else
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    continue;
	}
	name = valueList[i];
	while( (name && *name) && isspace(*name) )
	{
	    name++;
	}
	ptr = name + strlen(name) - 1;
	while( ptr && *ptr )
	{
            if ( isspace(*ptr) )
            {
                ptr--;
            }
            else
            {
                ptr++;
                break;
            }
        }
        if ( ptr && *ptr )
        {
            *ptr = '\0';
        }
	if ( ptr == NULL )
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    return;
	}
	
	/*
	 * Next, get the resource name to set.
	 */
	rsc = ++cptr;
	cptr = strchr(rsc, '=');
	if ( cptr != NULL )
	{
	    *cptr = '\000';
	}
	else
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    continue;
	}
        while( (rsc && *rsc) && isspace(*rsc) )
        {
            rsc++;
        }
	
        ptr = rsc + strlen(rsc) - 1;
        while( ptr && *ptr )
        {
            if ( isspace(*ptr) )
            {
                ptr--;
            }
            else
            {
                ptr++;
                break;
            }
        }
        if ( ptr && *ptr )
        {
            *ptr = '\0';
        }
	
	/*
	 * Lastly, get the string value to which to set the resource.
	 */
	start = ++cptr;
        while( (start && *start) && isspace(*start) )
        {
            start++;
        }

	if ( start == NULL )
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    return;
	}
	
        ptr = start + strlen(start) - 1;
        while( ptr && *ptr )
        {
            if ( isspace(*ptr) )
            {
                ptr--;
            }
            else
            {
                ptr++;
                break;
            }
        }
        if ( ptr && *ptr )
        {
            *ptr = '\0';
        }
	
	/*
	 * Now convert the widget name to a Widget ID
	 */
	current = BxWidgetIdsFromNames(w, "BxSetValuesCB", name);
        if ( current[0] == NULL )
        {
	    XtFree((char *)(valueList[i]));
	    continue;
        }
	
	/*
	 * If the widget name conversion succeeded, we now need to get the
	 * resource list for the widget so that we can do a resource conversion
	 * of the value.
	 */
	XtVaSetValues(*current, XtVaTypedArg, rsc, XtRString, start,
		      strlen(start)+1, NULL);
        XtFree((char *)(valueList[i]));
    }
    XtFree((char *)valueList);

#undef CHUNK
}


