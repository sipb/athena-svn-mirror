#include "config.h"

#include <gst/gst.h>
/* gcc -ansi -pedantic on GNU/Linux causes warnings and errors
 * unless this is defined:
 * warning: #warning "Files using this header must be compiled with _SVID_SOURCE or _XOPEN_SOURCE"
 */
#ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 1
#endif

#define USE_SHM

#if defined (HAVE_IPC_H) && defined (HAVE_SHM_H) && defined (HAVE_XSHM_H)
#define USE_SHM
#endif

#ifdef USE_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#else /* !USE_SHM */
#include <X11/Xlib.h>
#endif /* !USE_SHM */

#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <string.h>

#include "gstvideosink.h"

typedef struct _GstXImageInfo GstXImageInfo;
struct _GstXImageInfo {
  GstImageInfo info;
  Display *display;
  Window window;
  GC gc;
  gint x, y, w, h;
};

typedef struct _GstXvConnection GstXvConnection;
struct _GstXvConnection {
  GstImageConnection conn;
  XvPortID port;
  Display *display;
  int format;
  gint w, h;
};

typedef struct _GstXvImage GstXvImage;
struct _GstXvImage
{
  GstImageData data;
#ifdef USE_SHM
  XShmSegmentInfo *shm_info;
#else
  gpointer SHMInfo;
#endif
  XvImage *xvimage;
  
  GstXvConnection *conn;
};


static GstXImageInfo *		gst_ximage_info			(GstImageInfo *info);
static GstXvConnection *	gst_xv_connection 		(GstImageConnection *conn);
static gboolean 		gst_xvimage_check_xvideo	(Display *display);

static GstCaps * 		gst_xvimage_get_caps		(GstImageInfo *info); 
static GstImageConnection *	gst_xvimage_set_caps		(GstImageInfo *info, GstCaps *caps);
static GstImageData *		gst_xvimage_get_image		(GstImageInfo *info, GstImageConnection *conn);
static void 			gst_xvimage_put_image		(GstImageInfo *info, GstImageData *image);
static void 			gst_xvimage_free_image		(GstImageData *image);
static void 			gst_xvimage_open_conn		(GstImageConnection *conn, GstImageInfo *info);
static void 			gst_xvimage_close_conn		(GstImageConnection *conn, GstImageInfo *info);
static void 			gst_xvimage_free_conn		(GstImageConnection *conn);

GstImagePlugin* get_xvimage_plugin(void)
{
  static GstImagePlugin plugin = { gst_xvimage_get_caps,
				   gst_xvimage_set_caps,
				   gst_xvimage_get_image,
				   gst_xvimage_put_image,
    				   gst_xvimage_free_image};

  return &plugin;
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
static GstXvConnection *
gst_xv_connection (GstImageConnection *conn)
{
  if (conn == NULL || conn->free_conn != gst_xvimage_free_conn)
    return NULL; 
  return (GstXvConnection *) conn;
}
gboolean
gst_xvimage_check_xvideo (Display *display)
{
  int ver, rel, req, ev, err;
  
  if (display == NULL)
    return FALSE;
  if (Success == XvQueryExtension (display,&ver,&rel,&req,&ev,&err))
    return TRUE;

  return FALSE;
}

static GstCaps * 	
gst_xvimage_get_caps (GstImageInfo *info)
{
  gint i;
  int adaptors;
  XvAdaptorInfo *ai;
  int formats;
  XvImageFormatValues *fo;  
  GstCaps *caps = NULL;
  GstXImageInfo *xinfo = gst_ximage_info (info);
  
  /* we don't handle these image information */
  if (xinfo == NULL) return NULL;
  g_return_val_if_fail (xinfo->display != NULL, NULL);
  
  if (gst_xvimage_check_xvideo (xinfo->display) == FALSE)
  {
    g_warning("XvImage: Server has no Xvideo extention support\n");
    return NULL;
  }

  if (Success != XvQueryAdaptors (xinfo->display, DefaultRootWindow (xinfo->display), &adaptors, &ai))
  {
    g_warning("XvImage: XvQueryAdaptors failed\n");
    return NULL;
  }
  GST_INFO(GST_CAT_PLUGIN_INFO, "XvImage: %d adaptors available\n", adaptors);

  for (i = 0; i < adaptors; i++) {
    GST_INFO(GST_CAT_PLUGIN_INFO, "XvImage: %s:%s%s%s%s%s, ports %ld-%ld\n",
                     ai[i].name,
	            (ai[i].type & XvInputMask)  ? " input"  : "",
		    (ai[i].type & XvOutputMask) ? " output" : "",
		    (ai[i].type & XvVideoMask)  ? " video"  : "",
		    (ai[i].type & XvStillMask)  ? " still"  : "",
		    (ai[i].type & XvImageMask)  ? " image"  : "",
		     ai[i].base_id,
		     ai[i].base_id+ai[i].num_ports-1);

    if ((ai[i].type & XvInputMask) &&
        (ai[i].type & XvImageMask)) 
    {
      gint j;
      
      /* *** image scaler port *** */
      fo = XvListImageFormats(xinfo->display, ai[i].base_id, &formats);

      GST_INFO(GST_CAT_PLUGIN_INFO,"XvImage: image format list for port %d\n", (gint) ai[i].base_id);
      for(j = 0; j < formats; j++) {
        gulong fourcc;
        fourcc = GULONG_FROM_LE(fo[j].id);
        GST_INFO(GST_CAT_PLUGIN_INFO, "    0x%x (%4.4s) %s %.32s (%d:%d;%d,%d:%d:%d,%d:%d:%d)\n",
                       fo[j].id,
                       (char *) &fourcc,
                       (fo[j].format == XvPacked) ? "packed" : "planar",
		       fo[j].component_order,
		       fo[j].y_sample_bits,
		       fo[j].u_sample_bits,
		       fo[j].v_sample_bits,
		       fo[j].horz_y_period,
		       fo[j].horz_u_period,
		       fo[j].horz_v_period,
		       fo[j].vert_y_period,
		       fo[j].vert_u_period,
		       fo[j].vert_v_period);
        caps = gst_caps_append (caps, GST_CAPS_NEW (
	            "xvimage_caps",
	            "video/raw",
		      "format",  GST_PROPS_FOURCC (fo[j].id),
		      "width",   GST_PROPS_INT_RANGE (0, G_MAXINT),
		      "height",  GST_PROPS_INT_RANGE (0, G_MAXINT))
	       );
      }
    }
  }
  return caps;
}
static GstImageConnection *
gst_xvimage_set_caps (GstImageInfo *info, GstCaps *caps)
{
  gint i, j = 0;
  int adaptors;
  XvAdaptorInfo *ai;
  int formats;
  XvImageFormatValues *fo = NULL;  
  GstXvConnection *conn;
  GstXImageInfo *xinfo = gst_ximage_info (info);
  guint32 format;
  
  /* we don't handle these image information */
  if (xinfo == NULL) return NULL;
  
  conn = g_new0 (GstXvConnection, 1);
  conn->conn.open_conn = gst_xvimage_open_conn;
  conn->conn.close_conn = gst_xvimage_close_conn;
  conn->conn.free_conn = gst_xvimage_free_conn;

  gst_caps_get (caps, 
		  "width",  &conn->w,
		  "height", &conn->h,
		  "format", &format,
                  NULL);

  conn->port = (XvPortID) -1;
  conn->display = xinfo->display;

  if (Success != XvQueryAdaptors (xinfo->display, DefaultRootWindow (xinfo->display), &adaptors, &ai))
  {
    g_warning("XvImage: XvQueryAdaptors failed\n");
    g_free (conn);
    return NULL;
  }
  for (i = 0; conn->port == (XvPortID) -1 && i < adaptors; i++) {
    if ((ai[i].type & XvInputMask) &&
        (ai[i].type & XvImageMask)) 
    {	      
      fo = XvListImageFormats(xinfo->display, ai[i].base_id, &formats);
      for (j = 0; j < formats; j++)
      {
        if (format == fo[j].id) {
          conn->port = ai[i].base_id;
	  conn->format = fo[j].id;
          break;
        }
      }
    }
  }
  if (conn->port == (XvPortID) -1)
  {
    /* this happens if the plugin can't handle the caps, so no warning */
    g_free (conn);
    return NULL;
  }
  
  return (GstImageConnection *) conn;
}
static GstImageData *
gst_xvimage_get_image (GstImageInfo *info, GstImageConnection *conn)
{
  GstXvImage *image;
  GstXImageInfo *xinfo = gst_ximage_info (info);  
  GstXvConnection *xvconn = gst_xv_connection (conn);  
  
  /* checks */
  if (xinfo == NULL) return NULL;
  if (xvconn == NULL) return NULL;
  if (xinfo->display != xvconn->display)
  {
    g_warning ("XImage: wrong x display specified in 'get_image'\n");
    return NULL;
  }
  
  image = g_new0(GstXvImage, 1);
  image->conn = xvconn;
  
#ifdef USE_SHM
  image->shm_info = g_new (XShmSegmentInfo, 1);
  image->xvimage = XvShmCreateImage (xvconn->display, xvconn->port, 
	 xvconn->format, 0, xvconn->w, xvconn->h, image->shm_info);
#else /* !USE_SHM */
  image->xvimage = XvCreateImage (xvconn->display, xvconn->port, 
	 xvconn->format, 0, xvconn->w, xvconn->h);
#endif /* !USE_SHM */
  if (image->xvimage == NULL)
  {
    g_warning ("XvImage: CreateImage failed");
    g_free (image->shm_info);
    g_free (image);
    return NULL;
  }
    
#ifdef USE_SHM  
  image->shm_info->shmid = shmget (IPC_PRIVATE,
			  image->xvimage->data_size,
			  IPC_CREAT | 0777);
  if (image->shm_info->shmid == -1)
  {
    g_warning ("XvImage: shmget failed!");
    XFree (image->xvimage);
    g_free (image->shm_info);
    g_free (image);
    return NULL;
  }
  image->shm_info->readOnly = False;
  image->shm_info->shmaddr = shmat (image->shm_info->shmid, 0, 0);
  image->xvimage->data = image->shm_info->shmaddr;
  if (image->shm_info->shmaddr == (char*) -1)
  {
    g_warning ("XvImage: shmat failed!");
    XFree (image->xvimage);
    shmctl (image->shm_info->shmid, IPC_RMID, 0);
    g_free (image->shm_info);
    g_free (image);
    return NULL;
  }
  XShmAttach (xvconn->display, image->shm_info);
  XSync (xvconn->display, False);	      
   /* We mark the segment as destroyed so that when
    * the last process detaches, it will be deleted.
    * There is a small possibility of leaking if
    * we die in XShmAttach. In theory, a signal handler
    * could be set up.
    */
  shmctl (image->shm_info->shmid, IPC_RMID, 0);		      
#else /* ! USE_SHM */
  image->xvimage->data = g_malloc (image->xvimage->data_size);
  if (image->xvimage->data == NULL)
  {
    g_warning ("XvImage: data allocation failed!");
    XFree (image->xvimage);
    g_free (image);
    return NULL;
  }
#endif /* ! USE_SHM */

  image->data.data = image->xvimage->data;
  image->data.size = image->xvimage->data_size;

  return (GstImageData *) image;
}
static void 		
gst_xvimage_put_image (GstImageInfo *info, GstImageData *image)
{
  GstXvImage *im = (GstXvImage *) image;
  GstXImageInfo *xinfo = gst_ximage_info (info);  
  
  /* checks omitted for speed (and lazyness), do we need them? */
  g_assert (xinfo != NULL);
  
  /* g_print ("%p %p - %d %d %d %d\n", xinfo->window, xinfo->gc, xinfo->x, xinfo->y, xinfo->w, xinfo->h); */
#ifdef USE_SHM  
    XvShmPutImage (im->conn->display, im->conn->port, xinfo->window, xinfo->gc, im->xvimage, 0, 0, 
		   im->conn->w, im->conn->h, xinfo->x, xinfo->y, xinfo->w, xinfo->h, False);
#else /* ! USE_SHM */
    XvPutImage (im->conn->display, im->conn->port, xinfo->window, xinfo->gc, im->xvimage, 0, 0, 
	        im->conn->w, im->conn->h, xinfo->x, xinfo->y, xinfo->w, xinfo->h);
#endif /* ! USE_SHM */
  XSync(im->conn->display, False);
}
static void 		
gst_xvimage_free_image (GstImageData *image)
{
  GstXvImage *im = (GstXvImage *) image;
  
  g_return_if_fail (im != NULL);

#ifdef USE_SHM
  XShmDetach (im->conn->display, im->shm_info);
#endif /* USE_SHM */
  XFree (im->xvimage);
#ifdef USE_SHM
  shmdt (im->shm_info->shmaddr);      
  g_free (im->shm_info);
#endif /* USE_SHM */
  g_free (image);
}
static void
gst_xvimage_open_conn (GstImageConnection *conn, GstImageInfo *info)
{
  const GstXvConnection * const xvconn = gst_xv_connection (conn);
  Display * const display = xvconn->display;
  const XvPortID port = xvconn->port;
  
  int i, count;
  const XvAttribute * const attr = XvQueryPortAttributes(display, port, &count);
  static const char autopaint[] = "XV_AUTOPAINT_COLORKEY";

  for (i = 0; i < count; ++i)
    if (!strcmp(attr[i].name, autopaint))
      {
	const Atom atom = XInternAtom(display, autopaint, False);
	XvSetPortAttribute(display, port, atom, 1);
	break;
      }
}
static void
gst_xvimage_close_conn (GstImageConnection *conn, GstImageInfo *info)
{
  GstXvConnection *xvconn = gst_xv_connection (conn);  
  GstXImageInfo *xinfo = gst_ximage_info (info);  
  
  /* checks omitted for speed (and lazyness), do we need them? */
  if (xinfo != NULL) {
    XvStopVideo (xvconn->display, xvconn->port, xinfo->window);
  }
}
static void 			
gst_xvimage_free_conn (GstImageConnection *conn)
{
  GstXvConnection *xvconn = gst_xv_connection (conn);  

  g_free (xvconn);
}
