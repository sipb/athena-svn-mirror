#ifndef __CELL_RENDERER_URI_H__
#define __CELL_RENDERER_URI_H__

#include <gtk/gtkcellrenderertext.h>

G_BEGIN_DECLS

#define TYPE_CELL_RENDERER_URI         (cell_renderer_uri_get_type ())
#define CELL_RENDERER_URI(o)           (GTK_CHECK_CAST       ((o), TYPE_CELL_RENDERER_URI, CellRendererUri))
#define CELL_RENDERER_URI_CLASS(k)     (GTK_CHECK_CLASS_CAST ((k), TYPE_CELL_RENDERER_URI, CellRendererUriClass))
#define IS_CELL_RENDERER_URI(o)        (GTK_CHECK_TYPE       ((o), TYPE_CELL_RENDERER_URI))
#define IS_CELL_RENDERER_URI_CLASS(k)  (GTK_CHECK_CLASS_TYPE ((k), TYPE_CELL_RENDERER_URI))
#define CELL_RENDERER_URI_GET_CLASS(o) (GTK_CHECK_GET_CLASS  ((o), TYPE_CELL_RENDERER_URI, CellRendererUriClass))

typedef struct {
	GtkCellRendererText parent;

	char     *uri;
	gboolean  shown;
} CellRendererUri;

typedef struct {
	GtkCellRendererTextClass parent_class;

	void (*uri_visited) (CellRendererUri *cell,
			     const char      *path);
} CellRendererUriClass;

GType cell_renderer_uri_get_type (void);

G_END_DECLS

#endif /* __CELL_RENDERER_URI_H__ */
