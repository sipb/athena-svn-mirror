/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****
$Id: linelist.h,v 1.1.1.3 1999-05-04 18:50:43 mwhitson Exp $
 **** HEADER *****/

/*
 * arrays of pointers to lines
 */
struct line_list {
	char **list;	/* array of pointers to lines */
	int count;		/* number of entries */
	int max;		/* maximum number of entries */
};

/*
 * Types of options that we can initialize or set values of
 */
enum key_type { FLAG_K, INTEGER_K, STRING_K, LIST_K };

/*
 * datastructure for initialization
 */

struct keywords{
    char *keyword;		/* name of keyword */
    enum key_type type;	/* type of entry */
    void *variable;		/* address of variable */
	int  maxval;		/* value of token */
	int  flag;			/* flag for variable */
	char *default_value;		/* default value */
};

extern void Init_line_list( struct line_list *l );
extern void Free_line_list( struct line_list *l );
extern void Check_max( struct line_list *l, int incr );
extern void Add_line_list( struct line_list *l, char *str,
	char *sep, int sort, int uniq );
extern void Merge_list( struct line_list *dest, struct line_list *src,
	char *sep, int sort, int uniq );
extern void Split( struct line_list *l, char *str, char *sep,
	int sort, char *keysep, int uniq, int trim, int nocomments );
extern void Dump_line_list( char *title, struct line_list *l );
extern int Find_last_key( struct line_list *l, char *key, char *sep, int *m );
extern int Find_first_key( struct line_list *l, char *key, char *sep, int *m );
extern char *Find_value( struct line_list *l, char *key, char *sep );
extern char *Find_exists_value( struct line_list *l, char *key, char *sep );
extern char *Find_str_value( struct line_list *l, char *key, char *sep );
extern int Find_flag_value( struct line_list *l, char *key, char *sep );
extern char *Fix_val( char *s );

extern void Read_file_list( struct line_list *model, char *str,
	char *sep, int sort, char *keysep, int uniq, int trim, int marker,
	int doinclude, int nocomment );
extern void Read_file_and_split( struct line_list *model, char *file,
	char *sep, int sort, char *keysep, int uniq, int trim, int nocomment );
extern char *Select_model_info( struct line_list *model, struct line_list *raw,
	char *id );
extern void lowercase( char *s );
extern void uppercase( char *s );
extern void Build_printcap_info( 
	struct line_list *names, struct line_list *order,
	struct line_list *list, struct line_list *raw );
extern char *Select_pc_info( struct line_list *info, struct line_list *names,
	struct line_list *input, char *id, int server, char *hostname );
extern void Config_value_conversion( struct keywords *key, char *s );
extern void *malloc_or_die( size_t size, const char *file, int line );
extern void *realloc_or_die( void *p, size_t size, const char *file, int line );
extern char *safestrdup (const char *p, const char *file, int line);
extern char *safestrdup2( const char *s1, const char *s2,
	const char *file, int line );
extern char *safestrdup3( const char *s1, const char *s2, const char *s3,
	const char *file, int line );
extern void Remove_line_list( struct line_list *l, int n );
extern void Set_str_value( struct line_list *l, char *key, const char *value );
