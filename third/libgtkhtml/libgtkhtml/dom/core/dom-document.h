#ifndef __DOM_DOCUMENT_H__
#define __DOM_DOCUMENT_H__

#include <libgtkhtml/dom/dom-types.h>
#include <libgtkhtml/dom/core/dom-node.h>

#define DOM_TYPE_DOCUMENT             (dom_document_get_type ())
#define DOM_DOCUMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_DOCUMENT, DomDocument))
#define DOM_DOCUMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_DOCUMENT, DomDocumentClass))
#define DOM_IS_DOCUMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_DOCUMENT))
#define DOM_IS_DOCUMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_DOCUMENT))
#define DOM_DOCUMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_DOCUMENT, DomDocumentClass))

struct _DomDocument {
	DomNode parent;

	GSList *iterators;
};

struct _DomDocumentClass {
	DomNodeClass parent_class;
};

GType dom_document_get_type (void);

DomElement *dom_Document__get_documentElement (DomDocument *doc);
DomElement *dom_Document_createElement (DomDocument *doc, const DomString *tagName);
DomText *dom_Document_createTextNode (DomDocument *doc, const DomString *data);
DomComment *dom_Document_createComment (DomDocument *doc, const DomString *data);
DomNode *dom_Document_importNode (DomDocument *doc, DomNode *importedNode, DomBoolean deep, DomException *exc);

#endif /* __DOM_DOCUMENT_H__ */
