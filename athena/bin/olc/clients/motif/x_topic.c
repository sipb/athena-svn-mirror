#include "xolc.h"

TOPIC TopicTable[256];


ERRCODE
x_list_topics(Request, file)
     REQUEST *Request;
     char *file;
{
  int status;
  FILE *infile;
  char inbuf[BUF_SIZE];
  int i = 0;

  status = OListTopics(Request,file);
  switch(status)
    {
    case SUCCESS:
      infile = fopen(file, "r");
      i = 0;
      while (fgets(inbuf, BUF_SIZE, infile) != NULL)
	{
	  inbuf[strlen(inbuf) - 1] = (char) NULL;
	  sscanf(inbuf, "%s", TopicTable[i].topic);
	  i++;
	  AddItemToList(w_list, inbuf);
	}
      fclose(infile);
      break;
      
    case ERROR:
      MuError("Cannot list OLC topics.");
      status = ERROR;
      break;
      
    default:
      status = handle_response(status, Request);
      break;
    }
  return(status);
}
