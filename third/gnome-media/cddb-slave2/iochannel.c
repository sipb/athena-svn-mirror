/* GNet - Networking library
 * Copyright (C) 2000  David Helder
 * Copyright (C) 2000  Andrew Lanoix
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


#include "gnet-private.h"
#include "gnet.h"


/**
 * gnet_io_channel_writen:
 * @channel: the channel to write to
 * @buf: the buffer to read from
 * @len: length of the buffer
 * @bytes_written: pointer to integer for the function to store the
 * number of bytes writen.
 * 
 * Write all @len bytes in the buffer to the channel.  This is
 * basically a wrapper around g_io_channel_write().  The problem with
 * g_io_channel_write() is that it may not write all the bytes in the
 * buffer and return a short count even when there was not an error
 * (this is rare, but it can happen and is often difficult to detect
 * when it does).
 *
 * Returns: %G_IO_ERROR_NONE if everything is ok; something else
 * otherwise.  Also, returns the number of bytes writen by modifying
 * the integer pointed to by @bytes_written.
 *
 **/
GIOError
gnet_io_channel_writen (GIOChannel    *channel, 
			gpointer       buf, 
			guint          len,
			guint         *bytes_written)
{
  guint nleft;
  guint nwritten;
  gchar* ptr;
  GIOError error = G_IO_ERROR_NONE;

  ptr = buf;
  nleft = len;

  while (nleft > 0)
    {
      if ((error = g_io_channel_write(channel, ptr, nleft, &nwritten))
	  != G_IO_ERROR_NONE)
	{
	  if (error == G_IO_ERROR_AGAIN)
	    nwritten = 0;
	  else
	    break;
	}

      nleft -= nwritten;
      ptr += nwritten;
    }

  *bytes_written = (len - nleft);

  return error;
}



/**
 * gnet_io_channel_readn:
 * @channel: the channel to read from
 * @buf: the buffer to write to
 * @len: the length of the buffer
 * @bytes_read: pointer to integer for the function to store the 
 * number of of bytes read.
 *
 * Read exactly @len bytes from the channel the buffer to the channel.
 * This is basically a wrapper around g_io_channel_read().  The
 * problem with g_io_channel_read() is that it may not read all the
 * bytes wanted and return a short count even when there was not an
 * error (this is rare, but it can happen and is often difficult to
 * detect when it does).
 *
 * Returns: %G_IO_ERROR_NONE if everything is ok; something else
 * otherwise.  Also, returns the number of bytes read by modifying the
 * integer pointed to by @bytes_read.  If @bytes_read is 0, the end of
 * the file has been reached (eg, the socket has been closed).
 *
 **/
GIOError
gnet_io_channel_readn (GIOChannel    *channel, 
		       gpointer       buf, 
		       guint          len,
		       guint         *bytes_read)
{
  guint nleft;
  guint nread;
  gchar* ptr;
  GIOError error = G_IO_ERROR_NONE;

  ptr = buf;
  nleft = len;

  while (nleft > 0)
    {
      if ((error = g_io_channel_read(channel, ptr, nleft, &nread))
	  !=  G_IO_ERROR_NONE)
	{
	  if (error == G_IO_ERROR_AGAIN)
	    nread = 0;
	  else
	    break;
	}
      else if (nread == 0)
	break;

      nleft -= nread;
      ptr += nread;
    }

  *bytes_read = (len - nleft);

  return error;
}


/**
 * gnet_io_channel_readline:
 * @channel: the channel to read from
 * @buf: the buffer to write to
 * @len: length of the buffer
 * @bytes_read: pointer to integer for the function to store the 
 *   number of of bytes read.
 *
 * Read a line from the channel.  The line will be null-terminated and
 * include the newline character.  If there is not enough room for the
 * line, the line is truncated to fit in the buffer.
 * 
 * Warnings: (in the gotcha sense, not the bug sense)
 * 
 * 1. If the buffer is full and the last character is not a newline,
 * the line was truncated.  So, do not assume the buffer ends with a
 * newline.
 *
 * 2. @bytes_read is actually the number of bytes put in the buffer.
 * That is, it includes the terminating null character.
 * 
 * 3. Null characters can appear in the line before the terminating
 * null (I could send the string "Hello world\0\n").  If this matters
 * in your program, check the string length of the buffer against the
 * bytes read.
 *
 * I hope this isn't too confusing.  Usually the function works as you
 * expect it to if you have a big enough buffer.  If you have the
 * Stevens book, you should be familiar with the semantics.
 *
 * Returns: %G_IO_ERROR_NONE if everything is ok; something else
 * otherwise.  Also, returns the number of bytes read by modifying the
 * integer pointed to by @bytes_read (this number includes the
 * newline).  If an error is returned, the contents of @buf and
 * @bytes_read are undefined.
 * 
 **/
GIOError
gnet_io_channel_readline (GIOChannel    *channel, 
			  gchar         *buf, 
			  guint          len,
			  guint         *bytes_read)
{
  guint n, rc;
  gchar c, *ptr;
  GIOError error = G_IO_ERROR_NONE;

  ptr = buf;

  for (n = 1; n < len; ++n)
    {
    try_again:
      error = gnet_io_channel_readn(channel, &c, 1, &rc);

      if (error == G_IO_ERROR_NONE && rc == 1)		/* read 1 char */
	{
	  *ptr++ = c;
	  if (c == '\n')
	    break;
	}
      else if (error == G_IO_ERROR_NONE && rc == 0)	/* read EOF */
	{
	  if (n == 1)	/* no data read */
	    {
	      *bytes_read = 0;
	      return G_IO_ERROR_NONE;
	    }
	  else		/* some data read */
	    break;
	}
      else
	{
	  if (error == G_IO_ERROR_AGAIN)
	    goto try_again;

	  return error;
	}
    }

  *ptr = 0;
  *bytes_read = n;

  return error;
}



/**
 * gnet_io_channel_readline_strdup:
 * @channel: the channel to read from
 * @buf_ptr: pointer to gchar* for the functin to store the new buffer
 * @bytes_read: pointer to integer for the function to store the 
 *   number of of bytes read.
 *
 * Read a line from the channel.  The line will be null-terminated and
 * include the newline character.  Similarly to g_strdup_printf, a
 * buffer large enough to hold the string will be allocated.
 * 
 * Warnings: (in the gotcha sense, not the bug sense)
 * 
 * 1. If the last character of the buffer is not a newline, the line
 * was truncated by EOF.  So, do not assume the buffer ends with a
 * newline.
 *
 * 2. @bytes_read is actually the number of bytes put in the buffer.
 * That is, it includes the terminating null character.
 * 
 * 3. Null characters can appear in the line before the terminating
 * null (I could send the string "Hello world\0\n").  If this matters
 * in your program, check the string length of the buffer against the
 * bytes read.
 *
 * Returns: %G_IO_ERROR_NONE if everything is ok; something else
 * otherwise.  Also, returns the number of bytes read by modifying the
 * integer pointed to by @bytes_read (this number includes the
 * newline), and the data through the pointer pointed to by @buf_ptr.
 * This data should be freed with g_free().  If an error is returned,
 * the contents of @buf_ptr and @bytes_read are undefined.
 *
 **/
GIOError
gnet_io_channel_readline_strdup (GIOChannel    *channel, 
				 gchar         **buf_ptr, 
				 guint         *bytes_read)
{
  guint rc, n, len;
  gchar c, *ptr, *buf;
  GIOError error = G_IO_ERROR_NONE;

  len = 100;
  buf = (gchar *)g_malloc(len);
  ptr = buf;
  n = 1;

  while (1)
    {
    try_again:
      error = gnet_io_channel_readn(channel, &c, 1, &rc);

      if (error == G_IO_ERROR_NONE && rc == 1)          /* read 1 char */
        {
          *ptr++ = c;
          if (c == '\n')
            break;
        }
      else if (error == G_IO_ERROR_NONE && rc == 0)     /* read EOF */
        {
          if (n == 1)   /* no data read */
            {
              *bytes_read = 0;
	      *buf_ptr = NULL;
	      g_free(buf);

              return G_IO_ERROR_NONE;
            }
          else          /* some data read */
            break;
        }
      else
        {
          if (error == G_IO_ERROR_AGAIN)
            goto try_again;

          g_free(buf);

          return error;
        }

      ++n;

      if (n >= len)
        {
          len *= 2;
          buf = g_realloc(buf, len);
          ptr = buf + n - 1;
        }
    }

  *ptr = 0;
  *buf_ptr = buf;
  *bytes_read = n;

  return error;
}
