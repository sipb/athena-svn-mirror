#ifndef _PC_H_
#define _PC_H_
#include <sys/types.h>

#include "pc_err.h"

#ifndef _PC_BSD_H_
typedef struct _pc_port pc_port;
typedef struct _pc_state pc_state;
#endif /* _PC_BSD_H_ */

#define PC_READ 1
#define PC_WRITE 2

typedef struct {
	int fd;
	int events;
} pc_fd;

#define PC_BROKEN 0
#define PC_DATA 1
#define PC_FD 2
#define PC_SIGNAL 3
#define PC_NEWCONN 4

typedef struct {
  int type;
  int fd, event;

  pc_port *source;
  int length;
  void *data;
} pc_message;

#define PC_GRAB 1
#define PC_CHOWN 2
#define PC_CHMOD 4

extern long	pc_init(pc_state **);
extern long	pc_destroy(pc_state *);
extern long 	pc_addport(pc_state *, pc_port *);
extern long	pc_removeport(pc_state *, pc_port *);
extern long 	pc_addfd(pc_state *, pc_fd *);
extern long	pc_removefd(pc_state *, pc_fd *);
extern long	pc_wait(pc_message **, pc_state *);
extern long	pc_secure(pc_state *, pc_port *, pc_message *);
extern long	pc_makeport(pc_port **, char *, int,
			    uid_t, gid_t, mode_t);
extern long	pc_chprot(pc_port *, uid_t, gid_t, mode_t);
extern long	pc_openport(pc_port **, char *);
extern long	pc_send(pc_message *);
extern long	pc_receive(pc_message **, pc_port *);
extern long	pc_close(pc_port *);
extern long	pc_freemessage(pc_message *);

#endif /* _PC_H_ */
