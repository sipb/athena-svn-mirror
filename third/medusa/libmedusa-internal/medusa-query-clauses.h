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

/* medusa-query-clauses.h:  System for creating a
   vocabulary for medusa databases (phrases that correspond to
   a function that can find things that satisfy a phrase) */

#ifndef MEDUSA_QUERY_CLAUSE_H
#define MEDUSA_QUERY_CLAUSE_H

#include "medusa-rdb-table.h"


typedef struct MedusaQueryClauses MedusaQueryClauses;
typedef struct MedusaQueryClause MedusaQueryClause;

typedef struct MedusaQueryResult  MedusaQueryResult;


struct MedusaQueryResult {
  gboolean result;
  gpointer next_record;
};

typedef enum {
        MEDUSA_ARGUMENT_TYPE_STRING,
        MEDUSA_ARGUMENT_TYPE_INTEGER,
	MEDUSA_ARGUMENT_TYPE_NONE
} MedusaArgumentType;


typedef gboolean (* MedusaQueryFunc)   (gpointer database,
					gpointer current_record,
					gpointer arg);

MedusaQueryClauses *    medusa_query_clauses_new                  (void);
void                    medusa_query_clauses_ref                  (MedusaQueryClauses *clause);
void                    medusa_query_clauses_unref                (MedusaQueryClauses *clause);
void                    medusa_query_clauses_add_clause           (MedusaQueryClauses *clauses,
								   char *field_name,
								   char *operator_name,
								   MedusaQueryFunc evaluate,
								   MedusaArgumentType argument_type);
MedusaQueryFunc         medusa_query_clauses_get_function         (MedusaQueryClauses *clauses,
								   const char *clause,
								   MedusaArgumentType *argument_type);

gboolean                medusa_query_result_succeeded             (MedusaQueryResult *result);
gpointer                medusa_query_result_get_next_record       (MedusaQueryResult *result);
void                    medusa_query_result_set_succeeded         (MedusaQueryResult *result,
								   gboolean value);
void                    medusa_query_result_set_next_record       (MedusaQueryResult *result,
								   MedusaRDBRecordNumbers *next_record);
void                    medusa_query_result_free                  (MedusaQueryResult *result);


#endif /* MEDUSA_QUERY_CLAUSE_H */
