/*
    file: lert.c
    main bungler: Mike Barker <mbarker@mit.edu>
    started Dec. 2, 1994
    loosely based on Jeff Schiller's checkmyid program and
    the Athena get_message code
    client-server with Kerberos authentication

    version 1:  put details in a couple of subroutines
                a) call server and get magic letter
                b) use letter to pick a file and send to user

        man page

    later: add user .lertrc file to let user tailor processing
           e.g. frequency (daily, every login, etc.)
                method of display (zephyr, email, more,...)
           do some administrative side processing--let the administrator
                know who has been notified and so forth.

modified 1/11/95
    change timeouts/retries for 5 second
    -q quiet switch
    fallback after zephyr fails to cat...

modified 1/25/95
  mandatory
    check system returns and print error if fails
  niceties
    no_error variable name backwards -- if (err_messages)

modified 1/26/95
    replace bzero with memset
    replace bcopy with memmove
    replace include strings.h with string.h
    replace syserrlist with strerror
    change strrchr to buffer + strlen
    put subject line of mail message in a file (allow configuration)
    use isalnum to avoid bad returned characters
    cleanup printing code (really ugly!) somewhat improved

pending fixes 1/26/95
    use mk_priv instead of mk_auth to protect -n flag

*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <krb.h>
#include <des.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _AIX
#include <sys/select.h>
#endif
#include <fcntl.h>
#include <sys/socket.h>
#include <ndbm.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <hesiod.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include "lert.h"

/*
  bad coding practice and not currently needed
extern int errno;
extern char *sys_errlist[];

use #include <errno.h> and strerror(errno) instead...

 */

static struct timeval timeout = { LERT_TIMEOUT, 0 };

/*
  very bad coding practice, but it gets the job done fast
 */
static int error_messages = TRUE;

void Usage(pname, errname)
     char *pname, *errname;
{
  fprintf(stderr, "%s <%s>: Usage: %s [-zephyr|-z] [-mail|-m] [-no|-n]\n",
	  pname, errname, pname);
}

lert_says(get_lerts_blessing, no_p)
char * get_lerts_blessing;
int no_p;
{
  KTEXT_ST authent;
/*
  KTEXT_ST private;
 */
  unsigned char packet[2048];
  unsigned char ipacket[2048];
  u_long iplen;
  int biplen;
  int gotit;
  struct servent *buggy;
  struct hostent *hp;
  struct sockaddr_in sin, lsin;
  fd_set readfds;
  int i;
  char *ip;
  char **tip;
  int trys;
  char *krb_get_phost();
  char srealm[REALM_SZ];
  char sname[SNAME_SZ];
  char sinst[INST_SZ];
  char *cp;
  Key_schedule sched;
  CREDENTIALS cred;
  int plen;
  int status;
  int s;
  u_long checksum_sent;
  u_long checksum_rcv;
  MSG_DAT msg_data;
  char hostname[256];

  void bombout();

  /*

   find out where lert lives

    do it in this order
    getservbyname()
      struct servent *getservbyname(const char *name,
          char *proto);
    hesiod lookup
    hardcode

    rationale--if someone wants to override for some reason... 
    /etc/services is easier to fix

   */

  buggy = getservbyname(LERT_SERVED, LERT_PROTO);
  if (buggy != NULL){  
    ip = buggy->s_name;
  } else {
    /*
      note the presumption that there is only one lert!
     */
    tip = (hes_resolve(LERT_SERVER, LERT_TYPE));
    if (tip == NULL){
      /*
	No Hesiod available
	fall into hardcoded--below for realmofhost
       */
      ip = LERT_HOME;
    } else {
      ip = tip[0];
    }
  }

  /*
    now begin the real kerberos code
    find out which realm the server is in
    and the host name
    hardcode the service name
    and make a request
   */

  cp = (char *)krb_realmofhost(ip);
  if (cp == NULL) bombout(ERR_KERB_REALM);
  strcpy(srealm, cp);

  /*
    get hostname alias
   */
  cp = krb_get_phost(ip);
  if (cp == NULL) bombout(ERR_KERB_PHOST);
  strcpy(sinst, cp);

  /*
    why isn't this the same service we asked hesiod to resolve?
   */
  strcpy(sname, LERT_SERVICE);

  /*
     int krb_mk_req(authent,service,instance,realm,checksum)
     KTEXT authent;
     char *service;
     char *instance;
     char *realm;
     u_long checksum;

   need to check exactly what format the checksum is...

   note: since we use kerberos private message, we don't
   really need the checksum code anymore...

   */

  checksum_sent = (u_long) no_p;

  status = krb_mk_req(&authent, sname, sinst, srealm, checksum_sent);
  if (status != KSUCCESS) bombout(ERR_KERB_AUTH);

  /*
     now do your client-server exchange
   */

  /*
    zero out the packets
   */
  memset(packet, 0, sizeof(packet));
  memset(ipacket, 0, sizeof(ipacket));

  hp = gethostbyname(ip);

  if (hp == NULL) bombout(ERR_HOSTNAME);
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = hp->h_addrtype;
  memcpy((char *)&sin.sin_addr, (char *)hp->h_addr, 
	       sizeof(hp->h_addr));
  sin.sin_port = htons(LERT_PORT);

  /*
   lert's basic protocol
   client send version, one byte query code, and authentication
  */
  packet[0] = LERT_VERSION;
  packet[1] = no_p;
  memcpy((char *) &packet[LERT_LENGTH], (char *)&authent, sizeof(authent));
  plen = sizeof(authent) + LERT_LENGTH;

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) bombout(ERR_SOCKET);
  if (connect(s, (struct sockaddr *)&sin, sizeof(struct sockaddr)) < 0) bombout(ERR_CONNECT);
  trys = RETRIES;
  gotit = 0;
  while (trys > 0) {
    if (send(s, packet, plen, 0) < 0) bombout(ERR_SEND);
    FD_ZERO(&readfds);
    FD_SET(s, &readfds);
    if ((select(s+1, &readfds, (fd_set *) 0, (fd_set *)0, &timeout) < 1)
	|| !FD_ISSET(s, &readfds)) {
      trys--;
      continue;
    }
    /* 
      use "recvfrom" so we know client's address
      */
/*
    biplen = sizeof(struct sockaddr);
    iplen = recvfrom(s, (char *)ipacket, 2048, 0,
		     (struct sockaddr *)&sin, &biplen);
 */
    iplen = recv(s, (char *)ipacket, 2048, 0);

    if (iplen < 0) bombout(ERR_RCV);
    gotit++;
    break;
  }

  /*
    Get my address
   */
  memset((char *) &lsin, 0, sizeof(lsin));
  i = sizeof(lsin);
  if (getsockname(s, (struct sockaddr *)&lsin, &i) < 0) bombout(LERT_NO_SOCK);

  gethostname(hostname, sizeof(hostname));
  hp = gethostbyname(hostname);
  memcpy((char *) &lsin.sin_addr.s_addr, hp->h_addr, hp->h_length);
 
  shutdown (s, 2);
  close(s);
  if (!gotit) bombout(ERR_TIMEOUT);

  /*
    int krb_get_cred(service,instance,realm,c)
    char *service;
    char *instance;
    char *realm;
    CREDENTIALS *c;

    struct credentials {
    char    service[ANAME_SZ];   Service name 
    char    instance[INST_SZ];   Instance 
    char    realm[REALM_SZ];     Auth domain 
    C_Block session;             Session key 
    int     lifetime;            Lifetime 
    int     kvno;                Key version number 
    KTEXT_ST ticket_st;          The ticket itself 
    long    issue_date;          The issue time 
    char    pname[ANAME_SZ];     Principal's name 
    char    pinst[INST_SZ];      Principal's instance 
    };

    */

  status = krb_get_cred(sname, sinst, srealm, &cred);
  if (status != KSUCCESS) bombout(ERR_KERB_CRED);

  /*

    somehow, I don't think I should be doing this, but...

    need key schedule for session key

    */
  des_key_sched(cred.session, sched);

  /* 

NOTE: not currently implemented...but I'm retaining the comment and
line of code from the cmi program in case.

    The bcopy below is necessary so that on the DECstation the private
    message is word aligned  
  (void) bcopy((char *)ipacket, (char *) private.dat, iplen);
    */

  /*

    long krb_rd_priv(in,in_length,schedule,key,sender,receiver,msg_data)
    u_char *in;
    u_long in_length;
    Key_schedule schedule;
    des_cblock key;
    struct sockaddr_in *sender;
    struct sockaddr_in *receiver;
    MSG_DAT *msg_data;

    struct msg_dat {
    unsigned char *app_data;     pointer to appl data 
    unsigned long app_length;    length of appl data 
    unsigned long hash;          hash to lookup replay 
    int     swap;                swap bytes? 
    long    time_sec;            msg timestamp seconds 
    unsigned char time_5ms;      msg timestamp 5ms units 
    };

    LARGE RED WARNING NOTE!!!!

    the two parameters below (lsin and lsin) are supposed
    to be the sender and receiver--lsin and sin.

    kerberos.p9 has some odd code that messes with the
    time based on the relationship of the addresses (direction bit?)

    it doesn't seem to care (at present) whether the second one
    is a lie or not, and doing so cleared up a bug.

    i.e. this is an ugly, good-for-nothing workaround.  don't
    copy this code, get the library fixed.

    and if you get RD_AP_TIME (37) error returns and can't figure
    out why, try this...

    ugly, ugly, ugly.  don't do this in your own house.  don't
    attempt it with professionals watching, either, cause you'll
    get your hand slapped!

    but it does work.

    EVEN BIGGER RED NOTE!!!!

    ignore previous stupid note.  in reality (which has little to
    do with kerberos) you can put a lot of junk in.  however, try
    mk_priv -- local, remote  (in server for my appl)
    rd_priv -- remote, local  (here in client for my appl)

    make sure you set them up before use--watch for 0's in gdb

    and! voila!  c'est omelette (lots of broken eggs...)

    */

  status = krb_rd_priv(ipacket,
		       iplen,
		       sched, 
		       cred.session,
		       &sin,
		       &lsin,
		       &msg_data);
  if (status) bombout(ERR_KERB_FAKE);

  if (msg_data.app_length == 0) return(LERT_NOT_IN_DB);
  /*
    at this point, we have a packet.  check it out
    [0] LERT_VERSION
    [1] code response
    [2 on] data...

    server responds with version, one byte response, and authentication
    
    bump to version, one byte response,
    plus KTEXT format data -- length, data...

    */
  if (msg_data.app_data[0] != LERT_VERSION) bombout(ERR_VERSION);
  if (msg_data.app_data[1] == LERT_MSG) {
    /*
      apparently we have a message from the server...
      set it up, terminate the string, and do it!
      */

    memcpy(get_lerts_blessing, 
	   &(msg_data.app_data[LERT_CHECK]),
	   msg_data.app_length - LERT_CHECK);
/*
  should already be in packet
    get_lerts_blessing[msg_data.app_length - LERT_CHECK] = '\0';
 */
    return(LERT_GOTCHA);
  } else {
    /*
      better safe than sorry
      */
    get_lerts_blessing[0] = '\0';
  }
  return(LERT_NOT_IN_DB);
}

/*
    general error reporting
 */
  
void bombout(mess)
int mess;
{
  if (error_messages) {
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
	fprintf(stderr, "Lert timed out waiting for response from server.\n");
	break;
      case ERR_SERVER:
      case ERR_KERB_FAKE:
	fprintf(stderr, "Someone is spoofing lert.\n");
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
	fprintf(stderr,
		"lert could not get your name to send messages\n");
	break;
      case NO_PROCS:
	fprintf(stderr,
		"Error when lert ran child processes: %s\n", strerror(errno));
	break;
      case ERR_MEMORY:
	fprintf(stderr,
		"lert doesn't have memory for messages\n");
	break;
      default:
	fprintf(stderr, 
		"A problem (%d) occurred when lert checked her database \n",
		mess);
	break;
      }
    fprintf(stderr, "Please try again later.\n");
    fprintf(stderr, 
	    "If this problem persists, please report it to a consultant.\n");
    }
  exit (1);
}

/*
  programming note: this routine mallocs!  if used repeatedly, make
  sure you free the memory!
 */

char * get_lert_sub()
{
  FILE * myfile;
  char buffer[250];
  char * turkey;
  char * mugwump;

  strcpy(buffer, LERTS_DEF_SUBJECT);

  myfile = fopen(LERTS_MSG_SUBJECT, "r");
  if (myfile != NULL) {
    turkey = fgets(buffer, 250, myfile);
    if (turkey == NULL) strcpy(buffer, LERTS_DEF_SUBJECT);
    fclose(myfile);
    }
  /*
    take care of the line feed read by fgets
    */
  mugwump = strchr(buffer, '\n');
  if (mugwump != NULL)
    *mugwump = '\0';
  /*
    now make a buffer for the subject
    */
  mugwump = (char *)malloc(strlen(buffer) + 1);
  if (mugwump == NULL) bombout(ERR_MEMORY);
  strcpy(mugwump, buffer);
  return(mugwump);
}

/*
  currently supports cat, zephyr, and email messaging...
  why?  because it was there!
  :-)
 */

void view_message(message, type)
char * message;
int type;
{
  char *whoami, *getlogin();
  char *ptr;
  char * tail;
  char * type_buffer;
  char buffer[512];
  struct passwd *pw;
  int status;

/*
  these buffers permit centralized handling of the
  various options.  note that both z and m require
  on the fly construction (at least user name) so
  buffers are kept roomy...
 */

  char zprog[128];
  char mprog[512];
  char cprog[4];

  strcpy(zprog, "zwrite -q ");
  strcpy(mprog, "mhmail ");
  strcpy(cprog, "cat");

  whoami = getenv("USER");
  
  if (!whoami)
    whoami = getlogin();

  if(!whoami) {
    pw = getpwuid(getuid());
    if(pw) {
      whoami = pw->pw_name;
    } else {
      bombout(ERR_USER);
    }
  }
  ptr = message;

  while(*ptr) {
    switch (type) {
    case LERT_Z:
      /*
	zwrite -q whoami
	*/
      type_buffer = zprog;
      strcat(type_buffer, whoami);
      type = LERT_HANDLE;
      break;

    case LERT_MAIL:
      /*
	cat ... | mhmail whoami -subject "lert's msg"
	*/
      type_buffer = mprog;
/*
      strcpy(mprog, "mhmail ");
 */
      strcat(type_buffer, whoami);
      strcat(type_buffer, " -subject \"");
      strcat(type_buffer, get_lert_sub());
      strcat(type_buffer, "\" ");
      type = LERT_HANDLE;
      break;

    case LERT_HANDLE:
      /*
	take care of the case where there is one lonely message
	just combine the two (lert0 and lertx) for the zwrite
	hope the writer of the messages realizes they will be
	jammed together...
	*/
      if (strlen(ptr) == 1) {
	strcpy(buffer, "cat ");
	strcat(buffer, LERTS_MSG_FILES);
	strcat(buffer, "0 ");
	strcat(buffer, LERTS_MSG_FILES);
	if (isalnum(*ptr))
	  strcat(buffer, ptr);
	else
	  bombout(ERR_SERVED);
	strcat(buffer, " | ");
	strcat(buffer, type_buffer);
	status = system(buffer);
	if (status) {
	  /*
	    bad news
            if zwrite or email failed
	    try cat!  lazy code...
	    */
	  if (type_buffer != cprog)
	    type = LERT_CAT;
	  else
	    bombout(NO_PROCS);
	  break;
	}
	ptr++;
      } else {
	/*
	  do multiple message notification
	  */
	strcpy(buffer, type_buffer);
	strcat(buffer, " < ");
	strcat(buffer, LERTS_MSG_FILES);
	/*
	  give ourselves one more space to deal with
	  */
	tail = buffer + strlen(buffer);

	*(tail+1) = '\0';
	/*
	  always start with the lert0
	  */
	*tail = '0';
	status = system(buffer);
	if (status) {
	  /*
	    bad news-- zwrite or email  failed
	    try cat!  lazy code...
	    */
	  if (type_buffer != cprog)
	    type = LERT_CAT;
	  else
	    bombout(NO_PROCS);
	  break;
	}
    
	/*
	  then step through the rest of the messages for this user
	  */
	while (*ptr) {
	  if (isalnum(*ptr))
	    *tail = *ptr;
	  else
	    bombout(ERR_SERVED);
          status = system(buffer);
	  if (status) bombout(NO_PROCS);
	  ptr++;
	}
      }
      break;

    case LERT_CAT:
    default:
      /*
	cat notification
	*/
      type_buffer = cprog;
      type = LERT_HANDLE;
      break;

      }
    }
}

main(argc, argv)
int argc;
char ** argv;
{
  char buffer[512];
  char *get_lerts_blessing;
  char ** xargv = argv;
  int xargc = argc;
  int zephyr_p = LERT_CAT, no_p = FALSE;
  int result;

  get_lerts_blessing = buffer;

  /* Argument Processing:
   * 	-z or -zephyr: send the message as a zephyrgram.
   *    -m or -mail: send the message as email
   * 	-n or -no: no more messages!
   * note: why not use one of the argument processors?
   */
  if(argc>3) {
    Usage(argv[0], "too many arguments");
    exit(1);
  }
  /* note: initialized in declaration */
  while(--xargc) {
    xargv++;
    if((!strcmp(xargv[0],"-zephyr"))||(!strcmp(xargv[0],"-z"))) {
      zephyr_p = LERT_Z;
    } else if((!strcmp(xargv[0],"-mail"))||(!strcmp(xargv[0],"-m"))) {
      zephyr_p = LERT_MAIL;
    } else if((!strcmp(xargv[0],"-no"))||(!strcmp(xargv[0],"-n"))) {
      no_p = TRUE;
    } else if((!strcmp(xargv[0],"-quiet"))||(!strcmp(xargv[0],"-q"))) {
      error_messages = FALSE;
    } else {
      Usage(argv[0], xargv[0]);
      exit(1);
    }      
  }

  /*
    get the server's string for this user
   */
  result = lert_says(get_lerts_blessing, no_p);

  if (result == LERT_GOTCHA) {
    /*
      pass on the selection to the user
      */
    view_message(get_lerts_blessing, zephyr_p);
  }
  exit(0);
}





