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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/data_utils.c,v $
 *	$Id: data_utils.c,v 1.20 1990-08-20 04:47:46 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/data_utils.c,v 1.20 1990-08-20 04:47:46 lwvanels Exp $";
#endif

#include <mit-copyright.h>
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

#ifdef __STDC__
static int validate_instance (KNUCKLE *);
static int assign_instance (USER *);
static int was_connected (KNUCKLE *, KNUCKLE *);
void disconnect_knuckles (KNUCKLE *, KNUCKLE *);
#else
static int validate_instance ();
static int assign_instance ();
static int was_connected ();
void disconnect_knuckles ();
#endif /* STDC */


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
#ifdef __STDC__
create_user(PERSON *person)
#else
create_user(person)
     PERSON *person;
#endif /* STDC */
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
#ifdef __STDC__
create_knuckle(USER *user)
#else
create_knuckle(user)
     USER *user;
#endif /* STDC */
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

int
#ifdef __STDC__
insert_knuckle(KNUCKLE *knuckle)
#else
insert_knuckle(knuckle)
     KNUCKLE *knuckle;
#endif /* STDC */
{
  KNUCKLE **k_ptr;
  int n_knuckles=0;
  int n_inactive=0;
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

int
#ifdef __STDC__
insert_knuckle_in_user(KNUCKLE *knuckle, USER *user)
#else
insert_knuckle_in_user(knuckle, user)
     KNUCKLE *knuckle;
     USER *user;
#endif /* STDC */
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

int
#ifdef __STDC__
insert_topic(TOPIC *t)
#else
insert_topic(t)
     TOPIC *t;
#endif /* STDC */
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
#ifdef __STDC__
delete_user(USER *user)
#else
delete_user(user)
     USER *user;
#endif /* STDC */
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
#ifdef __STDC__
delete_knuckle(KNUCKLE *knuckle, int cont)
#else
delete_knuckle(knuckle,cont)
     KNUCKLE *knuckle;
     int cont;
#endif /* STDC */
{
  int n_knuckles, knuckle_idx = -1;
  char msgbuf[BUFSIZ];
  KNUCKLE **k_ptr;
  int i;

  for (n_knuckles = 0; Knuckle_List[n_knuckles]; n_knuckles++)
      if (Knuckle_List[n_knuckles] == knuckle)
	  knuckle_idx = n_knuckles;
  if (knuckle_idx == -1)
      return;

  Knuckle_List[knuckle_idx]  = Knuckle_List[n_knuckles-1];
  Knuckle_List[n_knuckles-1] = (KNUCKLE *) NULL;

  if(!cont)
    Knuckle_List = (KNUCKLE **) realloc(Knuckle_List, 
					sizeof(KNUCKLE *) * (n_knuckles-1));
      

  /* maintain continuity in the user knuckle list */
  k_ptr = knuckle->user->knuckles;
  for(i=0;i<knuckle->user->no_knuckles;i++)
      if (knuckle == k_ptr[i])
	  break;
  
  k_ptr[i] = k_ptr[knuckle->user->no_knuckles - 1];
  k_ptr[knuckle->user->no_knuckles - 1] = 0;

  /* delete user if last knuckle */
  if (knuckle->user->no_knuckles == 1)
      free((char *) knuckle->user);
  else
      knuckle->user->no_knuckles--;

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


int
#ifdef __STDC__
deactivate_knuckle(KNUCKLE *knuckle)
#else
deactivate_knuckle(knuckle)
     KNUCKLE *knuckle;
#endif /* STDC */
{
  if(knuckle->instance > 0)
    delete_knuckle(knuckle, /*???*/0);
  else
    knuckle->status = 0;
  return(SUCCESS);
}


void
#ifdef __STDC__
init_user(KNUCKLE *knuckle, PERSON *person)
#else
init_user(knuckle,person)
     KNUCKLE *knuckle;
     PERSON *person;
#endif /* STDC */
{
  (void) strncpy(knuckle->user->realname,person->realname, NAME_SIZE);
  (void) strncpy(knuckle->user->username,person->username, LOGIN_SIZE);
  (void) strncpy(knuckle->user->machine,person->machine, NAME_SIZE);
  (void) strncpy(knuckle->user->realm,person->realm, REALM_SZ);
  knuckle->user->uid = person->uid;
  knuckle->status = 0;
  init_dbinfo(knuckle->user);
  (void) strcpy(knuckle->title,knuckle->user->title1);
}


void
#ifdef __STDC__
init_dbinfo(USER *user)
#else
init_dbinfo(user)
    USER *user;
#endif /* STDC */
{
  (void) strcpy(user->title1, DEFAULT_TITLE);
  (void) strcpy(user->title2, DEFAULT_TITLE2);
  user->max_ask = 1;
  user->max_answer = 2;
  user->permissions = 0;
  user->specialties[0] = UNKNOWN_TOPIC;
  load_user(user);
}


int
#ifdef __STDC__
init_question(KNUCKLE *k, char *topic, char *text, char *machinfo)
#else
init_question(k,topic,text, machinfo)
     KNUCKLE *k;
     char *topic;
     char *text;
     char *machinfo;
#endif /* STDC */
{
  struct timeval tp;
  int i, j;

  k->question = (QUESTION *) malloc(sizeof(QUESTION));
  if(k->question == (QUESTION *) NULL)
    {
      perror("init_question");
      return(ERROR);
    }

  gettimeofday( &tp, 0 );

  k->timestamp = tp.tv_sec;
  k->question->owner = k;
  k->queue = ACTIVE_Q;
  k->question->nseen = 0;
  k->question->seen[0] = -1;
  k->question->comment[0] = '\0';
  k->question->topic_code = verify_topic(topic);
  (void) strcpy(k->title,k->user->title1);
  (void) strcpy(k->question->topic,topic);
  init_log(k, text, machinfo);
/*
 * Set the initial description
 */
  k->question->note[0] = '*';
  k->question->note[1] = '*';
  
  for (i=0,j=2; ((j < NOTE_SIZE-1) && (text[i] != '\0')); i++)
    {
      if ((text[i] == '\t') || (text[i] == '\n') || (text[i] == ' '))
	{
	  if (k->question->note[j-1] != ' ')
	    {
	      k->question->note[j] = ' ';
	      j++;
	    }
	}
      else
	{
	  k->question->note[j] = text[i];
	  j++;
	}
    }
  k->question->note[j] = '\0';

  return(SUCCESS);
}


int
#ifdef __STDC__
get_user(PERSON *person, USER **user)
#else
get_user(person,user)
     PERSON *person;
     USER **user;
#endif /* STDC */
{
  KNUCKLE **k_ptr;  

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
#ifdef __STDC__
get_knuckle(char *name, int instance, KNUCKLE **knuckle, int active)
#else
get_knuckle(name,instance,knuckle,active)
     char *name;
     int instance;
     KNUCKLE **knuckle;
     int active;
#endif /* STDC */
{
  KNUCKLE **k_ptr;  
  int status = 0;
#ifdef TEST
  char mesg[BUF_SIZE];
#endif TEST

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


#ifdef TEST
  sprintf(mesg,
	  "get_knuckle: matched on %s, incomplete instance %d (%d)",
	  name,instance,status);
  log_status(mesg);
#endif

  if(status) 
    return(INSTANCE_NOT_FOUND);

#ifdef TEST
  printf("get_knuckle: no match for %s\n",name);
#endif

  return(USER_NOT_FOUND);
}	


int
#ifdef __STDC__
match_knuckle(char *name, int instance, KNUCKLE **knuckle)
#else
match_knuckle(name,instance,knuckle)
     char *name;
     int instance;
     KNUCKLE **knuckle;
#endif /* STDC */
{
  KNUCKLE **k_ptr,*store_ptr;
  int status;
  int not_unique = 0;

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
/******
  I wonder whether this next "if" can ever evaluate FALSE.  If we got
  by the if statement just after the endif TEST above, then we are
  going to pass this one, too, aren't we?    -Chris.
  ******/ 
		if(store_ptr->instance == (*k_ptr)->instance)
		  not_unique = 1;
	      }
	    else
	      store_ptr = *k_ptr;
	  }
	else
	  status=1;
      }

  if(not_unique)
    return(NAME_NOT_UNIQUE);

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
#ifdef __STDC__
find_knuckle(PERSON *person, KNUCKLE **knuckle)
#else
find_knuckle(person,knuckle)
     PERSON *person;
     KNUCKLE **knuckle;
#endif /* STDC */
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
  
int
#ifdef __STDC__
get_instance(char *user, int *instance)
#else
get_instance(user,instance)
     char *user;
     int *instance;
#endif /* STDC */
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


int
#ifdef __STDC__
verify_instance(KNUCKLE *knuckle, int instance)
#else
verify_instance(knuckle,instance)
     KNUCKLE *knuckle;
     int instance;
#endif /* STDC */
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
      

static int
#ifdef __STDC__
validate_instance(KNUCKLE *knuckle)
#else
validate_instance(knuckle)
     KNUCKLE *knuckle;
#endif /* STDC */
{
  int i;

  i = assign_instance(knuckle->user);
  if(i < knuckle->instance)
    return(i);
  else
    return(knuckle->instance);
}


static int
#ifdef __STDC__
assign_instance(USER *user)
#else
assign_instance(user)
     USER *user;
#endif /* STDC */
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

int
#ifdef __STDC__
connect_knuckles(KNUCKLE *a, KNUCKLE *b)
#else
connect_knuckles(a,b)
     KNUCKLE *a, *b;
#endif /* STDC */
{
  char msg[BUFSIZ];
  KNUCKLE *owner, *consultant;
  int ret;

  if(is_connected(a) || is_connected(b))
    {
      log_error("connect: users already connected");
      return(FAILURE);
    }

  if(a->question == (QUESTION *) NULL)
    {
      if(b->question == (QUESTION *) NULL)
	{
	  sprintf(msg,
		  "connect: neither knuckle has a question -- %s[%d], %s[%d]",
		  a->user->username, a->instance,
		  b->user->username, b->instance);
	  log_error(msg);
	  return(ERROR);
	}
      if (owns_question(b))
	{
	  owner = b;
	  consultant = a;
	}
      else
	{
	  sprintf(msg,
		  "connect: conectee already connected -- %s[%d]",
		  b->user->username, b->instance);
	  log_error(msg);
	  return(ERROR);
	}
    }
  else  /*** a->question != NULL ***/
    {
      if(b->question != (QUESTION *) NULL)
	{
	  sprintf(msg,
		  "connect: both connectees have questions -- %s[%d], %s[%d]",
		  a->user->username, a->instance,
		  b->user->username, b->instance);
	  log_error(msg);
	  return(ERROR);
	}
      if (owns_question(a))
	{
	  owner = a;
	  consultant = b;
	}
      else
	{
	  sprintf(msg,
		  "connect: conectee already connected -- %s[%d]",
		  b->user->username, b->instance);
	  log_error(msg);
	  return(ERROR);
	}
    }
  
  consultant->question = owner->question;
  (void) strcpy(owner->title, owner->user->title1);
  (void) strcpy(consultant->title, consultant->user->title2);

  a->connected = b;
  (void) strcpy(a->cusername,b->user->username);
  a->cinstance = b->instance;

  b->connected = a;
  (void) strcpy(b->cusername,a->user->username);
  b->cinstance = a->instance;

  (void) sprintf(msg,"You are connected to %s %s (%s@%s [%d]).\n",
		 owner->title, owner->user->realname, owner->user->username,
		 owner->user->machine, owner->instance);
  if(write_message_to_user(consultant,msg,0)!=SUCCESS)
    {
      free_new_messages(consultant);
      deactivate(consultant);
      disconnect_knuckles(owner, consultant);
      return(FAILURE);
    }
      
  (void) sprintf(msg,"You are connected to %s %s (%s@%s [%d]).\n",
		 consultant->title, consultant->user->realname,
		 consultant->user->username,
		 consultant->user->machine, consultant->instance);
  ret = write_message_to_user(owner,msg,0);

  if (!was_connected(owner, consultant))
    {
      owner->question->seen[owner->question->nseen] = consultant->user->uid;
      owner->question->nseen++;
      owner->question->seen[owner->question->nseen] = -1;
    }

  return(SUCCESS);
/*
 *  It would be nice to be able to return "ret", since that way the
 *  consultant could be informed that the user has logged out when trying
 *  to grab a user, and then the consultant could back out or something.
 *  This, however, would require a protocol change...
 *
 *  It would also mean that the match_maker could abort from connecting
 *  somebody to a logged out user...  actually, this could be done
 *  without a protocol change, but there isn't time right now to implement
 *  it.
 *
 *  return(ret);
 *
 */
}


void
#ifdef __STDC__
disconnect_knuckles(KNUCKLE *a, KNUCKLE *b)
#else
disconnect_knuckles(a, b)
     KNUCKLE *a, *b;
#endif /* STDC */
{
  b->connected = (KNUCKLE *) NULL;
  b->cusername[0] = (char) NULL;
  if (owns_question(a))
    b->question = (QUESTION *) NULL;

  a->connected = (KNUCKLE *) NULL;
  a->cusername[0] = (char) NULL;
  if (owns_question(b))
    a->question = (QUESTION *) NULL;
}


void
#ifdef __STDC__
free_new_messages(KNUCKLE *knuckle)
#else
free_new_messages(knuckle)
     KNUCKLE *knuckle;
#endif /* STDC */
{
  if(knuckle->new_messages != (char *) NULL)
    {
      free((char *) knuckle->new_messages);
      knuckle->new_messages = (char *) NULL;
    }
}


/*
 * Function:	match_maker
 * Arguments:	KNUCKLE* knuckle: The user or consultant who should be
 *		connected to another person (of the opposite type),
 *		subject to various constraints.
 * Returns:	SUCCESS, FAILURE, or ERROR.
 * Description:	If the `knuckle' in question is an unconnected user,
 *		find her a consultant.  For unconnected consultants,
 *		find unconnected users.  Constraints:
 *
 *		1. Candidates must be logged in, and not connected to
 *		any other `knuckle'.  (This refers only to the
 *		`knuckles' to be connected; other instances are
 *		ignored.)
 *		2. Consultants with status FIRST or SECOND will not be
 *		automatically connected to questions with topics for
 *		which the consultant is not a specialist.
 *		3. Priority order for finding a consultant is: FIRST,
 *		DUTY, SECOND, URGENT.  Consultants not signed on are
 *		not automatically assigned questions.
 *		4. A question in "pickup" or "refer" state should not
 *		be connected to a consultant.
 *		5. Older questions should get connected to available
 *		consultants before newer ones.
 *		6. If two or more consultants are available and
 *		eligible for connection to a question, and they have
 *		the same status, consultants with the question's topic
 *		as a specialty have priority over others.
 *		7. Consultants with a status of URGENT will be
 *		connected only to UNSEEN questions.
 */

int
#ifdef __STDC__
match_maker(KNUCKLE *knuckle)
#else
match_maker(knuckle)
     KNUCKLE *knuckle;
#endif /* STDC */
{
    KNUCKLE *k, *match = (KNUCKLE *) NULL;	
    int i, k_status = knuckle->status;
    char msgbuf[BUFSIZ];
    int status;

    /* constraint 1 for this knuckle */
    if (is_logout (knuckle) || is_connected (knuckle))
	return FAILURE;
    
    if (!has_question (knuckle)) {
	/* this is a consultant's knuckle; look for questions */
	
	k_status &= SIGNED_ON;
	if (!is_signed_on (knuckle))
	    return FAILURE;
	
	for (i = 0; Knuckle_List[i]; i++) {
	    k = Knuckle_List[i];
	    /* go through unconnected users, find a match */
	    if(!has_question(k))
		continue;
	    if(is_connected(k))
		continue;
	    if(was_connected(k,knuckle))
		continue;
	    if(is_logout(k))
		continue;
	    switch (k->status) {
	    case PICKUP:
	    case REFERRED:
		continue;
	    case FIRST:
	    case SECOND:
	    case DUTY:
	    case URGENT:
		/* consultants? */
		continue;
	    default:
		/* users */
		break;
	    }
	    if (k == knuckle)
		continue;		/* don't connect to oneself */
#if 0
	    {
		printf ("<Cstatus=%x,", k_status);
		printf ("Ustatus=%x,", k->status);
		printf ("spec=(%d,%d)>\n",
			k->question->topic_code,
			is_specialty (knuckle->user, k->question->topic_code));
	    }
#endif
	    if (k_status == URGENT
		&& k->status != NOT_SEEN)
		continue;
	    else if ((k_status == FIRST
		      || k_status == SECOND)
		     && !is_specialty (knuckle->user, k->question->topic_code))
		continue;
	    /* last check: sort by time */
	    if (match && k->timestamp >= match->timestamp)
		continue;
	    /*
	     * XXX - When a consultant signs on, should specialty
	     * questions get priority over older non-specialty questions?
	     */
	    
	    match = k;
	}
	
    }
    else {
	/* unconnected user: find a consultant */
	k_status &= QUESTION_STATUS;
	switch (k_status) {
	case PICKUP:
	case REFERRED:
	    return FAILURE;
	case CANCEL:
	case DONE:
	    /* shouldn't get here */
	    log_error (fmt ("unconnected user has status %d in match_maker",
			    k_status));
	    return ERROR;
	default:
	    /* ok */
	    ;
	}

	for (i = 0; Knuckle_List[i]; i++) {
	    k = Knuckle_List[i];
	    /* check each consultant for availability */
	    if (k == knuckle)
		continue;
	    if (is_logout(k))
		continue;
	    if (is_connected(k))
		continue;
	    switch (k->status) {
	    case FIRST:
	    case SECOND:
		if (!is_specialty (k->user,
				   knuckle->question->topic_code))
		    continue;
		break;
	    case URGENT:
		if (k_status != NOT_SEEN)
		    continue;
		break;
	    case DUTY:
		break;
	    default:
		/* non-consultants */
		continue;
	    }
	    /* selection done; effect sorting when needed */
	    if (match) {
		int s1, s2;
		s1 = match->status & SIGNED_ON;
		s2 = k->status & SIGNED_ON;
		if (s1 > s2)
		    continue;
		if ((s1 == s2)
		    && is_specialty (match->user,
				     knuckle->question->topic_code))
		    continue;
	    }
	    /* if we get here, we won */
	    match = k;
	}
    }

    if (!match) /* oh well */
	return FAILURE;

    status = connect_knuckles(match,knuckle);
    if (status == FAILURE) /* try it again... */
	return match_maker (knuckle);
    else if (status != SUCCESS) /* ??? */
	return ERROR;

    /* log a message in the log file */
    if (!owns_question(match))
	knuckle = match;	/* use knuckle for consultant name now */
    (void) sprintf(msgbuf,"Connected to %s %s %s@%s [%d]",
		   knuckle->title,
		   knuckle->user->realname,
		   knuckle->user->username, 
		   knuckle->user->machine,
		   knuckle->instance);
    log_daemon(match, msgbuf);
    return SUCCESS;
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

void
#ifdef __STDC__
new_message(char **msg_field, KNUCKLE *sender, char *message)
#else
new_message(msg_field, sender, message)
     char **msg_field;	/* Place to store the new message. */
     KNUCKLE *sender;
     char *message;		/* Message string. */
#endif /* STDC */
{
  int curr_length;	        /* Length of current message. */
  int msg_length;		/* Length of the new message. */
  int length;	                 /* Length of header string. */
  char *new_message;	        /* Ptr. to constructed new message. */
  char timebuf[TIME_SIZE];      /* Current time. */
  char buf[BUF_SIZE];

  time_now(timebuf);
  sprintf(buf,"\n--- Message from %s %s (%s@%s [%d]).\n    [%s]\n",
	  sender->title, sender->user->realname,
	  sender->user->username, sender->user->machine,
	  sender->instance, timebuf);
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
#ifdef __STDC__
get_status_info(void)
#else
get_status_info()
#endif /* STDC */
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
#ifdef __STDC__
verify_topic(char *topic)
#else
verify_topic(topic)
     char *topic;
#endif /* STDC */
{
  TOPIC **t_ptr;

  if (strlen(topic) == 0) 
    return(FAILURE);
  
  for(t_ptr = Topic_List; *t_ptr != (TOPIC *) NULL; t_ptr++)
    if(string_eq(topic,(*t_ptr)->name))
      return((*t_ptr)->value);

  return(FAILURE);
}


  
int
#ifdef __STDC__
owns_question(KNUCKLE *knuckle)
#else
owns_question(knuckle)
     KNUCKLE *knuckle;
#endif /* STDC */
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


int
#ifdef __STDC__
is_topic(int *topics, int code)
#else
is_topic(topics,code)
     int *topics;
     int code;
#endif /* STDC */
{
    if (!topics)
	return FALSE;
    while (*topics != code) {
	if((*topics == UNKNOWN_TOPIC) || (*topics <= 0)) {
	    return FALSE;
	}
	++topics;
    }
    return TRUE;
}

static int
#ifdef __STDC__
was_connected(KNUCKLE *a, KNUCKLE *b)
#else
was_connected(a,b)
     KNUCKLE *a, *b;
#endif /* STDC */
{
  int i = 0;

  for(i=0; a->question->seen[i] != -1; i++)
    if(a->question->seen[i] == b->user->uid)
      return(TRUE);

  return(FALSE);
}
