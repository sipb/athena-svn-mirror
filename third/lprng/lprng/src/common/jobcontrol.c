/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: jobcontrol.c
 * PURPOSE: read and write the spool queue control file
 **************************************************************************/

static char *const _id =
"jobcontrol.c,v 3.22 1998/03/29 18:32:50 papowell Exp";

#include "lp.h"
#include "jobcontrol.h"
#include "getqueue.h"
#include "decodestatus.h"
#include "dump.h"
#include "errorcodes.h"
#include "fileopen.h"
#include "lockfile.h"
#include "malloclist.h"
#include "pathname.h"
#include "pr_support.h"
#include "setup_filter.h"
#include "sortorder.h"
/**** ENDINCLUDE ****/

/***************************************************************************
 * The job control file has the lines:
 *		<key> value
 *       route <key> value
 * The <key> fields are used to set various entries in the control file,
 *  while the route <key> set routing information.
 * We need to scan and generate this file using control file information.
 ***************************************************************************/

/**********************
 * Keywords
 **********************/
#define HOLD       1
#define PRIORITY   2
#define REMOVE     3
#define SERVEr     4
#define SUBSERVER  5
#define REDIRECT   6
#define ERROR      7
#define DONE       8
#define ROUTE      9
#define DEST       10
#define COPIES     11
#define COPY_DONE  12
#define STATUs     13
#define END        14
#define ROUTED     15
#define ATTEMPT    16
#define IDENT      17
#define RECEIVER   18
#define SEQUENCE   19
#define ACTIVE_TIME 20

static struct keywords status_key[] = {

{ "active_time", INTEGER_K, (void *)0, ACTIVE_TIME },
{ "attempt", INTEGER_K, (void *)0, ATTEMPT },
{ "copies", INTEGER_K, (void *)0, COPIES },
{ "copy_done", INTEGER_K, (void *)0, COPY_DONE },
{ "dest", INTEGER_K, (void *)0, DEST },
{ "done", INTEGER_K, (void *)0, DONE },
{ "end", INTEGER_K, (void *)0, END },
{ "error", STRING_K, (void *)0, ERROR },
{ "hold", INTEGER_K, (void *)0, HOLD },
{ "ident", INTEGER_K, (void *)0, IDENT },
{ "priority", INTEGER_K, (void *)0, PRIORITY },
{ "receiver", INTEGER_K, (void *)0, RECEIVER },
{ "redirect", STRING_K, (void *)0, REDIRECT },
{ "remove", INTEGER_K, (void *)0, REMOVE },
{ "route", INTEGER_K, (void *)0, ROUTE },
{ "routed", INTEGER_K, (void *)0, ROUTED },
{ "sequence", INTEGER_K, (void *)0, SEQUENCE },
{ "server", INTEGER_K, (void *)0, SERVEr },
{ "status", INTEGER_K, (void *)0, STATUs },
{ "subserver", INTEGER_K, (void *)0, SUBSERVER },

{ 0 }
};

#define status_key_len (sizeof( status_key )/sizeof( status_key[0] ))

struct keywords *Find_key( struct keywords *keys, int len, char *key );
static void get_destination( struct control_file *cfp );

/***************************************************************************
 * Lock_hold_file( struct control_file, struct_statb, wait_for_lock )
 *	Open and try to lock the hold file in the spool directory.
 *   If the lock fails and wait_for_lock is set, then wait until lock
 *   succeeds.
 * Returns: fd of (locked) file
 ***************************************************************************/

int Lock_hold_file( struct control_file *cfp, struct stat *statb )
{
	int fd;
	char *hold_file;
	hold_file = cfp->hold_file;
	if( hold_file[0] == 0 ){
		hold_file = Hold_file_pathname( cfp, CDpathname );
	}
	DEBUG3("Lock_hold_file: locking file '%s'", hold_file );
	fd = Lockf( hold_file, statb );
	if( fd < 0 ){
		Errorcode = JABORT;
		logerr_die( LOG_ERR,
			"Lock_hold_file: cannot lock file '%s'",hold_file);
	}
	DEBUG3("Lock_hold_file: locked file '%s', fd %d", hold_file, fd );
	return( fd );
}

/***************************************************************************
 * Get_job_control( struct control_file, int *fd )
 *	Get the job control file from the spool directory and
 *     decode the information in the file
 *  This will read the job control file into a buffer, and then
 *  parse it,  line by line.  Lines which have 'routing' information
 *  will then be parsed to find the various job destinations.
 * Returns: 1 change
 *          0 no change
 ***************************************************************************/

int Get_job_control( struct control_file *cfp, int *fdptr )
{
	struct stat statb;
	long value;
	char *s, *t, *end, *buffer, **list;
	int i, fd, len;
	struct keywords *key;
	char *hold_file;
	char id[LINEBUFFER];

	/* check to see if the file is open already */
	fd = -1;
	hold_file = Hold_file_pathname( cfp, CDpathname );
	if( fdptr ){
		fd = *fdptr;
	}


	DEBUG3("Get_job_control: file '%s', fd %d, Auto_hold %d, Hold_all %d",
		hold_file, fd, Auto_hold, Hold_all);

	memset( &statb, 0, sizeof( statb ) );
	if( fd < 0 ){
		fd = Checkread( hold_file, &statb );
		if( fd < 0 ){
			DEBUG3( "Get_job_control: cannot open file '%s'",
				hold_file);
		}
	}
	if( fdptr ){
		*fdptr = fd;
	}
	/* stat the file */
	if( fd > 0 && fstat( fd, &statb ) < 0 ){
		Errorcode = JABORT;
		logerr_die( LOG_ERR,
			"Get_job_control: cannot fstat fd %d, file '%s'",fd,hold_file);
	}

	/* at this point you have an open (possibly locked) hold file */

	/* update the hold information */
	if( Auto_hold || Hold_all ){
		cfp->hold_info.hold_time = time( (void *)0 );
	}

	/* get the hold file information */
	if( (buffer = (void *)cfp->hold_file_info) ){
		free( buffer );
		buffer = cfp->hold_file_info = 0;
	}

	/* allocate a buffer to hold the file */
	len = statb.st_size;
	buffer = malloc_or_die( len+1 );
	cfp->hold_file_info = buffer;
	DEBUG4("Get_job_control: buffer 0x%x, file len %d", buffer, len );

	/* get the values from the file */
	if( fd > 0 && lseek( fd, 0, SEEK_SET ) < 0 ){
		Errorcode = JABORT;
		logerr_die( LOG_ERR, "Get_job_control: lseek failed" );
	}
	i = 1;
	for( s = buffer;
		len > 0 && (i = read( fd, s, len)) > 0; len -= i, s += i );
	*s = 0;
	if( i < 0 ){
		Errorcode = JABORT;
		logerr_die( LOG_ERR,
			"Get_job_control: error reading hold file '%s'",hold_file);
	}
	/* close the file */
	if( fdptr == 0 ){
		close(fd);
	}

	DEBUG3("Get_job_control: hold file contents '%s'", buffer );

	/* split the lines up */
	/* we have to read the new status, clear the old */
	cfp->hold_file_lines.count = 0;
	if( cfp->hold_file_lines.max == 0 ){
		extend_malloc_list( &cfp->hold_file_lines,
			sizeof( char *), 100,__FILE__,__LINE__  );
	}
	list = cfp->hold_file_lines.list;
	for( s = buffer; s && *s; s = end ){
		end = strchr( s, '\n' );
		if( end ){
			*end++ = 0;
		}
		/* remove leading and trailing white space */
		while( *s && isspace( *s ) ) ++s;
		trunc_str(s);
		/* throw away blank lines */
		if( *s == 0 ) continue;

		if( cfp->hold_file_lines.count+1 >= cfp->hold_file_lines.max ){
			extend_malloc_list( &cfp->hold_file_lines,sizeof(list[0]),100,__FILE__,__LINE__ );
			list = cfp->hold_file_lines.list;
		}
		list[cfp->hold_file_lines.count++] = s;
		list[cfp->hold_file_lines.count] = 0;

		/* find the key */
		safestrncpy( id, s );
		if( (t = strpbrk( id, " \t")) == 0 ){
			Errorcode = JABORT;
			fatal(LOG_ERR,"Get_job_control: long line in hold file '%s'",
				hold_file );
		}
		*t = 0;
		if( (t = strpbrk( s, " \t")) == 0 ){
			Errorcode = JABORT;
			fatal(LOG_ERR,"Get_job_control: long line in hold file '%s'",
				hold_file );
		}
		while( *t && isspace( *t ) ) ++t;

		DEBUG3("Get_job_control: line '%s' id '%s' value='%s'",s,id,t );
		key = Find_key( status_key, status_key_len, id );
		if( key && key->keyword ){
			value = 0;
			if( t && *t ) value = strtol( t, (void *)0, 0 );
			DEBUG4("Get_job_control: found '%s' '%s' value %ld",
				s, t, value );
			switch( key->maxval ){
			case ATTEMPT: cfp->hold_info.attempt = value; break;
			case HOLD: cfp->hold_info.hold_time = value; break;
			case PRIORITY: cfp->hold_info.priority_time = value; break;
			case REMOVE: cfp->hold_info.remove_time = value; break;
			case DONE: cfp->hold_info.done_time = value; break;
			case ROUTED: cfp->hold_info.routed_time = value; break;
			case SERVEr: cfp->hold_info.server = value; break;
			case SUBSERVER: cfp->hold_info.subserver = value; break;
			case ACTIVE_TIME: cfp->hold_info.active_time = value; break;
			case REDIRECT:
				if( t && *t ){
					safestrncpy( cfp->hold_info.redirect, t );
				} else {
					cfp->hold_info.redirect[0] = 0;
				}
				break;
			case ERROR:
				if( t && *t && cfp->error[0] == 0 ){
					safestrncpy( cfp->error, t );
				}
				break;
			case ROUTE:
				if( cfp->destination_info_start == 0 ){
					cfp->destination_info_start = cfp->hold_file_lines.count-1;
				}
				break;
			default: break;
			}
		}
	}
	DEBUG3("Get_job_control: hold 0x%x, priority 0x%x, remove 0x%x",
		cfp->hold_info.hold_time, cfp->hold_info.priority_time,
		cfp->hold_info.remove_time );
	if(DEBUGL3 ){
		logDebug( "Get_job_control: hold_file_lines %d, destination_info_start %d",
			cfp->hold_file_lines.count, cfp->destination_info_start );
		for( i = 0; i < cfp->hold_file_lines.count; ++i ){
			logDebug("  [%d] '%s'", i, list[i] );
		}
	}
	/* now get the destination information */
	cfp->destination_list.count = 0;
	if( cfp->destination_info_start ){
		get_destination( cfp );
	}
	/* get the comparision information */
	DEBUG3("Get_job_control: getting cmp_str");
	safestrncpy( cfp->hold_info.cmp_str, make_cmp_str( cfp ) );
	DEBUG3("Get_job_control: cmp_str '%s'", cfp->hold_info.cmp_str );
	if(DEBUGL3 ) dump_control_file( "Get_job_control - return value", cfp );
	return( 1 );
}

/***************************************************************************
 * Find_key() - search the keywords entry for the key value
 *  - we handle tables where we have a 0 terminating value as well
 ***************************************************************************/
struct keywords *Find_key( struct keywords *keys, int len, char *key )
{
	int top, bottom, mid, compare;

	bottom = 0;
	top = len-1;
	/* we skip top one if it has a terminal 0 entry */
	if( keys[top].keyword == 0 ) --top;
	DEBUG4("Find_key: find '%s'", key );
	while( top >= bottom ){
		mid = (top+bottom)/2;
		compare = strcasecmp( keys[mid].keyword, key );
		/* DEBUG4("Find_key: top %d, bottom %d, mid='%s'",
			top, bottom, keys[mid].keyword ); */
		if( compare == 0 ){
			DEBUG4("Find_key: key '%s', found '%s'",
			key, keys[mid].keyword );
			return( &keys[mid] );
		} else if( compare > 0 ){
			top = mid - 1;
		} else {
			bottom = mid + 1;
		}
	}
	return( 0 );
}


/***************************************************************************
 * get_destination()
 *  get the destination routing information from the control file
 *  hold_file_lines fields.  This information has the form
 *  route <key> value.  The various values are used to set entries in
 *  the destination_list fields.
 ***************************************************************************/

static void get_destination( struct control_file *cfp )
{
	struct destination *destinationp, *d;
	int i, dest_start;
	char *s, *t;
	struct keywords *key;
	char id[LINEBUFFER];

	/* clear out the destination list */
	cfp->destination_list.count = 0;

	/* extend the list if necessary */
	if( cfp->destination_list.max <= cfp->hold_file_lines.count ){
		extend_malloc_list( &cfp->destination_list,
			sizeof( struct destination), 
			cfp->hold_file_lines.count - cfp->destination_list.max + 1,
			__FILE__,__LINE__  );
	}
	destinationp = (void *)cfp->destination_list.list;
	d = &destinationp[cfp->destination_list.count];
	memset( (void *)d, 0, sizeof( d[0] ) );
	dest_start = cfp->destination_info_start;
	for( i = dest_start;
		i < cfp->hold_file_lines.count; ++i ){
		s = cfp->hold_file_lines.list[i];
		/* DEBUG3("get_destination: checking '%s'", s ); */
		/* skip over the 'route' keyword */
		if( strncmp(s, "route", 5 ) == 0 ) s += 5;
		while( isspace( *s ) ) ++s;
		if( isupper( *s ) ){
			/* do not need to worry about lines starting with upper case */
			continue;
		}
		t = strpbrk( s, " \t");
		if( t == 0 ){
			t = s + strlen(s);
		}
		safestrncpy( id, s );
		id[ t - s ] = 0;
		while( isspace( *t ) ) ++t;
		/*DEBUG3("get_destination: checking '%s'='%s'", id, t );*/
		key = Find_key( status_key, status_key_len, id );
		if( key ){
			/*DEBUG3("get_destination: found key '%s'='%s'", key->keyword, t );*/
			switch( key->maxval ){
			default: break;
			case ROUTE: break;
			case IDENT: 
				safestrncpy( d->identifier,"A");
				safestrncat( d->identifier,t);
				break;
			case DEST: safestrncpy( d->destination,t); break;
			case ERROR: safestrncpy( d->error,t); break;
			case PRIORITY: if( isupper(t[0]) ) d->priority = t[0]; break;
			case COPIES: d->copies = atoi( t ); break;
			case COPY_DONE: d->copy_done = atoi( t ); break;
			case STATUs: d->status = atoi( t ); break;
			case SUBSERVER: d->subserver = atoi( t ); break;
			case DONE: d->done_time = atoi( t ); break;
			case HOLD: d->hold_time = atoi( t ); break;
			case ATTEMPT: d->attempt = atoi( t ); break;
			case SEQUENCE: d->sequence_number = atoi( t ); break;
			case END:
				/* watch out for the no destination */
				d->arg_start = dest_start;
				d->arg_count = i - dest_start;
				if( d->destination[0]
					&& ++cfp->destination_list.count >= cfp->destination_list.max ){
					extend_malloc_list( &cfp->destination_list,
						sizeof( struct destination), 5,__FILE__,__LINE__  );
				}
				destinationp = (void *)cfp->destination_list.list;
				d = &destinationp[cfp->destination_list.count];
				memset( (void *)d, 0, sizeof( d[0] ) );
				dest_start = i+1;
				break;
			}
		}
	}
	/* no end  - clear it up */
	d->arg_start = dest_start;
	d->arg_count = i - dest_start;
	if( d->destination[0] ){
		++cfp->destination_list.count;
	}
	for( i = 0; i < cfp->destination_list.count; ++i ){
		int len;
		d = &destinationp[i];
		if( d->identifier[0] == 0 ){
			safestrncpy( d->identifier, cfp->identifier );
			len = strlen( d->identifier );
			plp_snprintf( d->identifier+len,
				sizeof(d->identifier)-len, ".%d", i+1 );
		}
	}
}

static void write_line( int fd, char *buffer, char *file )
{
	if( Write_fd_str( fd, buffer ) < 0
		|| Write_fd_str( fd, "\n" ) < 0 ){
		Errorcode = JABORT;
		logerr_die( LOG_ERR,
			"write_line: cannot write file '%s'",file);
	}
}

static void write_route_str( int fd, char *header, char *str, char *file )
{
	char buffer[SMALLBUFFER];
	plp_snprintf( buffer, sizeof(buffer)-2, "route %s %s", 
		header, str );
	DEBUG4("write_route_str: dest line '%s'", buffer );
	write_line( fd, buffer, file );
}

static void write_route_int( int fd, char *header, int val, char *file )
{
	char buffer[SMALLBUFFER];
	plp_snprintf( buffer, sizeof(buffer)-2, "route %s %d", 
		header, val );
	DEBUG4("write_route_int: dest line '%s'", buffer );
	write_line( fd, buffer, file );
}


static void write_route_char( int fd, char *header, int val, char *file )
{
	char buffer[SMALLBUFFER];
	plp_snprintf( buffer, sizeof(buffer)-2, "route %s %c", 
		header, val );
	DEBUG4("write_route_char: dest line '%s'", buffer );
	write_line( fd, buffer, file );
}



/***************************************************************************
 * char *Fix_error_time( str, len )
 *  Fix up the string so that it ends up with , at time
 ***************************************************************************/

char *Fix_error_time( char *str, int len )
{
	int n;
	if( str[0] && !strstr(str,", at ") ){
		for( n = strlen( str ); --n >= 0 && isspace(str[n]); str[n] = 0 );
		n = strlen( str );
		plp_snprintf( str+n, len-n, ", at %s", Time_str( 1, 0) );
	}
	return( str );
}
/***************************************************************************
 * Set_job_control( struct control_file, int *fd )
 *	Lock the job control file from the spool directory and
 *     write new information.
 * Returns: 0
 ***************************************************************************/


int Set_job_control( struct control_file *cfp, int *fdptr )
{
	struct stat statb;
	char buffer[SMALLBUFFER];
	char *s, *t, *hold_file;
	char **lines;
	int i, j, value, fd;
	struct destination *destination, *d;
	char temp_hold_file[MAXPATHLEN];

	hold_file = Hold_file_pathname( cfp, CDpathname );
	safestrncpy( temp_hold_file, hold_file );
	s = strrchr( temp_hold_file, '/' );
	if( s == 0 || s[1] == 0 ){
		Errorcode = JABORT;
		fatal( LOG_ERR,
			"Set_job_control: bad name format hold file '%s'",temp_hold_file);
	}
	s[1] = '_';
	DEBUG2("Set_job_control: hold file '%s', temp '%s'",
		hold_file, temp_hold_file );

	/* we try to get exclusive access to the temp file */
	fd = Lockf(temp_hold_file, &statb );
	DEBUG2("Set_job_control: locked '%s', fd %d", temp_hold_file, fd );

	if( ftruncate( fd, 0 ) < 0 ){
		Errorcode = JABORT;
		logerr_die( LOG_ERR,
			"Set_job_control: cannot truncate temp hold file '%s'",
				temp_hold_file);
	}
	value = 0;
	for( i = 0; status_key[i].keyword ; ++i ){
		value = 0;
		t = 0;
		switch( status_key[i].maxval ){
		case HOLD:		value = cfp->hold_info.hold_time; break;
		case PRIORITY:	value = cfp->hold_info.priority_time; break;
		case REMOVE:	value = cfp->hold_info.remove_time; break;
		case SERVEr:	value = cfp->hold_info.server; break;
		case SUBSERVER:	value = cfp->hold_info.subserver; break;
		case ACTIVE_TIME:	value = cfp->hold_info.active_time; break;
		case DONE:		value = cfp->hold_info.done_time; break;
		case ROUTED:	value = cfp->hold_info.routed_time; break;
		case ATTEMPT:	value = cfp->hold_info.attempt; break;
		case REDIRECT:	t = cfp->hold_info.redirect; break;
		case ERROR:		t = Fix_error_time(cfp->error,sizeof(cfp->error)); break;
		default: continue;
		}
		buffer[0] = 0;
		if( t == 0 ){
			plp_snprintf( buffer, sizeof(buffer)-2, "%s %d",
				status_key[i].keyword, value );
		} else if( *t ){
			plp_snprintf( buffer, sizeof(buffer)-2, "%s %s",
				status_key[i].keyword, t );
		}
		if( buffer[0] ){
			DEBUG4("Set_job_control: '%s'", buffer );
			write_line( fd, buffer, temp_hold_file );
		}
	}
	if( cfp->hold_info.routed_time && cfp->destination_list.count > 0 ){
		destination = (void *)cfp->destination_list.list;
		for( i = 0; i < cfp->destination_list.count; ++i ){
			d = &destination[i];
			if( d->destination[0] == 0 ) continue;
			write_route_str( fd, "dest", d->destination, temp_hold_file );
			write_route_str( fd, "ident", d->identifier+1, temp_hold_file );
			write_route_str( fd, "error",
				Fix_error_time(d->error,sizeof(d->error)), temp_hold_file );
			write_route_int( fd, "copies", d->copies, temp_hold_file );
			write_route_int( fd, "copy_done", d->copy_done, temp_hold_file );
			write_route_int( fd, "status", d->status, temp_hold_file );
			write_route_int( fd, "subserver", d->subserver, temp_hold_file );
			write_route_int( fd, "attempt", d->attempt, temp_hold_file );
			write_route_int( fd, "done", d->done_time, temp_hold_file );
			write_route_int( fd, "hold", d->hold_time, temp_hold_file );
			write_route_int( fd, "sequence", d->sequence_number, temp_hold_file );
			if( d->priority){
				write_route_char( fd, "priority", d->priority, temp_hold_file );
			}
			lines = &cfp->hold_file_lines.list[d->arg_start];
			for( j = 0; j < d->arg_count; ++j ){
				if( (s = lines[j])[0] ){
					if( strncmp( s, "route", 5 ) == 0 ){
						s += 5;
					}
					while( isspace( *s ) ) ++s;
					if( isupper( *s ) ){
						plp_snprintf( buffer, sizeof(buffer)-2, "route %s", s );
						DEBUG4("Set_job_control: dest line '%s'", buffer );
						write_line( fd, buffer, temp_hold_file );
					}
				}
			}
			write_route_str( fd, "end", "", temp_hold_file );
		}
	}
	/* we now do the switcheroo - renaming the files */
	if( rename( temp_hold_file, hold_file ) < 0 ){
		Errorcode = JABORT;
		logerr_die(LOG_ERR, "Set_job_control: rename '%s' to '%s' failed",
			temp_hold_file, hold_file );
	}
	if( fdptr ){
		close( *fdptr );
		*fdptr = fd;
	} else {
		close( fd );
	}
	/* reread the control file */
	Get_job_control( cfp, fdptr );
	return( 0 );
}

/***************************************************************************
 * char *Hold_file_pathname( struct control_file *cfp )
 *  get the hold file name for the job and put it in
 *  the control file.
 ***************************************************************************/
char *Hold_file_pathname( struct control_file *cfp, struct dpathname *dpath )
{
	int len;
	/*
	 * get the hold file pathname
	 */
	safestrncpy( cfp->hold_file, Add_path( dpath, "hfA") );
	len = strlen(cfp->hold_file);
	plp_snprintf( cfp->hold_file+len, sizeof(cfp->hold_file)-len, "%0*d",
		cfp->number_len, cfp->number );
	DEBUG4("Hold_file_pathname: '%s'", cfp->hold_file );
	return( cfp->hold_file );
}

/***************************************************************************
 * Get_route( struct control_file, int fd )
 *	Get routing information from the routing filter
 *  This will have the format:
 *  <key> value
 *   where key is one of the routing keys
 *
 * char *command     - filter command
 * cfp               - control file data
 * fd                - control file file descriptor
 * pc_entry          - printcap
 * hold_fd           - hold file file descriptor
 *
 * Note: the output of the routing filter is appended to the
 *  control file,  which is then reparsed.
 *
 ***************************************************************************/

int Get_route( char *command,
	struct control_file *cfp, int fd, struct printcap_entry *pc_entry,
	int *hold_fd )
{
	int err, sequence, i, temp_fd, len;
	struct destination *destinationp, *d;
	char *buffer = 0, *s, *end;
	struct stat statb;

	DEBUG3("Get_route: '%s'", command );

	/* we save the hold file information */
	Set_job_control( cfp, hold_fd );

	/* position files */
	if( lseek( fd, 0, SEEK_SET ) < 0 ){
		Errorcode = JFAIL;
		logerr_die( LOG_INFO, "Get_route: lseek failed" );
	}
 	temp_fd = Make_temp_fd( 0, 0 );

	/* filter them */
	err = Make_filter( 'f', cfp, &As_fd_info, command, 0, 0,
		temp_fd, pc_entry, (void *)0, 0, 0, fd );
	if( err ){
		logerr_die( LOG_INFO, "Get_route: cannot create route_filter '%s'",
			command );
	}
	err = Close_filter( cfp, &As_fd_info, 0, "router" );

	DEBUG3("Get_route: filter exit status %s", Server_status(err) );

	if( lseek( temp_fd, 0, SEEK_SET ) < 0 ){
		Errorcode = JFAIL;
		logerr_die( LOG_INFO, "Get_route: lseek temp_fd %d failed", temp_fd );
	}
	if( fstat( temp_fd, &statb ) < 0 ){
		Errorcode = JABORT;
		logerr_die( LOG_ERR,
			"Get_route: cannot fstat fd %d",temp_fd);
	}
	len = statb.st_size;
	buffer = malloc_or_die( len+1 );
	for( s = buffer;
		len > 0 && (i = read( temp_fd, s, len)) > 0; len -= i, s += i );
	*s = 0;
	if( lseek( *hold_fd, 0, SEEK_END ) < 0 ){
		Errorcode = JFAIL;
		logerr_die( LOG_INFO, "Get_route: lseek hold_fd %d failed", *hold_fd );
	}
	for( s = buffer; s && *s; s = end ){
		end = strchr( s, '\n' );
		if( end ) *end++ = 0;
		if( *s &&
			(Write_fd_str( *hold_fd, "route " ) < 0
				|| Write_fd_str( *hold_fd, s ) < 0
				|| Write_fd_str( *hold_fd, "\n" ) < 0 ) ){
				Errorcode = JABORT;
				logerr_die( LOG_ERR,
					"Get_route: write error to %d",*hold_fd);
		}
	}

	Get_job_control( cfp, hold_fd );
	if( cfp->destination_list.count > 0 ){
		sequence = 1;
		cfp->hold_info.routed_time = time( (void *)0 );
		destinationp = (void *)cfp->destination_list.list;
		for( i = 0; i < cfp->destination_list.count; ++i ){
			d = &destinationp[i];
			d->sequence_number = sequence;
			if( d->copies > 0 ){
				sequence += d->copies;
			} else {
				++sequence;
			}
		}
	}

	if(DEBUGL3 ) dump_control_file( "Get_route- return value", cfp );
	DEBUG0("Get_route: returning '%d'", err );
	if( buffer ) free(buffer);
	buffer = 0;
	return( err );
}


/***************************************************************************
 * char *Copy_hf( struct control_file *cf, char *header )
 *  Make a copy of the status file with the header
 *  and return a pointer *  to it.
 *  If no_header is nonzero, do not put the header on.
 * Note that successive calls to an unchanged control file will
 *  not destroy the pointer validity.
 ***************************************************************************/

char *Copy_hf( struct malloc_list *data, struct malloc_list *copy,
	char *header, char *prefix )
{
	char **lines, *s;
	char *buffer = 0;
	int buffer_len;
	int i, len, prefix_len;

	DEBUG3("Copy_hf: data 0x%x, count %d, copy 0x%x, header '%s', prefix '%s'",
		data, data?data->count:0, copy, header, prefix );

	if( data->count ){
		if( prefix == 0 ) prefix = "";
		prefix_len = strlen( prefix );

		buffer = (void *)copy->list;
		buffer_len = copy->max;

		lines = data->list;
		len = 0;
		if( header && *header ) len += strlen( header ) + 2;
		for( i = 0; i < data->count; ++i ){
			if( (s = lines[i]) && *s ){
				len += strlen(s) + 1 + prefix_len;
			}
		}
		++len;
		if( len > buffer_len ){
			if( buffer ){
				free( buffer );
			}
			/* now we allocate a buffer */
			buffer = malloc_or_die( len );
			buffer_len = len;
			copy->max = len;
			copy->list = (void *)buffer;
		}
		buffer[0] = 0;
		if( header && *header ){
			plp_snprintf( buffer, buffer_len, "%s\n", header );
		}
		for( i = 0; i < data->count; ++i ){
			if( (s = lines[i]) && *s ){
				len = strlen( buffer );
				plp_snprintf( buffer+len, buffer_len - len,
					"%s%s\n", prefix, s );
			}
		}
	}
	return(buffer);
}

/***************************************************************************
 * int Find_non_colliding_job_number( struct control_file *cfp )
 *  Find a non-colliding job number for the new job
 * RETURNS: 0 if successful
 *          ack value if unsuccessful
 * Side effects: sets up control file fields;
 ***************************************************************************/
int Find_non_colliding_job_number( struct control_file *cfp,
	struct dpathname *dpath )
{
	int encountered = 0;		/* wrap around guard for job numbers */
	int hold_fd = -1;			/* job hold file fd */
	struct stat statb;			/* for status */

	/* we set the job number to a reasonable range */
	Fix_job_number( cfp );

	hold_fd = -1;

	DEBUGF(DRECV2)("Find_non_colliding_job_number: job_number %d, max %d",
		cfp->number, cfp->max_number  );
	/* we now try each of these in order */
	/* now check to see if there is a job number collision */

	if( cfp->priority == 0 ){
		cfp->priority = 'A';
	}
	while( hold_fd < 0 ){
		/* now we lock the hold file for the job */
		Hold_file_pathname( cfp, dpath );
		DEBUGF(DRECV2)("Find_non_colliding_job_number: trying %s",
			cfp->hold_file );
		hold_fd = Checkwrite(cfp->hold_file, &statb,
			O_RDWR|O_CREAT|O_EXCL, 0, 0 );
		/* if the hold file locked or is non-zero, we skip to a new one */
		if( hold_fd > 0 && Do_lock( hold_fd, cfp->hold_file, 0 ) < 0 ){
			close( hold_fd );
			hold_fd = -1;
		}
		if( hold_fd < 0 ){
			++cfp->number;
			if( cfp->number >= cfp->max_number ){
				cfp->number = 0;
				if( encountered++ ){
					DEBUGF(DRECV2)("Find_non_colliding_job_number: No space" );
					plp_snprintf( cfp->error, sizeof(cfp->error),
						_("%s: queue full - no space"), Printer );
					return( -1 );
				}
			}
		}
	}
	DEBUGF(DRECV2)("Find_non_colliding_job_number: using %s", cfp->hold_file );
	return( hold_fd );
}