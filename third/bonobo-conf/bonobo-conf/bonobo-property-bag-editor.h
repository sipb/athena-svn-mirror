/*
 * bonobo-property-bag-editor.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_PROPERTY_BAG_EDITOR_H_
#define _BONOBO_PROPERTY_BAG_EDITOR_H_

#include "bonobo/bonobo-control.h"

BEGIN_GNOME_DECLS

BonoboControl *
bonobo_property_bag_editor_new (Bonobo_PropertyBag  bag,
				Bonobo_UIContainer  uic,
				CORBA_Environment  *ev);


END_GNOME_DECLS

#endif /* _BONOBO_PROPERTY_BAG_EDITOR_H_ */
