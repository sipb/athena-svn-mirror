/**
 * bonobo-subproperty.h:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_SUB_PROPERTY_H_
#define _BONOBO_SUB_PROPERTY_H_

#include <bonobo/bonobo-xobject.h>
#include "bonobo-property-editor.h"

BEGIN_GNOME_DECLS

#define BONOBO_SUB_PROPERTY_TYPE        (bonobo_sub_property_get_type ())
#define BONOBO_SUB_PROPERTY(o)          (GTK_CHECK_CAST ((o), BONOBO_SUB_PROPERTY_TYPE, BonoboSubProperty))
#define BONOBO_SUB_PROPERTY_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_SUB_PROPERTY_TYPE, BonoboSubPropertyClass))
#define BONOBO_IS_SUB_PROPERTY(o)       (GTK_CHECK_TYPE ((o), BONOBO_SUB_PROPERTY_TYPE))
#define BONOBO_IS_SUB_PROPERTY_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_SUB_PROPERTY_TYPE))

typedef struct _BonoboSubPropertyPrivate BonoboSubPropertyPrivate;

typedef struct {
	BonoboXObject base;

	BonoboSubPropertyPrivate *priv;
} BonoboSubProperty;

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_Property__epv epv;

} BonoboSubPropertyClass;

typedef void (*BonoboSubPropertyChangeFn) (BonoboPEditor   *editor,
					   const CORBA_any *value,
					   int              offset);


GtkType            
bonobo_sub_property_get_type   (void);

BonoboSubProperty *
bonobo_sub_property_new        (BonoboPEditor             *editor,
				gchar                     *name, 
				CORBA_any                 *value, 
				int                        offset,
				BonoboEventSource         *es,
				BonoboSubPropertyChangeFn  change_fn);

void
bonobo_sub_property_set_value  (BonoboSubProperty *property,
				CORBA_any *value);

END_GNOME_DECLS

#endif /* _BONOBO_SUB_PROPERTY_H_ */
