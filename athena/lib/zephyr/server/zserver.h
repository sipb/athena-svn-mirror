/* Coypright 1987, Massachusetts Institute of Technology */
/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/zserver.h,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/zserver.h,v 1.1 1987-03-18 17:48:29 jtkohl Exp $
 */

/* definitions for the Zephyr server */

typedef struct _ZClientDesc_t {
    struct _ZClientDesc_t *q_forw;
    struct _ZClientDesc_t *q_back;
} ZClientDesc_t;

typedef struct _ZEntity_t {
    char *filler;			/* fill this in later */
} ZEntity_t;

/* Function declarations */

extern char *strsave();

/* server internal error codes */
#define	ZERR_S_FIRST	2000
#define ZERR_S_BADASSOC	2000		/* client not associated with class */
