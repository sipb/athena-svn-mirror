#include <gst/gst.h>

/* gcc -ansi -pedantic on GNU/Linux causes warnings and errors
 * unless this is defined:
 * warning: #warning "Files using this header must be compiled with _SVID_SOURCE or _XOPEN_SOURCE"
 */
#ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 1
#endif

#define USE_SHM

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined (HAVE_IPC_H) && defined (HAVE_SHM_H) && defined (HAVE_XSHM_H)
#define USE_SHM
#endif

#ifdef USE_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#endif /* USE_SHM */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef USE_SHM
#include <X11/extensions/XShm.h>
#endif /* USE_SHM */

#include "gstvideosink.h"

typedef struct _GstXImageInfo GstXImageInfo;
struct _GstXImageInfo {
  GstImageInfo info;
  Display *display;
  Window window;
  GC gc;
  gint x, y, w, h;
};

typedef struct _GstXImageConnection GstXImageConnection;
struct _GstXImageConnection {
  GstImageConnection conn;
  Display *display;
  gint w, h;
  gint bpp;
};

typedef struct _GstXImage GstXImage;
struct _GstXImage
{
  GstImageData data;
#ifdef USE_SHM
  XShmSegmentInfo SHMInfo;
#else
  gpointer SHMInfo;
#endif
  XImage *ximage;
  
  GstXImageConnection *conn;
};

static GstXImageInfo *		gst_ximage_info		(GstImageInfo *info);
static GstXImageConnection *	gst_ximage_connection 	(GstImageConnection *conn);

static GstCaps * 		gst_ximage_get_caps	(GstImageInfo *info); 
static GstImageConnection *	gst_ximage_set_caps	(GstImageInfo *info, GstCaps *caps);
static GstImageData *		gst_ximage_get_image	(GstImageInfo *info, GstImageConnection *conn);
static void 			gst_ximage_put_image	(GstImageInfo *info, GstImageData *image);
static void 			gst_ximage_free_image	(GstImageData *image);
static void 			gst_ximage_open_conn	(GstImageConnection *conn, GstImageInfo *info);
static void 			gst_ximage_close_conn	(GstImageConnection *conn, GstImageInfo *info);
static void 			gst_ximage_free_conn	(GstImageConnection *conn);

GstImagePlugin* get_ximage_plugin(void)
{
  static GstImagePlugin plugin = { gst_ximage_get_caps,
				   gst_ximage_set_caps,
				   gst_ximage_get_image,
				   gst_ximage_put_image,
    				   gst_ximage_free_image};

  return &plugin;
}


static int XJ_caught_error;
static int 
XJ_error_catcher (Display * d, XErrorEvent * xeev)
{
  ++XJ_caught_error;
  return 0;
}

static GstXImageInfo *
gst_ximage_info (GstImageInfo *info)
{
  if (info == NULL || info->id != GST_MAKE_FOURCC ('X', 'l', 'i', 'b'))
  {
    return NULL;
  }
  return (GstXImageInfo *) info;
}
static GstXImageConnection *
gst_ximage_connection (GstImageConnection *conn)
{
  if (conn == NULL || conn->free_conn != gst_ximage_free_conn)
    return NULL; 
  return (GstXImageConnection *) conn;
}
GstCaps *
gst_ximage_get_caps (GstImageInfo *info)
{
  GstCaps *caps = NULL;
  Visual *visual;
  int xpad;
  XWindowAttributes attrib;
  XImage *ximage;
  GstXImageInfo *xinfo = gst_ximage_info (info);
  
  /* we don't handle these image information */
  if (xinfo == NULL) return NULL;

  XGetWindowAttributes(xinfo->display, xinfo->window, &attrib);
  
  visual = attrib.visual;
  if (attrib.depth <= 8)
    xpad = 8;
  else if (attrib.depth <= 16)
    xpad = 16;
  else
    xpad = 32;
  
  ximage = XCreateImage (xinfo->display, visual, attrib.depth, ZPixmap, 0, NULL, 
			      100, 100, xpad, (attrib.depth + 7) / 8 * 100);
  if (ximage != NULL) {
    caps = GST_CAPS_NEW (
	     "ximage_caps",
	     "video/raw",
	     "format",       GST_PROPS_FOURCC (GST_MAKE_FOURCC ('R', 'G', 'B', ' ')),
	       "bpp",        GST_PROPS_INT (ximage->bits_per_pixel),
	       "depth",      GST_PROPS_INT (attrib.depth),
	       "endianness", GST_PROPS_INT ((ImageByteOrder (xinfo->display) == LSBFirst) ? G_LITTLE_ENDIAN : G_BIG_ENDIAN),
	       "red_mask",   GST_PROPS_INT (visual->red_mask),
	       "green_mask", GST_PROPS_INT (visual->green_mask),
	       "blue_mask",  GST_PROPS_INT (visual->blue_mask),
	       "width",      GST_PROPS_INT_RANGE (0, G_MAXINT),
	       "height",     GST_PROPS_INT_RANGE (0, G_MAXINT)
	  );
    XDestroyImage (ximage);
  }
  
  GST_DEBUG (GST_CAT_PLUGIN_INFO, "XImage: returning caps at %p", caps);
  return caps;
}
static GstImageConnection *	
gst_ximage_set_caps (GstImageInfo *info, GstCaps *caps)
{
  GstXImageConnection *new = NULL;
  Visual *visual;
  XWindowAttributes attrib;
  GstXImageInfo *xinfo = gst_ximage_info (info);  
  guint32 format;
  gint depth;
  gint endianness;
  gint red_mask, green_mask, blue_mask;
  gint width, height, bpp;

  /* check if this is the right image info */
  if (xinfo == NULL) return NULL;
    
  XGetWindowAttributes(xinfo->display, xinfo->window, &attrib);
  
  visual = attrib.visual;

  gst_caps_get (caps,
		  "format", 	&format,
		  "depth",  	&depth,
		  "endianness", &endianness,
		  "red_mask", 	&red_mask,
		  "green_mask",	&green_mask,
		  "blue_mask",	&blue_mask,
		  "width",	&width,
		  "height",	&height,
		  "bpp",	&bpp,
		  NULL);
  
  /* check if the caps are ok */
  if (format != GST_MAKE_FOURCC ('R', 'G', 'B', ' ')) return NULL;
  /* if (gst_caps_get_int (caps, "bpp") != ???) return NULL; */
  if (depth != attrib.depth) return NULL;
  if (endianness != ((ImageByteOrder (xinfo->display) == LSBFirst) ? G_LITTLE_ENDIAN : G_BIG_ENDIAN)) return NULL;
  if (red_mask != visual->red_mask) return NULL;
  if (green_mask != visual->green_mask) return NULL;
  if (blue_mask != visual->blue_mask) return NULL;
  GST_DEBUG (GST_CAT_PLUGIN_INFO, "XImage: caps %p are ok, creating image", caps);
  
  new = g_new (GstXImageConnection, 1);
  new->conn.open_conn = gst_ximage_open_conn;
  new->conn.close_conn = gst_ximage_close_conn;
  new->conn.free_conn = gst_ximage_free_conn;
  new->display = xinfo->display;
  new->w = width;
  new->h = height;
  new->bpp = bpp;
  
  return (GstImageConnection *) new;
}
static GstImageData *
gst_ximage_get_image (GstImageInfo *info, GstImageConnection *conn)
{
  int (*old_handler)();
  GstXImage *new;
  XWindowAttributes attrib;
  GstXImageInfo *xinfo = gst_ximage_info (info);  
  GstXImageConnection *xconn = gst_ximage_connection (conn);  
  
  /* checks */
  if (xinfo == NULL) return NULL;
  if (xconn == NULL) return NULL;
  if (xinfo->display != xconn->display)
  {
    g_warning ("XImage: wrong x display specified in 'get_image'\n");
    return NULL;
  }
  
  XGetWindowAttributes(xinfo->display, xinfo->window, &attrib);
  
  new = g_new0(GstXImage, 1);
  new->conn = xconn;

  XJ_caught_error = 0;

  old_handler = XSetErrorHandler(XJ_error_catcher);
  XSync(xconn->display, 0);

  new->ximage = XShmCreateImage(new->conn->display, attrib.visual, 
			   attrib.depth, ZPixmap, NULL, &new->SHMInfo, new->conn->w, new->conn->h);
  if(!new->ximage) {
    g_warning ("CreateImage Failed\n");
    return NULL;
  }
 
  new->data.size = new->ximage->bytes_per_line * new->ximage->height;

  new->SHMInfo.shmid=shmget(IPC_PRIVATE, 
		       new->data.size,
		       IPC_CREAT|0777);

  if(new->SHMInfo.shmid < 0) {
    g_warning ("shmget failed:");
    g_free (new);
    return NULL;
  }
 
  new->data.data = new->ximage->data = new->SHMInfo.shmaddr = shmat(new->SHMInfo.shmid, 0, 0);

  XShmAttach(new->conn->display, &new->SHMInfo);

  XSync(new->conn->display, 0);
  XSetErrorHandler(old_handler);

  if (XJ_caught_error) {
    g_warning ("Shared memory unavailable, using regular images\n");
    shmdt(new->SHMInfo.shmaddr);
    new->SHMInfo.shmaddr = 0;

    new->data.data = g_malloc (((attrib.depth + 7) / 8) * new->conn->w * new->conn->h);
    new->ximage = XCreateImage(new->conn->display, attrib.visual, attrib.depth, ZPixmap, 0, new->data.data, new->conn->w, 
                               new->conn->h, new->conn->bpp, new->conn->w * (attrib.depth + 7) / 8);
    if(!new->ximage) {
      g_warning ("CreateImage Failed\n");
      g_free (new);
      return NULL;
    }
  }
    return (GstImageData *) new;
}


static void
gst_ximage_put_image (GstImageInfo *info, GstImageData *image)
{
  GstXImageInfo *xinfo = gst_ximage_info (info);
  GstXImage *im = (GstXImage *) image;
  
  g_assert (xinfo != NULL);
  
  if (im->SHMInfo.shmaddr) {
    XShmPutImage(xinfo->display, xinfo->window, xinfo->gc, im->ximage, 0, 0, 
                 xinfo->x + (xinfo->w - im->conn->w) / 2, xinfo->y + (xinfo->h - im->conn->h) / 2, im->conn->w, im->conn->h, False);
  } else {
    XPutImage(xinfo->display, xinfo->window, xinfo->gc, im->ximage, 0, 0, 
              xinfo->x + (xinfo->w - im->conn->w) / 2, xinfo->y + (xinfo->h - im->conn->h) / 2, im->conn->w, im->conn->h);
  }
  XSync(xinfo->display, False);
}
void
gst_ximage_free_image (GstImageData *image)
{
  GstXImage *im = (GstXImage *) image;
  if (im->ximage)
    XDestroyImage(im->ximage);
  XShmDetach(im->conn->display, &im->SHMInfo);
  if(im->SHMInfo.shmaddr)
    shmdt(im->SHMInfo.shmaddr);
  if(im->SHMInfo.shmid > 0)
    shmctl(im->SHMInfo.shmid, IPC_RMID, 0);
  else
    g_free (im->data.data);
  g_free (im);
}
static void
gst_ximage_open_conn (GstImageConnection *conn, GstImageInfo *info)
{
}
static void 
gst_ximage_close_conn (GstImageConnection *conn, GstImageInfo *info)
{
}
static void
gst_ximage_free_conn (GstImageConnection *conn)
{
  GstXImageConnection *xconn = gst_ximage_connection (conn);
  
  g_assert (xconn != NULL);
  
  g_free (xconn);
}

   
