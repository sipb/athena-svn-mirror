/* SCCS_data: @(#) Stuff.c 1.1 92/03/18 10:55:05
*/

#include <X11/Intrinsic.h>
#include <X11/Wc/WcCreate.h>
#include <stdio.h>

void PrintCB( w, args, data )
    Widget	w;
    char*	args;
    XtPointer	data;
{
    fprintf( stderr, "PrintCB( %s, %s, (??)%d )\n", XtName(w), args, (int)data);
}

void PrintACT( w, event, params, num_params )
    Widget	w;
    XEvent*	event;
    char**	params;
    Cardinal*	num_params;
{
    int num = *num_params;

    fprintf( stderr, "PrintACT( %s, ?event?, (", XtName(w) );
    if ( num-- )
	fprintf( stderr, "%s", *params++ );
    while ( num-- )
	fprintf( stderr, ", %s", *params++ );
    fprintf( stderr, "), %d )\n", *num_params);
}
