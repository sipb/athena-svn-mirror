/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the functions for reading backup files
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/new_backup.c,v $
 *	$Id: new_backup.c,v 1.1 1992-01-07 15:53:32 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/new_backup.c,v 1.1 1992-01-07 15:53:32 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>

void
load_data_new()
{
  FILE *f;
  char errbuf[BUFSIZ];
  char line[BUFSIZ];
  char queue_name[128];
  int queue,question;
  int n_queues;
  int n_questions;
  char username[LOGIN_SIZE];
  char machine[NAME_SIZE];
  


  load_status("loading data...");
  
  /* Be really paranoid about errors-
     (my 6.170 TA would be proud...)
   */

  if ((f = fopen(LIST_FILE_NAME,"r")) == NULL) {
    sprintf(errbuf,"Unable to load queue list file %s: %%m", LIST_FILE_NAME);
    log_error(errbuf);
    return;
  }

  /* Read in number of queues */
  if (fgets(line,BUFSIZ,f) == NULL) {
    sprintf(errbuf,"Error reading number of queues from %s", LIST_FILE_NAME);
    log_error(errbuf);
    fclose(f);
    return;
  }
  n_queues = atoi(line);
  if (n_queues < 1) {
    log_error("Invalid number of queues in queue list file");
    fclose(f);
    return;
  }

  /* loop for each queue */
  for(queue=0;queue<n_queues;queue++) {
    if (fgets(queue_name,128,f) == NULL) {
      sprintf(errbuf,"Error reading name of queue %d", queue);
      log_error(errbuf);
      fclose(f);
      return;
    }

    /* Read in number of questions in queue */
    if (fgets(line,BUFSIZ,f) == NULL) {
      sprintf(errbuf,"Error reading number of questions in queue %s",
	      queue_name);
      log_error(errbuf);
      fclose(f);
      return;
    }

    n_questions = atoi(line);
    if (n_questions < 1) {
      sprintf(errbuf,"Invalid number of questions (%d) in queue %s",
	      n_questions, queue_name);
      log_error(errbuf);
      fclose(f);
      return;
    }

    /* loop for each question */
    for(question=0;question<n_questions;question++) {
      /* Format of question is:
	 - username of asker
	 - machine question asked from
	 - instance of asker
	 - Is the asker logged in? (+ = yes, ' '= no)
	 - status of question (done, unseen, etc. - these will change)
	 - username of connected consultant (blank if none connected)
	 - instance of connected consultant (-1 if none connected)
	 - Level the consultant is signed 'on'
	    (off if they are not signed on, or
	    'unknown' if there is no consultant connected)
	 - number of consultants that have been connected so far
	 - topic of question
	 - date asked
	 - time asked (hhmm, in 24 hour time)
	 - description
       */

      /* get username */
      if (fgets(username,LOGIN_SIZE,f) == NULL) {
	sprintf(errbuf,"Error reading username of question %d in queue %s",
		question,queue_name);
	log_error(errbuf);
	fclose(f);
	return;
      }
    }
  }
}
      

    
