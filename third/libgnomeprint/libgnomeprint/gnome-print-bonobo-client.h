/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  bonobo-print-client.c: a print client interface for compound documents.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Michael Meeks <mmeeks@gnu.org>
 *
 *  Copyright (C) 1999-2002 Ximian, Inc. and authors
 *
 */

#ifndef __GNOME_PRINT_BONOBO_CLIENT_H__
#define __GNOME_PRINT_BONOBO_CLIENT_H__

#include <bonobo/bonobo-object.h>
#include <libgnomeprint/gnome-print-meta.h>

G_BEGIN_DECLS

typedef struct _GnomePrintBonoboDimensions GnomePrintBonoboDimensions;
typedef struct _GnomePrintBonoboData       GnomePrintBonoboData;

/* Setup the dimensions */
GnomePrintBonoboDimensions *gnome_print_bonobo_dimensions_new       (double                 width,
								     double                 height);
void                        gnome_print_bonobo_dimensions_free      (GnomePrintBonoboDimensions *dims);

GnomePrintBonoboDimensions *gnome_print_bonobo_dimensions_new_full  (double                 width,
								     double                 height,
								     double                 width_first_page,
								     double                 width_per_page,
								     double                 height_first_page,
								     double                 height_per_page);

/* Remote render */
GnomePrintBonoboData       *gnome_print_bonobo_client_remote_render (Bonobo_Print                      print,
								     const GnomePrintBonoboDimensions *dims,
								     CORBA_Environment                *opt_ev);
void                        gnome_print_bonobo_data_free            (GnomePrintBonoboData             *pd);

/* Then re-render localy */
void                        gnome_print_bonobo_data_re_render       (GnomePrintContext           *ctx,
								     double                       x,
								     double                       y,
								     GnomePrintBonoboData        *pd,
								     double                       meta_x,
								     double                       meta_y);
GnomePrintMeta             *gnome_print_bonobo_data_get_meta        (GnomePrintBonoboData        *pd);

G_END_DECLS

#endif /* ! ___GNOME_PRINT_BONOBO_CLIENT_H__ */
