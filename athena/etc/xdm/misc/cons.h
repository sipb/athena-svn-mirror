#ifndef _CONS_H_
#define _CONS_H_

#define CONS_DOWN 1
#define CONS_UP 2
#define CONS_FROZEN 3

#if !defined(_CONS_SYSV_H_)
typedef struct _cons_state cons_state;
#endif

extern cons_state *	cons_init(void);
extern int 		cons_getpty(cons_state *);
extern int	 	cons_grabcons(cons_state *);
extern int 		cons_start(cons_state *);
extern int 		cons_stop(cons_state *);
extern int		cons_child(cons_state *, void *, pid_t);

#endif /* _CONS_H_ */
