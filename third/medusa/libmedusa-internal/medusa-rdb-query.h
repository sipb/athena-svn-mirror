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

/* medusa-rdb-query.h:  The API for creating relational database queries
   to be run on medusa relational databases */

#ifndef MEDUSA_RDB_QUERY_H
#define MEDUSA_RDB_QUERY_H

#include <glib.h>
#include "medusa-rdb-fields.h"

#ifndef NAME_MAX
#define NAME_MAX 512
#endif

typedef GSList MedusaRDBOperators;
typedef struct MedusaRDBQueryCriterion MedusaRDBQueryCriterion;
typedef GSList MedusaRDBQuery;
typedef GSList MedusaRDBOperands;
typedef gpointer MedusaRDBOperand;


typedef enum {
	MEDUSA_RDB_STRING_EQUALS,
	MEDUSA_RDB_NUMBER_EQUALS,
	MEDUSA_RDB_REGEXP_MATCH,
	MEDUSA_RDB_GREATER_THAN,
	MEDUSA_RDB_LESS_THAN
}  MedusaRDBOperator;





MedusaRDBQuery * medusa_rdb_query_new             (void);
void             medusa_rdb_query_free            (MedusaRDBQuery *query);


MedusaRDBQuery * medusa_rdb_query_add_selection_criterion     (MedusaRDBQuery *query,
							       MedusaRDBField *query_field,
							       MedusaRDBOperator operator,
							       MedusaRDBOperand operand,
							       gboolean value_desired,
							       gpointer field_decoding_data);

gboolean         medusa_rdb_query_match                       (MedusaRDBQuery *query, 
							       MedusaRDBRecord record,
							       MedusaRDBFieldInfo *field_info);

void             medusa_rdb_query_test                        (void);


#endif /* MEDUSA_RDB_QUERY_H */
