/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * gpdf bonobo control
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * Copyright 2002 Martin Kretzschmar
 */

#ifndef GPDF_CONTROL_H
#define GPDF_CONTROL_H

#include "gpdf-g-switch.h"
#  include <bonobo/bonobo-control.h>
#include "gpdf-g-switch.h"

G_BEGIN_DECLS

#define GPDF_TYPE_CONTROL            (gpdf_control_get_type ())
#define GPDF_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_CONTROL, GPdfControl))
#define GPDF_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_CONTROL, GPdfControlClass))
#define GPDF_IS_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_CONTROL))
#define GPDF_IS_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_CONTROL))

typedef struct _GPdfControl        GPdfControl;
typedef struct _GPdfControlClass   GPdfControlClass;
typedef struct _GPdfControlPrivate GPdfControlPrivate;

struct _GPdfControl {
	BonoboControl parent;

	GPdfControlPrivate *priv;
};

struct _GPdfControlClass {
	BonoboControlClass parent_class;
};

GType        gpdf_control_get_type  (void);

G_END_DECLS

#endif /* GPDF_CONTROL_H */
