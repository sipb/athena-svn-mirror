/*
 * Fast 'n Dirty Client
 *
 * This replays question logs
 *      and gets queue listings
 *      and shows new messages
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/oreplay/oreplay.c,v 1.27 1997-04-30 17:56:54 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
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

char *f_gets P((FILE *input_file , char *a, int n ));
void usage P((void ));
void punt P((int fd , char *filename ));
static void format_listing P((int input_fd, int output_fd));
#undef P

extern char DaemonHost[];

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
  char *inst = "olc";
  char *buf, *config, *cf_server;
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

  buf = strrchr(argv[0],'/');
  if (buf != NULL) argv[0] = buf+1;
  if (!strcmp(argv[0],"olist"))
    i_list = 1;

  if (!strcmp(argv[0],"oshow"))
    i_show = 1;

  config = getenv("OLXX_CONFIG");
  if (! config)
    config = OLC_CONFIG_PATH;

  while ((c = getopt(argc, argv, "C:f:s:rnlp")) != EOF)
    switch(c) {
    case 'C':	/* -C path: set path for the configuration file */
      config = optarg;
      break;
    case 'f':	/* -f file: save output in FILE */
      strcpy(filename,optarg);
      if ((output_fd = open(filename, O_RDWR|O_CREAT|O_TRUNC,
			    S_IREAD|S_IWRITE)) < 0) { 
	perror("oreplay: opening file");
	exit(1);
      }
      break;
    case 'i':	/* -i inst: use service INST (eg. "-i olta") */
      inst = optarg;
      /* Downcase the instance, so we can use it as the cfg file name. */
      for (buf=inst ; *buf ; buf++)
	*buf = tolower(*buf);
      break;
    case 's':	/* -s host: use server HOST */
      hp = gethostbyname(optarg);
      if (hp == NULL) {
	fprintf(stderr,"%s: Unknown host %s\n",argv[0],optarg);
	punt(output_fd,filename);
      }
      break;
    case 'l':	/* -l: "oreplay -l" == "olist" */
      i_list = 1;
      break;
    case 'p':	/* -p: "oreplay -p" == "oshow" */
      i_show = 1;
      break;
    case 'r':	/* -r: (olist) display raw, unformatted output */
      gimme_raw = 1;
      break;
    case 'n':	/* -n: (oshow) mark messages read */
      nuke = 1;
      break;
    case '?':	/* unknown option... */
    default:
      usage();
    }
  
  if (gimme_raw && !i_list) {
    usage();
  }

  if ((nuke && i_list) || (i_show && i_list)) {
    usage();
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

  if (incarnate(inst, config) == FATAL) {
    /* Fatal problem.  Messages indicating causes were already displayed... */
    exit(1);
  }

/* Find out where the server is */

#ifdef HESIOD
  if (hp == NULL) {
    olc_servers = hes_resolve(inst, OLC_SERV_NAME);
    if (olc_servers == NULL) {
      fprintf(stderr,"%s: Unable to get hesiod information for %s.%s\n",
	      inst, argv[0], OLC_SERV_NAME);
      punt(output_fd,filename);
    }
    
    hp = gethostbyname(olc_servers[0]);
    if (hp == NULL) {
      fprintf(stderr,"%s: Unknown host %s\n",argv[0],olc_servers[0]);
      punt(output_fd,filename);
    }
  }
#endif /* HESIOD */
  if (hp == NULL) {
    cf_server = client_hardcoded_server();
    if (cf_server) {
      hp = gethostbyname(cf_server);
      if (hp == NULL) {
	fprintf(stderr,"%s: Unknown host %s\n",argv[0],cf_server);
	punt(output_fd,filename);
      }
    }
  }
  if (hp == NULL) {
    fprintf(stderr,"%s: no server specified\n",argv[0]);
    punt(output_fd,filename);
  }

  strncpy(DaemonHost,hp->h_name,MAXHOSTNAMELEN);

  if (open_connection_to_nl_daemon(&sock) != SUCCESS)
    punt(output_fd,filename);

  bufsiz = 4096;
  buf = malloc(bufsiz);
  if (buf == NULL) {
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
#ifdef KERBEROS
	fprintf(stderr,"Kerberos Error: %s\n",krb_err_txt[-len]);
#else
	fprintf(stderr,"Unknown Error %d\n",-len);
#endif
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
    output_fd = open(templ,O_RDWR|O_EXCL|O_CREAT,0600);
    if (output_fd < 0) {
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
    p1 = strchr(p1,'\n')+1;
    
    for(i=0;i<nq;i++) {
      p1 = strchr(p1,'\n')+1; /* Scan past queue name */
      n = atoi(p1);           /* number of users in this queue */
      p1 = strchr(p1,'\n')+1;  /* past number of users */
      for(;n>0;n--) {
	if (strncmp(tusername,p1,len) == 0)
	  right_user = 1;
	else
	  right_user = 0;
	p1 = strchr(p1,'\n')+1;
	p1 = strchr(p1,'\n')+1;
	inst = atoi(p1);
	p1 = strchr(p1,'\n')+1;
	p1 = strchr(p1,'\n')+1;
	status = p1;
	p1 = strchr(p1,'\n')+1;
	if (strncmp(tusername,p1,len) == 0) {
	  p1 = strchr(p1,'\n')+1;
	  inst = atoi(p1);
	  p1 = strchr(p1,'\n')+1;
	  if (strncmp(p1,"off",3) != 0) {
	    p2 = strchr(p1,'\n');
	    *p2 = '\0';
	    sprintf(obuf,"%d\n%s\n",inst,status);
	    write(output_fd,obuf,strlen(obuf));
	    goto done;               /* Get out of this whole mess */
	  }
	} else {
	  p1 = strchr(p1,'\n')+1;
	  p1 = strchr(p1,'\n')+1;
	}
	p1 = strchr(p1,'\n')+1;
	p1 = strchr(p1,'\n')+1;
	
	/* username right, check instance */
	if (right_user && (strncmp(p1,"on-duty",7) ==0) ) {
	  p2 = strchr(status,'\n');
	  *p2 = '\0';
	  sprintf(obuf,"%d\n%s\n",inst,status);
	  write(output_fd,obuf,strlen(obuf));
	  goto done;               /* Get out of this whole mess */
	}
	p1 = strchr(p1,'\n')+1;
	p1 = strchr(p1,'\n')+1;
	p1 = strchr(p1,'\n')+1;
	p1 = strchr(p1,'\n')+1;
      }
    }
    write(output_fd,"off\n",4);
  }

 done: 

/* format listing if neccessary */

  if (!gimme_raw && i_list) {
    format_listing(output_fd,temp_fd);
    unlink(templ);
  }
  exit(0);
}

static void
format_listing(input_fd,output_fd)
     int input_fd;
     int output_fd;
{
  FILE *input_file;
  char qname[10];
  char username[10];
  char machine[80];
  int inst;
  char  login_stat[4];
  char  status[10];
  char consultant[10];
  int cinst;
  char cstat[10];
  int nseen;
  char topic[20];
  char date[10];
  char time[10];
  char descr[128];
  char tmp[40];
  char obuf[1024];
  int len;
  int nc,nq,j;
  
  lseek(input_fd,0,L_SET);
  input_file = fdopen(input_fd,"r+");
  
  nq = atoi(f_gets(input_file,tmp,40));
  for(j=0;j<nq;j++) {
    f_gets(input_file,qname,10);
    nc = atoi(f_gets(input_file,tmp,40));
    sprintf(obuf,"\n[%s]\n",qname);
    write(output_fd,obuf,strlen(obuf));
    for(;nc>0;nc--) {
      f_gets(input_file,username,10);
      f_gets(input_file,machine,80);
      len = strlen(username);
      machine[18-len] = '*';
      machine[19-len] = '\0';
      inst = atoi(f_gets(input_file,tmp,40));
      f_gets(input_file,login_stat,4);
      f_gets(input_file,status,10);
      f_gets(input_file,consultant,10);
      cinst = atoi(f_gets(input_file,tmp,40));
      f_gets(input_file,cstat,10);
      nseen = atoi(f_gets(input_file,tmp,40));
      f_gets(input_file,topic,20);
      f_gets(input_file,date,10);
      f_gets(input_file,time,10);
      f_gets(input_file,descr,128);
      sprintf(tmp,"%s@%s",username,machine);
      if (cinst <0)
	sprintf(obuf,"%-20s[%d] %1s %-8s %-8s          %2d %-10s %s %s\n",
		tmp, inst, login_stat, status, consultant, nseen, topic,
		date, time);
      else
	sprintf(obuf,"%-20s[%d] %1s %-8s %-8s[%2d] %-4s %2d %-10s %s %s\n",
		tmp, inst, login_stat, status, consultant, cinst, cstat,
		nseen, topic, date, time);
      write(output_fd,obuf,strlen(obuf));
    }
  }
  fclose(input_file);
  return;
}

char *
f_gets(input_file,a,n)
     FILE *input_file;
     char *a;
     int n;
{
  char *p;

  fgets(a,n,input_file);
  p = strchr(a,'\n');
  if (p != NULL) *p = '\0';
  return(a);
}  

void
usage()
{
  if (i_list)
    fprintf(stderr,"Usage: olist      [olist_options]\n"
		   "  or   oreplay -l [olist_options]\n"
		   "olist_options:\n"
		   "   -r            raw output (instead of formatted)\n");
  else if (i_show)
    fprintf(stderr,"Usage: oshow      [oshow_options] username instance\n"
		   "  or   oreplay -p [oshow_options] username instance\n"
		   "oshow_options:\n"
		   "   -n            mark the retrieved messages as read\n");
  else
    fprintf(stderr,"Usage: oreplay [oreplay_options] username instance\n"
		   "oreplay_options:\n");
  /* shared options */
  fprintf(stderr,"   -f filename   save output in FILENAME\n"
		 "   -i service    use SERVICE (olc/olta/owl)\n"
		 "   -s host       use server HOST\n"
		 "   -C path       use PATH to search for configuration\n");
  exit(1);
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
