#include <stdio.h>
#include "logger.h"
#include <sys/types.h>

main(argc, argv) 
int argc;
char *argv[];
{
    int i;
    char *name, *string;
    log_header head;
    int error=0;
    Pointer p;
    log_entity *lent;
    char buf[1024];
    
    if (argc != 4) {
	    fprintf(stderr, "Usage: consist journal_db string.db entry\n");
	    exit(1);
    }
    name = argv[1];
    string = argv[2];
    p = atoi(argv[3]);
    if(p<=0) {
	    fprintf(stderr, "Pointer must be positive\n");
	    exit(2);
    }
    

    if(logger_journal_set_name(name)) {
	printf("Could not set name\n");
	exit(1);
    }

    if(logger_string_set_name(string) ||
       logger_read_strings()==0){
	printf("Could not read string database\n");
	exit(1);
    }


    if(logger_journal_get_header(&head)) {
	printf("Could not get head\n");
	exit(1);
    }

    printf("ver %d, num_ent %d, last_q_time %d, pos %d\n", 
	   head.version, head.num_ent, head.last_q_time, head.quota_pos);
    if(head.version != LOGGER_VERSION) {
	printf("Version mismatch on logs database: %d != %d\n", 
	       head.version, LOGGER_VERSION);
	exit(1);
    }


    if (p > head.num_ent) {
	    fprintf(stderr, "Pointer is beyond end of data\n");
	    exit(2);
    }

    if(!(lent = logger_journal_get_line(p))) {
	fprintf(stderr, "Could not read line %d\n", p);
	exit(2);
    }

    printf("Prev %d\n", lent->prev);
    printf("Next %d\n", lent->next);

    logger_cvt_line(lent, buf);
    printf("%s\n", buf);
    exit(0);
}


void PROTECT(){}   
void UNPROTECT(){}   
	




