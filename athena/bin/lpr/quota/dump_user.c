#include <stdio.h>
#include "logger.h"

main(argc, argv) 
int argc;
char *argv[];
{
    int print();
    char *name, *string;
    
    if (argc < 2) {
	name = "/usr/spool/quota/user.db";
	string = "/usr/spool/quota/string.db";
    }
    else {
	if (argc != 3) {
	    fprintf(stderr, "Usage: dump_user [user_db  string_db]\n");
	    exit(1);
	}
	name = argv[1];
	string = argv[2];
    }

    if (logger_user_set_name(name)) {
	printf("Could not read user database\n");
	exit(1);
    }
    
    if(logger_string_set_name(string) ||
       logger_read_strings()==0){
	printf("Could not read string database\n");
	exit(1);
    }
    
    logger_user_iterate(print, 0);
    exit(0);
}

print(arg, u_db)
char *arg;
User_db *u_db;
{
    printf("%s %s %s - %d %d\n",
	   logger_num_to_string(u_db->user.name),
	   logger_num_to_string(u_db->user.instance),
	   logger_num_to_string(u_db->user.realm),
	   u_db->first,
	   u_db->last);
}

void PROTECT(){}
void UNPROTECT(){}
