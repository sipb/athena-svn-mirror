/*

					W3C Sample Code Library libwww CRLF STRIPPER STREAM





!Network Telnet To Internal Character Text!

*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This is a filter stream suitable for taking text from a socket and
passing it into a stream which expects text in the local C
representation.  It does newline conversion.  As usual, pass its
output stream to it when creating it. 

This module is implemented by HTNetTxt.c, and
it is a part of the  W3C
Sample Code Library.

*/

#ifndef HTNETTXT_H
#define HTNETTXT_H

extern HTStream * HTNetToText (HTStream * target);

#endif

/*



@(#) $Id: HTNetTxt.h,v 1.1.1.1 2000-03-10 17:53:00 ghudson Exp $


*/
