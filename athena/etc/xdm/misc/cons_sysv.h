#ifndef _CONS_SYSV_H_
#define _CONS_SYSV_H_

typedef struct _cons_state {
  int gotpty;
  int tty_fd, pty_fd;
  char ttydev[64], ptydev[64];

  int state;
  int exitStatus;
  pid_t pid;
  int fd[2];
} cons_state;

#include "cons.h"

#endif /* _CONS_SYSV_H_ */
