#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <bstring.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "pc_bsd.h"

pc_state *pc_init(void)
{
  pc_state *ret;

  ret = malloc(sizeof(pc_state));
  if (ret == NULL)
    return NULL;

  ret->numports = 0;

  ret->nfds = 0;
  FD_ZERO(&(ret->readfds));
  FD_ZERO(&(ret->writefds));

  return ret;
}

int pc_freemessage(pc_message *m)
{
  if (m == NULL)
    return 0;

  if (m->type == PC_DATA)
    free(m->data);

  free(m);
}

/*
 * Add a port to a state structure.
 */
int pc_addport(pc_state *s, pc_port *p)
{
  if (s == NULL || p == NULL)
    return 1;

  if (s->numports == PC_MAXPORTS)
    return 1;

  s->ports[s->numports++] = p;

  FD_SET(p->fd, &(s->readfds));
  s->nfds = MAX(s->nfds, p->fd + 1);

  return 0;
}

int pc_removeport(pc_state *s, pc_port *p)
{
  int i;

  for (i = 0; i < s->numports; i++)
    if (s->ports[i] == p)
      {
	s->numports--;
	for (; i < s->numports; i++)
	  s->ports[i] = s->ports[i+1];
	FD_CLR(p->fd, &(s->readfds)); /* should adjust nfds someday */
	return 0;
      }

  return 1;
}

int pc_addfd(pc_state *s, pc_fd *fd)
{
  if (fd == NULL)
    return 1;

  if (fd->events & PC_READ)
    FD_SET(fd->fd, &(s->readfds));
 
  if (fd->events & PC_WRITE)
    FD_SET(fd->fd, &(s->writefds));

  if (fd->events & (PC_READ | PC_WRITE))
    s->nfds = MAX(s->nfds, fd->fd + 1);

  return 0;
}

int pc_removefd(pc_state *s, pc_fd *fd)
{
  if (fd == NULL)
    return 1;

  if (fd->events & PC_READ)
    FD_CLR(fd->fd, &(s->readfds));
 
  if (fd->events & PC_WRITE)
    FD_CLR(fd->fd, &(s->writefds));

  return 0;
}

/* Should probably make it fail if chown or chmod fail. */
pc_port *pc_makeport(char *name, int flags,
		     uid_t owner, gid_t group, mode_t mode)
{
  pc_port *p;
  struct sockaddr_un n;

  if (name == NULL)
    return NULL;

  p = malloc(sizeof(pc_port));
  if (p == NULL)
    return NULL;

  p->parent = NULL;
  p->type = PC_LISTENER;
  p->path = malloc(strlen(name)+1);
  if (p->path == NULL)
    {
      free(p);
      return NULL;
    }
  strcpy(p->path, name);

  p->fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (p->fd == -1)
    {
      free(p->path);
      free(p);
      return NULL;
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
      return NULL;
    }

  /* Does listening after the ch* avoid a hole for us? */
  if (flags & PC_CHMOD)
    chmod(p->path, mode);
  if (flags & PC_CHOWN)
    chown(p->path, owner, group);

  listen(p->fd, 5);

  return p;
}

int pc_chprot(pc_port *p, uid_t owner, gid_t group, mode_t mode)
{
  if (p == NULL)
    return 1;

  if (p->type != PC_LISTENER)
    return 1;

  chown(p->path, owner, group);
  chmod(p->path, mode);

  return 0;
}

pc_port *pc_openport(char *name)
{
  pc_port *p;
  struct sockaddr_un n;

  if (name == NULL)
    return NULL;

  p = malloc(sizeof(pc_port));
  if (p == NULL)
    return NULL;

  p->parent = NULL;
  p->type = PC_REGULAR;
  p->path = malloc(strlen(name)+1);
  if (p->path == NULL)
    {
      free(p);
      return NULL;
    }
  strcpy(p->path, name);

  p->fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (p->fd == -1)
    {
      free(p->path);
      free(p);
      return NULL;
    }

  n.sun_family = PF_UNIX;
  strcpy(n.sun_path, p->path);

  if (connect(p->fd, &n, sizeof(struct sockaddr_un)) == -1)
    {
      close(p->fd);
      free(p->path);
      free(p);
      return NULL;
    }

  return p;
}

int pc_close(pc_port *p)
{
  if (p == NULL)
    return 0;

  close(p->fd);
  if (p->type == PC_LISTENER)
    unlink(p->path);

  if (p->path != NULL)
    free(p->path);

  free(p);

  return 0;
}

int pc_send(pc_message *m)
{
  int l = 0, r;

  if (m == NULL)
    return 1;

  if (m->source == NULL)
    return 1;

  if (m->data == NULL || m->length < 1)
    return 1;

  while (l < m->length)
    {
      r = write(m->source->fd, m->data, m->length);
      if (r == -1)
	return 1;
      l += r;
    }

  return 0;
}

pc_message *pc_receive(pc_port *p)
{
  char buf[2048];
  pc_message *m;
  int len;

  if (p == NULL)
    return NULL;

  m = malloc(sizeof(pc_message));
  if (m == NULL)
    return NULL;

  len = read(p->fd, buf, sizeof(buf) - 1);
  if (len == -1)
    {
      free(m);
      return NULL;
    }

  m->data = malloc(len + 1);
  if (m->data == NULL)
    {
      free(m);
      return NULL;
    }

  m->type = PC_DATA;
  m->source = p;
  buf[len] = '\0';
  memcpy(m->data, buf, len + 1);
  m->length = len;

  return m;
}

fd_print(fd_set *r)
{
  int i;

  for (i = 0; i < FD_SETSIZE; i++)
    if (FD_ISSET(i, r))
      fprintf(stderr, "%d ", i);

  printf("\n");
}

pc_message *pc_wait(pc_state *s)
{
  int i, j;
  fd_set r, w;
  int ret;
  pc_message *m;
  pc_port *p;

  r = s->readfds;
  w = s->writefds;

  fd_print(&r);

  m = malloc(sizeof(pc_message));
  if (m == NULL)
    return NULL;

  ret = select(s->nfds, &r, &w, NULL, NULL);
  if (ret == -1)
    {
      if (errno == EINTR)
	m->type = PC_SIGNAL;
      else
	m->type = PC_BROKEN;
      return m;
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
		      return NULL;
		    }

		  p->fd = accept(s->ports[j]->fd, NULL, 0);
		  if (p->fd == -1)
		    {
		      free(m);
		      free(p);
		      return NULL;
		    }
		  p->parent = s->ports[j];
		  p->type = PC_LISTENEE;
		  p->path = NULL;

		  m->type = PC_NEWCONN;
		  m->source = s->ports[j];
		  m->data = p;
		  return m;
		}
	      else
		{
		  free(m);
		  return pc_receive(s->ports[j]);
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
	return m;
      }
}
