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
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/sort.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/sort.c,v 1.3 1989-11-17 14:52:06 tjcoppet Exp $";
#endif

#include <olc/olc.h>

int OCompareListByTopic();
int OCompareListByNseen();
int OCompareListByUTime();
int OCompareListByUUsername();
int OCompareListByCUsername();
int OCompareListByUInstance();
int OCompareListByCInstance();
int OCompareListByUKStatus();
int OCompareListByUStatus();
int OCompareListByCKStatus();
int OCompareListByCStatus();

ERRCODE
OSortListByRule(list,rule)
     LIST *list;
     char **rule;
{
  if(list == (LIST *) NULL)
    return(ERROR);
  if(rule == (char **) NULL)
    return(ERROR);
  if(list->ustatus == END_OF_LIST)
    return(SUCCESS);

  while((*rule != (char *) NULL) && (*rule[0] != '\0'))
    {
      if(string_eq(*rule,"topic"))
	OSortListByTopic(list);
	
      if(string_eq(*rule,"nseen"))
	OSortListByNseen(list);
	
      if(string_eq(*rule,"uusername") || string_eq(*rule,"username"))
	OSortListByUUsername(list);
	
      if(string_eq(*rule,"cusername"))
	OSortListByCUsername(list);
      
      if(string_eq(*rule,"uinstance") || string_eq(*rule,"instance"))
	OSortListByUInstance(list);
	
      if(string_eq(*rule,"cinstance"))
	OSortListByCInstance(list);
	
      if(string_eq(*rule,"ukstatus") || string_eq(*rule,"status"))
	OSortListByUKStatus(list);
	
      if(string_eq(*rule,"ustatus"))
	OSortListByUStatus(list);
	
      if(string_eq(*rule,"cstatus"))
	OSortListByCStatus(list);
	
      if(string_eq(*rule,"ckstatus"))
	OSortListByCKStatus(list);
	
      if(string_eq(*rule,"utime"))
        OSortListByUTime(list);
      ++rule;
      if(rule == (char **) NULL)
	return(ERROR);
    }
  return(SUCCESS);
}


ERRCODE
OSortListByTopic(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByTopic));
}

ERRCODE
OSortListByNseen(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByNseen));
}

ERRCODE
OSortListByUTime(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByUTime));
}

ERRCODE
OSortListByUUsername(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByUUsername));
}

ERRCODE
OSortListByCUsername(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByCUsername));
}

ERRCODE
OSortListByUInstance(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByUInstance));
}

ERRCODE
OSortListByCInstance(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByCInstance));
}

ERRCODE
OSortListByUKStatus(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByUKStatus));
}

ERRCODE
OSortListByUStatus(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByUStatus));
}

ERRCODE
OSortListByCKStatus(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByCKStatus));
}

ERRCODE
OSortListByCStatus(list)
     LIST *list;
{
  return(OSortList(list,OCompareListByCStatus));
}

ERRCODE
OSortList(list,proc)
     LIST *list;
     int (*proc)();
{
  LIST *bd;
  int index = 0;

  if(list == (LIST *) NULL)
    return(ERROR);

  for(bd = list; bd->ustatus != END_OF_LIST; ++bd)
    ++index;

  qsort((char *) list, index, sizeof(LIST), proc);
  list[index].ustatus = END_OF_LIST;
  return(SUCCESS);
}

int 
OCompareListByTopic(a,b)
     LIST *a, *b;
{
  return(OCompareStrings(a->topic, b->topic));
}

int 
OCompareListByNseen(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->nseen, b->nseen));
}

int
OCompareListByUTime(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->utime, b->utime));
}

int 
OCompareListByUUsername(a,b)
     LIST *a, *b;
{
  return(OCompareStrings(a->user.username, b->user.username));
}

int 
OCompareListByCUsername(a,b)
     LIST *a, *b;
{
  return(OCompareStrings(a->connected.username, b->connected.username));
}

int 
OCompareListByUInstance(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->user.instance, b->user.instance));
}

int 
OCompareListByCInstance(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->connected.instance, b->connected.instance));
}

int 
OCompareListByUKStatus(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->ukstatus, b->ukstatus));
}

int 
OCompareListByUStatus(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->ustatus, b->ustatus));
}

int 
OCompareListByCKStatus(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->ckstatus, b->ckstatus));
}

int 
OCompareListByCStatus(a,b)
     LIST *a, *b;
{
  return(OCompareInts(a->ustatus, b->ustatus));
}

int 
OCompareStrings(a,b)
     char *a, *b;
{
  while(*a == *b) 
    {
      if(*a++ == '\0')
	return(0);
      b++;
    }

  if(*a < *b)
    return(-1);
  else
    if(*a > *b)
      return(1);
    else
      if(*a == '\0')
	return(-1);

  return(1);
}


int 
OCompareInts(a,b)
     int a, b;
{
  if(a < b)
    return(-1);
  else
    if(a > b)
      return(1);
  
  return(0);
}


  
