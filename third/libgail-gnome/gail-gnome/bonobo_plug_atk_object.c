/*
 * LIBGAIL-GNOME -  Accessibility Toolkit Implementation for Bonobo
 * Copyright 2002 Sun Microsystems Inc.
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
#include "bonobo_plug_atk_object.h"
#include "gail-gnome-debug.h"

static GQuark quark_private_control = 0;

static BonoboControl *
bonobo_plug_atk_object_get_control (BonoboPlugAtkObject *accessible)
{
  return BONOBO_CONTROL (
		  g_object_get_qdata (G_OBJECT (accessible), quark_private_control));
}

static Accessibility_Accessible
bonobo_plug_atk_object_get_accessible (SpiRemoteObject *remote)
{
  Accessibility_Accessible  retval;
  BonoboControl            *control;
  CORBA_Environment         env;

  dprintf ("Plug => SpiRemoteObjectIface->get_accessibile [%p]\n", remote);

  g_return_val_if_fail (BONOBO_IS_PLUG_ATK_OBJECT (remote), CORBA_OBJECT_NIL);

  control = bonobo_plug_atk_object_get_control (BONOBO_PLUG_ATK_OBJECT (remote));
  g_return_val_if_fail (BONOBO_IS_CONTROL (control), CORBA_OBJECT_NIL);
  
  CORBA_exception_init (&env);

  retval = Bonobo_Control_getAccessible (BONOBO_OBJREF (control), &env);
  if (BONOBO_EX (&env))
	  retval = CORBA_OBJECT_NIL;

  CORBA_exception_free (&env);

  return retval;
}

static void
bonobo_plug_finalize (GObject *object)
{
  BonoboControl *control;

  control = bonobo_plug_atk_object_get_control (BONOBO_PLUG_ATK_OBJECT (object));
  if (control) {
    g_object_unref (G_OBJECT (control));
    g_object_set_qdata (object, quark_private_control, NULL);
  }
}

static gint
bonobo_plug_get_index_in_parent (AtkObject *obj)
{
  return 0;
}

static void
bonobo_plug_atk_object_class_init (BonoboPlugAtkObjectClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

  gobject_class->finalize = bonobo_plug_finalize;

  class->get_index_in_parent = bonobo_plug_get_index_in_parent;

  quark_private_control = g_quark_from_static_string ("gail-gnome-private-control");
}

static void
bonobo_plug_atk_object_init  (BonoboPlugAtkObject        *accessible,
			      BonoboPlugAtkObjectClass   *klass)
{
	g_assert (ATK_IS_OBJECT (accessible));
	ATK_OBJECT (accessible)->role = ATK_ROLE_WINDOW;
}

static void
bonobo_plug_atk_object_remote_init (SpiRemoteObjectIface *iface)
{
  iface->get_accessible = bonobo_plug_atk_object_get_accessible;
}

GType
bonobo_plug_atk_object_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static GTypeInfo typeInfo =
      {
        0,
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) bonobo_plug_atk_object_class_init,
        (GClassFinalizeFunc) NULL,
        NULL, 0, 0,
        (GInstanceInitFunc) bonobo_plug_atk_object_init,
      };

      static const GInterfaceInfo remote_info =
      {
	(GInterfaceInitFunc) bonobo_plug_atk_object_remote_init,
	NULL,
	NULL
      };

      AtkObjectFactory *factory;
      GType             derived_type;
      GTypeQuery        query;

      factory = atk_registry_get_factory (atk_get_default_registry (), GTK_TYPE_PLUG);
      derived_type = atk_object_factory_get_accessible_type (factory);
      g_type_query (derived_type, &query);

      typeInfo.class_size = query.class_size;
      typeInfo.instance_size = query.instance_size;

      type = g_type_register_static (derived_type, "BonoboPlugAtkObject", &typeInfo, 0) ;

      g_type_add_interface_static (type, SPI_TYPE_REMOTE_OBJECT, &remote_info);
    }

  return type;
}

AtkObject *
bonobo_plug_atk_object_new (BonoboPlug *plug)
{
  BonoboPlugAtkObject *retval;

  dprintf ("bonobo_plug_atk_object_new [%p]\n", plug);

  g_return_val_if_fail (GTK_IS_PLUG (plug), NULL);

  retval = g_object_new (BONOBO_TYPE_PLUG_ATK_OBJECT, NULL);

  atk_object_initialize (ATK_OBJECT (retval), GTK_WIDGET (plug));

  g_object_ref (G_OBJECT (plug->control));

  g_object_set_qdata (G_OBJECT (retval), quark_private_control, plug->control);

  return ATK_OBJECT (retval);
}
