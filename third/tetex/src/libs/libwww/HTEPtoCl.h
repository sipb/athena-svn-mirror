/*

					W3C Sample Code Library libwww callback stream







!EPtoClient: interface between the XParse module and the Client!

*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This module contains the interface between the XParse module and
the client. The dummy function is only here so that clients that use
the XParse module can overwrite it. See also HTXParse

This module is implemented by HTEPtoCl.c,
and it is a part of the 
W3C Sample Code Library.

*/

#ifndef HTEPTOCLIENT_H
#define HTEPTOCLIENT_H

#include "HTStream.h"
#include "HTXParse.h"

extern CallClient HTCallClient;

#endif

/*



@(#) $Id: HTEPtoCl.h,v 1.1.1.1 2000-03-10 17:52:56 ghudson Exp $


*/
