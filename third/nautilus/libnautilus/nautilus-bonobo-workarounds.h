/* -*- Mode: C; tab-width: 8; indent-tabs-mode: 8; c-basic-offset: 8 -*- */

/*
 *  libnautilus: A library for nautilus view implementations.
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Darin Adler <darin@eazel.com>
 *
 */

#ifndef NAUTILUS_BONOBO_WORKAROUNDS_H
#define NAUTILUS_BONOBO_WORKAROUNDS_H

#include <bonobo/bonobo-object.h>

/* Gets a single global one. */
POA_Bonobo_Unknown__epv *nautilus_bonobo_object_get_epv (void);
POA_Bonobo_Stream__epv * nautilus_bonobo_stream_get_epv (void);

#endif /* NAUTILUS_BONOBO_WORKAROUNDS_H */
