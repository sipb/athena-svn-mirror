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
 * Modified:    08/20/90	Tom Coppeto
 *		Added HINFO and MX queries.
 *              List all returned addresses
 *              Allow for multiple hostnames.
 *			
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/hostinfo/hostinfo.c,v 1.9 1994-04-12 15:08:43 miki Exp $";
#endif

#include <stdio.h>			/* Standard IO */
#include <sys/types.h>			/* System type defs. */
#include <netdb.h>			/* Networking defs. */
#include <sys/socket.h>			/* More network defs. */
#include <strings.h>			/* String fct. declarations. */
#include <ctype.h>			/* Character type macros. */
#include <netinet/in.h>			/* Internet defs. */
#include <arpa/inet.h>			/* For inet_addr */

typedef int bool;

#define TRUE		1
#define FALSE		0
#define LINE_LENGTH	128
#define ERROR		-1
#define CPNULL		((char *) NULL)

extern char *gethinfobyname(), *getmxbyname();

static char *usage[] = {
  "Usage: %s <options> <host-names-or-addresses>",
  "   -h: output only hostname",
  "   -a: output only internet address",
  "   -i: output only host info record",
  "   -m: output only mx record",
  "   -q: disable additional query for hinfo & mx",
  "   -ns <server>: ask specified name server",
  CPNULL,
};

char *myname;				/* How program was invoked. */
char *hostname = NULL;                  /* desired host */

struct in_addr server_addr;
int server_specified;

bool h_flag = FALSE;
bool a_flag = FALSE;
bool n_flag = FALSE;
bool m_flag = FALSE;
bool i_flag = FALSE;
bool q_flag = FALSE;

static void usage_error();
static void clear_flags();

main(argc, argv)
     int argc;
     char **argv;
     
{
  extern int h_errno;
  struct hostent *host_entry = (struct hostent *)0;	
  unsigned long host_address;
  char *server = NULL;
  char *hinfo = NULL;
  char *mx = NULL;
  int status = 0;
  int set_server = 0;

  myname = (myname = rindex(argv[0], '/')) ? myname + 1 : argv[0];
  if(argc == 1)
    usage_error();
  if(strcmp(argv[1], "-help") == 0)
    usage_error();

  while(*++argv)
    {
      if(hostname && !(a_flag || m_flag || i_flag || h_flag))
	putchar('\n');

      set_server = 0;
      if(**argv == '-')
	switch((*argv)[1])
	  {
	  case 'h':
	    clear_flags();
	    h_flag = TRUE;
	    continue;
	  case 'a':
	    clear_flags();
	    a_flag = TRUE;
	    continue;
	  case 'm':
	    clear_flags();
	    m_flag = TRUE;
	    continue;
	  case 'i':
	    clear_flags();
	    i_flag = TRUE;
	    continue;
	  case 'q':
	    clear_flags();
	    q_flag = TRUE;
	    continue;
	  case 'n':
	    if(!*++argv)
	      usage_error();
	    server = *argv;
	    set_server = 1;
	    break;
	  case '?':
	  default:
	    usage_error();
	    break;
	  }
      
      hinfo = NULL;
      mx = NULL;
      host_address = 0;
      host_entry = NULL;
      hostname = *argv;
      
      if (*hostname >= '0' && *hostname <= '9')
	{
	  host_address = inet_addr(hostname);
	  if (host_address)
	    {
	      host_entry = gethostbyaddr((char *) &host_address, 4, AF_INET);
	      if(host_entry)
		if(set_server)		  
		  bcopy(host_entry->h_addr, &server_addr, sizeof(server_addr));
		else		  
		  print_host(host_entry);
	    }
	}

#ifdef SOLARIS
      if(!host_entry)
#else
      if(host_entry == (struct hostent *)0)
#endif
	{
	  host_entry = gethostbyname(hostname);
	  if(host_entry)
	    {
	      if(set_server)
		bcopy(host_entry->h_addr, &server_addr, sizeof(server_addr));
	      else
		{
		  print_host(host_entry);
		  if(!q_flag)
		    {
		      hinfo = (char *) gethinfobyname(hostname);
		      if(hinfo)
			{
			  if(!h_flag && !a_flag && !m_flag)
			    {
			      int i = (int) hinfo[0];
			      hinfo[i+1] = '/';
			      if(i_flag)
				printf("%s\n", hinfo+1);
			      else
				printf("Host info:\t%s\n", hinfo+1); 
			    }
			}
		      if(!h_flag && !a_flag && !i_flag)
			if(mx = (char *) getmxbyname(hostname))
			  if(m_flag)
			    printf("%s\n", mx); 
			  else
			    printf("MX address:\t%s\n", mx); 		  
		    }
		}
	    }
	}
  
      if(host_entry && set_server)
	{
	  struct in_addr internet_address;

	  server_specified = 1;
	  bcopy(host_entry->h_addr_list[0], &internet_address, 
		host_entry->h_length);
	  printf("Using domain server:\nHost name:\t%s\nHost Address\t%s\n",
		 host_entry->h_name, inet_ntoa(internet_address));
	  continue;
	}

      if (!host_entry)
	{
	  switch (h_errno) 
	    {
#ifdef ultrix
	      /* it can return NULL, h_errno == 0 in some cases when
		 there is no name */
	    case 0:
#endif
	    case HOST_NOT_FOUND:
	      printf("No such host '%s'.\n",hostname);
	      status = ERROR;
	      continue;
	      break;
	    default:
	    case TRY_AGAIN:
	    case NO_RECOVERY:
	      printf("Cannot resolve name '%s' due to network difficulties.\n",
		     hostname);
	      status = ERROR;
	      continue;
	      break;
	    case NO_ADDRESS:
	    /* should look up MX record? */
	      /* this error return appears if there is some entry for the
		 requested name, but no A record (e.g. only an NS record) */
	      printf("No address for '%s'.\n", hostname);
	      status = ERROR;
	      continue;
	      break;
	    }
	}
    }
  
  exit(status);
} /* main */


print_host(h)
     struct hostent *h;
{
  struct in_addr internet_address;
  char **alias;				   
  char **addr;

  if(m_flag || i_flag)
    return;

  if (h_flag) 
    printf("%s\n", h->h_name);
  else  
    {
      if(!a_flag)
	{
	  printf("Desired host:\t%s\n", hostname);
	  printf("Official name:\t%s\n", h->h_name);
	  for (alias = h->h_aliases; *alias; alias++)
	    printf("Alias:\t\t%s\n", *alias);
	}
	  
      for (addr = h->h_addr_list; *addr; addr++) 
	{
	  bcopy(*addr, &internet_address, h->h_length);
	  if(a_flag)
	    printf("%s\n", inet_ntoa(internet_address));
	  else
	    printf("Host address:\t%s\n", inet_ntoa(internet_address));
	}
    }
}


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


static void
clear_flags()
{
  i_flag = m_flag = h_flag = a_flag = 0;
}
