/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xmd.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#ifdef USE_SHM
#include <X11/extensions/XShm.h>
#endif /* USE_SHM */

#ifdef HAVE_XVIDEO
# include <X11/extensions/Xv.h>
# include <X11/extensions/Xvlib.h>
#endif

#include "gstxvimage.h"

#ifdef HAVE_XVIDEO
static int ver, rel, req, ev, err;
static int formats;
static int adaptors; 
static XvAdaptorInfo        *ai;
static XvImageFormatValues  *fo;
static int im_adaptor = -1, im_port = -1;
static int im_format = -1;
#endif
static GstCaps *capslist = NULL;
/* 
 * Desc: query the server for support for the MIT_SHM extension
 * Return:  0 = not available
 *          1 = shared XImage support available
 *          2 = shared Pixmap support available also
 */
/* defined but not used
static int
_gst_xvimage_check_xshm(void)
{
#ifdef USE_SHM
  int major, minor, ignore;
  Bool pixmaps;
  Display *display = XOpenDisplay(NULL);
  
  if (display == NULL)
    return 0;

  if (XQueryExtension(display, "MIT-SHM", &ignore, &ignore, &ignore)) 
    {
      if (XShmQueryVersion(display, &major, &minor, &pixmaps )==True) 
	{
	  return (pixmaps==True) ? 2 : 1;
	}
    }
#endif
  return 0;
}
*/

static void 	_gst_xvimage_destroy 		(GstXvImage *image);
static void 	_gst_xvimage_put		(GstXWindow *window, GstXvImage *image);

gboolean
_gst_xvimage_check_xvideo(void)
{
#ifdef HAVE_XVIDEO
  Display *display = XOpenDisplay(NULL);

  if (display == NULL)
    return FALSE;

  if (Success == XvQueryExtension(display,&ver,&rel,&req,&ev,&err)) {
    return TRUE;
  }
#endif /* HAVE_XVIDEO */
  return FALSE;
}

GstCaps*
_gst_xvimage_get_capslist (void)
{
  return capslist;
}

void
_gst_xvimage_init(void)
{
#ifdef HAVE_XVIDEO
  int i;
    
  Display *display = XOpenDisplay(NULL);

  if (display == NULL)
    return;

  if (!_gst_xvimage_check_xvideo()) {
    g_warning("Xv: Server has no Xvideo extention support\n");
    return;
  }
  if (Success != XvQueryAdaptors(display,DefaultRootWindow(display),&adaptors,&ai)) {
    g_error("Xv: XvQueryAdaptors failed");
    return;
  }
  GST_INFO(GST_CAT_PLUGIN_INFO, "Xv: %d adaptors available.",adaptors);

  for (i = 0; i < adaptors; i++) {
    GST_INFO(GST_CAT_PLUGIN_INFO, "Xv: %s:%s%s%s%s%s, ports %ld-%ld",
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
      if (im_port == -1) {
        GstCaps *caps = NULL;
	gint j;
	      
        im_port = ai[i].base_id;
        im_adaptor = i;

	{
	  int count;
	  const XvAttribute * const attr = XvQueryPortAttributes(display, im_port, &count);
	  static const char autopaint[] = "XV_AUTOPAINT_COLORKEY";

	  for (j = 0; j < count; ++j)
	    if (!strcmp(attr[j].name, autopaint))
	      {
		const Atom atom = XInternAtom(display, autopaint, False);
		XvSetPortAttribute(display, im_port, atom, 1);
		break;
	      }
	}

        /* *** image scaler port *** */
        fo = XvListImageFormats(display, im_port, &formats);

        GST_INFO(GST_CAT_PLUGIN_INFO,"  image format list for port %d",im_port);
        for(j = 0; j < formats; j++) {
          gulong fourcc;

          fourcc = GULONG_FROM_LE(fo[j].id);

          GST_INFO(GST_CAT_PLUGIN_INFO, "    0x%x (%4.4s) %s %.32s (%d:%d;%d,%d:%d:%d,%d:%d:%d)",
                       fo[j].id,
                       (char*)&fourcc,
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
		       fo[j].vert_v_period
			 );
          caps = gst_caps_new (
			  "xvideosink_caps",
		          "video/raw", 
          		  gst_props_new (
			    "format",  GST_PROPS_FOURCC (fo[j].id),
			      "width",   GST_PROPS_INT_RANGE (0, G_MAXINT),
			      "height",   GST_PROPS_INT_RANGE (0, G_MAXINT),
			     NULL
			  ));

  	  capslist = gst_caps_append (capslist, caps);
        }
      }
    }
  }
#else
  g_warning ("compiled without Xvideo extension support\n");
#endif
}

GstXvImage*
_gst_xvimage_new (GstXvImageType  type,
		  GstXWindow *window,
	          gint          width,
	          gint          height)
{
#ifdef HAVE_XVIDEO
#ifdef USE_SHM
  GstXvImage *image = NULL;
  XShmSegmentInfo *x_shm_info;
  gint i;
  gboolean have_xv_scale = FALSE;

  image = g_new (GstXvImage, 1);

  GST_IMAGE_TYPE (image) 	= GST_TYPE_XVIMAGE;
  GST_IMAGE_DESTROYFUNC (image) = (GstImageDestroyFunc) _gst_xvimage_destroy;
  GST_IMAGE_PUTFUNC (image) 	= (GstImagePutFunc) _gst_xvimage_put;

  image->type = type;
  image->width = width;
  image->height = height;
  image->window = window;

  /* *** image scaler port *** */
  if (im_port == -1) {
    GST_INFO(GST_CAT_PLUGIN_INFO, "Xv: no usable hw scaler port found");
    return NULL;
  } else {
    fo = XvListImageFormats(window->disp, im_port, &formats);
    for(i = 0; i < formats; i++) {
       if (type == fo[i].id) {
	 have_xv_scale = 1;
	 im_format = fo[i].id;
       }
    }
    if (!have_xv_scale) {
      GST_INFO(GST_CAT_PLUGIN_INFO,"Xv: no usable image format found (port %d)",
                   im_port);
      return NULL;
    }
  }

  image->x_shm_info = g_new (XShmSegmentInfo, 1);
  image->im_port = im_port;
  x_shm_info = image->x_shm_info;
  image->im_format = im_format;

  image->ximage = XvShmCreateImage(window->disp, image->im_port, 
     image->im_format, 0, width, height, x_shm_info);

  if (image->ximage == NULL)
  {
    g_warning ("XvShmCreateImage failed");
	  
    g_free (image);
    return NULL;
  }
    
  x_shm_info->shmid = shmget (IPC_PRIVATE,
			  image->ximage->data_size,
			  IPC_CREAT | 0777);

  if (x_shm_info->shmid == -1)
  {
    g_warning ("shmget failed!");

    XFree (image->ximage);
    g_free (image->x_shm_info);
    g_free (image);

    return NULL;
  }

  x_shm_info->readOnly = False;
  x_shm_info->shmaddr = shmat (x_shm_info->shmid, 0, 0);
  image->ximage->data = x_shm_info->shmaddr;

  if (x_shm_info->shmaddr == (char*) -1)
  {
    g_warning ("shmat failed!");

    XFree (image->ximage);
    shmctl (x_shm_info->shmid, IPC_RMID, 0);
		  
    g_free (image->x_shm_info);
    g_free (image);

    return NULL;
  }

  XShmAttach (window->disp, x_shm_info);
  XSync (window->disp, False);

  if (0)
  {
    /* this is the common failure case so omit warning */
    XFree (image->ximage);
    shmdt (x_shm_info->shmaddr);
    shmctl (x_shm_info->shmid, IPC_RMID, 0);
                  
    g_free (image->x_shm_info);
    g_free (image);


    return NULL;
  }
	      
   /* We mark the segment as destroyed so that when
    * the last process detaches, it will be deleted.
    * There is a small possibility of leaking if
    * we die in XShmAttach. In theory, a signal handler
    * could be set up.
    */
  shmctl (x_shm_info->shmid, IPC_RMID, 0);		      

  if (image)
  {
    GST_IMAGE_DATA (image) = image->ximage->data;
    GST_IMAGE_SIZE (image) = image->ximage->data_size;
  }

  return image;
#else
  g_error ("compiled without shared memory support: Xv image creation not supported");
  return NULL;
#endif /* USE_SHM */
#else
  g_error ("compiled without Xvideo extention support\n");
#endif
  return NULL; /* well tell me if it's wrong to do this */
}

static void
_gst_xvimage_destroy (GstXvImage *image)
{
#ifdef USE_SHM
  XShmSegmentInfo *x_shm_info;
#endif /* USE_SHM */

  g_return_if_fail (image != NULL);

#ifdef USE_SHM
  XShmDetach (image->window->disp, image->x_shm_info);
  XFree (image->ximage);

  x_shm_info = image->x_shm_info;
  shmdt (x_shm_info->shmaddr);
      
  g_free (image->x_shm_info);

#else /* USE_SHM */
  g_error ("trying to destroy shared memory image when gst was compiled without shared memory support");
#endif /* USE_SHM */

  g_free (image);
}

static void
_gst_xvimage_put (GstXWindow *window,
		 GstXvImage *image)
{
#ifdef HAVE_XVIDEO
#ifdef USE_SHM
  XWindowAttributes attr; 

  g_return_if_fail (window != NULL);
  g_return_if_fail (image != NULL);

  XGetWindowAttributes(window->disp, window->win, &attr); 

  XvShmPutImage (window->disp, image->im_port, window->win,
		window->gc, image->ximage,
		0, 0, image->width, image->height, 
		0, 0, attr.width, attr.height, False);
  XSync(window->disp, False);

#else /* USE_SHM */
  g_error ("trying to draw shared memory image when gst was compiled without shared memory support");
#endif /* USE_SHM */
#endif /* HAVE_XVIDEO */
}

