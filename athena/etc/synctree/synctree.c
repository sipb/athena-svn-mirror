/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#ifdef POSIX
#include <unistd.h>
#ifndef SOLARIS
#include <dirent.h>
#else
#include "dirent.h"
#endif
#else
#include <sys/dir.h>
#endif
#include <sys/stat.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <strings.h>
#ifdef OLD
#include <ndbm.h>
#endif
#ifdef sparc
#include <alloca.h>
#endif
#ifdef SOLARIS
#define NO_LINEBUF
#endif
#include "synctree.h"

#ifndef DEBUG
char *srcdir = "/server";
char *dstdir = "";            /* effectively "/" */
char *src_rule_file = ".rconf";
char *dst_rule_file = ".rconf.local";
char *date_db_name  = ".reconcile_dates";
#else DEBUG
char *srcdir = "s";
char *dstdir = "d";
char *src_rule_file = ".rconf";
char *dst_rule_file = ".rconf.local";
#ifdef DATE
char *date_db_name  = ".reconcile_dates";
#endif
#endif DEBUG

rule rules[MAXNRULES];
unsigned int lastrule;
unsigned int rflag = 0;

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

int dodir();
char *destination_pathname();

int  verbosef = 1;
int  nflag = 0;
int  aflag = 0;
char *afile;

bool nosrcrules = FALSE;
bool nodstrules = FALSE;
uid_t uid, euid;
#ifndef SOLARIS
uid_t getuid(), geteuid();
#endif
void usage(arg0)
     char *arg0;
{
  fprintf(stderr,"Usage: %s [-v] [-n] [-nosrcrules] [-nodstrules] [-s srcdir] [-d dstdir] [-a rules]\n",arg0);
  exit(1);
}

main(argc, argv)
     int argc;
     char *argv[];
{
  int i;
#ifndef POSIX
  char *getwd();
#endif
#ifndef NO_RLIMIT
  struct rlimit rl;
#endif

  printf("SyncTree version %s\n",version_string());

  uid = getuid();
  euid = geteuid();

#ifdef DEBUG
  if (uid != 11211 && uid != 14640) {
    printf("This SyncTree for debuging only.  Exiting.\n");
    exit(1);
  } else {
    printf("DEBUG SyncTree\n");
  }
#endif DEBUG

#ifndef NO_RLIMIT
  getrlimit(RLIMIT_DATA,&rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_DATA,&rl);

  getrlimit(RLIMIT_STACK,&rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_STACK,&rl);
#endif

#ifndef NO_LINEBUF
  setlinebuf(stdout);
#else
  setvbuf(stdout,NULL,_IOLBF,BUFSIZ);
#endif

  i = 0;
  
  while (++i < argc)
    {
      if (argv[i][0] != '-') usage(argv[0]);

      switch (argv[i][1]) {
      case 's':
	if (argv[i][2])
	  { srcdir = (char *) alloca(strlen(&argv[i][2])+1);
	    (void) strcpy(srcdir, &argv[i][2]);
	  }
	else if (++i >= argc) usage(argv[0]);
	else
	  { srcdir = (char *) alloca(strlen(&argv[i][0])+1);
	    (void) strcpy(srcdir, argv[i]);
	  }
	break;

      case 'd':
	if (argv[i][2])
	  { dstdir = (char *) alloca(strlen(&argv[i][2])+1);
	    (void) strcpy(dstdir, &argv[i][2]);
	  }
	else if (++i >= argc) usage(argv[0]);
	else
	  { dstdir = (char *) alloca(strlen(&argv[i][0])+1);
	    (void) strcpy(dstdir, argv[i]);
	  }
	if (strcmp(dstdir,"/") == 0)
	  dstdir[0] = '\0';
	break;

      case 'a':
	if (argv[i][2])
	  { afile = (char *) alloca(strlen(&argv[i][2])+1);
	    (void) strcpy(afile, &argv[i][2]);
	  }
	else if (++i >= argc) usage(argv[0]);
	else
	  { afile = (char *) alloca(strlen(&argv[i][0])+1);
	    (void) strcpy(afile, argv[i]);
	  }
	if (strcmp(afile,"/") == 0)
	  afile[0] = '\0';
	aflag++;
	break;

      case 'v':
        verbosef++;
	break;
      case 'q':
	verbosef = 0;
	break;
      case 'n':
	if (strcmp(&argv[i][1],"nosrcrules") == 0)
	  { nosrcrules = TRUE;
	    break;
	  }	    
	if (strcmp(&argv[i][1],"nodstrules") == 0)
	  { nodstrules = TRUE;
	    break;
	  }	    
	if (strcmp(&argv[i][1],"n") == 0)
	  { nflag = 1;
	    break;
	  }	    
	/* fall through to default */
      default:
	usage(argv[0]);
      }
    }
      
  (void) umask(0);

  if (srcdir[0] != '/')
    { char *p = srcdir;
      srcdir = (char *) alloca(MAXPATHLEN);
#ifdef POSIX 
      if ((srcdir = getcwd(srcdir,MAXPATHLEN )) == NULL)
	{ perror("getcwd() failed");
	  fprintf(stderr,"exiting\n");
	  exit(1);
	} 
#else 
      if ((srcdir = getwd(srcdir)) == NULL)
	{ perror("getwd() failed");
	  fprintf(stderr,"exiting\n");
	  exit(1);
	}
#endif 
      if ((strlen(srcdir) + strlen("/") + strlen(p) + 1) > MAXPATHLEN)
	{ fprintf(stderr,"full pathname is too long, exiting");
	  exit(1);
	}
      (void) strcat(srcdir,"/");
      (void) strcat(srcdir,p);
    }

  if ((dstdir[0] != '/') && (dstdir[0] != '\0'))
    { char *p = dstdir;
      dstdir = (char *) alloca(MAXPATHLEN);
#ifdef POSIX
      if ((dstdir = getcwd(dstdir, MAXPATHLEN)) == NULL)
	{ perror("getcwd() failed");
	  fprintf(stderr,"exiting\n");
	  exit(1);
	}
#else 
      if ((dstdir = getwd(dstdir)) == NULL)
	{ perror("getwd() failed");
	  fprintf(stderr,"exiting\n");
	  exit(1);
	}
#endif
      if ((strlen(dstdir) + strlen("/") + strlen(p) + 1) > MAXPATHLEN)
	{ fprintf(stderr,"full pathname is too long, exiting");
	  exit(1);
	}

      (void) strcat(dstdir,"/");
      (void) strcat(dstdir,p);
    }


  lastrule = 0;

  /* The hardwired rules (number zero and number one):
            map * $
            ignore *

	    */

  {
      static char *mapdests[] = { "$", (char *)0 };

      lstrule.type = R_MAP;
      lstrule.u.u_map.globexp    = "*";
      lstrule.u.u_map.file_types = TYPE_ALL;
      lstrule.u.u_map.dests      = mapdests;
  }
  newrule();
  lstrule.type = R_ACTION;
  lstrule.u.u_action.type        = ACTION_IGNORE;
  lstrule.u.u_action.globexp     = "*";
  lstrule.u.u_action.file_types  = TYPE_ALL;
  lstrule.u.u_action.options     = 0;

  exit(dodir(srcdir,dstdir,""));
}

struct src {
#ifdef POSIX
  struct dirent *dir;
#else
  struct direct *dir;
#endif
  char *pathname;
  struct stat stat;
  unsigned int type;
  int map_ruleno;
  bool chased;
  struct src *next;
};

struct targ {
  char *pathname;
  struct src *sp;
  int action_ruleno;
  bool updated; /* for when */
  struct targ *next;
};


int dodir(src,dst,part)
     char *src;
     char *dst;
     char *part;
{
    struct src srclist,*sp,*spp,*sppp;
    struct targ targlist,*tp,*tpp,*tppp;
    int i,j;
    unsigned long getmemt;
    bool sorted;
    DIR *dirp;
#ifdef POSIX
    struct dirent *dp;
#else
    struct direct *dp;
#endif
    char *testpath;
    struct stat teststat;

    if (verbosef)
	printf("processing %s\n", dst);
  
    srclist.next = 0;
    targlist.next = 0;

#define getmem(amt) (((getmemt = alloca(amt)) < 1)? panic("dodir: bad return from alloca") : getmemt)
#define newsrc(ptr)  (ptr = (struct src *)  getmem(sizeof(struct src)),  ptr->next=srclist.next,  srclist.next=ptr)
#define newtarg(ptr) (ptr = (struct targ *) getmem(sizeof(struct targ)), ptr->next=targlist.next, targlist.next=ptr)
#define allsrcs(ptr)  for (ptr=srclist.next; ptr!=NULL; ptr=ptr->next)
#define alltargs(ptr) for (ptr=targlist.next; ptr!=NULL; ptr=ptr->next)

    getrules(src,dst);
  
    /* read in source directory */
    if ((dirp = opendir(src)) == NULL) {
	printf("%s is not a directory.\n",src);
	return 1;
    }
    while ((dp = readdir(dirp)) != NULL) {
	if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
	    continue;
	newsrc(sp);
	sp->pathname = (char *)0;
	/**** the next two lines are space inefficient, it can be done better */
#ifdef POSIX
	sp->dir    = (struct dirent *) getmem(sizeof(struct dirent));
#else
	sp->dir    = (struct direct *) getmem(sizeof(struct direct));
#endif
	*(sp->dir) = *dp;
        strcpy(sp->dir->d_name, dp->d_name);
    }
    closedir(dirp);

    /* XXX */
    /* read in target directory */
    if ((rflag & RFLAG_SCAN_TARGET) && (dirp = opendir(dst))) {
	testpath = (char *) getmem(strlen(src) + 66);
	while ((dp = readdir(dirp)) != NULL) {
	    if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
		continue;
	    strcpy(testpath,src);
	    strcat(testpath,"/");
	    strcat(testpath,dp->d_name);
	    if (lstat(testpath, &teststat)) {
		newsrc(sp);
		/* the next two lines are space inefficient */
#ifdef POSIX
		sp->dir    = (struct dirent *) getmem(sizeof(struct dirent));
#else
		sp->dir    = (struct direct *) getmem(sizeof(struct direct));
#endif
		*(sp->dir) = *dp;

		sp->pathname =
		    (char *) getmem( strlen(src) + strlen(sp->dir->d_name) + 2 );
		strcpy(sp->pathname, testpath);
	    }
	}
	closedir(dirp);
    }
    /* XXX */

    /* process each src */
    allsrcs(sp) {

	/*
	 * If sp->pathname is already set, it must be a virtual file,
	 * which indicates that the file only exists in the target
	 * directory.  We still wish to get the stats for such files,
	 * but we want to mark it as a virtual file, so we frob with
	 * the sp->type field.
	 */
	if (!sp->pathname) {
	    sp->pathname =
		(char *) getmem( strlen(src) + strlen(sp->dir->d_name) + 2 );
	    (void) strcpy(sp->pathname,src);
	    (void) strcat(sp->pathname,"/");
	    (void) strcat(sp->pathname,sp->dir->d_name);
	    /* get information about file */
	    if (lstat(sp->pathname,&(sp->stat)) < 0) {
#define perror(problem, whatnext) printf("%s: %s: %s. %s\n", sp->pathname, problem, errno<sys_nerr ? sys_errlist[errno] : "unknown error", whatnext)
		perror("lstat() failed", "ignoring");
		sp->map_ruleno = 0;
		continue;
	    }

	    sp->type = 0;

	} else {
	    char path[MAXPATHLEN];

	    (void) strcpy(path, dst);
	    (void) strcat(path, "/");
	    (void) strcat(path, sp->dir->d_name);

	    if (lstat(path, &(sp->stat)) < 0) {
#define perror(problem, whatnext) printf("%s: %s: %s. %s\n", sp->pathname, problem, errno<sys_nerr ? sys_errlist[errno] : "unknown error", whatnext)
		perror("lstat() failed", "ignoring");
		sp->map_ruleno = 0;
		continue;
	    }
	      
	    sp->type = TYPE_V;
	}
    
	{
	    int mode = sp->stat.st_mode & S_IFMT;
	    switch(mode) {
#define X(a,b) case a: \
		if (verbosef > 1) printf("Found %s, mode %7.7o\n",sp->pathname,mode); \
		    sp->type |= b; break 
			X(S_IFDIR,TYPE_D);
		X(S_IFCHR,TYPE_C);
		X(S_IFBLK,TYPE_B);
		X(S_IFREG,TYPE_R);
		X(S_IFLNK,TYPE_L);
		X(S_IFSOCK,TYPE_S);
#undef X
	    default:
		printf("%s: unknown type of src file: %7.7o, ignoring.\n",
		       sp->pathname, mode);
		sp->map_ruleno = 0;
		continue;
	    }
	}
    
	if ((sp->type == TYPE_L) &&
	    (findrule(sp->pathname,R_CHASE,sp->type,sp->pathname) > 0)) {
	    sp->chased = TRUE;
	    if (stat(sp->pathname,&(sp->stat)) < 0) {
		perror("stat() after chase failed", "ignoring.");
#undef perror
		sp->map_ruleno = 0;
		continue;
	    }
	    {
		int mode = sp->stat.st_mode & S_IFMT;
		switch (mode) {
#define X(a,b) case a: sp->type = b ; break 
		    X(S_IFDIR,TYPE_D);
		    X(S_IFCHR,TYPE_C);
		    X(S_IFBLK,TYPE_B);
		    X(S_IFREG,TYPE_R);
		    X(S_IFLNK,TYPE_L);
		    X(S_IFSOCK,TYPE_S);
#undef X
		default:
		    printf("%s: unknown type of chased file: %7.7o, ignoring.\n",
			   sp->pathname, mode);
		    sp->map_ruleno = 0;
		    continue;
		}
	    }
	} else {
	    sp->chased = FALSE;
	}
  
	sp->map_ruleno = findrule(sp->pathname,R_MAP,sp->type,sp->pathname);

	for (i = 0; rules[sp->map_ruleno].u.u_map.dests[i] != 0; i++) {
	    char *r;
	    newtarg(tp);
	    r = destination_pathname(dst,
				     sp->dir->d_name, 
				     rules[sp->map_ruleno].u.u_map.dests[i]);
	    tp->pathname = (char *) getmem(strlen(r)+1);
	    strcpy(tp->pathname,r);
	    tp->sp = sp;
	    if (verbosef > 2) printf("new targ: %s (from %s)\n",
				     tp->pathname, sp->pathname);

	}
    }
    alltargs(tp) {
	tp->action_ruleno =
	    findrule(tp->pathname,R_ACTION,tp->sp->type,tp->sp->pathname);
	tp->updated = FALSE;
    }

    alltargs(tp) {
	bool exists = FALSE;
	bool chased = FALSE;
	struct stat targstat;
	unsigned int targtype;
	time_t dbdate;
	if (lstat(tp->pathname,&targstat) == 0) {
	    int mode = targstat.st_mode & S_IFMT;
	
	    exists = TRUE;	
	    switch (mode) {
#define X(a,b) case a: targtype = b ; break 
		X(S_IFDIR,TYPE_D);
		X(S_IFCHR,TYPE_C);
		X(S_IFBLK,TYPE_B);
		X(S_IFREG,TYPE_R);
		X(S_IFLNK,TYPE_L);
		X(S_IFSOCK,TYPE_S);
#undef X
	    }
	    if ((targtype == TYPE_L) &&
		option_on(tp->action_ruleno, 'l')) {
		if (stat(tp->pathname,&targstat) != 0) {
		    /*** the link exists but its target does not */
		    /*** hopefully, we can make it exist ..... */
		    chased = TRUE;
		    exists = FALSE;
		} else {
		    chased = TRUE;
		    switch (targstat.st_mode & S_IFMT) {
#define X(a,b) case a: targtype = b ; break 
			X(S_IFDIR,TYPE_D);
			X(S_IFCHR,TYPE_C);
			X(S_IFBLK,TYPE_B);
			X(S_IFREG,TYPE_R);
			X(S_IFLNK,TYPE_L);
			X(S_IFSOCK,TYPE_S);
#undef X
		    }
		}
	    }
	}

#define srcstat (tp->sp->stat)
	/*      targstat */

	/* "switch.c" knows only about what is defined or mentioned between here and where it is included */

#define typeofaction (rules[tp->action_ruleno].u.u_action.type)
#define pflag   (option_on(tp->action_ruleno,'p'))
#define fflag	(option_on(tp->action_ruleno,'f'))

	/* modemask is not known to switch.c */
#define modemask (pflag ? 07777 : 00777)

#define srcname (tp->sp->pathname)
#define srcdev  (int) (srcstat.st_rdev)
#define srctype (tp->sp->type)
#define srcuid  ((int) (srcstat.st_uid))

#define targname (tp->pathname)
#define targdev  (int) (targstat.st_rdev)
#define targuid  ((int) (targstat.st_uid))
#define targmode (int) (targstat.st_mode & modemask)
#define tmark()  (tp->updated = TRUE)

#define srcgid  ((gid_t) (srcstat.st_gid))
#define targgid  ((gid_t) (targstat.st_gid))

#if defined(_AIX) && defined(_IBMR2)
#define srcmode (int)(srcstat.st_mode & ((srctype==TYPE_D) ? 07777 : modemask))
#else
#define srcmode (int)(srcstat.st_mode & modemask)
#endif

#define ERROR -1
#define NODBDATE 0
#define NODBDATE_P LOCAL		/* (forceupdatef? OUTOFDATE : LOCAL) */
#define UPTODATE 1
#define OUTOFDATE 2
#define NEWERDATE 3
#define LOCAL 4
#define LOCAL_P LOCAL			/* (forceupdatef? OUTOFDATE : LOCAL) */
#define LOCALBUTOUTOFDATE 6
#define LOCALBUTOUTOFDATE_P LOCAL	/* (forceupdatef? OUTOFDATE : LOCAL) */
#define FIXMODE 6
#define FIXOWNANDMODE 7

#define filecheck3() \
	((srcstat.st_mtime > targstat.st_mtime) ? \
	 OUTOFDATE : \
	 ((srcstat.st_mtime < targstat.st_mtime) || \
	  (srcstat.st_size != targstat.st_size)) ? \
	 NEWERDATE : \
	 (pflag && ((srcuid != targuid) || (srcgid != targgid))) ? \
	 FIXOWNANDMODE : \
	 ((srcmode&modemask) != (targmode&modemask)) ? \
	 FIXMODE : \
	 UPTODATE)
	
#define dircheck() \
	((pflag && ((srcuid != targuid) || \
		    ((srcgid >= 0) && (srcgid != targgid)))) \
	 ? FIXOWNANDMODE : \
	 (pflag && ((srcmode&modemask) != (targmode&modemask))) \
	 ? FIXMODE : UPTODATE)

#define setdates() \
	{ time_t date = srcstat.st_mtime; \
	  struct timeval tv[2]; \
	  tv[0].tv_sec = tv[1].tv_sec = date; \
	  tv[0].tv_usec = tv[1].tv_usec = 0; \
	  (void) utimes(targname,tv); \
	  tmark(); \
	}

    if (verbosef > 2) {
      printf("entering switch.c:  %s ----> %s\n", srcname, targname);
      printf("              typeofaction   is %d\n", typeofaction);
      printf("              pflag	       is %d\n", pflag);
      printf("              modemask       is %4.4o\n", modemask);
      printf("              srcname	       is %s\n", srcname);
      printf("              srcmode	       is %7.7o\n", srcstat.st_mode);
      printf("              srcdev	       is %4.4x\n", srcdev);
      printf("              srctype	       is %d\n", srctype);
      printf("              srcuid	       is %d\n", srcuid);
      printf("              srcgid	       is %d\n", srcgid);
      printf("              targname       is %s\n", targname);
      printf("              exists	       is %d\n", exists);
      if (exists) {
	printf("              targmode	 is %7.7o\n", targstat.st_mode);
	printf("              targdev	 is %4.4x\n", targdev);
	printf("              targtype	 is %d\n", targtype);
	printf("              targuid	 is %d\n", targuid);
	printf("              targgid	 is %d\n", targgid);
      }
      printf("\n");
    }	  

#include "switch.c"
  }
  
  /* do when rules */
  alltargs(tp) {
    if (tp->updated) {
      void dowhenrule();
      int whenruleno;
      whenruleno = findrule(tp->pathname, R_WHEN, tp->sp->type, tp->sp->pathname);
      if (whenruleno != -1)
	dowhenrule(whenruleno, tp->pathname);
    }
  }

    {
	char * ptr = (char *)NULL;
	
	allsrcs(sp) {
	    freea(ptr);
	    ptr = (char *) sp;
	    freea(sp->pathname);
	    freea(sp->dir);
	}
	alltargs(tp) {
	    freea(ptr);
	    ptr = (char *) tp;
	    freea(tp->pathname);
	}
	freea(ptr);
    }
    
  poprules();
  return 0;
}

char * destination_pathname(dstdir, sname, mapstring)
     char *dstdir, *sname, *mapstring;
{
  static char buf[MAXPATHLEN];
  register char *p,*bp,*sp;
  register char *lastdot;
  bp = buf;

  if (verbosef>2) printf("destination_pathname(%s,%s,%s)\n",
			 dstdir, sname, mapstring);

  for (p = dstdir; *p != '\0'; p++) *bp++ = *p;

  *bp++ = '/';

  for (p = mapstring; *p != '\0'; p++) {
    switch (*p) {
    case '$':
      lastdot = NULL;
      for (sp = sname; *sp != '\0'; sp++)
	if ((*bp++ = *sp) == '.')
	  lastdot = bp - 1;
      p++;
      switch(*p) {
      default:
	p--; continue;
      case ':':
	p++;
	switch(*p) {
	case '\0':
	  /***** maybe this should be an error */
	  p--; p--; continue;
	case 'r':
	  if (lastdot)
	    bp = lastdot;
	  else
	    { ; /***** should be an error of some sort */ }
	  continue;
	default:
	  /***** should be an error of some sort */
	  continue;
	}
      }
      /*NOTREACHED*/
    default:
      *bp++ = *p;
      continue;
    }
  }

  *bp = '\0';
  
  if (verbosef>1) printf("destination_pathname returning %s\n",buf);

  return buf;
}


void dowhenrule(ruleno, tname)
     int ruleno;
     char *tname;
{
    char *cp;
    int pid;
    char *shell, *arg1;
    int pipefds[2];

    if (rules[ruleno].type != R_WHEN)
	panic("dowhenrule:  rule type is not R_WHEN");

    switch(rules[ruleno].u.u_when.type) {
    case WHEN_SH:
	shell = "/bin/sh";
	arg1  = "-s";
	break;
    case WHEN_CSH:
	shell = "/bin/csh";
	arg1  = "-fs";
	break;
    }
    
    if (pipe(pipefds) != 0)
	epanic("dowhenrule:  pipe");

    if (nflag == 0) {
	pid = fork();
    
	if (pid == 0) {

	    cp = tname;
	    while (*cp != '\0') cp++;
	    while (*cp != '/') cp--;
	    *cp = '\0';
	    if (cp == tname)
		chdir("/");
	    else
		chdir(tname);
	    *cp = '/';

	    dup2(pipefds[0], 0);
	    execl(shell, shell, arg1, 0);
	    epanic("dowhenrule:  execl");
	} else if (pid > 0) {
	    char **c;
	    FILE *f;

	    f = fdopen(pipefds[1],"w");
	    c = rules[ruleno].u.u_when.cmds;

	    while (*c) {
		if (verbosef) printf("%c %s",
				     (rules[ruleno].u.u_when.type == WHEN_SH)
				     ? '$' : '%', *c);
		fprintf(f,"%s\n", *(c++));
	    }
	    /* kludge killing the sh because I cannot figure out how to
	     * make it go away on end of file */
	    switch(rules[ruleno].u.u_when.type) {
	    case WHEN_SH:
		fprintf(f,"kill $$\n");
		break;
	    default:
		break;
	    }
	    fclose(f);
	} else if (pid < 0) {
	    epanic("dowhenrule: fork");
	}
    } else				/* If noop */
	{
	    char **c;
	    c = rules[ruleno].u.u_when.cmds;

	    while (*c) {
		if (verbosef) printf("%c %s",
				     (rules[ruleno].u.u_when.type == WHEN_SH)? '$': '%',
				     *c);
	    }	
	}
}

