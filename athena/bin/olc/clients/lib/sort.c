/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for exectuting olc commands.
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
 *	$Id: sort.c,v 1.15 1999-03-06 16:47:40 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: sort.c,v 1.15 1999-03-06 16:47:40 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/sort.h>

#if defined(__STDC__)
# define P_(s) s
#else
# define P_(s) ()
#endif

static int foo P_((int status));
static int bar P_((int status));
static int kstatusp P_((int status));
static int compare P_((LIST *first, LIST *second));
#undef P_

static int foo (status)
     int status;
{
  switch (status) {
  case SERVICED:
  case ACTIVE:
  case PENDING:
  case NOT_SEEN:
  case DONE:
  case CANCEL:
    /* unconnected-consultant status: map to active-question */
  case ON:
  case FIRST:
  case OFF:
  case DUTY:
  case SECOND:
  case URGENT:
    return 1;
  case REFERRED:
    return 3;
  case PICKUP:
    return 2;
  }
  {
    char buf[60];
    OGetStatusString (status, buf);
    printf ("*** foo(%08x[%s])\n", status, buf);
  }
  return 0x7000000;
}

static int bar (status) int status; {
    return (status == ON
	    || status == FIRST
	    || status == OFF
	    || status == DUTY
	    || status == SECOND);
}

static int kstatusp(status) int status; {
  switch(status) {
  case ACTIVE:
  case DONE:
  case CANCEL:
  case SERVICED:
  case PENDING:
    return 2;
  case NOT_SEEN:
    return 3;
  }
}

#define cmp(a,b) ((a)<(b)?-1:((a)==(b)?0:1))
#define sgn(n) ((n)<0?-1:((n)>0?1:0))
/* qsort should be persuaded to pass this */
static sort_keys *keys;

static int
compare (first, second)
  LIST *first, *second;
 {
    sort_keys *k;
    int order = 0;

    for (k = keys; !order; k++) {
	switch (k->key) {
	case sort_key__none:
	    return 0;
	case sort_key__user_name:
	    order = strcmp (first->user.username, second->user.username);
	    break;
	case sort_key__instance:
	    order = cmp (first->user.instance, second->user.instance);
	    break;
	case sort_key__consultant_name:
	    order = strcmp (first->connected.username,
			    second->connected.username);
	    break;
	case sort_key__time:
	    order = cmp(first->utime, second->utime);
	    break;
	case sort_key__question_status:
	    order = cmp(kstatusp(first->ukstatus), kstatusp(second->ukstatus));
	    break;
	case sort_key__topic:
	    order = strcmp (first->topic, second->topic);
	    break;
	case sort_key__nseen:
	    order = cmp (first->nseen, second->nseen);
	    break;
	case sort_key__foo:
	    order = cmp (foo (first->ukstatus),
			 foo (second->ukstatus));
	    break;
	case sort_key__connected_consultant:
	    order = bar (first->ukstatus) - bar (second->ukstatus);
	    break;
	default:
	    /* abort(); */
	    return 0;
	}
	if (!order)
	    continue;
	order = sgn(order);
	if (k->reversed)
	    order = - order;
	return order;
    }
}

ERRCODE OSortListByKeys (list, skeys)
    LIST *list;
    sort_keys *skeys;
{
    int n_entries;
    if (!list)
	return ERROR;
    keys = skeys;
    for (n_entries = 0; list[n_entries].ustatus != END_OF_LIST; n_entries++)
	;
    qsort (list, n_entries, sizeof (*list), compare);
    return 0;
}

ERRCODE
OSortListByRule(list,rule)
     LIST *list;
     char **rule;
{
    sort_keys keys[30];
    sort_keys *k = keys;
    if(list == (LIST *) NULL)
	return(ERROR);
    if(rule == (char **) NULL)
	return(ERROR);
    if(list->ustatus == END_OF_LIST)
	return(SUCCESS);

    while((*rule != (char *) NULL) && (*rule[0] != '\0')) {
	k->reversed = 0;
	if (string_eq(*rule,"topic"))
	    k++->key = sort_key__topic;
	else if (string_eq(*rule,"nseen"))
	    k++->key = sort_key__nseen;
	else if (string_eq(*rule,"rev_nseen")) {
	    k->key = sort_key__nseen;
	    k++->reversed = 1;
	  }
	else if (string_eq(*rule,"uusername") || string_eq(*rule,"username"))
	    k++->key = sort_key__user_name;
	else if(string_eq(*rule,"cusername"))
	    k++->key = sort_key__consultant_name;
	else if (string_eq (*rule, "foo")) /* what should i call this?? */
	    k++->key = sort_key__foo;
	else if (string_eq (*rule,"q_status"))
	  k++->key = sort_key__question_status;
	else if (string_eq(*rule,"utime") || string_eq (*rule, "time"))
	    k++->key = sort_key__time;
	else if (string_eq (*rule, "unconnected_consultants_last"))
	    k++->key = sort_key__connected_consultant;
	else {
	    printf ("unknown sort key %s\n", *rule);
	    return ERROR;
	}
	++rule;
    }
    return OSortListByKeys (list, keys);
}

#if __STDC__
ERRCODE OSortListByUTime (LIST *list) {
#else
ERRCODE OSortListByUTime (list) LIST *list; {
#endif
    static const sort_keys utime_only[] = {
	{ sort_key__time, 0 },
	{ sort_key__none }
    };
    return OSortListByKeys (list, utime_only);
}

#if __STDC__
ERRCODE OSortListByUInstance (LIST *list) {
#else
ERRCODE OSortListByUInstance (list) LIST *list; {
#endif
    static const sort_keys uinstance_only[] = {
	{ sort_key__instance, 0 },
	{ sort_key__none }
    };
    return OSortListByKeys (list, uinstance_only);
}
