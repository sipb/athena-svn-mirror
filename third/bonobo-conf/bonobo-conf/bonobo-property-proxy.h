/**
 * bonobo-property-proxy.h: a proxy for properties
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef __BONOBO_PROPERTY_PROXY_H__
#define __BONOBO_PROPERTY_PROXY_H__

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-property.h>

#include "bonobo-property-bag-proxy.h"

BEGIN_GNOME_DECLS

typedef struct {
	POA_Bonobo_Property   prop;
	char		     *property_name;
	BonoboTransient      *transient;
	BonoboPBProxy        *pbp;
} PropertyProxyServant;

BonoboTransient *bonobo_property_proxy_transient (BonoboPBProxy *pbp);

END_GNOME_DECLS

#endif /* ! __BONOBO_PROPERTY_PROXY_H__ */
