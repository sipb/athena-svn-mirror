#include "dom-comment.h"

static void
dom_comment_class_init (DomCommentClass *klass)
{
}

static void
dom_comment_init (DomComment *doc)
{
}

GType
dom_comment_get_type (void)
{
	static GType dom_comment_type = 0;

	if (!dom_comment_type) {
		static const GTypeInfo dom_comment_info = {
			sizeof (DomCommentClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dom_comment_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (DomComment),
			16,   /* n_preallocs */
			(GInstanceInitFunc) dom_comment_init,
		};

		dom_comment_type = g_type_register_static (DOM_TYPE_NODE, "DomComment", &dom_comment_info, 0);
	}

	return dom_comment_type;
}

