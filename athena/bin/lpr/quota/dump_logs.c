#include <stdio.h>
#include "logger.h"

log_header head;

main(argc, argv) 
int argc;
char *argv[];
{
    int i;
    log_entity *l;
    char buf[1024];
    char *name, *string;

    if (argc < 2) {
	name = "/usr/spool/quota/journal.db";
	string = "/usr/spool/quota/string.db";
    }
    else {
	if (argc != 3) {
	    fprintf(stderr, "Usage: dump_logs [journal_db  string_db]\n");
	    exit(1);
	}
	name = argv[1];
	string = argv[2];
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


    if(logger_string_set_name(string) ||
       logger_read_strings()==0){
	printf("Could not read string database\n");
	exit(1);
    }

    for(i=1; i < (int)head.num_ent; i++) {
	l = logger_journal_get_line(i);
	if(logger_cvt_line(l, buf)) {
	    fprintf(stderr, "line %d: printing bad %d\n", i,l->func);
	    /* exit(1); */
	}
	if(strlen(buf) > 1) printf("%s\n",buf);
    }
}


void PROTECT(){}   
void UNPROTECT(){}   
	
