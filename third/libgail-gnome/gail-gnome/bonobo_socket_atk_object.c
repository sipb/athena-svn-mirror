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

#include <libspi/remoteobject.h>

#include <bonobo/bonobo-exception.h>
#include "bonobo_socket_atk_object.h"
#include "gail-gnome-debug.h"

static GQuark quark_private_frame = 0;

static BonoboControlFrame *
bonobo_socket_atk_object_get_frame (BonoboSocketAtkObject *accessible)
{
  return g_object_get_qdata (G_OBJECT (accessible), quark_private_frame);
}

static Accessibility_Accessible
bonobo_socket_atk_object_get_accessible (SpiRemoteObject *remote)
{
  Accessibility_Accessible  retval;
  BonoboControlFrame       *frame;
  Bonobo_Control            control;
  CORBA_Environment         env;

  dprintf ("Socket => SpiRemoteObjectClass->get_accessibile [%p]\n", remote);

  g_return_val_if_fail (BONOBO_IS_SOCKET_ATK_OBJECT (remote), CORBA_OBJECT_NIL);

  frame = bonobo_socket_atk_object_get_frame (BONOBO_SOCKET_ATK_OBJECT (remote));
  g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), CORBA_OBJECT_NIL);
  control = bonobo_control_frame_get_control (frame);
  g_return_val_if_fail (control != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

  CORBA_exception_init (&env);

  retval = Bonobo_Control_getAccessible (control, &env);
  g_return_val_if_fail (retval != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

  if (BONOBO_EX (&env))
	  retval = CORBA_OBJECT_NIL;

  CORBA_exception_free (&env);

  return retval;
}

static void
bonobo_plug_finalize (GObject *object)
{
  BonoboControlFrame *frame;

  frame = bonobo_socket_atk_object_get_frame (BONOBO_SOCKET_ATK_OBJECT (object));
  if (frame) {
    g_object_unref (G_OBJECT (frame));
    g_object_set_qdata (object, quark_private_frame, NULL);
  }
}

static void
bonobo_socket_atk_object_class_init (BonoboSocketAtkObjectClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;

  gobject_class->finalize = bonobo_plug_finalize;

  quark_private_frame = g_quark_from_static_string ("gail-gnome-private-frame");
}

static void
bonobo_socket_atk_object_init  (BonoboSocketAtkObject      *accessible,
				BonoboSocketAtkObjectClass *klass)
{
}

static void
bonobo_socket_atk_object_remote_init (SpiRemoteObjectIface *iface)
{
  iface->get_accessible = bonobo_socket_atk_object_get_accessible;
}

GType
bonobo_socket_atk_object_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static GTypeInfo typeInfo =
      {
        0,
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) bonobo_socket_atk_object_class_init,
        (GClassFinalizeFunc) NULL,
        NULL, 0, 0,
        (GInstanceInitFunc) bonobo_socket_atk_object_init,
      };

      static const GInterfaceInfo remote_info =
      {
	(GInterfaceInitFunc) bonobo_socket_atk_object_remote_init,
	NULL,
	NULL
      };

      AtkObjectFactory *factory;
      GType             derived_type;
      GTypeQuery        query;

      factory = atk_registry_get_factory (atk_get_default_registry (), GTK_TYPE_WIDGET);
      derived_type = atk_object_factory_get_accessible_type (factory);
      g_type_query (derived_type, &query);

      typeInfo.class_size = query.class_size;
      typeInfo.instance_size = query.instance_size;

      type = g_type_register_static (derived_type, "BonoboSocketAtkObject", &typeInfo, 0) ;

      g_type_add_interface_static (type, SPI_TYPE_REMOTE_OBJECT, &remote_info);
    }

  return type;
}

AtkObject *
bonobo_socket_atk_object_new (BonoboSocket *socket)
{
  BonoboSocketAtkObject *retval;

  dprintf ("bonobo_socket_atk_object_new [%p]\n", socket);

  g_return_val_if_fail (BONOBO_IS_SOCKET (socket), NULL);

  retval = g_object_new (BONOBO_TYPE_SOCKET_ATK_OBJECT, NULL);

  atk_object_initialize (ATK_OBJECT (retval), GTK_WIDGET (socket));

  if (socket->frame)
    {
      g_object_ref (G_OBJECT (socket->frame));

      g_object_set_qdata (G_OBJECT (retval), quark_private_frame, socket->frame);
    }

  return ATK_OBJECT (retval);
}
