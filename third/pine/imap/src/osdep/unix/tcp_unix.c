/*
 * Program:	UNIX TCP/IP routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	1 August 1988
 * Last Edited:	8 November 2000
 * 
 * The IMAP toolkit provided in this Distribution is
 * Copyright 2000 University of Washington.
 * The full text of our legal notices is contained in the file called
 * CPYRIGHT, included with this Distribution.
 */

#undef write			/* don't use redefined write() */
 
static tcptimeout_t tmoh = NIL;	/* TCP timeout handler routine */
static long ttmo_open = 0;	/* TCP timeouts, in seconds */
static long ttmo_read = 0;
static long ttmo_write = 0;
static long rshtimeout = 15;	/* rsh timeout */
static char *rshcommand = NIL;	/* rsh command */
static char *rshpath = NIL;	/* rsh path */
static long sshtimeout = 15;	/* ssh timeout */
static char *sshcommand = NIL;	/* ssh command */
static char *sshpath = NIL;	/* ssh path */
static long allowreversedns =	/* allow reverse DNS lookup */
#ifdef DISABLE_REVERSE_DNS_LOOKUP
  NIL	/* Not recommended, especially if using Kerberos authentication */
#else
  T
#endif
  ;

extern long maxposint;		/* get this from write.c */

/* Local function prototypes */

int tcp_socket_open (struct sockaddr_in *sin,char *tmp,int *ctr,char *hst,
		     unsigned long port);
long tcp_abort (TCPSTREAM *stream);
char *tcp_name (struct sockaddr_in *sin,long flag);

/* TCP/IP manipulate parameters
 * Accepts: function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */

void *tcp_parameters (long function,void *value)
{
  void *ret = NIL;
  switch ((int) function) {
  case SET_TIMEOUT:
    tmoh = (tcptimeout_t) value;
  case GET_TIMEOUT:
    ret = (void *) tmoh;
    break;
  case SET_OPENTIMEOUT:
    ttmo_open = (long) value;
  case GET_OPENTIMEOUT:
    ret = (void *) ttmo_open;
    break;
  case SET_READTIMEOUT:
    ttmo_read = (long) value;
  case GET_READTIMEOUT:
    ret = (void *) ttmo_read;
    break;
  case SET_WRITETIMEOUT:
    ttmo_write = (long) value;
  case GET_WRITETIMEOUT:
    ret = (void *) ttmo_write;
    break;
  case SET_ALLOWREVERSEDNS:
    allowreversedns = (long) value;
  case GET_ALLOWREVERSEDNS:
    ret = (void *) allowreversedns;
    break;

  case SET_RSHTIMEOUT:
    rshtimeout = (long) value;
  case GET_RSHTIMEOUT:
    ret = (void *) rshtimeout;
    break;
  case SET_RSHCOMMAND:
    if (rshcommand) fs_give ((void **) &rshcommand);
    rshcommand = cpystr ((char *) value);
  case GET_RSHCOMMAND:
    ret = (void *) rshcommand;
    break;
  case SET_RSHPATH:
    if (rshpath) fs_give ((void **) &rshpath);
    rshpath = cpystr ((char *) value);
  case GET_RSHPATH:
    ret = (void *) rshpath;
    break;
  case SET_SSHTIMEOUT:
    sshtimeout = (long) value;
  case GET_SSHTIMEOUT:
    ret = (void *) sshtimeout;
    break;
  case SET_SSHCOMMAND:
    if (sshcommand) fs_give ((void **) &sshcommand);
    sshcommand = cpystr ((char *) value);
  case GET_SSHCOMMAND:
    ret = (void *) sshcommand;
    break;
  case SET_SSHPATH:
    if (sshpath) fs_give ((void **) &sshpath);
    sshpath = cpystr ((char *) value);
  case GET_SSHPATH:
    ret = (void *) sshpath;
    break;
  }
  return ret;
}

/* TCP/IP open
 * Accepts: host name
 *	    contact service name
 *	    contact port number and optional silent flag
 * Returns: TCP/IP stream if success else NIL
 */

TCPSTREAM *tcp_open (char *host,char *service,unsigned long port)
{
  TCPSTREAM *stream = NIL;
  int i,sock;
  int ctr = 0;
  int silent = (port & NET_SILENT) ? T : NIL;
  int *ctrp = &ctr;
  char *s;
  struct sockaddr_in sin;
  struct hostent *he;
  char hostname[MAILTMPLEN];
  char tmp[MAILTMPLEN];
  struct servent *sv = NIL;
  blocknotify_t bn = (blocknotify_t) mail_parameters (NIL,GET_BLOCKNOTIFY,NIL);
  void *data;
				/* don't do open timeout if this requested */
  if (port & NET_NOOPENTIMEOUT) ctrp = NIL;
  port &= 0xffff;		/* erase flags */
				/* lookup service */
  if (service && (sv = getservbyname (service,"tcp")))
    port = ntohs (sin.sin_port = sv->s_port);
 				/* copy port number in network format */
  else sin.sin_port = htons (port);
  /* The domain literal form is used (rather than simply the dotted decimal
     as with other Unix programs) because it has to be a valid "host name"
     in mailsystem terminology. */
				/* look like domain literal? */
  if (host[0] == '[' && host[(strlen (host))-1] == ']') {
    strcpy (hostname,host+1);	/* yes, copy number part */
    hostname[(strlen (hostname))-1] = '\0';
    if ((sin.sin_addr.s_addr = inet_addr (hostname)) != -1) {
      sin.sin_family = AF_INET;	/* family is always Internet */
      strcpy (hostname,host);	/* hostname is user's argument */
				/* get an open socket for this system */
      sock = tcp_socket_open (&sin,tmp,ctrp,hostname,port);
    }
    else {
      sprintf (tmp,"Bad format domain-literal: %.80s",host);
      mm_log (tmp,ERROR);
      return NIL;
    }
  }

  else {			/* lookup host name */
    (*bn) (BLOCK_DNSLOOKUP,NIL);/* quell alarms */
    data = (*bn) (BLOCK_SENSITIVE,NIL);
    if (he = gethostbyname (lcase (strcpy (hostname,host)))) {
      (*bn) (BLOCK_NONSENSITIVE,data);
      (*bn) (BLOCK_NONE,NIL);
				/* copy address type */
      sin.sin_family = he->h_addrtype;
				/* copy host name */
      strcpy (hostname,he->h_name);
#ifdef HOST_NOT_FOUND		/* muliple addresses only on DNS systems */
      for (sock = -1,i = 0; (sock < 0) && (s = he->h_addr_list[i]); i++) {
	if (i && !silent) mm_log (tmp,WARN);
	memcpy (&sin.sin_addr,s,he->h_length);
	(*bn) (BLOCK_TCPOPEN,NIL);
	sock = tcp_socket_open (&sin,tmp,ctrp,hostname,port);
	(*bn) (BLOCK_NONE,NIL);
      }
#else				/* the one true address then */
      memcpy (&sin.sin_addr,he->h_addr,he->h_length);
      (*bn) (BLOCK_DNSLOOKUP,NIL);
      sock = tcp_socket_open (&sin,tmp,ctrp,hostname,port);
      (*bn) (BLOCK_NONE,NIL);
#endif
    }
    else {
      (*bn) (BLOCK_NONSENSITIVE,data);
      (*bn) (BLOCK_NONE,NIL);
      sprintf (tmp,"No such host as %.80s",host);
      mm_log (tmp,ERROR);
      return NIL;
    }
  }
  if (sock >= 0)  {		/* won */
    stream = (TCPSTREAM *) memset (fs_get (sizeof (TCPSTREAM)),0,
				   sizeof (TCPSTREAM));
    stream->port = port;	/* port number */
				/* init sockets */
    stream->tcpsi = stream->tcpso = sock;
				/* stash in the snuck-in byte */
    if (stream->ictr = ctr) *(stream->iptr = stream->ibuf) = tmp[0];
				/* copy official host name */
    stream->host = cpystr (hostname);
  }
  else if (!silent) mm_log (tmp,ERROR);
  return stream;		/* return success */
}

/* Open a TCP socket
 * Accepts: Internet socket address block
 *	    scratch buffer
 *	    pointer to "first byte read in" storage or NIL
 *	    host name for error message
 *	    port number for error message
 * Returns: socket if success, else -1 with error string in scratch buffer
 */

int tcp_socket_open (struct sockaddr_in *sin,char *tmp,int *ctr,char *hst,
		     unsigned long port)
{
  int i,ti,sock,flgs;
  time_t now;
  struct protoent *pt = getprotobyname ("ip");
  fd_set fds,efds;
  struct timeval tmo;
  sprintf (tmp,"Trying IP address [%s]",inet_ntoa (sin->sin_addr));
  mm_log (tmp,NIL);
				/* make a socket */
  if ((sock = socket (sin->sin_family,SOCK_STREAM,pt ? pt->p_proto : 0)) < 0) {
    sprintf (tmp,"Unable to create TCP socket: %s",strerror (errno));
    return -1;
  }
  if (!ctr) {			/* no open timeout wanted */
				/* open connection */
    while ((i = connect (sock,(struct sockaddr *) sin,
			 sizeof (struct sockaddr_in))) < 0 && errno == EINTR);
    if (i < 0) {		/* failed? */
      sprintf (tmp,"Can't connect to %.80s,%lu: %s",hst,port,strerror (errno));
      close (sock);		/* flush socket */
      return -1;
    }
  }

  else {			/*  want open timeout */
				/* get current socket flags */
    flgs = fcntl (sock,F_GETFL,0);
				/* set non-blocking */
    fcntl (sock,F_SETFL,flgs | FNDELAY);
				/* open connection */
    while ((i = connect (sock,(struct sockaddr *) sin,
		       sizeof (struct sockaddr_in))) < 0 && errno == EINTR);
    if (i < 0) switch (errno) {	/* failed? */
    case EAGAIN:		/* DG brain damage */
    case EINPROGRESS:		/* what we expect to happen */
    case EISCONN:		/* restart after interrupt? */
    case EADDRINUSE:		/* restart after interrupt? */
      break;			/* well, not really, it was interrupted */
    default:
      sprintf (tmp,"Can't connect to %.80s,%lu: %s",hst,port,strerror (errno));
      close (sock);		/* flush socket */
      return -1;
    }
    now = time (0);		/* open timeout */
    ti = ttmo_open ? now + ttmo_open : 0;
    tmo.tv_usec = 0;
    FD_ZERO (&fds);		/* initialize selection vector */
    FD_ZERO (&efds);		/* handle errors too */
    FD_SET (sock,&fds);	/* block for error or writeable */
    FD_SET (sock,&efds);
    do {			/* block under timeout */
      tmo.tv_sec = ti ? ti - now : 0;
      i = select (sock+1,0,&fds,&efds,ti ? &tmo : 0);
      now = time (0);
    } while (((i < 0) && (errno == EINTR)) || (ti && !i && (ti > now)));
#ifdef KERBEROS
    if (port == 1109) {
      /* For kpop, we have to send kerberos authentication before we
       * can read anything.  So we have to do it here instead of in
       * the pop3 code. */
      KTEXT_ST ticket;

      if (krb_sendauth (0,sock,&ticket,"pop",hst,krb_realmofhost (hst),
			0,NULL,NULL,NULL,NULL,NULL,"KPOPV0.1") != KSUCCESS) {
	close (sock);
	return -1;		/* failure */
      }
    }
#endif  
    if (i > 0) {		/* success, make sure really connected */
      fcntl (sock,F_SETFL,flgs);/* restore blocking status */
      /* This used to be a zero-byte read(), but that crashes Solaris */
				/* get socket status */
      while (((i = *ctr = read (sock,tmp,1)) < 0) && (errno == EINTR));
    }	
    if (i <= 0) {		/* timeout or error? */
      i = i ? errno : ETIMEDOUT;/* determine error code */
      close (sock);		/* flush socket */
      errno = i;		/* return error code */
      sprintf (tmp,"Connection failed to %.80s,%lu: %s",hst,port,
	       strerror (errno));
      return -1;
    }
  }
  return sock;			/* return the socket */
}
  
/* TCP/IP authenticated open
 * Accepts: host name
 *	    service name
 *	    returned user name buffer
 * Returns: TCP/IP stream if success else NIL
 */

#define MAXARGV 20

TCPSTREAM *tcp_aopen (NETMBX *mb,char *service,char *usrbuf)
{
  TCPSTREAM *stream = NIL;
  struct hostent *he;
  char host[MAILTMPLEN],tmp[MAILTMPLEN],*path,*argv[MAXARGV+1];
  int i,ti,pipei[2],pipeo[2];
  time_t now;
  struct timeval tmo;
  fd_set fds,efds;
  blocknotify_t bn = (blocknotify_t) mail_parameters (NIL,GET_BLOCKNOTIFY,NIL);
  void *data;
  if (*service == '*') {	/* want ssh? */
				/* return immediately if ssh disabled */
    if (!(sshpath && (ti = sshtimeout))) return NIL;
				/* ssh command prototype defined yet? */
    if (!sshcommand) sshcommand = cpystr ("%s %s -l %s exec /etc/r%sd");
  }
  else if (ti = rshtimeout) {	/* set rsh timeout */
				/* rsh path/command prototypes defined yet? */
    if (!rshpath) rshpath = cpystr (RSHPATH);
    if (!rshcommand) rshcommand = cpystr ("%s %s -l %s exec /etc/r%sd");
  }
  else return NIL;		/* rsh disabled */
				/* look like domain literal? */
  if (mb->host[0] == '[' && mb->host[i = (strlen (mb->host))-1] == ']') {
    strcpy (host,mb->host+1);	/* yes, copy without brackets */
    host[i-1] = '\0';
    if (inet_addr (host) == -1) {
      sprintf (tmp,"Bad format domain-literal: %.80s",host);
      mm_log (tmp,ERROR);
      return NIL;
    }
  }
  else {			/* note that Unix requires lowercase! */
    (*bn) (BLOCK_DNSLOOKUP,NIL);
    data = (*bn) (BLOCK_SENSITIVE,NIL);
    if (he = gethostbyname (lcase (strcpy (host,mb->host))))
      strcpy (host,he->h_name);
    (*bn) (BLOCK_NONSENSITIVE,data);
    (*bn) (BLOCK_NONE,NIL);
  }

  if (*service == '*')		/* build ssh command */
    sprintf (tmp,sshcommand,sshpath,host,
	     mb->user[0] ? mb->user : myusername (),service + 1);
  else sprintf (tmp,rshcommand,rshpath,host,
		mb->user[0] ? mb->user : myusername (),service);
  for (i = 1,path = argv[0] = strtok (tmp," ");
       (i < MAXARGV) && (argv[i] = strtok (NIL," ")); i++);
  argv[i] = NIL;		/* make sure argv tied off */
				/* make command pipes */
  if (pipe (pipei) < 0) return NIL;
  if (pipe (pipeo) < 0) {
    close (pipei[0]); close (pipei[1]);
    return NIL;
  }
  (*bn) (BLOCK_TCPOPEN,NIL);	/* quell alarm up here for NeXT */
  if ((i = fork ()) < 0) {	/* make inferior process */
    close (pipei[0]); close (pipei[1]);
    close (pipeo[0]); close (pipeo[1]);
    return NIL;
  }
  if (!i) {			/* if child */
    alarm (0);			/* never have alarms in children */
    if (!fork ()) {		/* make grandchild so it's inherited by init */
      int maxfd = max (20,max (max(pipei[0],pipei[1]),max(pipeo[0],pipeo[1])));
      dup2 (pipei[1],1);	/* parent's input is my output */
      dup2 (pipei[1],2);	/* parent's input is my error output too */
      dup2 (pipeo[0],0);	/* parent's output is my input */
				/* close all unnecessary descriptors */
      for (i = 3; i <= maxfd; i++) close (i);
      setpgrp (0,getpid ());	/* be our own process group */
      execv (path,argv);	/* now run it */
    }
    _exit (1);			/* child is done */
  }
  grim_pid_reap (i,NIL);	/* reap child; grandchild now owned by init */
  close (pipei[1]);		/* close child's side of the pipes */
  close (pipeo[0]);

				/* create TCP/IP stream */
  stream = (TCPSTREAM *) memset (fs_get (sizeof (TCPSTREAM)),0,
				 sizeof (TCPSTREAM));
				/* copy remote host name from argument */
  stream->remotehost = cpystr (stream->host = cpystr (host));
  stream->tcpsi = pipei[0];	/* init sockets */
  stream->tcpso = pipeo[1];
  stream->ictr = 0;		/* init input counter */
  stream->port = 0xffffffff;	/* no port number */
  ti += now = time (0);		/* open timeout */
  tmo.tv_usec = 0;		/* initialize usec timeout */
  FD_ZERO (&fds);		/* initialize selection vector */
  FD_ZERO (&efds);		/* handle errors too */
  FD_SET (stream->tcpsi,&fds);	/* set bit in selection vector */
  FD_SET (stream->tcpsi,&efds);	/* set bit in error selection vector */
  FD_SET (stream->tcpso,&efds);	/* set bit in error selection vector */
  do {				/* block under timeout */
    tmo.tv_sec = ti - now;
    i = select (max (stream->tcpsi,stream->tcpso)+1,&fds,0,&efds,&tmo);
    now = time (0);
  } while (((i < 0) && (errno == EINTR)) || (ti && !i && (ti > now)));
  if (i <= 0) {			/* timeout or error? */
    sprintf (tmp,i ? "error in %s to IMAP server" :
	     "%s to IMAP server timed out",(*service == '*') ? "ssh" : "rsh");
    mm_log (tmp,WARN);
    tcp_close (stream);		/* punt stream */
    stream = NIL;
  }
  (*bn) (BLOCK_NONE,NIL);
				/* return user name */
  strcpy (usrbuf,mb->user[0] ? mb->user : myusername ());
  return stream;		/* return success */
}

/* TCP/IP receive line
 * Accepts: TCP/IP stream
 * Returns: text line string or NIL if failure
 */

char *tcp_getline (TCPSTREAM *stream)
{
  int n,m;
  char *st,*ret,*stp;
  char c = '\0';
  char d;
				/* make sure have data */
  if (!tcp_getdata (stream)) return NIL;
  st = stream->iptr;		/* save start of string */
  n = 0;			/* init string count */
  while (stream->ictr--) {	/* look for end of line */
    d = *stream->iptr++;	/* slurp another character */
    if ((c == '\015') && (d == '\012')) {
      ret = (char *) fs_get (n--);
      memcpy (ret,st,n);	/* copy into a free storage string */
      ret[n] = '\0';		/* tie off string with null */
      return ret;
    }
    n++;			/* count another character searched */
    c = d;			/* remember previous character */
  }
				/* copy partial string from buffer */
  memcpy ((ret = stp = (char *) fs_get (n)),st,n);
				/* get more data from the net */
  if (!tcp_getdata (stream)) fs_give ((void **) &ret);
				/* special case of newline broken by buffer */
  else if ((c == '\015') && (*stream->iptr == '\012')) {
    stream->iptr++;		/* eat the line feed */
    stream->ictr--;
    ret[n - 1] = '\0';		/* tie off string with null */
  }
				/* else recurse to get remainder */
  else if (st = tcp_getline (stream)) {
    ret = (char *) fs_get (n + 1 + (m = strlen (st)));
    memcpy (ret,stp,n);		/* copy first part */
    memcpy (ret + n,st,m);	/* and second part */
    fs_give ((void **) &stp);	/* flush first part */
    fs_give ((void **) &st);	/* flush second part */
    ret[n + m] = '\0';		/* tie off string with null */
  }
  return ret;
}

/* TCP/IP receive buffer
 * Accepts: TCP/IP stream
 *	    size in bytes
 *	    buffer to read into
 * Returns: T if success, NIL otherwise
 */

long tcp_getbuffer (TCPSTREAM *stream,unsigned long size,char *s)
{
  unsigned long n;
				/* make sure socket still alive */
  if (stream->tcpsi < 0) return NIL;
				/* can transfer bytes from buffer? */
  if (n = min (size,stream->ictr)) {
    memcpy (s,stream->iptr,n);	/* yes, slurp as much as we can from it */
    s += n;			/* update pointer */
    stream->iptr +=n;
    size -= n;			/* update # of bytes to do */
    stream->ictr -=n;
  }
  if (size) {
    int i;
    fd_set fds,efds;
    struct timeval tmo;
    time_t t = time (0);
    blocknotify_t bn=(blocknotify_t) mail_parameters (NIL,GET_BLOCKNOTIFY,NIL);
    (*bn) (BLOCK_TCPREAD,NIL);
    while (size > 0) {		/* until request satisfied */
      time_t tl = time (0);
      time_t now = tl;
      int ti = ttmo_read ? now + ttmo_read : 0;
      tmo.tv_usec = 0;
      FD_ZERO (&fds);		/* initialize selection vector */
      FD_ZERO (&efds);		/* handle errors too */
      FD_SET (stream->tcpsi,&fds);
      FD_SET (stream->tcpsi,&efds);
      errno = NIL;		/* block and read */
      do {			/* block under timeout */
	tmo.tv_sec = ti ? ti - now : 0;
	i = select (stream->tcpsi+1,&fds,0,&efds,ti ? &tmo : 0);
	now = time (0);
      } while (((i < 0) && (errno == EINTR)) || (ti && !i && (ti > now)));
      if (i > 0) {		/* select says there's data to read? */
	while (((i = read (stream->tcpsi,s,(int) min (maxposint,size))) < 0) &&
	       (errno == EINTR));
	if (i < 1) return tcp_abort (stream);
	s += i;			/* point at new place to write */
	size -= i;		/* reduce byte count */
      }
      else if (i || !tmoh || !(*tmoh) (now - t,now - tl))
	return tcp_abort (stream);
    }
    (*bn) (BLOCK_NONE,NIL);
  }
  *s = '\0';			/* tie off string */
  return T;
}

/* TCP/IP receive data
 * Accepts: TCP/IP stream
 * Returns: T if success, NIL otherwise
 */

long tcp_getdata (TCPSTREAM *stream)
{
  int i;
  fd_set fds,efds;
  struct timeval tmo;
  time_t t = time (0);
  blocknotify_t bn = (blocknotify_t) mail_parameters (NIL,GET_BLOCKNOTIFY,NIL);
  if (stream->tcpsi < 0) return NIL;
  (*bn) (BLOCK_TCPREAD,NIL);
  while (stream->ictr < 1) {	/* if nothing in the buffer */
    time_t tl = time (0);	/* start of request */
    time_t now = tl;
    int ti = ttmo_read ? now + ttmo_read : 0;
    tmo.tv_usec = 0;
    FD_ZERO (&fds);		/* initialize selection vector */
    FD_ZERO (&efds);		/* handle errors too */
    FD_SET (stream->tcpsi,&fds);/* set bit in selection vector */
    FD_SET(stream->tcpsi,&efds);/* set bit in error selection vector */
    errno = NIL;		/* block and read */
    do {			/* block under timeout */
      tmo.tv_sec = ti ? ti - now : 0;
      i = select (stream->tcpsi+1,&fds,0,&efds,ti ? &tmo : 0);
      now = time (0);
    } while (((i < 0) && (errno == EINTR)) || (ti && !i && (ti > now)));
    if (!i) {			/* timeout? */
      if (tmoh && ((*tmoh) (now - t,now - tl))) continue;
      else return tcp_abort (stream);
    }
    else if (i < 0) return tcp_abort (stream);
    while (((i = read (stream->tcpsi,stream->ibuf,BUFLEN)) < 0) &&
	   (errno == EINTR));
    if (i < 1) return tcp_abort (stream);
    stream->iptr = stream->ibuf;/* point at TCP buffer */
    stream->ictr = i;		/* set new byte count */
  }
  (*bn) (BLOCK_NONE,NIL);
  return T;
}

/* TCP/IP send string as record
 * Accepts: TCP/IP stream
 *	    string pointer
 * Returns: T if success else NIL
 */

long tcp_soutr (TCPSTREAM *stream,char *string)
{
  return tcp_sout (stream,string,(unsigned long) strlen (string));
}


/* TCP/IP send string
 * Accepts: TCP/IP stream
 *	    string pointer
 *	    byte count
 * Returns: T if success else NIL
 */

long tcp_sout (TCPSTREAM *stream,char *string,unsigned long size)
{
  int i;
  fd_set fds,efds;
  struct timeval tmo;
  time_t t = time (0);
  blocknotify_t bn = (blocknotify_t) mail_parameters (NIL,GET_BLOCKNOTIFY,NIL);
  if (stream->tcpso < 0) return NIL;
  (*bn) (BLOCK_TCPWRITE,NIL);
  while (size > 0) {		/* until request satisfied */
    time_t tl = time (0);	/* start of request */
    time_t now = tl;
    int ti = ttmo_write ? now + ttmo_write : 0;
    tmo.tv_usec = 0;
    FD_ZERO (&fds);		/* initialize selection vector */
    FD_ZERO (&efds);		/* handle errors too */
    FD_SET (stream->tcpso,&fds);/* set bit in selection vector */
    FD_SET(stream->tcpso,&efds);/* set bit in error selection vector */
    errno = NIL;		/* block and write */
    do {			/* block under timeout */
      tmo.tv_sec = ti ? ti - now : 0;
      i = select (stream->tcpso+1,0,&fds,&efds,ti ? &tmo : 0);
      now = time (0);
    } while (((i < 0) && (errno == EINTR)) || (ti && !i && (ti > now)));
    if (!i) {			/* timeout? */
      if (tmoh && ((*tmoh) (now - t,now - tl))) continue;
      else return tcp_abort (stream);
    }
    else if (i < 0) return tcp_abort (stream);
    while (((i = write (stream->tcpso,string,size)) < 0) && (errno == EINTR));
    if (i < 0) return tcp_abort (stream);
    size -= i;			/* how much we sent */
    string += i;
  }
  (*bn) (BLOCK_NONE,NIL);
  return T;			/* all done */
}

/* TCP/IP close
 * Accepts: TCP/IP stream
 */

void tcp_close (TCPSTREAM *stream)
{
  tcp_abort (stream);		/* nuke the stream */
				/* flush host names */
  if (stream->host) fs_give ((void **) &stream->host);
  if (stream->remotehost) fs_give ((void **) &stream->remotehost);
  if (stream->localhost) fs_give ((void **) &stream->localhost);
  fs_give ((void **) &stream);	/* flush the stream */
}


/* TCP/IP abort stream
 * Accepts: TCP/IP stream
 * Returns: NIL always
 */

long tcp_abort (TCPSTREAM *stream)
{
  blocknotify_t bn = (blocknotify_t) mail_parameters (NIL,GET_BLOCKNOTIFY,NIL);
  if (stream->tcpsi >= 0) {	/* no-op if no socket */
    (*bn) (BLOCK_TCPCLOSE,NIL);
    close (stream->tcpsi);	/* nuke the socket */
    if (stream->tcpsi != stream->tcpso) close (stream->tcpso);
    stream->tcpsi = stream->tcpso = -1;
  }
  (*bn) (BLOCK_NONE,NIL);
  return NIL;
}

/* TCP/IP get host name
 * Accepts: TCP/IP stream
 * Returns: host name for this stream
 */

char *tcp_host (TCPSTREAM *stream)
{
  return stream->host;		/* use tcp_remotehost() if want guarantees */
}


/* TCP/IP get remote host name
 * Accepts: TCP/IP stream
 * Returns: host name for this stream
 */

char *tcp_remotehost (TCPSTREAM *stream)
{
  if (!stream->remotehost) {
    struct sockaddr_in sin;
    int sinlen = sizeof (struct sockaddr_in);
    stream->remotehost =	/* get socket's peer name */
      getpeername (stream->tcpsi,(struct sockaddr *) &sin,(void *) &sinlen) ?
	cpystr (stream->host) : tcp_name (&sin,NIL);
  }
  return stream->remotehost;
}


/* TCP/IP return port for this stream
 * Accepts: TCP/IP stream
 * Returns: port number for this stream
 */

unsigned long tcp_port (TCPSTREAM *stream)
{
  return stream->port;		/* return port number */
}


/* TCP/IP get local host name
 * Accepts: TCP/IP stream
 * Returns: local host name
 */

char *tcp_localhost (TCPSTREAM *stream)
{
  if (!stream->localhost) {
    struct sockaddr_in sin;
    int sinlen = sizeof (struct sockaddr_in);
    stream->localhost =		/* get socket's name */
      ((stream->port & 0xffff000) ||
       getsockname (stream->tcpsi,(struct sockaddr *) &sin,(void *) &sinlen)) ?
	 cpystr (mylocalhost ()) : tcp_name (&sin,NIL);
  }
  return stream->localhost;	/* return local host name */
}

/* TCP/IP get client host name (server calls only)
 * Returns: client host name
 */

static char *myClientHost = NIL;

char *tcp_clienthost ()
{
  if (!myClientHost) {
    struct sockaddr_in sin;
    int sinlen = sizeof (struct sockaddr_in);
				/* get stdin's peer name */
    myClientHost = getpeername (0,(struct sockaddr *) &sin,(void *) &sinlen) ?
      cpystr ("UNKNOWN") : tcp_name (&sin,T);
  }
  return myClientHost;
}


/* TCP/IP get server host name (server calls only)
 * Returns: server host name
 */

static char *myServerHost = NIL;
static long myServerPort = -1;

char *tcp_serverhost ()
{
  if (!myServerHost) {
    struct sockaddr_in sin;
    int sinlen = sizeof (struct sockaddr_in);
				/* get stdin's name */
    if (getsockname (0,(struct sockaddr *) &sin,(void *) &sinlen))
      myServerHost = cpystr (mylocalhost ());
    else {
      myServerHost = tcp_name (&sin,NIL);
      myServerPort = ntohs (sin.sin_port);
    }
  }
  return myServerHost;
}


/* TCP/IP get server port number (server calls only)
 * Returns: server port number
 */

long tcp_serverport ()
{
  if (!myServerHost) tcp_serverhost ();
  return myServerPort;
}

/* TCP/IP return canonical form of host name
 * Accepts: host name
 * Returns: canonical form of host name
 */

char *tcp_canonical (char *name)
{
  char *ret,host[MAILTMPLEN];
  struct hostent *he;
  blocknotify_t bn = (blocknotify_t) mail_parameters (NIL,GET_BLOCKNOTIFY,NIL);
  void *data;
				/* look like domain literal? */
  if (name[0] == '[' && name[strlen (name) - 1] == ']') return name;
  (*bn) (BLOCK_DNSLOOKUP,NIL);	/* quell alarms */
  data = (*bn) (BLOCK_SENSITIVE,NIL);
				/* note that Unix requires lowercase! */
  ret = (he = gethostbyname (lcase (strcpy (host,name)))) ? he->h_name : name;
  (*bn) (BLOCK_NONSENSITIVE,data);
  (*bn) (BLOCK_NONE,NIL);	/* alarms OK now */
  return ret;
}


/* TCP/IP return name from socket
 * Accepts: socket
 *	    verbose flag
 * Returns: cpystr name
 */

char *tcp_name (struct sockaddr_in *sin,long flag)
{
  char *s,tmp[MAILTMPLEN];
  if (allowreversedns) {
    struct hostent *he;
    blocknotify_t bn = (blocknotify_t)mail_parameters(NIL,GET_BLOCKNOTIFY,NIL);
    void *data;
    (*bn) (BLOCK_DNSLOOKUP,NIL); /* quell alarms */
    data = (*bn) (BLOCK_SENSITIVE,NIL);
				/* translate address to name */
    if (!(he = gethostbyaddr ((char *) &sin->sin_addr,
			      sizeof (struct in_addr),sin->sin_family)))
      sprintf (s = tmp,"[%s]",inet_ntoa (sin->sin_addr));
    else if (flag) sprintf (s = tmp,"%s [%s]",he->h_name,
			    inet_ntoa (sin->sin_addr));
    else s = he->h_name;
    (*bn) (BLOCK_NONSENSITIVE,data);
    (*bn) (BLOCK_NONE,NIL);	/* alarms OK now */
  }
  else sprintf (s = tmp,"[%s]",inet_ntoa (sin->sin_addr));
  return cpystr (s);
}
