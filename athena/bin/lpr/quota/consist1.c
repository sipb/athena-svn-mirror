#include <stdio.h>
#include "logger.h"
#include "logger_ncs.h"
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
    char *name, *sname;
    log_header head;
    int error=0;
    
    if (argc < 2) {
	name = "/usr/spool/quota/journal.db";
    }
    else {
	if (argc >3) {
	    fprintf(stderr, "Usage: consist [journal_db] [string_db]\n");
	    exit(1);
	}
	name = argv[1];
    }

    if(argc < 3) {
	sname = "/usr/spool/quota/string.db";
    }
    else {
	if (argc > 3) {
	    fprintf(stderr, "Usage: consist [journal_db] [string_db]\n");
	    exit(1);
	}
	sname = argv[2];
    }

    if(logger_string_set_name(sname) ||( logger_read_strings()==0)) {
	fprintf(stderr, "Could not set string table");
	exit(3);
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


    /* Cheat and create the bitmap that we want */
    bitmap_size = sizeof(fd_set) + howmany(head.num_ent, NFDBITS) * sizeof(long);
    bitmap = (fd_set *) malloc(bitmap_size);
    bzero(bitmap, bitmap_size);

    
    for(i=1; i < (int)head.num_ent; i++) {
	if(ISSET(i)) continue; /* Skip entries already found */
	fprintf(stderr, "Checking %d\n", i);
	if(follow_chain(i)) {
	    fprintf(stderr, "Chain starting at %d failed\n", i);
	    exit(1);
	}
    }

    for(i=1; i < (int)head.num_ent; i++) {
	if(ISSET(i)) continue;
	fprintf(stderr, "Entry %d not in chains\n");
	error = -1;
    }

    /* All good */
    exit(error);

}


void PROTECT(){}   
void UNPROTECT(){}   
	


int follow_chain(start)
Pointer start;
{

    log_entity *lent;
    Pointer n = start, prev=0;
    Time ltime=0; 
    char buf[1024];

    /* Start off the progression */
    if(!(lent = logger_journal_get_line(n))) {
	fprintf(stderr, "Could not read line %d\n", n);
	return -1;
    }

    
    if(lent->prev != 0) {
	fprintf(stderr,"Not starting at head of chain %d\n", start); 
    }

    if(lent->func == LO_CHARGE) ltime = lent->trans.charge.subtime;
    prev = n;
    SET(n);
    while(lent->next != 0) {
	prev = n;
	n = lent->next;

	if(ISSET(n)) {
	    fprintf(stderr, "Already referenced - failure %d\n", n);
	    return -1;
	}

	if(!(lent = logger_journal_get_line(n))) {
	    fprintf(stderr, "Could not read line %d\n", n);
	    return -1;
	}
	if(lent->prev != prev) {
	    fprintf(stderr, "prev not match in line %d %d !=% d\n", n,
		    prev, lent->prev);
	    return -1;
	}
	if(lent->func == LO_CHARGE) {
	    if(abs(lent->time - lent->trans.charge.subtime) > 8*60*60) 
		if(logger_cvt_line(lent,buf)) fprintf(stderr, "Couldn't parse %d\n", n);
		else fprintf(stderr,"%d %s\n", n, buf);
	    
	    ltime = lent->trans.charge.subtime;
	}
	
	SET(n);
    }

    return 0;
}


