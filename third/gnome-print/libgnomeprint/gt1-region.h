#ifndef __GT1_REGION_H__
#define __GT1_REGION_H__

#include <libgnome/gnome-defs.h> /* for GNOME_DECLS */

BEGIN_GNOME_DECLS

/* A simple region-based allocator */

typedef struct _Gt1Region Gt1Region;

Gt1Region *gt1_region_new (void);
void *gt1_region_alloc (Gt1Region *r, int size);
void *gt1_region_realloc (Gt1Region *r, void *p, int old_size, int size);
void gt1_region_free (Gt1Region *r);

END_GNOME_DECLS

#endif /* __GT1_REGION_H__ */
