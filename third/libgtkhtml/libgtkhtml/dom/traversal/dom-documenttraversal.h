#ifndef __DOM_DOCUMENTTRAVERSAL_H__
#define __DOM_DOCUMENTTRAVERSAL_H__

typedef struct _DomDocumentTraversal DomDocumentTraversal;
typedef struct _DomDocumentTraversalIface DomDocumentTraversalIface;

#include <glib-object.h>
#include <libgtkhtml/dom/dom-types.h>
#include <libgtkhtml/dom/traversal/dom-nodefilter.h>
#include <libgtkhtml/dom/traversal/dom-nodeiterator.h>

#define DOM_TYPE_DOCUMENT_TRAVERSAL             (dom_document_traversal_get_type ())
#define DOM_DOCUMENT_TRAVERSAL(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_DOCUMENT_TRAVERSAL, DomDocumentTraversal))
#define DOM_DOCUMENT_TRAVERSAL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_DOCUMENT_TRAVERSAL, DomDocumentTraversalClass))
#define DOM_IS_DOCUMENT_TRAVERSAL(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_DOCUMENT_TRAVERSAL))
#define DOM_IS_DOCUMENT_TRAVERSAL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_DOCUMENT_TRAVERSAL))
#define DOM_DOCUMENT_TRAVERSAL_GET_IFACE(obj)  ((DomDocumentTraversalIface *)g_type_interface_peek (((GTypeInstance *)DOM_DOCUMENT_TRAVERSAL (obj))->g_class, DOM_TYPE_DOCUMENT_TRAVERSAL))

struct _DomDocumentTraversalIface {
	GTypeInterface g_iface;

	DomNodeIterator * (*createNodeIterator) (DomDocumentTraversal *traversal, DomNode *root, gulong whatToShow, DomNodeFilter *filter, gboolean entityReferenceExpansion, DomException *exc);
};

GType dom_document_traversal_get_type (void);

DomNodeIterator *dom_DocumentTraversal_createNodeIterator (DomDocumentTraversal *traversal, DomNode *root, gulong whatToShow, DomNodeFilter *filter, gboolean entityReferenceExpansion, DomException *exc);

#endif /* __DOM_DOCUMENTTRAVERSAL_H__ */
