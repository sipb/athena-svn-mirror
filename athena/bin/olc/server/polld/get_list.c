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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/get_list.c,v $
 *	$Id: get_list.c,v 1.1 1991-01-08 16:50:59 lwvanels Exp $
 *	$Author: lwvanels $
 */


#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/get_list.c,v 1.1 1991-01-08 16:50:59 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <polld.h>
#include <olcd.h>

int
get_user_list(users,max_people)
     PTF *users;
     int *max_people;
{
  FILE *f;
  char junk[BUF_SIZE], *p;
  int i,n,queues;
  int n_people = 0;

  f = fopen(LIST_FILE_NAME,"r");
  if (f == NULL) {
    syslog(LOG_ERR,"Could not open queue list file");
    return(-1);
  }
  for (queues = 0;queues < 3;queues++) {
    fgets(junk,BUF_SIZE,f); /* queue name */
    fscanf(f,"%d",&n); /* Number of entries in this queue */
    for(i=0;i<n;i++) {
      fscanf(f,"%s %s\n",users[n_people].username, users[n_people].machine);
      fgets(junk,BUF_SIZE,f); /* instance */

      fgets(junk,BUF_SIZE,f); /* status */
      p = index(junk,'\n');
      if (p != NULL) *p = '\0';
      if (strcmp(junk,"logout")== 0) {
	users[n_people].status = LOGGED_OUT;
      }
      else if (strcmp(junk,"mach down") == 0) {
	users[n_people].status = MACHINE_DOWN;
      }
      else
	users[n_people].status = ACTIVE;

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
  return (n_people);
}
