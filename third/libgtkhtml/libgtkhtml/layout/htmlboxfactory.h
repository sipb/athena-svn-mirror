#ifndef __HTMLBOXFACTORY_H__
#define __HTMLBOXFACTORY_H__

#include <libxml/parser.h>

#include <document/htmldocument.h>
#include <layout/htmlbox.h>
#include <view/htmlview.h>

G_BEGIN_DECLS

HtmlBox *       html_box_factory_get_box     (HtmlView *view, DomNode *node, HtmlBox *parent_box);
HtmlStyleChange html_box_factory_restyle_box (HtmlView *view, HtmlBox *box, HtmlAtom pseudo);
HtmlBox *       html_box_factory_new_box     (HtmlView *view, DomNode *node);

G_END_DECLS

#endif /* __HTMLBOXFACTORY_H__ */
