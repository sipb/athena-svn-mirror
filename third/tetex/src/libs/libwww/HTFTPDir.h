/*

					W3C Sample Code Library libwww FTP DIRECTORY LISTING




!FTP Directory Listings!

*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This module converts a FTP directory listing to a HTML object

This module is implemented by HTFTPDir.c, and it is
a part of the W3C
Sample Code Library.

*/

#ifndef HTFTPDIR_H
#define HTFTPDIR_H

#include "HTStream.h"
#include "HTFTP.h"

extern HTStream * HTFTPDir_new (HTRequest *	request,
				FTPServerType	server,
				char		list);
#endif


/*



@(#) $Id: HTFTPDir.h,v 1.1.1.1 2000-03-10 17:52:57 ghudson Exp $


*/
