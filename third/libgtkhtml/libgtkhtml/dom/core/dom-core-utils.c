#include "dom-core-utils.h"

const gchar *
dom_exception_get_name (DomException exc)
{
	switch (exc) {
	case DOM_HIERARCHY_REQUEST_ERR:
		return "DOM_HIERARCHY_REQUEST_ERR";
		break;
	default:
		g_warning ("Unknown exception %d", exc);
		return NULL;
	}
}
