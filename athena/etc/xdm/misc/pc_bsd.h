#ifndef _PCP_H_
#define _PCP_H_

#define PC_LISTENER 1
#define PC_LISTENEE 2
#define PC_REGULAR 3

typedef struct _pc_port {
  struct _pc_port *parent;
  int fd;
  int type;
  char *path;
} pc_port;

#define PC_MAXPORTS 10

typedef struct _pc_state {
  int numports;
  pc_port *ports[PC_MAXPORTS];
  int nfds;
  fd_set readfds, writefds;
} pc_state;

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#include "pc.h"

#endif /* _PCP_H_ */
