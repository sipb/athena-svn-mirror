/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for manipulating OLC data structures.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/data_utils.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/data_utils.c,v 1.5 1989-12-22 16:20:27 vanharen Exp $";
#endif


#include <olc/olc.h>
#include <olcd.h>

#include <ctype.h>
#include <sys/types.h>    
#include <sys/time.h>

extern PROC         Proc_List[];

/* Contents:
 *
 *           create_user()
 *           create_knuckle() 
 *           insert_knuckle()
 *           insert_knuckle_in_user()
 *           insert_topic()
 *           delete_user()
 *           delete_knuckle()
 *           init_user()
 *           init_question)
 *           get_knuckle()
 *           find_knuckle()
 *           assign_instance()
 *           connect_knuckles()
 *           match_maker()
 *           new_message()
 *           get_status_info()
 *           verify_topic()
 *           is_specialty()
 */



/*
 * Function:    create_user() 
 * Arguments:   REQUEST *request:	The request from olc.
 * Returns:     A pointer to the first knuckle structure, or NULL if an error
 *              occurs
 * Notes:
 *     First, allocate memory space for a new user structure and zero it
 *     out.  Then create a knuckle which inserts the knuckle into the user
 *     structure and into the Knuckle List. 
 *     Next order of business is to load info from the database, like
 *     acls, specialties and such. 
 */

KNUCKLE *
create_user(person)
     PERSON *person;
{
  KNUCKLE *knuckle;           /* Ptr. to new user struct. */
  USER *user;                 /* Ptr. to another new struct */

  /*
   * create user
   */

  if((user = (USER *) malloc(sizeof(USER))) == (USER *) NULL)
    {
      log_error("create_user: out of memory");
      return((KNUCKLE *) NULL);
    }
  bzero((char *) user, sizeof(USER));

  user->knuckles = (KNUCKLE **) NULL;

  /*
   * create knuckle
   */

  if ((knuckle = create_knuckle(user)) == (KNUCKLE *) NULL)
    {
      log_error("add_user: could not create knuckle");
      return((KNUCKLE *) NULL);
    }



  /*
   * allocate and get db info
   */

  init_user(knuckle,person);

#ifdef TEST
  printf("create_user: %s (%d) \n",
	 knuckle->user->username, knuckle->instance);
#endif TEST


  return(knuckle);
}






/*
 * Function:    create_knuckle() 
 * Arguments:   USER *user   
 * Returns:     A pointer to the newknuckle structure, or NULL if an error
 *              occurs
 * Notes:
 *     First, allocate memory space for a new knuckle structure and zero it
 *     out.  Hook up the knuckle with the user and assign it an instance id.
 *     Initialize the connected and question pointers.
 */

KNUCKLE *
create_knuckle(user)
     USER *user;
{
  KNUCKLE *k, **k_ptr;

  /*
   * first let's see if we already have one
   */

  if(user->knuckles != (KNUCKLE **) NULL)
    for (k_ptr = user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
      if(!is_active((*k_ptr)))
	{
	  (*k_ptr)->instance = validate_instance((*k_ptr));
	  return((*k_ptr));
	}

  /*
   * Make room for daddy
   */

  k = (KNUCKLE *) malloc(sizeof(KNUCKLE));
  if(k == (KNUCKLE *) NULL)
    {
      log_error("create_knuckle: out of memory");
      return((KNUCKLE *) NULL);
    }  
  bzero((char *) k, sizeof(KNUCKLE));

  /*
   * insert knuckle in Knuckle List
   */

  if (insert_knuckle(k) != SUCCESS) 
    {
      free((char *) k);
      return((KNUCKLE *) NULL);
    }

  /*
   * initialize knuckle and insert in User Knuckle List
   */

  k->user = user; 
  k->instance = assign_instance(user);

  if(insert_knuckle_in_user(k,user) != SUCCESS)
    {
      free((char *) k);
      return((KNUCKLE *) NULL);
    }

  k->user->no_knuckles++;
  k->question = (QUESTION *) NULL;
  k->connected = (KNUCKLE *)  NULL;
  k->status = 0;
  strcpy(k->title,k->user->title2);

#ifdef TEST
  printf("create_knuckle: %s (%d)\n",user->username, k->instance);
#endif TEST

  return(k);
}
  



/*
 * Function:    insert_knuckle() 
 * Arguments:   KNUCKLE *knuckle:	Knuckle to be inserted.
 * Returns:     Zero on success, non-zero otherwise.
 * Notes:       
 *          Inserts a knuckle to the Knuckle List. 
 */

insert_knuckle(knuckle)
     KNUCKLE *knuckle;
{
  KNUCKLE **k_ptr;
  int n_knuckles=0;
  int n_inactive=0;
  static struct timeval time;
  char mesg[BUF_SIZE];

  /*
   * Start off knuckle list
   */
  
  if (Knuckle_List == (KNUCKLE **) NULL) 
    {
      Knuckle_List = (KNUCKLE **) malloc(sizeof(KNUCKLE *));
      if (Knuckle_List == (KNUCKLE **) NULL) 
	{
	  perror("malloc(insert_knuckle)");
	  return(ERROR);
	}
      *Knuckle_List = (KNUCKLE *) NULL;
    }
  

  /* 
   * How many do we have, really
   */


  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    {
      ++n_knuckles;
      if(!is_active((*k_ptr)))
	++n_inactive;
    }

  sprintf(mesg,"no: %d   inactive: %d\n",n_knuckles, n_inactive);
  log_status(mesg);

  if(n_inactive < MAX_CACHE_SIZE)
    {
      n_knuckles++;
      
      /*
       * reallocate Knuckle List and add new knuckle
       */
      
      Knuckle_List = (KNUCKLE **) realloc((char *) Knuckle_List, (unsigned)
					  (n_knuckles+1) * sizeof(KNUCKLE *));
      Knuckle_List[n_knuckles]   = (KNUCKLE *) NULL;
      Knuckle_List[n_knuckles-1] = knuckle;
      return(SUCCESS);
    }
  else
    {
      for(k_ptr = Knuckle_List;*k_ptr != (KNUCKLE *) NULL; k_ptr++)
	{
	  if(!is_active((*k_ptr)))
	    {
	      if(((*k_ptr)->user->no_knuckles > 1) && 
		 ((*k_ptr)->instance == 0))
		continue;
	      delete_knuckle((*k_ptr),1);
	      Knuckle_List[n_knuckles-1] = knuckle;
	      Knuckle_List[n_knuckles]   = (KNUCKLE *) NULL;
	      return(SUCCESS);
	    }
	} 
    }
  return(SUCCESS);
}



/*
 * Function:    insert_knuckle_in_user() 
 * Arguments:   KNUCKLE *knuckle:	Knuckle to be inserted.
 * Returns:     Zero on success, non-zero otherwise.
 * Notes:       
 *          Inserts a knuckle to the User list. 
 */

insert_knuckle_in_user(knuckle, user)
     KNUCKLE *knuckle;
     USER *user;
{
  int n_knuckles;

  /*
   * Allocate first knuckle, if needed
   */

  if (user->knuckles == (KNUCKLE **) NULL) 
    {
      user->knuckles = (KNUCKLE **) malloc(sizeof(KNUCKLE *));
      if (user->knuckles == (KNUCKLE **) NULL) 
	{
	  perror("malloc(insert_knuckle)");
	  return(ERROR);
	}
      *(user->knuckles) = (KNUCKLE *) NULL;
    }

  /*
   * count knuckles
   */

  for (n_knuckles = 0; user->knuckles[n_knuckles] != 
       (KNUCKLE *) NULL; n_knuckles++);
    
  n_knuckles++;

  /*
   * reallocate user kncukle list and insert knuckle
   */

  user->knuckles = (KNUCKLE **) realloc((char *) user->knuckles, (unsigned)
				      ((n_knuckles+1) * sizeof(KNUCKLE *)));
  user->knuckles[n_knuckles]   = (KNUCKLE *) NULL;
  user->knuckles[n_knuckles-1] = knuckle;
  return(SUCCESS);
}




/*
 * Function:    insert_topic()
 * Arguments:   TOPIC *t   Topic structure
 * Returns:     Zero on success, non-zero otherwise.
 * Notes:       
 *          Inserts a Topic into the Topic List. The Topic List is useful
 *          to cache topic information so we donb't have to keep scanning
 *          the database. The List manipulation is the same.
 */

insert_topic(t)
     TOPIC *t;
{
  int n_topics;
  
  /*
   * Allocate first Topic
   */

  if (Topic_List == (TOPIC **) NULL) 
    {
      Topic_List = (TOPIC **) malloc(sizeof(TOPIC *));
      if (Topic_List == (TOPIC **) NULL) 
	{
	  perror("malloc(insert_topic)");
	  return(1); /* I'll get back to this */
	}
      *Topic_List = (TOPIC *) NULL;
    }
  
  /*
   * count topics
   */

  for (n_topics = 0; Topic_List[n_topics] != 
       (TOPIC *) NULL; n_topics++);
    
  n_topics++;

  /*
   * reallocate Topic List and insert new topic
   */

  Topic_List = (TOPIC **) realloc((char *) Topic_List, (unsigned)
				      ((n_topics+1) * sizeof(TOPIC *)));
  Topic_List[n_topics]   = (TOPIC *) NULL;
  Topic_List[n_topics-1] = t;
  return(SUCCESS);
}

    

/*
 * Function:	delete_user() 
 * Arguments:	USER *user:	Ptr. to user structure to be deleted.
 * Returns:	Nothing.
 * Notes:
 *        Delete every knuckle in the user list. Delete_knuckle() will
 *        take care of the user structure.   
 */

void
delete_user(user)
     USER *user;
{
  KNUCKLE *k;
  int i;
  
  k = *(user->knuckles);

  for(i=0; i< user->no_knuckles; i++)
    delete_knuckle((k+i),0);
}


/*
 * Function:	delete_knuckle() removes a knuckle from the Knuckle List.
 * Arguments:	KNUCKLE *knuckle:      Ptr. to user structure to be deleted.
 * Returns:	Nothing.
 * Notes:
 *      Find the entry that matches the Knuckle, and copy the last entry
 *      in the list into that slot.  (This is a no-op if the user is at
 *      the end of the list.)  Then put a NULL into the last slot, and
 *      free the KNUCKLE structure. The user is deleted if it is the last
 *      knuckle in the User List.
 */

void
delete_knuckle(knuckle,cont)
     KNUCKLE *knuckle;
     int cont;
{
  int n_knuckles, knuckle_idx;
  char msgbuf[BUFSIZ];
  KNUCKLE **k_ptr;
  int i;

  for (n_knuckles=0; Knuckle_List[n_knuckles] != (KNUCKLE *)NULL; n_knuckles++)
    if (Knuckle_List[n_knuckles] == knuckle)
      knuckle_idx = n_knuckles;

  Knuckle_List[knuckle_idx]  = Knuckle_List[n_knuckles-1];
  Knuckle_List[n_knuckles-1] = (KNUCKLE *) NULL;

  if(!cont)
    Knuckle_List = (KNUCKLE **) realloc(Knuckle_List, 
					sizeof(KNUCKLE *) * (n_knuckles-1));
      

  /* maintain continuity in the user knuckle list */
  k_ptr = knuckle->user->knuckles;
  for(i=0;i<knuckle->user->no_knuckles;i++)
    if(knuckle == *(k_ptr+i))
      break;
  
  *(k_ptr+i) = *(k_ptr+(knuckle->user->no_knuckles-1));
  *(k_ptr+(knuckle->user->no_knuckles-1)) = (KNUCKLE *) NULL;
  

  /* delete user if last knuckle */
  if(knuckle->user->no_knuckles == 1)
    free((char *) knuckle->user);
  else
    knuckle->user->no_knuckles -= 1;

  /* free question */
  if(knuckle->question != (QUESTION *) NULL)
    free((char *) knuckle->question);
      
  /* free new messages */
  if(knuckle->new_messages !=  (char *) NULL)
    free((char *) knuckle->new_messages);
      
  /* log it */
  (void) sprintf(msgbuf, "Deleting knuckle %s (%d)", 
	  knuckle->user->username, knuckle->instance);
  log_status(msgbuf);

  /* free it */
  free((char *)knuckle);
}


deactivate_knuckle(knuckle)
     KNUCKLE *knuckle;
{
  if(knuckle->instance > 0)
    delete_knuckle(knuckle);
  else
    knuckle->status = 0;
  
  return(SUCCESS);
}


void
init_user(knuckle,person)
     KNUCKLE *knuckle;
     PERSON *person;
{
  (void) strncpy(knuckle->user->realname,person->realname, NAME_LENGTH);
  (void) strncpy(knuckle->user->username,person->username, LOGIN_SIZE);
  (void) strncpy(knuckle->user->machine,person->machine,NAME_LENGTH);
  (void) strncpy(knuckle->user->realm,person->realm,REALM_SZ);
  knuckle->status = 0;
  init_dbinfo(knuckle->user);
  (void) strcpy(knuckle->title,knuckle->user->title1);
}


init_dbinfo(user)
    USER *user;
{
  (void) strcpy(user->title1, DEFAULT_TITLE);
  (void) strcpy(user->title2, DEFAULT_TITLE2);
  user->max_ask = 1;
  user->max_answer = 2;
  user->permissions = 0;
  user->specialties[0] = UNKNOWN_TOPIC;
  load_user(user);
}


init_question(k,topic,text)
     KNUCKLE *k;
     char *topic;
     char *text;
{
  struct timeval tp, tp1;
  struct timezone tz;

  k->question = (QUESTION *) malloc(sizeof(QUESTION));
  if(k->question == (QUESTION *) NULL)
    {
      perror("init_question");
      return(ERROR);
    }

  gettimeofday( &tp1, &tz );

  k->timestamp = tp1.tv_sec;
  k->question->owner = k;
  k->queue = ACTIVE_Q;
  k->question->nseen = 0;
  k->question->seen[0] = -1;
  k->question->note[0] = '\0';
  k->question->comment[0] = '\0';
  k->question->topic_code = verify_topic(topic);
  (void) strcpy(k->title,k->user->title1);
  (void) strcpy(k->question->topic,topic);
  init_log(k,text);
  if(k->new_messages != (char *) NULL)
    {
      free(k->new_messages);
      k->new_messages = (char *) NULL;
    }
  return(SUCCESS);
}


int
get_user(person,user)
     PERSON *person;
     USER **user;
{
  KNUCKLE **k_ptr;  
  int status = 0;
  char mesg[BUF_SIZE];

  if (Knuckle_List == (KNUCKLE **) NULL)
    {
      log_status("get_knuckle: empty list");
      return(EMPTY_LIST);
    }
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(string_eq((*k_ptr)->user->username,person->username))
      {
	*user = (*k_ptr)->user;
	return(SUCCESS);
      }

  return(USER_NOT_FOUND);
}	


/*
 * Function:	get_knuckle() finds a k  structure in the ring given an id.
 * Arguments:	id: id of desired node
 * Returns:	A pointer to the desired user, or NULL if the user is not
 *		in the ring.
 * Notes:
 *	Loop through the user ring, comparing the name with the ids of
 *	nodes in the ring.  If a match is found, return a pointer to the
 *	appropriate user structure.  Otherwise, return NULL. Only the number
 *      of characters to uniquely define the name is necessary.
 */

int
get_knuckle(name,instance,knuckle,active)
     char *name;
     int instance;
     KNUCKLE **knuckle;
     int active;
{
  KNUCKLE **k_ptr;  
  int status = 0;
  char mesg[BUF_SIZE];

#ifdef TEST
  printf("get_knuckle: %s %d...\n",name,instance);
#endif TEST

  if (Knuckle_List == (KNUCKLE **) NULL)
    {
      log_status("get_knuckle: empty list");
      return(EMPTY_LIST);
    }
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(string_eq((*k_ptr)->user->username,name))
      {

#ifdef TEST
	printf("get_knuckle: matched on %s (%d)\n", 
	       (*k_ptr)->user->username,
	       (*k_ptr)->instance);
#endif TEST

	if(active && (!is_active((*k_ptr))))
	  continue;

	if(((*k_ptr)->instance == instance) && !(!is_active((*k_ptr)) &&
						 (*k_ptr)->instance > 0))
	  {
	    *knuckle = *k_ptr;
	    return(SUCCESS);
	  }

	status=1;
      }


  sprintf(mesg,
	  "get_knuckle: matched on %s, incomplete instance %d (%d)",
	  name,instance,status);
  log_status(mesg);

  if(status) 
    return(INSTANCE_NOT_FOUND);

#ifdef TEST
  printf("get_knuckle: no match for %s\n",name);
#endif TEST

  return(USER_NOT_FOUND);
}	


int
match_knuckle(name,instance,knuckle)
     char *name;
     int instance;
     KNUCKLE **knuckle;
{
  KNUCKLE **k_ptr,*store_ptr;
  int status;

  status = get_knuckle(name,instance,knuckle,TRUE);
  if(status != USER_NOT_FOUND)
    return(status);

  status = 0;
  store_ptr = (KNUCKLE *) NULL;

#ifdef TEST
  printf("get_knuckle: %s %d...\n",name,instance);
#endif TEST

  if (Knuckle_List == (KNUCKLE **) NULL)
    return(EMPTY_LIST);
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(string_equiv(name,(*k_ptr)->user->username,strlen(name)) && 
       is_active((*k_ptr)))
      {

#ifdef TEST
	printf("match_knuckle: matched on %s (%d)\n", 
	       (*k_ptr)->user->username,
	       (*k_ptr)->instance);
#endif TEST

	if((*k_ptr)->instance == instance)
	  {
	    if(store_ptr != (KNUCKLE *) NULL)
	      {
		if(store_ptr->instance == (*k_ptr)->instance)
		  return(NAME_NOT_UNIQUE);
	      }
	    else
	      store_ptr = *k_ptr;
	  }
	else
	  status=1;
      }

  if(store_ptr != (KNUCKLE *) NULL)
    {
      *knuckle = store_ptr;
      return(SUCCESS);
    }

#ifdef TEST
  printf("match_knuckle: matched on %s, incomplete instance %d\n",name,status);
#endif TEST

  if(status) 
    return(INSTANCE_NOT_FOUND);

#ifdef TEST
  printf("get_knuckle: no match for %s\n",name);
#endif TEST

  return(USER_NOT_FOUND);
}	


int
find_knuckle(person,knuckle)
     PERSON *person;
     KNUCKLE **knuckle;
{
  char mesg[BUF_SIZE];
  int status;

  status = get_knuckle(person->username, person->instance,knuckle,0);
  if(status == USER_NOT_FOUND || status == EMPTY_LIST)
    {
      sprintf(mesg,"find_knuckle: creating %s",person->username);
      log_status(mesg);
      *knuckle = create_user(person);
      if(*knuckle != (KNUCKLE *) NULL)
	{
	  (*knuckle)->user->status = ACTIVE;
	  return(SUCCESS);
	}
      else
	return(ERROR);
    }
  else
    if(status == SUCCESS)
      {
        strcpy((*knuckle)->user->machine,person->machine);
	strcpy((*knuckle)->user->realname, person->realname);
        (*knuckle)->user->status = ACTIVE;
      }

  return(status);
}
  

get_instance(user,instance)
     char *user;
     int *instance;
{
  KNUCKLE **k_ptr;
  KNUCKLE *k_save = (KNUCKLE *) NULL;

  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
    if(string_eq(user,(*k_ptr)->user->username) && 
      is_active((*k_ptr)))
      {
	if(k_save != (KNUCKLE *) NULL)
	  {
	    if(((*k_ptr)->instance < k_save->instance))
	    k_save = *k_ptr;
	  }
	else
	  k_save = *k_ptr;
      }

  if(k_save != (KNUCKLE *) NULL)
    *instance = k_save->instance;
  else
    *instance = 0;
  
  return(SUCCESS);
}


verify_instance(knuckle,instance)
     KNUCKLE *knuckle;
     int instance;
{
  KNUCKLE **k;
  int i;

  if(instance == 0)
    return(SUCCESS);

  k = knuckle->user->knuckles;
   
  for(i=0; i<= knuckle->user->no_knuckles; i++)
    if(((*(k+i))->instance == instance) && is_active((*(k+i))))
      return(SUCCESS);
  return(FAILURE);
}
      

validate_instance(knuckle)
     KNUCKLE *knuckle;
{
  int i;

  i = assign_instance(knuckle->user);
  if(i < knuckle->instance)
    return(i);
  else
    return(knuckle->instance);
}

int
assign_instance(user)
     USER *user;
{
  KNUCKLE **k;
  int match;
  int i,j;

  k = user->knuckles;
  
  for(i=0; i<= user->no_knuckles; i++)
    {
      match = 0;
      for(j=0; j < user->no_knuckles; j++)
	{
	  if( (*(k+j))->instance == i)
	    {

#ifdef TEST
	      printf("assign_instance: match on %d, no = %d\n",i,
		     user->no_knuckles);
#endif TEST

	      match = 1;
	      break;
	    }
	}
      if(!match)
	return(i);
    }
  return(ERROR);
}

/*
 * Function:	connect_users() connects a user and a user.
 * Arguments:	a:	        User to be connected.
 *		b:		User to be connected.
 * Returns:	Nothing.
 * Notes:
 */

connect_knuckles(a,b)
     KNUCKLE *a, *b;
{
  char msg[BUFSIZ];

  if(is_connected(a) || is_connected(b))
    {
      log_error("connect: users already connected");
      return(FAILURE);
    }

  if(a->question == (QUESTION *) NULL)
    {
      if(b->question == (QUESTION *) NULL)
	{
	  log_error("connect: connectee has no question");
	  return(ERROR);
	}
      a->question = b->question;
      strcpy(a->title,a->user->title2);
      strcpy(b->title,b->user->title1);
    }
  else
    {
      if(b->question != (QUESTION *) NULL)
	{
	  log_error("connect: connectee already has question");
	  return(ERROR);
	}
      b->question = a->question;
      strcpy(a->title,a->user->title1);
      strcpy(b->title,b->user->title2);
    }
  
  a->connected = b;
  (void) strcpy(a->cusername,b->user->username);
  a->cinstance = b->instance;
  b->connected = a;
  (void) strcpy(b->cusername,a->user->username);
  b->cinstance = a->instance;

  (void) sprintf(msg,"You are connected to %s %s (%s).\n",
	  a->title, a->user->realname,a->user->username);
  if(write_message_to_user(b,msg,0)!=SUCCESS)
    {
/*      a->connected = (KNUCKLE *) NULL;
      b->connected = (KNUCKLE *) NULL;
      return(FAILURE);*/
    }
      
  (void) sprintf(msg,"You are connected to %s %s (%s).\n",
	  b->title, b->user->realname, b->user->username);
  if((write_message_to_user(a,msg,0) != SUCCESS) &&  (!is_signed_on(b)))
    {
/*      sprintf(msg,"Oops, he just logged out. Will try to find another...");
      write_message_to_user(b,msg,0);
      a->connected = (KNUCKLE *) NULL;
      b->connected = (KNUCKLE *) NULL;
      return(FAILURE);*/
    }

  if (!was_connected(a,b))
    {
      a->question->seen[a->question->nseen] = b->user->uid;
      a->question->nseen++;
      a->question->seen[a->question->nseen] = -1;
    }

  return(SUCCESS);
}



/*
 * Function:	find_available_consultant() finds an available consultant in
 *			the list.
 * Arguments:	user:	Ptr. to user structure for user needing a consultant.
 * Returns:	SUCCESS or FAILURE;
 * Notes:
 */

match_maker(knuckle)
     KNUCKLE *knuckle;
{
  KNUCKLE **k_ptr, *temp = (KNUCKLE *) NULL;	
  int priority, queue, foo;
  long t = 0;
  char msgbuf[BUFSIZ];
  int status;
  
  if(!has_question(knuckle))
    {
      if(is_logout(knuckle) ||
	 is_connected(knuckle) ||
	 !is_signed_on(knuckle))
	return(FAILURE);

      priority = CANCEL;   /* lowest connectable priority */
      
#ifdef TEST
      printf("match_maker: %s (%d) has no question\n", 
	     knuckle->user->username, knuckle->instance);
#endif TEST

      for(k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
	{

#ifdef TEST
	  printf("match_maker: status: %d   %d queue: %d   ts:  %d\n",
		 (*k_ptr)->status, (*k_ptr)->user->status,
		 (*k_ptr)->queue, (*k_ptr)->timestamp);
	  printf("match_maker: status: %d   %d queue: %d   ts:  %d\n",
		 knuckle->status, knuckle->user->status,
		 knuckle->queue, knuckle->timestamp);
#endif TEST
	  
	  if(!has_question((*k_ptr)))
	    continue;
	  if(is_connected((*k_ptr)))
	    continue;
	  if(was_connected((*k_ptr),knuckle))
	    continue;
	  if(is_logout((*k_ptr)))
	    continue;
	  if((*k_ptr)->status > QUESTION_STATUS)
	    continue;
	  if((*k_ptr) == knuckle)
	    continue;
	  else
	    if(((*k_ptr)->status & QUESTION_STATUS) > priority)
	      continue;
	    else
	      if(((*k_ptr)->timestamp >= t) && 
		 ((*k_ptr)->status == priority))
		continue;
		 
	  switch(knuckle->status & SIGNED_ON)
	    {
	    case FIRST:
	    case SECOND:
	      if(is_specialty(knuckle->user,(*k_ptr)->question->topic_code))
		{
		  temp = *k_ptr;
		  t = (*k_ptr)->timestamp;
		  priority = (*k_ptr)->status;
		}
	      break;
	    case DUTY:
	    case URGENT:
	      if(is_specialty(knuckle->user,(*k_ptr)->question->topic_code))
		{
		  temp = *k_ptr;
		  t = (*k_ptr)->timestamp;
		  foo = DUTY;
		  priority = (*k_ptr)->status;
		}
	      else
		if(foo != DUTY)
		  {
#ifdef TEST
		    printf("match_maker: assigning to %s\n",
			   (*k_ptr)->user->username);
#endif TEST
		    temp = *k_ptr;
		    t = (*k_ptr)->timestamp;
		    priority = (*k_ptr)->status;
		  }
	      break;
	    default:
	      break;
	    }
	}
    }
  else
    {
      if(is_logout(knuckle) ||
	 is_pitted(knuckle) ||
	 is_connected(knuckle) ||
	 (knuckle->status > QUESTION_STATUS))
	return(FAILURE);
      
      for(k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
	{
	  if((*k_ptr) == knuckle)
	    continue;
	  if(is_logout((*k_ptr)))
	    continue;
	  if(is_connected((*k_ptr)))
	    continue;
	  if(priority < (*k_ptr)->status & SIGNED_ON)
	     continue;
	  if((priority == is_signed_on((*k_ptr))) &&
	     t >= (*k_ptr)->timestamp)
	     continue;

	  switch((*k_ptr)->status & SIGNED_ON)
	    {
	    case FIRST:
	    case SECOND:
	      if(!is_specialty((*k_ptr)->user,knuckle->question->topic_code))
		break;
	      priority = (*k_ptr)->status & SIGNED_ON;
	      t = (*k_ptr)->timestamp;
	      temp = *k_ptr;
	      break;
	    case DUTY:
	    case URGENT:
	      if(is_specialty((*k_ptr)->user,(knuckle)->question->topic_code))
		{
		  temp = *k_ptr;
		  t = (*k_ptr)->timestamp;
		  foo = DUTY;
		  priority = (*k_ptr)->status & SIGNED_ON;
		}
	      else
		if(foo != DUTY)
		  {
		    temp = *k_ptr;
		    t = (*k_ptr)->timestamp;
		    priority = (*k_ptr)->status & SIGNED_ON;
		  }
	      break;
	    default:
	      break;
	    }
	}
    }
    
  if(temp != (KNUCKLE *) NULL)
    {
      status = connect_knuckles(temp,knuckle);
      if(status == SUCCESS)
	{
	  if(!owns_question(temp))
	    (void) sprintf(msgbuf,"Connected to %s %s (%d) %s@%s",
			   temp->title,
			   temp->user->realname,
			   temp->instance,
			   temp->user->username, 
			   temp->user->machine);
	  else
	    (void) sprintf(msgbuf,"Connected to %s %s (%d) %s@%s",
			   knuckle->title,
			   knuckle->user->realname,
			   knuckle->instance,
			   knuckle->user->username, 
			   knuckle->user->machine);
	  log_daemon(temp,msgbuf);
	  return(SUCCESS);
	}
      else
	if(status == FAILURE)
	  return(match_maker(knuckle));
	else
	  return(ERROR);
    }
  else
    return(FAILURE);  
}


/*
 * Function:	new_message() takes a new message for a user or a consultant
 *			and stores it in the messages field of the
 *			appropriate structure.
 * Arguments:	msg_field:	A pointer to the message field for the
 *				appropriate structure.
 *		message:	The new message.
 * Returns:	Nothing.
 * Notes:
 *	First, we allocate space for the new message string, which is then
 *	formed by concatenating the old messages, the date and time, and
 *	the new message, with some newlines to handle spacing.
 *	Then we free the old message memory and change the msg_field
 *	to point at the right string.
 */

new_message(msg_field, sender, message)
     char **msg_field;	/* Place to store the new message. */
     KNUCKLE *sender;
     char *message;		/* Message string. */
{
  int curr_length;	        /* Length of current message. */
  int msg_length;		/* Length of the new message. */
  int length;	                 /* Length of header string. */
  char *new_message;	        /* Ptr. to constructed new message. */
  char timebuf[TIME_SIZE];      /* Current time. */
  char buf[BUF_SIZE];

  time_now(timebuf);
  sprintf(buf,"\nMessage from %s %s (%s) on %s:\n\n",sender->title, 
	  sender->user->realname, sender->user->username, timebuf);
  length = strlen(buf);
  
  if (*msg_field == (char *) NULL)
    curr_length = 0;
  else
    curr_length = strlen(*msg_field);
  
  msg_length = strlen(message);
  new_message = malloc((unsigned) curr_length + msg_length + length + 10);
  if (new_message == (char *)NULL) 
    {
      log_error("new_message: malloc failed");
      return;
    }

  new_message[0] = '\0';
  if (*msg_field != (char *) NULL) 
    {
      (void) strcpy(new_message, *msg_field);
      (void) strcat(new_message, "\n");
    }

  (void) strcat(new_message, buf);
  (void) strcat(new_message, message);

  if (*msg_field != (char *)NULL)
    free(*msg_field);

  *msg_field = new_message;
}



/*
 * Function:	get_status_info() returns some status information.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 *	The status information is returned in a static structure.
 */

QUEUE_STATUS *
get_status_info()
{
  static QUEUE_STATUS status;	/* Static status structure. */
  KNUCKLE **k_ptr;	/* Current consultant. */

  status.consultants = 0;
  status.busy = 0;
  status.waiting = 0;

  if (Knuckle_List != (KNUCKLE **) NULL) 
    {
      for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
	{
	  if ((*k_ptr)->question == (QUESTION *) NULL)
	    {
	      if((*k_ptr)->connected != (KNUCKLE *) NULL)
		status.busy++;
	      status.consultants++;
	    }
	  else
	    if((*k_ptr)->connected == (KNUCKLE *) NULL)
	      status.waiting++;
	}
    }
  return(&status);
}

/*
 * Function:	verify_topic() checks to make sure that a topic is legitimate.
 * Arguments:	topic:	topic to be checked.
 * Returns:	SUCCESS, FAILURE, or ERROR.
 * Notes:
 *	Scan through the topics file, matching each line against the given
 *	topic.  If a match is found, return SUCCESS; otherwise return FAILURE.
 */



int
verify_topic(topic)
     char *topic;
{
  TOPIC **t_ptr;

  if (strlen(topic) == 0) 
    return(FAILURE);
  
  for(t_ptr = Topic_List; *t_ptr != (TOPIC *) NULL; t_ptr++)
    if(string_eq(topic,(*t_ptr)->name))
      return((*t_ptr)->value);

  return(FAILURE);
}


  
owns_question(knuckle)
     KNUCKLE *knuckle;
{
  if(knuckle == (KNUCKLE *) NULL)
    return(FALSE);

  if(!has_question(knuckle))
    return(FALSE);

  if(knuckle->question->owner == knuckle)
    return(TRUE);
  else
    return(FALSE);
}


is_topic(topics,code)
     int *topics;
     int code;
{
  while(topics != (int *) NULL)
    {
      if(*topics == code)
	return(TRUE);
      ++topics;
      if((*topics == UNKNOWN_TOPIC) || (*topics <= 0))
	break;
    }
  return(FALSE);
}

was_connected(a,b)
     KNUCKLE *a, *b;
{
  int i = 0;

  fprintf(stderr, "Nseen: %d  UIDs: ", a->question->nseen);
  for(i=0; a->question->seen[i] != -1; i++)
    fprintf(stderr, "%d ", a->question->seen[i]);
  fprintf(stderr, "\n");

  for(i=0; a->question->seen[i] != -1; i++)
    if(a->question->seen[i] == b->user->uid)
      return(TRUE);

  return(FALSE);
}
