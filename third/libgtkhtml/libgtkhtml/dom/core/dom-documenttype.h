#ifndef __DOM_DOCUMENTTYPE_H__
#define __DOM_DOCUMENTTYPE_H__

typedef struct _DomDocumentType DomDocumentType;
typedef struct _DomDocumentTypeClass DomDocumentTypeClass;

#include "dom-node.h"

#define DOM_TYPE_DOCUMENT_TYPE             (dom_document_type_get_type ())
#define DOM_DOCUMENT_TYPE(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_DOCUMENT_TYPE, DomDocumentType))
#define DOM_DOCUMENT_TYPE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_DOCUMENT_TYPE, DomDocumentTypeClass))
#define DOM_IS_DOCUMENT_TYPE(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_DOCUMENT_TYPE))
#define DOM_IS_DOCUMENT_TYPE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_DOCUMENT_TYPE))
#define DOM_DOCUMENT_TYPE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_DOCUMENT_TYPE, DomDocumentTypeClass))

struct _DomDocumentType {
	DomNode parent;
};

struct _DomDocumentTypeClass {
	DomNodeClass parent_class;
};

GType dom_document_type_get_type (void);

DomString *dom_DocumentType__get_name (DomDocumentType *dtd);
DomString *dom_DocumentType__get_publicId (DomDocumentType *dtd);
DomString *dom_DocumentType__get_systemId (DomDocumentType *dtd);
DomNamedNodeMap *dom_DocumentType__get_entities (DomDocumentType *dtd);

#endif /** __DOM_DOCUMENTTYPE_H__ */
