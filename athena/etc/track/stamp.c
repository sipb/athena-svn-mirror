/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v 1.1 1987-02-12 21:15:36 rfrench Exp $
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v 1.1 1987-02-12 21:15:36 rfrench Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
 * Place a stamp line in the format:
 *       type file uid.gid.mode.time
 * or a suitable derivate thereof
 */

stamp(file,sname,buf)
char *file,*sname,*buf;
{
	struct stat sbuf;
	int linklen,type;
	char linkbuf[LINELEN],*ptr;

	if (debug)
		printf("stamp(%s,%s)\n",file,sname);

	type = gettype(file);
	if (type == '*') {
		sprintf(buf,"*%s",sname);
		if (debug)
			printf("stamp return(buf): %s\n",buf);
		return;
	}

	sprintf(buf,"%c%s ",type,sname);
	ptr = buf+strlen(buf);

	if (lstat(file,&sbuf) == -1) {
		sprintf(errmsg,"inconsistent stat's on %s\n",file);
		do_panic();
	}

	if (buf[0] == 'l') {
		if((linklen = readlink(file,linkbuf,LINELEN)) == -1)
			sprintf(buf,"*%s",sname);
		else {
			linkbuf[linklen] = '\0';
			sprintf(ptr,"%s",linkbuf);
		}
	}
	else if (buf[0] == 'c' || buf[0] == 'b')
		sprintf(ptr,"%d.%d.%o.%d",sbuf.st_uid,sbuf.st_gid,
			sbuf.st_mode&07777,sbuf.st_rdev);
	else
		sprintf(ptr,"%d.%d.%o.%ld",sbuf.st_uid,sbuf.st_gid,
			sbuf.st_mode&07777,(long)sbuf.st_mtime);

	if (debug)
		printf("stamp return(buf): %s\n",buf);
}

/*
 * Decode a stamp into its individual fields
 */

dec_stamp(stmp,ret)
char *stmp;
struct stamp *ret;
{
	ret->type = *(stmp++);
	mycpy(ret->name,stmp);
	skipword(&stmp);
	skipspace(&stmp);
	if (*stmp == '*')
		return;
	if (ret->type == 'l') {
		strcpy(ret->link,stmp);
		return;
	}
	if (ret->type == 'c' || ret->type == 'b') {
		sscanf(stmp,"%d.%d.%o.%d",&ret->uid,&ret->gid,&ret->mode,
		       &ret->dev);
		return;
	}
	sscanf(stmp,"%d.%d.%o.%ld",&ret->uid,&ret->gid,&ret->mode,&ret->ftime);
}
