#ifndef __DOM_COMMENT_H__
#define __DOM_COMMENT_H__

#include <libgtkhtml/dom/core/dom-characterdata.h>

#define DOM_TYPE_COMMENT             (dom_comment_get_type ())
#define DOM_COMMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_COMMENT, DomComment))
#define DOM_COMMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_COMMENT, DomCommentClass))
#define DOM_IS_COMMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_COMMENT))
#define DOM_IS_COMMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_COMMENT))
#define DOM_COMMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_COMMENT, DomCommentClass))

struct _DomComment {
	DomNode parent;
};

struct _DomCommentClass {
	DomNodeClass parent_class;
};

GType dom_comment_get_type (void);

#endif /* __DOM_COMMENT_H__ */
