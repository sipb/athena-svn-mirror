/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************/

 static char *const _id =
"$Id: permission.c,v 1.1.1.3.2.1 2001-03-07 01:42:02 ghudson Exp $";


#include "lp.h"
#include "fileopen.h"
#include "globmatch.h"
#include "gethostinfo.h"
#include "getqueue.h"
#include "permission.h"
#include "linksupport.h"

/**** ENDINCLUDE ****/

 struct keywords permwords[] = {
{"REJECT", P_REJECT},
{"NOMATCHFOUND", 0},
{"ACCEPT", P_ACCEPT},
{"NOT", P_NOT},
{"SERVICE", P_SERVICE},
{"USER", P_USER},
{"HOST", P_HOST},
{"IP", P_IP},
{"PORT", P_PORT},
{"REMOTEHOST", P_REMOTEHOST},
{"REMOTEIP", P_REMOTEIP},
{"PRINTER", P_PRINTER},
{"DEFAULT", P_DEFAULT},
{"FORWARD", P_FORWARD},
{"SAMEUSER", P_SAMEUSER},
{"SAMEHOST", P_SAMEHOST},
{"CONTROLLINE", P_CONTROLLINE},
{"GROUP", P_GROUP},
{"SERVER", P_SERVER},
{"REMOTEUSER", P_REMOTEUSER},
{"REMOTEGROUP", P_REMOTEGROUP},
{"AUTH", P_AUTH},
{"AUTHTYPE", P_AUTHTYPE},
{"AUTHUSER", P_AUTHUSER},
{"AUTHFROM", P_AUTHFROM},
{"IFIP", P_IFIP},
{"LPC", P_LPC},
{"AUTHSAMEUSER", P_AUTHSAMEUSER},
{"AUTHJOB", P_AUTHJOB},
{0}
};

char *perm_str( int n )
{
	return(Get_keystr(n,permwords));
}
int perm_val( char *s )
{
	return(Get_keyval(s,permwords));
}

/***************************************************************************
 * Perms_check( struct line_list *perms, struct perm_check );
 * - run down the list of permissions
 * - do the check on each of them
 * - if you get a distinct fail or success, return
 * 1. the P_NOT field inverts the result of the next test
 * 2. if one test fails,  then we go to the next line
 * 3. The entire set of tests is accepted if all pass, i.e. none fail
 ***************************************************************************/

int Perms_check( struct line_list *perms, struct perm_check *check,
	struct job *job )
{
	int j, c, linecount, valuecount, key;
	int invert = 0;
	int result = 0, m = 0;
	struct line_list values, args;
	char *s, *t;					/* string */
	int last_default_perm;

	DEBUGFC(DDB1)Dump_perm_check( "Perms_check - checking", check );
	DEBUGFC(DDB1)Dump_line_list( "Perms_check - permissions", perms );
	Init_line_list(&values);
	Init_line_list(&args);
	last_default_perm = perm_val( Default_permission_DYN );
	DEBUGF(DDB1)("Perms_check: last_default_perm '%s', Default_perm '%s'",
		perm_str( last_default_perm ), Default_permission_DYN );
	if( check == 0 || perms == 0 ){
		return( last_default_perm );
	}
	for( linecount = 0; result == 0 && linecount < perms->count; ++linecount ){
		DEBUGF(DDB2)("Perms_check: line [%d]='%s'", linecount,
			perms->list[linecount]);
		Free_line_list(&values);
		Split(&values,perms->list[linecount],Whitespace,0,0,0,0,0);
		if( values.count == 0 ) continue;
		result = 0; m = 0; invert = 0;
		for( valuecount = 0; m == 0 && valuecount < values.count;
				++valuecount ){
			DEBUGF(DDB2)("Perms_check: [%d]='%s'",valuecount,
				values.list[valuecount] );
			Free_line_list(&args);
			Split(&args,values.list[valuecount],Perm_sep,0,0,0,0,0);
			if( args.count == 0 ) continue;
			if( invert > 0 ){
				invert = -1;
			} else {
				invert = 0;
			}
			key = perm_val( args.list[0] );
			if( key == 0 ){
				m = 1;
				break;
			}
			/* we remove the key entry */
			Remove_line_list( &args, 0 );
			DEBUGF(DDB2)("Perms_check: before doing %s, result %d, %s",
				perm_str(key), result, perm_str(result) );
			switch( key ){
			case P_NOT:
				invert = 1;
				continue;
			case P_REJECT: result = P_REJECT; m = 0; break;
			case P_ACCEPT: result = P_ACCEPT; m = 0; break;
			case P_USER:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					m = match( &args, check->user, invert );
					break;
				}
				break;
			case P_LPC:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					m = match( &args, check->lpc, invert );
					break;
				}
				break;
			case P_HOST:
				m = 1;
				switch (check->service){
				case 'X':
					m = match_host( &args, check->remotehost, invert );
					break;
				default:
					m = match_host( &args, check->host, invert );
					break;
				}
				break;
			case P_GROUP:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					m = match_group( &args, check->user, invert );
					break;
				}
				break;
			case P_IP:
				m = 1;
				switch (check->service){
				case 'X':
					m = match_host( &args, check->remotehost, invert );
					break;
				default:
					m = match_host( &args, check->host, invert );
					break;
				}
				break;

			case P_IFIP:
				m = 1;
				Get_hostinfo_byaddr( &LookupHost_IP, check->addr, 1);
				m = match_host( &args, &LookupHost_IP, invert );
				break;
			case P_PORT:
				m = 1;
				switch (check->service){
				case 'X': case 'M': case 'C': case 'S':
					m = match_range( &args, check->port, invert );
					break;
				}
				break;
			case P_REMOTEUSER:
				m = 1;
				switch (check->service){
				case 'X': break;
				case 'M': case 'C': case 'S':
					m = match( &args, check->remoteuser, invert );
					break;
				default:
					m = match( &args, check->user, invert );
					break;
				}
				break;
			case P_REMOTEGROUP:
				m = 1;
				switch (check->service){
				case 'X': break;
				case 'M': case 'C': case 'S':
					m = match_group( &args, check->remoteuser, invert );
					break;
				default:
					m = match_group( &args, check->user, invert );
					break;
				}
				break;
			case P_REMOTEHOST:
				m = 1;
				switch (check->service){
				default:
					m = match_host( &args, check->remotehost, invert );
					break;
				}
				break;
			case P_AUTH:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					DEBUGF(DDB3)(
						"Perms_check: P_AUTH auth_client_id '%s'", check->auth_client_id );
					m = !check->auth_client_id;
					if( invert ) m = !m;
				}
				break;
			case P_AUTHTYPE:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					DEBUGF(DDB3)(
						"Perms_check: P_AUTHTYPE authtype '%s'", check->authtype );
					m = match( &args, check->authtype, invert );
				}
				break;
			case P_AUTHFROM:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					m = match( &args, check->auth_from_id, invert );
				}
				break;
			case P_AUTHUSER:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					m = match( &args, check->auth_client_id, invert );
				}
				break;
			case P_REMOTEIP:
				m = 1;
				switch( check->service ){
				default:
					m = match_host( &args, check->remotehost, invert );
					break;
				}
				break;
			case P_CONTROLLINE:
				/* check to see if we have control line */
				m = 1;
				if( job && args.count){
					for(m = j = 0; m == 0 && j < args.count; ++j ){
						if( !(t = args.list[j]) ) continue;
						c = cval(t);
						if( isupper(c) &&
							(s = Find_first_letter(&job->controlfile,c,0))){
							m = Globmatch( t, s );
						}
					}
					if( invert ) m = !m;
				}
				break;
			case P_PRINTER:
				m = 1;
				switch (check->service){
				case 'X': break;
				default:
					m = match( &args, check->printer, invert );
					break;
				}
				break;
			case P_SERVICE:
				m = match_char( &args, check->service, invert );
				break;
			case P_FORWARD:
				m = 1;
				switch (check->service){
				default: break;
				case 'R': case 'Q': case 'M': case 'C': case 'S':
					/* P_FORWARD check succeeds if P_REMOTEIP != P_IP */
					m = !Same_host( check->host, check->remotehost );
					if( invert ) m = !m;
					break;
				}
				break;
			case P_SAMEHOST:
				m = 1;
				switch (check->service){
				default: break;
				case 'R': case 'Q': case 'M': case 'C': case 'S':
					/* P_SAMEHOST check succeeds if P_REMOTEIP == P_IP */
					m = Same_host(check->host, check->remotehost);
					if( m ){
					/* check to see if both remote and local are server */
					int r, h;
					r = Same_host(check->remotehost,&Host_IP);
					if( r ) r = Same_host(check->remotehost,&Localhost_IP);
					h = Same_host(check->host,&Host_IP);
					if( h ) h = Same_host(check->host,&Localhost_IP);
					DEBUGF(DDB3)(
						"Perms_check: P_SAMEHOST server name check r=%d,h=%d",
						r, h );
					if( h == 0 && r == 0 ){
						m = 0;
					}
					}
					if( invert ) m = !m;
					break;
				}
				break;
			case P_SAMEUSER:
				m = 1;
				switch (check->service){
				default: break;
				case 'Q': case 'M': case 'C': case 'S':
					/* check succeeds if remoteuser == user */
					m = (safestrcmp( check->user, check->remoteuser ) != 0);
					if( invert ) m = !m;
					DEBUGF(DDB3)(
					"Perms_check: P_SAMEUSER '%s' == remote '%s', rslt %d",
					check->user, check->remoteuser, m );
					break;
				}
				break;

			case P_AUTHSAMEUSER:
				m = 1;
				switch (check->service){
				default: break;
				case 'Q': case 'M': case 'C': case 'S':
					/* check succeeds if remoteuser == user */
					t = Find_str_value(&job->info,AUTHINFO,Value_sep);
					m = (safestrcmp( check->auth_client_id, t ) != 0);
					if( invert ) m = !m;
					DEBUGF(DDB3)(
					"Perms_check: P_AUTHSAMEUSER job authinfo '%s' == auth_id '%s', rslt %d",
					t, check->auth_client_id, m );
					break;
				}
				break;

			case P_AUTHJOB:
				m = 1;
				switch (check->service){
				default: break;
				case 'Q': case 'M': case 'C': case 'S':
					/* check succeeds if remoteuser == user */
					t = Find_str_value(&job->info,AUTHINFO,Value_sep);
					m = !t;
					if( invert ) m = !m;
					DEBUGF(DDB3)(
					"Perms_check: P_AUTHJOB job authinfo '%s', rslt %d", t, m );
					break;
				}
				break;

			case P_SERVER:
				m = 1;
				/* check succeeds if remote P_IP and server P_IP == P_IP */
				m = Same_host(check->remotehost,&Host_IP);
				if( m ) m = Same_host(check->remotehost,&Localhost_IP);
				if( invert ) m = !m;
				break;

			case P_DEFAULT:
				
				DEBUGF(DDB3)("Perms_check: DEFAULT - %d, values.count %d",
					valuecount, values.count );
				m = 1;
				if( values.count == 2 ){
					switch( perm_val( values.list[1]) ){
					case P_REJECT: last_default_perm =  P_REJECT; break;
					case P_ACCEPT: last_default_perm =  P_ACCEPT; break;
					}
				}
			}
			DEBUGF(DDB3)("Perms_check: match %d, result '%s' default now '%s'",
				m, perm_str(result), perm_str(last_default_perm) );
		}
		if( m ){
			result = 0;
		} else if( result == 0 ){
			result = last_default_perm;
		}
		DEBUGF(DDB3)("Perms_check: final match %d, result '%s' default now '%s'",
			m, perm_str(result), perm_str(last_default_perm) );
		DEBUGF(DDB2)("Perms_check: result %d '%s'",
					result, perm_str( result ) );
	}
	if( result == 0 ){
		result = last_default_perm;
	}
	DEBUGF(DDB2)("Perms_check: final result %d '%s'",
				result, perm_str( result ) );
	Free_line_list(&values);
	Free_line_list(&args);
	return( result );
}


/***************************************************************************
 * static int match( char **val, char *str );
 *  returns 1 on failure, 0 on success
 *  - match the string against the list of options
 *    options are glob type regular expressions;  we implement this
 *    currently using the most crude of pattern matching
 *  - if string is null or pattern list is null, then match fails
 *    if both are null, then match succeeds
 ***************************************************************************/

int match( struct line_list *list, const char *str, int invert )
{
 	int result = 1, i, c;
	char *s;
 	DEBUGF(DDB3)("match: str '%s', invert %d", str, invert );
 	if(str)for( i = 0; result && i < list->count; ++i ){
		if( !(s = list->list[i])) continue;
		DEBUGF(DDB3)("match: str '%s' to '%s'", str, s );
 		/* now do the match */
		c = cval(s);
		if( c == '@' ) {	/* look up host in netgroup */
#ifdef HAVE_INNETGR
			result = !innetgr( s+1, (char *)str, 0, 0 );
#else /* HAVE_INNETGR */
			DEBUGF(DDB3)("match: no innetgr() call, netgroups not permitted");
#endif /* HAVE_INNETGR */
		} else if( c == '<' && cval(s+1) == '/' ){
			struct line_list users;
			Init_line_list(&users);
			Get_file_image_and_split(0,s+1,0,0,&users,Whitespace,
				0,0,0,0,0);
			DEBUGFC(DDB3)Dump_line_list("match- file contents'", &users );
			result = match( &users,str,0);
			Free_line_list(&users);
		} else {
	 		result = Globmatch( s, str );
		}
		DEBUGF(DDB3)("match: list[%d]='%s', result %d", i, s,  result );
	}
	if( invert ) result = !result;
 	DEBUGF(DDB3)("match: str '%s' final result %d", str, result );
	return( result );
}

/***************************************************************************
 * static int match_host( char **list, char *host );
 *  returns 1 on failure, 0 on success
 *  - match the hostname/printer strings against the list of options
 *    options are glob type regular expressions;  we implement this
 *    currently using the most crude of pattern matching
 *  - if string is null or pattern list is null, then match fails
 *    if both are null, then match succeeds
 ***************************************************************************/

int match_host( struct line_list *list, struct host_information *host,
	int invert )
{
 	int result = Match_ipaddr_value(list,host);
	if( invert ) result = !result;
 	DEBUGF(DDB3)("match_host: host '%s' final result %d", host?host->fqdn:0,
		result );
	return( result );
}
/***************************************************************************
 * static int match_range( char **list, int port );
 * check the port number and/or range
 * entry has the format:  number     number-number
 ***************************************************************************/

int portmatch( char *val, int port )
{
	int low, high, err;
	char *end;
	int result = 1;
	char *s, *t, *tend;

	err = 0;
	s = strchr( val, '-' );
	if( s ){
		*s = 0;
	}
	end = val;
	low = strtol( val, &end, 10 );
	if( end == val || *end ) err = 1;

	high = low;
	if( s ){
		tend = t = s+1;
		high = strtol( t, &tend, 10 );
		if( t == tend || *tend ) err = 1;
		*s = '-';
	}
	if( err ){
		logmsg( LOG_ERR, "portmatch: bad port range '%s'", val );
	}
	if( high < low ){
		err = high;
		high = low;
		low = err;
	}
	result = !( port >= low && port <= high );
	DEBUGF(DDB3)("portmatch: low %d, high %d, port %d, result %d",
		low, high, port, result );
	return( result );
}

int match_range( struct line_list *list, int port, int invert )
{
	int result = 1;
	int i;
	char *s;

	DEBUGF(DDB3)("match_range: port '0x%x'", port );
	for( i = 0; result && i < list->count; ++i ){
		/* now do the match */
		if( !(s = list->list[i]) ) continue;
		result = portmatch( s, port );
	}
	if( invert ) result = !result;
	DEBUGF(DDB3)("match_range: port '%d' result %d", port, result );
	return( result );
}

/***************************************************************************
 * static int match_char( char **list, int value );
 * check for the character value in one of the option strings
 * entry has the format:  string
 ***************************************************************************/

int match_char( struct line_list *list, int value, int invert )
{
	int result = 1;
	int i;
	char *s;

	DEBUGF(DDB3)("match_char: value '0x%x' '%c'", value, value );
	DEBUGFC(DDB3)Dump_line_list("match_char - lines", list );
	for( i = 0; result && i < list->count; ++i ){
		if( !(s = list->list[i]) ) continue;
		result = (strchr( s, value ) == 0 );
		DEBUGF(DDB3)("match_char: val %c, str '%s', match %d",
			value, s, result);
	}
	if( invert ) result = !result;
	DEBUGF(DDB3)("match_char: value '%c' result %d", value, result );
	return( result );
}


/***************************************************************************
 * static int match_group( char **list, char *str );
 *  returns 1 on failure, 0 on success
 *  - get the UID for the named user
 *  - scan the listed groups to see if there is a group
 *    check to see if user is in group
 ***************************************************************************/

int match_group( struct line_list *list, const char *str, int invert )
{
 	int result = 1;
 	int i;
	char *s;

 	DEBUGF(DDB3)("match_group: str '%s'", str );
 	for( i = 0; str && result && i < list->count; ++i ){
 		/* now do the match */
		if( !(s = list->list[i]) ) continue;
 		result = ingroup( s, str );
	}
	if( invert ) result = !result;
 	DEBUGF(DDB3)("match: str '%s' value %d", str, result );
	return( result );
}

/***************************************************************************
 * static int ingroup( char* *group, char *user );
 *  returns 1 on failure, 0 on success
 *  scan group for user name
 * Note: we first check for the group.  If there is none, we check for
 *  wildcard (*) in group name, and then scan only if we need to
 ***************************************************************************/

int ingroup( char *group, const char *user )
{
	struct group *grent;
	struct passwd *pwent;
	char **members;
	int result = 1;

	DEBUGF(DDB3)("ingroup: checking '%s' for membership in group '%s'", user, group);
	if( group == 0 || user == 0 ){
		return( result );
	}
	/* first try getgrnam, see if it is a group */
	pwent = getpwnam(user);
	if( group[0] == '@' ) {	/* look up user in netgroup */
#ifdef HAVE_INNETGR
		if( !innetgr( group+1, 0, (char *)user, 0 ) ) {
			DEBUGF(DDB3)( "ingroup: user %s P_NOT in netgroup %s", user, group+1 );
		} else {
			DEBUGF(DDB3)( "ingroup: user %s in netgroup %s", user, group+1 );
			result = 0;
		}
#else /* HAVE_INNETGR */
		DEBUGF(DDB3)( "ingroup: no innetgr() call, netgroups not permitted" );
#endif /* HAVE_INNETGR */
	} else if( group[0] == '<' && group[1] == '/' ){
		struct line_list users;
		Init_line_list(&users);
		Get_file_image_and_split(0,group+1,0,0,&users,Whitespace,
			0,0,0,0,0);
		DEBUGFC(DDB3)Dump_line_list("match- file contents'", &users );
		result = match_group( &users,user,0);
		Free_line_list(&users);
	} else if( (grent = getgrnam( group )) ){
		DEBUGF(DDB3)("ingroup: group id: %d\n", grent->gr_gid);
		if( pwent && (pwent->pw_gid == grent->gr_gid) ){
			DEBUGF(DDB3)("ingroup: user default group id: %d\n", pwent->pw_gid);
			result = 0;
		} else for( members = grent->gr_mem; result && *members; ++members ){
			DEBUGF(DDB3)("ingroup: member '%s'", *members);
			result = (strcmp( user, *members ) != 0);
		}
	} else if( strpbrk( group, "*[]") ){
		/* wildcard in group name, scan through all groups */
		setgrent();
		while( result && (grent = getgrent()) ){
			DEBUGF(DDB3)("ingroup: group name '%s'", grent->gr_name);
			/* now do match against group */
			if( Globmatch( group, grent->gr_name ) == 0 ){
				if( pwent && (pwent->pw_gid == grent->gr_gid) ){
					DEBUGF(DDB3)("ingroup: user default group id: %d\n",
					pwent->pw_gid);
					result = 0;
				} else {
					DEBUGF(DDB3)("ingroup: found '%s'", grent->gr_name);
					for( members = grent->gr_mem; result && *members; ++members ){
						DEBUGF(DDB3)("ingroup: member '%s'", *members);
						result = (strcmp( user, *members ) != 0);
					}
				}
			}
		}
		endgrent();
	}
	DEBUGF(DDB3)("ingroup: result: %d", result );
	return( result );
}

/***************************************************************************
 * Dump_perm_check( char *title, struct perm_check *check )
 * Dump perm_check information
 ***************************************************************************/

void Dump_perm_check( char *title,  struct perm_check *check )
{
	char buffer[SMALLBUFFER];
	if( title ) logDebug( "*** perm_check %s ***", title );
	if( check ){
		logDebug(
		"  user '%s', rmtuser '%s', printer '%s', service '%c', lpc '%s'",
		check->user, check->remoteuser, check->printer, check->service, check->lpc );
		Dump_host_information( "  host", check->host );
		Dump_host_information( "  remotehost", check->remotehost );
		logDebug( "  ip '%s' port %d",
			check->addr?
				inet_ntop_sockaddr( check->addr, buffer, sizeof(buffer)):"<NONE>",
			check->port);
		logDebug( " authtype '%s', authfrom '%s', auth_client_id '%s', auth_from_id '%s'",
			check->authtype, check->authfrom,
			check->auth_client_id, check->auth_from_id );
	}
}
