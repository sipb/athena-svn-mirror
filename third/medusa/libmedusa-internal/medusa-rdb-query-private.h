/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

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


#ifndef MEDUSA_RDB_QUERY_PRIVATE_H
#define MEDUSA_RDB_QUERY_PRIVATE_H

struct MedusaRDBQueryCriterion {
        MedusaRDBField *query_field;
        /* The operator, equals, regexp_match, etc. */
        MedusaRDBOperator operator;
        /* the argument: in the criterion "equals foo" 
           foo is the operand  */
        MedusaRDBOperand operand;
        /* Any data needed to decode the field in
           the database */
        gpointer field_decoding_data;
        /* the value the query should have
           TRUE for normal queries
           FALSE for negative queries
           (eg doesn't equal foo) */
        gboolean result_desired;
};


#endif /* MEDUSA_RDB_QUERY_PRIVATE_H */
