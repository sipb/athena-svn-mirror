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

/* medusa-enums.h -- medusa enumerated types. */

#ifndef MEDUSA_ENUMS_H
#define MEDUSA_ENUMS_H

typedef enum MedusaLogLevel {
	MEDUSA_DB_LOG_NOTHING,            /* Send no messages, ever */
	MEDUSA_DB_LOG_ERRORS,             /* Only send information about errors that occur */
	MEDUSA_DB_LOG_ABBREVIATED,        /* Send information about errors, and list all file names */
	
	MEDUSA_DB_LOG_EVERYTHING,         /* Log each transaction */
	MEDUSA_DB_LOG_TEXT_INDEX_DATA     /* Log amount of space various types of information are
					     taking up in the text index */

} MedusaLogLevel;

#endif /* MEDUSA_ENUMS_H */

