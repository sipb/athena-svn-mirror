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

#include <atk/atk.h>
#include <libspi/remoteobject.h>
#include "bonobo-control-frame-accessible.h"
#include "gail-gnome-debug.h"

static CORBA_long
impl_bonobo_control_frame_accessible_get_child_count (PortableServer_Servant servant,
						      CORBA_Environment *ev)
{
  BonoboControlFrameAccessible *frame_accessible =
      BONOBO_CONTROL_FRAME_ACCESSIBLE (bonobo_object_from_servant (servant));

  dprintf ("Accessibility::Accessible::_get_ChildCount [%p]\n", frame_accessible);

  g_return_val_if_fail (frame_accessible != NULL, 0);
  g_return_val_if_fail (frame_accessible->control_frame != NULL, 0);
  g_return_val_if_fail (bonobo_control_frame_get_control (frame_accessible->control_frame), 0);
  
  return 1;
}

static Accessibility_Accessible
impl_bonobo_control_frame_accessible_get_child_at_index (PortableServer_Servant servant,
							 CORBA_long index,
							 CORBA_Environment *ev)
{
  BonoboControlFrameAccessible *frame_accessible =
      BONOBO_CONTROL_FRAME_ACCESSIBLE (bonobo_object_from_servant (servant));
  Bonobo_Control control;

  dprintf ("Accessibility::Accessible::getChildAtIndex [%p]\n", frame_accessible);

  g_return_val_if_fail (frame_accessible != NULL, CORBA_OBJECT_NIL);
  g_return_val_if_fail (frame_accessible->control_frame != NULL, CORBA_OBJECT_NIL);
  g_return_val_if_fail (index == 0, CORBA_OBJECT_NIL);
  
  control = bonobo_control_frame_get_control (frame_accessible->control_frame);

  return Bonobo_Control_getAccessible (control, ev);
}

static void
bonobo_control_frame_accessible_init (void)
{
}

static void
bonobo_control_frame_accessible_class_init (BonoboControlFrameAccessibleClass *klass)
{
	SPI_ACCESSIBLE_CLASS (klass)->epv._get_childCount =
		impl_bonobo_control_frame_accessible_get_child_count;
        SPI_ACCESSIBLE_CLASS (klass)->epv.getChildAtIndex =
	        impl_bonobo_control_frame_accessible_get_child_at_index;
}

BONOBO_TYPE_FUNC (BonoboControlFrameAccessible,
		  SPI_ACCESSIBLE_TYPE,
		  bonobo_control_frame_accessible);


BonoboControlFrameAccessible *
bonobo_control_frame_accessible_new (BonoboControlFrame *control_frame)
{
  BonoboControlFrameAccessible *retval;	
  GtkWidget *widget;
  AtkObject *atko;

  g_return_val_if_fail (control_frame != NULL, NULL);
  widget = bonobo_control_frame_get_widget (control_frame);
  g_assert (widget != NULL);
  atko = gtk_widget_get_accessible (widget);
  g_assert (atko != NULL);

  g_assert (SPI_IS_REMOTE_OBJECT (atko));

  retval = BONOBO_CONTROL_FRAME_ACCESSIBLE (
	  spi_accessible_construct (
		  BONOBO_TYPE_CONTROL_FRAME_ACCESSIBLE, atko));

  retval->control_frame = control_frame;

  return retval;
}
