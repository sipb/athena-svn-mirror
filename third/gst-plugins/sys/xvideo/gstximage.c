/* Based on xqcam.c by Paul Chinn <loomer@svpal.org> */
 
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

#include "gstximage.h"
#include "gstxwindow.h"

static int XJ_caught_error;

/* FIXME: this could cause problems with multiple xvideosinks */
static int 
XJ_error_catcher (Display * d, XErrorEvent * xeev)
{
/* NOTE: This is just for debug, don't enable the printf normally.
 * When shm is unavailable this path gets taken.
 */
/*
  char buf[255];
  XGetErrorText(d, xeev->error_code, buf, 255);
  fprintf(stderr, "Caught X Error: %s\n", buf);
*/
  ++XJ_caught_error;
  return 0;
}

static void 	_gst_ximage_destroy 	(GstXImage *image);
static void 	_gst_ximage_put		(GstXWindow *window, GstXImage *image);

void
_gst_ximage_init (void)
{
}

GstXImage *
_gst_ximage_new (GstXWindow *window, int width, int height)
{
  int (*old_handler)();
  GstXImage *new;

  new = g_new (GstXImage, 1);
  
  GST_IMAGE_TYPE (new) 		= GST_TYPE_XIMAGE;
  GST_IMAGE_DESTROYFUNC (new) 	= (GstImageDestroyFunc) _gst_ximage_destroy;
  GST_IMAGE_PUTFUNC (new) 	= (GstImagePutFunc) _gst_ximage_put;
  
  new->width = width;
  new->height = height;
  new->window = window;
  new->visual = DefaultVisual(window->disp, window->screen_num); 
  new->endianness = (ImageByteOrder (window->disp) == LSBFirst) ? G_LITTLE_ENDIAN:G_BIG_ENDIAN;

  XJ_caught_error = 0;

  old_handler = XSetErrorHandler(XJ_error_catcher);
  XSync(window->disp, 0);

  new->ximage = XShmCreateImage(window->disp, new->visual,
			   window->depth, ZPixmap, NULL, &new->SHMInfo,
			   new->width, new->height);
  if(!new->ximage) {
    fprintf(stderr, "CreateImage Failed\n");
    return NULL;
  }
 
  GST_IMAGE_SIZE (new) = new->ximage->bytes_per_line * new->ximage->height;

  new->SHMInfo.shmid = shmget(IPC_PRIVATE, 
		       GST_IMAGE_SIZE (new),
		       IPC_CREAT|0777);

  if (new->SHMInfo.shmid < 0) {
    perror("shmget failed:");
    return NULL;
  }
 
  GST_IMAGE_DATA (new) = new->ximage->data = new->SHMInfo.shmaddr = shmat(new->SHMInfo.shmid, 0, 0);

  if (new->SHMInfo.shmaddr < 0) {
    perror("shmat failed:");
    return NULL;
  }

  new->SHMInfo.readOnly = False;

  if (!XShmAttach(window->disp, &new->SHMInfo)) {
    fprintf(stderr, "XShmAttach failed\n");
    return NULL;;
  }

  XSync(window->disp, 0);
  XSetErrorHandler(old_handler);

  if (XJ_caught_error) {
    /* This path gets taken when shm is unavailable */
    /* fprintf(stderr, "Shared memory unavailable, using regular images\n"); */
    shmdt(new->SHMInfo.shmaddr);
    new->SHMInfo.shmaddr = 0;

    GST_IMAGE_DATA (new) = g_malloc (((window->depth + 7) / 8) * new->width * new->height);

    new->ximage = XCreateImage (window->disp, DefaultVisual (window->disp, window->screen_num), 
			  window->depth, ZPixmap, 0, 
			  GST_IMAGE_DATA (new), 
			  new->width, new->height, window->depth,
			  new->width * ((window->depth + 7) / 8));
    if(!new->ximage) {
      fprintf(stderr, "CreateImage Failed\n");
      return NULL;
    }
  }
  
  return new;
}


static void 
_gst_ximage_destroy (GstXImage *image)
{
  if (image->SHMInfo.shmaddr)
    XShmDetach(image->window->disp, &image->SHMInfo);
  if (image->ximage)
    XDestroyImage(image->ximage);
  if (image->SHMInfo.shmaddr)
    shmdt(image->SHMInfo.shmaddr);
  if (image->SHMInfo.shmid > 0)
    shmctl(image->SHMInfo.shmid, IPC_RMID, 0);
  g_free (image);
}

   
static void
_gst_ximage_put (GstXWindow *window, GstXImage *image)
{
 if (image->SHMInfo.shmaddr) {
   XShmPutImage(window->disp, window->win, 
		window->gc, image->ximage, 
		0, 0, 0, 0, image->width, image->height, 
		False);
 } else {
   XPutImage(window->disp, window->win, 
	     window->gc, image->ximage,  
	     0, 0, 0, 0, image->width, image->height);
 }
 XSync(window->disp, False);
}
