/**
 * bonobo-property-bag-proxy.h: a proxy for property bags
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef __BONOBO_PBPROXY_H__
#define __BONOBO_PBPROXY_H__

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-property.h>

BEGIN_GNOME_DECLS

#define BONOBO_PBPROXY_TYPE        (bonobo_pbproxy_get_type ())
#define BONOBO_PBPROXY(o)	      (GTK_CHECK_CAST ((o), BONOBO_PBPROXY_TYPE, BonoboPBProxy))
#define BONOBO_PBPROXY_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PBPROXY_TYPE, BonoboPBProxyClass))
#define BONOBO_IS_PBPROXY(o)	      (GTK_CHECK_TYPE ((o), BONOBO_PBPROXY_TYPE))
#define BONOBO_IS_PBPROXY_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PBPROXY_TYPE))

typedef struct _BonoboPBProxy        BonoboPBProxy;

struct _BonoboPBProxy {
	BonoboXObject       base;

	/* read only values */
	BonoboEventSource  *es;
	Bonobo_PropertyBag  bag; 
	BonoboTransient    *transient;
	Bonobo_EventSource_ListenerId lid;

	/* private */
	GList              *pl;
};

typedef struct {
	BonoboXObjectClass  parent_class;

	POA_Bonobo_PropertyBag__epv epv;

        void (*modified)                       (BonoboPBProxy *proxy);

} BonoboPBProxyClass;


GtkType		  bonobo_pbproxy_get_type           (void);
BonoboPBProxy	 *bonobo_pbproxy_new	            ();

void              bonobo_pbproxy_set_bag            (BonoboPBProxy     *proxy,
						     Bonobo_PropertyBag bag);

CORBA_TypeCode    bonobo_pbproxy_prop_type          (BonoboPBProxy     *proxy,
						     const char        *name,
						     CORBA_Environment *ev);

CORBA_any        *bonobo_pbproxy_get_value          (BonoboPBProxy     *proxy,
						     const char        *name,
						     CORBA_Environment *ev);

void              bonobo_pbproxy_set_value          (BonoboPBProxy     *proxy,
						     const char        *name,
						     const CORBA_any   *any,
						     CORBA_Environment *ev);

CORBA_any        *bonobo_pbproxy_get_default        (BonoboPBProxy     *proxy,
						     const char        *name,
						     CORBA_Environment *ev);

char             *bonobo_pbproxy_get_doc_string     (BonoboPBProxy     *proxy,
						     const char        *name,
						     CORBA_Environment *ev);

CORBA_long        bonobo_pbproxy_get_flags          (BonoboPBProxy     *proxy,
						     const char        *name,
						     CORBA_Environment *ev);

void              bonobo_pbproxy_update             (BonoboPBProxy *proxy);
void              bonobo_pbproxy_revert             (BonoboPBProxy *proxy);


END_GNOME_DECLS

#endif /* ! __BONOBO_PBPROXY_H__ */
