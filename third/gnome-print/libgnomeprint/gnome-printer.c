/* Must include these two first */
#include "config.h"
#include <libgnomeprint/gnome-print-i18n.h>

#include <gtk/gtk.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-private.h>

static void gnome_printer_class_init (GnomePrinterClass *klass);
static void gnome_printer_init (GnomePrinter *printer);
static void gnome_printer_finalize (GtkObject *object);

static GtkObjectClass *parent_class = NULL;

GtkType
gnome_printer_get_type (void)
{
  static GtkType printer_type = 0;

  if (!printer_type)
    {
      GtkTypeInfo printer_info =
      {
	"GnomePrinter",
	sizeof (GnomePrinter),
	sizeof (GnomePrinterClass),
	(GtkClassInitFunc) gnome_printer_class_init,
	(GtkObjectInitFunc) gnome_printer_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      printer_type = gtk_type_unique (gtk_object_get_type (), &printer_info);
    }

  return printer_type;
}

static void
gnome_printer_class_init (GnomePrinterClass *class)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass*) class;

  parent_class = gtk_type_class (gtk_object_get_type ());

  object_class->finalize = gnome_printer_finalize;


}

static void
gnome_printer_init (GnomePrinter *printer)
{
  printer->filename = NULL;
}

GnomePrinter *
gnome_printer_new_generic_ps (const char *filename)
{
  GnomePrinter *printer;

  printer = gtk_type_new (gnome_printer_get_type ());
  printer->driver = g_strdup ("gnome-print-ps");
  printer->filename = g_strdup (filename);

  return printer;
}


static void
gnome_printer_finalize (GtkObject *object)
{
  GnomePrinter *printer;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GNOME_IS_PRINTER (object));

  printer = GNOME_PRINTER (object);

  if (printer->filename)
    g_free (printer->filename);
  if (printer->driver)
    g_free (printer->driver);

  (* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

/**
 * gnome_printer_get_status:
 * @printer: The printer
 *
 * Returns the current status of @printer
 */
GnomePrinterStatus
gnome_printer_get_status (GnomePrinter *printer)
{
	g_return_val_if_fail (printer != NULL, GNOME_PRINTER_INACTIVE);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), GNOME_PRINTER_INACTIVE);

	return GNOME_PRINTER_INACTIVE;
}

/**
 * gnome_printer_str_status
 * @status: A status type
 *
 * Returns a string representation of the printer status code @status
 */
const char *
gnome_printer_str_status (GnomePrinterStatus status)
{
	switch (status){
	case GNOME_PRINTER_ACTIVE:
		return _("Printer is active");

	case GNOME_PRINTER_INACTIVE:
		return _("Printer is ready to print");

	case GNOME_PRINTER_OFFLINE:
		return _("Printer is off-line");

	case GNOME_PRINTER_NET_FAILURE:
		return _("Can not communicate with printer");

	}
	return _("Unknown status");
}
