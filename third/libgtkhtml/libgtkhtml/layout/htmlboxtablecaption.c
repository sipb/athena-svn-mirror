#include "htmlboxtable.h"
#include "htmlboxtablecaption.h"
#include "htmlrelayout.h"

void
html_box_table_caption_relayout_width (HtmlBoxTableCaption *caption, HtmlRelayout *relayout, gint width)
{
	caption->width = width;
	caption->height = 0;

	html_box_relayout (HTML_BOX (caption), relayout);
}

static void
html_box_table_caption_get_boundaries (HtmlBox *self, HtmlRelayout *relayout, gint *boxwidth, gint *boxheight)
{
	HtmlBoxTableCaption *caption = HTML_BOX_TABLE_CAPTION (self);

	*boxwidth  = caption->width - html_box_horizontal_mbp_sum (self);
	*boxheight = caption->height - html_box_vertical_mbp_sum (self);

	if (*boxwidth < 0)
		*boxwidth = 0;
	if (*boxheight < 0)
		*boxheight = 0;

	self->width  = *boxwidth  + html_box_horizontal_mbp_sum (self);
	self->height = *boxheight + html_box_vertical_mbp_sum (self);

	html_box_check_min_max_width_height (self, boxwidth, boxheight);
}

static void
html_box_table_caption_finalize (GObject *object)
{
	HtmlBoxTableCaption *caption = HTML_BOX_TABLE_CAPTION (object);
	HtmlBoxTable *table;

	if (!HTML_IS_BOX_TABLE (HTML_BOX (caption)->parent))
		return;

	table = HTML_BOX_TABLE (HTML_BOX (caption)->parent);
	html_box_table_remove_caption (table, caption);
}

static void
html_box_table_caption_class_init (HtmlBoxTableCaptionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	HtmlBoxBlockClass *block_class = (HtmlBoxBlockClass *) klass;

	object_class->finalize = html_box_table_caption_finalize;
	block_class->get_boundaries = html_box_table_caption_get_boundaries;
}

static void
html_box_table_caption_init (HtmlBox *box)
{
}

GType
html_box_table_caption_get_type (void)
{
       static GType html_type = 0;

       if (!html_type) {
               static GTypeInfo type_info = {
		       sizeof (HtmlBoxTableCaptionClass),
		       NULL,
		       NULL,
                       (GClassInitFunc) html_box_table_caption_class_init,
		       NULL,
		       NULL,
                       sizeof (HtmlBoxTableCaption),
		       16,
                       (GInstanceInitFunc) html_box_table_caption_init
               };

               html_type = g_type_register_static (HTML_TYPE_BOX_BLOCK, "HtmlBoxTableCaption", &type_info, 0);
       }
       return html_type;
}

HtmlBox *
html_box_table_caption_new (void)
{
	return g_object_new (HTML_TYPE_BOX_TABLE_CAPTION, NULL);
}


