/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 */

/* medusa-query-clauses.c:  System for creating a
   vocabulary for medusa databases (phrases that correspond to
   a function that can find things that satisfy a phrase) */

#include <glib.h>
#include <string.h>

#include "medusa-query-clauses.h"




struct MedusaQueryClauses {
  GList *clauses;
  int ref_count;
};


struct MedusaQueryClause {
	char *field_name;
	char *operator_name;
	MedusaQueryFunc evaluate;
	MedusaArgumentType argument_type;	
	int ref_count;
};



static void                    medusa_query_clauses_destroy             (MedusaQueryClauses *clause);

static MedusaQueryClause *     medusa_query_clause_new                  (char *field_name,
									 char *operator_name,
									 MedusaQueryFunc evaluate,
									 MedusaArgumentType argument_type);
/*static void                    medusa_query_clause_ref                  (MedusaQueryClause *clause); */
static void                    medusa_query_clause_unref                (MedusaQueryClause *clause);
static void                    medusa_query_clause_destroy              (MedusaQueryClause *clause);

MedusaQueryClauses *
medusa_query_clauses_new (void)
{
	MedusaQueryClauses *clauses;
	
	clauses = g_new0 (MedusaQueryClauses, 1);
	clauses->ref_count = 1;
	
	return clauses;
}

void
medusa_query_clauses_add_clause (MedusaQueryClauses *clauses,
				 char *field_name,
				 char *operator_name,
				 MedusaQueryFunc evaluate,
				 MedusaArgumentType argument_type)
{
	MedusaQueryClause *new_clause;
	
	new_clause = medusa_query_clause_new (field_name, operator_name, evaluate, argument_type);
	clauses->clauses = g_list_prepend (clauses->clauses, new_clause);
}

MedusaQueryFunc
medusa_query_clauses_get_function (MedusaQueryClauses *clauses,
				   const char *clause,
				   MedusaArgumentType *argument_type)
{
	GList *i;
	char **words, *field_name, *operator_name;
	
	words = g_strsplit (clause, " ", 3);
	field_name = words[0];
	operator_name = words[1];
	
	for (i = clauses->clauses; i != NULL; i = i->next) {
		if (strcmp (((MedusaQueryClause *) i->data)->field_name, field_name) == 0 &&
		    strcmp (((MedusaQueryClause *) i->data)->operator_name, operator_name) == 0) {
			*argument_type = ((MedusaQueryClause *) i->data)->argument_type;
			g_strfreev (words);
			return ((MedusaQueryClause *) i->data)->evaluate;
		}
	}

	g_strfreev (words);
	return NULL;
}

void
medusa_query_clauses_ref (MedusaQueryClauses *clauses)
{
	g_assert (clauses->ref_count > 0);
	clauses->ref_count++;
}

void
medusa_query_clauses_unref (MedusaQueryClauses *clauses)
{
	g_assert (clauses->ref_count > 0);
	if (clauses->ref_count == 1) {
		medusa_query_clauses_destroy (clauses);
	} else {
		clauses->ref_count--;
	}
}



gboolean
medusa_query_result_succeeded (MedusaQueryResult *result)
{
	return result->result;
}

gpointer
medusa_query_result_get_next_record (MedusaQueryResult *result)
{
	return result->next_record;
}

void
medusa_query_result_set_succeeded (MedusaQueryResult *result,
				   gboolean value)
{
	result->result = value;
}

void
medusa_query_result_set_next_record (MedusaQueryResult *result,
				     MedusaRDBRecordNumbers *next_record)
{
	result->next_record = next_record;
}

void
medusa_query_result_free (MedusaQueryResult *result)
{
	g_free (result);
}

static void
medusa_query_clauses_destroy (MedusaQueryClauses *clauses)
{
	GList *p;

	for (p = clauses->clauses; p != NULL; p = p->next) {
		medusa_query_clause_unref (p->data);
	}
	g_list_free (clauses->clauses);
	g_free (clauses);
}

static MedusaQueryClause *     
medusa_query_clause_new (char *field_name, 
			 char *operator_name, 
			 MedusaQueryFunc evaluate,
			 MedusaArgumentType argument_type)
{
	MedusaQueryClause *clause;
	
	clause = g_new0 (MedusaQueryClause, 1);
	clause->field_name = g_strdup (field_name);
	clause->operator_name = g_strdup (operator_name);
	clause->evaluate = evaluate;
	clause->argument_type = argument_type;
	
	clause->ref_count = 1;
	return clause;
}

/*
  static void
  medusa_query_clause_ref (MedusaQueryClause *clause)
  {
  g_assert (clause->ref_count > 0);
  clause->ref_count++;
  }
*/

static void
medusa_query_clause_unref (MedusaQueryClause *clause)
{
	g_assert (clause->ref_count > 0);
	if (clause->ref_count == 1) {
		medusa_query_clause_destroy (clause);
	} else {
		clause->ref_count--;
	}
}

static void
medusa_query_clause_destroy (MedusaQueryClause *clause)
{
	g_assert (clause->ref_count == 1);
	
	g_free (clause->field_name);
	g_free (clause->operator_name);
	g_free (clause);
}
