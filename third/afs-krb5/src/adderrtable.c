/*
 * $Id: adderrtable.c,v 1.2 2003-02-14 23:16:05 zacheiss Exp $
 *
 * adderrtable - A replacement for the AFS "add_to_error_table" function
 *
 * Sigh, this lossage is necessary because in AFS 3.5, Transarc made private
 * the _et_list symbol and replaced it with a function called
 * "add_to_error_table".  Because the V5 library depends on the existance
 * of _et_list, here is a stub add_to_error_table function that does
 * all of the stuff necessary to add an error table to the master list.
 * Note that we're not doing any locking (which I don't think is a problem
 * in the single-threaded case).
 *
 */

#ifndef LINT
static char rcs_id[]=
	"$Id: adderrtable.c,v 1.2 2003-02-14 23:16:05 zacheiss Exp $";
#endif
#include <afs/error_table.h>

struct et_list;
void add_error_table (struct error_table *);



void
add_to_error_table(struct et_list *new_table)
{

  add_error_table(new_table->table);
}


