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

static const char rcsid[] = "$Id: lert.c,v 1.10 2000-06-19 17:41:48 zacheiss Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <des.h>
#include <hesiod.h>
#include <krb.h>

#include "lert.h"

#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

static struct timeval timeout = { LERT_TIMEOUT, 0 };
static int error_messages = TRUE;

static void bombout(int mess);

static void usage(char *pname)
{
  fprintf(stderr,
	  "Usage: %s [-zephyr|-z] [-mail|-m] [-no|-n] [-quiet|-q] [-server|-s server]\n", pname);
}

static char *lert_says(int no_more, char *server)
{
  void *hes_context = NULL;
  char *lert_host, *message;
  struct hostent *hp;
  struct servent *sp;
  struct sockaddr_in sin, lsin;
  char *cp, *srealm, *sinst;
  KTEXT_ST authent;
  CREDENTIALS cred;
  Key_schedule sched;
  int plen, packetsize, s, i, tries, status, gotit;
  unsigned char *packet;
  fd_set readfds;
  MSG_DAT msg_data;

  /* Find out where lert lives. (Note the presumption that there is
   * only one lert!)
   */
  lert_host = NULL;
  if (server)
    lert_host = server;
  else if (hesiod_init(&hes_context) == 0)
    {
      char **slocs = hesiod_resolve(hes_context, LERT_SERVER, LERT_TYPE);
      if (slocs)
	{
	  lert_host = strdup(slocs[0]);
	  if (!lert_host)
	    bombout(ERR_MEMORY);
	  hesiod_free_list(hes_context, slocs);
	}
    }
  if (!lert_host)
    {
      lert_host = strdup(LERT_HOME);
      if (!lert_host)
	bombout(ERR_MEMORY);
    }

  hp = gethostbyname(lert_host);
  if (hp == NULL)
    bombout(ERR_HOSTNAME);

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = hp->h_addrtype;
  memcpy(&sin.sin_addr, hp->h_addr, sizeof(sin.sin_addr));

  /* Find out what port lert is on. */
  sin.sin_port = htons(LERT_PORT);
  sp = getservbyname(LERT_SERVED, LERT_PROTO);
  if (sp)
    sin.sin_port = sp->s_port;
  else if (hes_context)
    {
      sp = hesiod_getservbyname(hes_context, LERT_SERVED, LERT_PROTO);
      sin.sin_port = sp->s_port;
      hesiod_free_servent(hes_context, sp);
    }
    
  if (hes_context)
    hesiod_end(hes_context);

  /* Find out what Kerberos realm the server is in and get a ticket
   * for it.
   */
  cp = krb_realmofhost(lert_host);
  if (cp == NULL)
    bombout(ERR_KERB_REALM);
  srealm = strdup(cp);
  if (!srealm)
    bombout(ERR_MEMORY);

  /* Resolve hostname for service principal. */
  cp = krb_get_phost(lert_host);
  if (cp == NULL)
    bombout(ERR_KERB_PHOST);
  sinst = strdup(cp);
  if (!sinst)
    bombout(ERR_MEMORY);

  status = krb_mk_req(&authent, LERT_SERVICE, sinst, srealm, no_more);
  if (status != KSUCCESS)
    bombout(ERR_KERB_AUTH);

  /* Get the session key now... we'll need it later. */
  status = krb_get_cred(LERT_SERVICE, sinst, srealm, &cred);
  if (status != KSUCCESS)
    bombout(ERR_KERB_CRED);
  des_key_sched(cred.session, sched);

  free(srealm);
  free(sinst);

  /* Lert's basic protocol:
   * Client sends version, one byte query code, and authentication.
   */
  plen = LERT_LENGTH + sizeof(int) + authent.length;
  packet = malloc(plen);
  if (!packet)
    bombout(ERR_MEMORY);
  packet[0] = LERT_VERSION;
  packet[1] = no_more;
  memcpy(packet + LERT_LENGTH, &authent, sizeof(int) + authent.length);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    bombout(ERR_SOCKET);
  if (connect(s, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0)
    bombout(ERR_CONNECT);
  /* Get my address, for krb_rd_priv. */
  memset(&lsin, 0, sizeof(lsin));
  i = sizeof(lsin);
  if (getsockname(s, (struct sockaddr *)&lsin, &i) < 0)
    bombout(LERT_NO_SOCK);

  gotit = 0;
  for (tries = 0; tries < RETRIES && !gotit; tries++)
    {
      if (send(s, packet, plen, 0) < 0)
	bombout(ERR_SEND);
      FD_ZERO(&readfds);
      FD_SET(s, &readfds);
      gotit = select(s + 1, &readfds, NULL, NULL, &timeout) == 1;
    }
  free(packet);
  if (tries == 0)
    bombout(ERR_TIMEOUT);

  /* Read the response. */
  packetsize = 2048;
  packet = malloc(packetsize);
  if (!packet)
    bombout(ERR_MEMORY);
  plen = 0;
  while (1)
    {
      int nread;

      nread = recv(s, packet + plen, packetsize - plen, 0);
      if (nread < 0)
	bombout(ERR_RCV);
      plen += nread;

      if (nread < packetsize - plen)
	break;

      packetsize *= 2;
      packet = realloc(packet, packetsize);
      if (!packet)
	bombout(ERR_MEMORY);
    }

  /* Now close the socket. */
  shutdown(s, 2);
  close(s);

  status = krb_rd_priv(packet, plen, sched, cred.session,
		       &sin, &lsin, &msg_data);
  if (status)
    bombout(ERR_SERVER);

  if (msg_data.app_length == 0)
    return NULL;

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
      message = malloc(msg_data.app_length - LERT_CHECK + 1);
      if (!message)
	bombout(ERR_MEMORY);
      memcpy(message, msg_data.app_data + LERT_CHECK,
	   msg_data.app_length - LERT_CHECK);
      message[msg_data.app_length - LERT_CHECK] = '\0';
    }
  else
    message = NULL;
  free(packet);

  return message;
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
	case ERR_FILE:
	  fprintf(stderr, "Could not read message file\n");
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

/* Send a message from lert via zephyr. */
static void zephyr_message(char *user, char *header, char *message)
{
  FILE *p;
  char *cmd;

  cmd = malloc(11 + strlen(user));
  if (!cmd)
    bombout(ERR_MEMORY);
  sprintf(cmd, "zwrite -q %s", user);

  p = popen(cmd, "w");
  if (!p)
    bombout(NO_PROCS);
  fputs("@bold{", p);
  fputs(header, p);
  fputs("}", p);
  fputs(message, p);
  pclose(p);
}

/* Send a message from lert via mail. */
static void mail_message(char *user, char *subject,
			 char *header, char *message)
{
  FILE *p;
  char *cmd;

  cmd = malloc(20 + strlen(subject) + strlen(user));
  if (!cmd)
    bombout(ERR_MEMORY);
  sprintf(cmd, "mhmail %s -subject \"%s\"", user, subject);

  p = popen(cmd, "w");
  if (!p)
    bombout(NO_PROCS);
  fputs(header, p);
  fputs(message, p);
  pclose(p);
}

/* Print a message from lert to stdout. */
static void cat_message(char *header, char *message)
{
  fputs(header, stdout);
  fputs(message, stdout);
}	  

/* Read a file from the lert directory. */
static char *read_file(char *name)
{
  struct stat st;
  int fd;
  char *buf;

  fd = open(name, O_RDONLY);
  if (fd < 0 || fstat(fd, &st) < 0)
    bombout(ERR_FILE);

  buf = malloc(st.st_size + 1);
  if (!buf)
    bombout(ERR_MEMORY);

  if (read(fd, buf, st.st_size) < st.st_size)
    bombout(ERR_FILE);
  buf[st.st_size] = '\0';
  close(fd);

  return buf;
}

/* View the message from lert. */
static void view_message(char *msgs, int type, int no_more)
{
  char name[sizeof(LERTS_MSG_FILES) + 2];
  char *user, *subject = NULL;
  char *header, *body = NULL;

  if (type == LERT_Z || type == LERT_MAIL)
    {
      struct passwd *pw;

      pw = getpwuid(getuid());
      if (pw)
	user = pw->pw_name;
      else
	{
	  user = getenv("USER");
	  if (!user)
	    user = getlogin();
	}
      if (!user)
	type = LERT_CAT;
    }

  if (type == LERT_MAIL)
    {
      subject = read_file(LERTS_MSG_SUBJECT);
      if (!subject)
	subject = strdup(LERTS_DEF_SUBJECT);
      if (!subject)
	bombout(ERR_MEMORY);
    }

  header = read_file(no_more ? LERTS_MSG_FILES "1" : LERTS_MSG_FILES "0");

  while (*msgs)
    {
      sprintf(name, "%s%c", LERTS_MSG_FILES, *msgs);
      body = read_file(name);
      switch (type)
	{
	case LERT_Z:
	  zephyr_message(user, header, body);
	  break;

	case LERT_MAIL:
	  mail_message(user, subject, header, body);
	  break;

	default:
	  cat_message(header, body);
	  break;
	}

      free(body);
      msgs++;
    }

  free(header);
  free(subject);
}

int main(int argc, char **argv)
{
  char *whoami, *message, *server = NULL;
  int method = LERT_CAT, no_more = FALSE;
  char **arg = argv;

  whoami = strrchr(argv[0], '/');
  if (whoami)
    whoami++;
  else
    whoami = argv[0];

  /* Argument Processing:
   * 	-z or -zephyr: send the message as a zephyrgram.
   *    -m or -mail: send the message as email
   * 	-n or -no: no more messages!
   */

  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (argis("zephyr", "z"))
	    method = LERT_Z;
	  else if (argis("mail","m"))
	    method = LERT_MAIL;
	  else if (argis("no", "n"))
	    no_more = TRUE;
	  else if (argis("quiet", "q"))
	    error_messages = FALSE;
	  else if (argis("server", "s"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  server = *arg;
		}
	      else
		usage(whoami);
	    }
	  else
	    {
	      usage(whoami);
	      exit(1);
	    }
	}
    }
  /* Get the server's string for this user. */
  message = lert_says(no_more, server);
  if (message)
    {
      view_message(message, method, no_more);
      free(message);
    }

  return 0;
}
