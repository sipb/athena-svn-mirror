/*
 *	$Id: c_status.c,v 1.5 1999-03-06 16:48:16 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: c_status.c,v 1.5 1999-03-06 16:48:16 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>

STATUS Status_Table[] = 
{
  {OFF,           "off"},
  {ON,            "on"},
  {FIRST,         "sp1"},
  {DUTY,          "duty"},
  {SECOND,        "sp2"},
  {URGENT,        "urgent"},
  {BUSY,          "busy"},
  {CACHED,        "cached"},
  {PENDING,       "pending"},
  {NOT_SEEN,      "unseen"},
  {DONE,          "done"},
  {CANCEL,        "cancel"},
  {PICKUP,        "pickup"},
  {REFERRED,      "refer"},
  {LOGGED_OUT,    "logout"},
  {MACHINE_DOWN,  "mach down"},
  {ACTIVE,        "active"},
  {SERVICED,      "active"},
  {UNKNOWN_STATUS,"unknown"},
};

ERRCODE
OGetStatusString(status,string)
     int status;
     char *string;
{
  int ind = 0;
  
  while  ((status != Status_Table[ind].status)
          && (Status_Table[ind].status != UNKNOWN_STATUS)) 
    ind++;
    
  strcpy(string,Status_Table[ind].label);
  return 0;
}

ERRCODE
OGetStatusCode(string,status)
     char *string;
     int *status;
{
  int ind;

  *status = -2;

  for (ind = 0; Status_Table[ind].status != UNKNOWN_STATUS; ind++)
    {
      if (string_equiv(string, Status_Table[ind].label,
		       strlen(string)))
	if (*status == -2)
	  *status = Status_Table[ind].status;
    }

  if ((*status == UNKNOWN_STATUS) || (*status == -2))
    *status = -1;
  return 0;
}

