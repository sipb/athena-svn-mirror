/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains procedures for listing knuckles.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/list.c,v $
 *	$Id: list.c,v 1.13 1991-01-03 15:50:40 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/list.c,v 1.13 1991-01-03 15:50:40 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static void get_dlist_info P((D_LIST *item , KNUCKLE *k ));
static void put_queue P((FILE *f , D_LIST *q , int n , char *name ));

#undef P

#define NBLOCKS 100

int
list_knuckle(knuckle,data)
     KNUCKLE *knuckle;
     LIST *data;
{
    get_list_info(knuckle,data); /* doesn't return a value */
    return 0;
}


int
list_user_knuckles(knuckle,data,size)
     KNUCKLE *knuckle;
     LIST **data;
     int *size;
{
  USER *user;
  KNUCKLE **k_ptr;
  LIST *d;
  int n;

  user = knuckle->user;
  
  /* Allocate the maximum amount. The extra k isn't worth getting
   * paranoid about accurracy
   */

  d = (LIST *) malloc(sizeof(LIST) * (user->no_knuckles + 1));
  if(d == (LIST *) NULL)
    {
      log_error("malloc lossage: list user knuckles");
      return(ERROR);
    }
  *data = d;
  n = 0;

  for (k_ptr = user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(is_active((*k_ptr)))
      {
#ifdef TEST
        printf("getting %s %d\n",(*k_ptr)->user->username,n);
#endif /* TEST */

	get_list_info((*k_ptr),d++);
	n++;
      }
  d->ustatus = END_OF_LIST;
  d->ukstatus = END_OF_LIST;
  *size = n;

  return(SUCCESS);
}

list_redundant(knuckle)
     KNUCKLE *knuckle;
{
    return is_connected(knuckle)
	&&  knuckle->question->owner != knuckle;
}



int
list_queue(data,topics,stati,name,size)
     LIST **data;
     int *topics;
     int stati;
     char *name;
     int *size;
{
  KNUCKLE **k_ptr;
  LIST *d;
  int page = 1;
  int n;

  d = (LIST *) malloc(1024 * NBLOCKS);
  if(d == (LIST *) NULL)
    {
      log_error("malloc error: list queue");
      return(ERROR);
    }
  *data = d;
  n = 0;

#ifdef TEST
  printf("list ststu: %d %s\n",stati,name);
#endif /* TEST */

  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(!list_redundant((*k_ptr)) && is_active((*k_ptr)))
      {
	if((stati != 0) && ((*k_ptr)->status != stati) && 
	   ((*k_ptr)->user->status != stati))
	  continue;

	if(name != (char *) NULL)
	  if(*name != '\0')
	    if(!(string_equiv(name,(*k_ptr)->user->username,1)))
	      continue;

	if((topics != (int *) NULL) && (has_question((*k_ptr))))
	  if(!is_topic(topics,(*k_ptr)->question->topic_code))
	    continue;

	get_list_info((*k_ptr),d);
#ifdef TEST
	printf("putting %s %d\n",(*k_ptr)->user->username,n);
	printf("sizes: %d  %d\n",(1024 * NBLOCKS * page), ((n-1) * sizeof(LIST)));
	printf("status: %d title %s\n",d->ukstatus, d->user.title);
        if(d->connected.uid >=0)
	  printf("connect: %s status: %d\n",d->connected.username, d->ckstatus);
	if(d->nseen >= 0)
	  printf("question: %s \n",d->topic);

#endif /* TEST */
	d++;
	n++;
	if((1024 * NBLOCKS * page) <= ((n-1) * sizeof(LIST)))
	  {
	    ++page;
	    d = (LIST *) realloc(d,1024 * NBLOCKS * page);
	    if(d == (LIST *) NULL)
	      {
		log_error("realloc error: list queue");
		return(ERROR);
	      }
	  }
      }

  d->ustatus = END_OF_LIST;
  d->ukstatus = END_OF_LIST;
  *size = n;

#ifdef TEST
  printf("%d elements in list\n",n);
#endif /* TEST */

  return(SUCCESS);
}


void
dump_list()
{
  KNUCKLE **k_ptr;
  static D_LIST *pending_q, *pickup_q, *refer_q;
  static int mx_pending, mx_pickup, mx_refer;
  int n_pending, n_pickup, n_refer;
  FILE *f;

  /* Allocate initial space for queues */
  /* Make some good guesses about sizes- can't hurt to overguess a little */

  if (mx_pending == 0) {
    mx_pending = 50;
    mx_pickup = 50;
    mx_refer = 10;

    pending_q = calloc(mx_pending,sizeof(D_LIST));
    pickup_q = calloc(mx_pickup,sizeof(D_LIST));
    refer_q = calloc(mx_refer,sizeof(D_LIST));
  }

  n_pending = n_pickup = n_refer = 0;

  if ((pending_q == NULL) || (pickup_q == NULL) || (refer_q == NULL)) {
    log_error("dump_list: calloc failed");
    return;
  }

  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(!list_redundant((*k_ptr)) && is_active((*k_ptr)))
      {
	if ((*k_ptr)->status & (PENDING | NOT_SEEN | ACTIVE | SERVICED)) {
	  get_dlist_info(&pending_q[n_pending],*k_ptr);
	  n_pending++;
	  if (n_pending == mx_pending) {
	    mx_pending *= 2;
	    pending_q = realloc(pending_q,mx_pending);
	    if (pending_q == NULL) {
	      log_error("dump_list: realloc failed");
	      return;
	    }
	  }
	}
	else if ((*k_ptr)->status & PICKUP) {
	  get_dlist_info(&pickup_q[n_pickup],*k_ptr);
	  n_pickup++;
	  if (n_pickup == mx_pickup) {
	    mx_pickup *= 2;
	    pickup_q = realloc(pickup_q,mx_pickup);
	    if (pickup_q == NULL) {
	      log_error("dump_list: realloc failed");
	      return;
	    }
	  }
	}
	else {
	  get_dlist_info(&refer_q[n_refer],*k_ptr);
	  n_refer++;
	  if (n_refer == mx_refer) {
	    mx_refer *= 2;
	    refer_q = realloc(refer_q,mx_refer);
	    if (refer_q == NULL) {
	      log_error("dump_list: realloc failed");
	      return;
	    }
	  }
	}
      }

  /* Got lists, now output them... */
  f = fopen(LIST_TMP_NAME,"w+");
  if (f == NULL) {
    log_error("dump_list: unable to open list file");
    return;
  }
  put_queue(f,pending_q,n_pending,"active");
  put_queue(f,pickup_q,n_pickup,"pickup");
  put_queue(f,refer_q,n_refer,"refer");
  fclose(f);
  rename(LIST_TMP_NAME,LIST_FILE_NAME);
  return;
}

static void
  get_dlist_info(item,k)
D_LIST *item;
KNUCKLE *k;
{
  item->username = k->user->username;
  item->machine = k->user->machine;
  item->instance = k->instance;
  item->kstatus = k->status;
  item->ustatus = k->user->status;
  if (is_connected(k)) {
    item->cusername = k->connected->user->username;
    item->cinstance = k->connected->instance;
    item->cstatus = k->connected->status;
  }
  else {
    item->cusername = "";
    item->cinstance = -1;
    item->cstatus = -1;
  }
  if (k->question == NULL) {
    item->n_consult = 0;
    item->topic = "on-duty";
    item->note = "This consultant is on duty";
  }
  else {
    item->n_consult = k->question->nseen;
    item->topic = k->question->topic;
    item->note = k->question->note;
  }
  item->timestamp = k->timestamp;
}

static void
  put_queue(f,q,n,name)
FILE *f;
D_LIST *q;
int n;
char *name;
{
  int i;
  struct tm *t;
  char st[LABEL_SIZE];

  fprintf(f,"%s\n%d\n",name,n);

  for(i=0;i<n;i++) {
    fprintf(f,"%s\n%s\n%d\n", q[i].username, q[i].machine, q[i].instance);
    if (q[i].ustatus & LOGGED_OUT)
      fprintf(f,"logout\n");
    else if (q[i].ustatus & SERVICED) {
      if (q[i].ustatus & PICKUP)
	fprintf(f,"pickup\n");
      else if (q[i].ustatus & REFERRED)
	fprintf(f,"refer\n");
    }
    else {
      OGetStatusString(q[i].kstatus,st);
      if (q[i].ustatus & LOGGED_OUT)
	fprintf(f,"logout\n");
      else
	fprintf(f,"%s\n",st);
    }

    fprintf(f,"%s\n%d\n",q[i].cusername, q[i].cinstance);
    OGetStatusString(q[i].cstatus,st);
    if (q[i].cstatus = OFF)
      fprintf(f,"\n");
    else
      fprintf(f,"%s\n",st);
    
    t = localtime(&q[i].timestamp);

/* Why month + 1?  Because the months are numbered from 0-11, of course.. */

    fprintf(f,"%d\n%s\n%02d/%02d\n%02d%02d\n", q[i].n_consult, q[i].topic,
	    t->tm_mon +1 , t->tm_mday, t->tm_hour, t->tm_min);
    fprintf(f,"%s\n",q[i].note);
  }
}
