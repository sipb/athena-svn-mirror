/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-omni2.cpp: IBM Omni driver
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
 *    Mark Hamzy <hamzy@us.ibm.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2001 International Business Machines Corp. and Ximian, Inc.
 *
 */

#define __GNOME_PRINT_OMNI_C__

#include <stdio.h>
#include <memory.h>

#include <gmodule.h>

#include <libgnome/gnome-paper.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-rfont.h>
#include "gnome-print-omni2.hpp"

const static int fDebugOutput = 1;

static void gnome_print_omni_init (GnomePrintOmni *pOmni);
static void gnome_print_omni_class_init (GnomePrintOmniClass *klass);
static void gnome_print_omni_finalize (GObject *object);

static gint gnome_print_omni_construct (GnomePrintContext *ctx);

static int      gnome_print_omni_print_band    (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *rect);
static gint     gnome_print_omni_showpage      (GnomePrintContext *pc);
static gint     gnome_print_omni_close         (GnomePrintContext *pc);
static void     gnome_print_omni_write_file    (void *pMagicCookie, unsigned char *puchData, int iSize);

const gchar *gp_omni_form_name_from_id (const gchar *id);

static GnomePrintRGBPClass *parent_class;

/**
 * gnome_print_omni_get_type:
 *
 * GType identification routine for #GnomePrintOmni
 *
 * Returns: The GType for the #GnomePrintOmni object
 */

GType
gnome_print_omni_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintOmniClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_omni_class_init,
			NULL, NULL,
			sizeof (GnomePrintOmni),
			0,
			(GInstanceInitFunc) gnome_print_omni_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_RGBP, "GnomePrintOmni", &info, 0);
	}
	return type;
}

static void
gnome_print_omni_init (GnomePrintOmni *pOmni)
{
	pOmni->pDevice = NULL;
}

static void
gnome_print_omni_class_init (GnomePrintOmniClass *klass)
{
	GObjectClass *object_class;
	GnomePrintContextClass *pc_class;
	GnomePrintRGBPClass *rgbp_class;

	object_class = (GObjectClass *)klass;
	pc_class = (GnomePrintContextClass *)klass;
	rgbp_class = (GnomePrintRGBPClass *)klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_print_omni_finalize;

	pc_class->construct = gnome_print_omni_construct;

	pc_class->showpage = gnome_print_omni_showpage;
	pc_class->close = gnome_print_omni_close;

	rgbp_class->print_band = gnome_print_omni_print_band;
}

static void
gnome_print_omni_finalize (GObject *object)
{
	GnomePrintOmni *pOmni;

	pOmni = GNOME_PRINT_OMNI (object);

	if (pOmni->pDevice) {
		delete pOmni->pDevice;
		pOmni->pDevice = NULL;
	}

#if 0
	if (pOmni->puchBitmapBuffer) {
		free (pOmni->puchBitmapBuffer);
		pOmni->puchBitmapBuffer = 0;
		pOmni->icbBitmapBuffer  = 0;
	}

	if (pOmni->vhDevice) {
		int rc = dlclose (pOmni->vhDevice);
		if (fDebugOutput) printf (__FUNCTION__ ": dlclose (0x%08x) = %d\n", (int)pOmni->vhDevice, rc);
	}

	if (pOmni->vhOmni) {
		int rc = dlclose (pOmni->vhOmni);
		if (fDebugOutput) printf (__FUNCTION__ ": dlclose (0x%08x) = %d\n", (int)pOmni->vhOmni, rc);
	}
#endif

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gnome_print_omni_construct (GnomePrintContext *ctx)
{
	GnomePrintOmni *pOmni;
	ArtDRect margins;
	gdouble dpix, dpiy;
	gint ret;
	guchar *pszDeviceName;
	PFNNEWDEVICEWARGS new_device;
	GModule *module;
	gchar *path;
	gchar *paper;
	const gchar *pszFormName;
	gchar *achJobProperties;
	int iScanlineMultiple;
	gint band_height;

	pOmni = GNOME_PRINT_OMNI (ctx);

	margins.x0 = 0.0;
	margins.y0 = 0.0;
	/* fixme: register/replace name */
	/* fixme: use margins here */
	margins.x1 = 21.0 * 72.0 / 2.54;
	margins.y1 = 29.7 * 72.0 / 2.54;
	gpa_node_get_double_path_value (ctx->config, (guchar *) "Settings.Output.Media.PhysicalSize.Width", &margins.x1);
	gpa_node_get_double_path_value (ctx->config, (guchar *) "Settings.Output.Media.PhysicalSize.Height", &margins.y1);

	/* fixme: register/replace name */
	dpix = 360.0;
	dpiy = 360.0;
	gpa_node_get_double_path_value (ctx->config, (guchar *) "Settings.Output.Resolution.DPI.X", &dpix);
	gpa_node_get_double_path_value (ctx->config, (guchar *) "Settings.Output.Resolution.DPI.Y", &dpiy);

	/* fixme: */
	ret = gnome_print_context_create_transport (ctx);
	g_return_val_if_fail (ret == GNOME_PRINT_OK, GNOME_PRINT_ERROR_UNKNOWN);
	ret = gnome_print_transport_open (ctx->transport);
	g_return_val_if_fail (ret == GNOME_PRINT_OK, GNOME_PRINT_ERROR_UNKNOWN);

	/* fixme: register/replace name */
	/* fixme: autoconf OMNi path */
	pszDeviceName = gpa_node_get_path_value (ctx->config, (guchar *) "Settings.Engine.Backend.Driver.Module.OmniModule");
	g_return_val_if_fail (pszDeviceName != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	/* fixme; */
	path = g_module_build_path ("/usr/lib/Omni", (gchar *) pszDeviceName);
	g_free (pszDeviceName);
	module = g_module_open (path, G_MODULE_BIND_LAZY);
	g_free (path);
	if (!module) {
		g_warning ("Cannot open OMNi module\n");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	if (!g_module_symbol (module, "newDevice__FPcb", (void **) &new_device)) {
		g_warning ("Cannot resolve symbol newDevice__FPcb\n");
		g_module_close (module);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	paper = (gchar *) gpa_node_get_path_value (ctx->config, (guchar *) "Settings.Output.Media.PhysicalSize");
	if (paper == NULL) paper = g_strdup ("A4");
	pszFormName = gp_omni_form_name_from_id (paper);
	g_free (paper);
	achJobProperties = g_strdup_printf ("orientation=PORTRAIT form=%s resolution=RESOLUTION_360_X_360", pszFormName);

	pOmni->pDevice = (* new_device) (achJobProperties, FALSE);

	pOmni->pDevice->setOutputFunction (gnome_print_omni_write_file, ctx);

	band_height = 256;
	iScanlineMultiple = pOmni->pDevice->getScanlineMultiple ();
	if ((band_height % iScanlineMultiple) != 0) {
		band_height += iScanlineMultiple - (band_height % iScanlineMultiple);
	}

	ret = gnome_print_rgbp_construct (GNOME_PRINT_RGBP (pOmni), &margins, dpix, dpiy, band_height);
	g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);

#if 0
	gchar *paper;

	char  achJobProperties[1024]; // @TBD

	/* fixme: */
	pOmni->iPaperWidth = margins.x1 * dpix;
	pOmni->iPaperHeight = margins.y1 * dpiy;

	/* fixme: autoconf path */
	pOmni->vhOmni = dlopen ("/usr/lib/Omni/libomni.so", RTLD_NOW | RTLD_GLOBAL);
	g_return_val_if_fail (pOmni->vhOmni != NULL, FALSE);

	pOmni->pDevice = 0;
	
	/* fixme */
	if (!pszDeviceName) pszDeviceName = g_strdup ("libEpson_Stylus_Color_740.so");
	pOmni->vhDevice = dlopen (pszDeviceName, RTLD_NOW | RTLD_GLOBAL);
	g_return_val_if_fail (pOmni->vhDevice != NULL, FALSE);

	pOmni->pfnNewDeviceWArgs = (PFNNEWDEVICEWARGS)dlsym (pOmni->vhDevice, "newDevice__FPcb");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (newDevice__FPcb) = 0x%08x\n", (int)pOmni->pfnNewDeviceWArgs);

	pOmni->pfnDeleteDevice = (PFNDELETEDEVICE)dlsym (pOmni->vhDevice, "deleteDevice__FP6Device");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (deleteDevice__FP6Device) = 0x%08x\n", (int)pOmni->pfnDeleteDevice);

	pOmni->pfnSetOutputFunction = (PFNSETOUTPUTFUNCTION)dlsym (pOmni->vhOmni, "SetOutputFunction__FPvPFPvPUci_vT0");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (SetOutputFunction__FPvPFPvPUci_vT0) = 0x%08x\n", (int)pOmni->pfnSetOutputFunction);

	pOmni->pfnBeginJob = (PFNBEGINJOB)dlsym (pOmni->vhOmni, "BeginJob__FPv");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (BeginJob__FPv) = 0x%08x\n", (int)pOmni->pfnBeginJob);

	pOmni->pfnNewPage = (PFNNEWPAGE)dlsym (pOmni->vhOmni, "NewPage__FPv");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (NewPage__FPv) = 0x%08x\n", (int)pOmni->pfnNewPage);

	pOmni->pfnEndJob = (PFNENDJOB)dlsym (pOmni->vhOmni, "EndJob__FPv");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (EndJob__FPv) = 0x%08x\n", (int)pOmni->pfnEndJob);

	pOmni->pfnRasterize = (PFNRASTERIZE)dlsym (pOmni->vhOmni, "Rasterize");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (Rasterize) = 0x%08x\n", (int)pOmni->pfnRasterize);

	pOmni->pfnFindResolutionName = (PFNFINDRESOLUTIONNAME)dlsym (pOmni->vhOmni, "FindResolutionName__FPci");
	if (fDebugOutput) printf (__FUNCTION__ ": dlsym (FindResolutionName) = 0x%08x\n", (int)pOmni->pfnFindResolutionName);

	if (!pOmni->pfnNewDeviceWArgs
	    || !pOmni->pfnDeleteDevice
	    || !pOmni->pfnSetOutputFunction
	    || !pOmni->pfnBeginJob
	    || !pOmni->pfnNewPage
	    || !pOmni->pfnEndJob
	    || !pOmni->pfnRasterize
	    || !pOmni->pfnFindResolutionName
		)
	{
		if (fDebugOutput) printf (__FUNCTION__ ": dlerror returns %s\n", dlerror ());
		g_print ("Here\n");
		return FALSE;
	}

	pOmni->iPageNumber      = 0;
	pOmni->icbBitmapBuffer  = 0;
	pOmni->puchBitmapBuffer = 0;

	paper = gpa_node_get_path_value (ctx->config, "Settings.Output.Media.PhysicalSize");
	if (paper == NULL) paper = g_strdup ("A4");
	pszFormName = gp_omni_form_name_from_id (paper);
	g_free (paper);
	sprintf (achJobProperties, "orientation=PORTRAIT form=FORM_A4 resolution=RESOLUTION_360_X_360");

	if (fDebugOutput) printf (__FUNCTION__ ": job properties are %s\n", achJobProperties);

	pOmni->pDevice = pOmni->pfnNewDeviceWArgs (achJobProperties, 0);
	if (fDebugOutput) printf (__FUNCTION__ ": pDevice = 0x%08x\n", (int)pOmni->pDevice);

	/* fixme: handle this */
	g_free (pszDeviceName);
#endif

	return GNOME_PRINT_OK;
}

static int
gnome_print_omni_print_band (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *pRect)
{
	GnomePrintContext *pc;
	GnomePrintOmni *pOmni;
	BITMAPINFO2 bmi2;
	RECTL rectlPageLocation;
	gint x, y, width, height;
	int icbBitmapScanline;
	int icbRGBScanline;
	int iBytesToAlloc;
	static gint band = 0;
	GnomeFont *font;
	GnomeRFont *rfont;
	gint glyph;
	static gdouble a[] = {1,0,0,1,0,0};

	pc = GNOME_PRINT_CONTEXT (rgbp);
	pOmni = GNOME_PRINT_OMNI (rgbp);

	// Width and Height
	width = pRect->x1 - pRect->x0;
	height = pRect->y1 - pRect->y0;

	// Set up the bitblt location structure
	rectlPageLocation.xLeft = pRect->x0;
	rectlPageLocation.xRight = pRect->x1 - 1;
	rectlPageLocation.yBottom = pRect->y0;
	rectlPageLocation.yTop = pRect->y1 - 1;

	// Setup the bitmap info structure
	bmi2.cbFix = sizeof (BITMAPINFO2) - sizeof (((PBITMAPINFO2) 0)->argbColor);
	bmi2.cx = width;
	bmi2.cy = height;
	bmi2.cPlanes = 1;
	bmi2.cBitCount = 24;
	bmi2.cclrUsed = 1 << bmi2.cBitCount;
	bmi2.cclrImportant = 0;

	icbBitmapScanline = (3 * width + 0x3) & 0xfffffffc;
	icbRGBScanline = width * 3;
	iBytesToAlloc = icbBitmapScanline * height;

	// Allocate enough space
	if (iBytesToAlloc > pOmni->icbBitmapBuffer) {
		if (pOmni->puchBitmapBuffer) {
			free (pOmni->puchBitmapBuffer);
			pOmni->puchBitmapBuffer = NULL;
			pOmni->icbBitmapBuffer = 0;
		}

		pOmni->puchBitmapBuffer = (unsigned char *) malloc (iBytesToAlloc);
		if (pOmni->puchBitmapBuffer) {
			pOmni->icbBitmapBuffer = iBytesToAlloc;
		}
	}

	for (y = 0; y < height; y++) {
		guchar *dp, *sp;
		sp = rgb_buffer + y * 3 * width;
		dp = pOmni->puchBitmapBuffer + y * icbBitmapScanline;
		for (x = 0; x < width; x++) {
			*dp++ = sp[2];
			*dp++ = sp[1];
			*dp++ = sp[0];
			sp += 3;
		}
	}

	// Test
	band++;
	font = gnome_font_new ("Helvetica", 36.0);
	glyph = gnome_font_lookup_default (font, '@' + band);
	rfont = gnome_font_get_rfont (font, a);
	gnome_rfont_render_glyph_rgb8 (rfont, glyph,
				       0x000000ff,
				       100,100,
				       pOmni->puchBitmapBuffer,
				       width, height, icbBitmapScanline,
				       0);
	gnome_rfont_unref (rfont);
	gnome_font_unref (font);

	// Bitblt it to omni
	pOmni->pDevice->rasterize (pOmni->puchBitmapBuffer, &bmi2, &rectlPageLocation, BITBLT_BITMAP);

	return GNOME_PRINT_OK;
}

static gint
gnome_print_omni_showpage (GnomePrintContext *pc)
{
	GnomePrintOmni *pOmni;

	pOmni = GNOME_PRINT_OMNI (pc);

	pOmni->iPageNumber++;

	if (pOmni->iPageNumber == 1) {
		// Notify omni of the start of the job
		pOmni->pDevice->beginJob ();
	} else {
		// Notify omni of the start of another page
		pOmni->pDevice->newFrame ();
	}

	if (((GnomePrintContextClass *) parent_class)->showpage)
		return (* ((GnomePrintContextClass *) parent_class)->showpage) (pc);

	return GNOME_PRINT_OK;
}

static gint
gnome_print_omni_close (GnomePrintContext *pc)
{
	GnomePrintOmni *pOmni;

	pOmni = GNOME_PRINT_OMNI (pc);

	// Notify omni of the end of the job
	pOmni->pDevice->endJob ();

	if (pc->transport) {
		gnome_print_transport_close (pc->transport);
		pc->transport = NULL;
	}

	if (((GnomePrintContextClass *) parent_class)->close)
		(* ((GnomePrintContextClass *) parent_class)->close) (pc);

	return GNOME_PRINT_OK;
}

static void
gnome_print_omni_write_file (void *pMagicCookie, unsigned char *puchData, int iSize)
{
	GnomePrintContext *ctx;

	g_assert (pMagicCookie != NULL);
	g_assert (GNOME_IS_PRINT_OMNI (pMagicCookie));

///if (fDebugOutput) printf (__FUNCTION__ ": pMagicCookie = 0x%08x, puchData = 0x%08x, iSize = %d\n", (int)pMagicCookie, (int)puchData, iSize);

   // Access our information
	ctx = GNOME_PRINT_CONTEXT (pMagicCookie);
	g_return_if_fail (ctx->transport);

	gnome_print_transport_write (ctx->transport, puchData, iSize);
}

/* Methods */

GnomePrintContext *
gnome_print_omni_new (GPANode *config)
{
	GnomePrintContext *ctx;
	gint ret;

	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (config), NULL);

	ctx = (GnomePrintContext *) g_object_new (GNOME_TYPE_PRINT_OMNI, NULL);

	ret = gnome_print_context_construct (ctx, config);

	if (ret != GNOME_PRINT_OK) {
		g_object_unref (G_OBJECT (ctx));
		g_warning ("Cannot construct OMNi driver");
		return NULL;
	}

	ret = gnome_print_transport_open (ctx->transport);

	return ctx;
}

/* Helpers */

const gchar *
gp_omni_form_name_from_id (const gchar *id)
{
	/* fixme: No need for case insensitivity here */
	if (!g_strcasecmp (id, "US-Letter"))
		return "FORM_LETTER";
	else if (!g_strcasecmp (id, "US-Legal"))
		return "FORM_LEGAL";
	else if (!g_strcasecmp (id, "A3"))
		return "FORM_A3";
	else if (!g_strcasecmp (id, "A4"))
		return "FORM_A4";
	else if (!g_strcasecmp (id, "A5"))
		return "FORM_A5";
	else if (!g_strcasecmp (id, "B4"))
		return "FORM_B4";
	else if (!g_strcasecmp (id, "B5"))
		return "FORM_B5";
	else if (!g_strcasecmp (id, "B5-Japan"))
		return "FORM_JIS_B5";
	else if (!g_strcasecmp (id, "Half-Letter"))
		return "FORM_HALF_LETTER";
	else if (!g_strcasecmp (id, "Executive"))
		return "FORM_EXECUTIVE";
	else if (!g_strcasecmp (id, "Tabloid/Ledger"))
		return "FORM_LEDGER";
	else if (!g_strcasecmp (id, "Monarch"))
		return "@TBD"; // @TBD
	else if (!g_strcasecmp (id, "SuperB"))
		return "FORM_SUPER_B";
	else if (!g_strcasecmp (id, "Envelope-Commercial"))
		return "@TBD"; // @TBD
	else if (!g_strcasecmp (id, "Envelope-Monarch"))
		return "FORM_MONARCH_ENVELOPE";
	else if (!g_strcasecmp (id, "Envelope-DL"))
		return "FORM_DL_ENVELOPE";
	else if (!g_strcasecmp (id, "Envelope-C5"))
		return "FORM_C5_ENVELOPE";
	else if (!g_strcasecmp (id, "EuroPostcard"))
		return "@TBD"; // @TBD

	/* fixme: I am bandit, I know (Lauris) */
	return "FORM_A4";
}

extern "C" {

GType gnome_print__driver_get_type (void);

GType
gnome_print__driver_get_type (void)
{
	return GNOME_TYPE_PRINT_OMNI;
}

}

