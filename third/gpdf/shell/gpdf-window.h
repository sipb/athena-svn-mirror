/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * PDF viewer Bonobo container.
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *   Michael Meeks <michael@ximian.com>
 *
 * Copyright 1999, 2000 Ximian, Inc.
 * Copyright 2002 Martin Kretzschmar
 */

#ifndef GPDF_WINDOW_H
#define GPDF_WINDOW_H

#include <libbonoboui.h>

G_BEGIN_DECLS

#define GPDF_TYPE_WINDOW            (gpdf_window_get_type ())
#define GPDF_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_WINDOW, GPdfWindow))
#define GPDF_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_WINDOW, GPdfWindowClass))
#define GPDF_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_WINDOW))
#define GPDF_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_WINDOW))


typedef struct _GPdfWindow        GPdfWindow;
typedef struct _GPdfWindowClass   GPdfWindowClass;
typedef struct _GPdfWindowPrivate GPdfWindowPrivate;

struct _GPdfWindow {
	BonoboWindow         parent;

	GPdfWindowPrivate   *priv;
};

struct _GPdfWindowClass {
	BonoboWindowClass parent_class;
};

GType       gpdf_window_get_type     (void);
GtkWidget  *gpdf_window_new          (void);
GPdfWindow *gpdf_window_construct    (GPdfWindow *gpdf_window);

gboolean    gpdf_window_open         (GPdfWindow *gpdf_window, const char *name);
void        gpdf_window_close        (GPdfWindow *gpdf_window);

gboolean    gpdf_window_has_contents (const GPdfWindow *gpdf_window);

G_END_DECLS

#endif /* GPDF_WINDOW_H */
