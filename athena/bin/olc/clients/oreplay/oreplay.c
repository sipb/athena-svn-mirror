/*
 * Log Replayer Client
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/oreplay/oreplay.c,v 1.5 1990-12-02 13:33:23 lwvanels Exp $";
#endif
#endif

#include "oreplay.h"

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
#ifdef KERBEROS
  KTEXT_ST my_auth;
  int auth_result;
  char server_instance[INST_SZ];
  char server_realm[REALM_SZ];
#endif /* KERBEROS */

  output_fd = 1;
  hp = NULL;

  while ((c = getopt(argc, argv, "f:s:")) != EOF)
    switch(c) {
    case 'f':
      strcpy(filename,optarg);
      if ((output_fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC,
			    S_IREAD|S_IWRITE)) < 0) { 
	perror("oreplay: opening file");
	exit(1);
      }
      break;
    case 's':
      if ((hp = gethostbyname(optarg)) == NULL) {
	fprintf(stderr,"oreplay: Unknown host %s\n",optarg);
	punt(output_fd,filename);
      }
      break;
    case '?':
    default:
      usage();
      exit(1);
    }
  
  instance = 0;

  if (((argc - optind) > 2) || (argc == optind)) {
    usage();
    exit(1);
  }
    
  strcpy(username,argv[optind]);
  optind++;
  if ((argc - optind) == 1)
    instance = atoi(argv[optind]);

  if (hp == NULL) {
#ifdef HESIOD
    if ((olc_servers = hes_resolve("OLC","SLOC")) == NULL) {
      fprintf(stderr,"oreplay: Unable to get hesiod infomration for OLC/SLOC\n");
      punt(output_fd,filename);
    }
    
    if ((hp = gethostbyname(olc_servers[0])) == NULL) {
      fprintf(stderr,"oreplay: Unknown host %s\n",olc_servers[0]);
      punt(output_fd,filename);
    }
#else /* HESIOD */
    fprintf(stderr,"oreplay: no server specified\n");
    punt(output_fd,filename);
#endif /* HESIOD */
  }

  if ((sent = getservbyname("ols","tcp")) == NULL) {
    fprintf(stderr,"oreplay: ols/tcp unknown service\n");
    punt(output_fd,filename);
  }

#ifdef KERBEROS
  expand_hostname(hp->h_name, server_instance, server_realm);
  auth_result = krb_mk_req(&my_auth,K_SERVICE,server_instance,server_realm,0);
  if (auth_result != MK_AP_OK) {
    if (auth_result == MK_AP_TGTEXP) {
      fprintf(stderr,"oreplay: Your kerberos ticket-granting-ticket expired\n"); 
      punt(output_fd,filename);
    }
    else {
      fprintf(stderr,"oreplay: unknown kerberos error: %s\n",
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
    perror("oreplay: socket");
    punt(output_fd,filename);
  }
  
  if (connect(sock,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
    perror("oreplay: connect");
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
    switch (len) {
    case -1:
      fprintf(stderr,"No such question\n");
      break;
    case -2:
      fprintf(stderr,"Error on the server\n");
      break;
    default:
      fprintf(stderr,"Unknown error %d\n",len);
    }
    punt(output_fd,filename);
  }
  buf = malloc(len);
  total_read = 0;
  while (total_read < len) {
    c = read(sock,buf,(int)len);
    write(output_fd,buf,c);
    total_read += c;
  }
  close(sock);
  exit(0);
}

void
usage()
{
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
      free(p);
    }

  for(i=0; instance[i] != '\0'; i++)
    if(isupper(instance[i]))
      instance[i] = tolower(instance[i]);

  
  return;
}
