/* This program looks up a given hostname on the nameserver and prints
 * its Internet address, official name, etc.
 *
 * Win Treese
 * MIT Project Athena
 *
 * Copyright (c) 1986 by the Massachusetts Institute of Technology.
 *
 * Created:	08/29/86
 *
 * Modified:	02/03/87	Doug Alan
 * 		Let input also be in form of an internet address.
 *
 * Modified:	02/04/87	Doug Alan
 *		Take flags to control output.
 *			
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/hostinfo/hostinfo.c,v 1.1 1989-10-25 15:21:07 probe Exp $";
#endif

#include <stdio.h>			/* Standard IO */
#include <sys/types.h>			/* System type defs. */
#include <netdb.h>			/* Networking defs. */
#include <sys/socket.h>			/* More network defs. */
#include <strings.h>			/* String fct. declarations. */
#include <ctype.h>			/* Character type macros. */
#include <netinet/in.h>			/* Internet defs. */

typedef int bool;

#define TRUE		1
#define FALSE		0
#define LINE_LENGTH	128
#define ERROR		-1
#define CPNULL		((char *) NULL)

static char *usage[] = {
  "Usage: %s <host-name-or-address>",
  "   -h: output only hostname",
  "   -a: output only internet address",
  "   Both flags must not be set",
  CPNULL,
};

char *myname;				/* How program was invoked. */

static void usage_error();

main(argc, argv)
     int argc;
     char *argv[];
     
{
  extern int getopt();
  extern int optind;

  char hostname[LINE_LENGTH];	           /* Name of desired host. */
  struct hostent *host_entry = NULL;	   /* Host entry of it. */
  struct in_addr internet_address;
  char **alias;				   /* Ptr. to aliases. */
  bool h_flag = FALSE;
  bool a_flag = FALSE;
  int flagc;

  myname = argv[0];

  while ((flagc = getopt(argc, argv, "ha")) != EOF)
    switch ((char) flagc) {
    case 'h':
      if (a_flag) usage_error();
      else h_flag = TRUE;
      break;
    case 'a':
      if (h_flag) usage_error();
      else a_flag = TRUE;
      break;
    case '?':
    default:
      usage_error();
      break;
    }
  if (argc -1 != optind) usage_error();
  (void) strcpy(hostname, argv[optind]);
  if (*hostname >= '0' && *hostname <= '9')
    {
      unsigned long host_address = inet_addr(hostname);
      if (host_address == -1) goto oops;
      host_entry = gethostbyaddr((char *) &host_address, 4, AF_INET);
    }
  else
    {
    oops:
      host_entry = gethostbyname(hostname);
    }
  if (host_entry == NULL)
    {
      printf("No such host.\n");
      exit(ERROR);
    }
  bcopy(host_entry->h_addr, &internet_address, host_entry->h_length);
  if (h_flag) printf("%s\n", host_entry->h_name);
  else if (a_flag) printf("%s\n", inet_ntoa(internet_address));
  else
    {
      printf("Desired host:\t%s\n", hostname);
      printf("Official name:\t%s\n", host_entry->h_name);
      for (alias = host_entry->h_aliases; *alias; alias++)
	printf("Alias:\t\t%s\n", *alias);
      printf("Host address:\t%s\n", inet_ntoa(internet_address));
    }
} /* main */

static void
usage_error()
{
  int line = 0;				/* current line */
  while (usage[line] != CPNULL)
    {
      fprintf (stderr, usage[line++], myname);
      putc('\n', stderr);
    }
  exit(ERROR);
} /* usage_error */
