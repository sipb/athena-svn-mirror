/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* e-summary-rdf.h
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Iain Holmes
 */

#ifndef __E_SUMMARY_RDF_H__
#define __E_SUMMARY_RDF_H__

#include "e-summary-type.h"

typedef struct _ESummaryRDF ESummaryRDF;

char *e_summary_rdf_get_html (ESummary *summary);
void e_summary_rdf_init (ESummary *summary);
void e_summary_rdf_reconfigure (ESummary *summary);
void e_summary_rdf_free (ESummary *summary);
gboolean e_summary_rdf_update (ESummary *summary);

#endif