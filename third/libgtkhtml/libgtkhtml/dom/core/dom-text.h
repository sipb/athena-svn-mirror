#ifndef __DOM_TEXT_H__
#define __DOM_TEXT_H__

#include <dom/core/dom-characterdata.h>

#define DOM_TYPE_TEXT             (dom_text_get_type ())
#define DOM_TEXT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_TEXT, DomText))
#define DOM_TEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_TEXT, DomTextClass))
#define DOM_IS_TEXT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_TEXT))
#define DOM_IS_TEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_TEXT))
#define DOM_TEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_TEXT, DomTextClass))

struct _DomText {
	DomCharacterData parent;
};

struct _DomTextClass {
	DomCharacterDataClass parent_class;
};

GType dom_text_get_type (void);

DomText *dom_Text_splitText (DomText *text, gulong offset, DomException *exc);

#endif /* __DOM_TEXT_H__ */
