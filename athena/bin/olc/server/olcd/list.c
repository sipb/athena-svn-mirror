#include <olc/olc.h>
#include <olcd.h>

#define NBLOCKS 100

int list_knuckle(knuckle,data)
     KNUCKLE *knuckle;
     LIST *data;
{
    get_list_info(knuckle,data); /* doesn't return a value */
    return 0;
}


int list_user_knuckles(knuckle,data,size)
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
#endif TEST

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

printf("listing for %s %d\n",knuckle->user->username, knuckle->instance);
if((is_connected(knuckle)) && (knuckle->question->owner == knuckle))
printf("true\n");

  if((is_connected(knuckle)) &&  (knuckle->question->owner != knuckle))
    return(TRUE);
  else
    return(FALSE);
}



int list_queue(queue,data,queues,topics,stati,name,size)
     int queue;
     LIST **data;
     int queues;
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
#endif TEST

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

#endif TEST
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
#endif TEST

  return(SUCCESS);
}





