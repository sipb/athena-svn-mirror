/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: accounting.c,v 1.1.1.1 1999-02-17 15:31:05 ghudson Exp $";

#include "ifhp.h"

/**** ENDINCLUDE ****/

/*
 *  Do_accounting()
 *  writes the accounting information to the accounting file
 *  This has the format: user host printer pages format date
 */


void Do_accounting(int start, int elapsed, int pagecounter, int npages )
{
	int i, len, pid, status, starts;
	char working[SMALLBUFFER];
	struct line_list l;
	char *s, *t, *list;


	Init_line_list( &l );
	Check_max( &l, 100 );

	log( "Do_accounting: accounting at %s, pagecount %d, pages %d",
		start?"start":"end", pagecounter, npages);
	DEBUG3("Accounting script '%s', file '%s', fd %d, npages %d",
		Accounting_script, Accountfile, Accounting_fd, npages );

	/* we first set up the output line and arguments */
	Add_line_list( &l, Accounting_script, 0, 0, 0 );
	starts = l.count;
	if( start ){
		Add_line_list( &l, "start", 0, 0, 0 );
	} else {
		Add_line_list( &l, "end", 0, 0, 0 );
		plp_snprintf( working, sizeof(working), "-b%d", npages );
		Add_line_list( &l, working, 0, 0, 0 );
		plp_snprintf( working, sizeof(working), "-T%d", elapsed );
		Add_line_list( &l, working, 0, 0, 0 );
	}
	
	plp_snprintf( working, sizeof(working), "-q%d", getpid() );
	Add_line_list( &l, working, 0, 0, 0 );
	plp_snprintf( working, sizeof(working), "-p%d", pagecounter );
	Add_line_list( &l, working, 0, 0, 0 );
	plp_snprintf( working, sizeof(working), "-t%s", Time_str(0,0) );
	Add_line_list( &l, working, 0, 0, 0 );
	for( i = 1; i < Argc; ++i ){
		Add_line_list( &l, Argv[i], 0, 0, 0 );
	}
	if(DEBUGL3)Dump_line_list("Do_accounting",&l );
	Check_max( &l, 1 );
	l.list[l.count] = 0;
	len = 0;
	for( i = starts; i < l.count; ++i ){
		if( (t = l.list[i]) ){
			len += strlen(t)+1;
		}
	}
	len += 3;
	list = malloc_or_die( len,__FILE__,__LINE__);
	list[0] = 0;
	for( s = list, i = starts; i < l.count; ++i ){
		if( (t = l.list[i])){
			if( list[0] ){
				strcpy(s," ");
				s += strlen(s);
			}
			strcpy(s,t);
			s += strlen(s);
		}
	}
	DEBUG1("Accounting output '%s'", list );
	*s++='\n';
	*s = 0;

	/* this is probably the best way to do the output */
	if(Accounting_fd > 0 ||
	     (Accountfile && (Accounting_fd = open(Accountfile, O_WRONLY|O_APPEND )) >= 0 )) {
	    Write_fd_str(Accounting_fd,list);
	}
	if( Accounting_script && *Accounting_script ){
		pid = fork();
		if( pid == -1 ) {
			Errorcode = JABORT;
			logerr("Cannot fork to update accounting");
		} else if( pid == 0 ){
			/* child - Exec the script to update pages */
			dup2(2,1);
			close(0);
			if( open( "/dev/null", O_RDONLY ) != 0 ){
				logerr_die("Cannot open /dev/null");
			}
			for( len = 3; len < 20; ++len ){
				fcntl(len, F_SETFD, 1);
			}
			execve(Accounting_script,l.list,Envp);
			Errorcode = JABORT;
			logerr_die("exec %s failed",Accounting_script);
		} else {
			/* father - Wait for the update
			 * Child exits with 0 for Ok, 2 if signal
			 * terminated and 1 otherwise.
			 */
			while( (len = waitpid( pid, &status, 0 )) != pid );
			DEBUG0("Accounting process exited, status 0x%x", status);
			if( status ){
				Errorcode = JABORT;
				if( WIFEXITED(status) ){
					Errorcode = WEXITSTATUS(status);
				}
				fatal("Accounting process died, status 0x%x", status);
			}
		}
	}
}
