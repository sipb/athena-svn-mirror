#ifndef _GNOME_PRINT_RBUF_PRIVATE_H_
#define _GNOME_PRINT_RBUF_PRIVATE_H_

/*
 *  Copyright (C) 2000-2001 Ximian Inc. and authors
 *
 *  Authors:
 *    Lauris Kaplinski (lauris@ariman.ee)
 *
 *  Driver that renders into transformed rectangular RGB(A) buffer
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
 */

#include <libgnomeprint/gnome-print-private.h>

typedef struct _GnomePrintRBufPrivate GnomePrintRBufPrivate;

struct _GnomePrintRBuf {
	GnomePrintContext pc;

	GnomePrintRBufPrivate * private;
};

struct _GnomePrintRBufClass {
	GnomePrintContextClass parent_class;
};

#endif
