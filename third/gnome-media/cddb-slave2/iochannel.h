/* GNet - Networking library
 * Copyright (C) 2000  David Helder
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
 * Boston, MA  02111-1307, USA.
 */

#ifndef _GNET_IOCHANNEL_H
#define _GNET_IOCHANNEL_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


GIOError gnet_io_channel_writen (GIOChannel    *channel, 
				 gpointer       buf, 
				 guint          len,
				 guint         *bytes_written);

GIOError gnet_io_channel_readn (GIOChannel    *channel, 
				 gpointer      buf, 
				 guint         len,
				 guint        *bytes_read);

GIOError gnet_io_channel_readline (GIOChannel    *channel, 
				   gchar         *buf, 
				   guint          len,
				   guint         *bytes_read);

GIOError gnet_io_channel_readline_strdup (GIOChannel    *channel, 
					  gchar         **buf_ptr, 
					  guint         *bytes_read);

/* **************************************** */

/* This part is experimental, buggy, and unstable.  Use at your own risk. */
#ifdef GNET_EXPERIMENTAL 


/*
  Notes

  _Do_not_ call cancel if the callback has been called.  For example, if
  you receive a callback with the status of TIMEOUT, _do_not_ cancel
  the read or write.

  A timeout of 0 means there is no timeout.  If you are writing a
  server, you should use a timeout.  2 minutes is reasonable for most
  small things.

*/


typedef enum {
  GNET_IOCHANNEL_WRITE_ASYNC_STATUS_OK,
  GNET_IOCHANNEL_WRITE_ASYNC_STATUS_TIMEOUT,
  GNET_IOCHANNEL_WRITE_ASYNC_STATUS_ERROR
} GNetIOChannelWriteAsyncStatus;

typedef gpointer GNetIOChannelWriteAsyncID;

typedef
void (*GNetIOChannelWriteAsyncFunc)(GIOChannel* iochannel,
				    gchar* buffer, /* callee owns */
				    guint length,
				    guint bytes_writen,
				    GNetIOChannelWriteAsyncStatus status, 
				    gpointer user_data);

GNetIOChannelWriteAsyncID
gnet_io_channel_write_async (GIOChannel* iochannel, 
			     gchar* buffer, guint length, 
			     guint timeout,
			     GNetIOChannelWriteAsyncFunc func, 
			     gpointer user_data);

void gnet_io_channel_write_async_cancel (GNetIOChannelWriteAsyncID id, 
					 gboolean delete_buffer);



/* **************************************** */

typedef enum {
  GNET_IOCHANNEL_READ_ASYNC_STATUS_OK,
  GNET_IOCHANNEL_READ_ASYNC_STATUS_TIMEOUT,
  GNET_IOCHANNEL_READ_ASYNC_STATUS_ERROR
} GNetIOChannelReadAsyncStatus;

typedef gpointer GNetIOChannelReadAsyncID;

typedef
gboolean (*GNetIOChannelReadAsyncFunc)(GIOChannel* iochannel, 
				       GNetIOChannelReadAsyncStatus status, 
				       gchar* buffer, guint length, 
				       gpointer user_data);

/* ID is invalid if there is an error.  If OK and buffer and length is
   0, then it's an EOF. */

typedef
gint (*GNetIOChannelReadAsyncCheckFunc)(gchar* buffer, guint length, 
					gpointer user_data);
/* Return -1 if error, otherwise number of bytes read. */

/* 

   Set buffer to NULL if you want the buffer created dynamicly.
   Length will be the maximum length of the buffer.  GNet owns the
   buffer if you do this.

   If read_one_byte_at_a_time is TRUE, it will read one byte at a time
   and call the check function.  This is really used for the readline
   function when you read a line, return FALSE, and then call readany.
   The problem is that readline may have stuff still in its buffer
   that now gets lost.  This would be bad.

   If you call read_async once and always return TRUE, then don't set
   read_one_byte_at_a_time.

   \0 is appended if there is room.

*/


GNetIOChannelReadAsyncID
gnet_io_channel_read_async (GIOChannel* iochannel, 
			    gchar* buffer, guint length, 
			    guint timeout, 
			    gboolean read_one_byte_at_a_time, 
			    GNetIOChannelReadAsyncCheckFunc check_func, 
			    gpointer check_user_data,
			    GNetIOChannelReadAsyncFunc func, 	      
			    gpointer user_data);

void gnet_io_channel_read_async_cancel (GNetIOChannelReadAsyncID id);

gint gnet_io_channel_readany_check_func (gchar* buffer, guint length, 
					gpointer data);
gint gnet_io_channel_readline_check_func (gchar* buffer, guint length, 
					 gpointer data);

#define gnet_io_channel_readany_async(IO, BUF, LEN, TO, FUNC, UD)	\
  gnet_io_channel_read_async (IO, BUF, LEN, TO, FALSE, 			\
	             gnet_io_channel_readany_check_func, NULL, FUNC, UD)

#define gnet_io_channel_readline_async(IO, BUF, LEN, TO, FUNC, UD)	\
  gnet_io_channel_read_async (IO, BUF, LEN, TO, TRUE, 			\
		     gnet_io_channel_readline_check_func, NULL, FUNC, UD)


#endif /* GNET_EXPERIMENTAL */


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif /* _GNET_IOCHANNEL_H */
