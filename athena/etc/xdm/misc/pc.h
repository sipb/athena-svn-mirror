#ifndef _PC_H_
#define _PC_H_
#include <sys/types.h>

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

extern pc_state	*	pc_init(void);
extern int 		pc_addport(pc_state *, pc_port *);
extern int		pc_removeport(pc_state *, pc_port *);
extern int 		pc_addfd(pc_state *, pc_fd *);
extern int		pc_removefd(pc_state *, pc_fd *);
extern pc_message *	pc_wait(pc_state *);

extern pc_port *	pc_makeport(char *, int, uid_t, gid_t, mode_t);
extern int		pc_chprot(pc_port *, uid_t, gid_t, mode_t);
extern pc_port *	pc_openport(char *);
extern int		pc_send(pc_message *);
extern pc_message *	pc_receive(pc_port *);
extern int		pc_close(pc_port *);
extern int		pc_freemessage(pc_message *);

#endif /* _PC_H_ */
