/*
 * kinit.c
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * Routine to initialize user to Kerberos.  Prompts optionally for
 * user, instance and realm.  Authenticates user and gets a ticket
 * for the Kerberos ticket-granting service for future use. 
 *
 * Options are: 
 *   -i[instance]
 *   -r[realm]
 *   -v[erbose]
 *   -l[ifetime]
 * and you can supply a full blown principal.instance@realm arg also.
 */

#include "mit-copyright.h"
#include <stdio.h>
#define	DEFINE_SOCKADDR
#include "krb.h"
#include <string.h>

#define	LIFE	DEFAULT_TKT_LIFE   /* lifetime of ticket in 5-minute units */

char   *progname;

void
get_input(s, size, stream)
char *s;
int size;
FILE *stream;
{
	char *p;

	if (fgets(s, size, stream) == NULL)
	  exit(1);
	if ( (p = strchr(s, '\n')) != NULL)
		*p = '\0';
}


static char
hex_scan_nybble(c)
     char c;
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

/* returns: NULL for ok, pointer to error string for bad input */
static char*
hex_scan_four_bytes(out, in)
     char *out;
     char *in;
{
  int i;
  char c, c1;
  for (i=0; i<8; i++) {
    if(!in[i]) return "not enough input";
    c = hex_scan_nybble(in[i]);
    if(c<0) return "invalid digit";
    c1 = c; i++;
    if(!in[i]) return "not enough input";
    c = hex_scan_nybble(in[i]);
    if(c<0) return "invalid digit";
    *out++ = (c1 << 4) + c;
  }
  switch(in[i]) {
  case 0:
  case '\r':
  case '\n':
    return 0;
  default:
    return "extra characters at end of input";
  }
}

int 
main(argc, argv)
    int     argc;
    char   *argv[];
{
    char    aname[ANAME_SZ];
    char    inst[INST_SZ];
    char    realm[REALM_SZ];
    char    password[BUFSIZ];
    char    buf[MAXHOSTNAMELEN];
    char   *username = NULL;
    char   *usernameptr;
    int     iflag, rflag, vflag, lflag, pflag, sflag, lifetime, k_errno;
    register char *cp;
    register i;
 
#if 0
/* This code is for testing time conversion in micro Kerberos ports */
    {
	extern int krb_debug;
	static long time_0 = 0L;
	static long time_1 = 1L;
	static long time_epoch;
	static long time_epoch2;

	krb_debug = 0xFFF;
	time_epoch = win_time_get_epoch();
	time_epoch2 = -time_epoch;
	printf ("STime at zero: %s\n", krb_stime(&time_0));
	printf ("STime at one: %s\n", krb_stime(&time_1));
	printf ("STime at Epoch: %s\n", krb_stime(&time_epoch));
	printf ("STime at -Epoch: %s\n", krb_stime(&time_epoch2));
	printf ("CTime at zero: %s\n", ctime(&time_0));
	printf ("CTime at one: %s\n", ctime(&time_1));
	printf ("CTime at Epoch: %s\n", ctime(&time_epoch));
	printf ("CTime at -Epoch: %s\n", ctime(&time_epoch2));
    }
#endif

    *inst = *realm = '\0';
    iflag = rflag = vflag = lflag = pflag = sflag = 0;
    lifetime = LIFE;

    /* Take last component with either / or \ as path separator.  */
    cp = strrchr(*argv, '/');
    progname = cp ? cp + 1 : *argv;
    cp = strrchr(progname, '\\');
    progname = cp ? cp + 1 : progname;

    while (--argc) {
	if ((*++argv)[0] != '-') {
	    if (username)
		usage();
	    username = *argv;
	    continue;
	}
	for (i = 1; (*argv)[i] != '\0'; i++)
	    switch ((*argv)[i]) {
	    case 'i':		/* Instance */
		++iflag;
		continue;
	    case 'r':		/* Realm */
		++rflag;
		continue;
	    case 'v':		/* Verbose */
		++vflag;
		continue;
	    case 'l':
		++lflag;
		continue;
            case 'p':
                ++pflag;
                continue;
            case 's':
                ++sflag;
                continue;
	    default:
		usage();
		exit(1);
	    }
    }

    krb_start_session ((char *)NULL);

    if (username &&
	(k_errno = kname_parse(aname, inst, realm, username))
	!= KSUCCESS) {
	fprintf(stderr, "%s: %s\n", progname, krb_get_err_text(k_errno));
	iflag = rflag = 1;
	username = NULL;
    }

    buf[0] = '\0';
    if (GETHOSTNAME (buf, MAXHOSTNAMELEN))
      buf[0] = 0;

    if (buf[0])
      printf ("Kerberos initialization on %s\n", buf);
    else
      printf("Kerberos initialization\n");
    if (!username) {
	printf("Kerberos name: ");
	get_input(aname, sizeof(aname), stdin);
	if (!*aname)
	    exit(0);
    }
    /* optional instance */
    if (iflag) {
	printf("Kerberos instance: ");
	get_input(inst, sizeof(inst), stdin);
    }
    if (rflag) {
	printf("Kerberos realm: ");
	get_input(realm, sizeof(realm), stdin);
    }
    if (lflag) {
	 printf("Kerberos ticket lifetime (minutes): ");
	 get_input(buf, sizeof(buf), stdin);
	 lifetime = atoi(buf);
	 if (lifetime < 5)
	      lifetime = 1;
	 else
	      lifetime /= 5;
	 /* This should be changed if the maximum ticket lifetime */
	 /* changes */
	 if (lifetime > 255)
	      lifetime = 255;
    }
    if (!*realm && krb_get_lrealm(realm, 1)) {
	fprintf(stderr, "%s: Error attempting to determine local Kerberos realm name\n", progname);
	exit(1);
    }

 
    if (!sflag) {
    /*
     * On the Mac and Windows, we must now prompt for a password
     * before entering the Kerberos library.  This is a slight
     * behaviour change.
     */
#if defined(_WINDOWS) || defined(macintosh)
    printf("Password: ");
    get_input(password, sizeof(password), stdin);
#else
    password[0] = '\0';
#endif

    if (pflag) {
      k_errno = krb_get_pw_in_tkt_preauth(aname, inst, realm, "krbtgt", realm,
					  lifetime, 
					  password[0]? password: (char *)0);
    } else {
      k_errno = krb_get_pw_in_tkt(aname, inst, realm, "krbtgt", realm,
				  lifetime, password[0]? password: (char *)0);
    }
    } else { /* sflag is set, so we do snk4 support instead */
      /* for the SNK4 we have to get the ticket explicitly. */

      KTEXT_ST cip_st;
      KTEXT cip = &cip_st;	/* Returned Ciphertext */
      KTEXT_ST cip2_st;
      KTEXT cip2 = &cip2_st;	/* Returned Ciphertext */
      /* preauth is impossible in this context */
      if (pflag)
	{
	  fprintf(stderr, "%s: preauth not supported with SNK device\n");
	  exit(1);
	}

      /* flag the instance to get the right tickets */
      strcat(inst, "+SNK4");

      if (k_errno = krb_mk_in_tkt_preauth(aname, inst, realm, 
					  "krbtgt", realm,
					  lifetime, (char*)0, 0, cip))
	{
	    fprintf(stderr, "%s: %s for principal %s%s%s@%s\n", progname,
		    krb_get_err_text(k_errno), aname, inst[0]?".":"", inst,
		    realm);
	    exit(1);
	}

      /* here we've got the response. Pull off the challenge. */
      /* decrypt_tkt (user, instance, realm, arg, key_proc, &cip); */
      {
	static char version[] = "cnssnk01";
	des_cblock key1, key2;
	char challenge1[8], challenge2[8];
	char resp1[128], resp2[128];
	des_key_schedule ks;
	char *complain;

	if (memcmp (cip->dat, version, 8)) {
	  /* perhaps try the normal decrypt in this case? */
	  fprintf(stderr, "%s: not an SNK response packet\n", progname);
	  exit(1);
	}
	memcpy(challenge1, cip->dat+8, 8);
	memcpy(challenge2, cip->dat+16, 8);
	
	/* add input checks */
	if (memcmp (challenge1, challenge2, 8)) {
	  complain = NULL;
	  do {
	    if(complain) printf("Bad input: %s\n", complain);
	    printf("Challenge #1: %s\nResponse #1:", challenge1);
	    get_input(resp1, sizeof(resp1), stdin);
	  } while(complain = hex_scan_four_bytes((char*)&key1, resp1));
	  complain = NULL;
	  do {
	    if(complain) printf("Bad input: %s\n", complain);
	    printf("Challenge #2: %s\nResponse #2:", challenge2);
	    get_input(resp2, sizeof(resp2), stdin);
	  } while(complain = hex_scan_four_bytes(4+(char*)&key1, resp2));
	} else {
	  complain = NULL;
	  do {
	    if(complain) printf("Bad input: %s\n", complain);
	    printf("Challenge: %s\nResponse:", challenge1);
	    get_input(resp1, sizeof(resp1), stdin);
	  } while(complain = hex_scan_four_bytes((char*)&key1, resp1));
	  memcpy(4+(char*)&key1, (char*)&key1, 4);
	  strcpy(resp2, resp1);
	}
	des_fixup_key_parity(key1);
	des_key_sched(key1, ks);
	des_ecb_encrypt(&key1, &key2, ks, 1);
	des_fixup_key_parity(key2);

	des_key_sched(key2,ks);
	pcbc_encrypt((C_Block *)(cip->dat+24),(C_Block *)cip2->dat,
		     (long) (cip->length-24),ks,(C_Block *)&key2,0);
	cip2->length = cip->length-24;

      }
    
      /* if this fails, perhaps the user should get another chance
	 at the input? */
      if (k_errno = krb_parse_in_tkt(aname, inst, realm, 
				     "krbtgt", realm, lifetime, cip2))
	{
	  fprintf(stderr, "%s: %s\n", progname, krb_get_err_text(k_errno));
	  exit(1);
	}
    }

    if (vflag) {
	printf("Result from realm %s: ", realm);
	printf("%s\n", krb_get_err_text(k_errno));
    } else if (k_errno) {
        fprintf(stderr, "%s: %s for principal %s%s%s@%s\n", progname,
		krb_get_err_text(k_errno), aname, inst[0]?".":"", inst, realm);
	exit(1);
    }

#if 0
    /* More testing code for micro ports */
 
    usernameptr = krb_get_default_user();
    printf ("Default user is `%s'.\n", usernameptr);

    k_errno = krb_set_default_user ("Bogon");
    printf ("Set default user result is %d\n", k_errno);

    printf ("Now default user is `%s'.\n", krb_get_default_user());
    
    k_errno = krb_set_default_user (usernameptr);

    /* Test credentials caching code 
     * Tests are order dependent and depend on the results of previous tests.
     */
    {
    int numcreds;
    CREDENTIALS cr;
    char    aname[ANAME_SZ];
    char    inst[INST_SZ];
    char    realm[REALM_SZ];
    
    /* krb_get_num_cred 
     * returns the number of credentials in the default cache.
     */
    numcreds = krb_get_num_cred();
    printf ("After getting tgt...\n");
    printf ("The number of credentials in the default cache:%d.\n", numcreds);
    
    /* krb_get_tf_fullname
     * For the ticket file name input, return the principal's name, 
     * instance, and realm.	 Currently the ticket file name is 
     * unused because there is just one default ticket file/cache.
     */
     
     printf ("krb_get_tf_fullname returns %d.\n", 
     	krb_get_tf_fullname ((char*)0, aname, inst, realm));
	 printf ("principal's name: %s, instance: %s, realm: %s. \n", 
	 	aname, inst, realm);		
    
	/* krb_get_nth_cred 
	 * returns service name, service instance and realm of the nth credential. 
	 */
	 
	printf ("Get the service name, instance, and realm of the last credential.\n");
	printf ("krb_get_nth_cred returns %d.\n", 
			krb_get_nth_cred(aname, inst, realm, numcreds));
	printf ("service name: %s, instance: %s, realm: %s.\n",
    		aname, inst, realm);	
    		
    /* krb_get_cred 
     * fills in a cred with info from the first cred in the default cache
     * with matching sname, inst, and realm 
     */
    printf ("Make a new cred and fill it in with info from the cred\n");
    printf ("in the default cache with matching service name, inst and realm.\n");
    
    printf ("krb_get_cred returns: %d.\n", krb_get_cred(aname, inst, realm, &cr));

    printf ("service: %s, instance: %s, realm: %s\n", cr.service, cr.instance, cr.realm);
  	printf ("pname: %s, pinst: %s.\n", cr.pname, cr.pinst);
    
    /* krb_save_credentials 
     * adds a new credential to the default cache, using information input
     */ 
    printf ("Add a new cred to the cache, using mostly values from the cred made above\n");
    printf ("krb_save_credentials returns: %d.\n",
    		 krb_save_credentials ("bogus", cr.instance, cr.realm, 
    							   cr.session, cr.lifetime, cr.kvno, 
    							   &cr.ticket_st, cr.issue_date));
    							   
    printf ("number of credentials in the cache: %d.\n", krb_get_num_cred());			  
    							      
    /* krb_delete_cred 	
     * deletes the first credential in the default cache 
     * with matching service name, instance and realm. 
     */		
     printf ("Delete the first cred in the cache with matching service name, inst, realm\n");
     printf ("krb_delete_cred returns: %d.\n",
     		  krb_delete_cred ("bogus", inst, realm));
     
     printf ("number of credentials in the cache: %d.\n", krb_get_num_cred());			  
    } /* end credentials caching tests */
    
  {
	char buf[1500];
	unsigned KRB_INT32 buflen;
	des_cblock sessionKey;
	Key_schedule schedule;

	k_errno = krb_get_ticket_for_service ("rcmd.toad", buf, &buflen,
		0, sessionKey, schedule, "", 0);
	printf ("krb_get_ticket errno = %d (%s)\n", k_errno,
		krb_get_err_text (k_errno));
    }

#endif

    krb_end_session ((char *)NULL);
    return 0;
}

usage()
{
    fprintf(stderr, "Usage: %s [-irvlps] [name][.instance][@REALM]\n", progname);
    exit(1);
}
