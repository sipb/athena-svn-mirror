#ifndef __DOM_ENTITY_H__
#define __DOM_ENTITY_H__

#include "dom-node.h"

#define DOM_TYPE_ENTITY             (dom_entity_get_type ())
#define DOM_ENTITY(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_ENTITY, DomEntity))
#define DOM_ENTITY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_ENTITY, DomEntityClass))
#define DOM_IS_ENTITY(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_ENTITY))
#define DOM_IS_ENTITY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_ENTITY))
#define DOM_ENTITY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_ENTITY, DomEntityClass))

struct _DomEntity {
	DomNode parent;
};

struct _DomEntityClass {
	DomNodeClass parent_class;
};

GType dom_entity_get_type (void);

DomString *dom_Entity__get_publicId (DomEntity *entity);

#endif /* __DOM_ENTIT_H__ */
