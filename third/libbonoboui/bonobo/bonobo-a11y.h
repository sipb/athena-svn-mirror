/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo accessibility helpers
 *
 * Author:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2002 Sun Microsystems, Inc.
 */
#ifndef _BONOBO_A11Y_H_
#define _BONOBO_A11Y_H_

#include <stdarg.h>
#include <atk/atkaction.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

typedef void     (*BonoboA11YClassInitFn)    (AtkObjectClass *klass);

AtkObject *bonobo_a11y_get_atk_object        (gpointer              widget);
AtkObject *bonobo_a11y_set_atk_object_ret    (GtkWidget            *widget,
					      AtkObject            *object);

GType      bonobo_a11y_get_derived_type_for  (GType                 widget_type,
					      const char           *gail_parent_class,
					      BonoboA11YClassInitFn class_init);

AtkObject *bonobo_a11y_create_accessible_for (GtkWidget            *widget,
					      const char           *gail_parent_class,
					      BonoboA11YClassInitFn class_init,
					      /* pairs of: */
					      GType                 first_interface_type,
					      /* const GInterfaceInfo *interface */
					      ...);
				              /* NULL terminated */

void       bonobo_a11y_add_actions_interface (GType                 a11y_object_type,
					      AtkActionIface       *chain,
					      /* quads of: */
					      int                   first_id,
					      /* char * action name */
					      /* char * initial action description */
					      /* char * keybinding descr. */
					      ...);
				              /* -1 terminated */

G_END_DECLS

#endif /* _BONOBO_A11Y_H_ */

