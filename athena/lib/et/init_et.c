/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/et/init_et.c,v 1.2 1997-12-19 03:04:11 ghudson Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/et/init_et.c,v $
 * $Locker:  $
 *
 * Copyright 1986, 1987, 1988 by MIT Information Systems and
 *	the MIT Student Information Processing Board.
 *
 * For copyright info, see mit-sipb-copyright.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "error_table.h"
#include "mit-sipb-copyright.h"

static const char rcsid[] = "$Id: init_et.c,v 1.2 1997-12-19 03:04:11 ghudson Exp $";

struct foobar {
    struct et_list etl;
    struct error_table et;
};

extern struct et_list *_et_list;

int init_error_table(const char *const *msgs, int base, int count)
{
    struct foobar * new_et;

    if (!base || !count || !msgs)
	return 0;

    new_et = malloc(sizeof(struct foobar));
    if (!new_et)
	return errno;	/* oops */
    new_et->etl.table = &new_et->et;
    new_et->et.msgs = msgs;
    new_et->et.base = base;
    new_et->et.n_msgs= count;

    new_et->etl.next = _et_list;
    _et_list = &new_et->etl;
    return 0;
}
