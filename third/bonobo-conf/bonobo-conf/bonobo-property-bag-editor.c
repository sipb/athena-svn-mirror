/*
 * bonobo-property-bag-editor.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-container.h>
#include <bonobo/bonobo-widget.h>

#include "bonobo-property-editor.h"
#include "gtkwtree.h"
#include "gtkwtreeitem.h"

BonoboControl *
bonobo_property_bag_editor_new (Bonobo_PropertyBag  bag,
				Bonobo_UIContainer  uic,
				CORBA_Environment  *ev)
{
	GtkWidget           *tree, *item;
	Bonobo_PropertyList *plist;
	int                  i;

	tree = gtk_wtree_new();
	
	plist = Bonobo_PropertyBag_getProperties (bag, ev);
      	
	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;
	
	for (i = 0; i < plist->_length; i++) {
		CORBA_TypeCode        tc;
		GtkObject            *ed;
		GtkWidget            *w;
		char                 *pn;

		CORBA_exception_init (ev);
		
		pn = Bonobo_Property_getName (plist->_buffer [i], ev);
		if (BONOBO_EX (ev))
			continue;

		tc = Bonobo_Property_getType (plist->_buffer [i], ev);
		if (BONOBO_EX (ev)) {
			CORBA_free (pn);
			continue;
		}

		ed = bonobo_peditor_resolve (tc);
		w = bonobo_peditor_get_widget (BONOBO_PEDITOR(ed));
		gtk_widget_show (w);

		bonobo_peditor_set_property (BONOBO_PEDITOR (ed), bag, pn, tc,
					     NULL);

		CORBA_Object_release ((CORBA_Object)tc, NULL);

		item = gtk_wtree_item_new_with_widget (pn, w);
		gtk_wtree_append (GTK_WTREE (tree), item);

		CORBA_free (pn);

	}

	CORBA_free (plist);

	gtk_widget_show_all (tree);

	return bonobo_control_new (tree);
}

