/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: sortorder.c
 * PURPOSE: read and write the spool queue control file
 **************************************************************************/

static char *const _id =
"sortorder.c,v 3.22 1998/03/29 18:32:50 papowell Exp";

#include "lp.h"
#include "sortorder.h"
/**** ENDINCLUDE ****/

#define mval(field,val) \
	plp_snprintf(s, 2*sizeof(field)+2, "|%0*x", 2*sizeof(field), val ); s += strlen(s);

char *make_cmp_str( struct control_file *lcf )
{
	static char buffer[ 2*sizeof( struct cmp_struct ) ];
	struct cmp_struct *lc = 0;
	char *s = buffer;

	int pl = - lcf->priority;
	if( Ignore_requested_user_priority ) pl = 0;

	mval(lc->done_time, lcf->hold_info.done_time );
	mval(lc->error, lcf->error[0] != 0 );
	mval(lc->redirect, lcf->hold_info.redirect[0] == 0 );
	mval(lc->flags, lcf->flags );
	mval(lc->remove_time, lcf->hold_info.remove_time );
	mval(lc->held_class, lcf->hold_info.held_class );
	mval(lc->hold_time, lcf->hold_info.hold_time );
	mval(lc->priority_time, -(lcf->hold_info.priority_time +1 ));
	mval(lc->priority, pl );
	mval(lc->ctime, lcf->statb.st_ctime );
	mval(lc->number, pl );
	return( buffer );
}
