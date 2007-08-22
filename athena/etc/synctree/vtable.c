/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#include "synctree.h"
#include <string.h>
#define NBUCKETSL2 8
#define NBUCKETS (1 << (NBUCKETSL2 -1))
#define HASHMASK (NBUCKETS - 1)

static unsigned int hash(s)
     char *s;
{ unsigned int r = 0;
  while (*s != 0) r ^= (((*(s++)) * 91) & HASHMASK);
  return r;
}    
    
struct var {
  char            *name;
  unsigned int    hash;    /* what name hashes to */
  int             level;   /* the level at the time this entry was created */
  struct var      *next;   /* next of same hash */
  struct var      *chain;  /* previous variable added to table */
  bool	          val;
};

static struct var  *vars[NBUCKETS];
static struct var  *chain; /* the variable most recently added to table */
static int level;

static struct var *find_var(name)
     char *name;
{ struct var *v;
  unsigned int h;
  h = hash(name);
  v = vars[h];
  while (v != 0) {
    if (strcmp(name,v->name) == 0) break;
    v = v->next;
  }
  return v;
}

static struct var *new_var(name)
     char *name;
{ struct var *v;
  char *s;
  unsigned int h;

  h = hash(name);
  s = (char *) malloc(strlen(name) + 1);
  strcpy(s,name);
  v = (struct var *) malloc(sizeof(struct var));
  v->name = s;
  v->hash = h;
  v->level = level;
  v->next = vars[h];
  v->chain = chain;
  v->val = FALSE;
  chain = v;
  vars[h] = v;
  return v;
}

void setvar(name)
     char *name;
{ struct var *v;
  if (((v = find_var(name)) == 0) || (v->level < level))
    v = new_var(name);
  v->val = TRUE;
}

void unsetvar(name)
     char *name;
{ struct var *v;
  if (((v = find_var(name)) == 0) || (v->level < level))
    v = new_var(name);
  v->val = FALSE;
}

bool getvar(name)
     char *name;
{ struct var *v;
  unsigned int invert = 0;
  bool var_val;
  if (name[0] == '!') {
    name++;
    invert = 1;
  }
  /* C doesn't have a logical XOR ! */
  var_val = (bool) (((v = find_var(name)) != (struct var *) 0) &&
		    (v->val != FALSE));
  return invert? (bool) !var_val : var_val;
}

void vtable_push()
{ level++;
}

void vtable_pop(){
  if (level > 0) level--;
  while ((chain != 0) && (chain->level > level)) {
    struct var *old;
    vars[chain->hash] = chain->next;
    old = chain;
    chain = chain->chain;
    free(old->name);
    free(old);
  }
}
