/*
 * AT-SPI - Assistive Technology Service Provider Interface
 * (Gnome Accessibility Project; http://developer.gnome.org/projects/gap)
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libbonobo.h>
#include "GNOME_Magnifier.h"

static GNOME_Magnifier_Magnifier _the_magnifier = NULL;

static GNOME_Magnifier_Magnifier 
get_magnifier()
{
  CORBA_Object oclient;
  char *obj_id;
  CORBA_Environment ev;
  
  if (!_the_magnifier)
  {
    CORBA_exception_init (&ev);
    obj_id = "OAFIID:GNOME_Magnifier_Magnifier:0.9";

    oclient = bonobo_activation_activate_from_id (obj_id, 0, NULL, &ev);
    if (ev._major != CORBA_NO_EXCEPTION) {
      fprintf (stderr,
            ("Activation error: during magnifier activation: %s\n"),
            CORBA_exception_id(&ev));
      CORBA_exception_free(&ev);
    }

    if (CORBA_Object_is_nil (oclient, &ev))
    {
      g_error ("Could not locate magnifier");
    }

    _the_magnifier = (GNOME_Magnifier_Magnifier) oclient;

  }

  return _the_magnifier;
}

#ifdef WE_USE_THIS_AGAIN
static GNOME_Magnifier_ZoomRegion
test_client_magnifier_get_zoomer (GNOME_Magnifier_Magnifier magnifier, int index)
{
	CORBA_Environment ev;
	GNOME_Magnifier_ZoomRegionList *zoomers;
	CORBA_exception_init (&ev);
	zoomers = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier, &ev);
	if (zoomers && index < zoomers->_length)
		return zoomers->_buffer [index];
	else
		return CORBA_OBJECT_NIL;
}
#endif

static void
init_rectbounds (GNOME_Magnifier_RectBounds *bounds, int x1, int y1, int x2, int y2)
{
	bounds->x1 = x1;
	bounds->y1 = y1;
	bounds->x2 = x2;
	bounds->y2 = y2;
}

static void
test_new_region (GNOME_Magnifier_Magnifier magnifier,
		 float xscale,
		 float yscale,
		 int x1,
		 int y1,
		 int x2,
		 int y2,
		 GNOME_Magnifier_ZoomRegion_ScrollingPolicy scroll_policy,
		 gchar *smoothing_type,
		 gboolean is_inverse,
		 GNOME_Magnifier_ZoomRegion_AlignPolicy align)
{
	GNOME_Magnifier_RectBounds *viewport = GNOME_Magnifier_RectBounds__alloc ();
	GNOME_Magnifier_RectBounds *roi = GNOME_Magnifier_RectBounds__alloc ();
	GNOME_Magnifier_ZoomRegion zoomer;
	Bonobo_PropertyBag properties;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	init_rectbounds (roi, 0, 0, 100, 100);
	init_rectbounds (viewport, x1, y1, x2, y2);
	
	zoomer = GNOME_Magnifier_Magnifier_createZoomRegion (magnifier,
							     xscale,
							     yscale,
							     roi,
							     viewport,
							     &ev);
	GNOME_Magnifier_Magnifier_addZoomRegion (magnifier,
						 zoomer,
						 &ev);
	
	properties = GNOME_Magnifier_ZoomRegion_getProperties (zoomer, &ev);

	Bonobo_PropertyBag_setValue (properties, "smooth-scroll-policy",
				     bonobo_arg_new_from (BONOBO_ARG_INT,
							  &scroll_policy), &ev);
	Bonobo_PropertyBag_setValue (properties, "x-alignment",
				     bonobo_arg_new_from (BONOBO_ARG_INT,
							  &align), &ev);
	Bonobo_PropertyBag_setValue (properties, "y-alignment",
				     bonobo_arg_new_from (BONOBO_ARG_INT,
							  &align), &ev);
	
	if (smoothing_type)
	{
		BonoboArg *arg = bonobo_arg_new (BONOBO_ARG_STRING);
		BONOBO_ARG_SET_STRING (arg, smoothing_type);
		Bonobo_PropertyBag_setValue (properties, "smoothing-type",
					     arg, &ev);
	}
	
	if (is_inverse)
		Bonobo_PropertyBag_setValue (properties, "inverse-video",
					     bonobo_arg_new_from (BONOBO_ARG_BOOLEAN,
								  &is_inverse),
								  &ev);
}

int main(int argc, char ** argv){

	GNOME_Magnifier_Magnifier magnifier;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	
	if (!bonobo_init (&argc, argv))
	{
		g_error ("Could not initialize Bonobo");
	}
	
	magnifier = get_magnifier ();
	
	GNOME_Magnifier_Magnifier_clearAllZoomRegions (magnifier, &ev);
	
	test_new_region (magnifier, 2.0, 2.0, 0, 0, 400, 200,
			 GNOME_Magnifier_ZoomRegion_SCROLL_SMOOTHEST,
			 NULL, FALSE, GNOME_Magnifier_ZoomRegion_ALIGN_CENTER);
	test_new_region (magnifier, 2.0, 2.0, 0, 200, 400, 400,
			 GNOME_Magnifier_ZoomRegion_SCROLL_FASTEST,
			 NULL, FALSE, GNOME_Magnifier_ZoomRegion_ALIGN_CENTER);
	test_new_region (magnifier, 0.75, 1.5, 0, 400, 200, 600,
			 GNOME_Magnifier_ZoomRegion_SCROLL_FASTEST,
			 "bilinear", TRUE, GNOME_Magnifier_ZoomRegion_ALIGN_CENTER);
	test_new_region (magnifier, 3.0, 3.0, 200, 400, 400, 600,
			 GNOME_Magnifier_ZoomRegion_SCROLL_FASTEST,
			 "bilinear", FALSE, GNOME_Magnifier_ZoomRegion_ALIGN_MIN);	
	return 0;
}

