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
#include "bonobo-control-accessible.h"
#include "gail-gnome-debug.h"

#define PARENT_TYPE SPI_ACCESSIBLE_TYPE

static Accessibility_Accessible
impl_bonobo_control_accessible_get_parent (PortableServer_Servant  servant,
					   CORBA_Environment      *ev)
{
  BonoboControlAccessible *control_accessible;
  Bonobo_ControlFrame      control_frame;

  control_accessible = BONOBO_CONTROL_ACCESSIBLE (
		  		bonobo_object_from_servant (servant));

  dprintf ("Accessibility::Accessible::_get_parent [%p]\n", control_accessible);
  
  g_return_val_if_fail (control_accessible != NULL, CORBA_OBJECT_NIL);
  g_return_val_if_fail (control_accessible->control != NULL, CORBA_OBJECT_NIL);

  control_frame = bonobo_control_get_control_frame (control_accessible->control, ev);

  return Bonobo_ControlFrame_getParentAccessible (control_frame, ev);
}

static void
bonobo_control_accessible_init (void)
{
}

static void
bonobo_control_accessible_class_init (BonoboControlAccessibleClass *klass)
{
  SPI_ACCESSIBLE_CLASS (klass)->epv._get_parent =
	  impl_bonobo_control_accessible_get_parent;
}

BONOBO_TYPE_FUNC (BonoboControlAccessible, PARENT_TYPE, bonobo_control_accessible);

BonoboControlAccessible *
bonobo_control_accessible_new (BonoboControl *control)
{
  BonoboControlAccessible *retval;	
  GtkWidget               *widget;
  AtkObject               *atko;

  g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

  /* FIXME: should use bonobo_control_get_plug_here */
  widget = bonobo_control_get_widget (control);

  g_assert (GTK_IS_PLUG (widget->parent));

  atko = gtk_widget_get_accessible (widget->parent);

  g_assert (SPI_IS_REMOTE_OBJECT (atko));

  retval = BONOBO_CONTROL_ACCESSIBLE (
	  spi_accessible_construct (BONOBO_TYPE_CONTROL_ACCESSIBLE, atko));
  
  retval->control = control;

  return retval;
}
