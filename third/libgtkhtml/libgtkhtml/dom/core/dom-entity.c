#include <libxml/entities.h>
#include <libxml/debugXML.h>
#include "dom-entity.h"

DomString *
dom_Entity__get_publicId (DomEntity *entity)
{
#ifdef LIBXML_DEBUG_ENABLED
	xmlDebugDumpOneNode (stdout, DOM_NODE (entity)->xmlnode, 0);
#endif

	return NULL;
}

static void
dom_entity_class_init (DomEntityClass *klass)
{
}

static void
dom_entity_init (DomEntity *doc)
{
}

GType
dom_entity_get_type (void)
{
	static GType dom_entity_type = 0;

	if (!dom_entity_type) {
		static const GTypeInfo dom_entity_info = {
			sizeof (DomEntityClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dom_entity_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (DomEntity),
			16,   /* n_preallocs */
			(GInstanceInitFunc) dom_entity_init,
		};

		dom_entity_type = g_type_register_static (DOM_TYPE_NODE, "DomEntity", &dom_entity_info, 0);
	}

	return dom_entity_type;
}
