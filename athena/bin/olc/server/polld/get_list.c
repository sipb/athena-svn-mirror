/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the fuctions to communcate with the main daemon from polld
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: get_list.c,v 1.8 1999-03-06 16:49:08 ghudson Exp $
 */


#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: get_list.c,v 1.8 1999-03-06 16:49:08 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <polld.h>
#include <olcd.h>

int
get_user_list(users,max_people)
     PTF *users;
     int *max_people;
{
  FILE *f;
  char junk[BUF_SIZE];
  int i,n,queues;
  int n_people = 0;
  int n_queues;

  f = fopen(LIST_FILE_NAME,"r");
  if (f == NULL) {
    syslog(LOG_ERR,"Could not open queue list file %s: %m",LIST_FILE_NAME);
    return(-1);
  }
  fscanf(f,"%d\n",&n_queues);
  for (queues = 0;queues < n_queues;queues++) {
    fgets(junk,BUF_SIZE,f); /* queue name */
    fscanf(f,"%d",&n); /* Number of entries in this queue */
    for(i=0;i<n;i++) {
      fscanf(f,"%s %s\n",users[n_people].username, users[n_people].machine);
      fgets(junk,BUF_SIZE,f); /* instance */

      fgets(junk,BUF_SIZE,f); /* login status */
      if (junk[0] == '+')
	users[n_people].status = ACTIVE;
      else
	users[n_people].status = LOGGED_OUT;

      fgets(junk,BUF_SIZE,f); /* knuckle status */
      fgets(junk,BUF_SIZE,f); /* connect consultant */
      fgets(junk,BUF_SIZE,f); /* consultant instance */
      fgets(junk,BUF_SIZE,f); /* consultant status */
      fgets(junk,BUF_SIZE,f); /* n consultants */
      fgets(junk,BUF_SIZE,f); /* topic */
      fgets(junk,BUF_SIZE,f); /* date */
      fgets(junk,BUF_SIZE,f); /* time */
      fgets(junk,BUF_SIZE,f); /* description */
      n_people++;
      if (n_people > *max_people) {
	*max_people *= 2;
	users = (PTF *) realloc(users,*max_people);
	if (users == NULL) {
	  syslog(LOG_ERR,"error in realloc");
	  exit(1);
	}
      }
    }
  }
  fclose(f);
  return (n_people);
}