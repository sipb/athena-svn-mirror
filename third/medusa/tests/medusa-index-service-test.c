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
 *  Authors: Rebecca Schulman <rebecka@eazel.com>
 *  
 */


/* medusa-index-service-test.c -- API for externals users of the medusa
   indexing service. */


#include <medusa-index-service.h>
#include <glib.h>
#include <stdio.h>
#include <time.h>

int main ()
{
  MedusaIndexingRequestResult result;
  time_t last_index_unix_time;
  struct tm *last_index_time;

  result = medusa_index_service_request_reindex ();

  switch (result) {
  case MEDUSA_INDEXER_REQUEST_OK:
    printf ("result of index request was ok\n");
    break;
  case MEDUSA_INDEXER_ERROR_BUSY:
    printf ("indexer is busy now, no luck\n");
    break;
  case MEDUSA_INDEXER_ERROR_NO_RESPONSE:
    printf ("the indexer didn't write back!\n");
    break;
  case MEDUSA_INDEXER_ERROR_NO_INDEXER_PRESENT:
    printf ("no indexer is running\n");
    break;
  default:
    g_assert_not_reached ();
  }

  last_index_unix_time = medusa_index_service_get_last_index_update_time ();
  last_index_time = localtime (&last_index_unix_time);
  if (last_index_unix_time == -1) {
          printf ("Indexing error occurred\n");
  }
  printf ("index was last updated at %d/%d/%d, %d:%d:%d\n", 
	  last_index_time->tm_mon + 1, last_index_time->tm_mday,
	  last_index_time->tm_year + 1900, 
	  last_index_time->tm_hour, last_index_time->tm_min,
	  last_index_time->tm_sec);
  return 0;
}
