#ifndef __CSSMATCHER_H__
#define __CSSMATCHER_H__

#include <libxml/tree.h>

#include <css/cssparser.h>
#include <document/htmldocument.h>
#include <layout/htmlstyle.h>

G_BEGIN_DECLS

HtmlStyle *css_matcher_get_style (HtmlDocument *doc, HtmlStyle *parent_style, xmlNode *node, HtmlAtom *pseudo);
void css_matcher_apply_rule (HtmlDocument *document, HtmlStyle *style, HtmlStyle *parent_style, HtmlFontSpecification *old_font, CssDeclaration *decl);

G_END_DECLS

#endif /* __CSSMATCHER_H__ */
