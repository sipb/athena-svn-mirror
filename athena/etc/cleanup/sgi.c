#include <dirent.h>
#include <stdio.h>
#include <sys/procfs.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "cleanup.h"

#define PROCDIR "/proc/pinfo"

struct cl_proc *get_processes()
{
DIR *procdir;
struct dirent *procnum;
char buf[PATH_MAX];
int fd;
prpsinfo_t* p;
int retval;
int num,i=0;
static struct cl_proc procs[MAXPROCS];
	/* open the /proc/pinfo directory */
	if ( (procdir = opendir (PROCDIR) ) == NULL ) {
		fprintf(stderr,"cleanup: open of /proc/pinfo failed\n");
		procs[i].pid = procs[i].uid = -1;
		return(procs);
	} /* if */
	/* step through the directory and read off the procs */
	while ((procnum=readdir(procdir)) != NULL) {
	/* skip . & .. */
          if (!(strcmp(procnum->d_name,"."))) {
		;
	  } else if  (!(strcmp(procnum->d_name,".."))) {
		;
	  } else {
		/* we have valid number. initialize the buffer and turn it
		the name into a number */
		memset(buf,0,PATH_MAX*sizeof(char));
		sprintf(buf,"%s/%s",PROCDIR,procnum->d_name);
		sscanf(procnum->d_name,"%d",&num);
		/* open the /proc/pinfo/XXXX file to use the ioctl on it */
		/* if the open fails just cycle thru */
		if ((fd = open(buf,O_RDONLY)) != -1) {
			/* allocate the prpsinfo struct */
			p = (prpsinfo_t *)calloc(sizeof(prpsinfo_t),1); 
			/* if it fails just cycle thru */
			if (p == NULL) { 
				fprintf(stdout,"malloc failed\n");
			} else if ((retval = ioctl(fd,PIOCPSINFO,(void*)p)) != -1) {
					/* we have valid data get rid of root
					 * and save the rest 
					 */
					if (p->pr_uid != 0) {
						procs[i].pid = p->pr_pid;
						procs[i].uid = p->pr_uid;
						i++;
					}
			} else {
				fprintf(stdout,"errno == %i\n",errno);
			}
		if (p)
			free(p);
		close(fd);
		}
	  }
	} /* while */
	closedir(procdir);
	procs[i].pid = procs[i].uid = -1;
	return(procs);
}

#ifdef STAND_ALONE
void print_out(struct cl_proc * p)
{
int i=0;

	while ( p[i].pid != -1 ) {
		fprintf(stdout,"%i,%i\n",p[i].pid,p[i].uid);
		i++;
	}
}

main()
{
struct cl_proc *procs;
	procs = get_processes();
	print_out(procs);
}
#endif


