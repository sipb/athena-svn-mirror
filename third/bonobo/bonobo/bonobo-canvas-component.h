/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-canvas-component.h: implements the CORBA interface for
 * the Bonobo::Canvas:Item interface used in Bonobo::Views.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * (C) 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_CANVAS_COMPONENT_H_
#define _BONOBO_CANVAS_COMPONENT_H_

#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_CANVAS_COMPONENT_TYPE        (bonobo_canvas_component_get_type ())
#define BONOBO_CANVAS_COMPONENT(o)          (GTK_CHECK_CAST ((o), BONOBO_CANVAS_COMPONENT_TYPE, BonoboCanvasComponent))
#define BONOBO_CANVAS_COMPONENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CANVAS_COMPONENT__TYPE, BonoboCanvasComponentClass))
#define BONOBO_IS_CANVAS_COMPONENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_CANVAS_COMPONENT_TYPE))
#define BONOBO_IS_CANVAS_COMPONENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CANVAS_COMPONENT_TYPE))

typedef struct _BonoboCanvasComponentPrivate BonoboCanvasComponentPrivate;

typedef struct {
	BonoboObject base;
	BonoboCanvasComponentPrivate *priv;
} BonoboCanvasComponent;

typedef struct {
	BonoboObjectClass parent_class;

	/*
	 * Signals
	 */
	 void (*set_bounds) (BonoboCanvasComponent *component,
			     Bonobo_Canvas_DRect *bbox,
			     CORBA_Environment *ev);
} BonoboCanvasComponentClass;

GtkType                 bonobo_canvas_component_get_type         (void);
Bonobo_Canvas_Component bonobo_canvas_component_object_create    (BonoboObject                *object);
void                    bonobo_canvas_component_set_proxy        (BonoboCanvasComponent       *comp,
								  Bonobo_Canvas_ComponentProxy proxy);
BonoboCanvasComponent  *bonobo_canvas_component_construct        (BonoboCanvasComponent       *comp,
								  Bonobo_Canvas_Component      corba_canvas_comp,
								  GnomeCanvasItem             *item);
BonoboCanvasComponent  *bonobo_canvas_component_new              (GnomeCanvasItem             *item);
GnomeCanvasItem        *bonobo_canvas_component_get_item         (BonoboCanvasComponent       *comp);
void                    bonobo_canvas_component_set_ui_component (BonoboCanvasComponent       *comp,
								  Bonobo_UIComponent           ui_component);
Bonobo_UIComponent      bonobo_canvas_component_get_ui_component (BonoboCanvasComponent       *comp);
					  
POA_Bonobo_Canvas_Component__epv *bonobo_canvas_component_get_epv  (void);

END_GNOME_DECLS

#endif /* */
