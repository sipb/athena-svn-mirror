/*

  
  					W3C Sample Code Library libwww ANSI C File Streams


!
  ANSI C File Streams
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This module contains a set of basic file writer streams that are used to
dump data objects to disk at various places within the Library core. Most
notably, we use these streams in the Format Manager
in order to handle external presenters, for example post script viewers etc.
These streams can of course also be used in other contexts by the application.

	 
	   o 
	     An ANSI C File Writer Stream
  o 
	     Various Converters using the File Writer Stream

	 
This module is implemented by HTFWrite.c, and it
is a part of the W3C Sample Code
Library.
*/

#ifndef HTFWRITE_H
#define HTFWRITE_H

#include "HTStream.h"
#include "HTFormat.h"

/*
.
  ANSI C File Writer Stream
.

This function puts up a new stream given an open file descripter.
If the file is not to be closed afterwards, then set
leave_open=NO.
*/

extern HTStream * HTFWriter_new	(HTRequest * request,
				 FILE * fp,
				 BOOL leave_open);

/*
.
  Various Converters using the File Writer Stream
.

This is a set of functions that can be registered as converters. They all
use the basic ANSI C file writer stream for writing out to the local file
system.
*/

extern HTConverter HTSaveAndExecute, HTSaveLocally, HTSaveAndCallback;

/*


  
    HTSaveLocally
  
    Saves a file to local disk. This can for example be used to dump date objects
    of unknown media types to local disk. The stream prompts for a file name
    for the temporary file.
  
    HTSaveAndExecute
  
    Creates temporary file, writes to it and then executes system command (maybe
    an external viewer) when EOF has been reached. The stream finds
    a suitable name of the temporary file which preserves the suffix. This way,
    the system command can find out the file type from the name of the temporary
    file name.
  
    HTSaveAndCallback
  
    This stream works exactly like the HTSaveAndExecute stream but
    in addition when EOF has been reached, it checks whether a callback
    function has been associated with the request object in which case, this
    callback is being called. This can be use by the application to do some
    processing after the system command has terminated. The callback
    function is called with the file name of the temporary file as parameter.

*/

#endif

/*

  

  @(#) $Id: HTFWrite.h,v 1.1.1.1 2000-03-10 17:52:57 ghudson Exp $

*/
