/* 
 * Copyright 1987 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information,
 * please see the file <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <netdb.h>
#ifdef POSIX
#include <stdlib.h>
#endif
#ifdef NEED_SYS_FCNTL_H
/* for sco, to get O_RDONLY */
#include <sys/fcntl.h>
#endif
#include <krb.h>
#include <krbports.h>
#include <des.h>
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

/* for those broken Unixes without this defined... should be in sys/param.h */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

static char kprop_version[KPROP_PROT_VERSION_LEN] = KPROP_PROT_VERSION;

int     debug = 0;

char    my_realm[REALM_SZ];
int     princ_data_size = 3 * sizeof(long) + 3 * sizeof(unsigned char);
short   transfer_mode, net_transfer_mode;
int force_flag;
static char ok[] = ".dump_ok";
static int use_preauth = 0;
static char *kprop_srvtab = KPROP_SRVTAB;

struct slave_host {
    u_long  net_addr;
    u_short port;
    char   *name;
    char   *instance;
    char   *realm;
    int	   not_time_yet;
    int    succeeded;
    struct slave_host *next;
};

main(argc, argv)
    int     argc;
    char   *argv[];
{
    int     fd, i;
    char   *floc, *floc_ok;
    char   *fslv;
    struct stat stbuf, stbuf_ok;
    long   time(), l_init, l_final;
    char   *ctime(), *pc;
    int    l_diff, prop_to_slaves(), get_slaves();
    static struct slave_host *slave_host_list = NULL;
    struct slave_host *sh;

    transfer_mode = KPROP_TRANSFER_PRIVATE;

    time(&l_init);
    pc = ctime(&l_init);
    pc[strlen(pc) - 1] = '\0';
    printf("\nStart slave propagation: %s\n", pc);
 
    floc = (char *) NULL;
    fslv = (char *) NULL;

    if (krb_get_lrealm(my_realm,1) != KSUCCESS)
      Death ("Getting my kerberos realm.  Check krb.conf");

    for (i = 1; i < argc; i++) 
      switch (argv[i][0]) {
      case '-':
	if (strcmp (argv[i], "-private") == 0) 
	  transfer_mode = KPROP_TRANSFER_PRIVATE;
#ifdef not_safe_yet
	else if (strcmp (argv[i], "-safe") == 0) 
	  transfer_mode = KPROP_TRANSFER_SAFE;
	else if (strcmp (argv[i], "-clear") == 0) 
	  transfer_mode = KPROP_TRANSFER_CLEAR;
#endif
	else if (strcmp (argv[i], "-realm") == 0) {
	    i++;
	    if (i < argc)
		strcpy(my_realm, argv[i]);
	    else
		goto usage;
	} else if (strcmp (argv[i], "-force") == 0)
	    force_flag++;
	else if (strcmp (argv[i], "-p") == 0)
	    use_preauth++;
	else if (strcmp (argv[i], "-s") == 0) {
	    i++;
	    if (i < argc)
		kprop_srvtab = argv[i];
	    else
		goto usage;
	} else {
	  fprintf (stderr, "kprop: unknown control argument %s.\n",
		   argv[i]);
	  exit (1);
	}
	break;
      default:
	/* positional arguments are marginal at best ... */
	if (floc == (char *) NULL)
	  floc = argv[i];
	else {
	  if (fslv == (char *) NULL)
	    fslv = argv[i];
	  else {
	  usage:
		  /* already got floc and fslv, what is this? */
	    fprintf(stderr,
		    "\nUsage: kprop [-p] [-force] [-realm realm] [-private|-safe|-clear] data_file slaves_file\n\n");
	    exit(1);
	  }
	}
      }
    if ((floc == (char *)NULL) || (fslv == (char *)NULL))
	    goto usage;
    
    if ((floc_ok = (char *) malloc(strlen(floc) + strlen(ok) + 1))
	== NULL) {
	Death(floc);
    }
    strcat(strcpy(floc_ok, floc), ok);

    if ((fd = open(floc, O_RDWR)) < 0) {
	Death(floc);
    }
    if (flock(fd, LOCK_EX | LOCK_NB)) {
	Death(floc);
    }
    if (stat(floc, &stbuf)) {
	Death(floc);
    }
    if (stat(floc_ok, &stbuf_ok)) {
	Death(floc_ok);
    }
    if (stbuf.st_mtime > stbuf_ok.st_mtime) {
	fprintf(stderr, "kprop: '%s' more recent than '%s'.\n",
		floc, floc_ok);
	exit(1);
    }
    if (!get_slaves(&slave_host_list, fslv, stbuf_ok.st_mtime)) {
	fprintf(stderr,
		"kprop: can't read slave host file '%s'.\n", fslv);
	exit(1);
    }
#ifdef KPROP_DBG
    {
	struct slave_host *sh;
	int     i;
	fprintf(stderr, "\n\n");
	fflush(stderr);
	for (sh = slave_host_list; sh; sh = sh->next) {
	    fprintf(stderr, "slave %d: %s, %s", i++, sh->name,
		    inet_ntoa(sh->net_addr));
	    fflush(stderr);
	}
    }
#endif				/* KPROP_DBG */

    if (!prop_to_slaves(slave_host_list, fd, fslv)) {
	fprintf(stderr,
		"kprop: propagation failed.\n");
	exit(1);
    }
    if (flock(fd, LOCK_UN)) {
	Death(floc);
    }
    fprintf(stderr, "\n\n");
    for (sh = slave_host_list; sh; sh = sh->next) {
	fprintf(stderr, "%s:\t\t%s\n", sh->name,
		(sh->not_time_yet? "Not time yet" : (sh->succeeded ? "Succeeded" : "FAILED")));
    }

    time(&l_final);
    l_diff = l_final - l_init;
    printf("propagation finished, %d:%02d:%02d elapsed\n",
	   l_diff / 3600, (l_diff % 3600) / 60, l_diff % 60);

    exit(0);
}

Death(s)
    char   *s;
{
    fprintf(stderr, "kprop: ");
    perror(s);
    exit(1);
}

/* The master -> slave protocol looks like this:
     1) 8 byte version string
     2) 2 bytes of "transfer mode" (net byte order of course)
     3) ticket/authentication send by sendauth
     4) 4 bytes of "block" length (u_long)
     5) data

     4 and 5 repeat til EOF ...
*/

int prop_to_slaves(sl, fd, fslv)
    struct slave_host *sl;
    int     fd;
    char   *fslv;
{
    unsigned char buf[KPROP_BUFSIZ];
    /* leave room in obuf for private msg overhead */
    unsigned char obuf[KPROP_BUFSIZ + 64];
    struct servent *sp;
    struct sockaddr_in sin, my_sin;
    int     i, n, s;
    struct slave_host *cs;	/* current slave */
    char   path[256], my_host_name[MAXHOSTNAMELEN], *p_my_host_name;
    char   kprop_service_instance[INST_SZ];
    char   *pc;
    u_long cksum, get_data_checksum();
    u_long length;
    KRB_INT32 nlength;
    long   kerror;
    KTEXT_ST     ticket;
    CREDENTIALS  cred;
    MSG_DAT msg_dat;
    static char tkstring[] = "/tmp/kproptktXXXXXX";
    
    Key_schedule session_sched;

    (void) mktemp(tkstring);
    krb_set_tkt_string(tkstring);
    
    memset(&sin, 0, sizeof sin);

    if (sp = getservbyname("krb_prop", "tcp"))
      sin.sin_port = sp->s_port;
    else
      sin.sin_port = htons(KRB_PROP_PORT); /* krb_prop/tcp */

    sin.sin_family = AF_INET;

    strcpy(path, fslv);
    if (pc = strrchr(path, '/')) {
	pc += 1;
    } else {
	pc = path;
    }

    for (i = 0; i < 5; i++) {	/* try each slave five times max */
	for (cs = sl; cs; cs = cs->next) {
	    if (!cs->succeeded) {
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		    perror("kprop: socket");
		    exit(1);
		}
		memcpy(&sin.sin_addr, &cs->net_addr, sizeof cs->net_addr);
		if (cs->port) sin.sin_port = cs->port;

		if (connect(s, (struct sockaddr *) &sin, sizeof sin) < 0) {
		    fprintf(stderr, "%s: ", cs->name);
		    perror("connect");
		    close(s);
		    continue;	/*** NEXT SLAVE ***/
		}
		
		/* for krb_mk_{priv, safe} */
		memset (&my_sin, 0, sizeof my_sin);
		n = sizeof my_sin;
		if (getsockname (s, (struct sockaddr *) &my_sin, &n) != 0) {
		    fprintf (stderr, "kprop: can't get socketname.");
		    perror ("getsockname");
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}
		if (n != sizeof (my_sin)) {
		    fprintf (stderr, "kprop: can't get socketname. len");
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}
		
		/* Get ticket */
		kerror = krb_mk_req (&ticket, KPROP_SERVICE_NAME, 
				     cs->instance, cs->realm, (u_long) 0);
		/* if ticket has expired try to get a new one, but
		 * first get a TGT ...
		 */
		if (kerror != MK_AP_OK) {
		    if (gethostname (my_host_name, sizeof(my_host_name)) != 0) {
			fprintf (stderr, "%s:", cs->name);
			perror ("getting my hostname");
			close (s);
			break;	/* next one can't work either! */
		    }
		    /* get canonical kerberos service instance name */
		    p_my_host_name = krb_get_phost (my_host_name);
		    /* copy it to make sure gethostbyname static doesn't
		     * screw us. */
		    strcpy (kprop_service_instance, p_my_host_name);
		    if (use_preauth)
			kerror = krb_get_svc_in_tkt_preauth (KPROP_SERVICE_NAME, 
#if 1
						 kprop_service_instance,
#else
						 KRB_MASTER,
#endif
						 my_realm,
						 TGT_SERVICE_NAME,
						 my_realm,
						 96,
						 kprop_srvtab);
		    else
			kerror = krb_get_svc_in_tkt (KPROP_SERVICE_NAME, 
#if 1
						 kprop_service_instance,
#else
						 KRB_MASTER,
#endif
						 my_realm,
						 TGT_SERVICE_NAME,
						 my_realm,
						 96,
						 kprop_srvtab);
		    if (kerror != INTK_OK) {
			fprintf (stderr,
				 "%s: %s.  While getting initial ticket\n",
				 cs->name, krb_get_err_text(kerror));
			close (s);
			goto punt;
		    }
		    kerror = krb_mk_req (&ticket, KPROP_SERVICE_NAME, 
					 cs->instance, cs->realm, (u_long) 0);
		}
		if (kerror != MK_AP_OK) {
		    fprintf (stderr, "%s: %s. Calling krb_mk_req.",
			     cs->name, krb_get_err_text(kerror));
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}		    

		if (write(s, kprop_version, sizeof(kprop_version))
		    != sizeof(kprop_version)) {
		    fprintf (stderr, "%s: ", cs->name);
		    perror ("write (version) error");
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}

		net_transfer_mode = htons (transfer_mode);
		if (write(s, &net_transfer_mode, sizeof(net_transfer_mode))
		    != sizeof(net_transfer_mode)) {
		    fprintf (stderr, "%s: ", cs->name);
		    perror ("write (transfer_mode) error");
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}

		kerror = krb_get_cred (KPROP_SERVICE_NAME, cs->instance,
				       cs->realm, &cred);
		if (kerror != KSUCCESS) {
		    fprintf (stderr, "%s: %s.  Getting session key.", 
			     cs->name, krb_get_err_text(kerror));
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}
#ifdef NOENCRYPTION
		memset((char *)session_sched, 0, sizeof(session_sched));
#else
		if (key_sched (cred.session, session_sched)) {
		    fprintf (stderr, "%s: can't make key schedule.",
			     cs->name);
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}
#endif
		/* SAFE (quad_cksum) and CLEAR are just not good enough */
		cksum = 0;
#ifdef not_working_yet
		if (transfer_mode != KPROP_TRANSFER_PRIVATE) {
		    cksum = get_data_checksum(fd, session_sched);
		}
		else
#endif
           	{
		    struct stat st;
		    fstat (fd, &st);
		    cksum = st.st_size;
	        }
		lseek(fd, 0L, 0);
		kerror = krb_sendauth(KOPT_DO_MUTUAL,
				      s,
				      &ticket,
				      KPROP_SERVICE_NAME,
				      cs->instance,
				      cs->realm,
				      cksum,
				      &msg_dat,
				      &cred,
				      session_sched,
				      &my_sin,
				      &sin,
				      KPROP_PROT_VERSION);
		if (kerror != KSUCCESS) {
		    fprintf (stderr, "%s: %s.  Calling krb_sendauth.", 
			     cs->name, krb_get_err_text(kerror));
		    close (s);
		    continue;	/*** NEXT SLAVE ***/
		}

		while (n = read(fd, buf, sizeof buf)) {
		    if (n < 0) {
			perror("input file read error");
			exit(1);
		    }
		    switch (transfer_mode) {
		    case KPROP_TRANSFER_PRIVATE:
		    case KPROP_TRANSFER_SAFE:
			if (transfer_mode == KPROP_TRANSFER_PRIVATE)
			    length = krb_mk_priv (buf, obuf, n, 
						  session_sched, &cred.session,
						  &my_sin, &sin);
			else
			    length = krb_mk_safe (buf, obuf, n,
						  &cred.session,
						  &my_sin, &sin);
			if (length == -1) {
			    fprintf (stderr, "%s: %s failed.",
				     cs->name,
				     (transfer_mode == KPROP_TRANSFER_PRIVATE) 
				     ? "krb_rd_priv" : "krb_rd_safe");
			    close (s);
			    continue; /*** NEXT SLAVE ***/
			}
			nlength = htonl(length);
			if (write(s, &nlength, sizeof nlength)
			    != sizeof nlength) {
			    fprintf (stderr, "%s: ", cs->name);
			    perror ("write error");
			    close (s);
			    continue; /*** NEXT SLAVE ***/
			}
			if (write(s, obuf, length) != length) {
			    fprintf(stderr, "%s: ", cs->name);
			    perror("write error");
			    close(s);
			    continue; /*** NEXT SLAVE ***/
			}
			break;
		    case KPROP_TRANSFER_CLEAR:
			if (write(s, buf, n) != n) {
			    fprintf(stderr, "%s: ", cs->name);
			    perror("write error");
			    close(s);
			    continue; /*** NEXT SLAVE ***/
			}
			break;
		    }
		}
		close(s);
		cs->succeeded = 1;
		fprintf(stderr, "%s: success.\n", cs->name);
		strcat(strcpy(pc, cs->name), "-last-prop");
		close(creat(path, 0600));
	    }
	}
    }
punt:
    
    dest_tkt();
    for (cs = sl; cs; cs = cs->next) {
	if (!cs->succeeded)
	    return (0);		/* didn't get this slave */
    }
    return (1);
}

int get_slaves(psl, file, ok_mtime)
    struct slave_host **psl;
    char   *file;
    time_t  ok_mtime;
{
    FILE   *fin;
    char    namebuf[128], *inst;
    char   *pc;
    struct hostent *host;
    struct slave_host **th;
    char    path[256];
    char   *ppath;
    struct stat stbuf;
    int    tmp_port;
    char   *scol;

    if ((fin = fopen(file, "r")) == NULL) {
	fprintf(stderr, "Can't open slave host file, '%s'.\n", file);
	exit(-1);
    }
    strcpy(path, file);
    if (ppath = strrchr(path, '/')) {
	ppath += 1;
    } else {
	ppath = path;
    }
    for (th = psl; fgets(namebuf, sizeof namebuf, fin); th = &(*th)->next) {
	if (pc = strchr(namebuf, '\n')) {
	    *pc = '\0';
	} else {
	    fprintf(stderr, "Host name too long (>= %d chars) in '%s'.\n",
		    sizeof namebuf, file);
	    exit(-1);
	}
	if (0 != (scol = strchr(namebuf,':'))) {
	    tmp_port = htons(atoi(scol+1));
	    *scol = 0;
	} else
	    tmp_port = 0;
	host = gethostbyname(namebuf);
	if (host == NULL) {
	    fprintf(stderr, "Unknown host '%s' in '%s'.\n", namebuf, file);
	    exit(-1);
	}
	(*th) = (struct slave_host *) malloc(sizeof(struct slave_host));
	if (!*th) {
	    fprintf(stderr, "No memory reading host list from '%s'.\n",
		    file);
	    exit(-1);
	}
	(*th)->name = (char *) malloc(strlen(namebuf) + 1);
	if (!(*th)->name) {
	    fprintf(stderr, "No memory reading host list from '%s'.\n",
		    file);
	    exit(-1);
	}
	/* get kerberos cannonical instance name */
	strcpy((*th)->name, namebuf);
	inst = krb_get_phost ((*th)->name);
	(*th)->instance = (char *) malloc(strlen(inst) + 1);
	if (!(*th)->instance) {
	    fprintf(stderr, "No memory reading host list from '%s'.\n",
		    file);
	    exit(-1);
	}
	strcpy((*th)->instance, inst);
	/* what a concept, slave servers in different realms! */
	(*th)->realm = my_realm;
	(*th)->net_addr = *(u_long *) host->h_addr;
	(*th)->port = tmp_port;
	(*th)->not_time_yet = 0;
	(*th)->succeeded = 0;
	(*th)->next = NULL;
	strcat(strcpy(ppath, (*th)->name), "-last-prop");
	if (!force_flag && !stat(path, &stbuf) && stbuf.st_mtime > ok_mtime) {
	    (*th)->not_time_yet = 1;
	    (*th)->succeeded = 1;	/* no change since last success */
	}
    }
    fclose(fin);
    return (1);
}

#ifdef doesnt_work_yet
u_long get_data_checksum(fd, key_sched)
     int fd;
     Key_schedule key_sched;
{
	unsigned long cksum = 0;
	unsigned long cbc_cksum();
	int n;
	char buf[BUFSIZ];
	long obuf[2];

	while (n = read(fd, buf, sizeof buf)) {
	    if (n < 0) {
		fprintf(stderr, "Input data file read error: ");
		perror("read");
		exit(1);
	    }
	    cksum = cbc_cksum(buf, obuf, n, key_sched, key_sched);
	}
	return cksum;
}
#endif
