/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#include "synctree.h"

static bool_exp newb()
{
  register char *foo;
  if (foo = (char *) malloc(sizeof(struct boolstruct)))
    return (bool_exp) foo;
  else
    panic("bool newb: malloc returned null pointer");
}



bool_exp bool_var(s)
     char *s;
{
  bool_exp new;
  new = newb();
  new->type = VARIABLE;
  new->variable = s;
  return new;
}

bool_exp bool_not(b)
     bool_exp b;
{
  bool_exp new;
  new = newb();
  new->type = UNARY;
  new->op = NOT;
  new->left = b;
  return new;
}

bool_exp bool_and(bl,br)
     bool_exp bl,br;
{
  bool_exp new;
  new = newb();
  new->type = BINARY;
  new->op = AND;
  new->left = bl;
  new->right = br;
  return new;
}

bool_exp bool_or(bl,br)
     bool_exp bl,br;
{
  bool_exp new;
  new = newb();
  new->type = BINARY;
  new->op = OR;
  new->left = bl;
  new->right = br;
  return new;
}

void bool_free(b)
     bool_exp b;
{
  switch(b->type) {
  case VARIABLE:
    sfree(b->variable);
    break;
  case BINARY:
    bool_free(b->right);
  case UNARY:
    bool_free(b->left);
    break;
  default:
    panic("bool_free");
  }
  free(b);
}

bool bool_eval(b)
     bool_exp b;
{
  switch(b->type) {
  case VARIABLE:
    return getvar(b->variable);
  case UNARY:
    switch(b->op) {
    case NOT:
      return (bool) ! bool_eval(b->left);
    default:
      panic("bool_eval: unknown unary operator");
    }
  case BINARY:
    switch(b->op) {
    case AND:
      return (bool) (bool_eval(b->left) && bool_eval(b->right));
    case OR:
      return (bool) (bool_eval(b->left) || bool_eval(b->right));
    default:
      panic("bool_eval: unknown binary operator");
    }
  default:
    panic("bool_eval: unknown bool_exp type");
  }
}
      
