/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: errormsg.c,v 1.1.1.2 1999-04-01 20:09:16 mwhitson Exp $";

#include "ifhp.h"

/**** ENDINCLUDE ****/

/****************************************************************************
 * char *Errormsg( int err )
 *  returns a printable form of the
 *  errormessage corresponding to the valie of err.
 *  This is the poor man's version of sperror(), not available on all systems
 *  Patrick Powell Tue Apr 11 08:05:05 PDT 1995
 ****************************************************************************/
/****************************************************************************/
#if !defined(HAVE_STRERROR)

# if defined(HAVE_SYS_NERR)
#  if !defined(HAVE_SYS_NERR_DEF)
     extern int sys_nerr;
#  endif
#  define num_errors    (sys_nerr)
# else
#  define num_errors    (-1)            /* always use "errno=%d" */
# endif

# if defined(HAVE_SYS_ERRLIST)
#  if !defined(HAVE_SYS_ERRLIST_DEF)
     extern const char *const sys_errlist[];
#  endif
# else
#  undef  num_errors
#  define num_errors   (-1)            /* always use "errno=%d" */
# endif

#endif

static int in_log;

int udp_open( char *device );
void setstatus( char *msg, char *details, int trace );

const char * Errormsg ( int err )
{
    const char *cp;

	if( err == 0 ){
		cp = "No Error";
	} else {
#if defined(HAVE_STRERROR)
		cp = strerror(err);
#else
# if defined(HAVE_SYS_ERRLIST)
		if (err >= 0 && err < num_errors) {
			cp = sys_errlist[err];
		} else
# endif
		{
			static char msgbuf[32];     /* holds "errno=%d". */
			/* SAFE use of sprintf */
			(void) sprintf (msgbuf, "errno=%d", err);
			cp = msgbuf;
		}
#endif
	}
    return (cp);
}

static char log_buf[LARGEBUFFER];
static char *log_start;

static void log_init()
{
	int n;
	char *s;

	log_buf[0] = 0;
	n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
	if (Name && *Name) {
		(void) plp_snprintf( s, n, "%s ", Name );
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
	}
	(void) plp_snprintf( s, n, "%s ", Time_str(1,0) );
	n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
	(void) plp_snprintf(s, n, "[%d] ", (int)getpid ());
	n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
	log_start = s;
}

static void log_backend( int trace )
{
	int n;
	char *s;

	n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
	if( Trace_on_stderr || (Banner_only && !trace) ){
		(void)plp_snprintf( s, n, "\n" );
		Write_fd_str( 2, log_buf );
		*s = 0;
	}
	if( !Banner_only ){
		setstatus( log_buf, log_start, trace );
	}
}

/* VARARGS2 */
#ifdef HAVE_STDARGS
void logmsg( char *msg,...)
#else
void logmsg(va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    char *msg;
#endif
	int n;
	char *s;
	int err = errno;
    VA_LOCAL_DECL
    VA_START (msg);
    VA_SHIFT (msg, char *);

	if( in_log++ == 0 ){
		log_init();
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void) plp_vsnprintf(s, n, msg, ap);
		log_backend(0);
		in_log = 0;
	}
    VA_END;
	errno = err;
}


/* VARARGS2 */
#ifdef HAVE_STDARGS
void logDebug( char *msg,...)
#else
void logDebug(va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    char *msg;
#endif
	int n;
	char *s;
	int err = errno;
    VA_LOCAL_DECL
    VA_START (msg);
    VA_SHIFT (msg, char *);

	if( in_log++ == 0 ){
		log_init();
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void) plp_vsnprintf(s, n, msg, ap);
		log_backend(1);
		in_log = 0;
	}
    VA_END;
	errno = err;
}

/* VARARGS2 */
#ifdef HAVE_STDARGS
void fatal ( char *msg,...)
#else
void fatal (va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    char *msg;
#endif
	int n;
	char *s;
    VA_LOCAL_DECL
    VA_START (msg);
    VA_SHIFT (msg, char *);
	
	if( in_log++ == 0 ){
		log_init();
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void) plp_vsnprintf(s, n, msg, ap);
		log_backend(0);
		if( Errorcode == 0 ){
			Errorcode = JFAIL;
		}
		cleanup(0);
		in_log = 0;
	}
    VA_END;
}

/* VARARGS2 */
#ifdef HAVE_STDARGS
void logerr (char *msg,...)
#else
void logerr (va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    char *msg;
#endif
	int err = errno;
	int n;
	char *s;
    VA_LOCAL_DECL
    VA_START (msg);
    VA_SHIFT (msg, char *);

	if( in_log++ == 0 ){
		log_init();
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void) plp_vsnprintf(s, n, msg, ap);
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void) plp_snprintf (s, n, " - %s", Errormsg (err));
		log_backend(0);
		in_log = 0;
	}
    VA_END;
    errno = err;
}

/* VARARGS2 */
#ifdef HAVE_STDARGS
void logerr_die ( char *msg,...)
#else
void logerr_die (va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    char *msg;
#endif
	int err = errno;
	int n;
	char *s;
    VA_LOCAL_DECL

    VA_START (msg);
    VA_SHIFT (msg, char *);

	if( in_log++ == 0 ){
		log_init();
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void) plp_vsnprintf (s, n, msg, ap);
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void) plp_snprintf (s, n, " - %s", Errormsg (err));
		log_backend(0);
		if( Errorcode == 0 ){
			Errorcode = JFAIL;
		}
		cleanup(0);
		in_log = 0;
	}
    VA_END;
}

/*
 * Time_str: return "cleaned up" ctime() string...
 *
 * in YY/MO/DY/hr:mn:sc
 * Thu Aug 4 12:34:17 BST 1994 -> 12:34:17
 */

char *Time_str(int shortform, time_t t)
{
    static char buffer[99];
	struct tm *tmptr;
	struct timeval tv;

	tv.tv_usec = 0;
	if( t == 0 ){
		if( gettimeofday( &tv, 0 ) == -1 ){
			logerr_die( "gettimeofday failed");
		}
		t = tv.tv_sec;
	}
	tmptr = localtime( &t );
	if( shortform && Full_time == 0 ){
		plp_snprintf( buffer, sizeof(buffer),
			"%02d:%02d:%02d.%03d",
			tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec,
			(int)(tv.tv_usec/1000) );
	} else {
		plp_snprintf( buffer, sizeof(buffer),
			"%d-%02d-%02d-%02d:%02d:%02d.%03d",
			tmptr->tm_year+1900, tmptr->tm_mon+1, tmptr->tm_mday,
			tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec,
			(int)(tv.tv_usec/1000) );
	}
	return( buffer );
}

/*
 * Write_fd_str( int fd, char *str )
 */

int Write_fd_str( int fd, char *str )
{
	int len;
	if( str && *str ){
		len = strlen( str ); 
		return (Write_fd_len( fd, str, len ) );
	}
	return 0;
}

int Write_fd_len( int fd, char *str, int len )
{
	int i;
	i = 0;
	if( str && len ){
		while( len > 0 ){
			i =  write( fd, str, len );
			if( i <= 0 ) return(i);
			str += i;
			len -= i;
		}
	}
	return i;
}

/***************************************************************************
 * char *Sigstr(n)
 * Return a printable form the the signal
 ***************************************************************************/

struct signame {
    char *str;
    int value;
};

#undef PAIR
#ifndef _UNPROTO_
# define PAIR(X) { #X , X }
#else
# define __string(X) "X"
# define PAIR(X) { __string(X) , X }
#endif

#if !defined(HAVE_SYS_SIGLIST)
struct signame signals[] = {
{ "NO SIGNAL", 0 },
#ifdef SIGHUP
PAIR(SIGHUP),
#endif
#ifdef SIGINT
PAIR(SIGINT),
#endif
#ifdef SIGQUIT
PAIR(SIGQUIT),
#endif
#ifdef SIGILL
PAIR(SIGILL),
#endif
#ifdef SIGTRAP
PAIR(SIGTRAP),
#endif
#ifdef SIGIOT
PAIR(SIGIOT),
#endif
#ifdef SIGABRT
PAIR(SIGABRT),
#endif
#ifdef SIGEMT
PAIR(SIGEMT),
#endif
#ifdef SIGFPE
PAIR(SIGFPE),
#endif
#ifdef SIGKILL
PAIR(SIGKILL),
#endif
#ifdef SIGBUS
PAIR(SIGBUS),
#endif
#ifdef SIGSEGV
PAIR(SIGSEGV),
#endif
#ifdef SIGSYS
PAIR(SIGSYS),
#endif
#ifdef SIGPIPE
PAIR(SIGPIPE),
#endif
#ifdef SIGALRM
PAIR(SIGALRM),
#endif
#ifdef SIGTERM
PAIR(SIGTERM),
#endif
#ifdef SIGURG
PAIR(SIGURG),
#endif
#ifdef SIGSTOP
PAIR(SIGSTOP),
#endif
#ifdef SIGTSTP
PAIR(SIGTSTP),
#endif
#ifdef SIGCONT
PAIR(SIGCONT),
#endif
#ifdef SIGCHLD
PAIR(SIGCHLD),
#endif
#ifdef SIGCLD
PAIR(SIGCLD),
#endif
#ifdef SIGTTIN
PAIR(SIGTTIN),
#endif
#ifdef SIGTTOU
PAIR(SIGTTOU),
#endif
#ifdef SIGIO
PAIR(SIGIO),
#endif
#ifdef SIGPOLL
PAIR(SIGPOLL),
#endif
#ifdef SIGXCPU
PAIR(SIGXCPU),
#endif
#ifdef SIGXFSZ
PAIR(SIGXFSZ),
#endif
#ifdef SIGVTALRM
PAIR(SIGVTALRM),
#endif
#ifdef SIGPROF
PAIR(SIGPROF),
#endif
#ifdef SIGWINCH
PAIR(SIGWINCH),
#endif
#ifdef SIGLOST
PAIR(SIGLOST),
#endif
#ifdef SIGUSR1
PAIR(SIGUSR1),
#endif
#ifdef SIGUSR2
PAIR(SIGUSR2),
#endif
{0,0}
    /* that's all */
};

#else /* HAVE_SYS_SIGLIST */
# ifndef HAVE_SYS_SIGLIST_DEF
   extern const char *sys_siglist[];
# endif
#endif

#ifndef NSIG
# define  NSIG 32
#endif

const char *Sigstr (int n)
{
    static char buf[40];
	const char *s = 0;

	if( n == 0 ){
		return( "No signal");
	}
#ifdef HAVE_SYS_SIGLIST
    if (n < NSIG && n >= 0) {
		s = sys_siglist[n];
	}
#else
	{
	int i;
	for( i = 0; signals[i].str && signals[i].value != n; ++i );
	s = signals[i].str;
	}
#endif
	if( s == 0 ){
		s = buf;
		(void) plp_snprintf (buf, sizeof(buf), "signal %d", n);
	}
    return(s);
}

void setstatus( char *msg, char *details, int trace )
{
	char *str, *save, *s;
	int len, size, minsize, l, n;
	static int active = 0;
	struct stat statb;


	if( active ) return;
	++active;
	if( Status_fd < 0 && Statusfile && stat( Statusfile, &statb) != -1 ){
		/*
		char b[SMALLBUFFER];
		plp_snprintf(b, sizeof(b),"Statusfile %s\n", Statusfile );
		Write_fd_str( 2, b );
		*/
		size = Max_status_size;
		if( size == 0 ) size = 8;
		/*
		plp_snprintf(b,sizeof(b), "setstatus: size %d, max %d\n", size, (int)statb.st_size );
		Write_fd_str( 2, b );
		*/
		if( (statb.st_size/1024) > size ){
			minsize = Min_status_size;
			if( minsize > size ){
				minsize = size/4;
			}
			if( minsize == 0 ){
				minsize = 512;
			} else {
				minsize *= 1024;
			}
			/*
			plp_snprintf(b,sizeof(b), "setstatus: min size %d\n", minsize );
			Write_fd_str( 2, b );
			*/
			save = malloc_or_die(minsize+1,__FILE__,__LINE__);
			Status_fd = open( Statusfile, O_RDWR );
			if( Status_fd >= 0 ){
				if( lseek( Status_fd, -minsize, SEEK_END ) < 0 ){
					logerr_die( "setstatus: cannot seek '%s'", Statusfile );
				}
				for( len = minsize, str = save;
					len > 0 && (l = read( Status_fd, str, len ) ) > 0;
					str += l, len -= l );
				*str = 0;
				if( (s = strchr( save, '\n' )) ){
					str = s+1;
				} else {
					str = save;
				}
				close( Status_fd );
				Status_fd = open( Statusfile, O_WRONLY|O_TRUNC, 0600 );
				for( len = strlen(str);
					len > 0 && (l = write( Status_fd, str, len ) ) > 0;
					str += l, len -= l );
				free(save);
				close( Status_fd );
			}
		}
		Status_fd = open( Statusfile, O_WRONLY|O_APPEND, 0600 );
		/*
		stat( Statusfile, &statb );
		plp_snprintf(b,sizeof(b), "setstatus: new size %d\n", (int)(statb.st_size) );
		Write_fd_str( 2, b );
		*/
	}
	if( Debug || trace ){
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void)plp_snprintf( s, n, "\n" );
	} else {
		msg = details;
		if( (s = strpbrk(msg," \t:")) && *s == ':' && isspace(cval(s+1)) ){
			for(msg = s+1; isspace(cval(msg)); ++msg );
		}
		n = strlen(log_buf); s = log_buf+n; n = sizeof(log_buf)-n;
		(void)plp_snprintf( s, n, " at %s\n", Time_str(1,0) );
	}
	if( Status_fd >= 0 ){
		if( OF_Mode ){
			Write_fd_str(Status_fd, "(of) ");
		}
		Write_fd_str( Status_fd, msg );
	}
	if( Summaryfile  ){
		int is_file = 0;
		if( Summary_fd < 0 ){
			if( strpbrk( Summaryfile, "@%") ){
				Summary_fd = udp_open( Summaryfile );
			} else {
				Summary_fd = open( Summaryfile, O_RDWR|O_CREAT|O_TRUNC, 0755 );
				is_file = 1;
			}
		}
		if( Summary_fd >= 0 ){
			char buffer[SMALLBUFFER];
			if((s = Upperopts['P'-'A']) == 0 ) s = "?????";
			plp_snprintf( buffer, sizeof(buffer), "PRINTER=%s %s", s, msg );
			if( Write_fd_str(Summary_fd,buffer) < 0 || is_file ){
				close( Summary_fd );
				Summary_fd = -1;
			}
		}
	}
	--active;
}

/**********************************************
 * Support a simple one line summary message for status
 * This is handy for debugging and reporting information
 **********************************************/

int udp_open( char *device )
{
	int port, i, fd, err;
	struct hostent *hostent;
	struct sockaddr_in sin;
	struct servent *servent;
	char *s;

	logmsg( "udp_open: '%s'\n",device );
	if( (s = strpbrk( device, "@%" )) == 0 ){
		logmsg( "udp_open: missing port number '%s'\n",device );
		return( -1 );
	}
	if( strpbrk( s+1, "@%" ) ){
		logmsg( "udp_open: two '@' or '%' in name '%s'\n",
			device );
		return( -1 );
	}
	port = atoi( s+1 );
	if( port <= 0 ){
		servent = getservbyname( s+1, "udp" );
		if( servent ){
			port = ntohs( servent->s_port );
		}
	}
	if( port <= 0 ){
		logmsg( "udp_open: bad port number '%s'\n",s+1 );
		return( -1 );
	}
	i = *s;
	*s = 0;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = -1;

	if( (hostent = gethostbyname(device)) ){
		/*
		 * set up the address information
		 */
		if( hostent->h_addrtype != AF_INET ){
			logmsg( "udp_open: bad address type for host '%s'\n",
				device);
		}
		memcpy( &sin.sin_addr, hostent->h_addr, hostent->h_length );
	} else {
		sin.sin_addr.s_addr = inet_addr(device);
	}
	*s = i;
	if( sin.sin_addr.s_addr == -1){
		logmsg("udp_open: unknown host '%s'\n", device);
		return(-1);
	}
	sin.sin_port = htons( port );
	logmsg( "udp_open: destination '%s' port %d\n",
		inet_ntoa( sin.sin_addr ), ntohs( sin.sin_port ) );
	fd = socket (AF_INET, SOCK_DGRAM, 0);
	err = errno;
	if (fd < 0) {
		logmsg("udp_open: socket call failed - %s\n", Errormsg(err) );
		return( -1 );
	}
	i = connect (fd, (struct sockaddr *) & sin, sizeof (sin));
	err = errno;

	if( i < 0 ){
		logmsg("udp_open: connect to '%s port %d' failed - %s\n",
			inet_ntoa( sin.sin_addr ), ntohs( sin.sin_port ),
			Errormsg(errno) );
		close(fd);
		fd = -1;
	}
	return( fd );
}
