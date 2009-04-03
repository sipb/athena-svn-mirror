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

static const char rcsid[] = "$Id: lert.c,v 1.12 2002-09-01 03:31:25 zacheiss Exp $";

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

#include <hesiod.h>
#include <krb5.h>

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
  krb5_context context = NULL;
  krb5_auth_context auth_con = NULL;
  krb5_ccache ccache = NULL;
  krb5_data auth, packet, msg_data;
  krb5_error_code problem = 0;
  void *hes_context = NULL;
  char *lert_host, *message;
  struct hostent *hp;
  struct servent *sp;
  struct sockaddr_in sin;
  int plen, packetsize, s, tries, gotit, nread;
  unsigned char *pktbuf;
  fd_set readfds;

  memset(&auth, 0, sizeof(krb5_data));
  memset(&packet, 0, sizeof(krb5_data));
  memset(&msg_data, 0, sizeof(krb5_data));

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

  problem = krb5_init_context(&context);
  if (problem)
    goto out;

  problem = krb5_auth_con_init(context, &auth_con);
  if (problem)
    goto out;

  problem = krb5_cc_default(context, &ccache);
  if (problem)
    goto out;

  problem = krb5_mk_req(context, &auth_con, 0, LERT_SERVICE, lert_host,
			NULL, ccache, &auth);
  if (problem)
    goto out;

  /* Lert's basic protocol:
   * Client sends version, one byte query code, and authentication.
   */
  plen = LERT_LENGTH + sizeof(int) + auth.length;
  pktbuf = malloc(plen);
  if (!pktbuf)
    bombout(ERR_MEMORY);
  pktbuf[0] = LERT_VERSION;
  pktbuf[1] = no_more;
  memcpy(pktbuf + LERT_LENGTH, auth.data, auth.length);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    bombout(ERR_SOCKET);
  if (connect(s, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0)
    bombout(ERR_CONNECT);

  gotit = 0;
  for (tries = 0; tries < RETRIES && !gotit; tries++)
    {
      if (send(s, pktbuf, plen, 0) < 0)
	bombout(ERR_SEND);
      FD_ZERO(&readfds);
      FD_SET(s, &readfds);
      gotit = select(s + 1, &readfds, NULL, NULL, &timeout) == 1;
    }
  free(pktbuf);
  if (tries == RETRIES)
    bombout(ERR_TIMEOUT);

  /* Read the response. */
  packetsize = 2048;
  pktbuf = malloc(packetsize);
  if (!pktbuf)
    bombout(ERR_MEMORY);
  plen = 0;
  while (1)
    {
      nread = recv(s, pktbuf + plen, packetsize - plen, 0);
      if (nread < 0)
	bombout(ERR_RCV);
      plen += nread;

      if (nread < packetsize - plen)
	break;

      packetsize *= 2;
      pktbuf = realloc(pktbuf, packetsize);
      if (!pktbuf)
	bombout(ERR_MEMORY);
    }

  packet.data = pktbuf;
  packet.length = nread;

  problem = krb5_auth_con_genaddrs(context, auth_con, s,
				   KRB5_AUTH_CONTEXT_GENERATE_REMOTE_ADDR |
				   KRB5_AUTH_CONTEXT_GENERATE_LOCAL_ADDR);
  if (problem)
    goto out;

  /* Now close the socket. */
  shutdown(s, 2);
  close(s);

  /* Clear auth_context flags so we don't need to manually set up a 
   * replay cache.
   */
  problem = krb5_auth_con_setflags(context, auth_con, 0);
  if (problem)
    goto out;

  problem = krb5_rd_priv(context, auth_con, &packet, &msg_data, NULL);
  if (problem)
    goto out;

  if (msg_data.length == 0)
    return NULL;

  /* At this point, we have a packet.  Check it out:
   * [0] LERT_VERSION
   * [1] code response
   * [2 on] data...
   */

  if (msg_data.data[0] != LERT_VERSION)
    bombout(ERR_VERSION);

  if (msg_data.data[1] == LERT_MSG)
    {
      /* We have a message from the server. */
      message = malloc(msg_data.length - LERT_CHECK + 1);
      if (!message)
	bombout(ERR_MEMORY);
      memcpy(message, msg_data.data + LERT_CHECK,
	   msg_data.length - LERT_CHECK);
      message[msg_data.length - LERT_CHECK] = '\0';
    }
  else
    message = NULL;
  free(pktbuf);

  return message;

 out:
  /* krb5 library checks if this is allocated so we don't have to. */
  krb5_free_data_contents(context, &auth);
  krb5_free_data_contents(context, &msg_data);

  if (ccache)
    krb5_cc_close(context, ccache);

  if (auth_con)
    krb5_auth_con_free(context, auth_con);

  bombout(problem);
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
	  fprintf(stderr, "%s while accessing the database\n",
		  error_message(mess));
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
      user = getenv("ATHENA_USER");
      if(!user)
        user = getenv("USER");

      if(!user)
        user = getlogin();

      if(!user)
        {
          struct passwd *pw;
          pw = getpwuid(getuid());
          if (pw)
            user = pw->pw_name;
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
