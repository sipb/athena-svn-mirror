/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#if defined(_AIX) || defined(_AUX_SOURCE)
#define alloca malloc
#define freea free
#else
#define freea(x)
#endif

#if defined(_AUX_SOURCE)
#define NO_RLIMIT
#define NO_LINEBUF
#endif

#include <ctype.h>
#define MAXNRULES 2000

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
typedef enum bool { FALSE, TRUE} bool;


typedef struct boolstruct {
  enum boolexp_type { VARIABLE, UNARY, BINARY } type;
  enum bool_operator { NOT, AND, OR } op;
  char *variable;
  struct boolstruct *left;
  struct boolstruct *right;
} *bool_exp;

bool_exp bool_var();
bool_exp bool_not();
bool_exp bool_and();
bool_exp bool_or();
void bool_free();
bool bool_eval();
bool getvar();


typedef struct {
  enum rule_type { R_MAP, R_CHASE, R_ACTION, R_WHEN,
		     R_IF, R_IF_ELSE, R_SKIP,
		     R_FRAMEMARKER } type;
  union { struct { char *globexp;
		   unsigned int file_types;
		   char **dests;
		 } u_map;
	  struct { char *globexp; } u_chase;
	  struct { enum action_type
		     {  ACTION_COPY, ACTION_LOCAL, ACTION_LINK,
			  ACTION_DELETE, ACTION_IGNORE } type;
		   char *globexp;
		   unsigned int file_types;
		   unsigned int options;
		 } u_action;
	  struct { enum when_type { WHEN_SH, WHEN_CSH } type;
		   char *globexp;
		   unsigned int file_types; 
		   char **cmds;
		 } u_when;
	  struct { bool_exp boolexp;
		   int first;
		   bool inactive;  /* this is not filled in by readrules */
		 } u_if;
	  struct { int first;
		 } u_skip;
	} u;
} rule;

#define letternum(c) (c - (isupper(c)? 'A':'a'))
#define set_option(bf,c) (bf |= (0x01 << letternum(c)))
#define option_on(rno,c) ((bool) ((rules[rno].type == R_ACTION)? \
				  (((rules[rno].u.u_action.options) >> letternum(c)) & 0x01): \
				  panic("option_on: rules[rno].type is not R_ACTION")))

/* definitions for 'types' field */
#define TYPE_D 0x01
#define TYPE_C 0x02
#define TYPE_B 0x04
#define TYPE_R 0x08
#define TYPE_L 0x10
#define TYPE_S 0x20
#define TYPE_X 0x40
#define TYPE_V 0x80
#define TYPE_ALL (TYPE_D|TYPE_C|TYPE_B|TYPE_R|TYPE_L|TYPE_S|TYPE_V)

#define hastype(bf,tp) ((bool) ((bf & tp) != 0))


/*  now implemented as a procedure in rules.c
#define newrule()  { if (++lastrule == MAXNRULES) { \
    fprintf(stderr,"Too many rules.\n"); \
    exit(1); }}
*/
void newrule();
#define lstrule rules[lastrule]


/* definitions for 'rflag' variable */
#define RFLAG_SCAN_TARGET	0x01	/* Target directory scan necessary */
