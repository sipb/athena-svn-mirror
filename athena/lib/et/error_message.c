/*
 * $Id: error_message.c,v 1.6 2002-01-29 20:27:49 rbasch Exp $
 *
 * Copyright 1987 by the Student Information Processing Board
 * of the Massachusetts Institute of Technology
 *
 * For copyright info, see "mit-sipb-copyright.h".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "error_table.h"
#include "com_err.h"
#include "mit-sipb-copyright.h"

static const char rcsid[] = "$Id: error_message.c,v 1.6 2002-01-29 20:27:49 rbasch Exp $";
static const char copyright[] =
    "Copyright 1986, 1987, 1988 by the Student Information Processing Board\nand the department of Information Systems\nof the Massachusetts Institute of Technology";

static char buffer[25];

struct et_list *_et_list = NULL;

const char *error_message(errcode_t code)
{
    int offset;
    struct et_list *et;
    int table_num;
    int started = 0;
    char *cp;

    offset = code & ((1<<ERRCODE_RANGE)-1);
    table_num = code - offset;
    if (!table_num)
	return strerror(offset);
    for (et = _et_list; et; et = et->next) {
	if (et->table->base == table_num) {
	    /* This is the right table */
	    if (et->table->n_msgs <= offset)
		goto oops;
	    return(et->table->msgs[offset]);
	}
    }
oops:
    strcpy (buffer, "Unknown code ");
    if (table_num) {
	strcat (buffer, error_table_name (table_num));
	strcat (buffer, " ");
    }
    for (cp = buffer; *cp; cp++)
	;
    if (offset >= 100) {
	*cp++ = '0' + offset / 100;
	offset %= 100;
	started++;
    }
    if (started || offset >= 10) {
	*cp++ = '0' + offset / 10;
	offset %= 10;
    }
    *cp++ = '0' + offset;
    *cp = '\0';
    return(buffer);
}

errcode_t add_error_table(const struct error_table *et)
{
    struct et_list *el = _et_list;

    while (el) {
	if (el->table->base == et->base)
	    return EEXIST;
	el = el->next;
    }

    if (!(el = malloc(sizeof(struct et_list))))
	return ENOMEM;

    el->table = et;
    el->next = _et_list;
    _et_list = el;

    return 0;
}

errcode_t remove_error_table(const struct error_table *et)
{
    struct et_list *el = _et_list;
    struct et_list *el2 = 0;

    while (el) {
	if (el->table->base == et->base) {
	    if (el2)	/* Not the beginning of the list */
		el2->next = el->next;
	    else
		_et_list = el->next;
	    free(el);
	    return 0;
	}
	el2 = el;
	el = el->next;
    }
    return ENOENT;
}
