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

#ifndef BONOBO_SOCKET_ATK_OBJECT_FACTORY_H_
#define BONOBO_SOCKET_ATK_OBJECT_FACTORY_H_

#include <atk/atk.h>

G_BEGIN_DECLS

#define BONOBO_TYPE_SOCKET_ATK_OBJECT_FACTORY                    (bonobo_socket_atk_object_factory_get_type ())
#define BONOBO_SOCKET_ATK_OBJECT_FACTORY(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BONOBO_TYPE_SOCKET_ATK_OBJECT_FACTORY, BonoboSocketAtkObjectFactory))
#define BONOBO_SOCKET_ATK_OBJECT_FACTORY_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_SOCKET_ATK_OBJECT_FACTORY, BonoboSocketAtkObjectFactoryClass))
#define BONOBO_IS_SOCKET_ATK_OBJECT_FACTORY(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BONOBO_TYPE_SOCKET_ATK_OBJECT_FACTORY))
#define BONOBO_IS_SOCKET_ATK_OBJECT_FACTORY_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BONOBO_TYPE_SOCKET_ATK_OBJECT_FACTORY))
#define BONOBO_SOCKET_ATK_OBJECT_FACTORY_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BONOBO_TYPE_SOCKET_ATK_OBJECT_FACTORY, BonoboSocketAtkObjectFactoryClass))

typedef struct _BonoboSocketAtkObjectFactory                   BonoboSocketAtkObjectFactory;
typedef struct _BonoboSocketAtkObjectFactoryClass              BonoboSocketAtkObjectFactoryClass;

struct _BonoboSocketAtkObjectFactory
{
  AtkObjectFactory parent;
};

struct _BonoboSocketAtkObjectFactoryClass
{
  AtkObjectFactoryClass parent_class;
};

GType bonobo_socket_atk_object_factory_get_type();

AtkObjectFactory *bonobo_socket_atk_object_factory_new();

G_END_DECLS

#endif /* BONOBO_SOCKET_ATK_OBJECT_FACTORY_H_ */
