#include "include.h"

call_program(program, arg, ret, size)
     char *program;
     char *arg;
     char *ret;
     int size;
{
  int f[2];

  pipe(f);
  switch(fork()) 
    {
    case 0:             /* child */
      (void) close(f[0]);
      (void) close(0);
      (void) dup2(f[1], 1);
      execl(program, program, arg, 0);
      perror("exec");
      _exit(1);
      break;
    case -1:            /* error */
      perror("Can't fork");
      return(-1);
    default:            /* parent */
      (void) close(f[1]);
  }

  read(f[0],ret,size);
  wait(0);
  (void) close(f[0]);
  ret[size-1] = '\0';
}
  
