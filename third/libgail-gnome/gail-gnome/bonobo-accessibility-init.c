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

#include <stdio.h>
#include <glib-object.h>
#include <atk/atk.h>
#include <bonobo/bonobo-plug.h>
#include <bonobo/bonobo-socket.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-control-frame.h>
#include <libspi/accessible.h>
#include "bonobo-control-accessible.h"
#include "bonobo-control-frame-accessible.h"
#include "bonobo_plug_atk_object_factory.h"
#include "bonobo_socket_atk_object_factory.h"
#include "gail-gnome-debug.h"

/* For automatic libgnome init */
extern void gnome_accessibility_module_init     (void);
extern void gnome_accessibility_module_shutdown (void);

static BonoboControlClass      *bonobo_control_class;
static BonoboControlFrameClass *bonobo_control_frame_class;

static Bonobo_Unknown
impl_bonobo_control_frame_get_parent_accessible (PortableServer_Servant servant,
						 CORBA_Environment *ev)
{
  BonoboControlFrame *control_frame = BONOBO_CONTROL_FRAME (bonobo_object_from_servant (servant));
  AtkObject          *atko;
  AtkObject          *parent;

  dprintf ("Bonobo::ControlFame::getParentAccessible [%p]\n", control_frame);

  g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), CORBA_OBJECT_NIL);

  atko = gtk_widget_get_accessible (
	  bonobo_control_frame_get_widget (control_frame));

  parent = atk_object_get_parent (atko);  

  return CORBA_Object_duplicate (
	  BONOBO_OBJREF (spi_accessible_new (parent)), ev);
}

static Bonobo_Unknown
impl_bonobo_control_get_accessible (PortableServer_Servant servant,
				    CORBA_Environment *ev)
{
  BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));

  dprintf ("Bonobo::Control::getAccessible [%p]\n", control);

  g_return_val_if_fail (control != NULL, CORBA_OBJECT_NIL);
  
  return CORBA_Object_duplicate (
	  BONOBO_OBJREF (bonobo_control_accessible_new (control)), ev);
}

static void
gail_bonobo_ui_register_atk_factories (void)
{
  AtkRegistry *registry = atk_get_default_registry ();	
  atk_registry_set_factory_type (registry, BONOBO_TYPE_PLUG, BONOBO_TYPE_PLUG_ATK_OBJECT_FACTORY);
  atk_registry_set_factory_type (registry, BONOBO_TYPE_SOCKET, BONOBO_TYPE_SOCKET_ATK_OBJECT_FACTORY);   
}

static void
gail_bonobo_ui_set_accessible_epv (void)
{
  bonobo_control_class       = g_type_class_ref (BONOBO_TYPE_CONTROL);
  bonobo_control_frame_class = g_type_class_ref (BONOBO_TYPE_CONTROL_FRAME);

  bonobo_control_class->epv.getAccessible =
	  		impl_bonobo_control_get_accessible;

  bonobo_control_frame_class->epv.getParentAccessible =
			impl_bonobo_control_frame_get_parent_accessible;

  /* hold the refs, to make sure the vtable doesn't get reset */
}
 
G_GNUC_UNUSED static void
gail_accessibility_finalize (void)
{
  g_type_class_unref (bonobo_control_class);
  g_type_class_unref (bonobo_control_frame_class);
}

static void
gail_accessibility_init (void)
{
  gail_bonobo_ui_set_accessible_epv ();
  gail_bonobo_ui_register_atk_factories (); 
}

int
gtk_module_init(gint *argc, char** argv[])
{
  dprintf ("BonoboUI Accessibility Module loaded\n");

  gail_accessibility_init ();

  return 0;
}

void
gnome_accessibility_module_init (void)
{
  fprintf (stderr, "Bonobo accessibility support initialized\n");
  gail_accessibility_init ();
}

void
gnome_accessibility_module_shutdown (void)
{
  dprintf ("BonoboUI Accessibility Module shutdown\n");
}
