/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oaf-async: A library for accessing oafd in a nice way.
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
 *  Author: Mathieu Lacage <mathieu@eazel.com>
 *
 */


#ifndef BONOBO_ACTIVATION_ASYNC_CORBA_H
#define BONOBO_ACTIVATION_ASYNC_CORBA_H

#include <bonobo-activation/bonobo-activation-async.h>
#include <bonobo-activation/Bonobo_Activation_types.h>

CORBA_Object 
bonobo_activation_async_corba_callback_new (BonoboActivationCallback callback,
                                            gpointer user_data,
                                            CORBA_Environment * ev);



#endif /* BONOBO_ACTIVATION_ASYNC_CORBA_H */
