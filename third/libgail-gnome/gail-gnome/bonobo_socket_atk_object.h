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

#ifndef BONOBO_SOCKET_ATK_OBJECT_H_
#define BONOBO_SOCKET_ATK_OBJECT_H_

#include <gtk/gtkaccessible.h>
#include <bonobo/bonobo-socket.h>

G_BEGIN_DECLS

#define BONOBO_TYPE_SOCKET_ATK_OBJECT            (bonobo_socket_atk_object_get_type ())
#define BONOBO_SOCKET_ATK_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BONOBO_TYPE_SOCKET_ATK_OBJECT, BonoboSocketAtkObject))
#define BONOBO_SOCKET_ATK_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_SOCKET_ATK_OBJECT, BonoboSocketAtkObjectClass))
#define BONOBO_IS_SOCKET_ATK_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BONOBO_TYPE_SOCKET_ATK_OBJECT))
#define BONOBO_IS_SOCKET_ATK_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BONOBO_TYPE_SOCKET_ATK_OBJECT))
#define BONOBO_SOCKET_ATK_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BONOBO_TYPE_SOCKET_ATK_OBJECT, BonoboSocketAtkObjectClass))

typedef struct _BonoboSocketAtkObject      BonoboSocketAtkObject;
typedef struct _BonoboSocketAtkObjectClass BonoboSocketAtkObjectClass;

struct _BonoboSocketAtkObject
{
  GtkAccessible parent;
};

struct _BonoboSocketAtkObjectClass
{
  GtkAccessibleClass parent_class;
};

GType      bonobo_socket_atk_object_get_type ();

AtkObject *bonobo_socket_atk_object_new (BonoboSocket *socket);

G_END_DECLS

#endif /* BONOBO_SOCKET_ATK_OBJECT_H_ */
