/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v 1.1 1987-02-12 21:14:49 rfrench Exp $
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v 1.1 1987-02-12 21:14:49 rfrench Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
 * Check to see if a file exists.
 */

exists(name)
char *name;
{
	struct stat sbuf;
	int retval;

	retval = !lstat(name,&sbuf);

	if (debug)
		printf("exists(%s): %d\n",name,retval);

	return(retval);
}

/*
 * Check to see if a file is older than a certain amount of time.
 */

too_old(name,maxtime)
char *name;
long maxtime;
{
	struct stat sbuf;
	long now;
	int retval;

	time(&now);
	if(stat(name,&sbuf) == -1){
		sprintf(errmsg,
			"can't find file %s in routine too_old()",
			name);
		do_panic();
	}

	retval = ((sbuf.st_mtime + maxtime) <= now);

	if (debug)
		printf("too_old(%s,%ld): %d\n",name,maxtime,retval);

	return (retval);
}

isdir(name)
char *name;
{
	int retval;

	retval = (gettype(name) == 'd');

	if (debug)
		printf("isdir(%s): %d\n",name,retval);

	return(retval);
}

islink(name)
char *name;
{
	int retval;

	retval = (gettype(name) == 'l');

	if (debug)
		printf("islink(%s): %d\n",name,retval);

	return (retval);
}

isfile(name)
char *name;
{
	int retval;

	retval = (gettype(name) == 'f');

	if (debug)
		printf("isfile(%s): %d\n",name,retval);

	return (retval);
}

gettype(name)
char *name;
{
	struct stat sbuf;
	int retval;

	if (!lstat(name,&sbuf)) {
		switch(sbuf.st_mode & S_IFMT) {
			case S_IFDIR:
				retval = 'd';
				break;
			case S_IFREG:
				retval = 'f';
				break;
			case S_IFLNK:
				retval = 'l';
				break;
			case S_IFCHR:
				retval = 'c';
				break;
			case S_IFBLK:
				retval = 'b';
				break;
			default:
				sprintf(errmsg,
					"gettype -- bad file mode for %s\n",
					name);
				do_panic();
		}
	}
	else
		retval = '*';

	if (debug)
		printf("gettype(%s): %c\n",name,retval);

	return (retval);
}

zerolen(name)
char *name;
{
	struct stat sbuf;
	int retval;

	if(stat(name,&sbuf) == -1) {
		sprintf(errmsg,
			"can't find file %s in routine zerolen()\n",
			name);
		do_panic();
	}

	retval = sbuf.st_size;

	if (debug)
		printf("zerolen(%s): %d\n",name,retval);

	return (retval);
}

char *resolve(name)
char *name;
{
	static char retval[LINELEN];
	int linklen;

	if (islink(name) && ((linklen = readlink(name,retval,LINELEN)) != -1))
		retval[linklen] = '\0';
	else {
		sprintf(errmsg,"warning can't follow link named %s\n",name);
		do_gripe();
		strcpy(retval,name);
	}
	return(retval);
}

checkroot(ptr)
char *ptr;
{
	char checkbuf[LINELEN],*tmp;
	int retval;

	/*
	**	assume / exists
	*/
	if (!strcmp(ptr,"/"))
		return(1);
	strcpy(checkbuf,ptr);
	if (!(tmp = rindex(checkbuf,'/'))) {
		sprintf(errmsg,"checkroot can't find / in %s",checkbuf);
		do_gripe();
		return(0);
	}
	*tmp = '\0';

	retval = exists(checkbuf);
	if (debug)
		printf("checkroot(%s): %d\n",ptr,retval);

	return(retval);
}

makeroot(ptr,uid,gid,mode)
char *ptr;
int uid,gid,mode;
{
	char makebuf[LINELEN],*tmp;
	int usave;

	strcpy(makebuf,ptr);
	if (!(tmp = rindex(makebuf,'/'))) {
		sprintf(errmsg,"makeroot can't find / in %s",ptr);
		do_gripe();
		return(-1);
	}
	*tmp = '\0';

	if (exists(makebuf))
		return(0);

	makeroot(makebuf,uid,gid,mode);

	if (verboseflag)
		fprintf(stderr,"making root directory %s mode %o\n",makebuf,mode);

	usave = umask(000);

	if (mkdir(makebuf,mode) == -1) {
		sprintf(errmsg,
			"can't create directory %s",makebuf);
		do_gripe();
		umask(usave);
		return(-1);
	}
	umask(usave);
	
	if ((uid != fileuid(makebuf)) || (gid != filegid(makebuf))) 
		if (chown(makebuf,uid,gid) == -1) {
			sprintf(errmsg, "can't chown directory %s %d %d\n",
					makebuf,uid,gid);
				do_gripe();
		}

	if (chmod(makebuf,mode) == -1) {
		sprintf(errmsg,
			"can't chmod directory %s %d %d\n",
			makebuf,uid,gid);
		do_gripe();
	}

	return (0);
}

fileuid(name)
char *name;
{
	struct stat sbuf;

	if (!lstat(name,&sbuf))
		return(sbuf.st_uid);

	sprintf(errmsg,"fileuid can't find %s",name);
	do_gripe();
	return(-1);
}

filegid(name)
char *name;
{
	struct stat sbuf;

	if (!lstat(name,&sbuf))
		return(sbuf.st_gid);

	sprintf(errmsg,
		"fileuid can't find %s",name);
	do_gripe();
	return(-1);
}

removeit(name)
char *name;
{
	struct direct *next;
	DIR *dirp;
	char rmbuf[LINELEN];

	if (verboseflag)
		fprintf(stderr,"removing %s\n",name);

	switch (gettype(name)) {
	case 'd':
		if (!(dirp = (DIR *)opendir(name))) {
			sprintf(errmsg,"removeit got an error from opendir of %s",name);
			do_gripe();
			return(-1);
		}
		for(next = readdir(dirp);next != NULL ;next = readdir(dirp)) {
			if (strcmp(next->d_name,".") &&
			    strcmp(next->d_name,"..")) {
				strcpy(rmbuf,name);
				strcat(rmbuf,"/");
				strcat(rmbuf,next->d_name);
				if(removeit(rmbuf) == -1) {
					sprintf(errmsg, "removeit can't remove %s",
						rmbuf);
					do_gripe();
					return(-1);
				}
			}
		}
		if (rmdir(name) == -1) {
			sprintf(errmsg, "removeit can't remove directory %s",name);
			do_gripe();
			return(-1);
		}
		break;
	case '*':			/* Can't remove what isn't there! */
		return (0);
	default:
		return(unlink(name));
	}
	return(0);
}
