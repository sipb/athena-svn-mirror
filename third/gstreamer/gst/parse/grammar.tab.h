#ifndef BISON_GRAMMAR_TAB_H
# define BISON_GRAMMAR_TAB_H

#ifndef YYSTYPE
typedef union {
    gchar *s;
    GValue *v;
    graph_t *g;
    link_t *c;
    property_t *p;
    element_t *e;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	IDENTIFIER	257
# define	LINK	258
# define	BLINK	259
# define	FLINK	260
# define	VALUE	261


#endif /* not BISON_GRAMMAR_TAB_H */
