/*
 * Fast 'n Dirty Client
 *
 * This replays question logs
 *      and gets queue listings
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/oreplay/oreplay.c,v 1.7 1990-12-12 13:03:00 lwvanels Exp $";
#endif
#endif

#include "oreplay.h"

int i_list;

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

char *f_gets P((FILE *input_file , char *a ));
void usage P((void ));
void punt P((int fd , char *filename ));
void expand_hostname P((char *hostname , char *instance , char *realm ));

#undef P


main(argc,argv)
     int argc;
     char **argv;
{
  int sock;    /* the socket */
  long len;
  struct sockaddr_in sin;
  struct servent *sent;
  char username[9];
  int instance;
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
#ifdef KERBEROS
  KTEXT_ST my_auth;
  int auth_result;
  char server_instance[INST_SZ];
  char server_realm[REALM_SZ];
#endif /* KERBEROS */

  i_list = 0;
  gimme_raw = 0;
  output_fd = 1;
  hp = NULL;

  if (!strcmp(argv[0],"olist"))
    i_list = 1;

  while ((c = getopt(argc, argv, "f:s:rl")) != EOF)
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
    case '?':
    default:
      usage();
      exit(1);
    }
  
  if (gimme_raw && !i_list) {
    usage();
    exit(1);
  }

  instance = 0;

  if (i_list) {
    /* list */
    instance = -1;
    strcpy(username,"qlist");
  }
  else {
    /* replay */
    if (((argc - optind) > 2) || (argc == optind)) {
      usage();
      exit(1);
    }
    
    strcpy(username,argv[optind]);
    optind++;
    if ((argc - optind) == 1)
      instance = atoi(argv[optind]);
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
  
  if (connect(sock,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
    perror(" connect");
    punt(output_fd,filename);
  }
  
  version = htonl((int) VERSION);
  write(sock,&version,sizeof(version));
  write(sock,username,9);
  instance = htonl(instance);
  write(sock,&instance,sizeof(instance));
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
      case -11:
	fprintf(stderr,"No such question\n");
	break;
      case -12:
	fprintf(stderr,"Error on the server\n");
	break;
      case -13:
	fprintf(stderr,"Sorry, charlie, but you're not on the acl.\n");
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
  if (!gimme_raw) {
    /* Format list to output it... */
    temp_fd = output_fd;  /* Save the "real" output file descriptor away so */
			  /* we can output to a temp file, which we will */
			  /* then process to look "nice */
    strcpy(templ,"/tmp/oreplayXXXXXX");
    if ((output_fd = mkstemp(templ)) < 0) {
      perror("olist: opening temp file");
      punt(temp_fd,filename);
    }
  }
  while (total_read < len) {
    c = read(sock,buf,(int)len);
    write(output_fd,buf,c);
    total_read += c;
  }
  close(sock);

  if (!gimme_raw) {
    FILE *input_file;
    char qname[10];
    char username[9];
    char machine[80];
    int inst;
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
	machine[20-len] = '*';
	machine[21-len] = '\0';
	inst = atoi(f_gets(input_file,tmp));
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
	  sprintf(obuf,"%-20s[%d]  %-8s %-8s      %-4s %2d %-10s %s %s\n",
		 tmp, inst, status, consultant, cstat, nseen, topic,
		 date, time);
	else
	  sprintf(obuf,"%-20s[%d]  %-8s %-8s [%2d] %-4s %2d %-10s %s %s\n",
		 tmp, inst, status, consultant, cinst, cstat, nseen,
		 topic, date, time);
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

void
expand_hostname(hostname, instance, realm)
     char *hostname;
     char *instance;
     char *realm;
{
  char *p;
  int i;

  realm[0] = '\0';
  p = index(hostname, '.');
  
  if(p == NULL)
    {
      (void) strcpy(instance, hostname);

#ifdef KERBEROS
      krb_get_lrealm(realm,1);
#endif /* KERBEROS */

    }
  else
    {
      i = p-hostname;
      (void) strncpy(instance,hostname,i);
      instance[i] = '\0';
      p = krb_realmofhost(hostname);
      strcpy(realm,p);
    }

  for(i=0; instance[i] != '\0'; i++)
    if(isupper(instance[i]))
      instance[i] = tolower(instance[i]);

  
  return;
}
