#include <olc/olc.h>
#include <olcd.h>

#define NBLOCKS 100

list_knuckle(knuckle,data)
     KNUCKLE *knuckle;
     LIST *data;
{
  return(get_list_info(knuckle,data));
}


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
      log_error("malloc fuckup: list user knuckles");
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



list_queue(queue,data,size)
     int queue;
     LIST **data;
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

  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(!list_redundant((*k_ptr)) && is_active((*k_ptr)))
      {
	get_list_info((*k_ptr),d++);
	n++;
#ifdef TEST
	printf("putting %s %d\n",(*k_ptr)->user->username,n);
	printf("sizes: %d  %d\n",(1024 * NBLOCKS * page), ((n-1) * sizeof(LIST)));
#endif TEST
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





list_topic(topic,data,n)
     char *topic;
     LIST **data;
     int *n;
{
  KNUCKLE **k_ptr;
  LIST *d;
  int page = 1;

  d = (LIST *) malloc(1024 * NBLOCKS);
  if(d == (LIST *) NULL)
    {
      log_error("malloc error: list queue");
      return(ERROR);
    }
  *data = d;
  *n = 0;

  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(has_question((*k_ptr)))
       if(string_equiv(topic,(*k_ptr)->question->topic,strlen(topic)) &&
	  (*k_ptr)->question->owner == (*k_ptr))       
       {
	 get_list_info((*k_ptr),d++);
	 *n++;
	 if((1024 * NBLOCKS * page) <= (*n * sizeof(LIST)) + 1)
	   {
	     d = (LIST *) realloc(d,1024 * NBLOCKS);
	     if(d == (LIST *) NULL)
	       {
		 log_error("realloc error: list queue");
		 return(ERROR);
	       }
	     page++;
	   }
       }

  d->ustatus = END_OF_LIST;
  d->ukstatus = END_OF_LIST;

  return(SUCCESS);
}





list_name(knuckle,name,data,n)
     char *name;
     LIST **data;
     int *n;
{
  KNUCKLE **k_ptr;
  LIST *d;
  int page = 1;

  d = (LIST *) malloc(1024 * NBLOCKS);
  if(d == (LIST *) NULL)
    {
      log_error("malloc error: list queue");
      return(ERROR);
    }
  *data = d;
  *n = 0;

  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(is_active((*k_ptr)))
       if(string_equiv(name,(*k_ptr)->user->username,strlen(name)))
       {
	 get_list_info((*k_ptr),d++);
	 *n++;
	 if((1024 * NBLOCKS * page) <= (*n * sizeof(LIST)) + 1)
	   {
	     d = (LIST *) realloc(d,1024 * NBLOCKS);
	     if(d == (LIST *) NULL)
	       {
		 log_error("realloc error: list queue");
		 return(ERROR);
	       }
	     page++;
	   }
       }

  d->ustatus = END_OF_LIST;
  d->ukstatus = END_OF_LIST;

  return(SUCCESS);
}




