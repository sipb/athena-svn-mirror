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

#ifndef BONOBO_CONTROL_FRAME_ACCESSIBLE_H_
#define BONOBO_CONTROL_FRAME_ACCESSIBLE_H_

#include <libspi/accessible.h>
#include <bonobo/bonobo-control-frame.h>

G_BEGIN_DECLS

#define BONOBO_TYPE_CONTROL_FRAME_ACCESSIBLE        (bonobo_control_frame_accessible_get_type ())
#define BONOBO_CONTROL_FRAME_ACCESSIBLE(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_TYPE_CONTROL_FRAME_ACCESSIBLE, BonoboControlFrameAccessible))
#define BONOBO_CONTROL_FRAME_ACCESSIBLE_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), BONOBO_TYPE_CONTROL_FRAME_ACCESSIBLE, BonoboControlFrameAccessibleClass))
#define BONOBO_IS_CONTROL_FRAME_ACCESSIBLE(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_TYPE_CONTROL_FRAME_ACCESSIBLE))
#define BONOBO_IS_CONTROL_FRAME_ACCESSIBLE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_TYPE_CONTROL_FRAME_ACCESSIBLE))

typedef struct {
	SpiAccessible parent;
	BonoboControlFrame *control_frame;
} BonoboControlFrameAccessible;

typedef struct {
        SpiAccessibleClass parent_class;
} BonoboControlFrameAccessibleClass;

GType                            bonobo_control_frame_accessible_get_type   (void);
BonoboControlFrameAccessible    *bonobo_control_frame_accessible_new (BonoboControlFrame *control_frame);

G_END_DECLS

#endif /* BONOBO_CONTROL_FRAME_ACCESSIBLE_H_ */
