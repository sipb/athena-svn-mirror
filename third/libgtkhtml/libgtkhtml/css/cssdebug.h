#ifndef __CSSDEBUG_H__
#define __CSSDEBUG_H__

#include "cssparser.h"

G_BEGIN_DECLS

void css_debug_print_statement (CssStatement *stat);
void css_debug_print_ruleset (CssRuleset *rs);

G_END_DECLS

#endif /* __CSSDEBUG_H__ */
