#include <stdio.h>
#include "logger.h"
#include <sys/types.h>
#ifdef _IBMR2
#include <sys/select.h>
#endif

#define ISSET(n) FD_ISSET(n, bitmap)
#define SET(n)	FD_SET(n, bitmap)
#define CLR(n)	FD_CLR(n, bitmap)
fd_set *bitmap; 
int bitmap_size;

#ifdef _AUX_SOURCE
typedef long	fd_mask;
#define NBBY	8
#define NFDBITS	(sizeof(fd_mask) * NBBY)
#ifndef howmany
#define howmany(x, y)	 (((x)+((y)-1))/(y))
#endif
#define FD_SET(n, p)   ((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)   ((p)->fds_bits[0] & (1 << (n)))
#endif

main(argc, argv) 
int argc;
char *argv[];
{
    int i;
    char *name;
    log_header head;
    int error=0;
    
    if (argc < 2) {
	name = "/usr/spool/quota/journal.db";
    }
    else {
	if (argc != 2) {
	    fprintf(stderr, "Usage: consist [journal_db]\n");
	    exit(1);
	}
	name = argv[1];
    }

    if(logger_journal_set_name(name)) {
	printf("Could not set name\n");
	exit(1);
    }

    if(logger_journal_get_header(&head)) {
	printf("Could not get head\n");
	exit(1);
    }

#ifdef DEBUG
    printf("ver %d, num_ent %d, last_q_time %d, pos %d\n", 
	   head.version, head.num_ent, head.last_q_time, head.quota_pos);
#endif
    if(head.version != LOGGER_VERSION) {
	printf("Version mismatch on logs database: %d != %d\n", 
	       head.version, LOGGER_VERSION);
	exit(1);
    }

    while((scanf("%d\n", &i) == 1) && i != 0)
	delete_entry(i);
    /* All good */
    exit(0);

}


void PROTECT(){}   
void UNPROTECT(){}   
	
int delete_entry(start)
Pointer start;
{

    log_entity *lent;
    log_entity prev, cur, next;
    int pwrite=0, nwrite=0;

    bzero(&prev, sizeof(log_entity));
    bzero(&next, sizeof(log_entity));

    if(!(lent = logger_journal_get_line(start))) {
	fprintf(stderr, "Could not read line %d\n", start);
	return -1;
    }
    bcopy(lent, &cur, sizeof(log_entity));

    if(cur.prev != 0) {
	if(!(lent = logger_journal_get_line(cur.prev))) {
	    fprintf(stderr, "Could not read line %d\n", cur.prev);
	    return -1;
	}
	bcopy(lent, &prev, sizeof(log_entity));
	pwrite = 1;
	printf("prev %d %d\n", cur.prev, prev.next);
    }

    if(cur.next != 0) {
	if(!(lent = logger_journal_get_line(cur.next))) {
	    fprintf(stderr, "Could not read line %d\n", cur.next);
	    return -1;
	}
	bcopy(lent, &next, sizeof(log_entity));
	nwrite = 1;
	printf("prev %d %d\n", cur.next, next.prev);
    }


    prev.next = cur.next;
    next.prev = cur.prev;

    if(nwrite) {
	if(logger_journal_write_line(cur.next, &next)) {
	    fprintf(stderr, "Could not write line %d\n", cur.next);
	    return -1;
	}
    }

    if(pwrite) {
	if(logger_journal_write_line(cur.prev, &prev)) {
	    fprintf(stderr, "Could not write line %d\n", cur.prev);
	    return -1;
	}
    }

    cur.next = 0;
    cur.prev = 0;
    cur.user.name = 0;
    cur.user.instance = 0;
    cur.user.realm = 0;
    if(logger_journal_write_line(start, &cur)) {
	fprintf(stderr, "Could not write line %d\n", start);
	return -1;
    }
    printf("Entry %d removed\n", start);
    return 0;
}




