/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/c_status.c,v $
 *	$Id: c_status.c,v 1.1 1990-12-12 13:59:01 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/c_status.c,v 1.1 1990-12-12 13:59:01 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
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

OGetStatusString(status,string)
     int status;
     char *string;
{
  int index = 0;
  
  while  ((status != Status_Table[index].status)
          && (Status_Table[index].status != UNKNOWN_STATUS)) 
    index++;
    
  strcpy(string,Status_Table[index].label);
}

OGetStatusCode(string,status)
     char *string;
     int *status;
{
  int index;

  *status = -2;

  for (index = 0; Status_Table[index].status != UNKNOWN_STATUS; index++)
    {
      if (string_equiv(string, Status_Table[index].label,
		       strlen(string)))
	if (*status == -2)
	  *status = Status_Table[index].status;
    }

  if ((*status == UNKNOWN_STATUS) || (*status == -2))
    *status = -1;
}

