/*
 * Fast 'n Dirty Client
 *
 * This replays question logs
 *      and gets queue listings
 *      and shows new messages
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/oreplay/oreplay.c,v 1.17 1991-01-27 15:05:14 lwvanels Exp $";
#endif
#endif

#include "oclient.h"

int i_list;
int i_show;

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

char *f_gets P((FILE *input_file , char *a ));
void usage P((void ));
void punt P((int fd , char *filename ));

#undef P


main(argc,argv)
     int argc;
     char **argv;
{
  int sock;    /* the socket */
  long len;
  struct sockaddr_in sin;
  struct servent *sent;
  char username[9], tusername[9];
  int instance, tinstance;
  int c;
  char **olc_servers;
  char *buf;
  char filename[128];
  int total_read;
  struct hostent *hp;
  int output_fd;
  extern char *optarg;
  extern int optind;
  int version;
  char templ[80];
  int temp_fd;
  int gimme_raw;
  int nuke;
#ifdef KERBEROS
  KTEXT_ST my_auth;
  int auth_result;
  char server_instance[INST_SZ];
  char server_realm[REALM_SZ];
#endif /* KERBEROS */

  i_list = 0;
  gimme_raw = 0;
  output_fd = 1;
  nuke = 0;
  hp = NULL;
  i_show = 0;

  buf = rindex(argv[0],'/');
  if (buf != NULL) argv[0] = buf+1;
  if (!strcmp(argv[0],"olist"))
    i_list = 1;

  if (!strcmp(argv[0],"oshow"))
    i_show = 1;

  while ((c = getopt(argc, argv, "f:s:rnl")) != EOF)
    switch(c) {
    case 'f':
      strcpy(filename,optarg);
      if ((output_fd = open(filename, O_RDWR|O_CREAT|O_TRUNC,
			    S_IREAD|S_IWRITE)) < 0) { 
	perror("oreplay: opening file");
	exit(1);
      }
      break;
    case 's':
      if ((hp = gethostbyname(optarg)) == NULL) {
	fprintf(stderr,"%s: Unknown host %s\n",argv[0],optarg);
	punt(output_fd,filename);
      }
      break;
    case 'l':
      i_list = 1;
      break;
    case 'r':
      gimme_raw = 1;
      break;
    case 'n':
      nuke = 1;
      break;
    case '?':
    default:
      usage();
      exit(1);
    }
  
  if (gimme_raw && !i_list) {
    usage();
    exit(1);
  }

  if ((nuke && i_list) || (i_show && i_list)) {
    usage();
    exit(1);
  }

  instance = 0;

  if (i_list) {
    /* list */
    instance = -1;
    strcpy(username,"qlist");
    switch (argc-optind) {
    case 2:
      tinstance = atoi(argv[optind+1]);
    case 1:
      strcpy(tusername,argv[optind]);
    }      
  }
  else {
    /* replay/show */
    if ((((argc - optind) > 2) && !nuke) ||
	(argc == optind)) {
      usage();
      exit(1);
    }
    
    strcpy(username,argv[optind++]);
    switch (argc-optind) {
    case 3:
      tinstance = atoi(argv[optind+2]);
    case 2:
      strcpy(tusername,argv[optind+1]);
    case 1:
      instance = atoi(argv[optind]);
    }
  }

/* Find out where the server is */

  if (hp == NULL) {
#ifdef HESIOD
    if ((olc_servers = hes_resolve("OLC","SLOC")) == NULL) {
      fprintf(stderr,"%s: Unable to get hesiod infomration for OLC/SLOC\n",
	      argv[0]);
      punt(output_fd,filename);
    }
    
    if ((hp = gethostbyname(olc_servers[0])) == NULL) {
      fprintf(stderr,"%s: Unknown host %s\n",argv[0],olc_servers[0]);
      punt(output_fd,filename);
    }
#else /* HESIOD */
    fprintf(stderr,"%s: no server specified\n",argv[0]);
    punt(output_fd,filename);
#endif /* HESIOD */
  }

  if ((sent = getservbyname("ols","tcp")) == NULL) {
    fprintf(stderr,"%s: ols/tcp unknown service\n",argv[0]);
    punt(output_fd,filename);
  }

#ifdef KERBEROS
  expand_hostname(hp->h_name, server_instance, server_realm);
  auth_result = krb_mk_req(&my_auth,K_SERVICE,server_instance,server_realm,0);
  if (auth_result != MK_AP_OK) {
    if (auth_result == MK_AP_TGTEXP) {
      fprintf(stderr,"%s: Your kerberos ticket-granting-ticket expired\n",argv[0]); 
      punt(output_fd,filename);
    }
    else {
      fprintf(stderr,"%s: unknown kerberos error: %s\n",argv[0],
	      krb_err_txt[auth_result]);
      punt(output_fd,filename);
    }
  }
#endif KERBEROS

  bzero(&sin,sizeof(sin));
  bcopy(hp->h_addr,(char *)&sin.sin_addr,hp->h_length);
  sin.sin_family = hp->h_addrtype;
  sin.sin_port = sent->s_port;
  
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    punt(output_fd,filename);
  }
  
  /* This is a hack; the connection is occasionally refused the first time */
  /* for an unknown reason */

  if (connect(sock,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
    perror(" connect");
    punt(output_fd,filename);
  }
  
  version = htonl((u_long) VERSION);
  write(sock,&version,sizeof(version));

  if (i_show)
    version = htonl((u_long) (nuke ? SHOW_KILL_REQ : SHOW_NO_KILL_REQ));
  else
    if (nuke) {
      version = htonl((u_long) REPLAY_KILL_REQ);
    } else {
      version = htonl((u_long) LIST_REQ);
    }

  write(sock,&version,sizeof(version));

  write(sock,username,9);
  instance = htonl(instance);
  write(sock,&instance,sizeof(instance));

  if (!i_show && nuke) {
    write(sock,tusername,9);
    tinstance = htonl(tinstance);
    write(sock,&tinstance,sizeof(tinstance));
  }

#ifdef KERBEROS
  my_auth.length = htonl(my_auth.length);
  write(sock,&(my_auth.length),sizeof(my_auth.length));
  write(sock,my_auth.dat,ntohl(my_auth.length));
#endif KERBEROS

  read(sock,&len,sizeof(len));
  len = ntohl(len);
  if (len < 0) {
    if (len >= -256)
      switch (len) {
      case ERR_NO_SUCH_Q:
	if (i_show)
	  fprintf(stderr,"No new messages\n");
	else
	  fprintf(stderr,"No such question\n");
	break;
      case ERR_SERV:
	fprintf(stderr,"Error on the server\n");
	break;
      case ERR_NO_ACL:
	fprintf(stderr,"Sorry, charlie, but you're not on the acl.\n");
	break;
      case ERR_OTHER_SHOW:
        fprintf(stderr,"You can't delete someone else's new messages..\n");
        fprintf(stderr,"Don't use the -n option if you really want to be nosy.\n");
	break;
      case ERR_NOT_HERE:
	fprintf(stderr,"Unknown request\n");
	break;
      default:
	fprintf(stderr,"%s\n",krb_err_txt[-len]);
      }
    else
      fprintf(stderr,"Unknown error %d\n",-len);
    punt(output_fd,filename);
  }
  buf = malloc(len);
  total_read = 0;
  if (!gimme_raw && i_list) {
    /* Format list to output it... */
    temp_fd = output_fd;  /* Save the "real" output file descriptor away so */
			  /* we can output to a temp file, which we will */
			  /* then process to look "nice */
    strcpy(templ,"/tmp/oreplayXXXXXX");
    mktemp(templ);
    if ((output_fd = open(templ,O_RDWR|O_EXCL|O_CREAT,0644)) < 0) {
      perror("olist: opening temp file");
      punt(temp_fd,filename);
    }
  }

  while (total_read < len) {
    c = read(sock,(buf + total_read),(int)len);
    total_read += c;
  }
  write(output_fd,buf,total_read);
  close(sock);

/* If it's the special case where they want status of the user as well, */
/* continue on */

  if (i_list && (tusername[0] != '\0')) {
    char *p1,*p2; /* current position in buffer */
    int n;        /* number of users in this queue */
    int i,j;
    int len;
    char obuf[100];

    p1 = buf;
    len = strlen(tusername);

    for(i=0;i<2;i++) {
      p1 = index(p1,'\n')+1; /* Scan past queue name */
      n = atoi(p1);           /* number of users in this queue */
      p1 = index(p1,'\n')+1;  /* past number of users */
      for(;n>0;n--) {
	for(j=0;j<4;j++)
	  p1 = index(p1,'\n')+1;  /* Scan past the user info */
	if (strncmp(tusername,p1,len) == 0) {
	  /* username right, check instance */
	  p1 = index(p1,'\n')+1;
	  tinstance = atoi(p1);
	  p1 = index(p1,'\n')+1;
	  p2 = index(p1,'\n')+1;
	  *p2 = '\0';
	  if (strncmp(p1,"off",3) !=0 ) {
	    sprintf(obuf,"%d\n%s\n",tinstance,p1);
	    write(output_fd,obuf,strlen(obuf));
	    goto done;               /* Get out of this whole mess */
	  }
	}
	else
	  p1 = index(p1,'\n')+1;
	
	for(j=0;j<7;j++)   /* Skip rest of information for this user */
	  p1 = index(p1,'\n')+1;
      }
    }
    write(output_fd,"off\n",4);
  }

 done: 

/* format listing if neccessary */

  if (!gimme_raw && i_list) {
    FILE *input_file;
    char qname[10];
    char username[9];
    char machine[80];
    int inst;
    char  login_stat[4];
    char  status[10];
    char consultant[9];
    int cinst;
    char cstat[10];
    int nseen;
    char topic[10];
    char date[6];
    char time[5];
    char descr[128];
    char tmp[40];
    char obuf[1024];
    int len;
    int nq,j;
    
    lseek(output_fd,0,L_SET);
    input_file = fdopen(output_fd,"r+");

    for(j=0;j<3;j++) {
      f_gets(input_file,qname);
      nq = atoi(f_gets(input_file,tmp));
      sprintf(obuf,"\n[%s]\n",qname);
      write(temp_fd,obuf,strlen(obuf));
      for(;nq>0;nq--) {
	f_gets(input_file,username);
	f_gets(input_file,machine);
	len = strlen(username);
	machine[18-len] = '*';
	machine[19-len] = '\0';
	inst = atoi(f_gets(input_file,tmp));
	f_gets(input_file,login_stat);
	f_gets(input_file,status);
	f_gets(input_file,consultant);
	cinst = atoi(f_gets(input_file,tmp));
	f_gets(input_file,cstat);
	nseen = atoi(f_gets(input_file,tmp));
	f_gets(input_file,topic);
	f_gets(input_file,date);
	f_gets(input_file,time);
	f_gets(input_file,descr);
	sprintf(tmp,"%s@%s",username,machine);
	if (cinst <0)
	  sprintf(obuf,"%-20s[%d] %1s %-8s %-8s          %2d %-10s %s %s\n",
		  tmp, inst, login_stat, status, consultant, nseen, topic,
		  date, time);
	else
	  sprintf(obuf,"%-20s[%d] %1s %-8s %-8s[%2d] %-4s %2d %-10s %s %s\n",
		  tmp, inst, login_stat, status, consultant, cinst, cstat,
		  nseen, topic, date, time);
	write(temp_fd,obuf,strlen(obuf));
      }
    }
    fclose(input_file);
    unlink(templ);
  }
  exit(0);
}

char *
f_gets(input_file,a)
     FILE *input_file;
     char *a;
{
  char *p;

  fgets(a,1000,input_file);
  p = index(a,'\n');
  if (p != NULL) *p = '\0';
  return(a);
}  

void
usage()
{
  if (i_list)
    fprintf(stderr,"Usage: olist [-f filename] [-s server] [-r]\n");
  else if (i_show)
    fprintf(stderr,"Usage: oshow [-f filename] [-s server] [-n] username instance\n");
  else
    fprintf(stderr,"Usage: oreplay [-f filename] [-s server] username instance\n");
}

void
punt(fd,filename)
     int fd;
     char *filename;
{
  close(fd);
  unlink(filename);
  exit(1);
}
