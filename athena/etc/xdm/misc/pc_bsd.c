#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <bstring.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <errno.h>
#include "pc_bsd.h"

/*
 * Porting warning: make sure that the chmod calls on the socket
 * actually do something useful for access control before deciding
 * this library works. Under Solaris 2.3, chmod/chown are useless,
 * as apparently the BSD emulation doesn't deal with it. (My plan
 * was to get this library done with streams for SYSV in general.)
 */

long pc_init(pc_state **ps)
{
  pc_state *ret;

  initialize_pc_error_table();

  ret = malloc(sizeof(pc_state));
  if (ret == NULL)
    return PCerrNoMem;

  ret->numports = 0;

  ret->nfds = 0;
  FD_ZERO(&(ret->readfds));
  FD_ZERO(&(ret->writefds));

  *ps = ret;
  return 0L;
}

long pc_destroy(pc_state *ps)
{
  free(ps);
  return 0L;
}

long pc_freemessage(pc_message *m)
{
  if (m == NULL)
    return 0L;

  if (m->type == PC_DATA)
    free(m->data);

  free(m);
  return 0L;
}

/*
 * Add a port to a state structure.
 */
long pc_addport(pc_state *s, pc_port *p)
{
  if (s == NULL || p == NULL)
    return PCerrNullArg;

  if (s->numports == PC_MAXPORTS)
    return PCerrMaxPortsUsed;

  s->ports[s->numports++] = p;

  FD_SET(p->fd, &(s->readfds));
  s->nfds = MAX(s->nfds, p->fd + 1);

  return 0L;
}

long pc_removeport(pc_state *s, pc_port *p)
{
  int i;

  if (s == NULL || p == NULL)
    return PCerrNullArg;

  for (i = 0; i < s->numports; i++)
    if (s->ports[i] == p)
      {
	s->numports--;
	for (; i < s->numports; i++)
	  s->ports[i] = s->ports[i+1];
	FD_CLR(p->fd, &(s->readfds)); /* should adjust nfds someday */
	return 0L;
      }

  return PCerrNoSuchPort;
}

long pc_addfd(pc_state *s, pc_fd *fd)
{
  if (s == NULL || fd == NULL)
    return PCerrNullArg;

  if (fd->events & PC_READ)
    FD_SET(fd->fd, &(s->readfds));
 
  if (fd->events & PC_WRITE)
    FD_SET(fd->fd, &(s->writefds));

  if (fd->events & (PC_READ | PC_WRITE))
    s->nfds = MAX(s->nfds, fd->fd + 1);

  return 0L;
}

long pc_removefd(pc_state *s, pc_fd *fd)
{
  if (s == NULL || fd == NULL)
    return PCerrNullArg;

  if (fd->events & PC_READ)
    FD_CLR(fd->fd, &(s->readfds));
 
  if (fd->events & PC_WRITE)
    FD_CLR(fd->fd, &(s->writefds));

  return 0L;
}

/* Should probably make it fail if chown or chmod fail. */
long pc_makeport(pc_port **pp, char *name, int flags,
		 uid_t owner, gid_t group, mode_t mode)
{
  pc_port *p;
  struct sockaddr_un n;

  if (pp == NULL || name == NULL)
    return PCerrNullArg;

  p = malloc(sizeof(pc_port));
  if (p == NULL)
    return PCerrNoMem;

  p->parent = NULL;
  p->type = PC_LISTENER;
  p->path = malloc(strlen(name)+1);
  if (p->path == NULL)
    {
      free(p);
      return PCerrNoMem;
    }
  strcpy(p->path, name);

  p->fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (p->fd == -1)
    {
      free(p->path);
      free(p);
      return PCerrSocketFailed;
    }

  if (flags & PC_GRAB)
    unlink(p->path);
  n.sun_family = PF_UNIX;
  strcpy(n.sun_path, p->path);
  if (bind(p->fd, &n, sizeof(struct sockaddr_un)) == -1)
    {
      close(p->fd);
      free(p->path);
      free(p);
      return PCerrBindFailed;
    }

  /* Does listening after the ch* avoid a hole for us? */
  if (flags & PC_CHMOD)
    chmod(p->path, mode);
  if (flags & PC_CHOWN)
    chown(p->path, owner, group);

  listen(p->fd, 5);

  *pp = p;
  return 0L;
}

long pc_chprot(pc_port *p, uid_t owner, gid_t group, mode_t mode)
{
  if (p == NULL)
    return PCerrNullArg;

  if (p->type != PC_LISTENER)
    return PCerrBadType;

  chown(p->path, owner, group);
  chmod(p->path, mode);

  return 0L;
}

long pc_openport(pc_port **pp, char *name)
{
  pc_port *p;
  struct sockaddr_un n;
  int errcopy;

  if (pp == NULL || name == NULL)
    return PCerrNullArg;

  p = malloc(sizeof(pc_port));
  if (p == NULL)
    return PCerrNoMem;

  p->parent = NULL;
  p->type = PC_REGULAR;
  p->path = malloc(strlen(name)+1);
  if (p->path == NULL)
    {
      free(p);
      return PCerrNoMem;
    }
  strcpy(p->path, name);

  p->fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (p->fd == -1)
    {
      free(p->path);
      free(p);
      return PCerrSocketFailed;
    }

  n.sun_family = PF_UNIX;
  strcpy(n.sun_path, p->path);

  /* XXX Need to set an alarm here. If a nanny has run and died
     (not ever observed) and leaves its socket lying about, under
     Irix at least, this connect will block forever. Or several
     minutes at least. */
  if (connect(p->fd, &n, sizeof(struct sockaddr_un)) == -1)
    {
      errcopy = errno;
      close(p->fd);
      free(p->path);
      free(p);
      if (errcopy == EACCES)
	return PCerrNotAllowed;
      /* Hmmm. I've seen ECONNREFUSED when the listener has died.
	 If you can also get it when the listener is backed up,
	 we're screwed. */
      if (errcopy == ENOENT || errcopy == ECONNREFUSED)
	return PCerrNoListener;
      syslog(LOG_ERR, "connect returned %d\n", errcopy);
      return PCerrConnectFailed;
    }

  *pp = p;
  return 0L;
}

long pc_close(pc_port *p)
{
  if (p == NULL)
    return PCerrNullArg;

  close(p->fd);
  if (p->type == PC_LISTENER)
    unlink(p->path);

  if (p->path != NULL)
    free(p->path);

  free(p);

  return 0L;
}

long pc_send(pc_message *m)
{
  int l = 0, r;

  if (m == NULL)
    return PCerrNullArg;

  if (m->source == NULL)
    return PCerrNoDestination;

  if (m->data == NULL || m->length < 1)
    return PCerrNoData;

  while (l < m->length)
    {
      r = write(m->source->fd, m->data, m->length);
      if (r == -1)
	return PCerrWriteFailed;
      l += r;
    }

  return 0L;
}

long pc_receive(pc_message **mm, pc_port *p)
{
  char buf[2048];
  pc_message *m;
  int len;

  if (p == NULL || mm == NULL)
    return PCerrNullArg;

  m = malloc(sizeof(pc_message));
  if (m == NULL)
    return PCerrNoMem;

  len = read(p->fd, buf, sizeof(buf) - 1);
  if (len == -1)
    {
      free(m);
      return PCerrReadFailed;
    }

  m->data = malloc(len + 1);
  if (m->data == NULL)
    {
      free(m);
      return PCerrNoMem;
    }

  m->type = PC_DATA;
  m->source = p;
  buf[len] = '\0';
  memcpy(m->data, buf, len + 1);
  m->length = len;

  *mm = m;
  return 0L;
}

static fd_print(fd_set *r)
{
  int i;

  for (i = 0; i < FD_SETSIZE; i++)
    if (FD_ISSET(i, r))
      fprintf(stderr, "%d ", i);

  printf("\n");
}

long pc_wait(pc_message **mm, pc_state *s)
{
  int i, j;
  fd_set r, w;
  int ret;
  pc_message *m;
  pc_port *p;

  if (mm == NULL || s == NULL)
    return PCerrNullArg;

  r = s->readfds;
  w = s->writefds;

  m = calloc(1, sizeof(pc_message));
  if (m == NULL)
    return PCerrNoMem;

  ret = select(s->nfds, &r, &w, NULL, NULL);
  if (ret == -1)
    {
      if (errno == EINTR)
	m->type = PC_SIGNAL;
      else
	m->type = PC_BROKEN;
      *mm = m;
      return 0L;
    }

  for (i = 0; i < s->nfds; i++)
    if (FD_ISSET(i, &r) || FD_ISSET(i, &w))
      {
	for (j = 0; j < s->numports; j++)
	  if (i == s->ports[j]->fd)
	    {
	      if (s->ports[j]->type == PC_LISTENER)
		{
		  p = malloc(sizeof(pc_port));
		  if (p == NULL)
		    {
		      free(m);
		      return PCerrNoMem;
		    }

		  p->fd = accept(s->ports[j]->fd, NULL, NULL);
		  if (p->fd == -1)
		    {
		      free(m);
		      free(p);
		      return PCerrAcceptFailed;
		    }
		  p->parent = s->ports[j];
		  p->type = PC_LISTENEE;
		  p->path = NULL;

		  m->type = PC_NEWCONN;
		  m->source = s->ports[j];
		  m->data = p;
		  *mm = m;
		  return 0L;
		}
	      else
		{
		  free(m);
		  return pc_receive(mm, s->ports[j]);
		}
	    }

	/* It isn't a network port, so it must be something the user
	   requested info on. */
	m->type = PC_FD;
	if (FD_ISSET(i, &r))
	  m->event = PC_READ;
	else
	  m->event = PC_WRITE;
	m->fd = i;
	*mm = m;
	return 0L;
      }
}

/* Close all connections and pending connections associated with
   the port p we are listening on. Ship them an optional message
   before blowing them away. */
long pc_secure(pc_state *s, pc_port *p, pc_message *m)
{
  fd_set r;
  int fd, i, ret;
  struct timeval t;
  pc_port *pc;

  if (s == NULL || p == NULL)
    return PCerrNullArg;

  if (p->type != PC_LISTENER)
    return PCerrBadType;

  /* If this state is responsible for listening, clear the queue. */
  if (FD_ISSET(p->fd, &(s->readfds)))
    {
      t.tv_sec = 0; t.tv_usec = 0;
      FD_ZERO(&r);
      FD_SET(p->fd, &r);
      while (ret = select(p->fd + 1, &r, NULL, NULL, &t))
	{
	  if (ret == -1)
	    {
	      if (errno == EINTR)
		continue;
	      return PCerrSelectFailed;
	    }

	  fd = accept(p->fd, NULL, NULL);
	  if (m)
	    write(fd, m->data, m->length);
	  close(fd);
	}
    }

  /* find everything that came from p and shut it down. */
  for (i = 0; i < s->numports;)
    {
      pc = s->ports[i];
      if (pc->type == PC_LISTENEE && pc->parent == p)
	{
	  pc_removeport(s, pc);		/* in effect does our i++ */
	  if (m)
	    write(pc->fd, m->data, m->length);
	  pc_close(pc);
	}
      else
	i++;
    }
}
