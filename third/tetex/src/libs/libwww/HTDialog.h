/*

  					W3C Sample Code Library libwww messages and dialogs


!
  Application side Error Messages And the like
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This module provides some "make life easier" functions in order to get the
application going. The functionality of this module was originally in
HTAccess, but now It is part of the
application interface which the application
may use it if desired.

This module is implemented by HTHome.c, and it is
a part of the  W3C Sample Code
Library.
*/

#ifndef HTMSG_H
#define HTMSG_H
#include "WWWLib.h"

/*
.
  English Error Messages and Progress Notifications
.

This function provides an unformatted English string containing the possibly
nested status message that explains the result of a request. This is essentially
"flattening out" the information provided in the request
error strack. The string must be freed by the caller.
*/

extern char * HTDialog_errorMessage (HTRequest * request, HTAlertOpcode op,
			             int msgnum, const char * dfault,
				     void * input);

/*

This function provides a string containin an English progress message that
the application can present to the user if desired. The string must be freed
by the caller.
*/

extern char * HTDialog_progressMessage (HTRequest * request, HTAlertOpcode op,
			                int msgnum, const char * dfault,
				        void * input);

/*
.
  Default User Interaction Handlers
.

You can register a set of callback functions to handle user prompting, error
messages, confimations etc. Here we give a set of functions that can be used
on almost anu thinkable platform. If you want to provide your own platform
dependent implementation then fine :-)
(
  Display a message, then wait for 'YES' or 'NO'
)

This function prompts the user for a confirmation on the message passed as
a parameter. If the user reacts in the affirmative, returns TRUE,
returns FALSE otherwise.
*/

extern HTAlertCallback HTConfirm;

/*
(
  Prompt the User a Question
)

Prompt for answer and get text back. Reply text is either NULL on error or
a dynamic string which the caller must free.
*/
		
extern HTAlertCallback HTPrompt;

/*
(
  Prompt for a Password
)

Prompt for password without echoing the reply. Reply text is weither NULL
on error or a dynamic string which the caller must free.

NOTE: The current version uses getpass which on many systems
returns a string of 8 or 16 bytes.
*/

extern HTAlertCallback HTPromptPassword;

/*
(
  Prompt for a UserID and a Password
)

This is just a composite function using HTPrompt and
HTPromptPassword. The strings returned must be freed by caller.
*/

extern HTAlertCallback HTPromptUsernameAndPassword;

/*
(
  Display a Message
)

This function simply puts out the message passed.
*/

extern HTAlertCallback HTAlert;

/*
(
  Progress Notification
)

This function can be used to indicate the current status of a certain action.
In order to avoid having strings directly in the core parts of the Library,
this function is passed a "state" argument from which the message can be
generated in this module. The "param" argument is for additional information
to be passed.
*/

extern HTAlertCallback HTProgress;

/*
(
  Generating a User Error Message of a request
)

This function outputs the content of the error list to standard output (used
in Line Mode Browser), but smart clients and servers might overwrite this
function so that the error messages can be handled to the user in a nice(r)
way. That is the reason for putting the actual implementation in
HTAlert.c.

*/

extern HTAlertCallback HTError_print;

/*
(
  Generating a Server Error Message of a request
)

Default function that creates an error message using HTAlert() to put out
the contents of the error_stack messages. Furthermore, the error_info structure
contains a name of a help file that might be put up as a link. This file
can then be multi-linguistic.
*/

extern BOOL HTError_response (HTRequest * request, HTAlertOpcode op,
			      int msgnum, const char * dfault, void * input,
			      HTAlertPar * reply);

/*
*/

#endif /* HTHOME_H */

/*

  

  @(#) $Id: HTDialog.h,v 1.1.1.1 2000-03-10 17:52:56 ghudson Exp $

*/
