#ifndef __GT1_NAMECONTEXT_H__
#define __GT1_NAMECONTEXT_H__

BEGIN_GNOME_DECLS

typedef struct _Gt1NameContext Gt1NameContext;
typedef struct _Gt1NameContextHashEntry Gt1NameContextHashEntry;

typedef int Gt1NameId;

struct _Gt1NameContextHashEntry {
  char *name;
  int Gt1NameId;
};

struct _Gt1NameContext {
  int num_entries;
  int table_size;
  Gt1NameContextHashEntry *table;
};

Gt1NameContext *gt1_name_context_new (void);
void gt1_name_context_free (Gt1NameContext *nc);
Gt1NameId gt1_name_context_intern (Gt1NameContext *nc, const char *name);
Gt1NameId gt1_name_context_intern_size (Gt1NameContext *nc, const char *name, int size);
char *gt1_name_context_string (Gt1NameContext *nc, Gt1NameId Gt1NameId);

END_GNOME_DECLS

#endif /* __GT1_NAMECONTEXT_H__ */
