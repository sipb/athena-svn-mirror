/*
 * bonobo-property_frame.h:
 *
 * Authors:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_PROPERTY_FRAME_H_
#define _BONOBO_PROPERTY_FRAME_H_

#include <gtk/gtkframe.h>
#include <libgnomeui/gnome-propertybox.h>
#include <bonobo-conf/bonobo-property-bag-proxy.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_PROPERTY_FRAME_TYPE        (bonobo_property_frame_get_type ())
#define BONOBO_PROPERTY_FRAME(o)          (GTK_CHECK_CAST ((o), BONOBO_PROPERTY_FRAME_TYPE, BonoboPropertyFrame))
#define BONOBO_PROPERTY_FRAME_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PROPERTY_FRAME_TYPE, BonoboPropertyFrameClass))

#define BONOBO_IS_PROPERTY_FRAME(o)       (GTK_CHECK_TYPE ((o), BONOBO_PROPERTY_FRAME_TYPE))
#define BONOBO_IS_PROPERTY_FRAME_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PROPERTY_FRAME_TYPE))

typedef struct _BonoboPropertyFrame         BonoboPropertyFrame;
typedef struct _BonoboPropertyFrameClass    BonoboPropertyFrameClass;

struct _BonoboPropertyFrame {
	GtkFrame frame;

	BonoboPBProxy      *proxy;
	char               *moniker;
};

struct _BonoboPropertyFrameClass {
        GtkFrameClass parent_class;
};

GtkType           
bonobo_property_frame_get_type         (void);

GtkWidget *
bonobo_property_frame_new              (char *label, char *moniker);

void
bonobo_property_frame_set_moniker      (BonoboPropertyFrame *pf, 
					char                *moniker);

END_GNOME_DECLS

#endif /* _BONOBO_PROPERTY_FRAME_H */
