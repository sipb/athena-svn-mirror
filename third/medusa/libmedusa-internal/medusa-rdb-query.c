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

#include <medusa-rdb-query.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <medusa-test.h>

#include "medusa-rdb-query-private.h"


static MedusaRDBQueryCriterion *
medusa_rdb_query_criterion_new      (MedusaRDBField *query_field,
				     MedusaRDBOperator operator,
				     MedusaRDBOperand operand,
				     gboolean result_desired,
				     gpointer field_decoding_data);

static gboolean  medusa_rdb_query_criterion_match    (MedusaRDBQueryCriterion *query_criterion,
						      MedusaRDBRecord record,
						      MedusaRDBFieldInfo *field_info) ;

static void      medusa_rdb_query_criterion_free     (gpointer data,
						      gpointer user_data);

MedusaRDBQuery *
medusa_rdb_query_new (void)
{
	MedusaRDBQuery *query;

	query = g_new0 (MedusaRDBQuery, 1);
	return query;
}

void
medusa_rdb_query_free (MedusaRDBQuery *query)
{
	g_slist_foreach (query, medusa_rdb_query_criterion_free, NULL);
	g_slist_free (query);
}

static void
medusa_rdb_query_criterion_free (gpointer data,
				 gpointer user_data)
{
	MedusaRDBQueryCriterion *criterion;

	criterion = (MedusaRDBQueryCriterion *) data;
	/* FIXME bugzilla.eazel.com 2716:  
	   Should the fields be freed also? */
	g_free (criterion);
}

MedusaRDBQueryCriterion *
medusa_rdb_query_criterion_new (MedusaRDBField *query_field,
				MedusaRDBOperator operator,
				MedusaRDBOperand operand,
				gboolean result_desired,
				gpointer field_decoding_data)
{
	MedusaRDBQueryCriterion *query_criterion;
  
	query_criterion = g_new0 (MedusaRDBQueryCriterion, 1);
	query_criterion->query_field = query_field;
	query_criterion->operator = operator;
	query_criterion->operand = operand;
	query_criterion->result_desired = result_desired;
	query_criterion->field_decoding_data = field_decoding_data;

	return query_criterion;
}

MedusaRDBQuery *
medusa_rdb_query_add_selection_criterion (MedusaRDBQuery *query,
					  MedusaRDBField *query_field,
					  MedusaRDBOperator operator,
					  MedusaRDBOperand operand,
					  gboolean result_desired,
					  gpointer field_decoding_data)
{
	MedusaRDBQueryCriterion *query_criterion;
  
  
	query_criterion = medusa_rdb_query_criterion_new(query_field,
							 operator, 
							 operand, 
							 result_desired,
							 field_decoding_data);
	query = g_slist_prepend (query, query_criterion);

	return query;
}


static gboolean
medusa_rdb_query_criterion_match (MedusaRDBQueryCriterion *query_criterion,
				  MedusaRDBRecord record,
				  MedusaRDBFieldInfo *field_info) 
{
	char decoded_data[NAME_MAX];
	static char *last_pattern_as_text = NULL;
	static regex_t *pattern_data;
	int reg_comp_result;
  
	/* If the criterion is blank, it always matches */
	if (query_criterion == NULL) {
		return TRUE;
	}
  
	/* FIXME bugzilla.eazel.com 2646: 
	   This is probably bad.  How to fix it ? */
	memset (decoded_data, 0, NAME_MAX);
	medusa_rdb_record_get_field_value
		(record, field_info,
		 query_criterion->query_field->field_title,
		 query_criterion->field_decoding_data,
		 decoded_data);

	switch (query_criterion->operator) {
	case MEDUSA_RDB_STRING_EQUALS:
		if (strcmp ((gchar *)(query_criterion->operand),
			    (gchar *)(decoded_data)) == 0) {
			return (query_criterion->result_desired ? TRUE : FALSE);
			
		}
		else {
			return (query_criterion->result_desired ? FALSE : TRUE);
			
		}
		break;
	case MEDUSA_RDB_REGEXP_MATCH:
		/* if no last pattern, just go */
		if (last_pattern_as_text == NULL) {
			last_pattern_as_text = g_strdup (query_criterion->operand);
			pattern_data = g_new0 (regex_t, 1);
			reg_comp_result = regcomp (pattern_data, 
						   query_criterion->operand, 
						   REG_ICASE | REG_NOSUB);
			g_return_val_if_fail (reg_comp_result == 0, FALSE);
		}
		/* If we've changed patterns, get rid of the old stuff, 
		   and make new ones */
		if (strcmp (last_pattern_as_text, query_criterion->operand)) {
			g_free (last_pattern_as_text);
			last_pattern_as_text = g_strdup (query_criterion->operand);
			reg_comp_result = regcomp (pattern_data, 
						   query_criterion->operand, 
						   REG_ICASE | REG_NOSUB);
			g_return_val_if_fail (reg_comp_result == 0, FALSE);
		}
		if (regexec (pattern_data, (char *) decoded_data, 0, NULL, 0) == 0) {
			return (query_criterion->result_desired ? TRUE : FALSE);
		}
		else {
			return (query_criterion->result_desired ? FALSE : TRUE);
		}
		break;
	case MEDUSA_RDB_NUMBER_EQUALS:
		if (*(int *) decoded_data == *(int *) query_criterion->operand) {
			return (query_criterion->result_desired ? TRUE : FALSE);
		}
		else {
			return (query_criterion->result_desired ? FALSE : TRUE);
			
		}
		break;
	case MEDUSA_RDB_GREATER_THAN:
		if (*(int *) decoded_data > *(int *) query_criterion->operand) {
			return (query_criterion->result_desired ? TRUE : FALSE);
			
		}
		else {
			return (query_criterion->result_desired ? FALSE : TRUE);
			
		}
		break;
	case MEDUSA_RDB_LESS_THAN:
		if (*(int *) decoded_data < *(int *) query_criterion->operand) {
			return (query_criterion->result_desired ? TRUE : FALSE);
		}
		else {
			return (query_criterion->result_desired ? FALSE : TRUE);
		}
		break;
	}
	printf ("Testing data %swith operand %s\n", decoded_data, (char *) query_criterion->operand);
	return FALSE;
}


gboolean
medusa_rdb_query_match (MedusaRDBQuery *query, 
			MedusaRDBRecord record,
			MedusaRDBFieldInfo *field_info) 
{
	MedusaRDBQuery *query_iterator;
  
	for (query_iterator = query; query_iterator != NULL; query_iterator = query_iterator->next) {
		if (!medusa_rdb_query_criterion_match ((MedusaRDBQueryCriterion *) query_iterator->data,
						       record, field_info)) {
			break;
		}
#ifdef QUERY_DEBUG
		g_message ("One criterion matched");
#endif
	}
  
	if (query_iterator == NULL) {
#ifdef QUERY_DEBUG 
		g_message ("Query match!");
#endif
		return TRUE;
	}
	else {
		return FALSE;
	}
  
}


/* This takes the file system table from the test directory indexing */
void 
medusa_rdb_query_test (void)
{
	/* Test one query for each attribute and possible operator */
	/* Then test a few combinations to make sure booleans are working */
}
