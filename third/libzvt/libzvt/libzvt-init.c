#ifdef UNDEF 
#include <config.h>
#include <libzvt/libzvt.h>

#include <libgnomebase/libgnomebase.h>
#include <libgnomecanvas/libgnomecanvas.h>

static GnomeModuleRequirement libzvt_requirements[] = {
    /* We require libgnomebase setup to be run first as it
     * initializes the type system and some other stuff. */
    {VERSION, &libgnomecanvas_module_info},
    {NULL, NULL}
};

GnomeModuleInfo libzvt_module_info = {
    "libzvt", VERSION, "GNOME ZVT",
    libzvt_requirements,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL
};
#endif /* UNDEF */
