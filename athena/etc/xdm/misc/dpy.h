#ifndef _DPY_H_
#define _DPY_H_

#define DPY_NONE 1
#define DPY_X 2
#define DPY_CONS 3

#if !defined(_DPY_IRIX_H_)
typedef struct _dpy_state dpy_state;
#endif

dpy_state *	dpy_init(void);
char *		dpy_consDevice(dpy_state *);
int		dpy_startX(dpy_state *);
int		dpy_stopX(dpy_state *);
int		dpy_startCons(dpy_state *);
int		dpy_stopCons(dpy_state *);
int		dpy_status(dpy_state *);
int		dpy_child(dpy_state *, pid_t, void *);

#endif /* _DPY_H_ */
