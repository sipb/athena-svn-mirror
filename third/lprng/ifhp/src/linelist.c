/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: linelist.c,v 1.1.1.3 1999-05-04 18:50:42 mwhitson Exp $";

#include "ifhp.h"

/**** ENDINCLUDE ****/

/* lowercase and uppercase (destructive) a string */
void lowercase( char *s )
{
	int c;
	if( s ){
		for( ; (c = *s); ++s ){
			if( isupper(c) ) *s = tolower(c);
		}
	}
}
void uppercase( char *s )
{
	int c;
	if( s ){
		for( ; (c = *s); ++s ){
			if( islower(c) ) *s = toupper(c);
		}
	}
}

/*
 * Trunc str - remove trailing white space (destructive)
 */

char *trunc_str( char *s)
{
	char *t;
	if(s && *s){
		for( t=s+strlen(s); t > s && isspace(cval(t-1)); --t );
		*t = 0;
	}
	return( s );
}

/*
 * Memory Allocation Routines
 * - same as malloc, realloc, but with error messages
 */
void *malloc_or_die( size_t size, const char *file, int line )
{
    void *p;
    p = malloc(size);
    if( p == 0 ){
        logerr_die( "malloc of %d failed, file '%s', line %d",
			size, file, line );
    }
    return( p );
}
void *realloc_or_die( void *p, size_t size, const char *file, int line )
{
    p = realloc(p, size);
    if( p == 0 ){
        logerr_die( "realloc of %d failed, file '%s', line %d", size, file, line );
    }
    return( p );
}

/*
 * duplicate a string safely, generate an error message
 */

char *safestrdup (const char *p, const char *file, int line)
{
    char *new = 0;

	if( p == 0) p = "";
	new = malloc_or_die( strlen (p) + 1, file, line );
	strcpy( new, p );
	return( new );
}

/*
 * char *safestrdup2( char *s1, char *s2, char *file, int line )
 *  duplicate two concatenated strings
 *  returns: malloced string area
 */

char *safestrdup2( const char *s1, const char *s2, const char *file, int line )
{
	int n = 1 + (s1?strlen(s1):0) + (s2?strlen(s2):0);
	char *s = malloc_or_die( n, file, line );
	s[0] = 0;
	if( s1 ) strcat(s,s1);
	if( s2 ) strcat(s,s2);
	return( s );
}

/*
 * char *safestrdup3( char *s1, char *s2, char *s3, char *file, int line )
 *  duplicate three concatenated strings
 *  returns: malloced string area
 */

char *safestrdup3( const char *s1, const char *s2, const char *s3,
	const char *file, int line )
{
	int n = 1 + (s1?strlen(s1):0) + (s2?strlen(s2):0) + (s3?strlen(s3):0);
	char *s = malloc_or_die( n, file, line );
	s[0] = 0;
	if( s1 ) strcat(s,s1);
	if( s2 ) strcat(s,s2);
	if( s3 ) strcat(s,s3);
	return( s );
}

/*
  Line Splitting and List Management
 
  Model:  we have a list of malloced and duplicated lines
          we never remove the lines unless we free them.
          we never put them in unless we malloc them
 */

/*
 * void Init_line_list( struct line_list *l )
 *  - inititialize a list by zeroing it
 */

void Init_line_list( struct line_list *l )
{
	memset(l, 0, sizeof(l[0]));
}

/*
 * void Free_line_list( struct line_list *l )
 *  - clear a list by freeing the allocated array
 */

void Free_line_list( struct line_list *l )
{
	int i;
	if( l->list ){
		for( i = 0; i < l->count; ++i ){
			if( l->list[i] ) free( l->list[i]);
		}
		free(l->list);
	}
	l->count = 0;
	l->list = 0;
	l->max = 0;
}

/*
 * void Check_max( struct line_list *l, int incr )
 *
 */

void Check_max( struct line_list *l, int incr )
{
	if( l->count+incr >= l->max ){
		DEBUG5("Check_max: inc %d, count %d, max %d, list 0x%lx",
			incr, l->count, l->max, (long)l->list );
		l->max += 100+incr;
		if( !(l->list = realloc_or_die( l->list, l->max*sizeof(char *),
			__FILE__,__LINE__)) ){
			Errorcode = JABORT;
			logerr_die( "Check_max: realloc %d failed",
				l->max*sizeof(char*) );
		}
		DEBUG5("Check_max: new max %d, list 0x%lx", l->max, (long)l->list );
	}
}

/*
 *void Add_line_list( struct line_list *l, char *str,
 *  char *sep, int sort, int uniq )
 *  - add a copy of str to the line list
 *  sep      - key separator, used for sorting
 *  sort = 1 - sort the values
 *  uniq = 1 - only one value
 */

void Add_line_list( struct line_list *l, char *str,
		char *sep, int sort, int uniq )
{
	char *s = 0;
	int c = 0, cmp, mid;
	DEBUG5("Add_line_list: adding '%s', sep '%s', sort %d, uniq %d",
		str, sep, sort, uniq );
	DEBUG5("Add_line_list: max %d, count %d, list 0x%lx",
		l->count, l->max, (long)l->list );

	Check_max(l, 2);
	str = safestrdup( str,__FILE__,__LINE__);
	if( sort == 0 ){
		l->list[l->count++] = str;
	} else {
		s = 0;
		if( sep && (s = strpbrk( str, sep )) ){ c = *s; *s = 0; }
		/* find everything <= the mid point */
		/* cmp = key <> list[mid] */
		cmp = Find_last_key( l, str, sep, &mid );
		if( s ) *s = c;
		/* str < list[mid+1] */
		if( cmp == 0 && uniq ){
			DEBUG5("Add_line_list: replacing at %d", mid );
			/* we replace */
			free( l->list[mid] );		
			l->list[mid] = str;
		} else if( cmp >= 0 ){
			/* we need to insert after mid */
			++l->count;
			DEBUG5("Add_line_list: inserting after %d, count %d, moving %d",
				mid, l->count, l->count - mid - 1 );
			memmove( l->list+mid+2, l->list+mid+1,
				sizeof( char * ) * (l->count - mid - 1));
			l->list[mid+1] = str;
		} else if( cmp < 0 ) {
			/* we need to insert before mid */
			++l->count;
			DEBUG5("Add_line_list: inserting before %d, count %d, moving %d",
				mid, l->count, l->count - mid );
			memmove( l->list+mid+1, l->list+mid,
				sizeof( char * ) * (l->count - mid));
			l->list[mid] = str;
		}
	}
#ifdef DMALLOC
	dmalloc_verify(0);
#endif
#if VERBOSE
	if(DEBUGL5)Dump_line_list("Add_line_list: result", l);
#endif
}

void Merge_list( struct line_list *dest, struct line_list *src,
	char *sep, int sort, int uniq )
{
	int i;
	for( i = 0; i < src->count; ++i ){
		Add_line_list( dest, src->list[i], sep, sort, uniq );
	}
}

/*
 * Split( struct line_list *l, char *str, int sort, char *keysep,
 *		int uniq, int trim )
 *  Split the str up into strings, as delimted by sep.
 *   put duplicates of the original into the line_list l.
 *  If sort != 0, then sort them using keysep to separate sort key from value
 *  if uniq != then replace rather than add entries
 *  if trim != 0 then remove leading and trailing whitespace
 *
 */
void Split( struct line_list *l, char *str, char *sep,
	int sort, char *keysep, int uniq, int trim, int nocomments )
{
	char *sdup, *end = 0, *t;
	if(DEBUGL4){
		char b[40];
		int n;
		plp_snprintf( b,sizeof(b)-8,"%s",str );
		if( (n = strlen(b)) > sizeof(b)-10 ) strcpy( b+n," ..." );
		logDebug("Split: str '%s', sort %d, keysep '%s', uniq %d, trim %d",
			b, sort, keysep, uniq, trim );
	}
	if( str == 0 || *str == 0 ) return;
	sdup = str = safestrdup(str,__FILE__,__LINE__);
	for( ; str && *str; str = end ){
		end = 0;
		if( sep && (end = strpbrk( str, sep )) ){
			*end++ = 0;
		}
		DEBUG5("Split: working on '%s'", str );
		if( trim ){
			while( isspace(cval(str)) ) ++str;
			for( t = str+strlen(str)-1;
				t >= str && isspace(cval(t)); --t ) t[0] = 0;
		}
		DEBUG5("Split: after trim '%s'", str );
		if( *str == 0 || (nocomments && *str == '#') ) continue;
		Add_line_list( l, str, keysep, sort, uniq );
	}
	free( sdup );
}

void Dump_line_list( char *title, struct line_list *l )
{
	int i;
	logDebug("Dump_line_list: %s - count %d, max %d, list 0x%lx",
		title, l->count, l->max, (long)l->list );
	for( i = 0; i < l->count; ++i ){
		logDebug( "  [%2d]='%s'", i, l->list[i] );
	}
}


/*
 * int Find_last_key( struct line_list *l, char *key, char *sep, int *mid )
 *  Search the list for the last corresponding key value
 *  The list has lines of the form:
 *    key [separator] value
 *  returns:
 *    *at = index of last tested value
 *    return value: 0 if found;
 *                  <0 if list[*at] < key
 *                  >0 if list[*at] > key
 */

int Find_last_key( struct line_list *l, char *key, char *sep, int *m )
{
	int c=0, cmp=-1, cmpl = 0, bot, top, mid;
	char *s, *t;
	mid = bot = 0; top = l->count-1;
	DEBUG5("Find_last_key: count %d, key '%s'", l->count, key );
	while( cmp && bot <= top ){
		mid = (top+bot)/2;
		s = l->list[mid];
		t = 0;
		if( sep && (t = strpbrk(s, sep )) ) { c = *t; *t = 0; }
		cmp = strcasecmp(key,s);
		if( t ) *t = c;
		if( cmp > 0 ){
			bot = mid+1;
		} else if( cmp < 0 ){
			top = mid -1;
		} else while( mid+1 < l->count ){
			s = l->list[mid+1];
			DEBUG5("Find_last_key: existing entry, mid %d, '%s'",
				mid, l->list[mid] );
			t = 0;
			if( sep && (t = strpbrk(s, sep )) ) { c = *t; *t = 0; }
			cmpl = strcasecmp(s,key);
			if( t ) *t = c;
			if( cmpl ) break;
			++mid;
		}
		DEBUG5("Find_last_key: cmp %d, top %d, mid %d, bot %d",
			cmp, top, mid, bot);
	}
	if( m ) *m = mid;
	DEBUG4("Find_last_key: key '%s', cmp %d, mid %d", key, cmp, mid );
	return( cmp );
}

int Find_first_key( struct line_list *l, char *key, char *sep, int *m )
{
	int c=0, cmp=-1, cmpl = 0, bot, top, mid;
	char *s, *t;
	mid = bot = 0; top = l->count-1;
	DEBUG5("Find_first_key: count %d, key '%s', sep '%s'",
		l->count, key, sep );
	while( cmp && bot <= top ){
		mid = (top+bot)/2;
		s = l->list[mid];
		t = 0;
		if( sep && (t = strpbrk(s, sep )) ) { c = *t; *t = 0; }
		cmp = strcasecmp(key,s);
		if( t ) *t = c;
		if( cmp > 0 ){
			bot = mid+1;
		} else if( cmp < 0 ){
			top = mid -1;
		} else while( mid > 0 ){
			s = l->list[mid-1];
			t = 0;
			if( sep && (t = strpbrk(s, sep )) ) { c = *t; *t = 0; }
			cmpl = strcasecmp(s,key);
			if( t ) *t = c;
			if( cmpl ) break;
			--mid;
		}
		DEBUG5("Find_first_key: cmp %d, top %d, mid %d, bot %d",
			cmp, top, mid, bot);
	}
	if( m ) *m = mid;
	DEBUG4("Find_first_key: cmp %d, mid %d, key '%s', count %d",
		cmp, mid, key, l->count );
	return( cmp );
}

/*
 * char *Find_value( struct line_list *l, char *key, char *sep )
 *  Search the list for a corresponding key value
 *          value
 *   key    "1"
 *   key@   "0"
 *   key#v  v
 *   key=v  v
 *  If key does not exist, we return "0"
 */

char *Find_value( struct line_list *l, char *key, char *sep )
{
	char *s = "0";
	int mid, cmp;

	DEBUG4("Find_value: key '%s', sep '%s'", key, sep );
	if( sep ){
		cmp = Find_first_key( l, key, sep, &mid );
		DEBUG4("Find_value: key '%s', cmp %d, mid %d", key, cmp, mid );
		if( cmp==0 ){
			s = Fix_val( strpbrk(l->list[mid], sep ) );
		}
		DEBUG4( "Find_value: key '%s', value '%s'", key, s );
	}
	return(s);
}

/*
 * char *Find_exists_value( struct line_list *l, char *key, char *sep )
 *  Search the list for a corresponding key value
 *          value
 *   key    "1"
 *   key@   "0"
 *   key#v  v
 *   key=v  v
 *   If value exists we return 0 (null)
 */

char *Find_exists_value( struct line_list *l, char *key, char *sep )
{
	char *s = 0;
	int mid, cmp = -2;

	if( sep ){
		cmp = Find_first_key( l, key, sep, &mid );
		if( cmp==0 ){
			s = Fix_val( strpbrk(l->list[mid], sep ) );
		}
	}
	DEBUG4( "Find_exists_value: key '%s', cmp %d, value '%s'", key, cmp, s );
	return(s);
}


/*
 * char *Find_str_value( struct line_list *l, char *key, char *sep )
 *  Search the list for a corresponding key value
 *          value
 *   key    0
 *   key@   0
 *   key#v  0
 *   key=v  v
 */

char *Find_str_value( struct line_list *l, char *key, char *sep )
{
	char *s = 0;
	int mid, cmp;

	if( sep ){
		cmp = Find_first_key( l, key, sep, &mid );
		if( cmp==0 ){
			/*
			 *  value: NULL, "", "@", "=xx", "#xx".
			 *  returns: "0", "1","0",  "xx",  "xx"
			 */
			s = strpbrk(l->list[mid], sep );
			if( s && *s == '=' ){
				++s;
			} else {
				s = 0;
			}
		}
	}
	DEBUG4( "Find_str_value: key '%s', value '%s'", key, s );
	return(s);
}
 
/*
 * char *Find_flag_value( struct line_list *l, char *key, char *sep )
 *  Search the list for a corresponding key value
 *          value
 *   key    1
 *   key@   0
 *   key#v  0
 *   key=v  0
 */

int Find_flag_value( struct line_list *l, char *key, char *sep )
{
	char *s = 0, *e;
	int n = 0;

	e = s = Find_value( l, key, sep );
	n = strtol(s,&e,0);
	if( *e ) n = 0;
	DEBUG4( "Find_flag_value: key '%s', value '%d'", key, n );
	return(n);
}
 
/*
 * char *Fix_val( char *s )
 *  passed: NULL, "", "@", "=xx", "#xx".
 *  returns: "0", "1","0",  "xx",  "xx"
 */


char *Fix_val( char *s )
{
	int c = 0;
	if( s ){
		c = cval(s);
		++s;
	}
	if( isspace(c) || c == 0 ){
		s = "1";
	} else if( c == '@' ){
		s = "0";
	}
	while((c = cval(s)) && isspace(c) ) ++s;
	return( s );
}

/*
 * Read_file_list( struct line_list *model, char *str
 *	char *sep, int sort, char *keysep, int uniq, int trim, int marker )
 *  read the model information from these files
 *  if marker != then add a NULL line after each file
 */

void Read_file_list( struct line_list *model, char *str,
	char *sep, int sort, char *keysep, int uniq, int trim,
	int marker, int doinclude, int nocomment )
{
	struct line_list l;
	int i, start, end, c=0, n, found;
	char *s, *t;

	Init_line_list(&l);
	DEBUG4("Read_file_list: '%s'", str );
	Split( &l, str, Filesep, 0, 0, 0, 1, 0 );
	start = model->count;
	for( i = 0; i < l.count; ++i ){
		Read_file_and_split( model, l.list[i], sep, sort, keysep,
			uniq, trim, nocomment );
		if( doinclude ){
			/* scan through the list, looking for include lines */
			for( end = model->count; start < end; ){
				t = 0; 
				s = model->list[start];
				if( s && (t = strpbrk( s, Whitespace )) ){ c = *t; *t = 0; }
				found = (t && !strcasecmp( s, "include" ));
				if( t ) *t = c;
				if( found ){
					DEBUG3("Read_file_list: include '%s'", t+1 );
					Read_file_list( model, t+1, sep, sort, keysep, uniq, trim,
						marker, doinclude, nocomment );
					/* at this point the include lines are at
					 *  end to model->count-1
					 * we need to move the lines from start to end-1
					 * to model->count, and then move end to start
					 */
					n = end - start;
					Check_max( model, n );
					/* copy to end */
					if(DEBUGL5)Dump_line_list("Read_file_list: include before",
						model );
					memmove( &model->list[model->count], 
						&model->list[start], n*sizeof(char *) );
					memmove( &model->list[start], 
						&model->list[end],(model->count-start)*sizeof(char *));
					if(DEBUGL5)Dump_line_list("Read_file_list: include after",
						model );
					end = model->count;
					start = end - n;
					DEBUG4("Read_file_list: start now '%s'",model->list[start]);
					/* we get rid of include line */
					memmove( &model->list[start], &model->list[start+1],
						n*sizeof(char *) );
					--model->count;
					end = model->count;
				} else {
					++start;
				}
			}
		}
		if( marker ){
			Check_max( model, 1 );
			model->list[model->count++] = 0;
		}
	}
	Free_line_list(&l);
	if(DEBUGL5)Dump_line_list("Read_file_list: result", model);
}

void Read_fd_and_split( struct line_list *list, int fd,
	char *sep, int sort, char *keysep, int uniq, int trim, int nocomment )
{
	int size = 0, count, len;
	char *sv;
	char buffer[LARGEBUFFER];

	sv = 0;
	while( (count = read(fd, buffer, sizeof(buffer)-1)) > 0 ){
		buffer[count] = 0;
		len = size+count+1;
		if( (sv = realloc_or_die( sv, len,__FILE__,__LINE__)) == 0 ){
			Errorcode = JABORT;
			logerr_die( "Read_file_and_split: realloc %d failed", len );
		}
		memcpy( sv+size, buffer, count );
		size += count;
		sv[size] = 0;
	}
	close( fd );
	Split( list, sv, sep, sort, keysep, uniq, trim, nocomment );
	free( sv );
}

void Read_file_and_split( struct line_list *list, char *file,
	char *sep, int sort, char *keysep, int uniq, int trim, int nocomment )
{
	int fd;

	DEBUG4("Read_file_and_split: '%s', trim %d, nocomment %d",
		file, trim, nocomment );
	if( (fd = open( file, O_RDONLY )) < 0 ){
		DEBUG1("Read_file_and_split: cannot open file '%s' - %s",
			file, Errormsg(errno));
		return;
	}
	Read_fd_and_split( list, fd, sep, sort, keysep, uniq, trim, nocomment );
}




/*
 * char *Select_model_info( struct line_list *list, struct line_list *model,
 *	char *id )
 *  Select the ifhp model information,  using the id value for selection.
 *  If it is not set, then if model info is found, return the selected
 *  value.
 */

char *Select_model_info( struct line_list *model, struct line_list *list,
	char *id )
{
	int i, j, c, state = 0;
	char *s, *t, *keyid = 0;
	struct line_list keys;

	DEBUG1("Select_model_info: id '%s', list->count %d, model->count %d",
		id, list->count, model->count );
	Init_line_list( &keys );
	for( i = 0; i < list->count; ++i ){
		Free_line_list(&keys);
		s = list->list[i];
		if( s == 0 ){
			state = 0;
			continue;
		}
		DEBUG5("Select_model_info: doing '%s', id '%s'", s, id);
		/* split into words, trimming entries */
		for( t = s; isspace(cval(t)); ++t );
		if( *t == 0 || *t == '#' ){
			continue;
		}
		DEBUG1("Select_model_info: state %d doing '%s', id '%s'", state, s, id);
		/* we have a group line */
		if( *s == '[' ){
			Split( &keys, s, List_sep, 0, 0, 0, 1, 0 );
			state = 1;
			if( keyid ){ free(keyid); keyid = 0; }
			if( id ){
				DEBUG5("Select_model_info: group '%s' to id '%s'", s, id );
				for( j = 0;
					j < keys.count && (state = Globmatch( keys.list[j],id ));
					++j );
			}
			DEBUG1("Select_model_info: state %d after model '%s' Globmatch to '%s'",
				state, id, s );
			continue;
		}
		/* we are ignoring lines until the next group line */
		if( state ){ continue; }
		/* do we have a continuation line? */
		/* stop at end line */
		if( strcasecmp( s, "end") == 0 ) break;
		if( isspace( cval(s)) ){
			if( keyid ){
				if( !Find_last_key( model, keyid, Value_sep, &j ) ){
					t = model->list[j];
					model->list[j] = safestrdup3(t,"\n",s,__FILE__,__LINE__);
					free(t);
				} else {
					free(keyid); keyid = 0;
				}
			}
			continue;
		}
		/* adjust line, removing extra blanks */
		if( keyid ){ free(keyid); keyid = 0; }
		keyid = safestrdup(s,__FILE__,__LINE__);
		while( (t = strpbrk( keyid, Value_sep )) && isspace( cval(t) ) ){
			memmove(t, t+1, strlen(t)+1 );
		}
		if( t && *t ) while( isspace(cval(t+1)) ){
			memmove(t+1, t+2, strlen(t+1)+1 );
		}
		c = 0;
		if( t ){ c = *t; *t = 0; }
		lowercase(keyid);
		if( t ) *t = c;
		Add_line_list( model, keyid, Value_sep, 1, 1 );
		if( t ) *t = 0;

		/*
		 * find the model information from the file currently
		 * being read
		 */
		if( !id && t && c == '=' ){
			if( !strcmp(keyid, "model") ){
				id = safestrdup(t+1,__FILE__,__LINE__);
			} else if( !strcmp(keyid, "model_from_option") ){
				Free_line_list(&keys);
				Split(&keys, t+1, List_sep, 1, Value_sep, 1, 1, 0 );
				for(j = 0; j < keys.count; ++j ){
					s = keys.list[j];
					if( strlen(s) == 1 ){
						c = *s;
						s = 0;
						if( isupper(c) ) s = Upperopts[c-'A'];
						if( islower(c) ) s = Loweropts[c-'a'];
						if( s ) id = safestrdup(s,__FILE__,__LINE__);
						break;
					}
				}
				Free_line_list(&keys);
			}
		}
	}
	Free_line_list(&keys);
	if( keyid ){ free(keyid); keyid = 0; }
	DEBUG4("Select_model_info: id %s", id );
	if(DEBUGL4) Dump_line_list( "Select_model_info- end", model );
	return(id);
}

void Remove_line_list( struct line_list *l, int n )
{
	if( l && n < l->count){
		if( l->list[n] ) free(l->list[n]);  l->list[n] = 0;
		--l->count;
		while( n < l->count ){
			l->list[n] = l->list[n+1];
			++n;
		}
	}
}

/*
 * Set_str_value( struct line_list *l, char *key, char *value )
 *   set a string value in an ordered, sorted list
 */
void Set_str_value( struct line_list *l, char *key, const char *value )
{
	char *s = 0;
	int mid;
	if( key == 0 ) return;
	if( value && *value ){
		s = safestrdup3(key,"=",value,__FILE__,__LINE__);
		Add_line_list(l,s,Value_sep,1,1);
		if(s) free(s); s = 0;
	} else if( !Find_first_key(l, key, Value_sep, &mid ) ){
		Remove_line_list(l,mid);
	}
}

