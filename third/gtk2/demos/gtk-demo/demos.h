typedef	GtkWidget *(*GDoDemoFunc) (void);

typedef struct _Demo Demo;

struct _Demo 
{
  gchar *title;
  gchar *filename;
  GDoDemoFunc func;
  Demo *children;
};

GtkWidget *do_appwindow (void);
GtkWidget *do_button_box (void);
GtkWidget *do_changedisplay (void);
GtkWidget *do_colorsel (void);
GtkWidget *do_dialog (void);
GtkWidget *do_drawingarea (void);
GtkWidget *do_editable_cells (void);
GtkWidget *do_images (void);
GtkWidget *do_item_factory (void);
GtkWidget *do_list_store (void);
GtkWidget *do_menus (void);
GtkWidget *do_panes (void);
GtkWidget *do_pixbufs (void);
GtkWidget *do_sizegroup (void);
GtkWidget *do_stock_browser (void);
GtkWidget *do_textview (void);
GtkWidget *do_tree_store (void);

Demo child0[] = {
  { "Editable Cells", "editable_cells.c", do_editable_cells, NULL },
  { "List Store", "list_store.c", do_list_store, NULL },
  { "Tree Store", "tree_store.c", do_tree_store, NULL },
  { NULL } 
};

Demo testgtk_demos[] = {
  { "Application main window", "appwindow.c", do_appwindow, NULL }, 
  { "Button Boxes", "button_box.c", do_button_box, NULL }, 
  { "Change Display", "changedisplay.c", do_changedisplay, NULL }, 
  { "Color Selector", "colorsel.c", do_colorsel, NULL }, 
  { "Dialog and Message Boxes", "dialog.c", do_dialog, NULL }, 
  { "Drawing Area", "drawingarea.c", do_drawingarea, NULL }, 
  { "Images", "images.c", do_images, NULL }, 
  { "Item Factory", "item_factory.c", do_item_factory, NULL }, 
  { "Menus", "menus.c", do_menus, NULL }, 
  { "Paned Widgets", "panes.c", do_panes, NULL }, 
  { "Pixbufs", "pixbufs.c", do_pixbufs, NULL }, 
  { "Size Groups", "sizegroup.c", do_sizegroup, NULL }, 
  { "Stock Item and Icon Browser", "stock_browser.c", do_stock_browser, NULL }, 
  { "Text Widget", "textview.c", do_textview, NULL }, 
  { "Tree View", NULL, NULL, child0 },
  { NULL } 
};
