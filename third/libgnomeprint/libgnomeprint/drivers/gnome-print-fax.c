/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-fax.c: group 3 fax driver
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Roberto Majadas  <phoenix@nova.es>
 *
 *  Copyright 2000-2001 Ximian, Inc. and authors
 *
 */

#define __GNOME_PRINT_FAX_C__

#include <math.h>
#include <string.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-print-transport.h>

#include "gnome-print-fax.h"
#include "gnome-print-fax-g3.h"

#define GNOME_PRINT_FAX_MAX_COLS 1728 /* 0 to 1728 cols of A4-paper */

#define GNOME_PRINT_FAX_IOL_YES 1
#define GNOME_PRINT_FAX_IOL_NO 0

#define GNOME_PRINT_FAX_LASTCODE_YES 1
#define GNOME_PRINT_FAX_LASTCODE_NO 0

#define GNOME_PRINT_FAX_COLOR_BLACK 1
#define GNOME_PRINT_FAX_COLOR_WHITE 0

/* Gnome-print-fax prototipes */

static void gnome_print_fax_encode_of_row (GnomePrintContext *pc);
static void gnome_print_fax_encode_finish_of_row (GnomePrintContext *pc, gint cols);
static void gnome_print_fax_encode_rules_apply (GnomePrintContext *pc);
static void gnome_print_fax_code_write (GnomePrintContext *pc, struct g3table node, int lastcode);
static void gnome_print_fax_code_eol (GnomePrintContext *pc);
static void gnome_print_fax_code_eof (GnomePrintContext *pc);
static void gnome_print_fax_code (GnomePrintContext *pc, int run_length, int color, int iol);
static gint gnome_print_fax_ditering (guchar *rgb_buffer, gint actual_col, gint offset);

static gint gnome_print_fax_construct (GnomePrintContext *ctx);

GType gnome_print__driver_get_type (void);

static GnomePrintRGBPClass *parent_class;

/**
 * gnome_print_fax_code_write: This funtion write in the g3's file the respective code
 * @pc: 
 * @node:
 * @lastcode: 
 * 
 * 
 **/

static void
gnome_print_fax_code_write (GnomePrintContext *pc, struct g3table node, int lastcode)
{
	GnomePrintFAX *fax;
	gint power_of_2[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
	gint i,j;

	fax = GNOME_PRINT_FAX (pc);

	for (i = node.length; i > 0; )
	{
		if (fax->priv->fax_encode_buffer_pivot < 0)
		{
			fax->priv->fax_encode_buffer_pivot = 7;
			gnome_print_transport_write (pc->transport, (guchar *) &fax->priv->fax_encode_buffer, 1);
			fax->priv->fax_encode_buffer = 0;
		}

		j = node.code&power_of_2[i-1];
		
		if (j!=0)
		{
			fax->priv->fax_encode_buffer |= power_of_2[fax->priv->fax_encode_buffer_pivot];
		}

		fax->priv->fax_encode_buffer_pivot -= 1;
		i -= 1;
	}
	
	if (lastcode)
	{
		gnome_print_transport_write (pc->transport, (guchar *) &fax->priv->fax_encode_buffer, 1);
       	}
}


/**
 * gnome_print_fax_code_eol: this funtion write in the g3-file the EOL code
 * @pc: 
 * 
 * 
 **/

static void
gnome_print_fax_code_eol (GnomePrintContext *pc)
{
	gnome_print_fax_code_write (pc, g3eol, GNOME_PRINT_FAX_LASTCODE_NO);
}


/**
 * gnome_print_fax_code_eof:this funtion write in the g3-file the EOF code
 * @pc: 
 * 
 * 
 **/

static void
gnome_print_fax_code_eof (GnomePrintContext *pc)
{
	gnome_print_fax_code_write (pc, g3eol, GNOME_PRINT_FAX_LASTCODE_NO);
	gnome_print_fax_code_write (pc, g3eol, GNOME_PRINT_FAX_LASTCODE_NO);
	gnome_print_fax_code_write (pc, g3eol, GNOME_PRINT_FAX_LASTCODE_NO);
	gnome_print_fax_code_write (pc, g3eol, GNOME_PRINT_FAX_LASTCODE_NO);
	gnome_print_fax_code_write (pc, g3eol, GNOME_PRINT_FAX_LASTCODE_NO);
	gnome_print_fax_code_write (pc, g3eol, GNOME_PRINT_FAX_LASTCODE_YES);	
}


/**
 * gnome_print_fax_code:
 * @pc: 
 * @run_length: 
 * @color: 
 * @iol: 
 * 
 * 
 **/
static void
gnome_print_fax_code (GnomePrintContext *pc, int run_length, int color, int iol)
{
	
	if (run_length < 64)
	{
		if (color == GNOME_PRINT_FAX_COLOR_BLACK)
		{
			if (iol == GNOME_PRINT_FAX_IOL_YES)
			{
				gnome_print_fax_code_write (pc, terminating_white_table[0],
							    GNOME_PRINT_FAX_LASTCODE_NO);
				gnome_print_fax_code_write (pc, terminating_black_table[run_length],
							    GNOME_PRINT_FAX_LASTCODE_NO);
			}
			else
			{
				gnome_print_fax_code_write (pc, terminating_black_table[run_length],
							    GNOME_PRINT_FAX_LASTCODE_NO);
			}
		}
		else
		{
			gnome_print_fax_code_write (pc, terminating_white_table[run_length],
						    GNOME_PRINT_FAX_LASTCODE_NO);
		}
	}
	else if (run_length <= 1728)
	{
		gint x;
		gint y;
		
		x = run_length / 64;
		y = run_length % 64;
		
		if (color == GNOME_PRINT_FAX_COLOR_BLACK)
		{
			if (iol == GNOME_PRINT_FAX_IOL_YES)
			{					
				gnome_print_fax_code_write (pc, terminating_white_table[0],
							    GNOME_PRINT_FAX_LASTCODE_NO);
				gnome_print_fax_code_write (pc, makeup_black_table[x-1],
							    GNOME_PRINT_FAX_LASTCODE_NO);
				gnome_print_fax_code_write (pc, terminating_black_table[y],
							    GNOME_PRINT_FAX_LASTCODE_NO);
			}
			else
			{
				gnome_print_fax_code_write (pc, makeup_black_table[x-1],
							    GNOME_PRINT_FAX_LASTCODE_NO);
				gnome_print_fax_code_write (pc, terminating_black_table[y],
							    GNOME_PRINT_FAX_LASTCODE_NO);
			}
		}
		else
		{
			gnome_print_fax_code_write (pc, makeup_white_table[x-1],
						    GNOME_PRINT_FAX_LASTCODE_NO);
			gnome_print_fax_code_write (pc, terminating_white_table[y],
						    GNOME_PRINT_FAX_LASTCODE_NO);
		}			
	}
}


/**
 * gnome_print_fax_ditering: this funtion decide if a pixel is a black pixel or white pixel
 * @rgb_buffer: 
 * @actual_col: 
 * @offset: 
 * 
 * 
 * 
 * Return Value: 
 **/
 
static gint
gnome_print_fax_ditering (guchar *rgb_buffer, gint actual_col, gint offset)
{
	gint j ;

	j = actual_col;
		
	if (rgb_buffer [offset+j] + rgb_buffer [offset+j+1] + rgb_buffer [offset+j+2] < (200*3))
	{
		return GNOME_PRINT_FAX_COLOR_BLACK;
	}
	else
	{
		return GNOME_PRINT_FAX_COLOR_WHITE;
	}

}

/**
 * gnome_print_fax_encode_rules_apply: this funtion apply de rules for encode to g3
 * @pc: 
 * 
 * 
 **/

static void
gnome_print_fax_encode_rules_apply (GnomePrintContext *pc)
{

	GnomePrintFAX *fax;
	
	fax = GNOME_PRINT_FAX (pc);

	if (fax->priv->first_code_of_row == TRUE)
	{
		fax->priv->first_code_of_row = FALSE;
		gnome_print_fax_code (pc, fax->priv->run_length,
				      fax->priv->run_length_color, GNOME_PRINT_FAX_IOL_YES);	
	}
	else
	{
		gnome_print_fax_code (pc, fax->priv->run_length,
				      fax->priv->run_length_color, GNOME_PRINT_FAX_IOL_NO);
	}
}


/**
 * gnome_print_fax_encode_finish_of_row: complete the row with white pixels
 * @pc: 
 * @cols: 
 * 
 * 
 * 
 * Return Value: 
 **/
	
static void
gnome_print_fax_encode_finish_of_row (GnomePrintContext *pc, gint cols)
{

	GnomePrintFAX *fax;
	
	fax = GNOME_PRINT_FAX (pc);

	if (fax->priv->actual_color == GNOME_PRINT_FAX_COLOR_WHITE)
	{
		fax->priv->run_length += GNOME_PRINT_FAX_MAX_COLS - cols;
		gnome_print_fax_encode_rules_apply (pc);
		
	}
	else
	{
		gnome_print_fax_encode_rules_apply (pc);

		if (cols < GNOME_PRINT_FAX_MAX_COLS)
		{
			gnome_print_fax_code (pc, GNOME_PRINT_FAX_MAX_COLS - cols,
					      GNOME_PRINT_FAX_COLOR_WHITE, GNOME_PRINT_FAX_IOL_NO);
		}
	}
	
}	

/**
 * gnome_print_fax_encode_of_row: This funtion encode a row
 * @pc: 
 * 
 * 
 **/

static void
gnome_print_fax_encode_of_row (GnomePrintContext *pc)
{
	 	
	GnomePrintFAX *fax;
	
	fax = GNOME_PRINT_FAX (pc);
	
	if (fax->priv->run_length_color == fax->priv->actual_color)
	{
		fax->priv->run_length++;
	}
	else
	{
		gnome_print_fax_encode_rules_apply (pc);
		fax->priv->run_length_color = !fax->priv->run_length_color;
		fax->priv->run_length = 1;
	}
}

static int
gnome_print_fax_print_band (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *rect)
{
	GnomePrintContext *pc;
	GnomePrintFAX *fax;
	gint rows, actual_row, cols, actual_col, offset; 

	pc = GNOME_PRINT_CONTEXT (rgbp);
	fax = GNOME_PRINT_FAX (rgbp);
	
	
	rows = rect->y1 - rect->y0;
	cols = rect->x1 - rect->x0;

	g_return_val_if_fail (cols <= GNOME_PRINT_FAX_MAX_COLS, -1);
	
	if (fax->priv->first_code_of_doc == TRUE)
	{			
		gnome_print_fax_code_eol (pc) ;
		fax->priv->first_code_of_doc = FALSE ;
	}
	
	for (actual_row = 0; actual_row < rows; actual_row++)
	{

	       	actual_col = 0 ;
		offset = actual_row * cols * 3 ;

		fax->priv->actual_color = gnome_print_fax_ditering (rgb_buffer, actual_col, offset) ;
		fax->priv->run_length_color = fax->priv->actual_color ;
		fax->priv->run_length = 1;
		fax->priv->first_code_of_row = TRUE ;		
		
		for (actual_col = 1; actual_col < cols; actual_col++)
		{
			fax->priv->actual_color = gnome_print_fax_ditering (rgb_buffer, actual_col, offset) ;
			gnome_print_fax_encode_of_row (pc);
		}		
		
		gnome_print_fax_encode_finish_of_row (pc, cols);
				
		gnome_print_fax_code_eol (pc) ;
	}
			
	return 1;
}


static int
gnome_print_fax_page_end (GnomePrintRGBP *rgbp)
{
	GnomePrintContext *pc;

	g_return_val_if_fail (GNOME_IS_PRINT_RGBP (rgbp), -1);
	pc = GNOME_PRINT_CONTEXT (rgbp);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
		
	return 0;
}

#ifdef KILL_COMPILE_WARNING
static int
gnome_print_fax_page_begin (GnomePrintContext *pc)
{
	g_print ("Page begin\n");
	return 0;
}
#endif

static int
gnome_print_fax_close (GnomePrintContext *pc)
{
	gnome_print_fax_code_eof (pc);

	if (pc->transport) {
		gnome_print_transport_close (pc->transport);
		pc->transport = NULL;
	}

	if (((GnomePrintContextClass *) parent_class)->close)
		return (* ((GnomePrintContextClass *) parent_class)->close) (pc);

	return GNOME_PRINT_OK;
}

static void
gnome_print_fax_class_init (GObjectClass *klass)
{
	GnomePrintRGBPClass *rgbp_class = (GnomePrintRGBPClass *) klass;
	GnomePrintContextClass *pc_class = (GnomePrintContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	pc_class->close = gnome_print_fax_close;
	pc_class->construct = gnome_print_fax_construct;

	rgbp_class->print_band = gnome_print_fax_print_band;
	rgbp_class->page_end   = gnome_print_fax_page_end;
}

static gint
gnome_print_fax_construct (GnomePrintContext *ctx)
{
	GnomePrintFAX *fax;
	ArtDRect rect;
	gdouble dpix, dpiy;
	gboolean result;
	const GnomePrintUnit *unit;

	fax = GNOME_PRINT_FAX (ctx);

	fax->priv = g_new (GnomePrintFAXPrivate, 1);
	
	fax->priv->fax_encode_buffer_pivot = 7;
	fax->priv->first_code_of_doc = TRUE ;
	
	rect.x0 = 0.0;
	rect.y0 = 0.0;
	rect.x1 = 21.0 * 72.0 / 2.54;
	rect.y1 = 29.7 * 72.0 / 2.54;
	dpix = 198.0;
	dpiy = 198.0;
	
	if (gnome_print_config_get_length (ctx->config, GNOME_PRINT_KEY_PAPER_WIDTH, &rect.x1, &unit)) {
		gnome_print_convert_distance (&rect.x1, unit, GNOME_PRINT_PS_UNIT);
	}
	if (gnome_print_config_get_length (ctx->config, GNOME_PRINT_KEY_PAPER_HEIGHT, &rect.y1, &unit)) {
		gnome_print_convert_distance (&rect.x1, unit, GNOME_PRINT_PS_UNIT);
	}
	gnome_print_config_get_double (ctx->config, GNOME_PRINT_KEY_RESOLUTION_DPI_X, &dpix);
	gnome_print_config_get_double (ctx->config, GNOME_PRINT_KEY_RESOLUTION_DPI_Y, &dpiy);

	/* fixme: should rgbp_construct take settings as argument? */
	/* telemaco : the fax driver need width=1728 pixels , the heigth is arbitrary ..
	   usually heigth=2100 pixels . And the resolution we take it from gpa . */
	
	if (!gnome_print_rgbp_construct (GNOME_PRINT_RGBP (fax), &rect, dpix, dpiy, 256)) return GNOME_PRINT_ERROR_UNKNOWN;

	result = gnome_print_context_create_transport (ctx);
	g_return_val_if_fail (result == GNOME_PRINT_OK, GNOME_PRINT_ERROR_UNKNOWN);
	result = gnome_print_transport_open (ctx->transport);
	g_return_val_if_fail (result == GNOME_PRINT_OK, GNOME_PRINT_ERROR_UNKNOWN);

	return GNOME_PRINT_OK;
}

GnomePrintContext *
gnome_print_fax_new (GnomePrintConfig *config)
{
	GnomePrintContext *ctx;

	g_return_val_if_fail (config != NULL, NULL);

	ctx = g_object_new (GNOME_TYPE_PRINT_FAX, NULL);

	if (!gnome_print_context_construct (ctx, config)) {
		g_object_unref (G_OBJECT (ctx));
		g_warning ("Cannot construct fax driver");
		return NULL;
	}

	return ctx;
}

GType
gnome_print_fax_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintFAXClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_fax_class_init,
			NULL, NULL,
			sizeof (GnomePrintFAX),
			0,
			(GInstanceInitFunc) NULL
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_RGBP, "GnomePrintFAX", &info, 0);
	}
	return type;
}

GType
gnome_print__driver_get_type (void)
{
	return GNOME_TYPE_PRINT_FAX;
}

