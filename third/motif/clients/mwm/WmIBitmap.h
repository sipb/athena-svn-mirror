/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $RCSfile: WmIBitmap.h,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:33 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Global Variables And Definitions:
 */



#ifdef MWM_NEED_IIMAGE
#ifdef MOTIF_DEFAULT_ICON
/*
 * Default icon image with four buttons:
 */

#define iImage_width 50
#define iImage_height 50
static unsigned char iImage_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0xe0,
   0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca,
   0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0,
   0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff,
   0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00,
   0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55,
   0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf,
   0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0,
   0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca,
   0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0,
   0xaa, 0xca, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0xe0, 0xff,
   0x3f, 0xe0, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x7f, 0xf0, 0xff, 0x7f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x40,
   0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf,
   0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0,
   0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa,
   0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00,
   0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff,
   0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5,
   0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40,
   0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf,
   0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0,
   0xff, 0xdf, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0xe0, 0xff,
   0x3f, 0xe0, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x7f, 0xf0, 0xff, 0x7f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};
#else
/*
 * Default icon image with X logo:
 */

#define iImage_width 50
#define iImage_height 50
static unsigned char iImage_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0xe0,
   0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca,
   0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0,
   0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff,
   0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00,
   0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55,
   0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0, 0xff, 0xdf,
   0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca, 0x00, 0xe0,
   0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0, 0xaa, 0xca,
   0x00, 0xe0, 0xff, 0xdf, 0x40, 0x55, 0xd5, 0x00, 0xe0, 0xff, 0xdf, 0xa0,
   0xaa, 0xca, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0xe0, 0xff,
   0x3f, 0xe0, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x7f, 0xf0, 0xff, 0x7f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x40,
   0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf,
   0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0,
   0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa,
   0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00,
   0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff,
   0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40, 0x55, 0xd5,
   0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf, 0x00, 0x40,
   0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0, 0xff, 0xdf,
   0x00, 0x40, 0x55, 0xd5, 0xe0, 0xff, 0xdf, 0x00, 0xa0, 0xaa, 0xca, 0xe0,
   0xff, 0xdf, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0xe0, 0xff,
   0x3f, 0xe0, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x7f, 0xf0, 0xff, 0x7f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};

#endif /* MOTIF_DEFAULT_ICON */
#endif /* MWM_NEED_IIMAGE */

#ifdef MWM_NEED_ICONBOX
#define iconBox_width 50
#define iconBox_height 50
static unsigned char iconBox_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00,
   0xd0, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x02, 0x01, 0x00, 0x00,
   0x00, 0x00, 0xf0, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0xf9,
   0xe3, 0x8f, 0x3f, 0xfe, 0x10, 0x02, 0x09, 0x24, 0x90, 0x40, 0x02, 0xd1,
   0x02, 0x09, 0x24, 0x90, 0x40, 0x02, 0xd1, 0x02, 0x09, 0x24, 0x90, 0x40,
   0x02, 0xd1, 0x02, 0x09, 0x24, 0x90, 0x40, 0x02, 0xd1, 0x02, 0x09, 0x24,
   0x90, 0x40, 0x02, 0xd1, 0x02, 0x09, 0x24, 0x90, 0x40, 0x02, 0xd1, 0x02,
   0x09, 0x24, 0x90, 0x40, 0x02, 0xd1, 0x02, 0xf1, 0xc7, 0x1f, 0x7f, 0xfc,
   0xd0, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x01, 0x00, 0x00,
   0x00, 0x00, 0x10, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0xf9, 0xe3, 0x8f, 0x3f, 0xfe, 0x10,
   0x02, 0x09, 0x24, 0x90, 0x40, 0x02, 0x11, 0x02, 0x09, 0x24, 0x90, 0x40,
   0x02, 0x11, 0x02, 0x09, 0x24, 0x90, 0x40, 0x02, 0x11, 0x02, 0x09, 0x24,
   0x90, 0x40, 0x02, 0x11, 0x02, 0x09, 0x24, 0x90, 0x40, 0x02, 0x11, 0x02,
   0x09, 0x24, 0x90, 0x40, 0x02, 0x11, 0x02, 0x09, 0x24, 0x90, 0x40, 0x02,
   0x11, 0x02, 0xf1, 0xc7, 0x1f, 0x7f, 0xfc, 0x10, 0x02, 0x01, 0x00, 0x00,
   0x00, 0x00, 0x10, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x02, 0xf9, 0xe3, 0x8f, 0x3f, 0x00, 0x10, 0x02, 0x09, 0x24, 0x90, 0x40,
   0x00, 0x10, 0x02, 0x09, 0x24, 0x90, 0x40, 0x00, 0x10, 0x02, 0x09, 0x24,
   0x90, 0x40, 0x00, 0x10, 0x02, 0x09, 0x24, 0x90, 0x40, 0x00, 0x10, 0x02,
   0x09, 0x24, 0x90, 0x40, 0x00, 0x10, 0x02, 0x09, 0x24, 0x90, 0x40, 0x00,
   0x10, 0x02, 0x09, 0x24, 0x90, 0x40, 0x00, 0x10, 0x02, 0xf1, 0xc7, 0x1f,
   0x7f, 0x00, 0x10, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x01,
   0x00, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd0,
   0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x02, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x03, 0x09, 0x00, 0x00, 0x00, 0x00, 0x12, 0x02, 0xcf, 0xff,
   0xff, 0x01, 0x00, 0x1e, 0x02, 0xcf, 0xff, 0xff, 0x01, 0x00, 0x1e, 0x02,
   0x09, 0x00, 0x00, 0x00, 0x00, 0x12, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x03};
#endif /* MWM_NEED_ICONBOX */



/*
 * Used to create the stipple for greyed icons
 */

#ifdef MWM_NEED_GREYED75
#define greyed75_width 16
#define greyed75_height 16
static unsigned char greyed75_bits[] = {
   0xee, 0xee, 0xbb, 0xbb, 0xee, 0xee, 0xbb, 0xbb, 0xee, 0xee, 0xbb, 0xbb,
   0xee, 0xee, 0xbb, 0xbb, 0xee, 0xee, 0xbb, 0xbb, 0xee, 0xee, 0xbb, 0xbb,
   0xee, 0xee, 0xbb, 0xbb, 0xee, 0xee, 0xbb, 0xbb};
#endif /* MWM_NEED_GREYED75 */



/*
 * Used to create the stipple for greyed icons
 */

#ifdef MWM_NEED_GREYED50
#define greyed50_width 16
#define greyed50_height 16
static unsigned char greyed50_bits[] = {
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa};
#endif /* MWM_NEED_GREYED50 */




/*
 * Used to create the stipple for greyed icons
 */

#ifdef MWM_NEED_GREYED25
#define greyed25_width 16
#define greyed25_height 16
static unsigned char greyed25_bits[] = {
   0x11, 0x11, 0x44, 0x44, 0x11, 0x11, 0x44, 0x44, 0x11, 0x11, 0x44, 0x44,
   0x11, 0x11, 0x44, 0x44, 0x11, 0x11, 0x44, 0x44, 0x11, 0x11, 0x44, 0x44,
   0x11, 0x11, 0x44, 0x44, 0x11, 0x11, 0x44, 0x44};
#endif /* MWM_NEED_GREYED25 */




/*
 * Used to create the stipple for greyed icons
 */

#ifdef MWM_NEED_SLANT2
#define slant2_width 16
#define slant2_height 16
static unsigned char slant2_bits[] = {
   0x99, 0x99, 0xcc, 0xcc, 0x66, 0x66, 0x33, 0x33, 0x99, 0x99, 0xcc, 0xcc,
   0x66, 0x66, 0x33, 0x33, 0x99, 0x99, 0xcc, 0xcc, 0x66, 0x66, 0x33, 0x33,
   0x99, 0x99, 0xcc, 0xcc, 0x66, 0x66, 0x33, 0x33};
#endif /* MWM_NEED_SLANT2 */
