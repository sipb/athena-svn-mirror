/*
 * Copyright 1989 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * list and update contents of srvtab files
 */

#include <mit-copyright.h>
/*
 * ksrvutil
 * list and update the contents of srvtab files
 */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <kadm.h>
#include <krb_err.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef USE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>

#ifdef NOENCRYPTION
#define read_long_pw_string placebo_read_pw_string
#else /* NOENCRYPTION */
#define read_long_pw_string des_read_pw_string
#endif /* NOENCRYPTION */
int read_long_pw_string();

#define SRVTAB_MODE 0600	/* rw------- */
#define PAD "  "
#define VNO_HEADER "Version"
#define VNO_FORMAT "%4d   "
#define KEY_HEADER "       Key       " /* 17 characters long */
#define PRINC_HEADER "  Principal\n"
#define PRINC_FORMAT "%s"

extern int errno;
#ifndef HAVE_SYS_ERRLIST_DECL
extern char *sys_errlist[];
#endif
static int use_preauth = 0;

extern void krb_set_tkt_string();
void leave();
unsigned short get_mode();

void
copy_keyfile(progname, keyfile, backup_keyfile)
  char *progname;
  char *keyfile;
  char *backup_keyfile;
{
    int keyfile_fd;
    int backup_keyfile_fd;
    int keyfile_mode;
    char buf[BUFSIZ];		/* for copying keyfiles */
    int rcount;			/* for copying keyfiles */
    int try_again;
    
    (void) memset((char *)buf, 0, sizeof(buf));
    
    do {
	try_again = FALSE;
	if ((keyfile_fd = open(keyfile, O_RDONLY, 0)) < 0) {
	    if (errno != ENOENT) {
		(void)fprintf(stderr, "%s: Unable to read %s: %s\n", progname, 
			      keyfile, sys_errlist[errno]);
		exit(1);
	    }
	    else {
		try_again = TRUE;
		if ((keyfile_fd = 
		     open(keyfile, 
			  O_WRONLY | O_TRUNC | O_CREAT, SRVTAB_MODE)) < 0) {
		    (void) fprintf(stderr, "%s: Unable to create %s: %s\n", 
				   progname, keyfile, sys_errlist[errno]);
		    exit(1);
		}
		else
		    if (close(keyfile_fd) < 0) {
			(void) fprintf(stderr, "%s: Failure closing %s: %s\n",
				       progname, keyfile, sys_errlist[errno]);
			exit(1);
		    }
	    }
	}
    } while(try_again);

    keyfile_mode = get_mode(keyfile);

    if ((backup_keyfile_fd = 
	 open(backup_keyfile, O_WRONLY | O_TRUNC | O_CREAT, 
	      keyfile_mode)) < 0) {
	(void) fprintf(stderr, "%s: Unable to write %s: %s\n", progname, 
		       backup_keyfile, sys_errlist[errno]);
	exit(1);
    }
    do {
	if ((rcount = read(keyfile_fd, (char *)buf, sizeof(buf))) < 0) {
	    (void) fprintf(stderr, "%s: Error reading %s: %s\n", progname,
			   keyfile, sys_errlist[errno]);
	    exit(1);
	}
	if (rcount && (write(backup_keyfile_fd, buf, rcount) != rcount)) {
	    (void) fprintf(stderr, "%s: Error writing %s: %s\n", progname,
			   backup_keyfile, sys_errlist[errno]);
	    exit(1);
	}
    } while (rcount);
    if (close(backup_keyfile_fd) < 0) {
	(void) fprintf(stderr, "%s: Error closing %s: %s\n", progname,
		       backup_keyfile, sys_errlist[errno]);
	exit(1);
    }
    if (close(keyfile_fd) < 0) {
	(void) fprintf(stderr, "%s: Error closing %s: %s\n", progname,
		       keyfile, sys_errlist[errno]);
	exit(1);
    }
}

void
safe_read_stdin(prompt, buf, size)
  char *prompt;
  char *buf;
  int size;
{
    (void) printf(prompt);
    (void) fflush(stdout);
    (void) memset(buf, 0, size);
    if (read(0, buf, size - 1) < 0) {
	(void) fprintf(stderr, "Failure reading from stdin: %s\n", 
		       sys_errlist[errno]);
	leave((char *)NULL, 1);
    }
    fflush(stdin);
    buf[strlen(buf)-1] = 0;
}	
  

void
safe_write(progname, filename, fd, buf, len)
  char *progname;
  char *filename;
  int fd;
  char *buf;
  int len;
{
    if (write(fd, buf, len) != len) {
	(void) fprintf(stderr, "%s: Failure writing to %s: %s\n", progname,
		       filename, sys_errlist[errno]);
	(void) close(fd);
	leave("In progress srvtab in this file.", 1);
    }
}	

int
yn(string)
  char *string;
{
    char ynbuf[5];

    (void) printf("%s (y,n) [y] ", string);
    for (;;) {
	safe_read_stdin("", ynbuf, sizeof(ynbuf));
	
	if ((ynbuf[0] == 'n') || (ynbuf[0] == 'N'))
	    return(0);
	else if ((ynbuf[0] == 'y') || (ynbuf[0] == 'Y') || (ynbuf[0] == 0))
	    return(1);
	else {
	    (void) printf("Please enter 'y' or 'n': ");
	    fflush(stdout);
	}
    }
}

void
append_srvtab(progname, filename, fd, sname, sinst, 
		   srealm, key_vno, key)
  char *progname;
  char *filename;
  int fd;
  char *sname;
  char *sinst;
  char *srealm;
  unsigned char key_vno;
  des_cblock key;
{
    /* Add one to append null */
    safe_write(progname, filename, fd, sname, strlen(sname) + 1);
    safe_write(progname, filename, fd, sinst, strlen(sinst) + 1);
    safe_write(progname, filename, fd, srealm, strlen(srealm) + 1);
    safe_write(progname, filename, fd, (char *)&key_vno, 1);
    safe_write(progname, filename, fd, (char *)key, sizeof(des_cblock));
#ifndef NO_FSYNC
    (void) fsync(fd);
#endif
}    

unsigned short
get_mode(filename)
  char *filename;
{
    struct stat statbuf;
    unsigned short mode;

    (void) memset((char *)&statbuf, 0, sizeof(statbuf));
    
    if (stat(filename, &statbuf) < 0) 
	mode = SRVTAB_MODE;
    else
	mode = statbuf.st_mode;

    return(mode);
}

main(argc,argv)
  int argc;
  char *argv[];
{
    char sname[ANAME_SZ];	/* name of service */
    char sinst[INST_SZ];	/* instance of service */
    char srealm[REALM_SZ];	/* realm of service */
    unsigned char key_vno;	/* key version number */
    int real_kvno;		/* ... and one from the database */
    int status;			/* general purpose error status */
    des_cblock new_key;
    des_cblock old_key;
    char change_tkt[MAXPATHLEN]; /* Ticket to use for key change */
    char keyfile[MAXPATHLEN];	/* Original keyfile */
    char work_keyfile[MAXPATHLEN]; /* Working copy of keyfile */
    char backup_keyfile[MAXPATHLEN]; /* Backup copy of keyfile */
    unsigned short keyfile_mode; /* Protections on keyfile */
    int work_keyfile_fd = -1;	/* Initialize so that */
    int backup_keyfile_fd = -1;	/* compiler doesn't complain */
    char local_realm[REALM_SZ];	/* local kerberos realm */
    int i;
    int interactive = FALSE;
    int list = FALSE;
    int change = FALSE;
    int add = FALSE;
    int key = FALSE;		/* do we show keys? */
    int flag_delete = FALSE;
    int arg_entered = FALSE;
    int change_this_key = FALSE;
    char databuf[BUFSIZ];
    int first_printed = FALSE;	/* have we printed the first item? */
    
    int get_svc_new_key();
    void get_key_from_password();
    void print_key();
    void print_name();
    
    initialize_krb_error_table();
    initialize_kadm_error_table();

    (void) memset((char *)sname, 0, sizeof(sname));
    (void) memset((char *)sinst, 0, sizeof(sinst));
    (void) memset((char *)srealm, 0, sizeof(srealm));
    
    (void) memset((char *)change_tkt, 0, sizeof(change_tkt));
    (void) memset((char *)keyfile, 0, sizeof(keyfile));
    (void) memset((char *)work_keyfile, 0, sizeof(work_keyfile));
    (void) memset((char *)backup_keyfile, 0, sizeof(backup_keyfile));
    (void) memset((char *)local_realm, 0, sizeof(local_realm));
    
    (void) sprintf(change_tkt, "/tmp/tkt_ksrvutil.%d", getpid());
    krb_set_tkt_string(change_tkt);

    /* This is used only as a default for adding keys */
    if (krb_get_lrealm(local_realm, 1) != KSUCCESS)
	(void) strcpy(local_realm, KRB_REALM);
    
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-i") == 0) 
	    interactive++;
	else if (strcmp(argv[i], "-k") == 0) 
	    key++;
	else if (strcmp(argv[i], "-p") == 0)
	    use_preauth++;
	else if (strcmp(argv[i], "list") == 0) {
	    if (arg_entered)
		usage();
	    else {
		arg_entered++;
		list++;
	    }
	}
	else if (strcmp(argv[i], "change") == 0) {
	    if (arg_entered)
		usage();
	    else {
		arg_entered++;
		change++;
	    }
	}
	else if (strcmp(argv[i], "add") == 0) {
	    if (arg_entered)
		usage();
	    else {
		arg_entered++;
		add++;
	    }
	}
	else if (strcmp(argv[i], "delete") == 0) {
	    if (arg_entered)
		usage();
	    else {
		arg_entered++;
		flag_delete++;
		interactive++;	/* deletion is only interactive for now */
	    }
	}
	else if (strcmp(argv[i], "-f") == 0) {
	    if (++i == argc)
		usage();
	    else
		(void) strcpy(keyfile, argv[i]);
	}
	else
	    usage();
    }
    
    if (!arg_entered)
	usage();

    if (!keyfile[0])
	(void) strcpy(keyfile, KEYFILE);
    
    (void) strcpy(work_keyfile, keyfile);
    (void) strcpy(backup_keyfile, keyfile);
    
    if (change || flag_delete || add) {
	(void) strcat(work_keyfile, ".work");
	(void) strcat(backup_keyfile, ".old");
	
	copy_keyfile(argv[0], keyfile, backup_keyfile);
    }
    
    if (add)
	copy_keyfile(argv[0], backup_keyfile, work_keyfile);

    keyfile_mode = get_mode(keyfile);

    if (change || flag_delete || list) {
	if ((backup_keyfile_fd = open(backup_keyfile, O_RDONLY, 0)) < 0) {
	    (void) fprintf(stderr, "%s: Unable to read %s: %s\n", argv[0],
			   backup_keyfile, sys_errlist[errno]);
	    exit(1);
	}
    }

    if (change || flag_delete) {
	if ((work_keyfile_fd = 
	     open(work_keyfile, O_WRONLY | O_CREAT | O_TRUNC, 
		  SRVTAB_MODE)) < 0) {
	    (void) fprintf(stderr, "%s: Unable to write %s: %s\n", argv[0],
			   work_keyfile, sys_errlist[errno]);
	    exit(1);
	}
    }
    else if (add) {
	if ((work_keyfile_fd =
	     open(work_keyfile, O_APPEND|O_WRONLY|O_SYNC, SRVTAB_MODE)) < 0) {
	    (void) fprintf(stderr, "%s: Unable to open %s for append: %s\n",
			   argv[0], work_keyfile, sys_errlist[errno]);
	    exit(1);
	}
    }
    
    if (change || flag_delete || list) {
	while ((getst(backup_keyfile_fd, sname, SNAME_SZ) > 0) &&
	       (getst(backup_keyfile_fd, sinst, INST_SZ) > 0) &&
	       (getst(backup_keyfile_fd, srealm, REALM_SZ) > 0) &&
	       (read(backup_keyfile_fd, &key_vno, 1) > 0) &&
	       (read(backup_keyfile_fd,(char *)old_key,sizeof(old_key)) > 0)) {
	    if (list) {
		if (!first_printed) {
		    (void) printf(VNO_HEADER);
		    (void) printf(PAD);
		    if (key) {
			(void) printf(KEY_HEADER);
			(void) printf(PAD);
		    }
		    (void) printf(PRINC_HEADER);
		    first_printed = 1;
		}
		(void) printf(VNO_FORMAT, key_vno);
		(void) printf(PAD);
		if (key) {
		    print_key(old_key);
		    (void) printf(PAD);
		}
		print_name(sname, sinst, srealm);
		(void) printf("\n");
	    }
	    else if (change) {
		(void) printf("\nPrincipal: ");
		print_name(sname, sinst, srealm);
		(void) printf("; version %d\n", key_vno);

		status = get_kvno(sname, sinst, srealm, keyfile, &real_kvno);
		if (!status) {
		    if (key_vno != real_kvno) {
		        (void) printf("Server reports key version %d -- ", real_kvno);
			if (yn("Fix keyfile to match?"))
			    key_vno = real_kvno;
		    }
		       
		    if (interactive)
			change_this_key = yn("Change this key?");
		    else if (change)
			change_this_key = 1;
		    else
			change_this_key = 0;
		    
		    if (change_this_key)
			(void) printf("Changing to version %d.\n", key_vno + 1);
		    else if (change)
			(void) printf("Not changing this key.\n");
		} else {
		    com_err(argv[0], status + krb_err_base,
			    "\nCouldn't get version number -- not changing this key");
		    change_this_key = FALSE;
		}
		    
		if (change_this_key) {
		    /* 
		     * Pick a new key and determine whether or not
		     * it is safe to change
		     */
		    if ((status = 
			 get_svc_new_key(new_key, sname, sinst, 
					 srealm, keyfile)) == KADM_SUCCESS)
			key_vno++;
		    else {
			(void) memcpy(new_key, old_key, sizeof(new_key));
			com_err(argv[0], status, "Key NOT changed");
			change_this_key = FALSE;
		    }
		}
		else 
		    (void) memcpy(new_key, old_key, sizeof(new_key));
		append_srvtab(argv[0], work_keyfile, work_keyfile_fd, 
			      sname, sinst, srealm, key_vno, new_key);
		if (key && change_this_key) {
		    (void) printf("Old key: ");
		    print_key(old_key);
		    (void) printf("; new key: ");
		    print_key(new_key);
		    (void) printf("\n");
		}
		if (change_this_key) {
		    if ((status = kadm_change_pw(new_key)) == KADM_SUCCESS) {
			(void) printf("Key changed.\n");
			(void) dest_tkt();
		    }
		    else {
			com_err(argv[0], status, 
				" attempting to change password.");
			(void) dest_tkt();
			/* XXX This knows the format of a keyfile */
			if (lseek(work_keyfile_fd, -9, SEEK_CUR) >= 0) {
			    key_vno--;
			    safe_write(argv[0], work_keyfile,
				       work_keyfile_fd, (char *)&key_vno, 1);
			    safe_write(argv[0], work_keyfile, work_keyfile_fd,
				       (char *)old_key, sizeof(des_cblock));
#ifndef NO_FSYNC
			    (void) fsync(work_keyfile_fd);
#endif
			    (void) fprintf(stderr,"Key NOT changed.\n");
			}
			else {
			    (void)fprintf(stderr, 
					  "%s: Unable to revert keyfile: %s\n",
					  argv[0], sys_errlist[errno]);
			    leave("", 1);
			}
		    }
		}
	    }
	    else if (flag_delete) {
		(void) printf("\nPrincipal: ");
		print_name(sname, sinst, srealm);
		(void) printf("; version %d\n", key_vno);
		change_this_key = yn("Delete this key?");
		
		if (change_this_key)
		    (void) printf("Deleting this key.\n");
		else
		    (void) printf("Keeping this key.\n");
		
		if (!change_this_key) {
		    append_srvtab(argv[0], work_keyfile, work_keyfile_fd, 
				  sname, sinst, srealm, key_vno, old_key);
		}
		memset((char *)old_key, 0, sizeof(des_cblock));
	    }
	}
    }
    else if (add) {
	do {
	    do {
		safe_read_stdin("Name: ", databuf, sizeof(databuf));
		(void) strncpy(sname, databuf, sizeof(sname) - 1);
		safe_read_stdin("Instance: ", databuf, sizeof(databuf));
		(void) strncpy(sinst, databuf, sizeof(sinst) - 1);
		safe_read_stdin("Realm: ", databuf, sizeof(databuf));
		(void) strncpy(srealm, databuf, sizeof(srealm) - 1);
		safe_read_stdin("Version number: ", databuf, sizeof(databuf));
		key_vno = atoi(databuf);
		if (!srealm[0])
		    (void) strcpy(srealm, local_realm);
		(void) printf("New principal: ");
		print_name(sname, sinst, srealm);
		(void) printf("; version %d\n", key_vno);
	    } while (!yn("Is this correct?"));
	    get_key_from_password(new_key);
	    if (key) {
		(void) printf("Key: ");
		print_key(new_key);
		(void) printf("\n");
	    }
	    append_srvtab(argv[0], work_keyfile, work_keyfile_fd, 
			  sname, sinst, srealm, key_vno, new_key);
	    (void) printf("Key successfully added.\n");
	} while (yn("Would you like to add another key?"));
    }

    if (change || flag_delete || list) 
	if (close(backup_keyfile_fd) < 0) {
	    (void) fprintf(stderr, "%s: Failure closing %s: %s\n",
			   argv[0], backup_keyfile, sys_errlist[errno]);
	    (void) fprintf(stderr, "continuing...\n");
	}
    
    if (change || flag_delete || add) {
	if (close(work_keyfile_fd) < 0) {
	    (void) fprintf(stderr, "%s: Failure closing %s: %s\n",
			   argv[0], work_keyfile, sys_errlist[errno]);
	    exit(1);
	}
	if (rename(work_keyfile, keyfile) < 0) {
	    (void) fprintf(stderr, "%s: Failure renaming %s to %s: %s\n",
			   argv[0], work_keyfile, keyfile, 
			   sys_errlist[errno]);
	    exit(1);
	}
	(void) chmod(backup_keyfile, keyfile_mode);
	(void) chmod(keyfile, keyfile_mode);
	(void) printf("Old keyfile in %s.\n", backup_keyfile);
    }

    exit(0);
}

void
print_key(key)
  des_cblock key;
{
    int i;

    for (i = 0; i < 4; i++)
	(void) printf("%02x", key[i]);
    (void) printf(" ");
    for (i = 4; i < 8; i++)
	(void) printf("%02x", key[i]);
}

void
print_name(name, inst, realm)
  char *name;
  char *inst;
  char *realm;
{
    (void) printf("%s%s%s%s%s", name, inst[0] ? "." : "", inst,
		  realm[0] ? "@" : "", realm);
}

int
get_svc_new_key(new_key, sname, sinst, srealm, keyfile)
  des_cblock new_key;
  char *sname;
  char *sinst;
  char *srealm;
  char *keyfile;
{
    int status = KADM_SUCCESS;
    CREDENTIALS c;

    if (use_preauth)
	status = krb_get_svc_in_tkt_preauth(sname, sinst, srealm, PWSERV_NAME,
                                      	    KADM_SINST, 1, keyfile);
    else
	status = krb_get_svc_in_tkt(sname, sinst, srealm, PWSERV_NAME,
                                    KADM_SINST, 1, keyfile);

    if (status != KSUCCESS) {
	/* propagate basic krb_err_txt errors via the error table */
	return status + krb_err_base;
    }

    status = krb_get_cred(PWSERV_NAME, KADM_SINST, srealm, &c);
    
    if (status != KSUCCESS) {
	/* propagate basic krb_err_txt errors via the error table */
	return status + krb_err_base;
    }

    if ((status = kadm_init_link("changepw", KRB_MASTER, srealm)) == 
	KADM_SUCCESS) {
#ifdef NOENCRYPTION
	(void) memset((char *) new_key, 0, sizeof(des_cblock));
	new_key[0] = (unsigned char) 1;
#else /* NOENCRYPTION */
	des_init_random_number_generator(c.session);
	(void) des_new_random_key(new_key);
#endif /* NOENCRYPTION */
	return(KADM_SUCCESS);
    }
    /* kadm_init_link returns com_err error values. */
    return(status);
}

void
get_key_from_password(key)
  des_cblock key;
{
    char password[MAX_KPW_LEN];	/* storage for the password */

    if (read_long_pw_string(password, sizeof(password)-1, "Password: ", 1))
	leave("Error reading password.", 1);

#ifdef NOENCRYPTION
    (void) memset((char *) key, 0, sizeof(des_cblock));
    key[0] = (unsigned char) 1;
#else /* NOENCRYPTION */
    (void) des_string_to_key(password, key);
#endif /* NOENCRYPTION */
    (void) memset((char *)password, 0, sizeof(password));
}    

usage()
{
    (void) fprintf(stderr, "Usage: ksrvutil [-f keyfile] [-i] [-k] [-p] ");
    (void) fprintf(stderr, "{list | change | add | delete}\n");
    (void) fprintf(stderr, "   -i causes the program to ask for ");
    (void) fprintf(stderr, "confirmation before changing keys.\n");
    (void) fprintf(stderr, "   -k causes the key to printed for list or ");
    (void) fprintf(stderr, "change.\n");
    (void) fprintf(stderr, "   -p causes the program to preauthenticate.\n");
    exit(1);
}

void
leave(str,x)
char *str;
int x;
{
    if (str)
	(void) fprintf(stderr, "%s\n", str);
    (void) dest_tkt();
    exit(x);
}
/* simple routine to use the srvtab to get a ticket authenticating to 
   itself and then look at the credential to find out the current version 
   number. 
 */
int get_kvno(sname, sinst, srealm, keyfile, kvno)
     char *sname;
     char *sinst;
     char *srealm;
     char *keyfile;
     int *kvno;
{
  CREDENTIALS c;
  int status;
  if (use_preauth)
    status = krb_get_svc_in_tkt_preauth(sname,sinst,srealm,sname,sinst,1,keyfile);
  else
    status = krb_get_svc_in_tkt(sname,sinst,srealm,sname,sinst,1,keyfile);
  if (status != KSUCCESS) return status;

  status = krb_get_cred(sname,sinst,srealm,&c);
  if (status != KSUCCESS) return status;

  *kvno = c.kvno;
  return KSUCCESS;
}


/* Special trick: ksrvutil can get confused when talking to an out-of-date
   slave, so make it always talk to an admin server first.
 */
krb_get_krbhst(h,r,n)
    char *h;
    char *r;
    int n;
{
  return krb_get_admhst(h, r, n);
}
