/* Allocate input grammar variables for bison,
   Copyright (C) 1984, 1986, 1989, 2001, 2002 Free Software Foundation, Inc.

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
#include "symtab.h"
#include "gram.h"
#include "reduce.h"
#include "reader.h"

/* Comments for these variables are in gram.h.  */

item_number_t *ritem = NULL;
unsigned int nritems = 0;

rule_t *rules = NULL;
rule_number_t nrules = 0;

symbol_t **symbols = NULL;
int nsyms = 0;
int ntokens = 1;
int nvars = 0;

symbol_number_t *token_translations = NULL;

int max_user_token_number = 256;

int glr_parser = 0;
int pure_parser = 0;


/*--------------------------------------------------------------.
| Return true IFF the rule has a `number' smaller than NRULES.  |
`--------------------------------------------------------------*/

bool
rule_useful_p (rule_t *r)
{
  return r->number < nrules;
}


/*-------------------------------------------------------------.
| Return true IFF the rule has a `number' higher than NRULES.  |
`-------------------------------------------------------------*/

bool
rule_useless_p (rule_t *r)
{
  return r->number >= nrules;
}


/*--------------------------------------------------------------------.
| Return true IFF the rule is not flagged as useful *and* is useful.  |
| In other words, it was discarded because of conflicts.              |
`--------------------------------------------------------------------*/

bool
rule_never_reduced_p (rule_t *r)
{
  return !r->useful && r->number < nrules;
}


/*----------------------------------------------------------------.
| Print this RULE's number and lhs on OUT.  If a PREVIOUS_LHS was |
| already displayed (by a previous call for another rule), avoid  |
| useless repetitions.                                            |
`----------------------------------------------------------------*/

void
rule_lhs_print (rule_t *rule, symbol_t *previous_lhs, FILE *out)
{
  fprintf (out, "  %3d ", rule->number);
  if (previous_lhs != rule->lhs)
    {
      fprintf (out, "%s:", rule->lhs->tag);
    }
  else
    {
      int n;
      for (n = strlen (previous_lhs->tag); n > 0; --n)
	fputc (' ', out);
      fputc ('|', out);
    }
}


/*--------------------------------------.
| Return the number of symbols in RHS.  |
`--------------------------------------*/

int
rule_rhs_length (rule_t *rule)
{
  int res = 0;
  item_number_t *rhsp;
  for (rhsp = rule->rhs; *rhsp >= 0; ++rhsp)
    ++res;
  return res;
}


/*-------------------------------.
| Print this RULE's RHS on OUT.  |
`-------------------------------*/

void
rule_rhs_print (rule_t *rule, FILE *out)
{
  if (*rule->rhs >= 0)
    {
      item_number_t *r;
      for (r = rule->rhs; *r >= 0; r++)
	fprintf (out, " %s", symbols[*r]->tag);
      fputc ('\n', out);
    }
  else
    {
      fprintf (out, " /* %s */\n", _("empty"));
    }
}


/*-------------------------.
| Print this RULE on OUT.  |
`-------------------------*/

void
rule_print (rule_t *rule, FILE *out)
{
  fprintf (out, "%s:", rule->lhs->tag);
  rule_rhs_print (rule, out);
}


/*------------------------.
| Dump RITEM for traces.  |
`------------------------*/

void
ritem_print (FILE *out)
{
  unsigned int i;
  fputs ("RITEM\n", out);
  for (i = 0; i < nritems; ++i)
    if (ritem[i] >= 0)
      fprintf (out, "  %s", symbols[ritem[i]]->tag);
    else
      fprintf (out, "  (rule %d)\n", item_number_as_rule_number (ritem[i]));
  fputs ("\n\n", out);
}


/*------------------------------------------.
| Return the size of the longest rule RHS.  |
`------------------------------------------*/

size_t
ritem_longest_rhs (void)
{
  int max = 0;
  rule_number_t r;

  for (r = 0; r < nrules; ++r)
    {
      int length = rule_rhs_length (&rules[r]);
      if (length > max)
	max = length;
    }

  return max;
}


/*-----------------------------------------------------------------.
| Print the grammar's rules that match FILTER on OUT under TITLE.  |
`-----------------------------------------------------------------*/

void
grammar_rules_partial_print (FILE *out, const char *title,
			     rule_filter_t filter)
{
  int r;
  bool first = TRUE;
  symbol_t *previous_lhs = NULL;

  /* rule # : LHS -> RHS */
  for (r = 0; r < nrules + nuseless_productions; r++)
    {
      if (filter && !filter (&rules[r]))
	continue;
      if (first)
	fprintf (out, "%s\n\n", title);
      else if (previous_lhs && previous_lhs != rules[r].lhs)
	fputc ('\n', out);
      first = FALSE;
      rule_lhs_print (&rules[r], previous_lhs, out);
      rule_rhs_print (&rules[r], out);
      previous_lhs = rules[r].lhs;
    }
  if (!first)
    fputs ("\n\n", out);
}


/*------------------------------------------.
| Print the grammar's useful rules on OUT.  |
`------------------------------------------*/

void
grammar_rules_print (FILE *out)
{
  grammar_rules_partial_print (out, _("Grammar"), rule_useful_p);
}


/*-------------------.
| Dump the grammar.  |
`-------------------*/

void
grammar_dump (FILE *out, const char *title)
{
  fprintf (out, "%s\n\n", title);
  fprintf (out,
	   "ntokens = %d, nvars = %d, nsyms = %d, nrules = %d, nritems = %d\n\n",
	   ntokens, nvars, nsyms, nrules, nritems);


  fprintf (out, "Variables\n---------\n\n");
  {
    symbol_number_t i;
    fprintf (out, "Value  Sprec  Sassoc  Tag\n");

    for (i = ntokens; i < nsyms; i++)
      fprintf (out, "%5d  %5d   %5d  %s\n",
	       i,
	       symbols[i]->prec, symbols[i]->assoc,
	       symbols[i]->tag);
    fprintf (out, "\n\n");
  }

  fprintf (out, "Rules\n-----\n\n");
  {
    rule_number_t i;
    fprintf (out, "Num (Prec, Assoc, Useful, Ritem Range) Lhs -> Rhs (Ritem range) [Num]\n");
    for (i = 0; i < nrules + nuseless_productions; i++)
      {
	rule_t *rule = &rules[i];
	item_number_t *r = NULL;
	unsigned int rhs_itemno = rule->rhs - ritem;
  	unsigned int rhs_count = 0;
	/* Find the last RHS index in ritems. */
	for (r = rule->rhs; *r >= 0; ++r)
	  ++rhs_count;
	fprintf (out, "%3d (%2d, %2d, %2d, %2u-%2u)   %2d ->",
		 i,
		 rule->prec ? rule->prec->prec : 0,
		 rule->prec ? rule->prec->assoc : 0,
		 rule->useful,
		 rhs_itemno,
		 rhs_itemno + rhs_count - 1,
		 rule->lhs->number);
	/* Dumped the RHS. */
	for (r = rule->rhs; *r >= 0; r++)
	  fprintf (out, " %3d", *r);
	fprintf (out, "  [%d]\n", item_number_as_rule_number (*r));
      }
  }
  fprintf (out, "\n\n");

  fprintf (out, "Rules interpreted\n-----------------\n\n");
  {
    rule_number_t r;
    for (r = 0; r < nrules + nuseless_productions; r++)
      {
	fprintf (out, "%-5d  ", r);
	rule_print (&rules[r], out);
      }
  }
  fprintf (out, "\n\n");
}


/*------------------------------------------------------------------.
| Report on STDERR the rules that are not flagged USEFUL, using the |
| MESSAGE (which can be `useless rule' when invoked after grammar   |
| reduction, or `never reduced' after conflicts were taken into     |
| account).                                                         |
`------------------------------------------------------------------*/

void
grammar_rules_never_reduced_report (const char *message)
{
  rule_number_t r;
  for (r = 0; r < nrules ; ++r)
    if (!rules[r].useful)
      {
	LOCATION_PRINT (stderr, rules[r].location);
	fprintf (stderr, ": %s: %s: ",
		 _("warning"), message);
	rule_print (&rules[r], stderr);
      }
}

void
grammar_free (void)
{
  XFREE (ritem);
  free (rules);
  XFREE (token_translations);
  /* Free the symbol table data structure.  */
  symbols_free ();
  free_merger_functions ();
}
