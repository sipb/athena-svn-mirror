/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v 1.1 1987-02-12 21:16:00 rfrench Exp $
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v 1.1 1987-02-12 21:16:00 rfrench Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

update_file(remote,local,forcecp)
char *remote,*local;
int forcecp;
{
	struct stamp rstamp,lstamp;
	char fullrname[LINELEN],fulllname[LINELEN];
	int oumask;
	unsigned long timevec[2];

	if (debug)
		printf("update_file(%s,%s,%d)\n",remote,local,forcecp);

	dec_stamp(remote,&rstamp);
	dec_stamp(local,&lstamp);

	timevec[0] = rstamp.ftime;
	timevec[1] = rstamp.ftime;

	sprintf(fulllname,"%s%s",toroot,lstamp.name);
	sprintf(fullrname,"%s%s",fromroot,rstamp.name);

	if (verboseflag) {
		fprintf(stderr,"Updating: source - %s\n",
			make_name(fullrname,&rstamp));
		fprintf(stderr,"            dest - %s\n",
			make_name(fulllname,&lstamp));
	}

	if (!checkroot(fulllname))
		if (lstamp.type == 'd')
			makeroot(fulllname,rstamp.uid,rstamp.gid,rstamp.mode);
		else {
			sprintf(errmsg,"can't find parent directory for %s",
				fulllname);
			do_gripe();
			return;
		}

	if (exists(fulllname))
		if (!((rstamp.type == 'f' && lstamp.type == 'f') ||
		     (rstamp.type == 'd' && lstamp.type == 'd') ||
		     (((rstamp.type == 'c' && lstamp.type == 'c') ||
		       (rstamp.type == 'b' && lstamp.type == 'b')) &&
		      (rstamp.dev == lstamp.dev)))) {
				    if (removeit(fulllname) == -1) {
					    sprintf(errmsg,"can't remove %s\n",fulllname);
					    do_gripe();
					    return;
				    }
			    }

	switch (rstamp.type) {
	case '*':
		return;
	case 'l':
		oumask = umask(0); /* Symlinks don't really have modes */
		if (symlink(rstamp.link,fulllname)) {
			sprintf(errmsg,
				"can't create symbolic link from %s to %s\n",
				fulllname,rstamp.link);
			do_gripe();
			return;
		}
		umask(oumask);
		break;
	case 'd':
		if (!exists(fulllname) && (mkdir(fulllname,rstamp.mode) == -1)) {
			sprintf(errmsg,
				"can't create directory %s\n",fulllname);
			do_gripe();
		}
		break;
	case 'f':
		if (lstamp.type != 'f' || rstamp.ftime != lstamp.ftime || forcecp)
			if (copy_file(fullrname,fulllname))
				return;
		break;
	case 'b':
	case 'c':
		if (rstamp.dev != lstamp.dev || rstamp.type != lstamp.type)
		if (mknod(fulllname,rstamp.mode | (rstamp.type=='c' ? S_IFCHR :
					     (rstamp.type=='b' ? S_IFBLK : 0)),rstamp.dev)) {
			sprintf(errmsg,"can't make device %s\n",fulllname);
			do_gripe();
			return;
		}
		break;
	}

	if (rstamp.type != 'l') {
		if ((rstamp.uid != fileuid(fulllname)) ||
		    (rstamp.gid != filegid(fulllname))) {
			    if (chown(fulllname,rstamp.uid,rstamp.gid) == -1) {
				    sprintf(errmsg,"can't chown file %s %d %d\n",
					    fulllname,rstamp.uid,rstamp.gid);
				    do_gripe();
				    return;
			    }
		    }
		if (chmod(fulllname,rstamp.mode) == -1) {
			sprintf(errmsg,
				"can't chmod file %s %o\n",
				fulllname,rstamp.mode);
			do_gripe();
			return;
		}
		if (rstamp.type != 'b' && rstamp.type != 'c')
			utime(fulllname,timevec);
	}
}

copy_file(from,to)
char *from,*to;
{
	char buf[MAXBSIZE],tempname[LINELEN];
	int fdf,fdt,n;

	fdf = open(from,O_RDONLY);
	if (!fdf) {
		sprintf(errmsg,"can't open input file %s\n",from);
		do_gripe();
		return (1);
	}
	sprintf(tempname,"%s_trk.tmp",to);
	fdt = open(tempname,O_WRONLY|O_CREAT);
	if (!fdt) {
		sprintf(errmsg,"can't open temporary file %s\n",tempname);
		do_gripe();
		return (1);
	}

	for (;;) {
		n = read(fdf,buf,sizeof buf);
		if (!n)
			break;
		if (n < 0) {
			sprintf(errmsg,"error while reading file %s\n",from);
			do_gripe();
			close(fdf);
			close(fdt);
			return (1);
		}
		if (write(fdt,buf,n) != n) {
			sprintf(errmsg,"error while writing file %s\n",to);
			do_gripe();
			close(fdf);
			close(fdt);
			return (1);
		}
	}
	close(fdf);
	close(fdt);
	rename(tempname,to); /* atomic! */
	return (0);
}

char *make_name(name,stmp)
char *name;
struct stamp *stmp;
{
	static char buff[LINELEN];

	switch (stmp->type) {
	case '*':
		sprintf(buff,"nonexistant %s",name);
		break;
	case 'f':
		sprintf(buff,"file %s (uid %d, gid %d, mode %04o)",
			name,stmp->uid,stmp->gid,stmp->mode);
		break;
	case 'l':
		sprintf(buff,"link %s pointing to %s",name,stmp->link);
		break;
	case 'd':
		sprintf(buff,"dir %s (uid %d, gid %d, mode %04o)",
			name,stmp->uid,stmp->gid,stmp->mode);
		break;
	case 'c':
	case 'b':
		sprintf(buff,"device %s (uid %d, gid %d, mode %03o) maj %d min %d",
			name,stmp->uid,stmp->gid,stmp->mode&0777,
			major(stmp->dev),minor(stmp->dev));
		break;
	}
	return (buff);
}
