#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "varp.h"

#define NUMVARS 20

int var_init(varlist **vlist)
{
  varlist *vl;

  if (vlist == NULL)
    return 1;

  vl = malloc(sizeof(*vl));
  if (vl == NULL)
    return 1;

  vl->vars = malloc(NUMVARS * sizeof(var *));
  if (vl->vars == NULL)
    {
      free(vl);
      return 1;
    }

  vl->size = NUMVARS;
  vl->used = 0;

  *vlist = vl;
  return 0;
}

int var_destroy(varlist *vl)
{
  int i;

  if (vl == NULL)
    return 1;

  for (i = 0; i < vl->used; i++)
    {
      free(vl->vars[i]->name);
      free(vl->vars[i]->value);
      free(vl->vars[i]);
    }

  free(vl);
  return 0;
}

/* This sucks, but so does realloc. */
static int enlarge(varlist *vl)
{
  void *newptr;

  newptr = malloc(sizeof(var *) * (vl->size + NUMVARS));
  if (newptr == NULL)
    return 1;

  memcpy(newptr, vl->vars, vl->size * sizeof(var *));
  free(vl->vars);
  vl->vars = newptr;
  vl->size += NUMVARS;

  return 0;
}

static int nameIndex(varlist *vl, char *name)
{
  int i;

  for (i = 0; i < vl->used; i++)
    if (!strcmp(vl->vars[i]->name, name))
      return i;

  return -1;
}

int var_getString(varlist *vl, char *name, char **value)
{
  int i;

  if (vl == NULL || name == NULL || value == NULL)
    return 1;

  i = nameIndex(vl, name);
  if (i == -1)
    return 1;

  *value = vl->vars[i]->value;
  return 0;
}

int var_getValue(varlist *vl, char *name, void **value, int *length)
{
  int i;

  if (vl == NULL || name == NULL || value == NULL || length == NULL)
    return 1;

  i = nameIndex(vl, name);
  if (i == -1)
    return 1;

  *value = vl->vars[i]->value;
  *length = vl->vars[i]->length;
  return 0;
}

static int newName(varlist *vl, char *name)
{
  var *ptr;

  if (vl->size == vl->used)
    if (enlarge(vl))
      return 1;

  ptr = malloc(sizeof(var));
  if (ptr == NULL)
    return -1;

  ptr->name = malloc(strlen(name) + 1);
  if (ptr->name == NULL)
    {
      free(ptr);
      return -1;
    }

  strcpy(ptr->name, name);
  ptr->value = NULL;

  vl->vars[vl->used] = ptr;
  return vl->used++;
}

static int freeVar(varlist *vl, int i)
{
  int j;

  free(vl->vars[i]->name);
  free(vl->vars[i]->value);

  for (j = i; j < vl->used - 1; j++)
    vl->vars[j] = vl->vars[j + 1];

  vl->used--;
}

int var_setValue(varlist *vl, char *name, void *value, int length)
{
  int i;
  void *newvalue;

  if (vl == NULL || name == NULL || (value == NULL && length != 0) ||
      length < 0)
    return 1;

  i = nameIndex(vl, name);

  if (i == -1 && value == NULL)
    return 1;

  if (value == NULL)
    {
      freeVar(vl, i);
      return 0;
    }

  if (i == -1)
    {
      i = newName(vl, name);
      if (i == -1)
	return 1;
    }

  newvalue = malloc(length);
  if (newvalue == NULL)
    {
      if (vl->vars[i]->value == NULL) /* meaning if we just made this */
	freeVar(vl, i);
      return 1;
    }

  memcpy(newvalue, value, length);
  free(vl->vars[i]->value);
  vl->vars[i]->value = newvalue;
  vl->vars[i]->length = length;
  return 0;
}

int var_setString(varlist *vl, char *name, char *value)
{
  return var_setValue(vl, name, value, value ? strlen(value) + 1 : 0);
}

/* The elements of this list returned are only safe until the
   variables they name are freed. So don't attempt to dereference a
   name after you free its variable. */
int var_listVars(varlist *vl, char ***list)
{
  int i;
  char **names;

  if (vl == NULL || list == NULL)
    return 1;

  names = malloc(sizeof(char *) * (vl->used + 1));
  if (names == NULL)
    return 1;

  for (i = 0; i < vl->used; i++)
    names[i] = vl->vars[i]->name;

  names[i] = NULL;

  *list = names;
  return 0;
}

int var_freeList(varlist *vl, char **list)
{
  if (list)
    free(list);
  return 0;
}
