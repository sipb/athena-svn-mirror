/*

					W3C Sample Code Library libwww NNTP REQUEST STREAM




!NNTP Request Stream!

*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

The NNTP Request stream generates a NNTP request header and writes it
to the target which is normally a HTWriter stream.

This module is implemented by HTNewsRq.c, and
it is a part of the W3C
Sample Code Library.

*/

#ifndef HTNEWSREQ_H
#define HTNEWSREQ_H

#include "HTStream.h"
#include "HTAccess.h"

/*

(Streams Definition)

This stream makes a HTNews request header before it goes into
transparent mode.

*/

extern HTStream * HTNewsPost_new (HTRequest * request, HTStream * target);

#endif

/*



@(#) $Id: HTNewsRq.h,v 1.1.1.1 2000-03-10 17:53:00 ghudson Exp $


*/
