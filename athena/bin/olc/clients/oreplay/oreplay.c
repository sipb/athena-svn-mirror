/*
 * Fast 'n Dirty Client
 *
 * This replays question logs
 *      and gets queue listings
 *      and shows new messages
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/oreplay/oreplay.c,v 1.19 1991-03-11 13:42:45 lwvanels Exp $";
#endif
#endif

#include <stdio.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <olc/olc.h>
#include <nl_requests.h>

int i_list;
int i_show;
int select_timeout;

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

char *f_gets P((FILE *input_file , char *a ));
void usage P((void ));
void punt P((int fd , char *filename ));

#undef P

char DaemonHost[MAXHOSTNAMELEN];

main(argc,argv)
     int argc;
     char **argv;
{
  int sock;    /* the socket */
  int len;
  char username[9], tusername[9];
  int instance, tinstance;
  int c;
  char **olc_servers;
  char *buf;
  int bufsiz;
  char filename[128];
  struct hostent *hp;
  int output_fd;
  extern char *optarg;
  extern int optind;
  char templ[80];
  int temp_fd;
  int gimme_raw;
  int nuke;
  long code;

  select_timeout = 600;
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
      fprintf(stderr,"%s: Unable to get hesiod information for OLC/SLOC\n",
	      argv[0]);
      punt(output_fd,filename);
    }
    
    if ((hp = gethostbyname(olc_servers[0])) == NULL) {
      fprintf(stderr,"%s: Unknown host %s\n",argv[0],olc_servers[0]);
      punt(output_fd,filename);
    }
    strncpy(DaemonHost,hp->h_name,MAXHOSTNAMELEN);
#else /* HESIOD */
    fprintf(stderr,"%s: no server specified\n",argv[0]);
    punt(output_fd,filename);
#endif /* HESIOD */
  }
  else
    strncpy(DaemonHost,hp->h_name,MAXHOSTNAMELEN);

  if (open_connection_to_nl_daemon(&sock) != SUCCESS)
    punt(output_fd,filename);

  bufsiz = 4096;
  if ((buf = (char *) malloc(bufsiz)) == NULL) {
    fprintf(stderr,"%s: unable to allocate %d bytes\n",argv[0],bufsiz);
    punt(output_fd,filename);
  }

  if (i_show) { 
    code = nl_get_nm(sock,&buf,&bufsiz,username,instance,nuke,&len);
  } else if (i_list) {
    code = nl_get_qlist(sock,&buf,&bufsiz,&len);
  } else {
    code = nl_get_log(sock,&buf,&bufsiz,username,instance,&len);
  }

  if (code < 0) {
    if (code >= -256)
      switch (code) {
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

  if (!gimme_raw && i_list) {
    /* Format list to output it... */
    temp_fd = output_fd;  /* Save the "real" output file descriptor away so */
			  /* we can output to a temp file, which we will */
			  /* then process to look "nice */
    strcpy(templ,"/tmp/oreplayXXXXXX");
    mktemp(templ);
    if ((output_fd = open(templ,O_RDWR|O_EXCL|O_CREAT,0600)) < 0) {
      perror("olist: opening temp file");
      punt(temp_fd,filename);
    }
  }

  write(output_fd,buf,len);

/* If it's the special case where they want status of the user as well, */
/* continue on */

  if (i_list && (tusername[0] != '\0')) {
    char *p1,*p2,*status; /* current position in buffer */
    int n;        /* number of users in this queue */
    int nq;
    int i,inst;
    int len;
    int right_user;
    char obuf[100];

    p1 = buf;
    len = strlen(tusername);

    nq = atoi(p1);
    p1 = index(p1,'\n')+1;
    
    for(i=0;i<nq;i++) {
      p1 = index(p1,'\n')+1; /* Scan past queue name */
      n = atoi(p1);           /* number of users in this queue */
      p1 = index(p1,'\n')+1;  /* past number of users */
      for(;n>0;n--) {
	if (strncmp(tusername,p1,len) == 0)
	  right_user = 1;
	else
	  right_user = 0;
	p1 = index(p1,'\n')+1;
	p1 = index(p1,'\n')+1;
	inst = atoi(p1);
	p1 = index(p1,'\n')+1;
	p1 = index(p1,'\n')+1;
	status = p1;
	p1 = index(p1,'\n')+1;
	if (strncmp(tusername,p1,len) == 0) {
	  p1 = index(p1,'\n')+1;
	  inst = atoi(p1);
	  p1 = index(p1,'\n')+1;
	  if (strncmp(p1,"off",3) != 0) {
	    p2 = index(p1,'\n');
	    *p2 = '\0';
	    sprintf(obuf,"%d\n%s\n",inst,status);
	    write(output_fd,obuf,strlen(obuf));
	    goto done;               /* Get out of this whole mess */
	  }
	} else {
	  p1 = index(p1,'\n')+1;
	  p1 = index(p1,'\n')+1;
	}
	p1 = index(p1,'\n')+1;
	p1 = index(p1,'\n')+1;
	
	/* username right, check instance */
	if (right_user && (strncmp(p1,"on-duty",7) ==0) ) {
	  p2 = index(status,'\n');
	  *p2 = '\0';
	  sprintf(obuf,"%d\n%s\n",inst,status);
	  write(output_fd,obuf,strlen(obuf));
	  goto done;               /* Get out of this whole mess */
	}
	p1 = index(p1,'\n')+1;
	p1 = index(p1,'\n')+1;
	p1 = index(p1,'\n')+1;
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
    int nc,nq,j;
    
    lseek(output_fd,0,L_SET);
    input_file = fdopen(output_fd,"r+");

    nq = atoi(f_gets(input_file,tmp));
    for(j=0;j<nq;j++) {
      f_gets(input_file,qname);
      nc = atoi(f_gets(input_file,tmp));
      sprintf(obuf,"\n[%s]\n",qname);
      write(temp_fd,obuf,strlen(obuf));
      for(;nc>0;nc--) {
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
