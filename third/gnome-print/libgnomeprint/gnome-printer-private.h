#ifndef _GNOME_PRINTER_PRIVATE_H_
#define _GNOME_PRINTER_PRIVATE_H_

#include <gtk/gtkobject.h>

struct _GnomePrinter
{
	GtkObject object;

	char *driver;
	char *filename;
};

struct _GnomePrinterClass
{
	GtkObjectClass parent_class;
};

#endif

