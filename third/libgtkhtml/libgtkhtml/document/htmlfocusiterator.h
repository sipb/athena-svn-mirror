#ifndef __HTMLFOCUSITERATOR_H__
#define __HTMLFOCUSITERATOR_H__

#include <libgtkhtml/dom/dom-types.h>

#define HTML_TYPE_FOCUS_ITERATOR             (html_focus_iterator_get_type ())
#define HTML_FOCUS_ITERATOR(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), HTML_TYPE_FOCUS_ITERATOR, HtmlFocusIterator))
#define HTML_FOCUS_ITERATOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_FOCUS_ITERATOR, HtmlFocusIteratorClass))
#define HTML_IS_FOCUS_ITERATOR(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  HTML_TYPE_FOCUS_ITERATOR))
#define HTML_IS_FOCUS_ITERATOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_FOCUS_ITERATOR))
#define HTML_FOCUS_ITERATOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HTML_TYPE_FOCUS_ITERATOR, HtmlFocusIteratorClass))

typedef struct _HtmlFocusIterator HtmlFocusIterator;
typedef struct _HtmlFocusIteratorClass HtmlFocusIteratorClass;

struct _HtmlFocusIterator {
	GObject parent_instance;

	DomDocument *document;
	DomNode *current_node;
};

struct _HtmlFocusIteratorClass {
	GObjectClass parent_class;

};

DomElement *html_focus_iterator_next_element (DomDocument *document, DomElement *element);
DomElement *html_focus_iterator_prev_element (DomDocument *document, DomElement *element);

#endif /* __HTMLFOCUSITERATOR_H__ */
