/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * ggv-control.h
 *
 * Author:  Jaka Mocnik  <jaka@gnu.org>
 *
 * Copyright (c) 2001, Free Software Foundation
 */

#ifndef _GGV_CONTROL_H_
#define _GGV_CONTROL_H_

#include <ggv-postscript-view.h>

G_BEGIN_DECLS
 
#define GGV_CONTROL_TYPE           (ggv_control_get_type ())
#define GGV_CONTROL(o)             (GTK_CHECK_CAST ((o), GGV_CONTROL_TYPE, GgvControl))
#define GGV_CONTROL_CLASS(k)       (GTK_CHECK_CLASS_CAST((k), GGV_CONTROL_TYPE, GgvControlClass))

#define GGV_IS_CONTROL(o)          (GTK_CHECK_TYPE ((o), GGV_CONTROL_TYPE))
#define GGV_IS_CONTROL_CLASS(k)    (GTK_CHECK_CLASS_TYPE ((k), GGV_CONTROL_TYPE))

typedef struct _GgvControl              GgvControl;
typedef struct _GgvControlClass         GgvControlClass;
typedef struct _GgvControlPrivate       GgvControlPrivate;
typedef struct _GgvControlClassPrivate  GgvControlClassPrivate;

struct _GgvControl {
	BonoboControl control;

	GgvControlPrivate *priv;
};

struct _GgvControlClass {
	BonoboControlClass parent_class;

	GgvControlClassPrivate *priv;
};

GtkType        ggv_control_get_type  (void);
GgvControl    *ggv_control_new       (GgvPostScriptView *ps_view);
GgvControl    *ggv_control_construct (GgvControl *control,
									  GgvPostScriptView *ps_view);

G_END_DECLS

#endif /* _GGV_CONTROL */
