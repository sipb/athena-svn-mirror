/*
 * LIBGAIL-GNOME -  Accessibility Toolkit Implementation for Bonobo
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GAIL_GNOME_DEBUG_H__
#define __GAIL_GNOME_DEBUG_H__

#include <glib/gmacros.h>

G_BEGIN_DECLS

#ifdef GAIL_GNOME_DEBUG

#include <stdio.h>

#define dprintf(format...) fprintf (stderr, format)

#else /* G_ENABLE_DEBUG */

static inline void dprintf (const char *format, ...) { };

#endif /* G_ENABLE_DEBUG */

G_END_DECLS

#endif /* __GAIL_GNOME_DEBUG_H__ */
