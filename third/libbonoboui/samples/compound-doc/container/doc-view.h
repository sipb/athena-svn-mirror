#ifndef _SAMPLE_DOC_VIEW_H
#define _SAMPLE_DOC_VIEW_H

#include <bonobo/Bonobo.h>
#include <gtk/gtkwidget.h>

#include "document.h"

GtkWidget *sample_doc_view_new (SampleDoc *, Bonobo_UIContainer);

#endif
