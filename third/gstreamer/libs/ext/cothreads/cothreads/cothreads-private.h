/* Pthread-friendly coroutines with pth
 * Copyright (C) 2002 Andy Wingo <wingo@pobox.com>
 *
 * cothreads-private.h: private prototypes
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

#ifndef __COTHREADS_PRIVATE_H__
#define __COTHREADS_PRIVATE_H__

#include "cothreads.h"
#include "config-private.h"
#include "cothreads-pth-private.h"

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif
#define G_LOG_DOMAIN "Cothreads"

#ifdef G_HAVE_ISO_VARARGS

#ifdef COTHREADS_DEBUG_ENABLED
#define COTHREADS_DEBUG(...) \
  { \
     fprintf(stderr, __VA_ARGS__); \
     fprintf(stderr, "\n"); \
  }
#else
#define COTHREADS_DEBUG(...)
#endif

#elif defined(G_HAVE_GNUC_VARARGS)

#ifdef COTHREADS_DEBUG_ENABLED
#define COTHREADS_DEBUG(text, args...) fprintf(stderr, text "\n", ##args)
#else
#define COTHREADS_DEBUG(text, args...)
#endif

#endif


#endif /* __COTHREADS_PRIVATE_H__ */
