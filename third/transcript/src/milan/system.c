/* Copyright Milan Technology, 1991 1992 */
/* @(#)system.c	2.0 10/9/92 */

#include <signal.h>
#include "dp.h"
#include "errors.h"

#ifdef BSD
#include <sys/wait.h>
#endif


#ifdef BSD
void sig_child()
{
   int pid;
   union wait status;
   while ((pid = wait3(&status,WNOHANG,(struct rusage*)0))>0);
}
#endif

/* Makes a pipe, forks, makes the stdin in child come from
 * the read end of the pipe.  Returns the write end of the
 * pipe to the parent.
 */

#ifdef ANSI
int initPipes(char *cmd)
#else
int initPipes(cmd)
char* cmd;
#endif
{
   int pipe_fd[2], pid;

   if (pipe(pipe_fd) < 0)
      error_protect(ERR_PIPE);
   if ((pid = fork()) < 0)
      error_protect(ERR_FORK);
   else if (pid > 0) {
      /* parent */
      close(pipe_fd[readEnd]);
      return(pipe_fd[writeEnd]);
   }
   else {
      /* child */
      /* make childs stdin come from read end of pipe */
      if (pipe_fd[readEnd] != STDIN_FILENO) {
	 dup2(pipe_fd[readEnd], STDIN_FILENO);
	 close(pipe_fd[readEnd]);
      }
      close(pipe_fd[writeEnd]);
      execl("/bin/sh", "sh", "-c", cmd, (char*)0);
   }   
} 
