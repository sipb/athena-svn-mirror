/* Copyright 1994, 1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This is the client for the lert system. */

static const char rcsid[] = "$Id: lert.c,v 1.7 1999-12-09 22:24:24 danw Exp $";

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <krb.h>
#include <des.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <hesiod.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include "lert.h"

static struct timeval timeout = { LERT_TIMEOUT, 0 };
static int error_messages = TRUE;

static void bombout(int mess);

static void usage(char *pname, char *errname)
{
  fprintf(stderr,
	  "%s <%s>: usage: %s [-zephyr|-z] [-mail|-m] [-no|-n] [-quiet|-q]\n",
	  pname, errname, pname);
}

static int lert_says(char *get_lerts_blessing, int no_p)
{
  KTEXT_ST authent;
  unsigned char packet[2048];
  unsigned char ipacket[2048];
  u_long iplen;
  int biplen;
  int gotit;
  struct hostent *hp;
  struct sockaddr_in sin, lsin;
  fd_set readfds;
  int i;
  char *ip;
  char **tip;
  int tries;
  char srealm[REALM_SZ];
  char sname[SNAME_SZ];
  char sinst[INST_SZ];
  char *cp;
  Key_schedule sched;
  CREDENTIALS cred;
  int plen;
  int status;
  int s;
  u_long checksum_sent;
  u_long checksum_rcv;
  MSG_DAT msg_data;
  char hostname[256];

  /* Find out where lert lives. (Note the presumption that there is
   * only one lert!)
   */
  tip = hes_resolve(LERT_SERVER, LERT_TYPE);
  if (tip == NULL)
    {
      /* No Hesiod available. Fall back to hardcoded. */
      ip = LERT_HOME;
    }
  else
    ip = tip[0];

  /* Find out what Kerberos realm the server is in and get a ticket
   * for it.
   */
  cp = krb_realmofhost(ip);
  if (cp == NULL)
    bombout(ERR_KERB_REALM);
  strcpy(srealm, cp);

  /* Resolve hostname for service principal. */
  cp = krb_get_phost(ip);
  if (cp == NULL)
    bombout(ERR_KERB_PHOST);
  strcpy(sinst, cp);

  strcpy(sname, LERT_SERVICE);
  checksum_sent = (u_long) no_p;

  status = krb_mk_req(&authent, sname, sinst, srealm, checksum_sent);
  if (status != KSUCCESS)
    bombout(ERR_KERB_AUTH);

  /* Now do the client-server exchange. */

  /* Zero out the packets. */
  memset(packet, 0, sizeof(packet));
  memset(ipacket, 0, sizeof(ipacket));

  hp = gethostbyname(ip);
  if (hp == NULL)
    bombout(ERR_HOSTNAME);

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = hp->h_addrtype;
  memcpy(&sin.sin_addr, hp->h_addr, sizeof(sin.sin_addr));
  sin.sin_port = htons(LERT_PORT);

  /* Lert's basic protocol:
   * Client send version, one byte query code, and authentication.
   */
  packet[0] = LERT_VERSION;
  packet[1] = no_p;
  memcpy(&packet[LERT_LENGTH], &authent, sizeof(int) + authent.length);
  plen = LERT_LENGTH + sizeof(int) + authent.length;

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    bombout(ERR_SOCKET);
  if (connect(s, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0)
    bombout(ERR_CONNECT);
  tries = RETRIES;
  gotit = 0;
  while (tries > 0)
    {
      if (send(s, packet, plen, 0) < 0)
	bombout(ERR_SEND);
      FD_ZERO(&readfds);
      FD_SET(s, &readfds);
      if ((select(s+1, &readfds, NULL, NULL, &timeout) < 1)
	  || !FD_ISSET(s, &readfds))
	{
	  tries--;
	  continue;
	}

      iplen = recv(s, ipacket, 2048, 0);
      if (iplen < 0)
	bombout(ERR_RCV);

      gotit++;
      break;
    }

  /* Get my address. */
  memset(&lsin, 0, sizeof(lsin));
  i = sizeof(lsin);
  if (getsockname(s, (struct sockaddr *)&lsin, &i) < 0)
    bombout(LERT_NO_SOCK);

  gethostname(hostname, sizeof(hostname));
  hp = gethostbyname(hostname);
  memcpy(&lsin.sin_addr.s_addr, hp->h_addr, hp->h_length);

  shutdown(s, 2);
  close(s);
  if (!gotit)
    bombout(ERR_TIMEOUT);

  status = krb_get_cred(sname, sinst, srealm, &cred);
  if (status != KSUCCESS)
    bombout(ERR_KERB_CRED);
  des_key_sched(cred.session, sched);

  status = krb_rd_priv(ipacket, iplen, sched, cred.session,
		       &sin, &lsin, &msg_data);
  if (status)
    bombout(ERR_SERVER);

  if (msg_data.app_length == 0)
    return LERT_NOT_IN_DB;

  /* At this point, we have a packet.  Check it out:
   * [0] LERT_VERSION
   * [1] code response
   * [2 on] data...
   */

  if (msg_data.app_data[0] != LERT_VERSION)
    bombout(ERR_VERSION);
  if (msg_data.app_data[1] == LERT_MSG)
    {
      /* We have a message from the server. */
      memcpy(get_lerts_blessing, msg_data.app_data + LERT_CHECK,
	   msg_data.app_length - LERT_CHECK);
      return LERT_GOTCHA;
    }
  else
    {
      get_lerts_blessing[0] = '\0';
      return LERT_NOT_IN_DB;
    }
}

/* General error reporting */
static void bombout(int mess)
{
  if (error_messages)
    {
      fprintf(stderr, "lert: ");
      switch(mess)
	{
	case ERR_KERB_CRED:
	  fprintf(stderr, "Error getting kerberos credentials.\n");
	  break;
	case ERR_KERB_AUTH:
	  fprintf(stderr, "Error getting kerberos authentication.\n");
	  fprintf(stderr, "Are your tickets valid?\n");
	  break;
	case ERR_TIMEOUT:
	  fprintf(stderr, "Timed out waiting for response from server.\n");
	  break;
	case ERR_SERVER:
	  fprintf(stderr, "Unable to contact server.\n");
	  break;
	case ERR_SERVED:
	  fprintf(stderr, "Bad string from server.\n");
	  break;
	case ERR_SEND:
	  fprintf(stderr, "Error in send: %s\n", strerror(errno));
	  break;
	case ERR_RCV:
	  fprintf(stderr, "Error in recv: %s\n", strerror(errno));
	  break;
	case ERR_USER:
	  fprintf(stderr, "Could not get your name to send messages\n");
	  break;
	case NO_PROCS:
	  fprintf(stderr, "Error running child processes: %s\n",
		  strerror(errno));
	  break;
	case ERR_MEMORY:
	  fprintf(stderr, "Out of memory\n");
	  break;
	default:
	  fprintf(stderr, "A problem (%d) occurred when checking the "
		  "database\n", mess);
	  break;
	}
      fprintf(stderr, "Please try again later.\n");
      fprintf(stderr,
	      "If this problem persists, please report it to a consultant.\n");
    }
  exit(1);
}

/* Get a subject line for a lert email message.
 * (Returns malloc'ed memory.)
 */
static char *get_lert_sub(void)
{
  FILE *myfile;
  char buffer[250];
  char *turkey;
  char *mugwump;

  strcpy(buffer, LERTS_DEF_SUBJECT);

  myfile = fopen(LERTS_MSG_SUBJECT, "r");
  if (myfile != NULL)
    {
      turkey = fgets(buffer, 250, myfile);
      if (turkey == NULL)
	strcpy(buffer, LERTS_DEF_SUBJECT);
      fclose(myfile);
    }

  /* Take care of the line feed read by fgets. */
  mugwump = strchr(buffer, '\n');
  if (mugwump != NULL)
    *mugwump = '\0';

  /* Now make a buffer for the subject. */
  mugwump = malloc(strlen(buffer) + 1);
  if (mugwump == NULL)
    bombout(ERR_MEMORY);
  strcpy(mugwump, buffer);
  return mugwump;
}

/* View the message from lert. Currently supports cat, zephyr,
 * and email messaging.
 */
static void view_message(char *message, int type)
{
  char *whoami;
  char *ptr;
  char *tail;
  char *type_buffer;
  char buffer[512];
  struct passwd *pw;
  int status;

  /* These buffers permit centralized handling of the various options.
   * Note that both z and m require on the fly construction (at least
   * user name) so buffers are kept roomy...
   */
  char zprog[128];
  char mprog[512];
  char cprog[4];

  strcpy(zprog, "zwrite -q ");
  strcpy(mprog, "mhmail ");
  strcpy(cprog, "cat");

  whoami = getenv("USER");
  if (!whoami)
    whoami = getlogin();
  if (!whoami)
    {
      pw = getpwuid(getuid());
      if (pw)
	whoami = pw->pw_name;
      else
	bombout(ERR_USER);
    }

  ptr = message;
  while (*ptr)
    {
    switch (type)
      {
      case LERT_Z:
	/* zwrite -q whoami */
	type_buffer = zprog;
	strcat(type_buffer, whoami);
	type = LERT_HANDLE;
	break;

      case LERT_MAIL:
	/* cat ... | mhmail whoami -subject "lert's msg" */
	type_buffer = mprog;
	strcat(type_buffer, whoami);
	strcat(type_buffer, " -subject \"");
	strcat(type_buffer, get_lert_sub());
	strcat(type_buffer, "\" ");
	type = LERT_HANDLE;
	break;

      case LERT_HANDLE:
	/* Take care of the case where there is one lonely message.
	 * Just combine the two (lert0 and lertx) for the zwrite
	 * hope the writer of the messages realizes they will be
	 * jammed together...
	 */

	if (strlen(ptr) == 1)
	  {
	    strcpy(buffer, "cat ");
	    strcat(buffer, LERTS_MSG_FILES);
	    strcat(buffer, "0 ");
	    strcat(buffer, LERTS_MSG_FILES);
	    if (isalnum(*ptr))
	      strcat(buffer, ptr);
	    else
	      bombout(ERR_SERVED);
	    strcat(buffer, " | ");
	    strcat(buffer, type_buffer);
	    status = system(buffer);
	    if (status)
	      {
		/* If zwrite or email failed, try cat. */
		if (type_buffer != cprog)
		  type = LERT_CAT;
		else
		  bombout(NO_PROCS);
		break;
	      }
	    ptr++;
	  }
	else
	  {
	    /* Do multiple message notification. */
	    strcpy(buffer, type_buffer);
	    strcat(buffer, " < ");
	    strcat(buffer, LERTS_MSG_FILES);

	    tail = buffer + strlen(buffer);
	    *(tail + 1) = '\0';

	    /* Always start with the lert0. */
	    *tail = '0';
	    status = system(buffer);
	    if (status)
	      {
		/* If zwrite or emai failed, try cat. */
		if (type_buffer != cprog)
		  type = LERT_CAT;
		else
		  bombout(NO_PROCS);
		break;
	      }

	    /* Now step through the rest of the messages for this user. */
	    while (*ptr)
	      {
		if (isalnum(*ptr))
		  *tail = *ptr;
		else
		  bombout(ERR_SERVED);
		status = system(buffer);
		if (status)
		  bombout(NO_PROCS);
		ptr++;
	      }
	  }
	break;

      case LERT_CAT:
      default:
	/* Cat notification. */
	type_buffer = cprog;
	type = LERT_HANDLE;
	break;

      }
    }
}

int main(int argc, char **argv)
{
  char buffer[512];
  char *get_lerts_blessing;
  char **xargv = argv;
  int xargc = argc;
  int method = LERT_CAT, no_p = FALSE;
  int result;

  get_lerts_blessing = buffer;

  /* Argument Processing:
   * 	-z or -zephyr: send the message as a zephyrgram.
   *    -m or -mail: send the message as email
   * 	-n or -no: no more messages!
   */

  if (argc > 3)
    {
      usage(argv[0], "too many arguments");
      exit(1);
    }

  /* Note: xargc/xargv initialized in declaration. */
  while (--xargc)
    {
      xargv++;
      if (!strcmp(xargv[0],"-zephyr") || !strcmp(xargv[0],"-z"))
	method = LERT_Z;
      else if (!strcmp(xargv[0],"-mail") || !strcmp(xargv[0],"-m"))
	method = LERT_MAIL;
      else if (!strcmp(xargv[0],"-no") || !strcmp(xargv[0],"-n"))
	no_p = TRUE;
      else if (!strcmp(xargv[0],"-quiet") || !strcmp(xargv[0],"-q"))
	error_messages = FALSE;
      else
	{
	  usage(argv[0], xargv[0]);
	  exit(1);
	}
    }

  /* Get the server's string for this user. */
  result = lert_says(get_lerts_blessing, no_p);

  if (result == LERT_GOTCHA)
    view_message(get_lerts_blessing, method);

  return 0;
}





