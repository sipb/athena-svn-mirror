/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v 1.5 1987-09-08 15:55:42 shanzer Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.4  87/08/28  15:11:40  shanzer
 * Catches SIGINT, and removes lockfiles... 
 * 
 * Revision 1.3  87/08/28  13:47:30  shanzer
 * Put temp file in /tmp where the belong.. 
 * 
 * Revision 1.2  87/03/05  19:20:08  rfrench
 * Fixed lossage with extra /'s in filenames.
 * 
 * Revision 1.1  87/02/12  21:15:48  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v 1.5 1987-09-08 15:55:42 shanzer Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

char admin[WORDLEN] = DEF_ADM;		/* track administrator */
char twdir[LINELEN] = DEF_TOWDIR;	/* working directory for dest root
					 * where slists/statfiles etc. can be
					 * found */
char fwdir[LINELEN] = DEF_FROMWDIR;	/* working directory for source root */
char binarydir[LINELEN] = DEF_BINDIR;	/* directory in working dir to
					 * find executables */
char g_except[LINELEN] = DEF_EXCEPT;	/* default exceptions */
char subname[LINELEN] = DEF_SUB;	/* default subscription file */
char fromroot[LINELEN] = DEF_FROMROOT;	/* Root directory for source */
char toroot[LINELEN] = DEF_TOROOT;	/* Root directory for desination */
char prgname[LINELEN];

extern int errno;		/* global error number location */
char errmsg[LINELEN];

int writeflag = 0;	/* if set, output a subscription list rather than
				   pulling files */
int parseflag = 0;	/* if set, just parse the subscription list */
int forceflag = 0;	/* if set, will over-ride lock files */
int verboseflag = 0;	/* if set, files listed on stdout while being updated */
int dirflag = 0;	/* if set, create directories if necessary */
int nopullflag = 0;	/* if set, find out the differences,
				   but don't pull anything */
int quietflag = 0;	/* if set, don't print non-fatal error messages */
int interactive = 1;	/* if set, don't send errors via mail, print them */
int uflag = NO_CLOBBER;	/* if set, copy a older file on top of a newer one */
int debug = 0;		/* if set, print debugging information */
int incl_devs = 0;	/* if set, include devices in update */

char startname[LINELEN];	/* starting lock filename */
FILE *subfile;			/* pointer to the subscription list */
FILE *stampfile;		/* stamp command pipe */
FILE *statfile;			/* the statfile! */

Entry entries[ENTRYMAX];	/* Subscription list entries */
int entrycnt = 0;		/* Number of entries */
int cur_ent = 0;		/* Current entry number */

main(argc,argv)
int argc;
char **argv;
{
	char subtmp[LINELEN],scratch[LINELEN];
	int	cleanup();
	int i;

	strcpy(prgname,argv[0]);

	umask(022);	/* set default umask for daemons */
	signal(SIGINT, cleanup);

	strcpy(errmsg,"");

	for(i=1;i<argc;i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
/* -F dirname
 *    Specify source "root" directory.
 */
       				case 'F':
					get_arg(fromroot,argv,&i);
					break;
/* -T dirname
 *    Specify destination "root" directory.
 */ 
				case 'T':
					get_arg(toroot,argv,&i);
					break;
/* -m {user}
 *    Send mail to root/user instead of displaying messages on the
 * terminal.
 */
				case 'm':
					interactive = 0;
					get_arg(admin,argv,&i);
					break;
/* -d dirname
 *    Specify the working directory for the desitination root system.
 */
				case 'd':
					get_arg(twdir,argv,&i);
					break;
/* -r dirname
 *    Specify the working directory for the source root system.
 * If -r is not specified, it will default to the -d directory.
 */
				case 'r':
					get_arg(fwdir,argv,&i);
					break;
/* -w
 *    Create a statsfile.
 */
				case 'w':
					writeflag = 1;
					break;
/* -p
 *    Parse only.  Display a detailed list of the fields in the
 * subscription file.
 */
				case 'p':
					parseflag = 1;
					break;
/* -f
 *    Force updating regardless of locks.
 */
				case 'f':
					forceflag = 1;
					break;
/* -D
 *    Create directories as necessary during updating.
 */
				case 'D':
					dirflag = 1;
/* -v
 *    Explain what is going on verbosely.
 */
				case 'v':
					verboseflag = 1;
					break;
/* -q
 *    Be quiet about warning messages.
 */
				case 'q':
					quietflag = 1;
					break;
/* -u
 *    Copy over files regardless of which is newer.
 */
				case 'u':
					uflag = DO_CLOBBER;
					break;
/* -n
 *    Produce a list of files that need updating, but don't actually
 * do anything about them.
 */
				case 'n':
					nopullflag = 1;
					break;
/* -i
 *    Include devices in an update.
 */
				case 'i':
					incl_devs = 1;
					break;
/* -x
 *    Display debugging information.
 */
				case 'x':
					debug = 1;
					break;
/*
 * Something isn't right if we got this far...
 */
				default:
					fprintf(stderr,
					"track error: unknown option %s\n",
						argv[i]);
					break;
			}
		}
		else
			strcpy(subname,argv[i]);
	}

/*
 * By default, use the working directory in both of the root systems.
 */
	if (!*fwdir)
		strcpy(fwdir,twdir);

/*
 * Set up nullmail interface if not an interactive session.
 */

	if (!interactive)
		setuperr();

/*
 * Get the proper working directories.
 */

	sprintf(scratch,"%s%s",toroot,twdir);
	strcpy(twdir,scratch);

	sprintf(scratch,"%s%s",fromroot,fwdir);
	strcpy(fwdir,scratch);

	sprintf(subtmp,"%s%s/%s",twdir,DEF_SUBDIR,subname);
	if (!(subfile = fopen(subtmp,"r"))) {
		sprintf(errmsg,"Can't open %s\n",subtmp);
		do_panic();
	}

	/*
	**	redirect yacc/lex i/o
	*/
	parseinit();
	if (yyparse()) {
		strcpy(errmsg,"parser error");
		do_panic();
	}

	if (debug)
		printf("parse worked\n");

	if (parseflag) {  /* -p: Just show the fields */
		justshow();
		cleanup();
	}

	if (writeflag) {  /* -w: Write the initial statfile */
		writestat();
		cleanup();
	}

	readnames(nopullflag);  /* Go do the dirty work */

	cleanup();
}

cleanup()
{
	clearlocks();
	exit(0);
}

/*
 * See if any files are exempt from the exception lists.
 */

anygood(ptr)
char *ptr;
{
	for(cur_ent = 0;cur_ent<entrycnt;cur_ent++)
		if(goodname(ptr,entries[cur_ent].fromfile,cur_ent))
			return(1);
	return(0);
}

/*
 * Show parsing.
 */

justshow()
{
	int i,j;

	for (i = 0; i < entrycnt;i++) {
		fprintf(stderr,"entry %d:\n\tfollow -- %d\n\tfromfile--|%s|\n\tcmpfile--|%s|\n\ttofile--|%s|\n\texceptions--\n",
			i,
			entries[i].followlink,
			entries[i].fromfile,
			entries[i].cmpfile,
			entries[i].tofile);
			for(j=0;entries[i].exceptions[j] != (char*)0;j++)
				fprintf(stderr,"\t\t|%s|\n",entries[i].exceptions[j]);
			fprintf(stderr,"\tcommand--|%s|\n",entries[i].cmdbuf);
	}

}

/*
 *	redirect standard error output to mail to the administrator
 *	use nullmail so that null messages won't get sent.
 */

/* REWORK */

setuperr()
{
	char msg[LINELEN];
	FILE *tmp;

	sprintf(msg,"%s/nullmail %s",binarydir,admin);
	/*
	**	start a process that will send mail to the adminstrator
	*/
	if ((tmp = popen(msg,"w")) == NULL) {
/*		sprintf(msg,"echo HELP automatic software distribution --%s can't execute nullmail cmd  > /dev/console",
			gargv[0]);
		system(msg); */
		exit(1);
	}
	/*
	**	now connect stderr to the pipe
	*/
	if (dup2(fileno(tmp),2) ==  -1)
	{
/*		sprintf(msg,"echo HELP automatic software distribution --%s can't dup stderr > /dev/console",
			gargv[0]);
		system(msg); */
		exit(1);
	}
}

/*
 * Set lock for subscriptionlist file.
 */

setlock()
{
	sprintf(startname,"%s/%s.started",DEF_LOCKDIR,subname);

	if (exists(startname)) {
		if (too_old(startname,LOCK_TIME) || forceflag) {
			if (verboseflag)
				fprintf(stderr,"clearing lock %s\n",startname);
			if (unlink(startname) == -1) {
				sprintf(errmsg,"can't remove old lockfile %s",
						startname);
				do_panic();
			}
		}
		else {
			sprintf(errmsg,"lock set on %s--quitting\n",startname);
			do_gripe();
			exit(0);
		}
	}
	if (close(creat(startname,0)) == -1) {
		sprintf(errmsg,
			"can't create lockfile %s\n",
			startname);
		do_panic();
	}
}

/*
 * Erase those locks...
 */

clearlocks()
{
	unlink(startname);
}

/*
 * Act like a librarian and write out the statfile.
 */

writestat()
{
	char stampcmd[LINELEN],lbuf[LINELEN],outname[LINELEN];
	char tmpoutname[LINELEN],tmpcmp[LINELEN];
	int tmpstat;

	setlock();

	sprintf(outname,"%s%s/%s",twdir,DEF_STATDIR,subname);
	sprintf(tmpoutname,"%s/%s.tmp",DEF_LOCKDIR,subname);

	if (exists(tmpoutname) && (unlink(tmpoutname) == -1)) {
		sprintf(errmsg,"can't remove %s\n",tmpoutname);
		do_panic();
	}

	sprintf(stampcmd,"sort +0.2 >> %s",tmpoutname);

	for(cur_ent=0;cur_ent<entrycnt;cur_ent++) {
		if (!(stampfile = popen(stampcmd,"w")))	{
			sprintf(errmsg,"can't execute %s\n",stampcmd);
			do_panic();
		}

		if (debug)
			printf("%d: %s\n",cur_ent,entries[cur_ent].cmpfile);

		sprintf(tmpcmp,"%s%s",fromroot,entries[cur_ent].cmpfile);
		switch(gettype(tmpcmp)) {
			case 'd' :
				dofind('d');
				dofind('l');
				dofind('f');
				dofind('b');
				dofind('c');
				break;
			case 'l':
			case 'f':
				/*
				** no need to check for exceptions
				** since this entry is either a file
				** or a symbolic link and thus it won't
				** result it is the root of a
				** trivial subtree
				*/
				stamp(tmpcmp,entries[cur_ent].fromfile,
				      lbuf);
				fprintf(stampfile,"%s\n",lbuf);
				if (verboseflag)
					fprintf(stderr,"%s\n",lbuf);
				break;
			default:
				fprintf(stampfile,"*%s\n",
					entries[cur_ent].fromfile);
				break;
					
		}

		if (tmpstat = pclose(stampfile)) {
			sprintf(errmsg,
				"bad exit status %d from stamp/sort cmd",
				tmpstat);
			do_panic();
		}
	}

	if (exists(tmpoutname)) {
		if (copy_file(tmpoutname,outname) == 1 || unlink(tmpoutname) == -1) {
			sprintf(errmsg,
				"can't change file from %s to %s\n",
				tmpoutname,
				outname);
			do_panic();
		}
	}
}

/*
 * Find files, potentially all files in a given directory.
 */

dofind(thechar)
char thechar;
{
	char rootname[LINELEN],tmpbuf[LINELEN];

	if (debug)
		printf("dofind(%c):\n",thechar);

	if (thechar == 'd') {
		sprintf(rootname,"%s%s",fromroot,entries[cur_ent].cmpfile);
		stamp(rootname,entries[cur_ent].cmpfile,tmpbuf);
		mapname(tmpbuf,
			entries[cur_ent].cmpfile,
			entries[cur_ent].fromfile);
		fprintf(stampfile,"%s\n",tmpbuf);
		if (verboseflag)
			fprintf(stderr,"%s\n",tmpbuf);
	}

	walk_tree(entries[cur_ent].cmpfile,thechar);
}

walk_tree(myroot,thechar)
char *myroot,thechar;
{
	char rootname[LINELEN],tmpbuf[LINELEN],subdname[LINELEN];
	DIR *dirp;
	struct direct *dp;
	struct stat sbuf;
	int gotone;

	if (debug)
		printf("walk_tree(%s,%c)\n",myroot,thechar);

	sprintf(rootname,"%s%s",fromroot,myroot);
	dirp = opendir(rootname);
	if (!dirp) {
		sprintf(errmsg,"can't open directory %s\n",rootname);
		do_gripe();
		return;
	}

	while(dp = readdir(dirp)) {
		if (!dp->d_ino
		    || !strcmp(dp->d_name,".")
		    || !strcmp(dp->d_name,".."))
			continue;
		sprintf(rootname,"%s%s/%s",fromroot,myroot,dp->d_name);
		sprintf(subdname,"%s/%s",myroot,dp->d_name);
		if (debug)
			printf("   %s\n",rootname);
		if (lstat(rootname,&sbuf)) {
			sprintf(errmsg,"can't stat %s\n",rootname);
			do_gripe();
			continue;
		}
		gotone = 0;
		switch (sbuf.st_mode & S_IFMT) {
		case S_IFLNK:
			if (thechar == 'l')
				gotone = 1;
			break;
		case S_IFDIR:
			if (thechar == 'd')
				gotone = 1;
			break;
		case S_IFREG:
			if (thechar == 'f')
				gotone = 1;
			break;
		case S_IFCHR:
			if (thechar == 'c')
				gotone = 1;
			break;
		case S_IFBLK:
			if (thechar == 'b')
				gotone = 1;
			break;
		}
		if (gotone && goodname(subdname,entries[cur_ent].fromfile,
				       cur_ent)) {
			stamp(rootname,subdname,tmpbuf);
			mapname(tmpbuf,
				entries[cur_ent].cmpfile,
				entries[cur_ent].fromfile);
			fprintf(stampfile,"%s\n",tmpbuf);
			if (verboseflag)
				fprintf(stderr,"%s\n",tmpbuf);
		}
		if (((sbuf.st_mode & S_IFMT) == S_IFDIR)) {
			walk_tree(subdname,thechar);
			continue;
		}
	}
	closedir(dirp);
}

/*
 * Match the beginning of line with head - return 1 if different
 */

prematch(line,head)
char *line,*head;
{
	if (*head && strncmp(line,head,strlen(head))) {
		switch(*(line+strlen(head))) {
			case '\0':
			case '\t':
			case '\n':
			case '\r':
			case ' ' :
			case '/' :
				return(1);
			default:
				break;
		}
	}
	return(0);
}

/*
 * Subtract "old" from the beginning of line and add "new" to the beginning
 */

presub(line,old,new)
char *line,*old,*new;
{
	char buf[LINELEN];

	strcpy(buf,new);
	strcat(buf,line+strlen(old));
	strcpy(line,buf);
}

firstname(p,q)
char *p,*q;
{
	p++;
	mycpy(q,p);
}

add_cmp(ptr)
char *ptr;
{
	char first[LINELEN];

	firstname(ptr,first);
	presub(first,entries[cur_ent].fromfile,entries[cur_ent].cmpfile);
	skipword(&ptr);
	skipspace(&ptr);
	skipword(&ptr);
	strcpy(ptr," ");
	strcat(ptr,first);
	return(1);
}

/*
 * This is the REAL routine.  It might actually do updating.
 */

readnames(nopull)
int nopull;
{
	char rlinebfr[LINELEN],ourbuf[LINELEN],ourname[LINELEN];
	char localname[LINELEN],tempbuf1[LINELEN],tempbuf2[LINELEN];

	setlock();

	open_stat();

	for (cur_ent=0;cur_ent<entrycnt;cur_ent++) {
		rewind(statfile);
		while (fgets(rlinebfr,LINELEN,statfile)) {
			if (*rlinebfr && rlinebfr[strlen(rlinebfr)-1] == '\n')
				rlinebfr[strlen(rlinebfr)-1] = '\0';
			get_name(ourname,rlinebfr);
			if(!goodname(ourname,entries[cur_ent].fromfile,
				     cur_ent))
				continue;
			get_name(localname,rlinebfr);
			if (strcmp(entries[cur_ent].fromfile,
				   entries[cur_ent].cmpfile))
				mapname(localname,localname,
					entries[cur_ent].cmpfile);
			sprintf(ourname,"%s%s",toroot,localname);
			stamp(ourname,localname,ourbuf);
			if (!update(rlinebfr,ourbuf))
				continue;
			if (strcmp(entries[cur_ent].cmpfile,
				   entries[cur_ent].tofile))
				mapname(ourbuf,entries[cur_ent].cmpfile,
					entries[cur_ent].tofile);
			if (nopull) {
				mycpy(tempbuf1,rlinebfr+1);
				mycpy(tempbuf2,ourbuf+1);
				printf("%s%s -> %s%s\n",fromroot,tempbuf1,
				       toroot,tempbuf2);
				continue;
			}
			update_file(rlinebfr,ourbuf,
				    strcmp(entries[cur_ent].fromfile,
					   entries[cur_ent].tofile));
			do_cmds(entries[cur_ent].cmdbuf,ourbuf);
		}
	}
	close_stat();
}

/*
 * Execute shell commands
 */

do_cmds(cmds,local)
char *cmds,*local;
{
	char *ptr,*nptr;
	char args[LINELEN];
	FILE *shell;

	local++;
	mycpy(args,local);

	ptr = cmds;

	shell = popen(DEF_SHELL,"w");
	if (!shell) {
		sprintf(errmsg,"can't open shell %s\n",DEF_SHELL);
		do_gripe();
		return;
	}

	fprintf(shell,"chdir %s\n",toroot);
	fprintf(shell,"%sFILE=%s%s\n",DEF_SETCMD,toroot,args);
	fprintf(shell,"%sROOT=%s\n",DEF_SETCMD,toroot);

	for (;;) {
		nptr = index(ptr,'\n');
		if (nptr)
			*nptr = '\0';
		fprintf(shell,"%s\n",ptr);
		if (!nptr) {
			pclose(shell);
			return;
		}
		ptr = nptr+1;
	}
}

/*
 * Get a command line argument
 */

get_arg(to,list,ptr)
char *to,**list;
int *ptr;
{
	int offset = 2;

	if (strlen(list[*ptr]) == 2) {
		(*ptr)++;
		offset = 0;
	}
	strcpy(to,list[*ptr]+offset);
}

/*
 * Log a message to the logfile.
 */

log(ptr)
char *ptr;
{
	extern long time();
	extern char *ctime();
	static FILE *logfile = NULL;
	char namebuf[LINELEN];
	char timestring[LINELEN];
	long timebuf;

	if (!logfile) {
		sprintf(namebuf,"%s/%s",fwdir,DEF_LOG);
		if(!exists(namebuf))
			return;
		if ((logfile = fopen(namebuf,"a")) == NULL) {
			sprintf(errmsg,"can't open log file %s",namebuf);
			do_panic();
		}
	}
	timebuf = time(0);
	strcpy(timestring,ctime(&timebuf));
	*(timestring+strlen(timestring)-1) = '\0';	/* wipe out newline */
	fprintf(logfile,"%s %s %s\n",timestring,subname,ptr);
}

open_stat()
{
	char statname[LINELEN];

	sprintf(statname,"%s%s/%s",fwdir,DEF_STATDIR,subname);

	if (!(statfile = fopen(statname,"r"))) {
		sprintf(errmsg,"can't open statfile %s\n",statname);
		do_panic();
	}
}

close_stat()
{
	fclose(statfile);
}

update(stampone,stamptwo)
char *stampone,*stamptwo;
{
	struct stamp rstamp,lstamp;

	if (debug)
		printf("update(%s,%s)\n",stampone,stamptwo);

	if (!strcmp(stampone,stamptwo))
		return (0);

	dec_stamp(stampone,&rstamp);
	dec_stamp(stamptwo,&lstamp);

	if (rstamp.type != lstamp.type)
		return (1);

	switch (rstamp.type) {
	case '*':
		return (0);
	case 'd':
		return (rstamp.uid != lstamp.uid ||
			rstamp.gid != lstamp.gid ||
			rstamp.mode != lstamp.mode);
	case 'l':
		return (strcmp(rstamp.link,lstamp.link));
	case 'f':
		if (uflag == DO_CLOBBER)
			return (1);
		return (rstamp.uid != lstamp.uid ||
			rstamp.gid != lstamp.gid ||
			rstamp.mode != lstamp.mode ||
			rstamp.ftime > lstamp.ftime);
	case 'c':
	case 'b':
		if (!incl_devs)
			return (0);
		return (rstamp.dev != lstamp.dev ||
			rstamp.uid != lstamp.uid ||
			rstamp.gid != lstamp.gid ||
			rstamp.mode != lstamp.mode ||
			rstamp.type != lstamp.type);
	default:
		sprintf(errmsg,"bad string passed to update\n");
		do_panic();
	}
/*NOTREACHED*/
}

get_name(dest,src)
char *dest,*src;
{
	src++;
	mycpy(dest,src);
}
