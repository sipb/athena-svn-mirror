/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-dock-item-grip.h
 *
 * Author:
 *    Michael Meeks
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 */

#ifndef _BONOBO_DOCK_ITEM_GRIP_H_
#define _BONOBO_DOCK_ITEM_GRIP_H_

#include <gtk/gtkwidget.h>
#include <bonobo/bonobo-dock-item.h>

G_BEGIN_DECLS

#define BONOBO_TYPE_DOCK_ITEM_GRIP            (bonobo_dock_item_grip_get_type())
#define BONOBO_DOCK_ITEM_GRIP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BONOBO_TYPE_DOCK_ITEM_GRIP, BonoboDockItemGrip))
#define BONOBO_DOCK_ITEM_GRIP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_DOCK_ITEM_GRIP, BonoboDockItemGripClass))
#define BONOBO_IS_DOCK_ITEM_GRIP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BONOBO_TYPE_DOCK_ITEM_GRIP))
#define BONOBO_IS_DOCK_ITEM_GRIP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BONOBO_TYPE_DOCK_ITEM_GRIP))
#define BONOBO_DOCK_ITEM_GRIP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BONOBO_TYPE_DOCK_ITEM_GRIP, BonoboDockItemGripClass))

typedef struct {
	GtkWidget parent;

	BonoboDockItem *item;
} BonoboDockItemGrip;

typedef struct {
	GtkWidgetClass parent_class;

	void (*activate) (BonoboDockItemGrip *grip);
} BonoboDockItemGripClass;

GType      bonobo_dock_item_grip_get_type (void);
GtkWidget *bonobo_dock_item_grip_new      (BonoboDockItem *item);

G_END_DECLS

#endif /* _BONOBO_DOCK_ITEM_GRIP_H_ */
