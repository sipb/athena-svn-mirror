/* 
 * Copyright 1987 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information,
 * please see the file <mit-copyright.h>.
 */

/*
 * This program is run on slave servers, to catch updates "pushed"
 * from the master kerberos server in a realm.
 */

#include <mit-copyright.h>

#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <krb.h>
#include <krbports.h>
#include <krb_db.h>
#include <klog.h>
#include <prot.h>
#include <kdc.h>

#include "kprop.h"

#ifdef POSIX
/* POSIX means we don't have flock... */
#define flock(f,c)	emul_flock(f,c)
#ifndef LOCK_SH
#define   LOCK_SH   1    /* shared lock */
#define   LOCK_EX   2    /* exclusive lock */
#define   LOCK_NB   4    /* don't block when locking */
#define   LOCK_UN   8    /* unlock */
#endif
#endif

static char kprop_version[KPROP_PROT_VERSION_LEN] = KPROP_PROT_VERSION;

extern int errno;
#ifndef HAVE_SYS_ERRLIST_DECL
extern char *sys_errlist[];
#endif
int     debug = 0;

char   *logfile = K_LOGFIL;

char    errmsg[256];
int     pause_int = 300;	/* 5 minutes in seconds */
unsigned long get_data_checksum();
static void SlowDeath();
static char    buf[KPROP_BUFSIZ+64 /* leave room for private msg overhead */];
static int use_inetd = 0;

static void usage()
{
  char *umsg="\nUsage: kpropd [-r realm] [-s srvtab] [-d database] [-u port] [-l logfile]\n\t[-i] [-c command] [-C arg] fname";
  klog(L_KRB_PERR, umsg);
  if (!use_inetd) {
    fprintf(stderr, "%s\n\n", umsg);
  }
  SlowDeath();
}

#define KPROPD_SUCCESS 0
#define KPROPD_RECV_LOCAL_FAILURE 1
#define KPROPD_RECV_REMOTE_FAILURE 2

char *srvtab = "";
char *local_db = DBM_FILE;
char    local_file[256];
char    local_temp[256];
char my_realm[REALM_SZ];
char *base_cmd = "kdb_util";
char *base_arg = "load";

main(argc, argv)
    int     argc;
    char  **argv;
{
    struct sockaddr_in from;
    struct sockaddr_in sin;
    struct servent *sp;
    int     s, s2;
    int     from_len;
    unsigned long cksum_read;
    unsigned long cksum_calc;
    KRB_INT32  length;
    long    kerror;
    int c;
    extern char *optarg;
    extern int optind;
    int rflag = 0;
    int use_port = 0;

    if (argv[argc - 1][0] == 'k' && isdigit(argv[argc - 1][1])) {
	argc--;			/* ttys file hack */
    }
    while ((c = getopt(argc, argv, "r:s:d:l:u:ic:C:")) != EOF) {
	switch(c) {
	case 'r':
	    rflag++;
	    strcpy(my_realm, optarg);
	    break;
	case 's':
	    srvtab = optarg;
	    break;
	case 'd':
	    local_db = optarg;
	    break;
	case 'l':
	    logfile = optarg;
	    break;	    
	case 'u':
	    use_port = htons(atoi(optarg));
	    break;
	case 'i':
	    use_inetd++;
	    break;
	case 'c':
	    base_cmd = optarg;
	    break;
	case 'C':
	    base_arg = optarg;
	    break;
	default:
	    usage();
	    break;
	}
    }
    if (optind != argc-1)
	usage();

    kset_logfile(logfile);

    klog(L_KRB_PERR, "\n\n***** kpropd started *****");

    strcpy(local_file, argv[optind]);
    strcat(strcpy(local_temp, argv[optind]), ".tmp");

    if (use_inetd) {
	int status;
	if (!rflag) {
	    kerror = krb_get_lrealm(my_realm,1);
	    if (kerror != KSUCCESS) {
		sprintf (errmsg, "kpropd: Can't get local realm. %s",
			 krb_get_err_text(kerror));
		klog (L_KRB_PERR, errmsg);
		exit(1);
	    }
	}
	from_len = sizeof from;
	if (getpeername(0, (struct sockaddr *) &from, &from_len) < 0) {
	    sprintf(errmsg, "kpropd: getpeername: %s", sys_errlist[errno]);
	    klog(L_KRB_PERR, errmsg);
	    exit(1);
	}

	status = accept_one_kprop(0, &from, from_len);
	exit(status);
    }

    if (use_port)
      sin.sin_port = use_port;
    else if (sp = getservbyname("krb_prop", "tcp"))
      sin.sin_port = sp->s_port;
    else
      sin.sin_port = htons(KRB_PROP_PORT); /* krb_prop/tcp */

    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	sprintf(errmsg, "kpropd: socket: %s", sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	SlowDeath();
    }
    if (bind(s, (struct sockaddr *) &sin, sizeof sin) < 0) {
	sprintf(errmsg, "kpropd: bind: %s", sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	SlowDeath();
    }
 
    if (!rflag) {
	kerror = krb_get_lrealm(my_realm,1);
	if (kerror != KSUCCESS) {
	    sprintf (errmsg, "kpropd: Can't get local realm. %s",
		     krb_get_err_text(kerror));
	    klog (L_KRB_PERR, errmsg);
	    SlowDeath();
	}
    }
    
    klog(L_KRB_PERR, "Established socket");

    listen(s, 5);
    for (;;) {
	from_len = sizeof from;
	if ((s2 = accept(s, (struct sockaddr *) &from, &from_len)) < 0) {
	    sprintf(errmsg, "kpropd: accept: %s", sys_errlist[errno]);
	    klog(L_KRB_PERR, errmsg);
	    continue;
	}
	switch(accept_one_kprop(s2, &from, from_len)) {
	case -1:
	    continue;
	case -2:
	    SlowDeath();
	}
    }
}

int accept_one_kprop(s2, from_sock, from_size)
    int s2;			/* fd of incoming connection */
    struct sockaddr_in *from_sock;
    int from_size;
{
    char cmd[1024];
    struct sockaddr_in sin;
    char    from_str[128];
    char    hostname[256];
    struct hostent *hp;
    int n, fdlock, fd;
    int     auth_status;
    short net_transfer_mode, transfer_mode;
    long    kerror;
    AUTH_DAT auth_dat;
    KTEXT_ST ticket;
    char version[9];
    char my_instance[INST_SZ];
    char *my_p_instance;
    Key_schedule session_sched;

    if (gethostname(my_instance, sizeof(my_instance)) != 0) {
	sprintf(errmsg, "kpropd: gethostname: %s", sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	return -2;
    }
    my_p_instance = krb_get_phost (my_instance);
    strcpy(my_instance, my_p_instance);
    
    strcpy(from_str, inet_ntoa(from_sock->sin_addr));
#ifdef hpux
    if (from_size == sizeof *from_sock) {
	/* hppa hpux appears to return the size of the sockaddr_in,
	   instead of the size of the actual address. */
	from_size = sizeof from_sock->sin_addr.s_addr;
    }
#endif
    if ((hp = gethostbyaddr((char*)&(from_sock->sin_addr.s_addr), from_size, AF_INET)) == NULL) {
	extern int h_errno;
	strcpy(hostname, "UNKNOWN");
	sprintf(errmsg, 
		"Failed to reverse-resolve connection %s (reason %d)",
		from_str, h_errno);
	klog(L_KRB_PERR, errmsg);
    } else {
	strcpy(hostname, hp->h_name);
    }
    
    sprintf(errmsg, "Connection from %s, %s", hostname, from_str);
    klog(L_KRB_PERR, errmsg);
    
    /* for krb_rd_{priv, safe} */
    n = sizeof sin;
    if (getsockname (s2, (struct sockaddr *) &sin, &n) != 0) {
	fprintf (stderr, "kpropd: can't get socketname.\n");
	perror ("getsockname");
	close(s2); return -1; /* repeat loop */
    }
    if (n != sizeof (sin)) {
	fprintf (stderr, "kpropd: can't get socketname. len");
	close(s2); return -1; /* repeat loop */
    }
    
    if ((fdlock = open(local_temp, O_WRONLY | O_CREAT, 0600)) < 0) {
	sprintf(errmsg, "kpropd: open: %s", sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	close(s2); return -1; /* repeat loop */
    }
    if (flock(fdlock, LOCK_EX | LOCK_NB)) {
	sprintf(errmsg, "kpropd: flock: %s", sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	close(s2); close(fdlock); return -1; /* repeat loop */
    }
    if ((fd = creat(local_temp, 0600)) < 0) {
	sprintf(errmsg, "kpropd: creat: %s", sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	close(s2); close(fdlock); return -1; /* repeat loop */
    }
    if ((n = read (s2, buf, sizeof (kprop_version)))
	!= sizeof (kprop_version)) {
	klog (L_KRB_PERR, "kpropd: can't read kprop protocol version str.");
	close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
    }
    if (strncmp (buf, kprop_version, sizeof (kprop_version))
	!= 0) {
	sprintf (errmsg, "kpropd: unsupported version %s", buf);
	klog (L_KRB_PERR, errmsg);
	close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
    }
    
    if ((n = read (s2, &net_transfer_mode, sizeof (net_transfer_mode)))
	!= sizeof (net_transfer_mode)) {
	klog (L_KRB_PERR, "kpropd: can't read transfer mode.");
	close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
    }
    transfer_mode = ntohs (net_transfer_mode);
    kerror = krb_recvauth(KOPT_DO_MUTUAL, s2, &ticket,
			  KPROP_SERVICE_NAME,
			  my_instance,
			  from_sock,
			  &sin,
			  &auth_dat,
			  srvtab,
			  session_sched,
			  version);
    if (kerror != KSUCCESS) {
	sprintf (errmsg, "kpropd: %s: Calling getkdata", 
		 krb_get_err_text(kerror));
	klog (L_KRB_PERR, errmsg);
	close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
    }
    
    sprintf (errmsg, "kpropd: Connection from %s.%s@%s",
	     auth_dat.pname, auth_dat.pinst, auth_dat.prealm);
    klog (L_KRB_PERR, errmsg);
    
    /* AUTHORIZATION is done here.  We might want to expand this to
     * read an acl file at some point, but allowing for now
     * KPROP_SERVICE_NAME.KRB_MASTER@local-realm is fine ... */
    /* actually, check rcmd and the realm here. Use admin host below. */
    
    if ((strcmp (KPROP_SERVICE_NAME, auth_dat.pname) != 0) ||
	(strcmp (my_realm, auth_dat.prealm) != 0)) {
	klog (L_KRB_PERR, "Authorization denied!");
	close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
    }
    /* Overload the "admin" host listing in krb.conf. */
    {
	char authorized[INST_SZ];
	int i = 0;
	while(KFAILURE != krb_get_admhst(authorized,my_realm,i)) {
	    char *scol;
	    scol = strchr(authorized,':');
	    if (scol) *scol = 0;
	    
	    if(strcmp(krb_get_phost(authorized), auth_dat.pinst) == 0) {
		i = (-1); break;
	    }
	    i++;
	}
	if (i != -1) {
	    klog (L_KRB_PERR, "kprop from non-admin host, rejected");
	    close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
	}
    }

    switch (transfer_mode) {
    case KPROP_TRANSFER_PRIVATE: 
	auth_status = 
	    recv_auth (s2, fd, 1 /* private */, from_sock, &sin, &auth_dat);
	break;
    case KPROP_TRANSFER_SAFE: 
	auth_status = 
	    recv_auth (s2, fd, 0 /* safe */, from_sock, &sin, &auth_dat);
	break;
    case KPROP_TRANSFER_CLEAR: 
	auth_status = 
	    recv_clear (s2, fd);
	break;
    default: 
	sprintf (errmsg, "kpropd: bad transfer mode %d", transfer_mode);
	klog (L_KRB_PERR, errmsg);
	close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
    }
    
    if (auth_status) {
	if (auth_status == KPROPD_RECV_REMOTE_FAILURE) {
	    klog(L_KRB_PERR, "remote failure. resetting.");
	    close(s2); close(fdlock); close(fd); return -1;
	}
	if (auth_status == KPROPD_RECV_LOCAL_FAILURE) {
	    klog(L_KRB_PERR, "local (key_sched) failure. resetting.");
	    close(s2); close(fdlock); close(fd); return -1;
	}
    }
    
    if (transfer_mode != KPROP_TRANSFER_PRIVATE) {
	klog(L_KRB_PERR, "kpropd: non-private transfers not supported\n");
	close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
#ifdef doesnt_work_yet
	lseek(fd, (long) 0, L_SET);
	if (auth_dat.checksum != get_data_checksum (fd, session_sched)) {
	    klog(L_KRB_PERR, "kpropd: checksum doesn't match");
	    close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
	}
#endif
    } else
	
    {
	struct stat st;
	fstat(fd, &st);
	if (st.st_size != auth_dat.checksum) {
	    sprintf(errmsg, "kpropd: length 0x%x doesn't match cks 0x%x",
		    st.st_size, auth_dat.checksum);
	    klog(L_KRB_PERR, errmsg);
	    close(s2); close(fdlock); close(fd); return -1; /* repeat loop */
	}
    }
    close(fd);
    close(s2);
    klog(L_KRB_PERR, "File received.");
    
    if (rename(local_temp, local_file) < 0) {
	sprintf(errmsg, "kpropd: rename: %s", sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	close(fdlock); return -1; /* repeat loop */
    }
    klog(L_KRB_PERR, "Temp file renamed to %s", local_file);
    
    if (flock(fdlock, LOCK_UN)) {
	sprintf(errmsg, "kpropd: flock (unlock): %s",
		sys_errlist[errno]);
	klog(L_KRB_PERR, errmsg);
	close(fdlock); return -1; /* repeat loop */
    }
    close(fdlock);
    sprintf(cmd, "%s %s %s %s", base_cmd, base_arg, local_file, local_db);
    if (system (cmd) != 0) {
	sprintf(errmsg, "kpropd: couldn't load database (%s)", cmd);
	klog (L_KRB_PERR, errmsg);
	return -1; /* repeat loop */
    }
    return 0;
}



int
recv_auth (in, out, private, remote, local, ad)
     int in, out;
     int private;
     struct sockaddr_in *remote, *local;
     AUTH_DAT *ad;
{
      u_long length;
      long kerror;
      int n;
      MSG_DAT msg_data;
      Key_schedule session_sched;

      if (private)
#ifdef NOENCRYPTION
	memset((char *)session_sched, 0, sizeof(session_sched));
#else
	if (key_sched (ad->session, session_sched)) {
	  klog (L_KRB_PERR, "kpropd: can't make key schedule");
	  return KPROPD_RECV_LOCAL_FAILURE;
	}
#endif

      while (1) {
	n = krb_net_read (in, &length, sizeof length);
	if (n == 0) break;
	if (n < 0) {
	  sprintf (errmsg, "kpropd: read: %s", sys_errlist[errno]);
	  klog (L_KRB_PERR, errmsg);
	  return KPROPD_RECV_REMOTE_FAILURE;
	}
	length = ntohl (length);
	if (length > sizeof buf) {
	  sprintf (errmsg, "kpropd: read length %d, bigger than buf %d",
		   length, sizeof buf);
	  klog (L_KRB_PERR, errmsg);
	  return KPROPD_RECV_REMOTE_FAILURE;
	}
	n = krb_net_read(in, buf, length);
	if (n < 0) {
	  sprintf(errmsg, "kpropd: read: %s", sys_errlist[errno]);
	  klog(L_KRB_PERR, errmsg);
	  return KPROPD_RECV_REMOTE_FAILURE;
	  }
	if (private)
	  kerror = krb_rd_priv (buf, n, session_sched, &ad->session, 
				remote, local, &msg_data);
	else
	  kerror = krb_rd_safe (buf, n, &ad->session,
				remote, local, &msg_data);
	if (kerror != KSUCCESS) {
	  sprintf (errmsg, "kpropd: %s: %s",
		   private ? "krb_rd_priv" : "krb_rd_safe",
		   krb_get_err_text(kerror));
	  klog (L_KRB_PERR, errmsg);
	  return KPROPD_RECV_REMOTE_FAILURE;
	}
	if (write(out, msg_data.app_data, msg_data.app_length) != 
	    msg_data.app_length) {
	  sprintf(errmsg, "kpropd: write: %s", sys_errlist[errno]);
	  klog(L_KRB_PERR, errmsg);
	  return KPROPD_RECV_REMOTE_FAILURE;
	}
      }
      return KPROPD_SUCCESS;
    }

int
recv_clear (in, out)
    int in, out;
    {
      int n;

      while (1) {
	n = read (in, buf, sizeof buf);
	if (n == 0) break;
	if (n < 0) {
	    sprintf (errmsg, "kpropd: read: %s", sys_errlist[errno]);
	    klog (L_KRB_PERR, errmsg);
	    return KPROPD_RECV_REMOTE_FAILURE;
	  }
	if (write(out, buf, n) != n) {
	      sprintf(errmsg, "kpropd: write: %s", sys_errlist[errno]);
	      klog(L_KRB_PERR, errmsg);
	      return KPROPD_RECV_REMOTE_FAILURE;
	    }
      }
      return KPROPD_SUCCESS;
    }

static void 
SlowDeath()
{
    if (!use_inetd) {
      char msg[256];
      int i = getpid();

      sprintf(msg, "kpropd[%d] pausing before exit so as not to loop init", i);
      klog(L_KRB_PERR, msg);
      sleep(pause_int);
      sprintf(msg, "kpropd[%d] terminated", i);
      klog(L_KRB_PERR, msg);
    }
    exit(1);
}
#ifdef doesnt_work_yet
unsigned long get_data_checksum(fd, key_sched)
int fd;
Key_schedule key_sched;
{
	unsigned long cksum = 0;
	unsigned long cbc_cksum();
	int n;
	char buf[BUFSIZ];
	char obuf[8];

	while (n = read(fd, buf, sizeof buf)) {
	    if (n < 0) {
		sprintf(errmsg, "kpropd: read (in checksum test): %s",
						sys_errlist[errno]);
		klog(L_KRB_PERR, errmsg);
		SlowDeath();
	    }
#ifndef NOENCRYPTION
	    cksum += cbc_cksum(buf, obuf, n, key_sched, key_sched);
#endif
	  }
	return cksum;
}
#endif
