/* Input parser for bison
   Copyright (C) 1984, 1986, 1989, 1992, 1998, 2000, 2001, 2002
   Free Software Foundation, Inc.

   This file is part of Bison, the GNU Compiler Compiler.

   Bison is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Bison is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bison; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */


#include "system.h"
#include "quotearg.h"
#include "quote.h"
#include "getargs.h"
#include "files.h"
#include "symtab.h"
#include "symlist.h"
#include "gram.h"
#include "complain.h"
#include "output.h"
#include "reader.h"
#include "conflicts.h"
#include "muscle_tab.h"

static symbol_list_t *grammar = NULL;
static int start_flag = 0;
merger_list *merge_functions;

/* Nonzero if %union has been seen.  */
int typed = 0;

/*-----------------------.
| Set the start symbol.  |
`-----------------------*/

void
grammar_start_symbol_set (symbol_t *s, location_t l)
{
  if (start_flag)
    complain_at (l, _("multiple %s declarations"), "%start");
  else
    {
      start_flag = 1;
      startsymbol = s;
      startsymbol_location = l;
    }
}


/*----------------------------------------------------------------.
| There are two prologues: one before %union, one after.  Augment |
| the current one.                                                |
`----------------------------------------------------------------*/

void
prologue_augment (const char *prologue, location_t location)
{
  struct obstack *oout =
    !typed ? &pre_prologue_obstack : &post_prologue_obstack;

  if (!no_lines_flag)
    {
      obstack_fgrow2 (oout, muscle_find ("linef"),
		      location.first_line,
		      quotearg_style (c_quoting_style,
				      muscle_find ("filename")));
    }
  obstack_sgrow (oout, prologue);
}




/*----------------------.
| Handle the epilogue.  |
`----------------------*/

void
epilogue_set (const char *epilogue, location_t location)
{
  if (!no_lines_flag)
    {
      obstack_fgrow2 (&muscle_obstack, muscle_find ("linef"),
		      location.first_line,
		      quotearg_style (c_quoting_style,
				      muscle_find ("filename")));
    }
  obstack_sgrow (&muscle_obstack, epilogue);
  obstack_1grow (&muscle_obstack, 0);
  muscle_insert ("epilogue", obstack_finish (&muscle_obstack));
}




 /*-------------------------------------------------------------------.
| Return the merger index for a merging function named NAME, whose   |
| arguments have type TYPE.  Records the function, if new, in        |
| merger_list.							     |
`-------------------------------------------------------------------*/

static int
get_merge_function (const char* name, const char* type,
		    location_t loc)
{
  merger_list *syms;
  merger_list head;
  int n;

  if (! glr_parser)
    return 0;

  if (type == NULL)
    type = "";

  head.next = merge_functions;
  for (syms = &head, n = 1; syms->next != NULL; syms = syms->next, n += 1)
    if (strcmp (name, syms->next->name) == 0)
      break;
  if (syms->next == NULL)
    {
      syms->next = XMALLOC (merger_list, 1);
      syms->next->name = xstrdup (name);
      syms->next->type = xstrdup (type);
      syms->next->next = NULL;
      merge_functions = head.next;
    }
  else if (strcmp (type, syms->next->type) != 0)
    warn_at (loc, _("result type clash on merge function %s: `%s' vs. `%s'"),
	     name, type, syms->next->type);
  return n;
}

/*--------------------------------------.
| Free all merge-function definitions.	|
`--------------------------------------*/

void
free_merger_functions (void)
{
  merger_list *L0;
  if (! glr_parser)
    return;
  L0 = merge_functions;
  while (L0 != NULL)
    {
      merger_list *L1 = L0->next;
      free (L0);
      L0 = L1;
    }
}


/*-------------------------------------------------------------------.
| Parse the input grammar into a one symbol_list_t structure.  Each  |
| rule is represented by a sequence of symbols: the left hand side   |
| followed by the contents of the right hand side, followed by a     |
| null pointer instead of a symbol to terminate the rule.  The next  |
| symbol is the lhs of the following rule.                           |
|                                                                    |
| All actions are copied out, labelled by the rule number they apply |
| to.                                                                |
|                                                                    |
| Bison used to allow some %directives in the rules sections, but    |
| this is no longer consider appropriate: (i) the documented grammar |
| doesn't claim it, (ii), it would promote bad style, (iii), error   |
| recovery for %directives consists in skipping the junk until a `%' |
| is seen and helrp synchronizing.  This scheme is definitely wrong  |
| in the rules section.                                              |
`-------------------------------------------------------------------*/

/* The (currently) last symbol of GRAMMAR. */
symbol_list_t *grammar_end = NULL;

/* Append S to the GRAMMAR. */
void
grammar_symbol_append (symbol_t *symbol, location_t location)
{
  symbol_list_t *p = symbol_list_new (symbol, location);

  if (grammar_end)
    grammar_end->next = p;
  else
    grammar = p;

  grammar_end = p;
}

/* The rule currently being defined, and the previous rule.
   CURRENT_RULE points to the first LHS of the current rule, while
   PREVIOUS_RULE_END points to the *end* of the previous rule (NULL).  */
symbol_list_t *current_rule = NULL;
symbol_list_t *previous_rule_end = NULL;


/*----------------------------------------------.
| Create a new rule for LHS in to the GRAMMAR.  |
`----------------------------------------------*/

void
grammar_rule_begin (symbol_t *lhs, location_t location)
{
  if (!start_flag)
    {
      startsymbol = lhs;
      startsymbol_location = location;
      start_flag = 1;
    }

  /* Start a new rule and record its lhs.  */
  ++nrules;
  ++nritems;

  previous_rule_end = grammar_end;
  grammar_symbol_append (lhs, location);
  current_rule = grammar_end;

  /* Mark the rule's lhs as a nonterminal if not already so.  */

  if (lhs->class == unknown_sym)
    {
      lhs->class = nterm_sym;
      lhs->number = nvars;
      ++nvars;
    }
  else if (lhs->class == token_sym)
    complain_at (location, _("rule given for %s, which is a token"), lhs->tag);
}

/* Check that the last rule (CURRENT_RULE) is properly defined.  For
   instance, there should be no type clash on the default action.  */

static void
grammar_current_rule_check (void)
{
  symbol_t *lhs = current_rule->sym;
  char const *lhs_type = lhs->type_name;
  symbol_t *first_rhs = current_rule->next->sym;

  /* If there is an action, then there is nothing we can do: the user
     is allowed to shoot herself in the foot.  */
  if (current_rule->action)
    return;

  /* Don't worry about the default action if $$ is untyped, since $$'s
     value can't be used.  */
  if (! lhs_type)
    return;

  /* If $$ is being set in default way, report if any type mismatch.  */
  if (first_rhs)
    {
      const char *rhs_type = first_rhs->type_name ? first_rhs->type_name : "";
      if (strcmp (lhs_type, rhs_type))
	complain_at (current_rule->location,
		     _("type clash (`%s' `%s') on default action"),
		     lhs_type, rhs_type);
    }
  /* Warn if there is no default for $$ but we need one.  */
  else
    complain_at (current_rule->location,
		 _("empty rule for typed nonterminal, and no action"));
}


/*-------------------------------------.
| End the currently being grown rule.  |
`-------------------------------------*/

void
grammar_rule_end (location_t location)
{
  /* Put an empty link in the list to mark the end of this rule  */
  grammar_symbol_append (NULL, grammar_end->location);
  current_rule->location = location;
  grammar_current_rule_check ();
}


/*-------------------------------------------------------------------.
| The previous action turns out the be a mid-rule action.  Attach it |
| to the current rule, i.e., create a dummy symbol, attach it this   |
| mid-rule action, and append this dummy nonterminal to the current  |
| rule.                                                              |
`-------------------------------------------------------------------*/

void
grammar_midrule_action (void)
{
  /* Since the action was written out with this rule's number, we must
     give the new rule this number by inserting the new rule before
     it.  */

  /* Make a DUMMY nonterminal, whose location is that of the midrule
     action.  Create the MIDRULE.  */
  location_t dummy_location = current_rule->action_location;
  symbol_t *dummy = dummy_symbol_get (dummy_location);
  symbol_list_t *midrule = symbol_list_new (dummy, dummy_location);

  /* Make a new rule, whose body is empty, before the current one, so
     that the action just read can belong to it.  */
  ++nrules;
  ++nritems;
  /* Attach its location and actions to that of the DUMMY.  */
  midrule->location = dummy_location;
  midrule->action = current_rule->action;
  midrule->action_location = dummy_location;
  current_rule->action = NULL;

  if (previous_rule_end)
    previous_rule_end->next = midrule;
  else
    grammar = midrule;

  /* End the dummy's rule.  */
  previous_rule_end = symbol_list_new (NULL, dummy_location);
  previous_rule_end->next = current_rule;

  midrule->next = previous_rule_end;

  /* Insert the dummy nonterminal replacing the midrule action into
     the current rule.  */
  grammar_current_rule_symbol_append (dummy, dummy_location);
}

/* Set the precedence symbol of the current rule to PRECSYM. */

void
grammar_current_rule_prec_set (symbol_t *precsym, location_t location)
{
  if (current_rule->ruleprec)
    complain_at (location, _("two @prec's in a row"));
  current_rule->ruleprec = precsym;
}

/* Attach dynamic precedence DPREC to the current rule. */

void
grammar_current_rule_dprec_set (int dprec, location_t location)
{
  if (! glr_parser)
    warn_at (location, _("%%dprec affects only GLR parsers"));
  if (dprec <= 0)
    complain_at (location, _("%%dprec must be followed by positive number"));
  else if (current_rule->dprec != 0)
    complain_at (location, _("only one %%dprec allowed per rule"));
  current_rule->dprec = dprec;
}

/* Attach a merge function NAME with argument type TYPE to current
   rule. */

void
grammar_current_rule_merge_set (const char* name, location_t location)
{
  if (! glr_parser)
    warn_at (location, _("%%merge affects only GLR parsers"));
  if (current_rule->merger != 0)
    complain_at (location, _("only one %%merge allowed per rule"));
  current_rule->merger =
    get_merge_function (name, current_rule->sym->type_name, location);
}

/* Attach a SYMBOL to the current rule.  If needed, move the previous
   action as a mid-rule action.  */

void
grammar_current_rule_symbol_append (symbol_t *symbol, location_t location)
{
  if (current_rule->action)
    grammar_midrule_action ();
  ++nritems;
  grammar_symbol_append (symbol, location);
}

/* Attach an ACTION to the current rule.  If needed, move the previous
   action as a mid-rule action.  */

void
grammar_current_rule_action_append (const char *action, location_t location)
{
  if (current_rule->action)
    grammar_midrule_action ();
  current_rule->action = action;
  current_rule->action_location = location;
}


/*---------------------------------------------------------------.
| Convert the rules into the representation using RRHS, RLHS and |
| RITEM.                                                         |
`---------------------------------------------------------------*/

static void
packgram (void)
{
  unsigned int itemno = 0;
  rule_number_t ruleno = 0;
  symbol_list_t *p = grammar;

  ritem = XCALLOC (item_number_t, nritems);
  rules = XCALLOC (rule_t, nrules);

  while (p)
    {
      symbol_t *ruleprec = p->ruleprec;
      rules[ruleno].user_number = ruleno;
      rules[ruleno].number = ruleno;
      rules[ruleno].lhs = p->sym;
      rules[ruleno].rhs = ritem + itemno;
      rules[ruleno].location = p->location;
      rules[ruleno].useful = TRUE;
      rules[ruleno].action = p->action;
      rules[ruleno].action_location = p->action_location;
      rules[ruleno].dprec = p->dprec;
      rules[ruleno].merger = p->merger;

      p = p->next;
      while (p && p->sym)
	{
	  /* item_number_t = symbol_number_t.
	     But the former needs to contain more: negative rule numbers. */
	  ritem[itemno++] = symbol_number_as_item_number (p->sym->number);
	  /* A rule gets by default the precedence and associativity
	     of the last token in it.  */
	  if (p->sym->class == token_sym)
	    rules[ruleno].prec = p->sym;
	  if (p)
	    p = p->next;
	}

      /* If this rule has a %prec,
         the specified symbol's precedence replaces the default.  */
      if (ruleprec)
	{
	  rules[ruleno].precsym = ruleprec;
	  rules[ruleno].prec = ruleprec;
	}
      ritem[itemno++] = rule_number_as_item_number (ruleno);
      ++ruleno;

      if (p)
	p = p->next;
    }

  assert (itemno == nritems);

  if (trace_flag & trace_sets)
    ritem_print (stderr);
}

/*------------------------------------------------------------------.
| Read in the grammar specification and record it in the format     |
| described in gram.h.  All actions are copied into ACTION_OBSTACK, |
| in each case forming the body of a C function (YYACTION) which    |
| contains a switch statement to decide which action to execute.    |
`------------------------------------------------------------------*/

void
reader (void)
{
  gram_control_t gram_control;

  /* Initialize the symbol table.  */
  symbols_new ();

  /* Construct the accept symbol. */
  accept = symbol_get ("$accept", empty_location);
  accept->class = nterm_sym;
  accept->number = nvars++;

  /* Construct the error token */
  errtoken = symbol_get ("error", empty_location);
  errtoken->class = token_sym;
  errtoken->number = ntokens++;

  /* Construct a token that represents all undefined literal tokens.
     It is always token number 2.  */
  undeftoken = symbol_get ("$undefined", empty_location);
  undeftoken->class = token_sym;
  undeftoken->number = ntokens++;

  /* Initialize the obstacks. */
  obstack_init (&pre_prologue_obstack);
  obstack_init (&post_prologue_obstack);

  finput = xfopen (infile, "r");
  gram_in = finput;

  gram_debug = !!getenv ("parse");
  gram__flex_debug = !!getenv ("scan");
  scanner_initialize ();
  gram_parse (&gram_control);

  /* If something went wrong during the parsing, don't try to
     continue.  */
  if (complain_message_count)
    exit (1);

  /* Grammar has been read.  Do some checking */
  if (nrules == 0)
    fatal (_("no rules in the input grammar"));

  /* Report any undefined symbols and consider them nonterminals.  */
  symbols_check_defined ();

  /* If the user did not define her ENDTOKEN, do it now. */
  if (!endtoken)
    {
      endtoken = symbol_get ("$end", empty_location);
      endtoken->class = token_sym;
      endtoken->number = 0;
      /* Value specified by POSIX.  */
      endtoken->user_token_number = 0;
    }

  /* Insert the initial rule, which line is that of the first rule
     (not that of the start symbol):

     accept: %start EOF.  */
  {
    symbol_list_t *p = symbol_list_new (accept, empty_location);
    p->location = grammar->location;
    p->next = symbol_list_new (startsymbol, empty_location);
    p->next->next = symbol_list_new (endtoken, empty_location);
    p->next->next->next = symbol_list_new (NULL, empty_location);
    p->next->next->next->next = grammar;
    nrules += 1;
    nritems += 3;
    grammar = p;
  }

  if (SYMBOL_NUMBER_MAX < nsyms)
    fatal (_("too many symbols (tokens plus nonterminals); maximum %d"),
	   SYMBOL_NUMBER_MAX);

  assert (nsyms == ntokens + nvars);

  xfclose (finput);

  /* Assign the symbols their symbol numbers.  Write #defines for the
     token symbols into FDEFINES if requested.  */
  symbols_pack ();

  /* Convert the grammar into the format described in gram.h.  */
  packgram ();

  /* The grammar as a symbol_list_t is no longer needed. */
  LIST_FREE (symbol_list_t, grammar);
}