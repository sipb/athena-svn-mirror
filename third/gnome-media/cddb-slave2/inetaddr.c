/* GNet - Networking library
 * Copyright (C) 2000  David Helder
 * Copyright (C) 2000  Andrew Lanoix
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "gnet-private.h"
#include "inetaddr.h"

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif


/* **************************************** */

static gboolean gnet_gethostbyname(const char* hostname, struct sockaddr_in* sa, gchar** nicename);
static gchar* gnet_gethostbyaddr(const char* addr, size_t length, int type);

/* Testing stuff */
/*  #undef   HAVE_GETHOSTBYNAME_R_GLIBC */
/*  #define  HAVE_GETHOSTBYNAME_R_GLIB_MUTEX */

/* TODO: Move this to an init function */
#ifdef HAVE_GETHOSTBYNAME_R_GLIB_MUTEX
#  ifndef G_THREADS_ENABLED
#    error Using GLib Mutex but thread are not enabled.
#  endif
G_LOCK_DEFINE (gethostbyname);
#endif

/* Thread safe gethostbyname.  The only valid fields are sin_len,
   sin_family, and sin_addr.  Nice name */

gboolean
gnet_gethostbyname(const char* hostname, struct sockaddr_in* sa, gchar** nicename)
{
  gboolean rv = FALSE;

#ifdef HAVE_GETHOSTBYNAME_R_GLIBC
  {
    struct hostent result_buf, *result;
    size_t len;
    char* buf;
    int herr;
    int res;

    len = 1024;
    buf = g_new(gchar, len);

    while ((res = gethostbyname_r (hostname, &result_buf, buf, len, &result, &herr))
	   == ERANGE)
      {
	len *= 2;
	buf = g_renew (gchar, buf, len);
      }

    if (res || result == NULL || result->h_addr_list[0] == NULL)
      goto done;

    if (sa)
      {
	sa->sin_family = result->h_addrtype;
	memcpy(&sa->sin_addr, result->h_addr_list[0], result->h_length);
      }

    if (nicename && result->h_name)
      *nicename = g_strdup(result->h_name);

    rv = TRUE;

  done:
    g_free(buf);
  }

#else
#ifdef HAVE_GET_HOSTBYNAME_R_SOLARIS
  {
    struct hostent result;
    size_t len;
    char* buf;
    int herr;
    int res;

    len = 1024;
    buf = g_new(gchar, len);

    while ((res = gethostbyname_r (hostname, &result, buf, len, &herr)) == ERANGE)
      {
	len *= 2;
	buf = g_renew (gchar, buf, len);
      }

    if (res || hp == NULL || hp->h_addr_list[0] == NULL)
      goto done;

    if (sa)
      {
	sa->sin_family = result->h_addrtype;
	memcpy(&sa->sin_addr, result->h_addr_list[0], result->h_length);
      }

    if (nicename && result->h_name)
      *nicename = g_strdup(result->h_name);

    rv = TRUE;

  done:
    g_free(buf);
  }

#else
#ifdef HAVE_GETHOSTBYNAME_R_HPUX
  {
    struct hostent result;
    struct hostent_data buf;
    int res;

    res = gethostbyname_r (hostname, &result, &buf);

    if (res == 0)
      {
	if (sa)
	  {
	    sa->sin_family = result.h_addrtype;
	    memcpy(&sa->sin_addr, result.h_addr_list[0], result.h_length);
	  }

	if (nicename && result.h_name)
	  *nicename = g_strdup(result.h_name);

	rv = TRUE;
      }
  }

#else
#ifdef HAVE_GETHOSTBYNAME_R_GLIB_MUTEX
  {
    struct hostent* he;

    if (!g_threads_got_initialized)
      g_thread_init (NULL);

    G_LOCK (gethostbyname);
    he = gethostbyname(hostname);

    if (he != NULL && he->h_addr_list[0] != NULL)
      {
	if (sa)
	  {
	    sa->sin_family = he->h_addrtype;
	    memcpy(&sa->sin_addr, he->h_addr_list[0], he->h_length);
	  }

	if (nicename && he->h_name)
	  *nicename = g_strdup(he->h_name);

	rv = TRUE;
      }
    G_UNLOCK (gethostbyname);
  }
#else
#ifdef GNET_WIN32
  {
    struct hostent *result;

    WaitForSingleObject(gnet_hostent_Mutex, INFINITE);
    result = gethostbyname(hostname);

    if (result != NULL)
      {
	if (sa)
	  {
	    sa->sin_family = result->h_addrtype;
	    memcpy(&sa->sin_addr, result->h_addr_list[0], result->h_length);
	  }

	if (nicename && result->h_name)
	  *nicename = g_strdup(result->h_name);

	ReleaseMutex(gnet_hostent_Mutex);
	rv = TRUE;

      }
  }
#else
  {
    struct hostent* he;

    he = gethostbyname(hostname);
    if (he != NULL && he->h_addr_list[0] != NULL)
      {
	if (sa)
	  {
	    sa->sin_family = he->h_addrtype;
	    memcpy(&sa->sin_addr, he->h_addr_list[0], he->h_length);
	  }

	if (nicename && he->h_name)
	  *nicename = g_strdup(he->h_name);

	rv = TRUE;
      }
  }
#endif
#endif
#endif
#endif
#endif

  return rv;
}

/*

   Thread safe gethostbyaddr (we assume that gethostbyaddr_r follows
   the same pattern as gethostbyname_r, so we don't have special
   checks for it in configure.in.

   Returns: the hostname, NULL if there was an error.
*/

gchar*
gnet_gethostbyaddr(const char* addr, size_t length, int type)
{
  gchar* rv = NULL;

#ifdef HAVE_GETHOSTBYNAME_R_GLIBC
  {
    struct hostent result_buf, *result;
    size_t len;
    char* buf;
    int herr;
    int res;

    len = 1024;
    buf = g_new(gchar, len);

    while ((res = gethostbyaddr_r (addr, length, type, &result_buf, buf, len, &result, &herr))
	   == ERANGE)
      {
	len *= 2;
	buf = g_renew (gchar, buf, len);
      }

    if (res || result == NULL || result->h_name == NULL)
      goto done;

    rv = g_strdup(result->h_name);

  done:
    g_free(buf);
  }

#else
#ifdef HAVE_GET_HOSTBYNAME_R_SOLARIS
  {
    struct hostent result;
    size_t len;
    char* buf;
    int herr;
    int res;

    len = 1024;
    buf = g_new(gchar, len);

    while ((res = gethostbyaddr_r (addr, lenght, type, &result, buf, len, &herr)) == ERANGE)
      {
	len *= 2;
	buf = g_renew (gchar, buf, len);
      }

    if (res || hp == NULL || hp->h_name == NULL)
      goto done;

    rv = g_strdup(result->h_name);

  done:
    g_free(buf);
  }

#else
#ifdef HAVE_GETHOSTBYNAME_R_HPUX
  {
    struct hostent result;
    struct hostent_data buf;
    int res;

    res = gethostbyaddr_r (addr, length, type, &result, &buf);

    if (res == 0)
      rv = g_strdup (result.h_name);
  }

#else
#ifdef HAVE_GETHOSTBYNAME_R_GLIB_MUTEX
  {
    struct hostent* he;

    if (!g_threads_got_initialized)
      g_thread_init (NULL);

    G_LOCK (gethostbyname);
    he = gethostbyaddr(addr, length, type);
    if (he != NULL && he->h_name != NULL)
      rv = g_strdup(he->h_name);
    G_UNLOCK (gethostbyname);
  }
#else
#ifdef GNET_WIN32
  {
    struct hostent* he;

    WaitForSingleObject(gnet_hostent_Mutex, INFINITE);
    he = gethostbyaddr(addr, length, type);
    if (he != NULL && he->h_name != NULL)
      rv = g_strdup(he->h_name);
    ReleaseMutex(gnet_hostent_Mutex);
  }
#else
  {
    struct hostent* he;

    he = gethostbyaddr(addr, length, type);
    if (he != NULL && he->h_name != NULL)
      rv = g_strdup(he->h_name);
  }
#endif
#endif
#endif
#endif
#endif

  return rv;
}

/* **************************************** */
/* TO IMPLEMENT?:			    */

/***
 *
 * Create an internet address from raw bytes.
 *
 **/
/*  InetAddr* inetaddr_bytes_new(const guint8* addr, const gint length); */



/* **************************************** */




/**
 *  gnet_inetaddr_new:
 *  @name: a nice name (eg, mofo.eecs.umich.edu) or a dotted decimal name
 *    (eg, 141.213.8.59).  You can delete the after the function is called.
 *  @port: port number (0 if the port doesn't matter)
 *
 *  Create an internet address from a name and port.  This function
 *  may block.
 *
 *  Returns: a new #GInetAddr, or NULL if there was a failure.
 *
 **/
GInetAddr*
gnet_inetaddr_new (const gchar* name, gint port)
{
  struct sockaddr_in* sa_in;
  struct in_addr inaddr;
  GInetAddr* ia = NULL;

  g_return_val_if_fail(name != NULL, NULL);

  /* Try to read the name as if were dotted decimal */
  if (inet_pton(AF_INET, name, &inaddr) > 0)
    {
      ia = g_new0(GInetAddr, 1);

      ia->ref_count = 1;
      sa_in = (struct sockaddr_in*) &ia->sa;
      sa_in->sin_family = AF_INET;
      sa_in->sin_port = g_htons(port);
      memcpy(&sa_in->sin_addr, (char*) &inaddr, sizeof(struct in_addr));
    }

  else
    {
      struct sockaddr_in sa;

      /* Try to get the host by name (ie, DNS) */
      if (gnet_gethostbyname(name, &sa, NULL))
	{
	  ia = g_new0(GInetAddr, 1);
	  ia->name = g_strdup(name);
	  ia->ref_count = 1;

	  sa_in = (struct sockaddr_in*) &ia->sa;
	  sa_in->sin_family = AF_INET;
	  sa_in->sin_port = g_htons(port);
	  memcpy(&sa_in->sin_addr, &sa.sin_addr, 4);
	}
    }

  return ia;
}


static void* gethostbyname_async_child (void* arg);


/**
 *  gnet_inetaddr_new_async:
 *  @name: a nice name (eg, mofo.eecs.umich.edu) or a dotted decimal name
 *    (eg, 141.213.8.59).  You can delete the after the function is called.
 *  @port: port number (0 if the port doesn't matter)
 *  @func: Callback function.
 *  @data: User data passed when callback function is called.
 *
 *  Create a GInetAddr from a name and port asynchronously.  Once the
 *  structure is created, it will call the callback.  It may call the
 *  callback if there is a failure.
 *
 *  The Unix version creates a pthread thread which does the lookup.
 *  If pthreads aren't available, it forks and does the lookup.
 *  Forking will be slow or even fail when using operating systems
 *  that copy the entire process when forking.
 *
 *  If you need to lookup hundreds of addresses, we recommend calling
 *  g_main_iteration(FALSE) between calls.  This will help prevent an
 *  explosion of threads or processes.
 *
 *  If you need a more robust library for Unix, look at <ulink
 *  url="http://www.gnu.org/software/adns/adns.html">GNU ADNS</ulink>.
 *  GNU ADNS is under the GNU GPL.  This library does not use threads
 *  or processes.
 *
 *  The Windows version should work fine.  Windows has an asynchronous
 *  DNS lookup function.
 *
 *  Returns: ID of the lookup which can be used with
 *  gnet_inetaddr_new_async_cancel() to cancel it; NULL on failure.
 *
 **/
GInetAddrNewAsyncID
gnet_inetaddr_new_async (const gchar* name, gint port,
			 GInetAddrNewAsyncFunc func, gpointer data)
{
  int pipes[2];
  GInetAddr* ia;
  struct sockaddr_in* sa_in;
  GInetAddrAsyncState* state;

  g_return_val_if_fail(name != NULL, NULL);
  g_return_val_if_fail(func != NULL, NULL);

  /* Open a pipe */
  if (pipe(pipes) == -1)
    {
      (*func)(NULL, GINETADDR_ASYNC_STATUS_ERROR, data);
      return NULL;
    }

# ifdef HAVE_LIBPTHREAD			/* Pthread */
  {
    pthread_t pthread;
    void** args;

    args = g_new (void*, 2);
    args[0] = (void*) name;
    args[1] = g_new (int, 1);
    *(int*) args[1] = pipes[1];

    if (pthread_create (&pthread, NULL/*&pthread_attr*/, gethostbyname_async_child, args))
      {
	g_warning ("Pthread_create error\n");
	return NULL;
      }
    state = g_new0(GInetAddrAsyncState, 1);
    state->pthread = pthread;

  }
# else 					/* Fork */
  {
    pid_t pid = -1;

    /* Fork to do the look up. */
  fork_again:
    errno = 0;
    if ((pid = fork()) == 0)
      {
	void** args;

	args = g_new (void*, 2);
	args[0] = (void*) name;
	args[1] = g_new (int, 1);
	*(int*) args[1] = pipes[1];

	gethostbyname_async_child (args);

	/* Exit (we don't want atexit called, so do _exit instead) */
	_exit(EXIT_SUCCESS);
      }

    /* Set up state */
    else if (pid > 0)
      {
	state = g_new0(GInetAddrAsyncState, 1);
	state->pid = pid;
      }

    /* Try again */
    else if (errno == EAGAIN)
      {
	sleep(0);	/* Yield the processor */
	goto fork_again;
      }

    /* Else fork failed completely */
    else
      {
	g_warning ("Fork error: %s (%d)\n", g_strerror(errno), errno);
	(*func)(NULL, GINETADDR_ASYNC_STATUS_ERROR, data);
	return NULL;
      }
  }
#endif

  /* Create a new InetAddr */
  ia = g_new0(GInetAddr, 1);
  ia->name = g_strdup(name);
  ia->ref_count = 1;

  sa_in = (struct sockaddr_in*) &ia->sa;
  sa_in->sin_family = AF_INET;
  sa_in->sin_port = g_htons(port);

  /* Create a structure for the call back */
  state->ia = ia;
  state->func = func;
  state->data = data;
  state->fd = pipes[0];

  /* Add a watch */
  state->watch = g_io_add_watch(g_io_channel_unix_new(pipes[0]),
				(G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL),
				gnet_inetaddr_new_async_cb,
				state);

  return state;
}



static void*
gethostbyname_async_child (void* arg) /* pthread_create friendly */
{
  void** args      = (void**) arg;
  const char* name = (const char*) args[0];
  int outfd        = *(int*) args[1];
  struct sockaddr_in sa;

  /* Try to get the host by name (ie, DNS) */
  if (gnet_gethostbyname(name, &sa, NULL))
    {
      guchar size = 4;	/* FIX for IPv6 */

      if ( (write(outfd, &size, sizeof(guchar)) == -1) ||
	   (write(outfd, &sa.sin_addr, size) == -1) )
	g_warning ("Problem writing to pipe\n");
    }
  else
    {
      /* Write a zero */
      guchar zero = 0;

      if (write(outfd, &zero, sizeof(zero)) == -1)
	g_warning ("Problem writing to pipe\n");
    }

  /* Close the socket */
  g_free (args[1]);
  g_free (args);
  close(outfd);

  return NULL;
}


gboolean
gnet_inetaddr_new_async_cb (GIOChannel* iochannel,
			    GIOCondition condition,
			    gpointer data)
{
  GInetAddrAsyncState* state = (GInetAddrAsyncState*) data;

  /* Read from the pipe */
  if (condition & G_IO_IN)
    {
      int rv;
      char* buf;
      int length;

      buf = &state->buffer[state->len];
      length = sizeof(state->buffer) - state->len;

      if ((rv = read(state->fd, buf, length)) >= 0)
	{
	  state->len += rv;

	  /* Return true if there's more to read */
	  if ((state->len - 1) != state->buffer[0])
	    return TRUE;

	  /* We're done reading.  Copy into the addr if we were
             successful. */
	  if (state->len > 1)
	    {
	      struct sockaddr_in* sa_in;

	      sa_in = (struct sockaddr_in*) &state->ia->sa;
	      memcpy(&sa_in->sin_addr, &state->buffer[1], (state->len - 1));
	    }

	  /* Otherwise, we got a 0 because there was an error */
	  else
	    goto error;

	  /* Remove the watch now in case we don't return immediately */
	  g_source_remove (state->watch);

	  /* Call back */
	  (*state->func)(state->ia, GINETADDR_ASYNC_STATUS_OK, state->data);
	  close (state->fd);
#	  ifdef HAVE_LIBPTHREAD
	    pthread_join (state->pthread, NULL);
#	  else
	    waitpid (state->pid, NULL, 0);
#	  endif
	  g_free(state);
	  return FALSE;
	}
      /* otherwise, there was an error */
    }
  /* otherwise, there was an error */

 error:
  /* Remove the watch now in case we don't return immediately */
  g_source_remove (state->watch);

  (*state->func)(NULL, GINETADDR_ASYNC_STATUS_ERROR, state->data);
  gnet_inetaddr_new_async_cancel(state);
  return FALSE;
}



/**
 *  gnet_inetaddr_new_async_cancel:
 *  @id: ID of the lookup
 *
 *  Cancel an asynchronous GInetAddr creation that was started with
 *  gnet_inetaddr_new_async().
 *
 */
void
gnet_inetaddr_new_async_cancel (GInetAddrNewAsyncID id)
{
  GInetAddrAsyncState* state = (GInetAddrAsyncState*) id;

  g_return_if_fail(state != NULL);

  gnet_inetaddr_delete (state->ia);
  g_source_remove (state->watch);

  close (state->fd);
# ifdef HAVE_LIBPTHREAD
    pthread_join (state->pthread, NULL);
# else
    kill (state->pid, SIGKILL);
    waitpid (state->pid, NULL, 0);
# endif

  g_free(state);
}

/**
 *  gnet_inetaddr_new_nonblock:
 *  @name: a nice name (eg, mofo.eecs.umich.edu) or a dotted decimal name
 *    (eg, 141.213.8.59).  You can delete the after the function is called.
 *  @port: port number (0 if the port doesn't matter)
 *
 *  Create an internet address from a name and port, but don't block
 *  and fail if success would require blocking.  This is, if the name
 *  is a canonical name or "localhost", it returns the address.
 *  Otherwise, it returns NULL.
 *
 *  Returns: a new #GInetAddr, or NULL if there was a failure.
 *
 **/
GInetAddr*
gnet_inetaddr_new_nonblock (const gchar* name, gint port)
{
  struct sockaddr_in* sa_in;
  struct in_addr inaddr;
  GInetAddr* ia = NULL;

  g_return_val_if_fail (name, NULL);

  /* Try to read the name as if were dotted decimal */
 try_again:
  if (inet_pton(AF_INET, name, &inaddr) > 0)
    {
      ia = g_new0(GInetAddr, 1);

      ia->ref_count = 1;
      sa_in = (struct sockaddr_in*) &ia->sa;
      sa_in->sin_family = AF_INET;
      sa_in->sin_port = g_htons(port);
      memcpy(&sa_in->sin_addr, (char*) &inaddr, sizeof(struct in_addr));
    }

  /* If the name is localhost, change it to 127.0.0.1 and try again */
  /* FIX: I don't think this is legal... */
  else if (!strcmp (name, "localhost"))
    {
      name = "127.0.0.1";
      goto try_again;
    }

  return ia;
}

/**
 *   gnet_inetaddr_clone:
 *   @ia: Address to clone
 *
 *   Create an internet address from another one.
 *
 *   Returns: a new InetAddr, or NULL if there was a failure.
 *
 **/
GInetAddr*
gnet_inetaddr_clone(const GInetAddr* ia)
{
  GInetAddr* cia;

  g_return_val_if_fail (ia != NULL, NULL);

  cia = g_new0(GInetAddr, 1);
  cia->ref_count = 1;
  cia->sa = ia->sa;
  if (ia->name != NULL)
    cia->name = g_strdup(ia->name);

  return cia;
}


/**
 *  gnet_inetaddr_delete:
 *  @ia: GInetAddr to delete
 *
 *  Delete a GInetAddr.
 *
 **/
void
gnet_inetaddr_delete(GInetAddr* ia)
{
  if (ia != NULL)
    gnet_inetaddr_unref(ia);
}


/**
 *  gnet_inetaddr_ref
 *  @ia: GInetAddr to reference
 *
 *  Increment the reference counter of the GInetAddr.
 *
 **/
void
gnet_inetaddr_ref(GInetAddr* ia)
{
  g_return_if_fail(ia != NULL);

  ++ia->ref_count;
}


/**
 *  gnet_inetaddr_unref
 *  @ia: GInetAddr to unreference
 *
 *  Remove a reference from the GInetAddr.  When reference count
 *  reaches 0, the address is deleted.
 *
 **/
void
gnet_inetaddr_unref(GInetAddr* ia)
{
  g_return_if_fail(ia != NULL);

  --ia->ref_count;

  if (ia->ref_count == 0)
    {
      if (ia->name != NULL)
	g_free (ia->name);
      g_free (ia);
    }
}

/**
 *  gnet_inetaddr_get_name_async:
 *  @ia: Address to get the name of.
 *  @func: Callback function.
 *  @data: User data passed when callback function is called.
 *
 *  Get the nice name of the address (eg, "mofo.eecs.umich.edu").
 *  This function will use the callback once it knows the nice name.
 *  It may even call the callback before it returns.  The callback
 *  will be called if there is an error.
 *
 *  The Unix version forks and does the reverse lookup.  This has
 *  problems.  See the notes for gnet_inetaddr_new_async().  The
 *  Windows version should work fine.
 *
 *  Returns: ID of the lookup which can be used with
 *  gnet_inetaddrr_get_name_async_cancel() to cancel it; NULL on
 *  immediate success or failure.
 *
 **/
GInetAddrGetNameAsyncID
gnet_inetaddr_get_name_async (GInetAddr* ia,
			      GInetAddrGetNameAsyncFunc func,
			      gpointer data)
{
  g_return_val_if_fail(ia != NULL, NULL);
  g_return_val_if_fail(func != NULL, NULL);

  /* If we already know the name, just copy that */
  if (ia->name != NULL)
    {
      (func)(ia, GINETADDR_ASYNC_STATUS_OK, g_strdup(ia->name), data);
    }

  /* Otherwise, fork and look it up */
  else
    {
      pid_t pid = -1;
      int pipes[2];


      /* Open a pipe */
      if (pipe(pipes) == -1)
	{
	  (func)(ia, GINETADDR_ASYNC_STATUS_ERROR, NULL, data);
	  return NULL;
	}


      /* Fork to do the look up. */
    fork_again:
      if ((pid = fork()) == 0)
	{
	  gchar* name;
	  guchar len;

	  /* Write the name to the pipe.  If we didn't get a name, we
             just write the canonical name. */
	  if ((name = gnet_gethostbyaddr((char*) &((struct sockaddr_in*)&ia->sa)->sin_addr,
					sizeof(struct in_addr), AF_INET)) != NULL)
	    {
	      guint lenint = strlen(name);

	      if (lenint > 255)
		{
		  g_warning ("Truncating domain name: %s\n", name);
		  name[256] = '\0';
		  lenint = 255;
		}

	      len = lenint;

	      if ((write(pipes[1], &len, sizeof(len)) == -1) ||
		  (write(pipes[1], name, len) == -1) )
		g_warning ("Problem writing to pipe\n");

	      g_free(name);
	    }
	  else
	    {
	      gchar buffer[INET_ADDRSTRLEN];	/* defined in netinet/in.h */
	      guchar* p = (guchar*) &(GNET_SOCKADDR_IN(ia->sa).sin_addr);

	      g_snprintf(buffer, sizeof(buffer),
			 "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	      len = strlen(buffer);

	      if ((write(pipes[1], &len, sizeof(len)) == -1) ||
		  (write(pipes[1], buffer, len) == -1))
		g_warning ("Problem writing to pipe\n");
	    }

	  /* Close the socket */
	  close(pipes[1]);

	  /* Exit (we don't want atexit called, so do _exit instead) */
	  _exit(EXIT_SUCCESS);

	}

      /* Set up an IOChannel to read from the pipe */
      else if (pid > 0)
	{
	  GInetAddrReverseAsyncState* state;

	  /* Create a structure for the call back */
	  state = g_new0(GInetAddrReverseAsyncState, 1);
	  state->ia = ia;
	  state->func = func;
	  state->data = data;
	  state->pid = pid;
	  state->fd = pipes[0];

	  /* Add a watch */
	  state->watch = g_io_add_watch(g_io_channel_unix_new(pipes[0]),
					(G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL),
					gnet_inetaddr_get_name_async_cb,
					state);
	  return state;
	}

      /* Try again */
      else if (errno == EAGAIN)
	{
	  sleep(0);	/* Yield the processor */
	  goto fork_again;
	}

      /* Else there was a goofy error */
      else
	{
	  g_warning ("Fork error: %s (%d)\n", g_strerror(errno), errno);
	  (*func)(ia, GINETADDR_ASYNC_STATUS_ERROR, NULL, data);
	}
    }

  return NULL;
}



gboolean
gnet_inetaddr_get_name_async_cb (GIOChannel* iochannel,
				 GIOCondition condition,
				 gpointer data)
{
  GInetAddrReverseAsyncState* state = (GInetAddrReverseAsyncState*) data;
  gchar* name = NULL;

  g_return_val_if_fail (state != NULL, FALSE);

  /* Read from the pipe */
  if (condition & G_IO_IN)
    {
      int rv;
      char* buf;
      int length;

      buf = &state->buffer[state->len];
      length = sizeof(state->buffer) - state->len;

      if ((rv = read(state->fd, buf, length)) >= 0)
	{
	  state->len += rv;

	  /* Return true if there's more to read */
	  if ((state->len - 1) != state->buffer[0])
	    return TRUE;

	  /* Copy the name */
	  name = g_new(gchar, state->buffer[0] + 1);
	  strncpy(name, &state->buffer[1], state->buffer[0]);
	  name[state->buffer[0]] = '\0';
	  state->ia->name = g_strdup(name);

	  /* Remove the watch now in case we don't return immediately */
	  g_source_remove (state->watch);

	  /* Call back */
	  (*state->func)(state->ia, GINETADDR_ASYNC_STATUS_OK, name, state->data);
	  close (state->fd);
	  waitpid (state->pid, NULL, 0);
	  g_free (state);
	  return FALSE;
	}
      /* otherwise, there was a read error */
    }
  /* otherwise, there was some error */

  /* Remove the watch now in case we don't return immediately */
  g_source_remove (state->watch);

  /* Call back */
  (*state->func)(state->ia, GINETADDR_ASYNC_STATUS_ERROR, NULL, state->data);
  gnet_inetaddr_get_name_async_cancel(state);
  return FALSE;
}




/**
 *  gnet_inetaddr_get_name_async_cancel:
 *  @id: ID of the lookup
 *
 *  Cancel an asynchronous nice name lookup that was started with
 *  gnet_inetaddr_get_name_async().
 *
 */
void
gnet_inetaddr_get_name_async_cancel(GInetAddrGetNameAsyncID id)
{
  GInetAddrReverseAsyncState* state = (GInetAddrReverseAsyncState*) id;

  g_return_if_fail(state != NULL);

  gnet_inetaddr_delete (state->ia);
  g_source_remove (state->watch);

  close (state->fd);
  kill (state->pid, SIGKILL);
  waitpid (state->pid, NULL, 0);

  g_free(state);
}


/**
 *  gnet_inetaddr_get_canonical_name:
 *  @ia: Address to get the canonical name of.
 *
 *  Get the "canonical" name of an address (eg, for IP4 the dotted
 *  decimal name 141.213.8.59).
 *
 *  Returns: NULL if there was an error.  The caller is responsible
 *  for deleting the returned string.
 *
 **/
gchar*
gnet_inetaddr_get_canonical_name(const GInetAddr* ia)
{
  gchar buffer[INET_ADDRSTRLEN];	/* defined in netinet/in.h */
  guchar* p = (guchar*) &(GNET_SOCKADDR_IN(ia->sa).sin_addr);

  g_return_val_if_fail (ia != NULL, NULL);

  g_snprintf(buffer, sizeof(buffer),
	     "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

  return g_strdup(buffer);
}


/**
 *  gnet_inetaddr_get_port:
 *  @ia: Address to get the port number of.
 *
 *  Get the port number.
 *  Returns: the port number.
 */
gint
gnet_inetaddr_get_port(const GInetAddr* ia)
{
  g_return_val_if_fail(ia != NULL, -1);

  return (gint) g_ntohs(((struct sockaddr_in*) &ia->sa)->sin_port);
}


/**
 *  gnet_inetaddr_set_port:
 *  @ia: Address to set the port number of.
 *  @port: New port number
 *
 *  Set the port number.
 *
 **/
void
gnet_inetaddr_set_port(const GInetAddr* ia, guint port)
{
  g_return_if_fail(ia != NULL);

  ((struct sockaddr_in*) &ia->sa)->sin_port = g_htons(port);
}



/* **************************************** */


/**
 *  gnet_inetaddr_is_canonical:
 *  @name: Name to check
 *
 *  Check if the domain name is canonical.  For IPv4, a canonical name
 *  is a dotted decimal name (eg, 141.213.8.59).
 *
 *  Returns: TRUE if @name is canonical; FALSE otherwise.
 *
 **/
gboolean
gnet_inetaddr_is_canonical (const gchar* name)
{
  struct in_addr inaddr;

  g_return_val_if_fail (name, FALSE);

  return (inet_pton(AF_INET, name, &inaddr) > 0);
}



/**
 *  gnet_inetaddr_is_internet:
 *  @inetaddr: Address to check
 *
 *  Check if the address is a sensible internet address.  This mean it
 *  is not private, reserved, loopback, multicast, or broadcast.
 *
 *  Note that private and loopback address are often valid addresses,
 *  so this should only be used to check for general Internet
 *  connectivity.  That is, if the address passes, it is reachable on
 *  the Internet.
 *
 *  Returns: TRUE if the address is an 'Internet' address; FALSE
 *  otherwise.
 *
 **/
gboolean
gnet_inetaddr_is_internet (const GInetAddr* inetaddr)
{
  g_return_val_if_fail(inetaddr != NULL, FALSE);

  if (!gnet_inetaddr_is_private(inetaddr) 	&&
      !gnet_inetaddr_is_reserved(inetaddr) 	&&
      !gnet_inetaddr_is_loopback(inetaddr) 	&&
      !gnet_inetaddr_is_multicast(inetaddr) 	&&
      !gnet_inetaddr_is_broadcast(inetaddr))
    {
      return TRUE;
    }

  return FALSE;
}



/**
 *  gnet_inetaddr_is_private:
 *  @inetaddr: Address to check
 *
 *  Check if the address is an address reserved for private networks
 *  or something else.  This includes:
 *
 *   10.0.0.0        -   10.255.255.255  (10/8 prefix)
 *   172.16.0.0      -   172.31.255.255  (172.16/12 prefix)
 *   192.168.0.0     -   192.168.255.255 (192.168/16 prefix)
 *
 *  (from RFC 1918.  See also draft-manning-dsua-02.txt)
 *
 *  Returns: TRUE if the address is reserved for private networks;
 *  FALSE otherwise.
 *
 **/
gboolean
gnet_inetaddr_is_private (const GInetAddr* inetaddr)
{
  guint addr;

  g_return_val_if_fail (inetaddr != NULL, FALSE);

  addr = GNET_SOCKADDR_IN(inetaddr->sa).sin_addr.s_addr;
  addr = g_ntohl(addr);

  if ((addr & 0xFF000000) == (10 << 24))
    return TRUE;

  if ((addr & 0xFFF00000) == 0xAC100000)
    return TRUE;

  if ((addr & 0xFFFF0000) == 0xC0A80000)
    return TRUE;

  return FALSE;
}


/**
 *  gnet_inetaddr_is_reserved:
 *  @inetaddr: Address to check
 *
 *  Check if the address is reserved for 'something'.  This excludes
 *  address reserved for private networks.
 *
 *  We check for:
 *    0.0.0.0/16  (top 16 bits are 0's)
 *    Class E (top 5 bits are 11110)
 *
 *  Returns: TRUE if the address is reserved for something; FALSE
 *  otherwise.
 *
 **/
gboolean
gnet_inetaddr_is_reserved (const GInetAddr* inetaddr)
{
  guint addr;

  g_return_val_if_fail (inetaddr != NULL, FALSE);

  addr = GNET_SOCKADDR_IN(inetaddr->sa).sin_addr.s_addr;
  addr = g_ntohl(addr);

  if ((addr & 0xFFFF0000) == 0)
    return TRUE;

  if ((addr & 0xF8000000) == 0xF0000000)
    return TRUE;

  return FALSE;
}



/**
 *  gnet_inetaddr_is_loopback:
 *  @inetaddr: Address to check
 *
 *  Check if the address is a loopback address.  Loopback addresses
 *  have the prefix 127.0.0.1/16.
 *
 *  Returns: TRUE if the address is a loopback address; FALSE
 *  otherwise.
 *
 **/
gboolean
gnet_inetaddr_is_loopback (const GInetAddr* inetaddr)
{
  guint addr;

  g_return_val_if_fail (inetaddr != NULL, FALSE);

  addr = GNET_SOCKADDR_IN(inetaddr->sa).sin_addr.s_addr;
  addr = g_ntohl(addr);

  if ((addr & 0xFF000000) == (127 << 24))
    return TRUE;

  return FALSE;
}


/**
 *  gnet_inetaddr_is_multicast:
 *  @inetaddr: Address to check
 *
 *  Check if the address is a multicast address.  Multicast address
 *  are in the range 224.0.0.1 - 239.255.255.255 (ie, the top four
 *  bits are 1110).
 *
 *  Returns: TRUE if the address is a multicast address; FALSE
 *  otherwise.
 *
 **/
gboolean
gnet_inetaddr_is_multicast (const GInetAddr* inetaddr)
{
  guint addr;

  g_return_val_if_fail (inetaddr != NULL, FALSE);

  addr = GNET_SOCKADDR_IN(inetaddr->sa).sin_addr.s_addr;
  addr = g_htonl(addr);

  if ((addr & 0xF0000000) == 0xE0000000)
    return TRUE;

  return FALSE;
}


/**
 *  gnet_inetaddr_is_broadcast:
 *  @inetaddr: Address to check
 *
 *  Check if the address is a broadcast address.  The broadcast
 *  address is 255.255.255.255.  (Network broadcast address are
 *  network dependent.)
 *
 *  Returns: TRUE if the address is a broadcast address; FALSE
 *  otherwise.
 *
 **/
gboolean
gnet_inetaddr_is_broadcast (const GInetAddr* inetaddr)
{
  guint addr;

  g_return_val_if_fail (inetaddr != NULL, FALSE);

  addr = GNET_SOCKADDR_IN(inetaddr->sa).sin_addr.s_addr;

  if (addr == 0xFFFFFFFF)
    return TRUE;

  return FALSE;
}




/* **************************************** */



/**
 *  gnet_inetaddr_hash:
 *  @p: Pointer to an #GInetAddr.
 *
 *  Hash the address.  This is useful for glib containers.
 *
 *  Returns: hash value.
 *
 **/
guint
gnet_inetaddr_hash (gconstpointer p)
{
  const GInetAddr* ia;
  guint32 port;
  guint32 addr;

  g_assert(p != NULL);

  ia = (const GInetAddr*) p;
  /* We do pay attention to network byte order just in case the hash
     result is saved or sent to a different host.  */
  port = (guint32) g_ntohs(((struct sockaddr_in*) &ia->sa)->sin_port);
  addr = g_ntohl(((struct sockaddr_in*) &ia->sa)->sin_addr.s_addr);

  return (port ^ addr);
}



/**
 *  gnet_inetaddr_equal:
 *  @p1: Pointer to first #GInetAddr.
 *  @p2: Pointer to second #GInetAddr.
 *
 *  Compare two #GInetAddr's.
 *
 *  Returns: 1 if they are the same; 0 otherwise.
 *
 **/
gint
gnet_inetaddr_equal(gconstpointer p1, gconstpointer p2)
{
  const GInetAddr* ia1 = (const GInetAddr*) p1;
  const GInetAddr* ia2 = (const GInetAddr*) p2;

  g_assert(p1 != NULL && p2 != NULL);

  /* Note network byte order doesn't matter */
  return ((GNET_SOCKADDR_IN(ia1->sa).sin_addr.s_addr ==
	   GNET_SOCKADDR_IN(ia2->sa).sin_addr.s_addr) &&
	  (GNET_SOCKADDR_IN(ia1->sa).sin_port ==
	   GNET_SOCKADDR_IN(ia2->sa).sin_port));
}


/**
 *  gnet_inetaddr_noport_equal:
 *  @p1: Pointer to first GInetAddr.
 *  @p2: Pointer to second GInetAddr.
 *
 *  Compare two #GInetAddr's, but does not compare the port numbers.
 *
 *  Returns: 1 if they are the same; 0 otherwise.
 *
 **/
gint
gnet_inetaddr_noport_equal(gconstpointer p1, gconstpointer p2)
{
  const GInetAddr* ia1 = (const GInetAddr*) p1;
  const GInetAddr* ia2 = (const GInetAddr*) p2;

  g_assert(p1 != NULL && p2 != NULL);

  /* Note network byte order doesn't matter */
  return (GNET_SOCKADDR_IN(ia1->sa).sin_addr.s_addr ==
	  GNET_SOCKADDR_IN(ia2->sa).sin_addr.s_addr);
}



/* **************************************** */

/**
 *  gnet_inetaddr_gethostname:
 *
 *  Get the primary host's name.
 *
 *  Returns: the name of the host; NULL if there was an error.  The
 *  caller is responsible for deleting the returned string.
 *
 **/
gchar*
gnet_inetaddr_gethostname(void)
{
  gchar* name = NULL;
  struct utsname myname;

  if (uname(&myname) < 0)
    return NULL;

  if (!gnet_gethostbyname(myname.nodename, NULL, &name))
    return NULL;

  return name;
}


/**
 *  gnet_inetaddr_gethostaddr:
 *
 *  Get the primary host's #GInetAddr.
 *
 *  Returns: the #GInetAddr of the host; NULL if there was an error.
 *  The caller is responsible for deleting the returned #GInetAddr.
 *
 **/
GInetAddr*
gnet_inetaddr_gethostaddr(void)
{
  gchar* name;
  GInetAddr* ia = NULL;

  name = gnet_inetaddr_gethostname();
  if (name != NULL)
    {
      ia = gnet_inetaddr_new(name, 0);
      g_free(name);
    }

  return ia;
}


/* **************************************** */




/**
 *  gnet_inetaddr_new_any:
 *
 *  Create a #GInetAddr with the address INADDR_ANY and port 0.  This
 *  is useful for creating default addresses for binding.  The
 *  address's name will be "<INADDR_ANY>".
 *
 *  Returns: INADDR_ANY #GInetAddr.
 *
 **/
GInetAddr*
gnet_inetaddr_new_any (void)
{
  GInetAddr* ia;
  struct sockaddr_in* sa_in;

  ia = g_new0 (GInetAddr, 1);
  sa_in = (struct sockaddr_in*) &ia->sa;
  sa_in->sin_addr.s_addr = g_htonl(INADDR_ANY);
  sa_in->sin_port = 0;
  ia->name = g_strdup ("<INADDR_ANY>");

  return ia;
}



/**
 *  gnet_inetaddr_autodetect_internet_interface:
 *
 *  Find an Internet interface.  Usually, this interface routes
 *  packets to and from the Internet.  It can be used to automatically
 *  configure simple servers that must advertise their address.  This
 *  sometimes doesn't work correctly when the user is behind a NAT.
 *
 *  Returns: Address of an Internet interface; NULL if it couldn't
 *  find one or there was an error.
 *
 **/
GInetAddr*
gnet_inetaddr_autodetect_internet_interface (void)
{
  GInetAddr* jm_addr;
  GInetAddr* iface;

  /* First try to get the interface with a route to
     junglemonkey.net (141.213.11.1).  This uses the connected UDP
     socket method described by Stevens.  It does not work on all
     systems.  (see Stevens UNPv1 pp 231-3) */
  jm_addr = gnet_inetaddr_new_nonblock ("141.213.11.1", 0);
  g_assert (jm_addr);

  iface = gnet_inetaddr_get_interface_to (jm_addr);
  gnet_inetaddr_delete (jm_addr);

  /* We want an internet interface */
  if (iface && gnet_inetaddr_is_internet(iface))
    return iface;
  gnet_inetaddr_delete (iface);

  /* Try getting an internet interface from the list via
     SIOCGIFCONF. (see Stevens UNPv1 pp 428-) */
  iface = gnet_inetaddr_get_internet_interface ();

  return iface;
}

/**
 *  gnet_inetaddr_get_interface_to:
 *  @addr: address
 *
 *  Figure out which local interface would be used to send a packet to
 *  @addr.  This works on some systems, but not others.  We recommend
 *  using gnet_inetaddr_autodetect_internet_interface() to find an
 *  Internet interface since it's more likely to work.
 *
 *  Returns: Address of an interface used to route packets to @addr;
 *  NULL if there is no such interface or the system does not support
 *  this check.
 *
 **/
GInetAddr*
gnet_inetaddr_get_interface_to (const GInetAddr* addr)
{
  int sockfd;
  struct sockaddr_in myaddr;
  socklen_t len;
  GInetAddr* iface;

  g_return_val_if_fail (addr, NULL);

  sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1)
    return NULL;

  if (connect (sockfd, &addr->sa, sizeof(addr->sa)) == -1)
    {
      GNET_CLOSE_SOCKET(sockfd);
      return NULL;
    }

  len = sizeof (myaddr);
  if (getsockname (sockfd, (struct sockaddr*) &myaddr, &len) != 0)
    {
      GNET_CLOSE_SOCKET(sockfd);
      return NULL;
    }

  iface = g_new0 (GInetAddr, 1);
  iface->ref_count = 1;
  memcpy (&iface->sa, (char*) &myaddr, sizeof (struct sockaddr_in));

  return iface;
}


/**
 *  gnet_inetaddr_get_internet_interface:
 *
 *  Find an Internet interface.  This just calls
 *  gnet_inetaddr_list_interfaces() and returns the first one that
 *  passes gnet_inetaddr_is_internet().  This works well on some
 *  systems, but not so well on others.  We recommend using
 *  gnet_inetaddr_autodetect_internet_interface() to find an Internet
 *  interface since it's more likely to work.
 *
 *  Returns: Address of an Internet interface; NULL if there is no
 *  such interface or the system does not support this check.
 *
 **/
GInetAddr*
gnet_inetaddr_get_internet_interface (void)
{
  GInetAddr* iface = NULL;
  GList* interfaces;
  GList* i;

  /* Get a list of interfaces */
  interfaces = gnet_inetaddr_list_interfaces ();
  if (interfaces == NULL)
    return NULL;

  /* Find the first interface that's an internet interface */
  for (i = interfaces; i != NULL; i = i->next)
    {
      GInetAddr* ia;

      ia = (GInetAddr*) i->data;

      if (gnet_inetaddr_is_internet (ia))
	{
	  iface = gnet_inetaddr_clone (ia);
	  break;
	}
    }

  /* If we didn't find one, return the first interface. */
  if (iface == NULL)
    iface = gnet_inetaddr_clone ((GInetAddr*) interfaces->data);

  /* Delete the interface list */
  for (i = interfaces; i != NULL; i = i->next)
    gnet_inetaddr_delete ((GInetAddr*) i->data);
  g_list_free (interfaces);

  return iface;
}

/**
 *  gnet_inetaddr_list_interfaces:
 *
 *  Get a list of #GInetAddr interfaces's on this host.  This list
 *  includes all "up" Internet interfaces and the loopback interface,
 *  if it exists.
 *
 *  Note that the Windows version supports a maximum of 10 interfaces.
 *  In Windows NT, Service Pack 4 (or higher) is required.
 *
 *  Returns: A list of #GInetAddr's representing available interfaces.
 *  The caller should delete the list and the addresses.
 *
 **/
GList*
gnet_inetaddr_list_interfaces (void)
{
  GList* list = NULL;
  gint len, lastlen;
  gchar* buf;
  gchar* ptr;
  gint sockfd;
  struct ifconf ifc;


  /* Create a dummy socket */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) return NULL;

  len = 8 * sizeof(struct ifreq);
  lastlen = 0;

  /* Get the list of interfaces.  We might have to try multiple times
     if there are a lot of interfaces. */
  while(1)
    {
      buf = g_new0(gchar, len);

      ifc.ifc_len = len;
      ifc.ifc_buf = buf;
      if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
	{
	  /* Might have failed because our buffer was too small */
	  if (errno != EINVAL || lastlen != 0)
	    {
	      g_free(buf);
	      return NULL;
	    }
	}
      else
	{
	  /* Break if we got all the interfaces */
	  if (ifc.ifc_len == lastlen)
	    break;

	  lastlen = ifc.ifc_len;
	}

      /* Did not allocate big enough buffer - try again */
      len += 8 * sizeof(struct ifreq);
      g_free(buf);
    }


  /* Create the list.  Stevens has a much more complex way of doing
     this, but his is probably much more correct portable.  */
  for (ptr = buf; ptr < (buf + ifc.ifc_len); )
    {
      struct ifreq* ifr = (struct ifreq*) ptr;
      struct sockaddr addr;
      GInetAddr* ia;

#ifdef HAVE_SOCKADDR_SA_LEN
      ptr += sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len;
#else
      ptr += sizeof(struct ifreq);
#endif

      /* Ignore non-AF_INET */
      if (ifr->ifr_addr.sa_family != AF_INET)
	continue;

      /* FIX: Skip colons in name?  Can happen if aliases, maybe. */

      /* Save the address - the next call will clobber it */
      memcpy(&addr, &ifr->ifr_addr, sizeof(addr));

      /* Get the flags */
      ioctl(sockfd, SIOCGIFFLAGS, ifr);

      /* Ignore entries that aren't up or loopback.  Someday we'll
	 write an interface structure and include this stuff. */
      if (!(ifr->ifr_flags & IFF_UP) ||
	  (ifr->ifr_flags & IFF_LOOPBACK))
	continue;

      /* Create an InetAddr for this one and add it to our list */
      ia = g_new0 (GInetAddr, 1);
      ia->ref_count = 1;
      memcpy(&ia->sa, &addr, sizeof(addr));
      list = g_list_prepend (list, ia);
    }

  list = g_list_reverse (list);

  return list;
}
