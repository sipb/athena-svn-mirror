#ifndef __GNOME_PRINT_OMNI_H__
#define __GNOME_PRINT_OMNI_H__

/*
 * IBM Omni driver
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Mark Hamzy <hamzy@us.ibm.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 International Business Machines Corp. and Ximian, Inc.
 *
 */

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-rgbp.h>
#include <Device.hpp>

#define GNOME_TYPE_PRINT_OMNI (gnome_print_omni_get_type ())
#define GNOME_PRINT_OMNI(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_OMNI, GnomePrintOmni))
#define GNOME_PRINT_OMNI_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GNOME_TYPE_PRINT_OMNI, GnomePrintOmniClass))
#define GNOME_IS_PRINT_OMNI(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_OMNI))
#define GNOME_IS_PRINT_OMNI_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GNOME_TYPE_PRINT_OMNI))
#define GNOME_PRINT_OMNI_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GNOME_TYPE_PRINT_OMNI, GnomePrintOmniClass))

typedef struct _GnomePrintOmni       GnomePrintOmni;
typedef struct _GnomePrintOmniClass  GnomePrintOmniClass;

#if 0
typedef unsigned char byte, BYTE, *PBYTE;

typedef struct _Rectl {
   int xLeft;
   int yBottom;
   int xRight;
   int yTop;
} RECTL, *PRECTL;

typedef struct _Sizel {
   int cx;
   int cy;
} SIZEL, *PSIZEL;

typedef struct _RGB2        /* rgb2 */
{
   BYTE bBlue;              /* Blue component of the color definition */
   BYTE bGreen;             /* Green component of the color definition*/
   BYTE bRed;               /* Red component of the color definition  */
   BYTE fcOptions;          /* Reserved, must be zero                 */
} RGB2, *PRGB2;

typedef struct _BitmapInfo {
   int  cbFix;
   int  cx;
   int  cy;
   int  cPlanes;
   int  cBitCount;
   int  ulCompresstion;
   int  cclrUsed;
   int  cclrImportant;
   RGB2 argbColor[1];
} BITMAPINFO2, *PBITMAPINFO2;

typedef enum {
   BITBLT_BITMAP,
   BITBLT_AREA,
   BITBLT_TEXT
} BITBLT_TYPE;

typedef void * (*PFNNEWDEVICEWARGS)     (char              *pszJobProperties,
                                         int                fAdvanced);
typedef void   (*PFNDELETEDEVICE)       (void              *pDevice);

typedef void   (*PFNOUTPUTFUNCTION)     (void              *pMagicCookie,
                                         unsigned char     *puchData,
                                         int                iSize);
typedef void   (*PFNSETOUTPUTFUNCTION)  (void              *pDev,
                                         PFNOUTPUTFUNCTION  pfn,
                                         void              *pMC);
typedef void   (*PFNBEGINJOB)           (void              *pDev);
typedef void   (*PFNNEWPAGE)            (void              *pDev);
typedef void   (*PFNENDJOB)             (void              *pDev);
typedef void   (*PFNRASTERIZE)          (void              *pDev,
                                         PBYTE              pbBits,
                                         PBITMAPINFO2       pbmi,
                                         PSIZEL             psizelPage,
                                         PRECTL             prectlPageLocation,
                                         BITBLT_TYPE        eType);

typedef char * (*PFNFINDRESOLUTIONNAME) (char              *pszDeviceName,
                                         int                iDpi);
#endif

struct _GnomePrintOmni {
	GnomePrintRGBP        rgbp;

	Device *pDevice;

	int icbBitmapBuffer;
	unsigned char *puchBitmapBuffer;
	int iPageNumber;

#if 0
	void                 *vhOmni;
	void                 *vhDevice;
	PFNNEWDEVICEWARGS     pfnNewDeviceWArgs;
	PFNDELETEDEVICE       pfnDeleteDevice;
	void                 *pDevice;
	PFNSETOUTPUTFUNCTION  pfnSetOutputFunction;
	PFNBEGINJOB           pfnBeginJob;
	PFNNEWPAGE            pfnNewPage;
	PFNENDJOB             pfnEndJob;
	PFNRASTERIZE          pfnRasterize;
	PFNFINDRESOLUTIONNAME pfnFindResolutionName;



	int iPaperWidth;
	int iPaperHeight;
#endif

#if 0
	/* Seems, that we do not need these (Lauris) */
	char                 *pszPaperProps;
	const GnomePaper     *paper_info;
#endif
};

struct _GnomePrintOmniClass {
	GnomePrintRGBPClass  parent_class;
};

extern "C" {
GType gnome_print_omni_get_type  (void);
}

#if 0
GnomePrintContext *gnome_print_omni_new (GPANode *config);
#endif

#endif

