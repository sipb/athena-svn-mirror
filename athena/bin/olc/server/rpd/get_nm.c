#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/get_nm.c,v 1.3 1991-04-11 09:41:27 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

char *
  get_nm(username,instance,result,nuke)
char *username;
int instance;
int *result;
int nuke;
{
  char fnbuf[MAXPATHLEN];
  static char *buf;
  static int buflen;
  struct stat statbuf;
  int len;
  int fd;

  sprintf(fnbuf,"%s/%s_%d.nm",LOG_DIRECTORY,username,instance);
  if ((fd = open(fnbuf,O_RDONLY,0)) < 0) {
    *result = 0;
    return(NULL);
  }

  if (fstat(fd,&statbuf) < 0) {
    *result = errno;
    return(NULL);
  }

  if (statbuf.st_size > buflen) {
    if (buflen != 0)
      free(buf);
    buflen = statbuf.st_size;
    if ((buf = (char *)malloc(buflen)) == NULL) {
      syslog(LOG_ERR,"get_nm: malloc: error alloc'ind %d bytes\n",
	     statbuf.st_size);
      close(fd);
      *result = -1;
      return(NULL);
    }
  }

  len = read(fd,buf,statbuf.st_size);
  if (len != statbuf.st_size) {
    syslog(LOG_ERR,"get_nm: read: %m on %s",fnbuf);
    close(fd);
    free(buf);
    buflen = 0;
    *result = -1;
    return(NULL);
  }
    
  close(fd);
  *result = len;
  if (nuke)
    unlink(fnbuf);
  return (buf);
}
